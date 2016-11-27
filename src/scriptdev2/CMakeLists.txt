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

## magic to include revision data in SD2 version string
# revision.h: FORCE
#   $(top_builddir)/src/tools/genrevision/genrevision $(srcdir)

file(GLOB_RECURSE mangosscript_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

source_group("Other"
  REGULAR_EXPRESSION .*
)

foreach(SRC ${mangosscript_SRCS})
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

add_library(mangosscript SHARED
  ${mangosscript_SRCS}
)

if(WIN32)
  target_link_libraries(mangosscript
    mangosd # FIXME: could this be done for unix? because unix won't generate exe.libs
    debug ${WIN_DEBUGLIBS}
  )
endif()

add_dependencies(mangosscript mangosd)

if(UNIX)
  set(mangosscript_LINK_FLAGS "-pthread")
  if(APPLE)
    set(mangosscript_LINK_FLAGS "-framework Carbon ${mangosscript_LINK_FLAGS}")
    # Needed for the linking because of the missing symbols
    set(mangosscript_LINK_FLAGS "-Wl,-undefined -Wl,dynamic_lookup ${mangosscript_LINK_FLAGS}")
  endif()

  if(APPLE)
    set(mangosscript_PROPERTIES INSTALL_NAME_DIR "${LIBS_DIR}")
  else()
    set(mangosscript_PROPERTIES INSTALL_RPATH ${LIBS_DIR})
  endif()

  # Run out of build tree
  set(mangosscript_PROPERTIES
    ${mangosscript_PROPERTIES}
    BUILD_WITH_INSTALL_RPATH OFF
  )

  set_target_properties(mangosscript PROPERTIES
    LINK_FLAGS ${mangosscript_LINK_FLAGS}
    ${mangosscript_PROPERTIES}
  )
endif()

# Because size for linker is to big - seriously ?!
if(WIN32)
  set_target_properties(mangosscript PROPERTIES
      LINK_FLAGS_DEBUG "/DEBUG /INCREMENTAL:NO"
  )
endif()

## libtool settings
#  API versioning
#  Link against dependencies
#  How to increase version info:
#  - only bug fixes implemented:
#    bump the version to LTMANGOS_CURRENT:LTMANGOS_REVISION+1:LTMANGOS_AGE
#  - augmented the interface:
#    bump the version to LTMANGOS_CURRENT+1:0:LTMANGOS_AGE+1
#  - broken old interface:
#    bump the version to LTMANGOS_CURRENT+1:0:0
# set(LTMANGOS_CURRENT 0)
# set(LTMANGOS_REVISION 0)
# set(LTMANGOS_AGE 0)
# set_target_properties(script PROPERTIES LINK_FLAGS
#   "-version-info ${LTMANGOS_CURRENT}:${LTMANGOS_REVISION}:${LTMANGOS_AGE}"
# )

# Generate precompiled header
if(PCH)
  cotire(mangosscript)
endif()

# LIBRARY = dyld / so, RUNTIME = dll
install(TARGETS mangosscript
  LIBRARY DESTINATION ${LIBS_DIR}
  RUNTIME DESTINATION ${LIBS_DIR}
)
