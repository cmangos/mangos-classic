# output generic information about the core
message(STATUS "CMaNGOS-Core revision : ${GIT_REVISION}")
message(STATUS "Install server to     : ${CMAKE_INSTALL_PREFIX}")

# Show infomation about the options selected during configuration

# if(CLI)
#   message(STATUS "Build with CLI        : Yes (default)")
#   add_definitions(-DENABLE_CLI)
# else()
#   message(STATUS "Build with CLI        : No")
# endif()

# if(RA)
#   message(STATUS "* Build with RA       : Yes")
#   add_definitions(-DENABLE_RA)
# else(RA)
#   message(STATUS "* Build with RA       : No  (default)")
# endif(RA)

if(PCH)
  message(STATUS "Use PCH               : Yes")
else()
  message(STATUS "Use PCH               : No  (default)")
endif()

if(DEBUG)
  message(STATUS "Build in debug-mode   : Yes")
else()
  message(STATUS "Build in debug-mode   : No  (default)")
endif()

if(BUILD_CORE)
  message(STATUS "Build core            : Yes (default)")
else()
  message(STATUS "Build core            : No")
endif()

if(BUILD_EXTRACTOR)
  message(STATUS "Build extractor       : Yes")
else()
  message(STATUS "Build extractor       : No  (default)")
endif()

if(BUILD_VMAP_EXTRACTOR)
  message(STATUS "Build VMAP extractor  : Yes")
else()
  message(STATUS "Build VMAP extractor  : No  (default)")
endif()

if(BUILD_MMAP_EXTRACTOR)
  message(STATUS "Build MMAP extractor  : Yes")
else()
  message(STATUS "Build MMAP extractor  : No  (default)")
endif()

# if(SQL)
#   message(STATUS "Install SQL-files     : Yes")
# else()
#   message(STATUS "Install SQL-files     : No  (default)")
# endif()

# if(TOOLS)
#   message(STATUS "Build map/vmap tools  : Yes")
# else()
#   message(STATUS "Build map/vmap tools  : No  (default)")
# endif()

message("")
