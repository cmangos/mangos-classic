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

#ifndef MANGOSSERVER_LFGMGR_H
#define MANGOSSERVER_LFGMGR_H

#include <list>
#include <map>

#include "Policies/Singleton.h"
#include "Common.h"
#include "LFG/LFGDefines.h"

class Group;

class LFGMgr
{
    public:
        LFGMgr() {}
        ~LFGMgr() {}

        void AddToQueue(Player* leader, uint32 queAreaID);
        void UpdateGroup(Group* group, bool join, ObjectGuid playerGuid);

        static void BuildSetQueuePacket(WorldPacket& data, uint32 areaId, uint8 status);
        static void BuildMemberAddedPacket(WorldPacket& data, ObjectGuid plrGuid);
        static void BuildInProgressPacket(WorldPacket& data);
        static void BuildCompletePacket(WorldPacket& data);

        static LfgRoles CalculateRoles(Classes playerClass);
        static LfgRoles CalculateTalentRoles(Player* player);
        static uint32 GetHighestTalentTree(Player* player);
        static std::map<uint32, int32> GetTalentTrees(Player* player);
        static LfgRolePriority GetPriority(Classes playerClass, LfgRoles playerRoles);

        static uint32 GetMaximumDPSSlots() { return 3u; }
};

#define sLFGMgr MaNGOS::Singleton<LFGMgr>::Instance()

#endif
