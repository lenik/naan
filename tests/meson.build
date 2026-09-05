test_exe = executable(
    'name_number_test',
    [
        'name_number_test.c',
        '../src/name_number.c',
    ],
    include_directories: include_directories('../src'),
    dependencies: [crypto_dep],
    install: false,
)

test('name_number', test_exe)

test(
    'naan-cli',
    find_program('naan_cli_test.sh'),
    args: [naan],
)
