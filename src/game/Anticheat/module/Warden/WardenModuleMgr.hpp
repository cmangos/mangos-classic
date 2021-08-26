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

public:
    WardenModuleMgr();

    const WardenModule *GetWindowsModule() const;
    const WardenModule *GetMacModule() const;
};

#define sWardenModuleMgr MaNGOS::Singleton<WardenModuleMgr>::Instance()

#endif /* !__WARDENMODULEMGR_HPP_ */