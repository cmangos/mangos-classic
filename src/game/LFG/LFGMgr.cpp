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

#include "Common.h"
#include "Util/ProgressBar.h"
#include "Globals/SharedDefines.h"
#include "Entities/Player.h"
#include "Maps/Map.h"
#include "Globals/ObjectMgr.h"
#include "Chat/Chat.h"
#include "World/World.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Tools/Language.h"
#include "Groups/Group.h"
#include "LFG/LFGMgr.h"
#include "LFG/LFGQueue.h"

INSTANTIATE_SINGLETON_1(LFGMgr);

// Add group or player into queue. If player has group and he's a leader then whole party will be added to queue.
void LFGMgr::AddToQueue(Player* leader, uint32 queueAreaID)
{
    if (!leader)
        return;

    Group* grp = leader->GetGroup();

    //add players from group to queue list & group to group list
    // Calculate what roles group need and add it to the GroupQueueList, (DONT'ADD PLAYERS!)
    if (grp && grp->IsLeader(leader->GetObjectGuid()))
    {
        // Add group to queued groups list
        LFGGroupQueueInfo groupInfo;

        grp->CalculateLFGRoles(groupInfo);
        groupInfo.team = leader->GetTeam();
        groupInfo.areaId = queueAreaID;
        groupInfo.playerCount = grp->GetMembersCount();
        groupInfo.groupTimer = 5 * MINUTE * IN_MILLISECONDS; // Minute timer for SMSG_MEETINGSTONE_IN_PROGRESS

        grp->SetLFGAreaId(queueAreaID);

        sWorld.GetLFGQueue().GetMessager().AddMessage([groupInfo, groupId = grp->GetId()](LFGQueue* queue)
        {
            queue->AddGroup(groupInfo, groupId);
        });
    }
    else if (!grp)
    {
        // Add player to queued players list
        LFGPlayerQueueInfo playerInfo;

        playerInfo.team = leader->GetTeam();
        playerInfo.areaId = queueAreaID;
#ifdef ENABLE_PLAYERBOTS
        playerInfo.hasQueuePriority = !leader->GetPlayerbotAI();
#else
        playerInfo.hasQueuePriority = false;
#endif
        playerInfo.playerClass = leader->getClass();
        if (sWorld.getConfig(CONFIG_BOOL_LFG_MATCHMAKING))
            playerInfo.CalculateTalentRoles(leader);
        else
            playerInfo.CalculateRoles((Classes)leader->getClass());
        playerInfo.name = leader->GetName();
        leader->SetLFGAreaId(queueAreaID);

        sWorld.GetLFGQueue().GetMessager().AddMessage([playerInfo, playerGuid = leader->GetObjectGuid()](LFGQueue* queue)
        {
            queue->AddPlayer(playerInfo, playerGuid);
        });
    }
}

LfgRoles LFGMgr::CalculateRoles(Classes playerClass)
{
    switch (playerClass)
    {
        case CLASS_DRUID:   return (LfgRoles)(PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE | PLAYER_ROLE_HEALER);
        case CLASS_HUNTER:  return (LfgRoles)(PLAYER_ROLE_DAMAGE);
        case CLASS_MAGE:    return (LfgRoles)(PLAYER_ROLE_DAMAGE);
        case CLASS_PALADIN: return (LfgRoles)(PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE | PLAYER_ROLE_HEALER);
        case CLASS_PRIEST:  return (LfgRoles)(PLAYER_ROLE_DAMAGE | PLAYER_ROLE_HEALER);
        case CLASS_ROGUE:   return (LfgRoles)(PLAYER_ROLE_DAMAGE);
        case CLASS_SHAMAN:  return (LfgRoles)(PLAYER_ROLE_DAMAGE | PLAYER_ROLE_HEALER);
        case CLASS_WARLOCK: return (LfgRoles)(PLAYER_ROLE_DAMAGE);
        case CLASS_WARRIOR: return (LfgRoles)(PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE);
        default:            return (LfgRoles)(PLAYER_ROLE_NONE);
    }
}

