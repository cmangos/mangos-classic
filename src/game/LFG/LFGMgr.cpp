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
#include "ProgressBar.h"
#include "Globals/SharedDefines.h"
#include "Entities/Player.h"
#include "Maps/Map.h"
#include "Globals/ObjectMgr.h"
#include "Chat/Chat.h"
#include "World/World.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "Tools/Language.h"
#include "Groups/Group.h"
#include "LFGMgr.h"
#include "LFGHandler.h"

INSTANTIATE_SINGLETON_1(LFGQueue);

std::array<LfgRoles, 3> PotentialRoles =
{
    LFG_ROLE_TANK,
    LFG_ROLE_HEALER,
    LFG_ROLE_DPS
};

void LFGPlayerQueueInfo::CalculateRoles(Classes playerClass)
{
    roleMask = LFGQueue::CalculateRoles(playerClass);

    // Determine role priority
    for (LfgRoles role : PotentialRoles)
    {
        rolePriority.emplace_back(std::pair<LfgRoles, LfgRolePriority>(role, LFGQueue::GetPriority(playerClass, role)));
    }
}

void LFGPlayerQueueInfo::CalculateTalentRoles(Player* player)
{
    roleMask = LFGQueue::CalculateTalentRoles(player);

    // Determine role priority
    for (LfgRoles role : PotentialRoles)
    {
        rolePriority.emplace_back(std::pair<LfgRoles, LfgRolePriority>(role, LFGQueue::GetPriority(Classes(player->getClass()), role)));
    }
}

LfgRolePriority LFGPlayerQueueInfo::GetRolePriority(LfgRoles role)
{
    for (const auto& iter : rolePriority)
    {
        if (iter.first == role)
            return iter.second;
    }

    return LFG_PRIORITY_NONE;
}

// Add group or player into queue. If player has group and he's a leader then whole party will be added to queue.
void LFGQueue::AddToQueue(Player* leader, uint32 queueAreaID)
{
    if (!leader)
        return;

    Group* grp = leader->GetGroup();

    //add players from group to queue list & group to group list
    // Calculate what roles group need and add it to the GroupQueueList, (DONT'ADD PLAYERS!)
    if (grp && grp->IsLeader(leader->GetObjectGuid()))
    {
        // Add group to queued groups list
        LFGGroupQueueInfo& i_Group = m_queuedGroups[grp->GetId()];

        grp->CalculateLFGRoles(i_Group);
        i_Group.team = leader->GetTeam();
        i_Group.areaId = queueAreaID;
        i_Group.groupTimer = 5 * MINUTE * IN_MILLISECONDS; // Minute timer for SMSG_MEETINGSTONE_IN_PROGRESS

        grp->SetLFGAreaId(queueAreaID);

        WorldPacket data;
        BuildSetQueuePacket(data, queueAreaID, MEETINGSTONE_STATUS_JOINED_QUEUE);

        grp->BroadcastPacket(data, true);
    }
    else if (!grp)
    {
        // Add player to queued players list
        LFGPlayerQueueInfo& i_Player = m_queuedPlayers[leader->GetObjectGuid()];

        i_Player.team = leader->GetTeam();
        i_Player.areaId = queueAreaID;
        i_Player.hasQueuePriority = false;
        if (sWorld.getConfig(CONFIG_BOOL_LFG_MATCHMAKING))
            i_Player.CalculateTalentRoles(leader);
        else
            i_Player.CalculateRoles((Classes)leader->getClass());
        i_Player.name = leader->GetName();

        leader->GetSession()->SendMeetingstoneSetqueue(queueAreaID, MEETINGSTONE_STATUS_JOINED_QUEUE);
    }
}

void LFGQueue::RestoreOfflinePlayer(Player* player)
{
    if (!player || !player->GetSession())
        return;

    QueuedPlayersMap::iterator offlinePlr = m_offlinePlayers.find(player->GetObjectGuid());

    if (offlinePlr != m_offlinePlayers.end())
    {
        player->GetSession()->SendMeetingstoneSetqueue(offlinePlr->second.areaId, MEETINGSTONE_STATUS_JOINED_QUEUE);
        m_queuedPlayers[player->GetObjectGuid()] = offlinePlr->second;
        m_offlinePlayers.erase(offlinePlr);
    }
    else
    {
        player->GetSession()->SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);
    }
}

