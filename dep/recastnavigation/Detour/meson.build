detour_sources = [
    'Source/DetourAlloc.cpp',
    'Source/DetourAssert.cpp',
    'Source/DetourCommon.cpp',
    'Source/DetourNavMesh.cpp',
    'Source/DetourNavMeshBuilder.cpp',
    'Source/DetourNavMeshQuery.cpp',
    'Source/DetourNode.cpp',
    'Include/DetourAlloc.h',
    'Include/DetourAssert.h',
    'Include/DetourCommon.h',
    'Include/DetourMath.h',
    'Include/DetourNavMesh.h',
    'Include/DetourNavMeshBuilder.h',
    'Include/DetourNavMeshQuery.h',
    'Include/DetourNode.h',
    'Include/DetourStatus.h'
]

detour_inc = include_directories('Include')

detourlib = static_library('Detour',
                        detour_sources,
                        include_directories: detour_inc,
                        cpp_args: recastnav_args,
                        install : false)