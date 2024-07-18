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
#include "Util/ProgressBar.h"
#include "Log/Log.h"
#include "Master.h"
#include "SystemConfig.h"
#include "AuctionHouseBot/AuctionHouseBot.h"
#include "PlayerBot/config.h"

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/provider.h>
#include <openssl/err.h>

#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

#ifdef _WIN32
#include "Platform/ServiceWin32.h"
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
#include "Platform/PosixDaemon.h"
#endif

DatabaseType WorldDatabase;                                 ///< Accessor to the world database
DatabaseType CharacterDatabase;                             ///< Accessor to the character database
DatabaseType LoginDatabase;                                 ///< Accessor to the realm/login database
DatabaseType LogsDatabase;                                  ///< Accessor to the logs database

uint32 realmID;                                             ///< Id of the realm

/// Launch the mangos server
int main(int argc, char* argv[])
{
    std::string auctionBotConfig, configFile, playerBotConfig, serviceParameter;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("ahbot,a", boost::program_options::value<std::string>(&auctionBotConfig), "ahbot configuration file")
    ("config,c", boost::program_options::value<std::string>(&configFile)->default_value(_MANGOSD_CONFIG), "configuration file")
#ifdef BUILD_DEPRECATED_PLAYERBOT
    ("playerbot,p", boost::program_options::value<std::string>(&playerBotConfig)->default_value(_D_PLAYERBOT_CONFIG), "playerbot configuration file")
#endif
    ("help,h", "prints usage")
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

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        if (vm.count("version"))
        {
            std::cout << _FULLVERSION(REVISION_DATE, REVISION_ID) << std::endl;
            std::cout << "Boost version " << (BOOST_VERSION / 10000) << "." << ((BOOST_VERSION / 100) % 1000) << "." << (BOOST_VERSION % 100) << std::endl;
            return 0;
        }
    }
    catch (boost::program_options::error const& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;

        return 1;
    }

    if (vm.count("ahbot"))
        sAuctionHouseBot.SetConfigFileName(auctionBotConfig);

#ifdef BUILD_DEPRECATED_PLAYERBOT
    if (vm.count("playerbot"))
        _PLAYERBOT_CONFIG = playerBotConfig;
#endif

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

    if (!sConfig.SetSource(configFile, "Mangosd_"))
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

    sLog.outString("[%s Classic World server v%s] id(%d) port(%d)", _PACKAGENAME, VERSION
        , sConfig.GetIntDefault("RealmID", -1), sConfig.GetIntDefault("WorldServerPort", -1));
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

    DETAIL_LOG("Using Boost: %s", BOOST_LIB_VERSION);

    sLog.outString();
    sLog.outString("<Ctrl-C> to stop.");

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
