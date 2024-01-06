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

#ifdef DO_SQLITE

#if !defined(QUERYRESULTSQLITE_H)
#define QUERYRESULTSQLITE_H

#include "Common.h"
#include "QueryResult.h"

#ifdef _WIN32
  #include <WinSock2.h>
#endif

#include <sqlite3.h>

class QueryResultSqlite : public QueryResult
{
    public:
        QueryResultSqlite(sqlite3_stmt** stmt);

        ~QueryResultSqlite();

        bool NextRow() override;

        operator bool() const
        {
          return (mRowCount * mFieldCount) > 0;
        }

    private:
        enum Field::DataTypes ConvertNativeType(int sqliteType) const;
        //enum Field::DataTypes ConvertNativeType(enum_field_types mysqlType) const;
        void EndQuery();

        sqlite3_stmt** mStmt;
};
#endif
#endif
