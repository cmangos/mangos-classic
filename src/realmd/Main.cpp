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

/// \addtogroup realmd Realm Daemon
/// @{
/// \file

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "RealmList.h"

#include "Config/Config.h"
#include "Log/Log.h"
#include "AuthSocket.h"
#include "SystemConfig.h"
#include "revision.h"
#include "revision_sql.h"
#include "Util/Util.h"
#include "Network/AsyncListener.hpp"

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/provider.h>
#include <openssl/err.h>

#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include "Platform/ServiceWin32.h"
char serviceName[] = "realmd";
char serviceLongName[] = "MaNGOS realmd service";
char serviceDescription[] = "Massive Network Game Object Server";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#else
#include "Platform/PosixDaemon.h"
#endif

bool StartDB();
void UnhookSignals();
void HookSignals();

bool stopEvent = false;                                     // Setting it to true stops the server
bool restart = false;

DatabaseType LoginDatabase;                                 // Accessor to the realm server database

boost::asio::io_context context;

// Launch the realm server
int main(int argc, char* argv[])
{
    std::string configFile, serviceParameter;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("config,c", boost::program_options::value<std::string>(&configFile)->default_value(_REALMD_CONFIG), "configuration file")
    ("version,v", "print version and exit")
#ifdef _WIN32
    ("s", boost::program_options::value<std::string>(&serviceParameter), "<run, install, uninstall> service");
#else
    ("s", boost::program_options::value<std::string>(&serviceParameter), "<run, stop> service");
#endif

    boost::program_options::variables_map vm;

    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);
    }
    catch (boost::program_options::error const& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;

        return 1;
    }

#ifdef _WIN32                                                // windows service command need execute before config read
    if (vm.count("s"))
    {
        switch (::tolower(serviceParameter[0]))
        {
            case 'i':
                if (WinServiceInstall())
                    sLog.outString("Installing service");
                return 1;
            case 'u':
                if (WinServiceUninstall())
                    sLog.outString("Uninstalling service");
                return 1;
            case 'r':
                WinServiceRun();
                break;
        }
    }
#endif

    if (!sConfig.SetSource(configFile, "Realmd_"))
    {
        sLog.outError("Could not find configuration file %s.", configFile.c_str());
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

#ifndef _WIN32                                               // posix daemon commands need apply after config read
    if (vm.count("s"))
    {
        switch (::tolower(serviceParameter[0]))
        {
            case 'r':
                startDaemon();
                break;
            case 's':
                stopDaemon();
                break;
        }
    }
#endif

    sLog.Initialize();

    sLog.outString("[%s Auth server v%s] port(%d)", _PACKAGENAME, VERSION
        , sConfig.GetIntDefault("RealmServerPort", -1));
    sLog.outString("\n\n"
        "       _____     __  __       _   _  _____  ____   _____ \n"
        "      / ____|   |  \\/  |     | \\ | |/ ____|/ __ \\ / ____|\n"
        "     | |        | \\  / |     |  \\| | |  __  |  | | (___  \n"
        "     | |ontinued| |\\/| | __ _| . ` | | |_ | |  | |\\___ \\ \n"
        "     | |____    | |  | |/ _` | |\\  | |__| | |__| |____) |\n"
        "      \\_____|   |_|  |_| (_| |_| \\_|\\_____|\\____/ \\____/ \n"
        "      http://cmangos.net\\__,_|     Doing emulation right!\n\n");

    sLog.outString("Built on %s at %s", __DATE__, __TIME__);
    sLog.outString("Built for %s", _ENDIAN_PLATFORM);
    sLog.outString("Using commit hash(%s) committed on %s", REVISION_ID, REVISION_DATE);
    sLog.outString("Using configuration file %s.", configFile.c_str());

    // Check the version of the configuration file
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION)
    {
        sLog.outError("*****************************************************************************");
        sLog.outError(" WARNING: Your realmd.conf version indicates your conf file is out of date!");
        sLog.outError("          Please check for updates, as your current default values may cause");
        sLog.outError("          strange behavior.");
        sLog.outError("*****************************************************************************");
        Log::WaitBeforeContinueIfNeed();
    }

    DETAIL_LOG("%s (Library: %s)", OPENSSL_VERSION_TEXT, OpenSSL_version(OPENSSL_VERSION));
    // Load OpenSSL 3.0+ providers
#ifdef _WIN32
    // For bundled OpenSSL library
    OSSL_PROVIDER_set_default_search_path(nullptr, boost::filesystem::current_path().string().c_str());
