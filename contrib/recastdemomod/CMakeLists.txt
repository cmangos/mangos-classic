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

# Find all the input files
FILE(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}" "Source/*.cpp")
FILE(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" "Include/*.h")

add_executable(RecastDemoMod ${SRCS} ${HEADERS})
#add_definitions(-DDT_POLYREF64)
target_compile_definitions(RecastDemoMod PRIVATE -DDT_POLYREF64)
target_link_libraries(RecastDemoMod
    PUBLIC fastlz
    PUBLIC Detour
    PUBLIC Recast
    PUBLIC g3dlite
    PUBLIC DetourCrowd
    PUBLIC DetourTileCache
    PUBLIC DebugUtils
    PUBLIC vmaplib
    PUBLIC mmaplib
)

if (WIN32)
  if(PLATFORM MATCHES X86) # 32-bit
    set(FOLDER_ARCH x86)
  else() # 64-bit
    set(FOLDER_ARCH x64)
  endif()
  target_link_libraries(RecastDemoMod
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL/lib/${FOLDER_ARCH}/SDL.lib"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL/lib/${FOLDER_ARCH}/SDLmain.lib"
    PRIVATE "opengl32.lib"
    PRIVATE "glu32.lib"
  )
endif()

add_subdirectory(dep/fastlz)

if (MSVC)
  set_target_properties(fastlz PROPERTIES FOLDER "Dependencies")
endif()

target_include_directories(RecastDemoMod
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dep/stb_image"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/game"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/game/vmap"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/framework"
)

if (WIN32 AND MSVC)
  # Define OutDir to source/bin/(platform)_(configuaration) folder.
  set_target_properties(RecastDemoMod PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${DEV_BIN_DIR}/RecastDemoMod")
  set_target_properties(RecastDemoMod PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${DEV_BIN_DIR}/RecastDemoMod")
  set_target_properties(RecastDemoMod PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

  # Define windows application type to not see a console
  set_target_properties(RecastDemoMod PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

  # Add script file to dev folder
  set(FILES_LIST_TO_COPY
    \"${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL/lib/${FOLDER_ARCH}/SDL.dll\"
    \"${CMAKE_CURRENT_SOURCE_DIR}/dep/font/DroidSans.ttf\")
  add_custom_command(TARGET RecastDemoMod POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILES_LIST_TO_COPY} \"$<TARGET_FILE_DIR:RecastDemoMod>\")
endif()