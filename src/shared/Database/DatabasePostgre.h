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

#ifndef _DatabasePostgre_H
#define _DatabasePostgre_H

#include "Common.h"
#include "Database.h"
#include "Policies/Singleton.h"
#include <stdarg.h>

#ifdef _WIN32
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <postgre/libpq-fe.h>
#else
#include <postgresql/libpq-fe.h>
#endif

class PostgreSQLConnection : public SqlConnection
{
    public:
        PostgreSQLConnection(Database& db) : SqlConnection(db), mPGconn(nullptr) {}
        ~PostgreSQLConnection();

        bool Initialize(const char* infoString) override;

        QueryResult* Query(const char* sql) override;
        QueryNamedResult* QueryNamed(const char* sql) override;
        bool Execute(const char* sql) override;

        unsigned long escape_string(char* to, const char* from, unsigned long length);

        bool BeginTransaction() override;
        bool CommitTransaction() override;
        bool RollbackTransaction() override;

    private:
        bool _TransactionCmd(const char* sql);
        bool _Query(const char* sql, PGresult** pResult, uint64* pRowCount, uint32* pFieldCount);

        PGconn* mPGconn;
};

class DatabasePostgre : public Database
{
        friend class MaNGOS::OperatorNew<DatabasePostgre>;

    public:
        DatabasePostgre();
        ~DatabasePostgre();

        //! Initializes Postgres and connects to a server.
        /*! infoString should be formated like hostname;username;password;database. */

    protected:
        virtual SqlConnection* CreateConnection() override;

    private:
        static size_t db_count;
};
#endif
