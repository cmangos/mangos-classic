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

QueryResultSqlite::QueryResultSqlite(sqlite3_stmt* stmt) :
    QueryResult(0, 0), mNextRow(0)
{
    if (!stmt)
        return;

    mFieldCount = sqlite3_column_count(stmt);

    // The whole result is read before it is used: a statement still running keeps a read transaction open on its
    // connection, and a write on that connection then fails at once when another connection has written meanwhile.
    int result;
    while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        for (int i = 0; i < int(mFieldCount); ++i)
        {
            // the type is read first, reading the text can convert the value
            mValueTypes.push_back(uint8(ConvertNativeType(sqlite3_column_type(stmt, i))));

            const unsigned char* value = sqlite3_column_text(stmt, i);
            if (!value)
            {
                mValueOffsets.push_back(NULL_VALUE);
                continue;
            }

            mValueOffsets.push_back(mValues.size());
            mValues.append(reinterpret_cast<const char*>(value), size_t(sqlite3_column_bytes(stmt, i)));
            mValues.push_back('\0');
        }
        ++mRowCount;
    }

    if (result != SQLITE_DONE)
    {
        sLog.outErrorDb("SQL: %s", sqlite3_sql(stmt));
        sLog.outErrorDb("query ERROR: %s", sqlite3_errmsg(sqlite3_db_handle(stmt)));
        mRowCount = 0;
        mValues.clear();
        mValueOffsets.clear();
        mValueTypes.clear();
    }

    sqlite3_finalize(stmt);

    mCurrentRow = new Field[mFieldCount];
}

QueryResultSqlite::~QueryResultSqlite()
{
    EndQuery();
}

bool QueryResultSqlite::NextRow()
{
    if (!mCurrentRow)
        return false;

    if (mNextRow >= mRowCount)
    {
        EndQuery();
        return false;
    }

    size_t const firstValue = size_t(mNextRow) * mFieldCount;
    for (uint32 i = 0; i < mFieldCount; ++i)
    {
        size_t const offset = mValueOffsets[firstValue + i];
        mCurrentRow[i].SetValue(offset == NULL_VALUE ? nullptr : mValues.c_str() + offset);
        mCurrentRow[i].SetType(Field::DataTypes(mValueTypes[firstValue + i]));
    }

    ++mNextRow;
    return true;
}

void QueryResultSqlite::EndQuery()
{
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
