project(
    'shiftlight',
    'cpp',
    'c',
    version: run_command('git', 'rev-parse', '--short', 'HEAD').stdout().strip(),
    meson_version : '>= 1.4.0',
    default_options: [
        'c_std=c11', 
        'cpp_std=c++23',
        'b_lto=true',
        'warning_level=1',
        'buildtype=minsize',
        'werror=true',
    ]
)

project_sources = []
project_include_dirs = []

add_project_arguments([
        '-fno-exceptions',
        '-ffunction-sections',
        '-fdata-sections',
        # '-g',
        # '-gdwarf-3',
        # '-gstrict-dwarf',
    ], 
    language: ['c', 'cpp']
)

add_project_arguments(['-fno-rtti', '-fno-unwind-tables', '-fno-threadsafe-statics',], language: 'cpp')

add_project_link_arguments([
    '-fno-exceptions',
    '-fno-unwind-tables',
    '-Wl,-Map=' + meson.project_name() + '.map',
    '-nostartfiles',
    '-static',
    '-Wl,--gc-sections',
    ], 
    language: ['c', 'cpp']
)

linkfiles = files('mspm0c1104.lds')

foreach linkfile : linkfiles
    add_project_link_arguments('-Wl,-T,' + linkfile.full_path(), language: ['c', 'cpp'])
endforeach

ti_driverlib_dep = declare_dependency(
    dependencies : meson.get_compiler('c', native : false).find_library('driverlib', dirs : [join_paths(meson.current_source_dir(), 'mspm0-sdk/source/ti/driverlib/lib/gcc/m0p/mspm0c110x/')]),
    include_directories : include_directories(['mspm0-sdk/source/', 'mspm0-sdk/source/third_party/CMSIS/Core/Include']),
)

subdir('src')

executable(
    meson.project_name(),
    project_sources,
    name_suffix : 'elf',
    dependencies: [
        ti_driverlib_dep,
        lib_lib,
    ],
    include_directories: project_include_dirs,
)
