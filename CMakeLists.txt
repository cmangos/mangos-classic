#
# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

project(CMaNGOS)

cmake_minimum_required(VERSION 2.8.12)

include(cmake/common.cmake)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH ${LIBS_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# set default buildoptions and print them
include(cmake/options.cmake)

# Force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR
    "This project requires an out of source build. Remove the file 'CMakeCache.txt' found in this directory before continuing, create a separate build directory and run 'cmake [options] <srcs>' from there."
  )
endif()

# TODO: allow other compilers under windows in the future
if(WIN32 AND NOT MSVC)
  message(FATAL_ERROR
    "Under Windows other compiler than Microsoft Visual Studio are not supported."
  )
endif()

# TODO: remove this in the future! it has only been added to make the switch easier for end users
if(PREFIX)
  message(FATAL_ERROR "The parameter PREFIX has been removed. Please re-run CMake and use CMAKE_INSTALL_PREFIX instead to define your installation location!")
endif()

include(CheckPlatform)

# TODO: use MSVC_CXX_ARCHITECTURE_ID instead to identify platform on windows (not required on other systems)
# find platform: required to build 3rd party libraries w/o CMake files
# Find out what system we use to include the needed libs
if(WIN32)
  if(PLATFORM MATCHES X86) # 32-bit
    set(DEP_ARCH win32)
  else() # 64-bit
    set(DEP_ARCH x64)
  endif()
endif()

# find Git: used to get the revision number
find_package(Git)

# check if the platform supports precomiled headers
#find_package(PCHSupport)
include(cotire)

# TODO: remove this (refactor TBB CMake code)
# VS100 uses MSBuild.exe instead of devenv.com, so force it to use devenv.com
if(WIN32 AND MSVC_VERSION MATCHES 1600)
  find_package(VisualStudio2010)
endif()

# Override configuration-types - we don't use anything else than debug and release
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Release Debug)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
endif()

set(BIN_FOLDER_NAME bin)
set(CONF_FOLDER_NAME etc)
set(LIBS_FOLDER_NAME lib)

set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/${BIN_FOLDER_NAME})
set(CONF_DIR ${CMAKE_INSTALL_PREFIX}/${CONF_FOLDER_NAME})
# If win32 put it in the bin dir not lib
if(WIN32)
  set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/${BIN_FOLDER_NAME})
else()
  set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/${LIBS_FOLDER_NAME})
endif()

# For Unix systems set the rpath so that libraries are found
set(CMAKE_INSTALL_RPATH ../${LIBS_FOLDER_NAME})
set(CMAKE_INSTALL_NAME_DIR ${LIBS_DIR})

# Find needed packages and if necessery abort if something important is missing
find_package(Boost REQUIRED COMPONENTS system program_options thread)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "This project requires boost.  Please install from http://www.boost.org")
endif()

if(NOT USE_STD_MALLOC)
  unset(TBB_INCLUDE_DIR CACHE)
  unset(TBB_LIBRARIES CACHE)
  unset(TBB_LIBRARIES_DIR CACHE)
  unset(TBB_INCLUDE_DIR)
  unset(TBB_LIBRARIES)
  unset(TBB_LIBRARIES_DIR)
  if(TBB_USE_EXTERNAL)
    find_package(TBB)
    if(NOT TBB_FOUND)
      message(FATAL_ERROR
        "This project requires TBB installed when TBB_USE_EXTERNAL is set. Please download the TBB Stable Release from http://www.threadingbuildingblocks.org/ and install it. If this script didn't find TBB and it was correctly installed please set TBB_ROOT to the correct path."
      )
    endif()
  else()
    include(cmake/macros/ImportTBB.cmake)
  endif()
endif()

# Win32 delivered packages
if(WIN32)
  set(MYSQL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/include/mysql)
  set(MYSQL_LIBRARY ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libmySQL.lib)
  set(MYSQL_DEBUG_LIBRARY ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libmySQL.lib)
  set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/include/openssl)
  set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libeay32.lib)
  set(OPENSSL_DEBUG_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libeay32.lib)
  # zlib is build
endif()

