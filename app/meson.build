app_include_dirs = include_directories('include')

app_sources = files(
    'main.cpp', 
    'ti_msp_dl_config.c', 
    'startup_mspm0c110x_gcc.c'
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
        lib_lib,
    ],
    native:false
)
