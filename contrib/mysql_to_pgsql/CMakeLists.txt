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

SET(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required (VERSION 2.6)

INCLUDE(cmake/FindMySQL.cmake)
INCLUDE(cmake/FindPostgreSQL.cmake)

MESSAGE("-- Check PostgreSQL")
FIND_PGSQL()
ADD_DEFINITIONS(-DDO_POSTGRESQL)

MESSAGE("-- Check MySQL")
FIND_MYSQL()
ADD_DEFINITIONS(-DDO_MYSQL)

INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PGSQL_INCLUDE_DIR})

ADD_EXECUTABLE (mysql2pgsql src/main.cpp)

TARGET_LINK_LIBRARIES(mysql2pgsql ${PGSQL_LIBRARIES})
TARGET_LINK_LIBRARIES(mysql2pgsql ${MYSQL_LIBRARIES})
