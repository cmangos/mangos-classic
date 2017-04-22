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

project(CMaNGOS_Classic)

# Define here name of the binaries
set(CMANGOS_BINARY_SERVER_NAME "mangosd")
set(CMANGOS_BINARY_REALMD_NAME "realmd")

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

include(CheckPlatform)

# TODO: use MSVC_CXX_ARCHITECTURE_ID instead to identify platform on windows (not required on other systems)
# find platform: required to build 3rd party libraries w/o CMake files
# Find out what system we use to include the needed libs
if(WIN32)
  if(PLATFORM MATCHES X86) # 32-bit
    set(DEP_ARCH Win32)
  else() # 64-bit
    set(DEP_ARCH x64)
  endif()
endif()

# Override configuration-types - we don't use anything else than debug and release
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Reset the configurations to what we need" FORCE)
else()
  if(NOT CMAKE_BUILD_TYPE)
    if (NOT DEBUG)
      message("Defaulting to release build.")
      set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
    else()
      set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    endif()
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
  # set the valid options for cmake-gui drop-down list
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
endif()

# find Git: used to get the revision number
find_package(Git)

# Include cotire to manage PCH support
include(cotire)

set(BIN_FOLDER_NAME bin)
set(CONF_FOLDER_NAME etc)
set(LIBS_FOLDER_NAME lib)

# Set Install folders
set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/${BIN_FOLDER_NAME})
set(CONF_DIR ${CMAKE_INSTALL_PREFIX}/${CONF_FOLDER_NAME})
set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/${LIBS_FOLDER_NAME})

# On windows define more developpement folders and install provided libs
if(WIN32)
  set(DEV_BIN_DIR ${CMAKE_SOURCE_DIR}/${BIN_FOLDER_NAME}/_${DEP_ARCH}_$(Configuration))
  set(DEV_PROVIDED_LIBS_FOLDER ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_$(Configuration))

  # Define install rules for provided libs
  install(
    FILES
      ${PROVIDED_LIBS_FOLDER}/libeay32.dll
      ${PROVIDED_LIBS_FOLDER}/libmySQL.dll
    DESTINATION ${LIBS_DIR}
  )

  # TODO:: check why x64 is not provided
  if(PLATFORM MATCHES X86)
    install(
      FILES
        ${PROVIDED_LIBS_FOLDER}/dbghelp.dll
      DESTINATION ${LIBS_DIR}
    )
  endif()
endif()

# For Unix systems set the rpath so that libraries are found
set(CMAKE_INSTALL_RPATH ../${LIBS_FOLDER_NAME})
set(CMAKE_INSTALL_NAME_DIR ${LIBS_DIR})

# Add alternate lib dir for boost on windows
if(WIN32 AND NOT "$ENV{BOOST_ROOT}" STREQUAL "")
  set(BOOST_LIBRARYDIR "$ENV{BOOST_ROOT}\\lib\\${DEP_ARCH}")
  set(BOOST_LIBRARYDIR "${BOOST_LIBRARYDIR};$ENV{BOOST_ROOT}\\stage\\${DEP_ARCH}")
  set(BOOST_LIBRARYDIR "${BOOST_LIBRARYDIR};$ENV{BOOST_ROOT}\\stage")
endif()

# Be sure to search only static lib
add_definitions(-DBOOST_ALL_NO_LIB)
set(Boost_USE_STATIC_LIBS ON)

# Find needed packages and if necessery abort if something important is missing
find_package(Boost REQUIRED COMPONENTS system program_options thread regex)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries(${Boost_LIBRARIES})
else()
  message(STATUS "BOOST_ROOT = $ENV{BOOST_ROOT}")
  message(STATUS "BOOST_LIBRARYDIR = $ENV{BOOST_LIBRARYDIR}")
  if ("$ENV{BOOST_ROOT}" STREQUAL "")
    message(WARNING "BOOST_ROOT is not set, please set it correctly to your boost folder.")
  else()
    if (BOOST_LIBRARYDIR_FORCED STREQUAL "TRUE")
        message(STATUS "Please put all ${DEP_ARCH} boost lib files in $ENV{BOOST_ROOT}\\lib\\${DEP_ARCH} folder")
        message(STATUS "Or set BOOST_LIBRARYDIR to your boost lib folder.")
    else()
        message(STATUS "BOOST_LIBRARYDIR is set, please verify if correct libs files are in that folder.")
    endif()
  endif()
  message(FATAL_ERROR "This project requires boost.  Please install from http://www.boost.org")
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

  # If OpenSSL path isn't specified on mac we set the one that homebrew uses
  # since that's what most people will be using.
  if (APPLE)
    if (NOT OPENSSL_ROOT_DIR)
      set(OPENSSL_ROOT_DIR /usr/local/opt/openssl/)
    endif()
    if (NOT OPENSSL_INCLUDE_DIR)
      set(OPENSSL_INCLUDE_DIR /usr/local/opt/openssl/include)
    endif()
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

set(DEFINITIONS_RELEASE NDEBUG)
set(DEFINITIONS_DEBUG _DEBUG MANGOS_DEBUG)

# print out the results before continuing
include(cmake/showoptions.cmake)

if(NOT BUILD_CORE AND NOT BUILD_EXTRACTOR AND NOT BUILD_VMAP_EXTRACTOR AND NOT BUILD_MMAP_EXTRACTOR)
  message(FATAL_ERROR "You must select something to build!")
endif()

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

if(XCODE)
  if(PLATFORM MATCHES X86)
    set(CMAKE_OSX_ARCHITECTURES i386)
  else()
    set(CMAKE_OSX_ARCHITECTURES x86_64)
  endif()
endif()

add_subdirectory(dep)

# Add definitions for all build types
# Don't place this above 'dep' subdirectory! Because of defines build will crash.
set(DEFINITIONS "")

if(WIN32)
  set(DEFINITIONS ${DEFINITIONS} WIN32 _WIN32)
else()
  set(DEFINITIONS SYSCONFDIR="../${CONF_FOLDER_NAME}/")
endif()

if(POSTGRESQL)
  set(DEFINITIONS ${DEFINITIONS} DO_POSTGRESQL)
else()
  set(DEFINITIONS ${DEFINITIONS} DO_MYSQL)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS};${DEFINITIONS_DEBUG}")
else()
  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS};${DEFINITIONS_RELEASE}")
endif()

if(BUILD_CORE)
  add_subdirectory(src)
  if(WIN32)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMANGOS_BINARY_SERVER_NAME})
  endif()
endif()

if(BUILD_EXTRACTOR)
  add_subdirectory(contrib/extractor)
endif()

if(BUILD_VMAP_EXTRACTOR)
  add_subdirectory(contrib/vmap_extractor)
  add_subdirectory(contrib/vmap_assembler)
endif()

if(BUILD_MMAP_EXTRACTOR)
  add_subdirectory(contrib/mmap)
endif()

# if(SQL)
#   add_subdirectory(sql)
# endif()