LfgRoles LFGQueue::CalculateRoles(Classes playerClass)
{
    switch (playerClass)
    {
        case CLASS_DRUID:   return (LfgRoles)(LFG_ROLE_TANK | LFG_ROLE_DPS | LFG_ROLE_HEALER);
        case CLASS_HUNTER:  return (LfgRoles)(LFG_ROLE_DPS);
        case CLASS_MAGE:    return (LfgRoles)(LFG_ROLE_DPS);
        case CLASS_PALADIN: return (LfgRoles)(LFG_ROLE_TANK | LFG_ROLE_DPS | LFG_ROLE_HEALER);
        case CLASS_PRIEST:  return (LfgRoles)(LFG_ROLE_DPS | LFG_ROLE_HEALER);
        case CLASS_ROGUE:   return (LfgRoles)(LFG_ROLE_DPS);
        case CLASS_SHAMAN:  return (LfgRoles)(LFG_ROLE_DPS | LFG_ROLE_HEALER);
        case CLASS_WARLOCK: return (LfgRoles)(LFG_ROLE_DPS);
        case CLASS_WARRIOR: return (LfgRoles)(LFG_ROLE_TANK | LFG_ROLE_DPS);
        default:            return (LfgRoles)(LFG_ROLE_NONE);
    }
}

LfgRolePriority LFGQueue::GetPriority(Classes playerClass, LfgRoles playerRoles)
{
    switch (playerRoles)
    {
        case LFG_ROLE_TANK:
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

        case LFG_ROLE_HEALER:
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

        case LFG_ROLE_DPS:
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

std::map<uint32, int32> LFGQueue::GetTalentTrees(Player* _player)
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

uint32 LFGQueue::GetHighestTalentTree(Player* _player)
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
    case CLASS_SHAMAN:
        tab = 2;
        break;
    case CLASS_PRIEST:
        tab = 1;
        break;
    case CLASS_DRUID:
        tab = 2;
        break;
    case CLASS_WARRIOR:
        tab = 2;
        break;
    }
    return tab;
}

LfgRoles LFGQueue::CalculateTalentRoles(Player* _player)
{
    LfgRoles role = LFG_ROLE_NONE;
    uint32 tab = GetHighestTalentTree(_player);
    switch (_player->getClass())
    {
    case CLASS_PRIEST:
        if (tab == 2)
            role = LFG_ROLE_DPS;
        else
            role = LFG_ROLE_HEALER;
        break;
    case CLASS_SHAMAN:
        if (tab == 2)
            role = LFG_ROLE_HEALER;
        else
            role = LFG_ROLE_DPS;
        break;
    case CLASS_WARRIOR:
        if (tab == 2)
            role = LFG_ROLE_TANK;
        else
            role = LFG_ROLE_DPS;
        break;
    case CLASS_PALADIN:
        if (tab == 0)
            role = LFG_ROLE_HEALER;
        else if (tab == 1)
            role = LFG_ROLE_TANK;
        else if (tab == 2)
            role = LFG_ROLE_DPS;
        break;
    case CLASS_DRUID:
        if (tab == 0)
            role = LFG_ROLE_DPS;
        else if (tab == 1)
            role = (LfgRoles)(LFG_ROLE_TANK | LFG_ROLE_DPS);
        else if (tab == 2)
            role = LFG_ROLE_HEALER;
        break;
    default:
        role = LFG_ROLE_DPS;
        break;
    }
    return role;
}

void LFGQueue::UpdateGroup(uint32 groupId)
{
    QueuedGroupsMap::iterator qGroup = m_queuedGroups.find(groupId);

    if (qGroup != m_queuedGroups.end())
    {
        if (Group* grp = sObjectMgr.GetGroupById(qGroup->first))
        {
            // If the group is full, we don't care about the roles
            if (!grp->IsFull())
                grp->CalculateLFGRoles(qGroup->second);
        }
    }
}

