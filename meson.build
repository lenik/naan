project(
    'naan',
    'c',
    version: run_command(
        'sh',
        '-c', '''
        v=$(
            command -v zfr >/dev/null 2>&1 && zfr version 2>/dev/null || true
        )
        if [ -z "$v" ] && [ -f VERSION ]; then
            v=$(head -n1 VERSION | tr -d '\r')
            v=${v#v}
        fi
        if [ -z "$v" ]; then
            v="0.0.0" # FIXED TO 0.0.0, DO NOT MODIFY
        fi
        printf '%s' "$v"
        ''',
        check: false,
    ).stdout().strip(),
    license: 'AGPL-3.0-or-later',
    default_options: [
        'c_std=c99',
        'warning_level=2',
    ],
)

project_author = 'Lenik'
project_email = 'naan@bodz.net'
project_year = 2026

asciidoctor = find_program('asciidoctor', required: true)
fs = import('fs')

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
datadir = prefix / get_option('datadir')
mandir = prefix / get_option('mandir')
localedir = prefix / get_option('localedir')
pkgdocdir = datadir / 'doc' / meson.project_name()

config_h = configuration_data()
config_h.set_quoted('PROJECT_VERSION', meson.project_version())
config_h.set_quoted('PROJECT_AUTHOR', project_author)
config_h.set_quoted('PROJECT_EMAIL', project_email)
config_h.set_quoted('GETTEXT_PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', localedir)
config_h.set('PROJECT_YEAR', project_year)

configure_file(
    output: 'config.h',
    configuration: config_h,
)

inc = include_directories('src', '.')

crypto_dep = dependency('libcrypto')

app_sources = [
    'src/naan.c',
]

shared_sources = [
    'src/name_number.c',
]

foreach source : app_sources
    name = fs.stem(source.split('/')[-1])
    exe = executable(
        name,
        [source] + shared_sources,
        include_directories: inc,
        dependencies: [crypto_dep],
        install: true,
        install_dir: bindir,
    )
    if name == 'naan'
        naan = exe
    endif
endforeach

bash_files = [
    'naan.bash',
]

foreach file : bash_files
    name = fs.stem(file)
    install_data(
        file,
        install_dir: datadir / 'bash-completion' / 'completions',
        rename: name,
    )
endforeach

install_data(
    [
        'LICENSE',
        'README.md',
        'README-zh.md',
    ],
    install_dir: pkgdocdir,
)

subdir('po')
subdir('tests')

run_target(
    'look',
    command: [
        'bash',
        '-euc', '''
            tmpdir=$(mktemp -d)
            DESTDIR=$tmpdir meson install -C "@BUILD_ROOT@"
            tree "$tmpdir"
            rm -fr "$tmpdir"
        ''',
    ],
)

run_target(
    'install-symlinks',
    command: [
        'bash',
        '-euc', '''
            prefix="@0@"
            bindir="@1@"
            datadir="@2@"
            mandir="@3@"
            mkdir -p "$bindir" "$datadir/bash-completion/completions" "$mandir/man1"
            printf "Symlinks installed under %s\n" "$prefix"
        '''.format(get_option('prefix'), bindir, datadir, mandir),
    ],
)

run_target(
    'uninstall-symlinks',
    command: [
        'bash',
        '-euc', '''
            for p in; do
                if [ -L "$p" ]; then
                    sudo rm -f "$p"
                    printf "Removed %s\n" "$p"
                else
                    printf "Skipped (not a symlink): %s\n" "$p"
                fi
            done
        '''.format(get_option('prefix'), bindir, datadir, mandir),
    ],
)

custom_target(
    'naan-man',
    input: 'docs/naan.adoc',
    output: 'naan.1',
    command: [
        asciidoctor,
        '-b', 'manpage',
        '-a', 'project-version=' + meson.project_version(),
        '-a', 'project-year=@0@'.format(project_year),
        '-a', 'project-author=' + project_author,
        '-a', 'project-email=' + project_email,
        '-o', '@OUTPUT@',
        '@INPUT@',
    ],
    build_by_default: true,
    install: true,
    install_dir: mandir / 'man1',
)

man_i18n_langs = [
    'fr', 'it', 'de', 'es_MX', 'zh_CN', 'ja', 'ko', 'ar',
    'id', 'vi', 'th', 'pt_BR', 'nl', 'pl', 'tr', 'ru', 'hi',
    'sv', 'no', 'da', 'fi', 'cs', 'ro', 'el', 'hu', 'bg', 'uk',
    'kk', 'fil_PH', 'bn', 'mr', 'ta', 'te', 'he', 'sw',
]

_install_man_i18n = find_program('sh')
foreach lang : man_i18n_langs
    _man_i18n = custom_target(
        'naan-man-' + lang,
        input: 'docs' / lang / 'naan.adoc',
        output: 'naan-' + lang + '.1',
        command: [
            asciidoctor,
            '-b', 'manpage',
            '-a', 'project-version=' + meson.project_version(),
            '-a', 'project-year=@0@'.format(project_year),
            '-a', 'project-author=' + project_author,
            '-a', 'project-email=' + project_email,
            '-o', '@OUTPUT@',
            '@INPUT@',
        ],
        build_by_default: true,
    )
    meson.add_install_script(
        _install_man_i18n,
        '-c',
        'base="${MESON_INSTALL_DESTDIR_PREFIX:-${DESTDIR:-}${MESON_INSTALL_PREFIX:-}}"; install -Dm644 "$0" "$base/$1"',
        _man_i18n,
        get_option('mandir') / lang / 'man1' / 'naan.1',
    )
endforeach
