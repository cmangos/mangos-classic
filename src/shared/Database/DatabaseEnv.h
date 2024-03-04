/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if !defined(DATABASEENV_H)
#define DATABASEENV_H

#include "Common.h"
#include "Util/Errors.h"

#include "Database/Field.h"
#include "Database/QueryResult.h"

#ifdef DO_POSTGRESQL
#include "Database/QueryResultPostgre.h"
#include "Database/Database.h"
#include "Database/DatabasePostgre.h"
typedef DatabasePostgre DatabaseType;
#define _LIKE_           "ILIKE"
#define _TABLE_SIM_      "\""
#define _CONCAT3_(A,B,C) "( " A " || " B " || " C " )"
#define _OFFSET_         "LIMIT 1 OFFSET %d"
#define _TRUNCATE_       "TRUNCATE TABLE"
#define _NOW_            "NOW()"
#define _UNIXTIME_       "UNIX_TIMESTAMP()"
#define _UNIXNOW_        "UNIX_TIMESTAMP(NOW())"
#define _FROM_UNIXTIME_(X) "TO_TIMESTAMP(" X ")"
#elif DO_SQLITE
#include "Database/QueryResultSqlite.h"
#include "Database/Database.h"
#include "Database/DatabaseSqlite.h"
typedef DatabaseSqlite DatabaseType;
#define _LIKE_           "LIKE"
#define _TABLE_SIM_      '`'
#define _CONCAT3_(A,B,C) "( " A " || " B " || " C " )"
#define _OFFSET_         "LIMIT %d,1"
#define _TRUNCATE_       "DELETE FROM"
#define _NOW_            "datetime()"
#define _UNIXTIME_       "unixepoch()"
#define _UNIXNOW_        "unixepoch('now')"
#define _FROM_UNIXTIME_(X) "datetime(" X ",'unixepoch')"
#else
#include "Database/QueryResultMysql.h"
#include "Database/Database.h"
#include "Database/DatabaseMysql.h"
typedef DatabaseMysql DatabaseType;
#define _LIKE_           "LIKE"
#define _TABLE_SIM_      '`'
#define _CONCAT3_(A,B,C) "CONCAT( " A " , " B " , " C " )"
#define _OFFSET_         "LIMIT %d,1"
#define _TRUNCATE_       "TRUNCATE TABLE"
#define _NOW_            "NOW()"
#define _UNIXTIME_       "UNIX_TIMESTAMP()"
#define _UNIXNOW_        "UNIX_TIMESTAMP(NOW())"
#define _FROM_UNIXTIME_(X) "FROM_UNIXTIME(" X ")"
#endif

extern DatabaseType WorldDatabase;
extern DatabaseType CharacterDatabase;
extern DatabaseType LoginDatabase;
extern DatabaseType LogsDatabase;

#endif
