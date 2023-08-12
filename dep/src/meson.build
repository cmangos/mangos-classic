subdir('gsoap')
if get_option('BUILD_EXTRACTORS')
  bzip2_dep = dependency('bzip2', required: false)
  if not bzip2_dep.found()
    subdir('bzip2')
  endif
endif