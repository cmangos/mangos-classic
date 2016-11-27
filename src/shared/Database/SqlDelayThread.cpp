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

#include "Database/SqlDelayThread.h"
#include "Database/SqlOperations.h"
#include "DatabaseEnv.h"

SqlDelayThread::SqlDelayThread(Database* db, SqlConnection* conn) : m_dbEngine(db), m_dbConnection(conn), m_running(true)
{
}

SqlDelayThread::~SqlDelayThread()
{
    // process all requests which might have been queued while thread was stopping
    ProcessRequests();
}

void SqlDelayThread::run()
{
#ifndef DO_POSTGRESQL
    mysql_thread_init();
#endif

    const uint32 loopSleepms = 10;

    const uint32 pingEveryLoop = m_dbEngine->GetPingIntervall() / loopSleepms;

    uint32 loopCounter = 0;
    while (m_running)
    {
        // if the running state gets turned off while sleeping
        // empty the queue before exiting
        MaNGOS::Thread::Sleep(loopSleepms);

        ProcessRequests();

        if ((loopCounter++) >= pingEveryLoop)
        {
            loopCounter = 0;
            m_dbEngine->Ping();
        }
    }

#ifndef DO_POSTGRESQL
    mysql_thread_end();
#endif
}

void SqlDelayThread::Stop()
{
    m_running = false;
}

void SqlDelayThread::ProcessRequests()
{
    std::queue<std::unique_ptr<SqlOperation>> sqlQueue;

    // we need to move the contents of the queue to a local copy because executing these statements with the
    // lock in place can result in a deadlock with the world thread which calls Database::ProcessResultQueue()
    {
        std::lock_guard<std::mutex> guard(m_queueMutex);
        sqlQueue = std::move(m_sqlQueue);
    }

    while (!sqlQueue.empty())
    {
        auto const s = std::move(sqlQueue.front());
        sqlQueue.pop();
        s->Execute(m_dbConnection);
    }
}
