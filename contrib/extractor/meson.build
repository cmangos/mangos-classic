ad_sources = [
  'loadlib/loadlib.cpp',
  'loadlib/adt.cpp',
  'loadlib/wdt.cpp',
  'dbcfile.cpp',
  'mpq_libmpq.cpp',
  'System.cpp',
]

executable('ad',
            ad_sources,
            link_with: [lib_libmpq],
            include_directories: global_includes,
            dependencies: [boost_dep, zlib_dep],
            link_args: _linker_args,
            install_dir: 'bin/tools',
            install: true
)
