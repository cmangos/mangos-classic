# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

cmake_minimum_required (VERSION 3.0)

# disable warnings for dependencies
DisableWarnings()

file(GLOB mpqfiles libmpq/*.c libmpq/*.h *.h)

add_library(mpqlib STATIC
  ${mpqfiles}
)

target_link_libraries(mpqlib
  PRIVATE zlib
  PRIVATE ${BZIP2_LIBRARIES}
)

target_include_directories(mpqlib
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${BZIP2_INCLUDE_DIRS}
)

if(MSVC)
  target_compile_options(mpqlib PRIVATE /wd4103)
  target_include_directories(mpqlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/win)
  set_target_properties(mpqlib PROPERTIES FOLDER "Dependencies")
endif()
