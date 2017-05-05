# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

cmake_minimum_required (VERSION 2.8)
project(mpqlib)

# disable warnings for dependencies
DisableWarnings()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
                    ${ZLIB_INCLUDE_DIRS}
                    ${BZIP2_INCLUDE_DIRS})

file(GLOB mpqfiles libmpq/*.c libmpq/*.h *.h)

add_library(mpqlib STATIC ${mpqfiles})
target_link_libraries(mpqlib ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})

if(MSVC)
  target_compile_options(mpqlib PRIVATE /wd4103)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win)
  set_target_properties(mpqlib PROPERTIES FOLDER "Dependencies")
endif()
