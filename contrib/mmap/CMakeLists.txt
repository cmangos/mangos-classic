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

# disable warnings for dependencies
DisableWarnings()

ADD_DEFINITIONS(-DMMAP_GENERATOR -DNO_CORE_FUNCS)
ADD_DEFINITIONS(-DDT_POLYREF64)
ADD_DEFINITIONS(-DNO_vsnprintf)

if(UNIX)
  ADD_DEFINITIONS("-ggdb")
endif()

include_directories(
    ../../src
    ../../src/shared
    ../../src/game
    ../../src/game/vmap
    ../../dep/include/g3dlite
    ../../src/framework
    ../../dep/recastnavigation/Detour/Include
    ../../dep/recastnavigation/Recast/Include
    ${ZLIB_INCLUDE_DIRS}
)

# TODO: Do we still need that vmaplib?
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

target_link_libraries(vmaplib g3dlite ${ZLIB_LIBRARIES} ${EXTRA_LIBS})

set(SOURCES
    ./src/IntermediateValues.cpp
    ./src/generator.cpp
    ./src/MapBuilder.cpp
    ./src/TerrainBuilder.cpp
    ./src/VMapExtensions.cpp
)

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} g3dlite vmaplib detour recast ${ZLIB_LIBRARIES})

if(MSVC)
  # Define OutDir to source/bin/(platform)_(configuaration) folder.
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${DEV_BIN_DIR}/Extractors")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${DEV_BIN_DIR}/Extractors")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES PROJECT_LABEL "MoveMapGen")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Extractors")
  set_target_properties(vmaplib PROPERTIES FOLDER "Dependencies")
  
  # Add script file to dev folder
  set(FILES_LIST_TO_COPY
        \"${CMAKE_SOURCE_DIR}/contrib/extractor_scripts/MoveMapGen.sh\"
        \"${CMAKE_SOURCE_DIR}/contrib/extractor_scripts/offmesh.txt\")
  add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILES_LIST_TO_COPY} \"$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>\")
endif()

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${BIN_DIR}/tools)
