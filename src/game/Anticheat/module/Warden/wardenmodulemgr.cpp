/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "WardenModuleMgr.hpp"
#include "WardenModule.hpp"
#include "../config.hpp"

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Util/Util.h"
#include "Log/Log.h"

#include <vector>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>

INSTANTIATE_SINGLETON_1(WardenModuleMgr);

namespace
{
    std::vector<std::string> GetModuleNames(const std::string &moduleDir)
    {
        std::vector<std::string> results;

        if (boost::filesystem::exists(moduleDir))
        {
            // look only for .bin files, and assume (for now) that the corresponding .key and .cr files exist
            for (const auto& entry : boost::filesystem::directory_iterator(moduleDir))
                if (entry.path().extension() == ".bin")
                    results.emplace_back(moduleDir + "/" + entry.path().filename().string());
        }

        return results;
    }
}

void WardenModuleMgr::Load()
{
    if (_loaded)
        return;

    _loaded = true;

    sLog.outString("Loading Warden modules...");

    auto const moduleDir = sAnticheatConfig.GetWardenModuleDirectory();
    std::vector<std::string> modules;

    try
    {
        modules = GetModuleNames(moduleDir);
    }
    catch (const std::runtime_error &e)
    {
        sLog.outError("[Anticheat] Cannot read Warden module directory '%s' (Warden.ModuleDir): %s", moduleDir.c_str(), e.what());
    }

    for (auto const &mod : modules)
    {
        auto const key = mod.substr(0, mod.length() - 3) + "key";
        auto const cr = mod.substr(0, mod.length() - 3) + "cr";

        try
        {
            auto newMod = WardenModule(mod, key, cr);

            if (newMod.Windows())
                _winModules.emplace_back(std::move(newMod));
            else
                _macModules.emplace_back(std::move(newMod));
        }
        catch (const std::runtime_error &e)
        {
            sLog.outError("[Anticheat] Skipping Warden module %s: %s", mod.c_str(), e.what());
            continue;
        }
    }

    sLog.outBasic(">> %lu Windows Warden modules and %lu Mac Warden modules loaded", uint64(_winModules.size()), uint64(_macModules.size()));

    if (_winModules.empty())
        sLog.outError("[Anticheat] No Windows Warden module in '%s' (Warden.ModuleDir).  Windows clients run without Warden.", moduleDir.c_str());

    if (_macModules.empty())
        sLog.outError("[Anticheat] No Mac Warden module in '%s' (Warden.ModuleDir).  Mac x86 clients run without Warden.", moduleDir.c_str());
}

const WardenModule *WardenModuleMgr::GetWindowsModule() const
{
    MANGOS_ASSERT(!_winModules.empty());

    return &_winModules[urand(0, _winModules.size() - 1)];
}

const WardenModule *WardenModuleMgr::GetMacModule() const
{
    MANGOS_ASSERT(!_macModules.empty());

    return &_macModules[urand(0, _macModules.size() - 1)];
}
