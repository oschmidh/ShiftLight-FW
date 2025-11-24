project(
    'shiftlight',
    'cpp',
    'c',
    meson_version : '>= 1.4.0',
    default_options: [
        'c_std=c11', 
        'cpp_std=c++23',
        # 'b_lto=false',
        'b_asneeded=false', 
        # 'b_lundef=false',
        # 'warning_level=3'
        'warning_level=1',
        'debug=true',
        'werror=true',
    ]
)

project_sources = []
project_include_dirs = []

add_project_arguments(['-Og',], language: ['c', 'cpp'])
add_project_arguments(['-fno-rtti', '-fno-unwind-tables', '-fno-threadsafe-statics',], language: 'cpp')


add_project_link_arguments([
    '-fno-exceptions',
    '-fno-unwind-tables',
    '-Wl,-Map=' + meson.project_name() + '.map',
    '-nostartfiles',
    '-nostdlib',
    # '-nodefaultlibs',
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
        meson.get_compiler('c', native : false).find_library(
            'gcc', 
            dirs: [
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/lib/gcc/arm-none-eabi/14.3.1/thumb/v6-m/nofp',
            ],),
        meson.get_compiler('c', native : false).find_library(
            'libc',
            dirs: [
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/lib/gcc/arm-none-eabi/14.3.1/thumb/v6-m/nofp',
            ],
        ),
        meson.get_compiler('c', native : false).find_library(
            'libm',
            dirs: [
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/lib/gcc/arm-none-eabi/14.3.1/thumb/v6-m/nofp',
            ],
        ),
        ti_driverlib_dep,
        lib_lib,
    ],
    include_directories: project_include_dirs,
)
