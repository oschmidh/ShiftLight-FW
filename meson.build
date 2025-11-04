project(
    'shiftLight',
    'cpp',
    'c',
    # default_options: ['c_std=c11', 'cpp_std=c++20'],
)

c_args = [
    # '--target=arm-none-eabi',
    '-mcpu=cortex-m0plus',
    '-march=armv6-m',
    '-mthumb',
    '-mfloat-abi=soft',
    '-fno-exceptions',
    '-D__MSPM0C1104__',
    # '-fsigned-char',
    '-ffunction-sections',
    '-fdata-sections',
    '-Wall',
    '-Werror',
    # '-ffreestanding',
    '-g', # TODO ??
    '-gdwarf-3', # TODO ??
    '-gstrict-dwarf', # TODO ??
    '-Og', # TODO define where?
    # '-v',
]

c_link_args = [
    # '--target=arm-none-eabi',
    # '-v',
    # '-Tmspm0c1104.lds',
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-unwind-tables',
    '-D__MSPM0C1104__',
    '-Og',
    '-ffunction-sections',
    '-fdata-sections',
    '-g',
    '-gdwarf-3',
    '-gstrict-dwarf',
    '-Wall',
    '-Werror',
    '-mthumb',
    '-mfloat-abi=soft',
    '-Wl,-Map=shiftLight.map', #TODO use proj name
    '-nostartfiles',
    '-nostdlib',
    # '-nodefaultlibs',
    '-static',
    '-Wl,--gc-sections',
    '-march=armv6-m',
    '-mthumb',
    '--specs=nosys.specs',
    # '--specs=nano.specs',
    # '-LC:/ti/mspm0_sdk_2_05_01_00/source/ti/driverlib/lib/gcc/m0p/mspm0c110x',
    # '-LC:/ti/mspm0_sdk_2_05_01_00/source',
    # '-L"C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp"', 
    'C:/ti/mspm0_sdk_2_05_01_00/source/ti/driverlib/lib/gcc/m0p/mspm0c110x/driverlib.a',
    # 'C:/ti/mspm0_sdk_2_05_01_00/source/ti/driverlib/lib/gcc/m0p/mspm0c110x/driverlib.a',
    # '-l:driverlib.a',
    # '-lgcc',
    # '-lc',
    # '-lm',
]

# [default_options]

linkfiles = files('mspm0c1104.lds')
# startupfile = files('startup_mspm0c110x_gcc.c')

foreach linkfile : linkfiles
    c_link_args += ['-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), linkfile)]
endforeach

# meson.get_compiler('c').find_library(
#     'm',
#     dirs: [
#         'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
#     ],
# ),

executable(
    'shiftLight.out',
    ['main.cpp', 'ti_msp_dl_config.c', 'startup_mspm0c110x_gcc.c'],
    c_args: [c_args, '-std=c11'],
    cpp_args: [c_args, '-std=c++23', '-fno-rtti', '-fno-unwind-tables'],
    link_args: [c_link_args],
    dependencies: [
        meson.get_compiler('c').find_library('gcc'),
        meson.get_compiler('c').find_library(
            'libc',
            dirs: [
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
            ],
        ),
    ],
    include_directories: [
        # TODO replace abs paths
        'C:/ti/mspm0_sdk_2_05_01_00/source',
        'C:/ti/mspm0_sdk_2_05_01_00/source/third_party/CMSIS/Core/Include',
        'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/include',
        'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/include/newlib-nano',
    ],
)