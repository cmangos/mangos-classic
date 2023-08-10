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

# EXCLUDE_FROM_ALL important to bypass install target witch we dont need as we link statically
if (BUILD_GAME_SERVER OR BUILD_EXTRACTORS OR BUILD_RECASTDEMOMOD)
  add_definitions(-DDT_POLYREF64)
  add_subdirectory(Detour EXCLUDE_FROM_ALL)
  target_include_directories(Detour
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
  set_target_properties(Detour PROPERTIES DEBUG_POSTFIX "")
  )
endif()

if (BUILD_EXTRACTORS OR BUILD_RECASTDEMOMOD)
  add_subdirectory(Recast EXCLUDE_FROM_ALL)
  set_target_properties(Recast PROPERTIES DEBUG_POSTFIX "")
endif()

if (BUILD_RECASTDEMOMOD)
  add_subdirectory(DetourCrowd EXCLUDE_FROM_ALL)
  add_subdirectory(DetourTileCache EXCLUDE_FROM_ALL)
  add_subdirectory(DebugUtils EXCLUDE_FROM_ALL)
endif()
