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
#include "Master.h"
#include "Server/WorldSocket.h"
#include "WorldRunnable.h"
#include "World/World.h"
#include "Log.h"
#include "Util/Timer.h"
#include "SystemConfig.h"
#include "CliRunnable.h"
#include "RASocket.h"
#include "Util/Util.h"
#include "revision_sql.h"
#include "MaNGOSsoap.h"
#include "Mails/MassMailMgr.h"
#include "Server/DBCStores.h"

#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "Policies/Singleton.h"
#include "Network/Listener.hpp"
#include "Network/Socket.hpp"

#include <memory>

#ifdef _WIN32
#include "Platform/ServiceWin32.h"
extern int m_ServiceStatus;
#else
#include "Platform/PosixDaemon.h"
#endif

INSTANTIATE_SINGLETON_1(Master);

volatile bool Master::m_canBeKilled = false;

class FreezeDetectorRunnable : public MaNGOS::Runnable
{
    public:
        FreezeDetectorRunnable() { _delaytime = 0; }
        uint32 m_loops, m_lastchange;
        uint32 w_loops, w_lastchange;
        uint32 _delaytime;
        void SetDelayTime(uint32 t) { _delaytime = t; }
        void run(void)
        {
            if (!_delaytime)
                return;
            sLog.outString("Starting up anti-freeze thread (%u seconds max stuck time)...", _delaytime / 1000);
            m_loops = 0;
            w_loops = 0;
            m_lastchange = 0;
            w_lastchange = 0;
            while (!World::IsStopped())
            {
                MaNGOS::Thread::Sleep(1000);

                uint32 curtime = WorldTimer::getMSTime();

                // normal work
                if (w_loops != World::m_worldLoopCounter)
                {
                    w_lastchange = curtime;
                    w_loops = World::m_worldLoopCounter;
                }
                // possible freeze
                else if (WorldTimer::getMSTimeDiff(w_lastchange, curtime) > _delaytime)
                {
                    sLog.outError("World Thread hangs, kicking out server!");
                    *((uint32 volatile*)nullptr) = 0;          // bang crash
                }
            }
            sLog.outString("Anti-freeze thread exiting without problems.");
        }
};

