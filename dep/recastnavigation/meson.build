if get_option('BUILD_GAME_SERVER') or get_option('BUILD_EXTRACTORS') or get_option('BUILD_RECASTDEMOMOD')
  recastnav_args = '-DDT_POLYREF64'
  subdir('Detour')
endif

if get_option('BUILD_EXTRACTORS') or get_option('BUILD_RECASTDEMOMOD')
  subdir('Recast')
endif

if get_option('BUILD_RECASTDEMOMOD')
  subdir('DetourCrowd')
  subdir('DetourTileCache')
  subdir('DebugUtils')
endif