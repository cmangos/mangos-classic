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

set(LIBRARY_NAME anticheat)

add_definitions(-DBUILD_ANTICHEAT)

set(SRC_WARDEN
    Warden/Warden.cpp
    Warden/Warden.h
    Warden/WardenCheckMgr.h
    Warden/WardenCheckMgr.cpp
    Warden/WardenMac.cpp
    Warden/WardenMac.h
    Warden/WardenWin.cpp
    Warden/WardenWin.h
)

set(LIBRARY_SRCS
    ${SRC_WARDEN}
)

source_group("Warden"
  FILES
    ${SRC_WARDEN}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

target_include_directories(${LIBRARY_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${LIBRARY_NAME}
  PUBLIC shared
  PUBLIC game
)