# *nix-specific packages
if(UNIX)
  if(POSTGRESQL)
    find_package(PostgreSQL REQUIRED)

    if(POSTGRESQL_FOUND)
      include_directories(${POSTGRESQL_INCLUDE_DIRS})
    endif(POSTGRESQL_FOUND)

  else()
    find_package(MySQL REQUIRED)
  endif()

  find_package(OpenSSL REQUIRED)
  find_package(ZLIB REQUIRED)
endif()

# Find core revision
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_REVISION
    RESULT_VARIABLE GIT_RESULT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(GIT_RESULT)
    set(GIT_REVISION "Git repository not found")
  endif()
else()
  set(GIT_REVISION "Git not found")
endif()

if(DEFINED INCLUDE_BINDINGS_DIR AND INCLUDE_BINDINGS_DIR)
  # check if the directory exists
  if(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/src/bindings/${INCLUDE_BINDINGS_DIR})
    message(FATAL_ERROR "Could not find the script library which was supposed to be: " ${CMAKE_SOURCE_DIR}/src/bindings/${INCLUDE_BINDINGS_DIR})
  endif()
  # check if it really contains a CMakeLists.txt
  if(NOT EXISTS ${CMAKE_SOURCE_DIR}/src/bindings/${INCLUDE_BINDINGS_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "The script library does not contain a CMakeLists.txt!")
  endif()
else()
endif()

#if(PCH AND NOT PCHSupport_FOUND)
#  set(PCH 0 CACHE BOOL
#    "Use precompiled headers"
#    FORCE)
#  message(WARNING "No PCH for your system possible but PCH was set to 1. Resetting it."
#  )
#endif()

set(DEFINITIONS_RELEASE NDEBUG)
set(DEFINITIONS_DEBUG _DEBUG MANGOS_DEBUG)

if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

# print out the results before continuing
include(cmake/showoptions.cmake)

# Generate revision-extractor
set(GENREV_SRC
  src/tools/genrevision/genrevision.cpp
)

add_executable(genrev
  ${GENREV_SRC}
)

add_custom_target("revision.h" ALL
  COMMAND genrev ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  DEPENDS genrev
)

if(WIN32)
  install(
    FILES
      ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libeay32.dll
      ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libmySQL.dll
    DESTINATION ${LIBS_DIR}
    CONFIGURATIONS Release
  )
  install(
    FILES
      ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libeay32.dll
      ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libmySQL.dll
    DESTINATION ${LIBS_DIR}
    CONFIGURATIONS Debug
  )
  if(PLATFORM MATCHES X86)
    # Copy dll's Windows needs
    install(
      FILES
        ${CMAKE_SOURCE_DIR}/dep/lib/win32_release/dbghelp.dll
      DESTINATION ${LIBS_DIR}
      CONFIGURATIONS Release
    )
    install(
      FILES
        ${CMAKE_SOURCE_DIR}/dep/lib/win32_debug/dbghelp.dll
      DESTINATION ${LIBS_DIR}
      CONFIGURATIONS Debug
    )
  endif()
endif()

if(XCODE)
  if(PLATFORM MATCHES X86)
    set(CMAKE_OSX_ARCHITECTURES i386)
  else()
    set(CMAKE_OSX_ARCHITECTURES x86_64)
  endif()
endif()

add_subdirectory(dep) # TODO: add vmap extractor build support

# Add definitions for all build types
# Don't place this above 'dep' subdirectory! Because of defines build will crash.
set(DEFINITIONS
  SYSCONFDIR="../${CONF_FOLDER_NAME}/"
)

if(POSTGRESQL)
  set(DEFINITIONS ${DEFINITIONS} DO_POSTGRESQL)
else()
  set(DEFINITIONS ${DEFINITIONS} DO_MYSQL)
endif()

if(WIN32)
  set(DEFINITIONS ${DEFINITIONS} WIN32 _WIN32)
endif()

if(USE_STD_MALLOC)
  set(DEFINITIONS ${DEFINITIONS} USE_STANDARD_MALLOC)
endif()

if(DEBUG)
  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS};${DEFINITIONS_DEBUG}")
else()
  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS};${DEFINITIONS_RELEASE}")
endif()

add_subdirectory(src)
# if(SQL)
#   add_subdirectory(sql)
# endif()
