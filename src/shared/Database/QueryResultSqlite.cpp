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

#include "DatabaseEnv.h"
#include "Util/Errors.h"
#include "sqlite3.h"
#include "QueryResultSqlite.h"

QueryResultSqlite::QueryResultSqlite(sqlite3_stmt** stmt) :
    QueryResult(0, 0), mStmt(stmt)
{
    if (mStmt && *mStmt)
    {
        while (sqlite3_step(*mStmt) == SQLITE_ROW)
        {
            // Process each row's data here
            mRowCount++;
        }
        sqlite3_reset(*mStmt);
        mFieldCount = sqlite3_column_count(*mStmt);
        mCurrentRow = new Field[mFieldCount];
        MANGOS_ASSERT(mCurrentRow);

        for (int i = 0; i < mFieldCount; ++i)
        {
            const unsigned char* value = sqlite3_column_text(*mStmt, i);
            mCurrentRow[i].SetValue(value ? reinterpret_cast<const char*>(value) : "");
            mCurrentRow[i].SetType(ConvertNativeType(sqlite3_column_type(*mStmt, i)));
        }
    }
}

QueryResultSqlite::~QueryResultSqlite()
{
    EndQuery();
}

bool QueryResultSqlite::NextRow()
{
    if (!mStmt || !(*mStmt))
        return false;

    int rc = sqlite3_step(*mStmt);
    if (rc != SQLITE_ROW)
    {
        EndQuery();
        return false;
    }

    for (int i = 0; i < mFieldCount; ++i)
    {
        const unsigned char* value = sqlite3_column_text(*mStmt, i);

        mCurrentRow[i].SetValue(value ? reinterpret_cast<const char*>(value) : nullptr);
        mCurrentRow[i].SetType(ConvertNativeType(sqlite3_column_type(*mStmt, i)));
    }

    return true;
}

void QueryResultSqlite::EndQuery()
{
    if (mStmt && *mStmt)
    {
        sqlite3_finalize(*mStmt);
        *mStmt = nullptr;
        //mStmt = nullptr;
    }

    delete[] mCurrentRow;
    mCurrentRow = nullptr;
}

enum Field::DataTypes QueryResultSqlite::ConvertNativeType(int sqliteType) const
{
    switch (sqliteType)
    {
        case SQLITE_INTEGER:
            return Field::DB_TYPE_INTEGER;
        case SQLITE_FLOAT:
            return Field::DB_TYPE_FLOAT;
        case SQLITE_TEXT:
        case SQLITE_BLOB:
        case SQLITE_NULL:
            return Field::DB_TYPE_STRING;
        default:
            return Field::DB_TYPE_UNKNOWN;
    }
}
#endif
