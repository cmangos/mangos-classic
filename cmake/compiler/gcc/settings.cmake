# Set build-directive (used in core to tell which buildtype we used)
add_definitions(-D_BUILD_DIRECTIVE='"${CMAKE_BUILD_TYPE}"')

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(ARM_FLAGS "-latomic")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARM_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_FLAGS}")
endif()

# Additional compaitibility checks and flags for commonly found LTS GCC versions
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0 AND NOT MINGW)
  # Set minimum C++17 compliant GCC version target to 7.0
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    message(SEND_ERROR "GCC: This project requires GCC version 7.0 or higher")
  endif()
endif()

add_definitions(-DHAVE_SSE2)
message(STATUS "GCC: SFMT enabled, SSE2 flags forced")

if(WARNINGS)
  set(WARNING_FLAGS "-W -Wall -Wextra -Winit-self -Winvalid-pch -Wfatal-errors")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -Woverloaded-virtual")
  message(STATUS "GCC: All warnings enabled")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-result")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Wno-psabi")
endif()

if(DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
  message(STATUS "GCC: Debug-flags set (-g3)")
endif()
