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

set(LIBRARY_NAME framework)

set(SRC_GRP_DYNAMIC
    Dynamic/FactoryHolder.h
    Dynamic/ObjectRegistry.h
)

set(SRC_GRP_GAMESYSTEM
    GameSystem/Grid.h
    GameSystem/GridLoader.h
    GameSystem/GridReference.h
    GameSystem/GridRefManager.h
    GameSystem/NGrid.h
    GameSystem/TypeContainer.h
    GameSystem/TypeContainerFunctions.h
    GameSystem/TypeContainerVisitor.h
)

set(SRC_GRP_PLATFORM
    Platform/CompilerDefs.h
    Platform/Define.h
)

set(SRC_GRP_POLICIES
    Policies/CreationPolicy.h
    Policies/MemoryManagement.cpp
    Policies/ObjectLifeTime.cpp
    Policies/ObjectLifeTime.h
    Policies/Singleton.h
    Policies/ThreadingModel.h
)

set(SRC_GRP_UTIL
    Utilities/ByteConverter.h
    Utilities/Callback.h
    Utilities/EventProcessor.cpp
    Utilities/EventProcessor.h
    Utilities/LinkedList.h
    Utilities/TypeList.h
    Utilities/UnorderedMapSet.h
)

set(SRC_GRP_LINKED_REFERENCE
    Utilities/LinkedReference/Reference.h
    Utilities/LinkedReference/RefManager.h
)

set(LIBRARY_SRCS
    ${SRC_GRP_DYNAMIC}
    ${SRC_GRP_GAMESYSTEM}
    ${SRC_GRP_PLATFORM}
    ${SRC_GRP_POLICIES}
    ${SRC_GRP_UTIL}
    ${SRC_GRP_LINKED_REFERENCE}
)

source_group("Dynamic"
  FILES
    ${SRC_GRP_DYNAMIC}
)

source_group("GameSystem"
  FILES
    ${SRC_GRP_GAMESYSTEM}
)

source_group("Platform"
  FILES
    ${SRC_GRP_PLATFORM}
)

source_group("Policies"
  FILES
    ${SRC_GRP_POLICIES}
)

source_group("Utilities"
  FILES
    ${SRC_GRP_UTIL}
)

source_group("Utilities\\LinkedReference"
  FILES
    ${SRC_GRP_LINKED_REFERENCE}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ACE_INCLUDE_DIR}
)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

if(NOT TBB_USE_EXTERNAL)
  add_dependencies(${LIBRARY_NAME} TBB_Project)
endif()
if(NOT ACE_USE_EXTERNAL)
  add_dependencies(${LIBRARY_NAME} ACE_Project)
endif()

target_link_libraries(${LIBRARY_NAME}
  ${TBB_LIBRARIES}
)
