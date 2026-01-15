doxygen = find_program('doxygen', required : false)
if doxygen.found()
  doxy_file = configure_file(
    configuration: {'PACKAGE': meson.project_name(), 'VERSION': meson.project_version(), 'TOPSRCDIR': meson.source_root(), 'DESTDIR': meson.build_root() + '/doc/docs'},
    input: 'Doxyfile.dist',
    output: 'Doxyfile'
    )
  run_command([doxygen, meson.build_root() + '/doc/Doxyfile'], check: false)
  install_subdir(meson.build_root() + '/doc/docs/html', install_dir: 'docs')
else
  warning('Documentation disabled without doxygen')
endif
