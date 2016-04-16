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

set(LIBRARY_NAME shared)

set(SRC_GRP_AUTH
    Auth/AuthCrypt.cpp
    Auth/AuthCrypt.h
    Auth/BigNumber.cpp
    Auth/BigNumber.h
    Auth/Hmac.cpp
    Auth/Hmac.h
    Auth/md5.h
    Auth/Sha1.cpp
    Auth/Sha1.h
)

set(SRC_GRP_CONFIG
    Config/Config.cpp
    Config/Config.h
)

set(SRC_GRP_DATABASE
    Database/Database.cpp
    Database/Database.h
    Database/DatabaseEnv.h
    Database/DatabaseImpl.h
    Database/DatabaseMysql.cpp
    Database/DatabaseMysql.h
    Database/DatabasePostgre.cpp
    Database/DatabasePostgre.h
    Database/Field.cpp
    Database/Field.h
    Database/PGSQLDelayThread.h
    Database/QueryResult.h
    Database/QueryResultMysql.cpp
    Database/QueryResultMysql.h
    Database/QueryResultPostgre.cpp
    Database/QueryResultPostgre.h
    Database/SqlDelayThread.cpp
    Database/SqlDelayThread.h
    Database/SqlOperations.cpp
    Database/SqlOperations.h
    Database/SqlPreparedStatement.cpp
    Database/SqlPreparedStatement.h
    Database/SQLStorage.cpp
    Database/SQLStorage.h
    Database/SQLStorageImpl.h
)

set(SRC_GRP_DATABASE_DBC
    Database/DBCFileLoader.cpp
    Database/DBCFileLoader.h
    Database/DBCStore.h
)

set(SRC_GRP_LOG
    Log.cpp
    Log.h
)

set(SRC_GRP_NETWORK
    Network/PacketBuffer.cpp
    Network/Socket.cpp
    Network/Listener.hpp
    Network/NetworkThread.hpp
    Network/PacketBuffer.hpp
    Network/Socket.hpp
)

set(SRC_GRP_UTIL
    ByteBuffer.cpp
    ByteBuffer.h
    Errors.h
    # dep/include/mersennetwister/MersenneTwister.h is part of this group in the VC 2012 file but it is not part of src/shared, so it is omitted here
    ProgressBar.cpp
    ProgressBar.h
    Timer.h
    Util.cpp
    Util.h
    WorldPacket.h
)


set(LIBRARY_SRCS
    ${SRC_GRP_AUTH}
    ${SRC_GRP_CONFIG}
    ${SRC_GRP_DATABASE}
    ${SRC_GRP_DATABASE_DBC}
    ${SRC_GRP_LOG}
    ${SRC_GRP_UTIL}
    ${SRC_GRP_NETWORK}
    Common.cpp
    Common.h
    revision_sql.h
    Threading.cpp
    Threading.h
)


source_group("Auth"
  FILES
    ${SRC_GRP_AUTH}
)

source_group("Config"
  FILES
    ${SRC_GRP_CONFIG}
)

source_group("Database"
  FILES
    ${SRC_GRP_DATABASE}
)

source_group("Database\\DataStores"
  FILES
    ${SRC_GRP_DATABASE_DBC}
)

source_group("Log"
  FILES
    ${SRC_GRP_LOG}
)

source_group("Util"
  FILES
    ${SRC_GRP_UTIL}
)

# OS specific files
if(WIN32)
  set(LIBRARY_SRCS
      ${LIBRARY_SRCS}
      WheatyExceptionReport.cpp
      WheatyExceptionReport.h
      ServiceWin32.cpp
      ServiceWin32.h
     )
else()
  set(LIBRARY_SRCS
      ${LIBRARY_SRCS}
      PosixDaemon.cpp
      PosixDaemon.h
     )
endif()

# include external dependencies without warnings
include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}/dep/include
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
)

if(POSTGRESQL)
  include_directories(${PostgreSQL_INCLUDE_DIRS})
else()
  include_directories(${MYSQL_INCLUDE_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SystemConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/SystemConfig.h)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

if(POSTGRESQL)
  target_link_libraries(${LIBRARY_NAME} ${PostgreSQL_LIBRARIES})
endif()

add_dependencies(${LIBRARY_NAME} revision.h)

