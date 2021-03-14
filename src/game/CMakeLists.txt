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
#

set(LIBRARY_NAME game)

add_definitions(-DDT_POLYREF64)
add_definitions(-D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)

# Find all the input files
FILE(GLOB_RECURSE LIBRARY_SRCS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h" "*.cpp")

if(NOT BUILD_SCRIPTDEV)
  # exclude script folder for faster build time (used for dev mode)
  set (EXCLUDE_DIR "AI/ScriptDevAI/scripts/")
  foreach (TMP_PATH ${LIBRARY_SRCS})
      string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
      if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
          list(REMOVE_ITEM LIBRARY_SRCS ${TMP_PATH})
      endif ()
  endforeach()
endif()

if(NOT BUILD_PLAYERBOT)
  # exclude Playerbot folder
  set (EXCLUDE_DIR "PlayerBot/")
  foreach (TMP_PATH ${LIBRARY_SRCS})
      string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
      if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
          list(REMOVE_ITEM LIBRARY_SRCS ${TMP_PATH})
      endif ()
  endforeach()
endif()

set(PCH_BASE_FILENAME "pchdef")
# exclude pchdef files
set (EXCLUDE_FILE "${PCH_BASE_FILENAME}")
foreach (TMP_PATH ${LIBRARY_SRCS})
    string (FIND ${TMP_PATH} ${EXCLUDE_FILE} EXCLUDE_FILE_FOUND)
    if (NOT ${EXCLUDE_FILE_FOUND} EQUAL -1)
        list(REMOVE_ITEM LIBRARY_SRCS ${TMP_PATH})
    endif ()
endforeach()

# set pch file names
set(PCH_FILE_HEADER "${PCH_BASE_FILENAME}.h")
set(PCH_FILE_SOURCE "${PCH_BASE_FILENAME}.cpp")

# Create visual studio groups using folder structure
foreach(source ${LIBRARY_SRCS})
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

target_link_libraries(${LIBRARY_NAME}
  PRIVATE shared
  PRIVATE g3dlite
  PRIVATE detour
  PRIVATE ${ZLIB_LIBRARIES}
)

# include additionals headers
set(ADDITIONAL_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vmap
  ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouseBot
  ${CMAKE_CURRENT_SOURCE_DIR}/BattleGround
  ${CMAKE_CURRENT_SOURCE_DIR}/OutdoorPvP
  ${CMAKE_CURRENT_SOURCE_DIR}/PlayerBot
  ${CMAKE_BINARY_DIR}
)

target_include_directories(${LIBRARY_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${ADDITIONAL_INCLUDE_DIRS}
  PRIVATE ${Boost_INCLUDE_DIRS}
)

if(UNIX)
  # Both systems don't have libdl and don't need them
  if (NOT (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD"))
    target_link_libraries(${LIBRARY_NAME} PUBLIC dl)
  endif()
endif()

# Define BUILD_SCRIPTDEV if need
if (BUILD_SCRIPTDEV)
  add_definitions(-DBUILD_SCRIPTDEV)
endif()

# Define BUILD_AHBOT if need
if (BUILD_AHBOT)
  add_definitions(-DBUILD_AHBOT)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouseBot/ahbot.conf.dist.in DESTINATION ${CONF_DIR} RENAME ahbot.conf.dist)
endif()

# Define BUILD_METRICS if need
if (BUILD_METRICS)
  add_definitions(-DBUILD_METRICS)
endif()

# Define BUILD_PLAYERBOT if need
if (BUILD_PLAYERBOT)
  add_definitions(-DBUILD_PLAYERBOT)
endif()

if (MSVC)
  set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "Game")
endif()

# Generate precompiled header
if(PCH)
  target_precompile_headers(${LIBRARY_NAME} PRIVATE "${PCH_FILE_HEADER}")
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/PlayerBot/playerbot.conf.dist.in DESTINATION ${CONF_DIR} RENAME playerbot.conf.dist)
