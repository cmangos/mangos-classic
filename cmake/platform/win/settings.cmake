if ( MSVC )
  include("${ROOT_DIR}/cmake/compiler/msvc/settings.cmake")
elseif ( MINGW )
  include("${ROOT_DIR}/cmake/compiler/gcc/settings.cmake")
  # On windows we generally want our binaries to be static
  # Override default linker flags and we are good to go
  set(CMAKE_EXE_LINKER_FLAGS "-static")
  # remove -Wl,-Bdynamic
  set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
  set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
  # remove -fPIC
  set(CMAKE_SHARED_LIBRARY_C_FLAGS)
  set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
  # remove -rdynamic
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
endif()
