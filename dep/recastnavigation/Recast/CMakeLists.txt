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

# enumerate all library files
FILE(GLOB_RECURSE SRCS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Source/*.cpp")
FILE(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Include/*.h")

# create the lib
add_library(recast STATIC
  ${SRCS}
  ${HEADERS}
)

# add header include folder
target_include_directories(recast
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

if(MSVC)
  # Put this lib in dependencies category
  set_target_properties(recast PROPERTIES FOLDER "Dependencies")
endif()
