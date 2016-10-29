#
# This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

set(LIBRARY_NAME scriptdev2)

file(GLOB_RECURSE LIBRARY_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

source_group("Other"
  REGULAR_EXPRESSION .*
)

foreach(SRC ${LIBRARY_SRCS})
  get_filename_component(PTH ${SRC} PATH)
  if(PTH)
    if(NOT XCODE) # FIXME: Xcode Generator has bug with nested dirs
      string(REPLACE "/" "\\\\" PTH ${PTH})
    endif()
    source_group(${PTH} FILES ${SRC})
  endif()
endforeach(SRC)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/base
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
)

if(PCH)
  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

if(WIN32)
  target_link_libraries(mangosscript
    mangosd # FIXME: could this be done for unix? because unix won't generate exe.libs
    debug ${WIN_DEBUGLIBS}
  )
endif()

add_dependencies(${LIBRARY_NAME} revision.h)

if(UNIX)
  set(LIBRARY_LINK_FLAGS "-pthread")
  if(APPLE)
    set(LIBRARY_LINK_FLAGS "-framework Carbon ${LIBRARY_LINK_FLAGS}")
    # Needed for the linking because of the missing symbols
    set(LIBRARY_LINK_FLAGS "-Wl,-undefined -Wl,dynamic_lookup ${LIBRARY_LINK_FLAGS}")
  endif()

  # Run out of build tree
  set(LIBRARY_PROPERTIES
    BUILD_WITH_INSTALL_RPATH OFF
  )

  set_target_properties(${LIBRARY_NAME} PROPERTIES
    LINK_FLAGS ${LIBRARY_LINK_FLAGS}
    ${LIBRARY_PROPERTIES}
  )
endif()

# Because size for linker is to big - seriously ?!
if(WIN32)
  set_target_properties(LIBRARY_PROPERTIES PROPERTIES
      LINK_FLAGS_DEBUG "/DEBUG /INCREMENTAL:NO"
  )
endif()

if(PCH)
  cotire(${LIBRARY_NAME})
endif()
