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

/// \addtogroup mangosd Mangos Daemon
/// @{
/// \file

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Config/Config.h"
#include "ProgressBar.h"
#include "Log.h"
#include "Master.h"
#include "SystemConfig.h"
#include "AuctionHouseBot/AuctionHouseBot.h"
#include "revision.h"

#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include <boost/program_options.hpp>
#include <boost/version.hpp>

#include <iostream>
#include <string>

#ifdef WIN32
#include "ServiceWin32.h"
char serviceName[] = "mangosd";
char serviceLongName[] = "MaNGOS world service";
char serviceDescription[] = "Massive Network Game Object Server";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#else
#include "PosixDaemon.h"
#endif

DatabaseType WorldDatabase;                                 ///< Accessor to the world database
DatabaseType CharacterDatabase;                             ///< Accessor to the character database
DatabaseType LoginDatabase;                                 ///< Accessor to the realm/login database

uint32 realmID;                                             ///< Id of the realm

/// Print out the usage string for this program on the console.
void usage(const char* prog)
{
    sLog.outString("Usage: \n %s [<options>]\n"
                   "    -v, --version            print version and exist\n\r"
                   "    -c config_file           use config_file as configuration file\n\r"
                   "    -a, --ahbot config_file  use config_file as ahbot configuration file\n\r"
#ifdef WIN32
                   "    Running as service functions:\n\r"
                   "    -s run                   run as service\n\r"
                   "    -s install               install service\n\r"
                   "    -s uninstall             uninstall service\n\r"
#else
                   "    Running as daemon functions:\n\r"
                   "    -s run                   run as daemon\n\r"
                   "    -s stop                  stop daemon\n\r"
#endif
                   , prog);
}

/// Launch the mangos server
int main(int argc, char *argv[])
{
    std::string auctionBotConfig, configFile, serviceParameter;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("ahbot,a", boost::program_options::value<std::string>(&auctionBotConfig), "ahbot configuration file")
        ("config,c", boost::program_options::value<std::string>(&configFile)->default_value(_MANGOSD_CONFIG), "configuration file")
        ("help,h", "prints usage")
        ("version,v", "print version and exit")
#ifdef WIN32
        ("s", boost::program_options::value<std::string>(&serviceParameter), "<run, install, uninstall> service");
#else
        ("s", boost::program_options::value<std::string>(&serviceParameter), "<run, stop> service");
#endif

    boost::program_options::variables_map vm;

    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        if (vm.count("version"))
        {
            std::cout << _FULLVERSION(REVISION_DATE, REVISION_TIME, REVISION_ID) << std::endl;
            std::cout << "Boost version " << (BOOST_VERSION / 10000) << "." << ((BOOST_VERSION / 100) % 1000) << "." << (BOOST_VERSION % 100) << std::endl;
            return 0;
        }
    }
    catch (boost::program_options::error const &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;

        return 1;
    }

    if (vm.count("ahbot"))
        sAuctionBotConfig.SetConfigFileName(auctionBotConfig);

#ifdef WIN32                                                // windows service command need execute before config read
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

    if (!sConfig.SetSource(configFile))
    {
        sLog.outError("Could not find configuration file %s.", configFile.c_str());
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

#ifndef WIN32                                               // posix daemon commands need apply after config read
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

    sLog.outString("%s [world-daemon]", _FULLVERSION(REVISION_DATE, REVISION_TIME, REVISION_ID));
    sLog.outString("<Ctrl-C> to stop.");
    sLog.outString("\n\n"
                   "       _____     __  __       _   _  _____  ____   _____ \n"
                   "      / ____|   |  \\/  |     | \\ | |/ ____|/ __ \\ / ____|\n"
                   "     | |        | \\  / |     |  \\| | |  __  |  | | (___  \n"
                   "     | |ontinued| |\\/| | __ _| . ` | | |_ | |  | |\\___ \\ \n"
                   "     | |____    | |  | |/ _` | |\\  | |__| | |__| |____) |\n"
                   "      \\_____|   |_|  |_| (_| |_| \\_|\\_____|\\____/ \\____/ \n"
                   "      http://cmangos.net\\__,_|     Doing things right!\n\n");

    sLog.outString("Using configuration file %s.", configFile.c_str());

    DETAIL_LOG("%s (Library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    if (SSLeay() < 0x009080bfL)
    {
        DETAIL_LOG("WARNING: Outdated version of OpenSSL lib. Logins to server may not work!");
        DETAIL_LOG("WARNING: Minimal required version [OpenSSL 0.9.8k]");
    }

    DETAIL_LOG("Using Boost: %s", BOOST_LIB_VERSION);

    ///- Set progress bars show mode
    BarGoLink::SetOutputState(sConfig.GetBoolDefault("ShowProgressBars", true));

    ///- and run the 'Master'
    /// \todo Why do we need this 'Master'? Can't all of this be in the Main as for Realmd?
    return sMaster.Run();

    // at sMaster return function exist with codes
    // 0 - normal shutdown
    // 1 - shutdown at error
    // 2 - restart command used, this code can be used by restarter for restart mangosd
}

/// @}
