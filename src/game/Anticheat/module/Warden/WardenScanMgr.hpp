/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDENSCANMGR_HPP_
#define __WARDENSCANMGR_HPP_

#include "WardenScan.hpp"
#include "Policies/Singleton.h"

#include <vector>
#include <memory>

class WardenScanMgr
{
    private:
        // these are stored as shared pointers to allow this collection to be emptied (presumably in the
        // process of repopulating it) without invalidating pointers held elsewhere, namely in the queues
        // of existing clients
        std::vector<std::shared_ptr<const Scan>> m_scans;

    public:
        // load static scans from database
        void loadFromDB();

        size_t Count() const { return m_scans.size(); }

        void AddMacScan(const MacScan *);
        void AddMacScan(std::shared_ptr<MacScan>);

        void AddWindowsScan(const WindowsScan *);
        void AddWindowsScan(std::shared_ptr<WindowsScan>);

        std::vector<std::shared_ptr<const Scan>> GetRandomScans(ScanFlags flags) const;
};

#define sWardenScanMgr MaNGOS::Singleton<WardenScanMgr>::Instance()

#endif /*!__WARDENSCANMGR_HPP_*/