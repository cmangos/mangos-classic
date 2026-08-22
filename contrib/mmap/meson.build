movemapgen_args = ''
movemapgen_args = [movemapgen_args, '-DMMAP_GENERATOR', '-DNO_CORE_FUNCS', '-DNO_vsnprintf', '-DDT_POLYREF64']
movemapgen_sources = [
  'src/generator.cpp',
  'src/IntermediateValues.cpp',
  'src/MapBuilder.cpp',
  'src/TerrainBuilder.cpp',
  'src/VMapExtensions.cpp'
]

if build_machine.system() == 'linux'
  movemapgen_args = [movemapgen_args, '-ggdb']
endif

vmaplib = static_library('vmaplib',
                        [
                          '../../src/game/vmap/BIH.cpp',
                          '../../src/game/vmap/VMapManager2.cpp',
                          '../../src/game/vmap/MapTree.cpp',
                          '../../src/game/vmap/TileAssembler.cpp',
                          '../../src/game/vmap/WorldModel.cpp',
                          '../../src/game/vmap/ModelInstance.cpp'
                        ],
                        include_directories: global_includes,
                        dependencies: [zlib_dep, boost_dep],
                        cpp_args: movemapgen_args,
                        link_with: [g3dlib, detourlib, recastlib, frameworklib, sharedlib],
                        install : false
)

mmaplib = static_library('mmaplib',
                         ['src/generator.cpp', 'src/IntermediateValues.cpp', 'src/MapBuilder.cpp', 'src/TerrainBuilder.cpp', 'src/VMapExtensions.cpp'],
                         include_directories: global_includes,
                         dependencies: [zlib_dep, boost_dep],
                         cpp_args: movemapgen_args,
                         link_with: [vmaplib, detourlib, recastlib, g3dlib, frameworklib],
                         install: false
)

if get_option('BUILD_EXTRACTORS')
  if build_machine.system() == 'linux'
    local_linker = [_linker_args, '-rdynamic']
  endif
  executable('MoveMapGen',
              movemapgen_sources,
              link_with: [vmaplib, detourlib, recastlib, mmaplib],
              include_directories: global_includes,
              dependencies: [boost_dep, zlib_dep],
              cpp_args: movemapgen_args,
              link_args: _linker_args,
              install_dir: 'bin/tools',
              install: true
  )
  install_data(movemapgen_script_file, install_dir: 'bin/tools')
  install_data(offmesh_file, install_dir: 'bin/tools')
  install_data(config_json_file, install_dir: 'bin/tools')
endif

