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

include(FetchContent)
# avoid online check when already downloaded (little speed up)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

if(BUILD_EXTRACTORS AND NOT (BZip2_FOUND OR BZIP2_FOUND))
  add_subdirectory(bzip2)
endif()

# zlib
FetchContent_Declare(
    zlib
    GIT_REPOSITORY "https://github.com/madler/zlib.git"
    GIT_TAG "v1.2.12"
)
FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
    message(STATUS "Setting up zlib ...")
    FetchContent_Populate(zlib)
    # patch provided cmake to avoid unneeded target install and deprecation warning.
    #file(COPY_FILE "patch-zlib-CMakeLists.txt" "${zlib_SOURCE_DIR}/CMakeLists.txt" ONLY_IF_DIFFERENT) #not working on cmake < 3.21 so replaced with 3 next lines
    file(COPY "patch-zlib-CMakeLists.txt" DESTINATION "${zlib_SOURCE_DIR}")
    file(REMOVE "${zlib_SOURCE_DIR}/CMakeLists.txt")
    file(RENAME "${zlib_SOURCE_DIR}/patch-zlib-CMakeLists.txt" "${zlib_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    target_include_directories(zlib PUBLIC ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    message(STATUS "zlib ${ZLIB_FULL_VERSION} successfully installed!")
endif()

if (BUILD_GAME_SERVER OR BUILD_LOGIN_SERVER)
  add_subdirectory(gsoap)
endif()

if (BUILD_GAME_SERVER OR BUILD_LOGIN_SERVER OR BUILD_EXTRACTORS)
  add_subdirectory(utf8cpp)
endif()