void LFGQueue::Update(uint32 diff)
{
    if (m_queuedGroups.empty() && m_queuedPlayers.empty())
        return;

    // Iterate over QueuedPlayersMap to update players timers and remove offline/disconnected players.
    for (QueuedPlayersMap::iterator iter = m_queuedPlayers.begin(); iter != m_queuedPlayers.end();)
    {
        Player* plr = sObjectMgr.GetPlayer(iter->first);

        // Player could have been disconnected
        if (!plr || !plr->IsInWorld())
        {
            m_offlinePlayers[iter->first] = iter->second;
            iter = m_queuedPlayers.erase(iter);
            continue;
        }

        iter->second.timeInLFG += diff;

        // Update player timer and give him queue priority.
        if (iter->second.timeInLFG >= (30 * MINUTE * IN_MILLISECONDS))
            iter->second.hasQueuePriority = true;

        // if matchmaking enabled, ignore talents after some time (default 5 min)
        if (sWorld.getConfig(CONFIG_BOOL_LFG_MATCHMAKING) && iter->second.timeInLFG >= (sWorld.getConfig(CONFIG_UINT32_LFG_MATCHMAKING_TIMER) * IN_MILLISECONDS))
        {
            iter->second.rolePriority.clear();
            iter->second.CalculateRoles((Classes)plr->getClass());
        }

        ++iter;
    }

    if (!m_queuedGroups.empty())
    {
        // Iterate over QueuedGroupsMap to fill groups with roles they're missing.
        for (QueuedGroupsMap::iterator qGroup = m_queuedGroups.begin(); qGroup != m_queuedGroups.end();)
        {
            Group* grp = sObjectMgr.GetGroupById(qGroup->first);

            // Safe check
            if (!grp)
            {
                qGroup = m_queuedGroups.erase(qGroup);
                continue;
            }

            // Remove group from queue if it has been filled by players manually inviting.
            // We fill 1 group per update since removing it from the queue invalidates
            // the iterator and we can save on performance a little.
            if (grp->IsFull())
            {
                RemoveGroupFromQueue(qGroup->first, GROUP_SYSTEM_LEAVE);
                break;
            }

            // Iterate over QueuedPlayersMap to find suitable player to join group
            QueuedPlayersMap::iterator next = m_queuedPlayers.begin();
            for (QueuedPlayersMap::iterator qPlayer = next; next != m_queuedPlayers.end(); qPlayer = next)
            {
                // Pre-increment iterator here since FindRoleToGroup() may remove qPlayer
                // from the map
                ++next;

                // Check here that players team and areaId they're in queue are same
                if (qPlayer->second.team == qGroup->second.team &&
                   qPlayer->second.areaId == qGroup->second.areaId)
                {
                    bool groupFound = false;
                    // Find any role that this player matches and that the group requires. If none,
                    // then continue onto the next group.
                    for (LfgRoles role : PotentialRoles)
                    {
                        if (!(qPlayer->second.roleMask & role))
                            continue;

                        if ((role & qGroup->second.availableRoles) == role && FindRoleToGroup(qPlayer->first, grp, role))
                        {
                            groupFound = true;
                            break;
                        }
                    }

                    // If we found a group and it's now full, break. If it's not full,
                    // go onto the next player and maybe they can fill it.
                    if (groupFound && grp->IsFull())
                    {
                        break;
                    }
                }
            }

            if (grp->IsFull())
            {
                RemoveGroupFromQueue(qGroup->first, GROUP_SYSTEM_LEAVE);
                break;
            }

            // Update group timer. After each 5 minutes group will be broadcasted they're still waiting more members.
            if (qGroup->second.groupTimer <= diff)
            {
                WorldPacket data;
                BuildInProgressPacket(data);

                grp->BroadcastPacket(data, true);

                qGroup->second.groupTimer = 5 * MINUTE * IN_MILLISECONDS;
            }
            else
            {
                qGroup->second.groupTimer -= diff;
            }

            ++qGroup;
        }
    }

    // Pick first 2 players and form group out of them also inserting them into queue as group.
    if (m_queuedPlayers.size() >= m_groupSize)
    {
        // Pick Leader as first target.
        QueuedPlayersMap::iterator leader = m_queuedPlayers.begin();

        std::list<ObjectGuid> playersInArea;
        FindInArea(playersInArea, leader->second.areaId, leader->second.team, leader->first);

        // 4 players + the leader
        if (playersInArea.size() >= m_groupSize-1)
        {
            Player* pLeader = sObjectMgr.GetPlayer(leader->first);
            Player* pMember = sObjectMgr.GetPlayer(playersInArea.front());

            if (!pLeader || !pMember)
            {
                sLog.outError("LFGQueue::Update - null leader or member when forming group from queued players");
                return;
            }

            Group* newQueueGroup = new Group;
            if (!newQueueGroup->IsCreated())
            {
                if (newQueueGroup->Create(pLeader->GetObjectGuid(), pLeader->GetName()))
                    sObjectMgr.AddGroup(newQueueGroup);
                else
                    return;
            }

            WorldPacket data;
            BuildMemberAddedPacket(data, pMember->GetObjectGuid());

            pLeader->GetSession()->SendPacket(data);

            // Add member to the group. Leader is already added upon creation of group.
            newQueueGroup->AddMember(pMember->GetObjectGuid(), pMember->GetName(), GROUP_LFG);

            // Add this new group to GroupQueue now and remove players from PlayerQueue
            AddToQueue(pLeader, leader->second.areaId);

            RemovePlayerFromQueue(leader->first, PLAYER_SYSTEM_LEAVE);
            RemovePlayerFromQueue(pMember->GetObjectGuid(), PLAYER_SYSTEM_LEAVE);
        }
    }
}

