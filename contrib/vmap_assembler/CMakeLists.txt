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

cmake_minimum_required (VERSION 2.6)
project (MANGOS_VMAP_ASSEMB_IO)

include(../../cmake/common.cmake)
include(CheckPlatform)

set(CMAKE_VERBOSE_MAKEFILE true)

# uncomment next line to disable debug mode
ADD_DEFINITIONS("-DIOMAP_DEBUG")
ADD_DEFINITIONS("-DNO_CORE_FUNCS")

ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-ggdb")
ADD_DEFINITIONS("-O3")

include_directories(../../src/shared)
include_directories(../../src/game/vmap/)
include_directories(../../dep/include/g3dlite/)
include_directories(../../src/framework/)

add_library(g3dlite ../../dep/src/g3dlite/AABox.cpp
    ../../dep/src/g3dlite/Box.cpp
    ../../dep/src/g3dlite/Crypto.cpp
    ../../dep/src/g3dlite/format.cpp
    ../../dep/src/g3dlite/Matrix3.cpp
    ../../dep/src/g3dlite/Plane.cpp
    ../../dep/src/g3dlite/System.cpp
    ../../dep/src/g3dlite/Triangle.cpp
    ../../dep/src/g3dlite/Vector3.cpp
    ../../dep/src/g3dlite/Vector4.cpp
    ../../dep/src/g3dlite/debugAssert.cpp
    ../../dep/src/g3dlite/fileutils.cpp
    ../../dep/src/g3dlite/g3dmath.cpp
    ../../dep/src/g3dlite/g3dfnmatch.cpp
    ../../dep/src/g3dlite/prompt.cpp
    ../../dep/src/g3dlite/stringutils.cpp
    ../../dep/src/g3dlite/Any.cpp
    ../../dep/src/g3dlite/BinaryFormat.cpp
    ../../dep/src/g3dlite/BinaryInput.cpp
    ../../dep/src/g3dlite/BinaryOutput.cpp
    ../../dep/src/g3dlite/Capsule.cpp
    ../../dep/src/g3dlite/CollisionDetection.cpp
    ../../dep/src/g3dlite/CoordinateFrame.cpp
    ../../dep/src/g3dlite/Cylinder.cpp
    ../../dep/src/g3dlite/Line.cpp
    ../../dep/src/g3dlite/LineSegment.cpp
    ../../dep/src/g3dlite/Log.cpp
    ../../dep/src/g3dlite/Matrix4.cpp
    ../../dep/src/g3dlite/MemoryManager.cpp
    ../../dep/src/g3dlite/Quat.cpp
    ../../dep/src/g3dlite/Random.cpp
    ../../dep/src/g3dlite/Ray.cpp
    ../../dep/src/g3dlite/ReferenceCount.cpp
    ../../dep/src/g3dlite/Sphere.cpp
    ../../dep/src/g3dlite/TextInput.cpp
    ../../dep/src/g3dlite/TextOutput.cpp
    ../../dep/src/g3dlite/UprightFrame.cpp
    ../../dep/src/g3dlite/Vector2.cpp
    ../../dep/src/g3dlite/FileSystem.cpp
    ../../dep/src/g3dlite/uint128.cpp
    ../../dep/src/g3dlite/PhysicsFrame.cpp
    )
add_library(vmap
    ../../src/game/vmap/BIH.cpp
    ../../src/game/vmap/VMapManager2.cpp
    ../../src/game/vmap/MapTree.cpp
    ../../src/game/vmap/TileAssembler.cpp
    ../../src/game/vmap/WorldModel.cpp
    ../../src/game/vmap/ModelInstance.cpp
    )
target_link_libraries(vmap g3dlite z)

add_executable(vmap_assembler vmap_assembler.cpp)
target_link_libraries(vmap_assembler vmap)

# add_executable(vmap_test coordinate_test.cpp)
# target_link_libraries(vmap_test vmap)
