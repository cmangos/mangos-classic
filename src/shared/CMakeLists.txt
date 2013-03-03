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

set(LIBRARY_NAME shared)

set(LIBRARY_SRCS
    Auth/AuthCrypt.cpp
    Auth/AuthCrypt.h
    Auth/BigNumber.cpp
    Auth/BigNumber.h
    Auth/Hmac.cpp
    Auth/Hmac.h
    Auth/md5.h
    Auth/Sha1.cpp
    Auth/Sha1.h
    ByteBuffer.cpp
    ByteBuffer.h
    Common.cpp
    Common.h
    Config/Config.cpp
    Config/Config.h
    Config/ConfigEnv.h
    Database/Database.cpp
    Database/Database.h
    Database/DatabaseEnv.h
    Database/DatabaseImpl.h
    Database/DatabaseMysql.cpp
    Database/DatabaseMysql.h
    Database/DatabasePostgre.cpp
    Database/DatabasePostgre.h
    Database/DBCFileLoader.cpp
    Database/DBCFileLoader.h
    Database/DBCStore.h
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
    Errors.h
    LockedQueue.h
    Log.cpp
    Log.h
    ProgressBar.cpp
    ProgressBar.h
    revision_nr.h
    revision_sql.h
    Threading.cpp
    Threading.h
    Timer.h
    Util.cpp
    Util.h
    WorldPacket.h
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

source_group("Util"
  REGULAR_EXPRESSION .*
)

# TODO: really needed?
foreach(SRC ${LIBRARY_SRCS})
  get_filename_component(PTH ${SRC} PATH)
  if(PTH)
    if(NOT XCODE) # FIXME: Xcode Generator has bug with nested dirs
      string(REPLACE "/" "\\\\" PTH ${PTH})
    endif()
    source_group(${PTH} FILES ${SRC})
  endif()
endforeach(SRC)

source_group("DataStores"
  REGULAR_EXPRESSION DBC
)

source_group("Log"
  REGULAR_EXPRESSION Log
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SystemConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/SystemConfig.h)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

add_dependencies(${LIBRARY_NAME} revision.h)
if(NOT ACE_USE_EXTERNAL)
  add_dependencies(${LIBRARY_NAME} ACE_Project)
endif()
