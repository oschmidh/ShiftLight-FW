polledtimer_test_exec = executable(
    'polledtimer_test',
    ['main.cpp'],
    dependencies: [doctest_dep, trompeloeil_dep, shiftlight_lib],
    include_directories: project_include_dirs,
    native: true,
)

test('test polledtimer', polledtimer_test_exec)