LfgRolePriority LFGMgr::GetPriority(Classes playerClass, LfgRoles playerRoles)
{
    switch (playerRoles)
    {
        case PLAYER_ROLE_TANK:
        {
            switch (playerClass)
            {
                case CLASS_DRUID:   return LFG_PRIORITY_NORMAL;
                case CLASS_PALADIN: return LFG_PRIORITY_NORMAL;
                case CLASS_WARRIOR: return LFG_PRIORITY_HIGH;
                default:            return LFG_PRIORITY_NONE;
            }
            break;
        }

        case PLAYER_ROLE_HEALER:
        {
            switch (playerClass)
            {
                case CLASS_DRUID:   return LFG_PRIORITY_HIGH;
                case CLASS_PALADIN: return LFG_PRIORITY_HIGH;
                case CLASS_PRIEST:  return LFG_PRIORITY_HIGH;
                case CLASS_SHAMAN:  return LFG_PRIORITY_HIGH;
                default:            return LFG_PRIORITY_NONE;
            }

            break;
        }

        case PLAYER_ROLE_DAMAGE:
        {
            switch (playerClass)
            {
                case CLASS_DRUID:   return LFG_PRIORITY_NORMAL;
                case CLASS_HUNTER:  return LFG_PRIORITY_HIGH;
                case CLASS_MAGE:    return LFG_PRIORITY_HIGH;
                case CLASS_PALADIN: return LFG_PRIORITY_NORMAL;
                case CLASS_PRIEST:  return LFG_PRIORITY_LOW;
                case CLASS_ROGUE:   return LFG_PRIORITY_HIGH;
                case CLASS_SHAMAN:  return LFG_PRIORITY_NORMAL;
                case CLASS_WARLOCK: return LFG_PRIORITY_HIGH;
                case CLASS_WARRIOR: return LFG_PRIORITY_NORMAL;
                default:            return LFG_PRIORITY_NONE;
            }

            break;
        }
        default:                    return LFG_PRIORITY_NONE;
    }
}

std::map<uint32, int32> LFGMgr::GetTalentTrees(Player* _player)
{
    std::map<uint32, int32> tabs;
    for (uint32 i = 0; i < uint32(3); i++)
        tabs[i] = 0;

    uint32 classMask = _player->getClassMask();
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        int maxRank = 0;
        for (int rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            if (!talentInfo->RankID[rank])
                continue;

            uint32 spellid = talentInfo->RankID[rank];
            if (spellid && _player->HasSpell(spellid))
                maxRank = rank + 1;

        }
        tabs[talentTabInfo->tabpage] += maxRank;
    }

    return tabs;
}

uint32 LFGMgr::GetHighestTalentTree(Player* _player)
{
    if (_player->GetLevel() >= 10)
    {
        std::map<uint32, int32> tabs = GetTalentTrees(_player);

        uint32 tab = 4;
        int32 max = 0;
        for (uint32 i = 0; i < uint32(3); i++)
        {
            if (tab == 4 || max < tabs[i])
            {
                tab = i;
                max = tabs[i];
            }
        }
        if (tab != 4)
            return tab;
    }

    uint32 tab = 0;

    // if no talents picked, use standard values
    switch (_player->getClass())
    {
        case CLASS_SHAMAN: tab = 2; break;
        case CLASS_PRIEST: tab = 1; break;
        case CLASS_DRUID:  tab = 2; break;
        case CLASS_WARRIOR: tab = 2; break;
    }
    return tab;
}

LfgRoles LFGMgr::CalculateTalentRoles(Player* _player)
{
    LfgRoles role = PLAYER_ROLE_NONE;
    uint32 tab = GetHighestTalentTree(_player);
    switch (_player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
                role = PLAYER_ROLE_DAMAGE;
            else
                role = PLAYER_ROLE_HEALER;
            break;
        case CLASS_SHAMAN:
            if (tab == 2)
                role = PLAYER_ROLE_HEALER;
            else
                role = PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                role = PLAYER_ROLE_TANK;
            else
                role = PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_PALADIN:
            if (tab == 0)
                role = PLAYER_ROLE_HEALER;
            else if (tab == 1)
                role = PLAYER_ROLE_TANK;
            else if (tab == 2)
                role = PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_DRUID:
            if (tab == 0)
                role = PLAYER_ROLE_DAMAGE;
            else if (tab == 1)
                role = LfgRoles(PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE);
            else if (tab == 2)
                role = PLAYER_ROLE_HEALER;
            break;
        default:
            role = PLAYER_ROLE_DAMAGE;
            break;
    }
    return role;
}

void LFGMgr::UpdateGroup(Group* group, bool join, ObjectGuid playerGuid)
{
    LFGGroupQueueInfo groupInfo;
    if (!group->IsFull())
        group->CalculateLFGRoles(groupInfo);
    sWorld.GetLFGQueue().GetMessager().AddMessage([groupInfo, join, playerGuid, groupId = group->GetId()](LFGQueue* queue)
    {
        queue->UpdateGroup(groupInfo, join, playerGuid, groupId);
    });
}

void LFGMgr::BuildSetQueuePacket(WorldPacket& data, uint32 areaId, uint8 status)
{
    data.Initialize(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << uint32(areaId);
    data << uint8(status);
}

void LFGMgr::BuildMemberAddedPacket(WorldPacket& data, ObjectGuid plrGuid)
{
    data.Initialize(SMSG_MEETINGSTONE_MEMBER_ADDED, 8);
    data << uint64(plrGuid);
}

void LFGMgr::BuildInProgressPacket(WorldPacket& data)
{
    data.Initialize(SMSG_MEETINGSTONE_IN_PROGRESS, 0);
}

void LFGMgr::BuildCompletePacket(WorldPacket& data)
{
    data.Initialize(SMSG_MEETINGSTONE_COMPLETE, 0);
}
