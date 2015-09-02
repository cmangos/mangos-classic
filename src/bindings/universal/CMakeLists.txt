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

set(LIBRARY_NAME mangosscript)

set(LIBRARY_SRCS
    config.h
    ScriptMgr.cpp
    ScriptMgr.h
    system.cpp
    Scripts/sc_default.cpp
    Scripts/sc_defines.cpp
    Scripts/sc_defines.h
   )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

add_library(${LIBRARY_NAME} SHARED
  ${LIBRARY_SRCS}
)

add_dependencies(${LIBRARY_NAME} revision.h)
if(NOT ACE_USE_EXTERNAL)
  add_dependencies(${LIBRARY_NAME} ACE_Project)
endif()

target_link_libraries(${LIBRARY_NAME}
  ${ZLIB_LIBRARIES}
  ${ACE_LIBRARIES}
)

if(WIN32)
  target_link_libraries(${LIBRARY_NAME}
    mangosd # FIXME: could this be done for unix? because unix won't generate exe.libs
  )
  if(WIN32 AND PLATFORM MATCHES X86)
    target_link_libraries(${LIBRARY_NAME}
      debug ${WIN_DEBUGLIBS}
    )
  endif()
endif()

if(UNIX)
  set(LIBRARY_LINK_FLAGS "-pthread")
  if(APPLE)
    set(LIBRARY_LINK_FLAGS "-framework Carbon ${LIBRARY_LINK_FLAGS}")
    # Needed for the linking because of the missing symbols
    set(LIBRARY_LINK_FLAGS "-Wl,-undefined -Wl,dynamic_lookup ${LIBRARY_LINK_FLAGS}")
  endif()

  if(APPLE)
    set(LIBRARY_PROPERTIES INSTALL_NAME_DIR "${LIBS_DIR}")
  else()
    set(LIBRARY_PROPERTIES INSTALL_RPATH ${LIBS_DIR})
  endif()

  # Run out of build tree
  set(LIBRARY_PROPERTIES
    ${LIBRARY_PROPERTIES}
    BUILD_WITH_INSTALL_RPATH OFF
  )

  set_target_properties(${LIBRARY_NAME} PROPERTIES
    LINK_FLAGS ${LIBRARY_LINK_FLAGS}
    ${LIBRARY_PROPERTIES}
  )
endif()

# LIBRARY = dyld / so, RUNTIME = dll
install(TARGETS ${LIBRARY_NAME}
  LIBRARY DESTINATION ${LIBS_DIR}
  RUNTIME DESTINATION ${LIBS_DIR}
)
