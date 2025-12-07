project(
    'shiftlight',
    'cpp',
    'c',
    version: run_command('git', 'rev-parse', '--short', 'HEAD', check: true).stdout().strip(),
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
    ],
    language: ['c', 'cpp'],
)

add_project_arguments([
        '-fno-rtti',
        '-fno-unwind-tables',
        '-fno-threadsafe-statics',
        '-U_GLIBCXX_ASSERTIONS',
        '-U_FILE_OFFSET_BITS',
    ],
    language: ['cpp'],
    native: false
)

add_project_link_arguments([
        '-fno-exceptions',
        '-fno-unwind-tables',
        '-static',
        '-Wl,--gc-sections',
        '-Wl,-Map=' + meson.project_name() + '.map',
        '-nostartfiles',
        '--specs=nano.specs',
        '--specs=nosys.specs',
    ],
    language: ['c', 'cpp'],
    native: false
)

linker_script =  meson.get_external_property('linker_script', '')
if linker_script != ''
    add_project_link_arguments([
        '-Wl,-T,' + meson.project_source_root() / linker_script
    ],
    language: ['c', 'cpp'],
    native: false
)
endif

ti_driverlib_dep = declare_dependency(
    dependencies : meson.get_compiler('c', native : false).find_library(
        'driverlib',
        dirs : [join_paths(meson.current_source_dir(), 'mspm0-sdk/source/ti/driverlib/lib/gcc/m0p/mspm0c110x/')]
    ),
    include_directories : include_directories(['mspm0-sdk/source/', 'mspm0-sdk/source/third_party/CMSIS/Core/Include']),
)

project_include_dirs += include_directories('subprojects/TemplateMetaProgramming/include')

subdir('lib')
subdir('app')
