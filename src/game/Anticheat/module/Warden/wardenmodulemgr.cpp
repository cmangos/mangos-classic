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
#include "Util.h"
#include "Log.h"

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

WardenModuleMgr::WardenModuleMgr()
{
    auto const moduleDir = sAnticheatConfig.GetWardenModuleDirectory();
    auto const modules = GetModuleNames(moduleDir);

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
        catch (const std::runtime_error &)
        {
            continue;
        }
    }

    sLog.outBasic(">> %lu Windows Warden modules and %lu Mac Warden modules loaded", uint64(_winModules.size()), uint64(_macModules.size()));

    if (_winModules.empty() && _macModules.empty())
        throw std::runtime_error("Failed to locate Warden modules");
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