#endif
    OSSL_PROVIDER* openssl_legacy = OSSL_PROVIDER_load(nullptr, "legacy");
    if (!openssl_legacy)
    {
        sLog.outError("OpenSSL3: Failed to load Legacy provider: %s", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }
    OSSL_PROVIDER* openssl_default = OSSL_PROVIDER_load(nullptr, "default");
    if (!openssl_default)
    {
        sLog.outError("OpenSSL3: Failed to load Default provider: %s", ERR_error_string(ERR_get_error(), NULL));
        OSSL_PROVIDER_unload(openssl_legacy);
        return 1;
    }

    sLog.outString();
    sLog.outString("<Ctrl-C> to stop.");

    // realmd PID file creation
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

    // Initialize the database connection
    if (!StartDB())
    {
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    // Get the list of realms for the server
    sRealmList.Initialize(sConfig.GetIntDefault("RealmsStateUpdateDelay", 20));
    if (sRealmList.size() == 0)
    {
        sLog.outError("No valid realms specified.");
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    // cleanup query
    // set expired bans to inactive
    LoginDatabase.BeginTransaction();
    LoginDatabase.Execute("UPDATE account_banned SET active = 0 WHERE expires_at<=" _UNIXTIME_ " AND expires_at<>banned_at");
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE expires_at<=" _UNIXTIME_ " AND expires_at<>banned_at");
    LoginDatabase.CommitTransaction();

    uint32 networkThreadCount = sConfig.GetIntDefault("ListenerThreads", 1);
    MaNGOS::AsyncListener<AuthSocket> listener(context,
            sConfig.GetStringDefault("BindIP", "0.0.0.0"),
            sConfig.GetIntDefault("RealmServerPort", DEFAULT_REALMSERVER_PORT)
    );

    std::vector<std::thread> threads;
    for (uint32 i = 0; i < networkThreadCount; ++i)
        threads.emplace_back([&]() { context.run(); });

    // Catch termination signals
    HookSignals();

    // Handle affinity for multiple processors and process priority on Windows
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
                    sLog.outError("Processors marked in UseProcessors bitmask (hex) %x not accessible for realmd. Accessible processors bitmask (hex): %x", Aff, appAff);
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

        if (Prio)
        {
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                sLog.outString("realmd process priority class set to HIGH");
            else
                sLog.outError("Can't set realmd process priority class.");
            sLog.outString();
        }
    }
#endif

    // server has started up successfully => enable async DB requests
    LoginDatabase.AllowAsyncTransactions();

    // maximum counter for next ping
    auto const numLoops = sConfig.GetIntDefault("MaxPingTime", 30) * MINUTE * 10;
    uint32 loopCounter = 0;

#ifndef _WIN32
    detachDaemon();
#endif
    ///- Wait for termination signal
    while (!stopEvent)
    {
        if ((++loopCounter) == numLoops)
        {
            loopCounter = 0;
            DETAIL_LOG("Ping MySQL to keep connection alive");
            LoginDatabase.Ping();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#ifdef _WIN32
        if (m_ServiceStatus == 0) stopEvent = true;
        while (m_ServiceStatus == 2) Sleep(1000);
#endif
    }

    context.stop();

    for (uint32 i = 0; i < networkThreadCount; ++i)
        threads[i].join();

    // Wait for the delay thread to exit
    LoginDatabase.HaltDelayThread();

    // Remove signal handling before leaving
    UnhookSignals();

    sLog.outString("Halting process...");
    return restart ? 2 : 0; // unified exit codes with mangosd
}

// Handle termination signals
/** Put the global variable stopEvent to 'true' if a termination signal is caught **/
void OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
            stopEvent = true;
            restart = true;
            break;
        case SIGTERM:
#ifdef _WIN32
        case SIGBREAK:
#endif
            stopEvent = true;
            break;
    }

    signal(s, OnSignal);
}

// Initialize connection to the database
bool StartDB()
{
    std::string dbstring = sConfig.GetStringDefault("LoginDatabaseInfo");
    if (dbstring.empty())
    {
        sLog.outError("Database not specified");
        return false;
    }

    sLog.outString("Login Database total connections: %i", 1 + 1);

    if (!LoginDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
        return false;
    }

    if (!LoginDatabase.CheckRequiredField("realmd_db_version", REVISION_DB_REALMD))
    {
        // Wait for already started DB delay threads to end
        LoginDatabase.HaltDelayThread();
        return false;
    }

    return true;
}

// Define hook 'OnSignal' for all termination signals
void HookSignals()
{
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, OnSignal);
#endif
}

// Unhook the signals before leaving
void UnhookSignals()
{
    signal(SIGINT, nullptr);
    signal(SIGTERM, nullptr);
#ifdef _WIN32
    signal(SIGBREAK, nullptr);
#endif
}

/// @}
