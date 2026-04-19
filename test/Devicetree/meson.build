devicetree_test_exec = executable(
    'devicetree_test',
    ['main.cpp'],
    dependencies: [shiftlight_lib, doctest_dep, trompeloeil_dep],
    include_directories: project_include_dirs,
    native: true,
)

test('test devicetree', devicetree_test_exec)
