vmap_extractor_sources = [
    'adtfile.cpp',
    'dbcfile.cpp',
    'gameobject_extract.cpp',
    'model.cpp',
    'mpq_libmpq.cpp',
    'vmapexport.cpp',
    'wdtfile.cpp',
    'wmo.cpp'
]

executable('vmap_extractor',
            vmap_extractor_sources,
            include_directories: global_includes,
            link_with: [g3dlib, lib_libmpq, detourlib, recastlib],
            link_args: _linker_args,
            install_dir: 'bin/tools',
            install: true
)
install_data(extractor_script_file, install_dir: 'bin/tools')