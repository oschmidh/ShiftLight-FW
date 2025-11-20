project(
    'shiftLight',
    'cpp',
    'c',
    meson_version : '>= 1.4.0',
    default_options: ['c_std=c11', 'cpp_std=c++23',
                            # 'b_lto=false',
                             'b_asneeded=false', 
                            #  'b_lundef=false',
                            #  'warning_level=3'
                             'warning_level=1'
                             ]
)

project_sources = []
project_include_dirs= [
 'C:/ti/mspm0_sdk_2_05_01_00/source',
 'C:/ti/mspm0_sdk_2_05_01_00/source/third_party/CMSIS/Core/Include',
#  'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/include',
#  'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/include/newlib-nano',
]

common_comp_args = [
     # '--target=arm-none-eabi',
    '-mcpu=cortex-m0plus',
    '-march=armv6-m',
    '-mthumb',
    '-mfloat-abi=soft',
    '-fno-exceptions',
    '-D__MSPM0C1104__',
    # '-fsigned-char',
    '-Og', # TODO define where?
    '-ffunction-sections',
    '-fdata-sections',
    # '-ffreestanding',
    '-g', # TODO ??
    '-gdwarf-3', # TODO ??
    '-gstrict-dwarf', # TODO ??
    '-Wall',
    '-Werror',
    # '--specs=nano.specs',
    # '-v',
]

cpp_args = [
   '-fno-rtti', '-fno-unwind-tables',    '-fno-threadsafe-statics', '-U_GLIBCXX_ASSERTIONS','-U_FILE_OFFSET_BITS',
]

c_link_args = [
    # '--target=arm-none-eabi',
    # '-v',
    # '-Wl,-z undefs',
    # '-Wl,-TW:/repos/ShiftLight/mspm0c1104.lds',#TODO get rid of abs path
    '-fno-exceptions',
    # '-fno-rtti',
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
    # '-fno-math-errno',
    # '--specs=nano.specs',
    '--specs=nosys.specs',
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
    c_link_args += ['-Wl,-T,'+linkfile.full_path()]
endforeach


subdir('src')

executable(
    'shiftLight',
    main_source,
    name_suffix : 'elf',
    c_args: [common_comp_args],
    cpp_args: [common_comp_args, cpp_args],
    link_args: [c_link_args],
    dependencies: [
        meson.get_compiler('c', native : false).find_library('gcc', dirs: [
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/arm-none-eabi/lib/thumb/v6-m/nofp',
                'C:/tools/Arm_GNU_Toolchain_arm-none-eabi/14.3 rel1/lib/gcc/arm-none-eabi/14.3.1/thumb/v6-m/nofp',
            ],),
        meson.get_compiler('c', native : false).find_library(
            # 'libc_nano',
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
        lib_lib,
    ],
    include_directories: project_include_dirs,
)
