# This definition is necessary to work around a bug with Intellisense described
# here: http://tinyurl.com/2cb428.  Syntax highlighting is important for proper
# debugger functionality.
add_definitions("-D_WIN64")
message(STATUS "MSVC: 64-bit platform, enforced -D_WIN64 parameter")

add_library(cmangos-compile-option-interface INTERFACE)

# Enable extended object support for debug compiles on X64 (not required on X86)
target_compile_options(cmangos-compile-option-interface
  INTERFACE
    /bigobj)
message(STATUS "MSVC: Enabled increased number of sections in object files")

# multithreaded compiling on VS
target_compile_options(cmangos-compile-option-interface
  INTERFACE
    /MP)

# disable permissive mode to make msvc more eager to reject code that other compilers don't already accept
target_compile_options(cmangos-compile-option-interface
  INTERFACE
    /permissive-)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(cmangos-compile-option-interface
    INTERFACE
      /Zc:__cplusplus   # Enable updated __cplusplus macro value
      /Zc:preprocessor  # Enable preprocessor conformance mode
      /Zc:templateScope # Check template parameter shadowing
      /Zc:throwingNew)  # Assume operator new throws
endif()

# Define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES - eliminates the warning by changing the strcpy call to strcpy_s, which prevents buffer overruns
add_definitions(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
message(STATUS "MSVC: Overloaded standard names")

# Ignore warnings about older, less secure functions
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
message(STATUS "MSVC: Disabled NON-SECURE warnings")

# disable warnings in Visual Studio 8 and above if not wanted
if(NOT WARNINGS)
  if(NOT CMAKE_GENERATOR MATCHES "Visual Studio 7")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267")
    message(STATUS "MSVC: Disabled generic compiletime warnings")
  endif()
endif()

# Specify the maximum PreCompiled Header memory allocation limit
# Fixes a compiler-problem when using PCH - the /Ym flag is adjusted by the compiler in MSVC2012, hence we need to set an upper limit with /Zm to avoid discrepancies)
# (And yes, this is a verified , unresolved bug with MSVC... *sigh*)
string(REGEX REPLACE "/Zm[0-9]+ *" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm500" CACHE STRING "" FORCE)
