/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDENMODULEMGR_HPP_
#define __WARDENMODULEMGR_HPP_

#include "WardenModule.hpp"

#include "Policies/Singleton.h"
#include "Platform/Define.h"

#include <vector>

class WardenModuleMgr
{
private:
    std::vector<WardenModule> _winModules;
    std::vector<WardenModule> _macModules;

    bool _loaded;

public:
    WardenModuleMgr() : _loaded(false) {}

    // reads the modules from Warden.ModuleDir, only the first time it is called: each Warden keeps a pointer to its module
    void Load();

    bool HasWindowsModule() const { return !_winModules.empty(); }
    bool HasMacModule() const { return !_macModules.empty(); }

    const WardenModule *GetWindowsModule() const;
    const WardenModule *GetMacModule() const;
};

#define sWardenModuleMgr MaNGOS::Singleton<WardenModuleMgr>::Instance()

#endif /* !__WARDENMODULEMGR_HPP_ */