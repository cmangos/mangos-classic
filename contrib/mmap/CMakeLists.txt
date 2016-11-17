# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

cmake_minimum_required (VERSION 2.6)

set(EXECUTABLE_NAME "MoveMapGen")
project (${EXECUTABLE_NAME})

ADD_DEFINITIONS(-DMMAP_GENERATOR -DNO_CORE_FUNCS -DDEBUG)
ADD_DEFINITIONS(-DDT_POLYREF64)

# zlib
ADD_DEFINITIONS( -DNO_vsnprintf )

ADD_DEFINITIONS("-ggdb")
ADD_DEFINITIONS("-O3")

include_directories(
    ../../src
    ../../src/shared
    ../../src/game
    ../../src/game/vmap
    ../../dep/include/g3dlite
    ../../src/framework
    ../../dep/recastnavigation/Detour/Include
    ../../dep/recastnavigation/Recast/Include
    ../../dep/src/zlib
)

add_library(vmaplib
    ../../src/game/vmap/BIH.cpp
    ../../src/game/vmap/VMapManager2.cpp
    ../../src/game/vmap/MapTree.cpp
    ../../src/game/vmap/TileAssembler.cpp
    ../../src/game/vmap/WorldModel.cpp
    ../../src/game/vmap/ModelInstance.cpp
)

IF(APPLE)
   FIND_LIBRARY(CORE_SERVICES CoreServices )
   SET(EXTRA_LIBS ${CORE_SERVICES})
ENDIF (APPLE)

target_link_libraries(vmaplib g3dlite zlib ${EXTRA_LIBS})

set(SOURCES
    ./src/IntermediateValues.cpp
    ./src/generator.cpp
    ./src/MapBuilder.cpp
    ./src/TerrainBuilder.cpp
    ./src/VMapExtensions.cpp
)

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} g3dlite vmaplib detour recast zlib)
install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${BIN_DIR}/tools)
