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

/** \file
    \ingroup mangosd
*/

#include "Common.h"
#include "World/World.h"
#include "WorldRunnable.h"
#include "Util/Timer.h"
#include "Globals/ObjectAccessor.h"
#include "Maps/MapManager.h"

#include "Database/DatabaseEnv.h"

#define WORLD_SLEEP_CONST 50

#ifdef _WIN32
#include "Platform/ServiceWin32.h"
extern int m_ServiceStatus;
#endif

/// Heartbeat for the World
void WorldRunnable::run()
{
    ///- Init new SQL thread for the world database
    WorldDatabase.ThreadStart();                            // let thread do safe mySQL requests (one connection call enough)
    sWorld.InitResultQueue();

    uint32 diffTick = WorldTimer::tick(); // initialize world timer vars
    uint32 diffTime = 0; // used to compute real time elapsed in World::Update()
    uint32 overCounter = 0; // count overtime loops

    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;

        diffTick = WorldTimer::tick();
        sWorld.Update(diffTick);
        diffTime = WorldTimer::getMSTime() - WorldTimer::tickTime();

        // we have to wait WORLD_SLEEP_CONST max between loops
        // don't wait if over
        if (diffTime < WORLD_SLEEP_CONST)
        {
            MaNGOS::Thread::Sleep(WORLD_SLEEP_CONST - diffTime);
        }
#ifdef MANGOS_DEBUG
        else
        {
            ++overCounter;
            sLog.outString("WorldRunnable:run Long loop #%d : %dms (total : %d loop(s), %.3f%%)", World::m_worldLoopCounter, diffTime, overCounter, (float)(100*overCounter) / (float)World::m_worldLoopCounter);
        }
#endif

#ifdef _WIN32
        if (m_ServiceStatus == 0) World::StopNow(SHUTDOWN_EXIT_CODE);
        while (m_ServiceStatus == 2) Sleep(1000);
#endif
    }

    sWorld.CleanupsBeforeStop();

    ///- End the database thread
    WorldDatabase.ThreadEnd();                              // free mySQL thread resources
}
