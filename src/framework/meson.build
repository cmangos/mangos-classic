framework_sources = [
    'Policies/ObjectLifeTime.cpp',
    'Utilities/EventProcessor.cpp'
]

if get_option('BUILD_GAME_SERVER') or get_option('BUILD_LOGIN_SERVER') or get_option('BUILD_EXTRACTORS')
frameworklib = static_library('framework',
                        framework_sources,
                        include_directories: global_includes,
                        dependencies: [boost_dep, dblib_dep, zlib_dep, openssl_dep, utfcpp_dep],
                        install : false
)
endif