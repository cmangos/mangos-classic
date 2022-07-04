# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

set(EXECUTABLE_NAME "vmap_assembler")
project (${EXECUTABLE_NAME})

# uncomment next line to disable debug mode
if(DEBUG)
    ADD_DEFINITIONS("-DIOMAP_DEBUG")
endif()

ADD_DEFINITIONS("-DNO_CORE_FUNCS")

if(UNIX)
  ADD_DEFINITIONS("-ggdb")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/game/vmap)

list(APPEND VMAP_ASSEMBLER_SOURCE
    ${CMAKE_SOURCE_DIR}/src/game/vmap/BIH.cpp
    ${CMAKE_SOURCE_DIR}/src/game/vmap/VMapManager2.cpp
    ${CMAKE_SOURCE_DIR}/src/game/vmap/MapTree.cpp
    ${CMAKE_SOURCE_DIR}/src/game/vmap/TileAssembler.cpp
    ${CMAKE_SOURCE_DIR}/src/game/vmap/WorldModel.cpp
    ${CMAKE_SOURCE_DIR}/src/game/vmap/ModelInstance.cpp
    vmap_assembler.cpp)

IF(APPLE)
   FIND_LIBRARY(CORE_SERVICES CoreServices)
   SET(EXTRA_LIBS ${CORE_SERVICES})
ENDIF (APPLE)

add_executable(${EXECUTABLE_NAME} ${VMAP_ASSEMBLER_SOURCE})

target_link_libraries(${EXECUTABLE_NAME}
  shared
  g3dlite
  ${EXTRA_LIBS}
)

if(MSVC)
  # Define OutDir to source/bin/(platform)_(configuaration) folder.
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${DEV_BIN_DIR}/Extractors")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${DEV_BIN_DIR}/Extractors")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${DEV_BIN_DIR}/Extractors")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES PROJECT_LABEL "VMapAssembler")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Extractors")
endif()

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${BIN_DIR}/tools)

# add_executable(vmap_test coordinate_test.cpp)
# target_link_libraries(vmap_test vmap)
