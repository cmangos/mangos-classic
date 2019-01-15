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

#include "SqlOperations.h"
#include "SqlDelayThread.h"
#include "DatabaseEnv.h"
#include "DatabaseImpl.h"

#include <cstdarg>

#define LOCK_DB_CONN(conn) SqlConnection::Lock guard(conn)

/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

bool SqlPlainRequest::Execute(SqlConnection* conn)
{
    /// just do it
    LOCK_DB_CONN(conn);
    return conn->Execute(m_sql);
}

SqlTransaction::~SqlTransaction()
{
    while (!m_queue.empty())
    {
        delete m_queue.back();
        m_queue.pop_back();
    }
}

bool SqlTransaction::Execute(SqlConnection* conn)
{
    if (m_queue.empty())
        return true;

    LOCK_DB_CONN(conn);

    conn->BeginTransaction();

    const int nItems = m_queue.size();
    for (int i = 0; i < nItems; ++i)
    {
        SqlOperation* pStmt = m_queue[i];

        if (!pStmt->Execute(conn))
        {
            conn->RollbackTransaction();
            return false;
        }
    }

    return conn->CommitTransaction();
}

SqlPreparedRequest::SqlPreparedRequest(int nIndex, SqlStmtParameters* arg) : m_nIndex(nIndex), m_param(arg)
{
}

SqlPreparedRequest::~SqlPreparedRequest()
{
    delete m_param;
}

bool SqlPreparedRequest::Execute(SqlConnection* conn)
{
    LOCK_DB_CONN(conn);
    return conn->ExecuteStmt(m_nIndex, *m_param);
}

/// ---- ASYNC QUERIES ----

bool SqlQuery::Execute(SqlConnection* conn)
{
    if (!m_callback || !m_queue)
        return false;

    LOCK_DB_CONN(conn);
    /// execute the query and store the result in the callback
    m_callback->SetResult(conn->Query(&m_sql[0]));
    /// add the callback to the sql result queue of the thread it originated from
    m_queue->Add(m_callback);

    return true;
}

void SqlResultQueue::Update()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    /// execute the callbacks waiting in the synchronization queue
    while (!m_queue.empty())
    {
        auto const callback = std::move(m_queue.front());
        m_queue.pop();
        callback->Execute();
    }
}

void SqlResultQueue::Add(MaNGOS::IQueryCallback* callback)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_queue.push(std::unique_ptr<MaNGOS::IQueryCallback>(callback));
}

bool SqlQueryHolder::Execute(MaNGOS::IQueryCallback* callback, SqlDelayThread* thread, SqlResultQueue* queue)
{
    if (!callback || !thread || !queue)
        return false;

    /// delay the execution of the queries, sync them with the delay thread
    /// which will in turn resync on execution (via the queue) and call back
    SqlQueryHolderEx* holderEx = new SqlQueryHolderEx(this, callback, queue);
    thread->Delay(holderEx);
    return true;
}

bool SqlQueryHolder::SetQuery(size_t index, const char* sql)
{
    if (m_queries.size() <= index)
    {
        sLog.outError("Query index (" SIZEFMTD ") out of range (size: " SIZEFMTD ") for query: %s", index, m_queries.size(), sql);
        return false;
    }

    if (m_queries[index].first != nullptr)
    {
        sLog.outError("Attempt assign query to holder index (" SIZEFMTD ") where other query stored (Old: [%s] New: [%s])",
                      index, m_queries[index].first, sql);
        return false;
    }

    /// not executed yet, just stored (it's not called a holder for nothing)
    m_queries[index] = SqlResultPair(mangos_strdup(sql), (QueryResult*)nullptr);
    return true;
}

bool SqlQueryHolder::SetPQuery(size_t index, const char* format, ...)
{
    if (!format)
    {
        sLog.outError("Query (index: " SIZEFMTD ") is empty.", index);
        return false;
    }

    va_list ap;
    char szQuery [MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if (res == -1)
    {
        sLog.outError("SQL Query truncated (and not execute) for format: %s", format);
        return false;
    }

    return SetQuery(index, szQuery);
}

QueryResult* SqlQueryHolder::GetResult(size_t index)
{
    if (index < m_queries.size())
    {
        /// the query strings are freed on the first GetResult or in the destructor
        if (m_queries[index].first != nullptr)
        {
            delete[](const_cast<char*>(m_queries[index].first));
            m_queries[index].first = nullptr;
        }
        /// when you get a result aways remember to delete it!
        return m_queries[index].second;
    }
    return nullptr;
}

void SqlQueryHolder::SetResult(size_t index, QueryResult* result)
{
    /// store the result in the holder
    if (index < m_queries.size())
        m_queries[index].second = result;
}

SqlQueryHolder::~SqlQueryHolder()
{
    for (auto& m_querie : m_queries)
    {
        /// if the result was never used, free the resources
        /// results used already (getresult called) are expected to be deleted
        if (m_querie.first != nullptr)
        {
            delete[](const_cast<char*>(m_querie.first));
            delete m_querie.second;
        }
    }
}

void SqlQueryHolder::SetSize(size_t size)
{
    /// to optimize push_back, reserve the number of queries about to be executed
    m_queries.resize(size);
}

bool SqlQueryHolderEx::Execute(SqlConnection* conn)
{
    if (!m_holder || !m_callback || !m_queue)
        return false;

    LOCK_DB_CONN(conn);
    /// we can do this, we are friends
    std::vector<SqlQueryHolder::SqlResultPair>& queries = m_holder->m_queries;
    for (size_t i = 0; i < queries.size(); ++i)
    {
        /// execute all queries in the holder and pass the results
        char const* sql = queries[i].first;
        if (sql) m_holder->SetResult(i, conn->Query(sql));
    }

    /// sync with the caller thread
    m_queue->Add(m_callback);

    return true;
}
