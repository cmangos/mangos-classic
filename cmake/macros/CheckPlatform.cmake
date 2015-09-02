# check what platform we're on (64-bit or 32-bit), and create a simpler test than CMAKE_SIZEOF_VOID_P
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM X64)
    # This definition is necessary to work around a bug with Intellisense described
    # here: http://tinyurl.com/2cb428.  Syntax highlighting is important for proper
    # debugger functionality.
    if(WIN32)
        add_definitions("-D_WIN64")
    endif()
    message(STATUS "Detected 64-bit platform")
else()
    set(PLATFORM X86)
    message(STATUS "Detected 32-bit platform")
endif()

if(UNIX)
  include("${CMAKE_SOURCE_DIR}/cmake/platform/unix/settings.cmake")
endif()