// Don't pass playerGuid by ref since we may destroy it in RemovePlayerFromQueue
bool LFGQueue::FindRoleToGroup(ObjectGuid playerGuid, Group* group, LfgRoles role)
{
    QueuedGroupsMap::iterator qGroup = m_queuedGroups.find(group->GetId());
    QueuedPlayersMap::iterator qPlayer = m_queuedPlayers.find(playerGuid);

    if (qGroup != m_queuedGroups.end() && qPlayer != m_queuedPlayers.end())
    {
        bool queueTimePriority = qPlayer->second.hasQueuePriority;
        bool classPriority = qPlayer->second.GetRolePriority(role);
        // Iterate over QueuedPlayersMap to find if players have been longer in Queue.
        for (auto& itr : m_queuedPlayers)
        {
            if (qPlayer->first == itr.first)
                continue;

            // Ignore players queuing for a different dungeon or from opposite factions
            if (qPlayer->second.areaId != itr.second.areaId || qPlayer->second.team != itr.second.team)
                continue;

            // Compare priority/queue time to players that can fill the same role
            if ((itr.second.roleMask & role) == role)
            {
                bool otherTimePriority = itr.second.hasQueuePriority;
                bool otherClassPriority = itr.second.GetRolePriority(role);
                bool otherLongerInQueue = itr.second.timeInLFG > qPlayer->second.timeInLFG;

                // Another player is more valuable in this role, they have priority
                if (otherClassPriority > classPriority)
                    return false;

                // If the other player has time priority and has spent longer in the queue,
                // they are ahead of us
                if (otherTimePriority && otherLongerInQueue)
                    return false;

                // We do not have priority in the queue and they have spent longer
                // in the queue, that means they are ahead
                if (!queueTimePriority && otherLongerInQueue)
                    return false;
            }
        }

        switch (role)
        {
            case LFG_ROLE_TANK:
            {
                // Remove tank flag if player can perform tank role.
                qGroup->second.availableRoles &= ~LFG_ROLE_TANK;
                break;
            }

            case LFG_ROLE_HEALER:
            {
                // Remove healer flag if player can perform healer role.
                qGroup->second.availableRoles &= ~LFG_ROLE_HEALER;
                break;
            }

            case LFG_ROLE_DPS:
            {
                if (qGroup->second.dpsCount < LFGQueue::GetMaximumDPSSlots())
                {
                    // Update dps count first.
                    ++qGroup->second.dpsCount;

                    // Remove dps flag if there is enough dps in group.
                    if (qGroup->second.dpsCount >= LFGQueue::GetMaximumDPSSlots())
                        qGroup->second.availableRoles &= ~LFG_ROLE_DPS;
                }
                break;
            }

            default:
            {
                return false;
            }
        }

        WorldPacket data;
        BuildMemberAddedPacket(data, playerGuid);
        group->BroadcastPacket(data, true);

        // Add member to the group.
        group->AddMember(playerGuid, qPlayer->second.name.c_str(), GROUP_LFG);

        // Remove player from queue.
        RemovePlayerFromQueue(qPlayer->first, PLAYER_SYSTEM_LEAVE);

        // Found player return true.
        return true;
    }

    return false;
}

