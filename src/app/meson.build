app_include_dirs = include_directories('include')

app_sources = files(
    'main.cpp', 
    'ti_msp_dl_config.c', 
    'startup_mspm0c110x_gcc.c'
)

app_link_args = [
    '-Wl,-Map=' + meson.project_name() + '.map',
    '-nostartfiles',
]

linker_script_flags = []

linker_script =  meson.get_external_property('linker_script', '')
if linker_script != ''
	linker_script_flags += '-Wl,-T,' + meson.project_source_root() / linker_script
endif

executable(
    meson.project_name(),
    app_sources,
    name_suffix : 'elf',
    include_directories: app_include_dirs,
    dependencies: [
        ti_driverlib_dep,
        lib_lib,
    ],
    link_args: [
        app_link_args,
        linker_script_flags,
    ],
)
