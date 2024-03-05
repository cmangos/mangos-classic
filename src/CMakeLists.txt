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

if(BUILD_GAME_SERVER OR BUILD_LOGIN_SERVER OR BUILD_EXTRACTORS)
  add_subdirectory(framework)
  add_subdirectory(shared)
endif()

# Playerbots module
if(BUILD_PLAYERBOTS)
  include(FetchContent)

  FetchContent_Declare(
    playerbots
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/playerbots"
    GIT_REPOSITORY "https://github.com/cmangos/playerbots.git"
    GIT_TAG "master"
  )

  FetchContent_GetProperties(playerbots)
  if (NOT playerbots_POPULATED)
    FetchContent_Populate(playerbots)
    message(STATUS "Playerbots module source dir: ${playerbots_SOURCE_DIR}")
  else()
    message(STATUS "Playerbots module already populated: ${playerbots_POPULATED}")
  endif()

  #target_include_directories(Bots PUBLIC ${playerbots_SOURCE_DIR})
  message(STATUS "Playerbots module source dir: ${playerbots_SOURCE_DIR}")
  message(STATUS "Playerbots module binary dir: ${playerbots_BINARY_DIR}")
  message(STATUS "Playerbots module populated: ${playerbots_POPULATED}")

  add_subdirectory(${playerbots_SOURCE_DIR})

else()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules/playerbots)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/modules/playerbots)
  endif()
endif()

if(BUILD_GAME_SERVER)
  add_subdirectory(game)
  add_subdirectory(mangosd)
endif()

if(BUILD_LOGIN_SERVER)
  add_subdirectory(realmd)
endif()