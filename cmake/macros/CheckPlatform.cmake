# check what platform we're on (64-bit or 32-bit), and create a simpler test than CMAKE_SIZEOF_VOID_P
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM X64)
    message(STATUS "Detected 64-bit platform")
else()
    set(PLATFORM X86)
    message(STATUS "Detected 32-bit platform")
endif()

if(WIN32)
  include("${ROOT_DIR}/cmake/platform/win/settings.cmake")
elseif(UNIX)
  include("${ROOT_DIR}/cmake/platform/unix/settings.cmake")
endif()
