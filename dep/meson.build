if get_option('BUILD_GAME_SERVER') or get_option('BUILD_LOGIN_SERVER') or get_option('BUILD_EXTRACTORS')
  subdir('recastnavigation')
  subdir('src')
  subdir('g3dlite')
  if get_option('BUILD_EXTRACTORS')
    subdir('libmpq')
  endif
endif