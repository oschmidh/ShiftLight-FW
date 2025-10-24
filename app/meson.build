app_include_dirs = include_directories('include')

app_sources = files(
    'src/main.cpp',
    'src/ti_msp_dl_config.c',
    'src/startup.cpp'
)

project_include_dirs += app_include_dirs
project_sources += app_sources

executable(
    meson.project_name(),
    project_sources,
    name_suffix : 'elf',
    include_directories: project_include_dirs,
    dependencies: [
        ti_driverlib_dep,
        shiftlight_lib,
    ],
    native:false
)
