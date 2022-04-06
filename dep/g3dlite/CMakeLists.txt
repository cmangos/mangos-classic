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

set(LIBRARY_NAME g3dlite)

set(LIBRARY_SRCS
    AABox.cpp
    Any.cpp
    #AnyVal.cpp
    #AreaMemoryManager.cpp
    BinaryFormat.cpp
    BinaryInput.cpp
    BinaryOutput.cpp
    Box.cpp
    #Box2D.cpp
    #BumpMapPreprocess.cpp
    Capsule.cpp
    CollisionDetection.cpp
    #Color1.cpp
    #Color1uint8.cpp
    #Color3.cpp
    #Color3uint8.cpp
    #Color4.cpp
    #Color4uint8.cpp
    #Cone.cpp
    #ConvexPolyhedron.cpp
    CoordinateFrame.cpp
    Crypto.cpp
    #Crypto_md5.cpp
    Cylinder.cpp
    FileSystem.cpp
    #GCamera.cpp
    #GImage.cpp
    #GImage_bayer.cpp
    #GImage_bmp.cpp
    #GImage_jpeg.cpp
    #GImage_png.cpp
    #GImage_ppm.cpp
    #GImage_tga.cpp
    #GLight.cpp
    #GThread.cpp
    #GUniqueID.cpp
    #Image1.cpp
    #Image1uint8.cpp
    #Image3.cpp
    #Image3uint8.cpp
    #Image4.cpp
    #Image4uint8.cpp
    #ImageFormat.cpp
    #ImageFormat_convert.cpp
    #Intersect.cpp
    Line.cpp
    LineSegment.cpp
    Log.cpp
    #Matrix.cpp
    Matrix3.cpp
    Matrix4.cpp
    MemoryManager.cpp
    #MeshAlg.cpp
    #MeshAlgAdjacency.cpp
    #MeshAlgWeld.cpp
    #MeshBuilder.cpp
    #NetAddress.cpp
    #NetworkDevice.cpp
    PhysicsFrame.cpp
    #PhysicsFrameSpline.cpp
    Plane.cpp
    #PrecomputedRandom.cpp
    Quat.cpp
    Random.cpp
    Ray.cpp
    #Rect2D.cpp
    ReferenceCount.cpp
    RegistryUtil.cpp
    Sphere.cpp
    #SplineBase.cpp
    #Stopwatch.cpp
    System.cpp
    TextInput.cpp
    TextOutput.cpp
    #ThreadSet.cpp
    Triangle.cpp
    UprightFrame.cpp
    Vector2.cpp
    #Vector2int16.cpp
    Vector3.cpp
    #Vector3int16.cpp
    #Vector3int32.cpp
    Vector4.cpp
    #Vector4int8.cpp
    #Welder.cpp
    #WinMain.cpp
    #XML.cpp
    #constants.cpp
    debugAssert.cpp
    fileutils.cpp
    #filter.cpp
    format.cpp
    g3dfnmatch.cpp
    g3dmath.cpp
    #license.cpp
    prompt.cpp
    stringutils.cpp
    uint128.cpp
)

# Find all the input files
FILE(GLOB LIBRARY_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/G3D/*.h")
list(APPEND LIBRARY_SRCS "${LIBRARY_HDRS}")

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)
target_link_libraries(${LIBRARY_NAME}
  PRIVATE zlib
)

target_include_directories(${LIBRARY_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "Dependencies")
endif()
