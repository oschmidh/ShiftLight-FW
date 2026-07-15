app_include_dirs = include_directories('include')

app_sources = files(
    'src/Devices.cpp',
    'src/main.cpp',
    'src/Mspm0c110x.cpp',
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
        shiftlight_lib,
    ],
    native:false
)
