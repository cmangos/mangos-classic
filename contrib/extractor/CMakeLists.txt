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

cmake_minimum_required (VERSION 2.6)
project (MANGOS_MAP_EXTRACTOR)

include(../../cmake/common.cmake)
include(CheckPlatform)

add_subdirectory (libmpq)
add_subdirectory (loadlib)

include_directories (${MANGOS_MAP_EXTRACTOR_SOURCE_DIR}/libmpq)
include_directories (${MANGOS_MAP_EXTRACTOR_SOURCE_DIR}/loadlib)

link_directories (${MANGOS_MAP_EXTRACTOR_SOURCE_DIR}/libmpq)
link_directories (${MANGOS_MAP_EXTRACTOR_SOURCE_DIR}/loadlib)

add_executable (ad dbcfile.cpp mpq_libmpq.cpp System.cpp)

target_link_libraries (ad libmpq)
target_link_libraries (ad loadlib)
