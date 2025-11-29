ledbuffer_test_exec = executable(
    'ledbuffer_test',
    ['main.cpp'],
    dependencies: [doctest_dep, trompeloeil_dep],
    include_directories: project_include_dirs,
    native: true,
)

test('test ledbuffer', ledbuffer_test_exec)