bool LFGQueue::IsPlayerInQueue(ObjectGuid const& plrGuid) const
{
    return m_queuedPlayers.find(plrGuid) != m_queuedPlayers.end();
}

bool LFGQueue::IsGroupInQueue(uint32 groupId) const
{
    return m_queuedGroups.find(groupId) != m_queuedGroups.end();
}

void LFGQueue::RemovePlayerFromQueue(ObjectGuid const& plrGuid, PlayerLeaveMethod leaveMethod)
{
    QueuedPlayersMap::iterator iter = m_queuedPlayers.find(plrGuid);
    if (iter != m_queuedPlayers.end())
    {
        if (leaveMethod == PLAYER_CLIENT_LEAVE)
        {
            Player* player = sObjectMgr.GetPlayer(plrGuid);

            if (player && player->GetSession())
            {
                WorldPacket data;
                BuildSetQueuePacket(data, 0, MEETINGSTONE_STATUS_LEAVE_QUEUE);
                player->GetSession()->SendPacket(data);
            }
        }

        m_queuedPlayers.erase(iter);
    }
}

void LFGQueue::RemoveGroupFromQueue(uint32 groupId, GroupLeaveMethod leaveMethod)
{
    QueuedGroupsMap::iterator iter = m_queuedGroups.find(groupId);

    if (iter != m_queuedGroups.end())
    {
        if (Group* grp = sObjectMgr.GetGroupById(groupId))
        {
            if (leaveMethod == GROUP_CLIENT_LEAVE)
            {
                WorldPacket data;
                BuildSetQueuePacket(data, 0, MEETINGSTONE_STATUS_LEAVE_QUEUE);
                grp->BroadcastPacket(data, true);
            }
            else
            {
                // Send complete information to party
                WorldPacket data;
                BuildCompletePacket(data);
                grp->BroadcastPacket(data, true);

                // Reset UI for party
                BuildSetQueuePacket(data, 0, MEETINGSTONE_STATUS_NONE);
                grp->BroadcastPacket(data, true);
            }

            grp->SetLFGAreaId(0);
        }

        m_queuedGroups.erase(iter);
    }
}

void LFGQueue::FindInArea(std::list<ObjectGuid>& players, uint32 area, uint32 team, ObjectGuid const& exclude)
{
    for (const auto& itr : m_queuedPlayers)
    {
        if (itr.first == exclude)
            continue;

        if (itr.second.areaId == area && itr.second.team == team)
            players.push_back(itr.first);
    }
}

void LFGQueue::GetPlayerQueueInfo(LFGPlayerQueueInfo* info, ObjectGuid const& plrGuid)
{
    QueuedPlayersMap::iterator iter = m_queuedPlayers.find(plrGuid);
    if (iter != m_queuedPlayers.end())
        *info = iter->second;

    return;
}

void LFGQueue::GetGroupQueueInfo(LFGGroupQueueInfo* info, uint32 groupId)
{
    QueuedGroupsMap::iterator iter = m_queuedGroups.find(groupId);
    if (iter != m_queuedGroups.end())
        *info = iter->second;

    return;
}

void LFGQueue::BuildSetQueuePacket(WorldPacket& data, uint32 areaId, uint8 status)
{
    data.Initialize(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << uint32(areaId);
    data << uint8(status);
}

void LFGQueue::BuildMemberAddedPacket(WorldPacket& data, ObjectGuid plrGuid)
{
    data.Initialize(SMSG_MEETINGSTONE_MEMBER_ADDED, 8);
    data << uint64(plrGuid);
}

void LFGQueue::BuildInProgressPacket(WorldPacket& data)
{
    data.Initialize(SMSG_MEETINGSTONE_IN_PROGRESS, 0);
}

void LFGQueue::BuildCompletePacket(WorldPacket& data)
{
    data.Initialize(SMSG_MEETINGSTONE_COMPLETE, 0);
}
