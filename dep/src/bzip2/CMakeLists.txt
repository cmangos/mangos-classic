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

set(LIBRARY_SRCS
  compress.c
  crctable.c
  decompress.c
  huffman.c
  randtable.c
  blocksort.c
  bzlib.c
)

add_library(${BZIP2_LIBRARIES} STATIC
  ${LIBRARY_SRCS}
)

target_include_directories(${BZIP2_LIBRARIES}
  INTERFACE ${CMAKE_SOURCE_DIR}/dep/include/bzip2
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  set_target_properties(${BZIP2_LIBRARIES} PROPERTIES FOLDER "Dependencies")
endif()
