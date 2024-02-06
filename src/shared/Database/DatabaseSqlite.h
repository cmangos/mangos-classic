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

#ifndef DO_POSTGRESQL

#ifndef _DatabaseSqlite_H
#define _DatabaseSqlite_H

//#include "Common.h"
#include "Database.h"
#include "Policies/Singleton.h"
#include "QueryResultSqlite.h"

#include <sqlite3.h>

// MySQL prepared statement class
class SqlitePreparedStatement : public SqlPreparedStatement
{
    public:
        SqlitePreparedStatement(const std::string& fmt, SqlConnection& conn, sqlite3* mysql);
        ~SqlitePreparedStatement();

        // prepare statement
        virtual bool prepare() override;

        // bind input parameters
        virtual void bind(const SqlStmtParameters& holder) override;

        // execute DML statement
        virtual bool execute() override;

    protected:
        // bind parameters
        void addParam(unsigned int nIndex, const SqlStmtFieldData& data);

    private:
        void RemoveBinds();

        sqlite3* m_pSqliteConn;
        sqlite3_stmt** m_stmt;
        sqlite3_value* m_pResult;
};

class SQLiteConnection : public SqlConnection
{
    public:
        SQLiteConnection(Database& db) : SqlConnection(db), mSqlite(nullptr) {}
        ~SQLiteConnection();

        //! Initializes Mysql and connects to a server.
        /*! infoString should be formated like hostname;username;password;database. */
        bool Initialize(const char* infoString) override;

        std::unique_ptr<QueryResult> Query(const char* sql) override;
        QueryNamedResult* QueryNamed(const char* sql) override;
        bool Execute(const char* sql) override;

        unsigned long escape_string(char* to, const char* from, unsigned long length) override;

        bool BeginTransaction() override;
        bool CommitTransaction() override;
        bool RollbackTransaction() override;

    protected:
        SqlPreparedStatement* CreateStatement(const std::string& fmt) override;

    private:
        bool _TransactionCmd(const char* sql);
        bool _Query(const char* sql, sqlite3_stmt** pStmt);

        sqlite3* mSqlite;
};

class DatabaseSqlite : public Database
{
        friend class MaNGOS::OperatorNew<DatabaseSqlite>;

    protected:
        virtual SqlConnection* CreateConnection() override;
};

#endif
#endif
