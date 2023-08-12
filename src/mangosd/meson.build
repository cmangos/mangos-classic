mangosd_sources = [
    'Master.cpp',
    'MaNGOSsoap.cpp',
    'soapC.cpp',
    'WorldRunnable.cpp',
    'Main.cpp',
    'CliRunnable.cpp',
    'RASocket.cpp',
    'soapServer.cpp'
]

mangosd_inc_dirs = []

if utfcpp_dep.found() == false
  mangosd_inc_dirs = [mangosd_inc_dirs, '../../dep/include/utf8cpp']
endif

if build_machine.system() == 'windows'
  mangosd_inc_dirs = [mangosd_inc_dirs, '../../dep/lib/include', '../../dep/lib/include/mysql']
endif

mangosd_inc = include_directories(mangosd_inc_dirs)

if get_option('BUILD_GAME_SERVER')
  executable('mangosd',
              mangosd_sources,
              link_with: [sharedlib, gamelib, gsoaplib],
              include_directories: [mangosd_inc, global_includes],
              dependencies: [boost_dep, dblib_dep, zlib_dep, openssl_dep, utfcpp_dep],
              link_args: _linker_args,
              install: true
  )
  install_data('mangosd.conf.dist.in', rename: 'mangosd.conf.dist', install_dir: 'etc')
endif