# Version is injected by packaging/rpm/Makefile via `zfr version`.
# RPM Version cannot contain '-'; use `zfr version -r` (hyphens → '_').
# srcversion is the unsanitized Meson/git version and names the tarball.
%{!?version:%global version 0.0.0}
%{!?srcversion:%global srcversion %{version}}

Name:           naan
Version:        %{version}
Release:        1%{?dist}
Summary:        compute a number from a name

License:        AGPL-3.0-or-later
URL:            https://github.com/lenik/naan
Packager:       Lenik <naan@bodz.net>
Source0:        %{name}-%{srcversion}.tar.xz

BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  pkgconf
BuildRequires:  openssl-devel
BuildRequires:  gettext
BuildRequires:  asciidoctor

%description
naan hashes each name and prints one decimal number:
(digest(NAME) mod DIV) + BIAS. Profiles set the hash and DIV/BIAS
(b/w/dw use SHA-256; wm uses SHA-1 in 2000 through 3999).

%prep
%setup -q -n %{name}-%{srcversion}

%build
meson setup build \
    --prefix=%{_prefix} \
    --bindir=%{_bindir} \
    --datadir=%{_datadir} \
    --mandir=%{_mandir} \
    --sysconfdir=%{_sysconfdir} \
    --localstatedir=%{_localstatedir} \
    --buildtype=plain
meson compile -C build

%install
meson install -C build --destdir=%{buildroot}

%files
%{_bindir}/naan
%{_datadir}/bash-completion/completions/naan
%{_mandir}/man1/naan.1*
%{_mandir}/*/man1/naan.1*
%{_datadir}/locale/*/LC_MESSAGES/naan.mo
%{_datadir}/doc/%{name}/

%changelog
* Thu Aug 20 2026 Lenik <naan@bodz.net>
- Align spec with debian/control (Meson, AGPL-3.0-or-later).
- Version comes from `zfr version`, the same method meson.build uses.
