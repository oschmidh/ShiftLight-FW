shiftlight_test_exec = executable(
    'shiftlight_test',
    ['main.cpp'],
    dependencies: [doctest_dep, mp_units_dep, shiftlight_lib],
    include_directories: project_include_dirs,
    native: true,
)

test('test shiftlight', shiftlight_test_exec)
