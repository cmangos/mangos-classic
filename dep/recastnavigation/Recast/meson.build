recast_sources = [
  'Source/Recast.cpp',
  'Source/RecastAlloc.cpp',
  'Source/RecastArea.cpp',
  'Source/RecastAssert.cpp',
  'Source/RecastContour.cpp',
  'Source/RecastFilter.cpp',
  'Source/RecastLayers.cpp',
  'Source/RecastMesh.cpp',
  'Source/RecastMeshDetail.cpp',
  'Source/RecastRasterization.cpp',
  'Source/RecastRegion.cpp',
  'Include/Recast.h',
  'Include/RecastAlloc.h',
  'Include/RecastAssert.h'
]

recastlib = static_library('recast',
                        recast_sources,
                        include_directories: include_directories('Include'),
                        cpp_args: recastnav_args,
                        install : false
)