/// Main function
int Master::Run()
{
    /// worldd PID file creation
    std::string pidfile = sConfig.GetStringDefault("PidFile");
    if (!pidfile.empty())
    {
        uint32 pid = CreatePIDFile(pidfile);
        if (!pid)
        {
            sLog.outError("Cannot create PID file %s.\n", pidfile.c_str());
            Log::WaitBeforeContinueIfNeed();
            return 1;
        }

        sLog.outString("Daemon PID: %u\n", pid);
    }

    ///- Start the databases
    if (!_StartDB())
    {
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    ///- Initialize the World
    sWorld.SetInitialWorldSettings();

#ifndef _WIN32
    detachDaemon();
#endif
    // server loaded successfully => enable async DB requests
    // this is done to forbid any async transactions during server startup!
    CharacterDatabase.AllowAsyncTransactions();
    WorldDatabase.AllowAsyncTransactions();
    LoginDatabase.AllowAsyncTransactions();
    LogsDatabase.AllowAsyncTransactions();

    ///- Catch termination signals
    _HookSignals();

    ///- Launch WorldRunnable thread
    MaNGOS::Thread world_thread(new WorldRunnable);
    world_thread.setPriority(MaNGOS::Priority_Highest);

    // set realmbuilds depend on mangosd expected builds, and set server online
    {
        std::string builds = AcceptableClientBuildsListStr();
        LoginDatabase.escape_string(builds);
        LoginDatabase.DirectPExecute("UPDATE realmlist SET realmflags = realmflags & ~(%u), population = 0, realmbuilds = '%s'  WHERE id = '%u'", REALM_FLAG_OFFLINE, builds.c_str(), realmID);
    }

    sWorld.StartLFGQueueThread();

    MaNGOS::Thread* cliThread = nullptr;

#ifdef _WIN32
    if (sConfig.GetBoolDefault("Console.Enable", true) && (m_ServiceStatus == -1)/* need disable console in service mode*/)
#else
    if (sConfig.GetBoolDefault("Console.Enable", true))
#endif
    {
        ///- Launch CliRunnable thread
        cliThread = new MaNGOS::Thread(new CliRunnable);
    }

    ///- Handle affinity for multiple processors and process priority on Windows
#ifdef _WIN32
    {
        HANDLE hProcess = GetCurrentProcess();

        uint32 Aff = sConfig.GetIntDefault("UseProcessors", 0);
        if (Aff > 0)
        {
            ULONG_PTR appAff;
            ULONG_PTR sysAff;

            if (GetProcessAffinityMask(hProcess, &appAff, &sysAff))
            {
                ULONG_PTR curAff = Aff & appAff;            // remove non accessible processors

                if (!curAff)
                {
                    sLog.outError("Processors marked in UseProcessors bitmask (hex) %x not accessible for mangosd. Accessible processors bitmask (hex): %x", Aff, appAff);
                }
                else
                {
                    if (SetProcessAffinityMask(hProcess, curAff))
                        sLog.outString("Using processors (bitmask, hex): %x", curAff);
                    else
                        sLog.outError("Can't set used processors (hex): %x", curAff);
                }
            }
            sLog.outString();
        }

        bool Prio = sConfig.GetBoolDefault("ProcessPriority", false);

//        if(Prio && (m_ServiceStatus == -1)/* need set to default process priority class in service mode*/)
        if (Prio)
        {
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                sLog.outString("mangosd process priority class set to HIGH");
            else
                sLog.outError("Can't set mangosd process priority class.");
            sLog.outString();
        }
    }
#endif

    ///- Start up freeze catcher thread
    MaNGOS::Thread* freeze_thread = nullptr;
    if (uint32 freeze_delay = sConfig.GetIntDefault("MaxCoreStuckTime", 0))
    {
        FreezeDetectorRunnable* fdr = new FreezeDetectorRunnable();
        fdr->SetDelayTime(freeze_delay * 1000);
        freeze_thread = new MaNGOS::Thread(fdr);
        freeze_thread->setPriority(MaNGOS::Priority_Highest);
    }

    {
        int32 networkThreadWorker = sConfig.GetIntDefault("Network.Threads", 1);
        if (networkThreadWorker <= 0)
        {
            sLog.outError("Invalid network thread workers setting in mangosd.conf. (%d) should be > 0", networkThreadWorker);
            networkThreadWorker = 1;
        }
        MaNGOS::Listener<WorldSocket> listener(sConfig.GetStringDefault("BindIP", "0.0.0.0"), int32(sWorld.getConfig(CONFIG_UINT32_PORT_WORLD)), networkThreadWorker);

        std::unique_ptr<MaNGOS::Listener<RASocket>> raListener;
        if (sConfig.GetBoolDefault("Ra.Enable", false))
            raListener.reset(new MaNGOS::Listener<RASocket>(sConfig.GetStringDefault("Ra.IP", "0.0.0.0"), sConfig.GetIntDefault("Ra.Port", 3443), 1));

        std::unique_ptr<SOAPThread> soapThread;
        if (sConfig.GetBoolDefault("SOAP.Enabled", false))
            soapThread.reset(new SOAPThread(sConfig.GetStringDefault("SOAP.IP", "127.0.0.1"), sConfig.GetIntDefault("SOAP.Port", 7878)));

        // wait for shut down and then let things go out of scope to close them down
        while (!World::IsStopped())
            std::this_thread::sleep_for(std::chrono::seconds(1));

        world_thread.wait();
    }

    ///- Stop freeze protection before shutdown tasks
    if (freeze_thread)
    {
        freeze_thread->destroy();
        delete freeze_thread;
    }

    ///- Set server offline in realmlist
    LoginDatabase.DirectPExecute("UPDATE realmlist SET realmflags = realmflags | %u WHERE id = '%u'", REALM_FLAG_OFFLINE, realmID);

    ///- Remove signal handling before leaving
    _UnhookSignals();

    // when the main thread closes the singletons get unloaded
    // since worldrunnable uses them, it will crash if unloaded after master
    world_thread.wait();

    ///- Clean account database before leaving
    clearOnlineAccounts();

    // send all still queued mass mails (before DB connections shutdown)
    sMassMailMgr.Update(true);

    ///- Wait for DB delay threads to end
    CharacterDatabase.HaltDelayThread();
    WorldDatabase.HaltDelayThread();
    LoginDatabase.HaltDelayThread();
    LogsDatabase.HaltDelayThread();

    sLog.outString("Halting process...");

    if (cliThread)
    {
#ifdef _WIN32
        // send keyboard input to safely unblock the CLI thread that is waiting for an user input
        HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
        INPUT_RECORD ir[2] = { 0 };

        ir[0].EventType = KEY_EVENT;
        ir[0].Event.KeyEvent.bKeyDown = TRUE;
        ir[0].Event.KeyEvent.wRepeatCount = 1;
        ir[0].Event.KeyEvent.uChar.AsciiChar = VK_RETURN;
        ir[0].Event.KeyEvent.dwControlKeyState = 0;

        ir[1] = ir[0];
        ir[1].Event.KeyEvent.bKeyDown = FALSE;

        DWORD dwNumWrtn = -1;
        WriteConsoleInput(hStdIn, ir, 2, &dwNumWrtn);

        cliThread->wait();
#else

        cliThread->destroy();

#endif

        delete cliThread;
    }

    // mark this can be killable
    m_canBeKilled = true;

    ///- Exit the process with specified return value
    return World::GetExitCode();
}

/// Initialize connection to the databases
bool Master::_StartDB()
{
    ///- Get world database info from configuration file
    std::string dbstring = sConfig.GetStringDefault("WorldDatabaseInfo");
    int nConnections = sConfig.GetIntDefault("WorldDatabaseConnections", 1);
    if (dbstring.empty())
    {
        sLog.outError("Database not specified in configuration file");
        return false;
    }
    sLog.outString("World Database total connections: %i", nConnections + 1);

    ///- Initialise the world database
    if (!WorldDatabase.Initialize(dbstring.c_str(), nConnections))
    {
        sLog.outError("Cannot connect to world database %s", dbstring.c_str());
        return false;
    }

    if (!WorldDatabase.CheckRequiredField("db_version", REVISION_DB_MANGOS))
    {
        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        return false;
    }

    dbstring = sConfig.GetStringDefault("CharacterDatabaseInfo");
    nConnections = sConfig.GetIntDefault("CharacterDatabaseConnections", 1);
    if (dbstring.empty())
    {
        sLog.outError("Character Database not specified in configuration file");

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        return false;
    }
    sLog.outString("Character Database total connections: %i", nConnections + 1);

    ///- Initialise the Character database
    if (!CharacterDatabase.Initialize(dbstring.c_str(), nConnections))
    {
        sLog.outError("Cannot connect to Character database %s", dbstring.c_str());

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        return false;
    }

    if (!CharacterDatabase.CheckRequiredField("character_db_version", REVISION_DB_CHARACTERS))
    {
        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        return false;
    }

    ///- Get login database info from configuration file
    dbstring = sConfig.GetStringDefault("LoginDatabaseInfo");
    nConnections = sConfig.GetIntDefault("LoginDatabaseConnections", 1);
    if (dbstring.empty())
    {
        sLog.outError("Login database not specified in configuration file");

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        return false;
    }

    ///- Initialise the login database
    sLog.outString("Login Database total connections: %i", nConnections + 1);
    if (!LoginDatabase.Initialize(dbstring.c_str(), nConnections))
    {
        sLog.outError("Cannot connect to login database %s", dbstring.c_str());

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        return false;
    }

    if (!LoginDatabase.CheckRequiredField("realmd_db_version", REVISION_DB_REALMD))
    {
        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        LoginDatabase.HaltDelayThread();
        return false;
    }

    ///- Get logs database info from configuration file
    dbstring = sConfig.GetStringDefault("LogsDatabaseInfo", "");
    nConnections = sConfig.GetIntDefault("LogsDatabaseConnections", 1);
    if (dbstring.empty())
    {
        sLog.outError("logs database not specified in configuration file");

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        LoginDatabase.HaltDelayThread();
        return false;
    }

    ///- Initialise the logs database
    sLog.outString("Logs Database total connections: %i", nConnections + 1);
    if (!LogsDatabase.Initialize(dbstring.c_str(), nConnections))
    {
        sLog.outError("Cannot connect to logs database %s", dbstring.c_str());

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        LoginDatabase.HaltDelayThread();
        return false;
    }

    if (!LogsDatabase.CheckRequiredField("logs_db_version", REVISION_DB_LOGS))
    {
        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        LoginDatabase.HaltDelayThread();
        return false;
    }

    sLog.outString();

    ///- Get the realm Id from the configuration file
    realmID = sConfig.GetIntDefault("RealmID", 0);
    if (!realmID)
    {
        sLog.outError("Realm ID not defined in configuration file");

        ///- Wait for already started DB delay threads to end
        WorldDatabase.HaltDelayThread();
        CharacterDatabase.HaltDelayThread();
        LoginDatabase.HaltDelayThread();
        return false;
    }

    sLog.outString("Realm running as realm ID %d", realmID);
    sLog.outString();

    ///- Clean the database before starting
    clearOnlineAccounts();

    sWorld.LoadDBVersion();

    sLog.outString("Using World DB: %s", sWorld.GetDBVersion());
    sLog.outString("Using creature EventAI: %s", sWorld.GetCreatureEventAIVersion());
    sLog.outString();
    return true;
}

/// Clear 'online' status for all accounts with characters in this realm
void Master::clearOnlineAccounts()
{
    // Cleanup online status for characters hosted at current realm
    /// \todo Only accounts with characters logged on *this* realm should have online status reset. Move the online column from 'account' to 'realmcharacters'?
    LoginDatabase.PExecute("UPDATE account SET active_realm_id = 0 WHERE active_realm_id = '%u'", realmID);

    CharacterDatabase.Execute("UPDATE characters SET online = 0 WHERE online<>0");

    // Battleground instance ids reset at server restart
    CharacterDatabase.Execute("UPDATE character_battleground_data SET instance_id = 0");
}

/// Handle termination signals
void Master::_OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
            World::StopNow(RESTART_EXIT_CODE);
            break;
        case SIGTERM:
#ifdef _WIN32
        case SIGBREAK:
#endif
            World::StopNow(SHUTDOWN_EXIT_CODE);
            break;
    }

    // give a 30 sec timeout in case of Master cannot finish properly
    int32 timeOut = 200;
    while (!m_canBeKilled && timeOut > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        --timeOut;
    }

    signal(s, _OnSignal);
}

/// Define hook '_OnSignal' for all termination signals
void Master::_HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
#endif
}

/// Unhook the signals before leaving
void Master::_UnhookSignals()
{
    signal(SIGINT, nullptr);
    signal(SIGTERM, nullptr);
#ifdef _WIN32
    signal(SIGBREAK, nullptr);
#endif
}
