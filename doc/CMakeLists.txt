# This file is part of the Continued-MaNGOS Project
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

cmake_minimum_required(VERSION 2.8)

INCLUDE(GNUInstallDirs)
find_package(Doxygen)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.dist" DOXYFILE)
string(REPLACE "@PACKAGE@" ${CMAKE_PROJECT_NAME} DOXYFILE_GENERATED "${DOXYFILE}")
string(REPLACE "@VERSION@" ${REVISION_ID} DOXYFILE_GENERATED "${DOXYFILE_GENERATED}")
string(REPLACE "@TOPSRCDIR@" ${CMAKE_SOURCE_DIR} DOXYFILE_GENERATED "${DOXYFILE_GENERATED}")
string(REPLACE "@DESTDIR@" ${CMAKE_BINARY_DIR}/doc DOXYFILE_GENERATED "${DOXYFILE_GENERATED}")
file(WRITE "${CMAKE_BINARY_DIR}/doc/Doxyfile" "${DOXYFILE_GENERATED}")

add_custom_target(
  doc ALL
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile &> doxygen.out
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating documentation with Doxygen"
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/man/man3 DESTINATION ${CMAKE_INSTALL_MANDIR})
