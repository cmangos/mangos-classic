vmap_extractor_args = ['-DIOMAP_DEBUG', '-DUSE_LIBMPQ04']

if build_machine.system() == 'linux'
  vmap_extractor_args = [vmap_extractor_args, '-ggdb']
endif

subdir('vmapextract')