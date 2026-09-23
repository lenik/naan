#!/usr/bin/env bash
# Verify every scripts/ci/matrix.json cell has a zip on the GitHub Release.
# Usage: verify-release-assets.sh <tag>
set -euo pipefail
TAG=${1:?tag}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
NAME=$(basename "$ROOT")
MATRIX="$ROOT/scripts/ci/matrix.json"

assets=$(gh release view "$TAG" --json assets -q '.assets[].name' | sort)
echo "$assets"
missing=0

while read -r kind release arch; do
  [ -z "$kind" ] && continue
  case "$kind" in
    deb) want="${NAME}-debian-${release}-${arch}.zip" ;;
    rpm) want="${NAME}-el-${release}-${arch}.zip" ;;
    mingw) want="${NAME}-mingw-${arch}.zip" ;;
    ucrt) want="${NAME}-ucrt-${arch}.zip" ;;
    *) continue ;;
  esac
  if ! printf '%s\n' "$assets" | grep -qxF "$want"; then
    echo "MISSING: $want"
    missing=$((missing + 1))
  else
    echo "OK: $want"
  fi
done < <(python3 - "$MATRIX" <<'PY'
import json, sys
from pathlib import Path
data = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
for kind in ("deb", "rpm", "mingw", "ucrt"):
    for cell in data.get(kind) or []:
        rel = cell.get("release", "")
        print(kind, rel, cell["arch"])
PY
)

if [ "$missing" -gt 0 ]; then
  echo "verify: $missing matrix package(s) missing from release $TAG" >&2
  exit 1
fi
echo "verify: all matrix packages present on $TAG"
