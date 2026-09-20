#!/usr/bin/env bash
# Build an RPM inside a Rocky/CentOS container (no host tooling, no zfr).
# Usage: build-rpm.sh <image> <platform> <el_release> <arch> [outdir]
set -euo pipefail

IMAGE=${1:?image}
PLATFORM=${2:?platform}
EL=${3:?el_release}
ARCH=${4:?arch}
OUTDIR=${5:-"dist/el-${EL}-${ARCH}"}

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
NAME=$(basename "$ROOT")
VERSION=$(head -n1 "$ROOT/VERSION" 2>/dev/null | tr -d '[:space:]' | sed 's/^v//')
VERSION=${VERSION:-0.0.0}
RPM_VERSION=${VERSION//-/_}
SPEC="$ROOT/packaging/rpm/${NAME}.spec"

if [ ! -f "$SPEC" ]; then
  echo "build-rpm: missing $SPEC" >&2
  exit 1
fi

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$OUTDIR"
OUTDIR=$(cd "$OUTDIR" && pwd)

# Source tarball from the working tree (not git archive) so local fixes are included.
mkdir -p "$STAGE/SOURCES" "$STAGE/SPECS" "$STAGE/RPMS" "$STAGE/BUILD" "$STAGE/BUILDROOT" "$STAGE/SRPMS"
tar -C "$ROOT" \
  --exclude='./.git' \
  --exclude='./build' \
  --exclude='./obj-*' \
  --exclude='./debian/build' \
  --exclude='./debian/tmp' \
  --exclude='./dist' \
  --exclude='./ci-deps' \
  --transform "s,^\\./,${NAME}-${VERSION}/," \
  -cJf "$STAGE/SOURCES/${NAME}-${VERSION}.tar.xz" .

{
  printf '%s\n' "%global version ${RPM_VERSION}" "%global srcversion ${VERSION}" ""
  cat "$SPEC"
} >"$STAGE/SPECS/${NAME}.spec"

if [ -n "${CI_DEPS_DIR:-}" ] && [ -d "$CI_DEPS_DIR" ]; then
  mkdir -p "$STAGE/deps"
  cp -a "$CI_DEPS_DIR"/. "$STAGE/deps/" || true
fi

# RHEL package names for common C build deps; projects may still declare Debian
# names in the spec — we install a baseline set and use --nodeps as fallback.
docker run --rm --platform "$PLATFORM" \
  -v "$STAGE:/rpmbuild" \
  -e NAME="$NAME" \
  -e EL="$EL" \
  -e http_proxy -e https_proxy -e HTTP_PROXY -e HTTPS_PROXY \
  -e no_proxy -e NO_PROXY \
  "$IMAGE" \
  bash -lc '
set -euo pipefail
if command -v dnf >/dev/null 2>&1; then
  PM=dnf
elif command -v yum >/dev/null 2>&1; then
  PM=yum
else
  echo "no dnf/yum" >&2; exit 1
fi
# CentOS 7 vault (mirrors are EOL).
if [[ "${EL}" == "7" ]] && [[ -f /etc/yum.repos.d/CentOS-Base.repo ]]; then
  sed -i \
    -e "s|^mirrorlist=|#mirrorlist=|g" \
    -e "s|^#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g" \
    /etc/yum.repos.d/CentOS-*.repo || true
fi
$PM -y install epel-release 2>/dev/null || true
if command -v dnf >/dev/null 2>&1; then
  $PM -y install dnf-plugins-core 2>/dev/null || true
  $PM config-manager --set-enabled crb 2>/dev/null || \
    $PM config-manager --set-enabled powertools 2>/dev/null || true
fi
$PM -y install rpm-build rpmdevtools pkgconf gcc gcc-c++ make \
  tar xz which python3 python3-pip \
  openssl-devel zlib-devel || true
# meson/ninja: distro packages (EPEL/CRB) or pip fallback.
$PM -y install meson ninja-build 2>/dev/null \
  || pip3 install --no-cache-dir meson ninja
# Optional deps used by bas-c / similar C libs (ignore if unavailable).
$PM -y install glib2-devel libcurl-devel libicu-devel rubygem-asciidoctor asciidoctor \
  gettext gettext-devel bash 2>/dev/null || true
# Optional prebuilt dependency rpms (never nested-build other projects).
if ls /rpmbuild/deps/*.rpm >/dev/null 2>&1; then
  $PM -y install /rpmbuild/deps/*.rpm || rpm -Uvh --nodeps /rpmbuild/deps/*.rpm || true
fi
command -v meson >/dev/null
command -v ninja >/dev/null || command -v ninja-build >/dev/null
rpmbuild --define "_topdir /rpmbuild" -bb /rpmbuild/SPECS/${NAME}.spec || \
  rpmbuild --define "_topdir /rpmbuild" --nodeps -bb /rpmbuild/SPECS/${NAME}.spec
'

shopt -s nullglob
copied=0
for f in "$STAGE"/RPMS/*/*.rpm "$STAGE"/SRPMS/*.rpm; do
  [ -f "$f" ] || continue
  base=$(basename "$f")
  dest="el${EL}_${base}"
  cp -a "$f" "$OUTDIR/$dest"
  copied=$((copied + 1))
done

if [ "$copied" -eq 0 ]; then
  echo "build-rpm: no packages produced for ${NAME} el${EL}/${ARCH}" >&2
  exit 1
fi

(
  cd "$OUTDIR"
  zip -q -r "${NAME}-el-${EL}-${ARCH}.zip" ./*.rpm
)
echo "build-rpm: wrote $copied artifact(s) → $OUTDIR"
