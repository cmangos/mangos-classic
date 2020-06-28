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
#include "Globals/SharedDefines.h"
#include "Entities/Player.h"
#include "BattleGroundMgr.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundWS.h"
#include "Maps/MapManager.h"
#include "Maps/Map.h"
#include "Globals/ObjectMgr.h"
#include "ProgressBar.h"
#include "Chat/Chat.h"
#include "World/World.h"
#include "WorldPacket.h"
#include "Tools/Language.h"
#include "GameEvents/GameEventMgr.h"

#include "Policies/Singleton.h"

INSTANTIATE_SINGLETON_1(BattleGroundMgr);

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattleGroundQueue::BattleGroundQueue()
{
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        for (uint8 j = 0; j < MAX_BATTLEGROUND_BRACKETS; ++j)
        {
            m_sumOfWaitTimes[i][j] = 0;
            m_waitTimeLastPlayer[i][j] = 0;

            for (uint8 k = 0; k < COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME; ++k)
                m_waitTimes[i][j][k] = 0;
        }
    }
}

BattleGroundQueue::~BattleGroundQueue()
{
    m_queuedPlayers.clear();
    for (auto& m_queuedGroup : m_queuedGroups)
    {
        for (uint8 j = 0; j < BG_QUEUE_GROUP_TYPES_COUNT; ++j)
        {
            for (GroupsQueueType::iterator itr = m_queuedGroup[j].begin(); itr != m_queuedGroup[j].end(); ++itr)
                delete (*itr);

            m_queuedGroup[j].clear();
        }
    }
}

/*********************************************************/
/***      BATTLEGROUND QUEUE SELECTION POOLS           ***/
/*********************************************************/

// selection pool initialization, used to clean up from prev selection
void BattleGroundQueue::SelectionPool::Init()
{
    selectedGroups.clear();
    playerCount = 0;
}

/**
  Function that removes group infr from pool selection
  - returns true when we need to try to add new group to selection pool
  - returns false when selection pool is ok or when we kicked smaller group than we need to kick
  - sometimes it can be called on empty selection pool

  @param    size
*/
bool BattleGroundQueue::SelectionPool::KickGroup(uint32 size)
{
    // find maxgroup or LAST group with size == size and kick it
    bool found = false;
    GroupsQueueType::iterator groupToKick = selectedGroups.begin();

    for (GroupsQueueType::iterator itr = groupToKick; itr != selectedGroups.end(); ++itr)
    {
        if (abs((int32)((*itr)->players.size() - size)) <= 1)
        {
            groupToKick = itr;
            found = true;
        }
        else if (!found && (*itr)->players.size() >= (*groupToKick)->players.size())
            groupToKick = itr;
    }

    // if pool is empty, do nothing
    if (GetPlayerCount())
    {
        // update player count
        GroupQueueInfo* queueInfo = (*groupToKick);
        selectedGroups.erase(groupToKick);
        playerCount -= queueInfo->players.size();

        // return false if we kicked smaller group or there are enough players in selection pool
        if (queueInfo->players.size() <= size + 1)
            return false;
    }
    return true;
}

/**
  Function that adds group to selection pool
  - returns true if we can invite more players, or when we added group to selection pool
  - returns false when selection pool is full

  @param    group queue info
  @param    desired count
*/
bool BattleGroundQueue::SelectionPool::AddGroup(GroupQueueInfo* queueInfo, uint32 desiredCount)
{
    // if group is larger than desired count - don't allow to add it to pool
    if (!queueInfo->isInvitedToBgInstanceGuid && desiredCount >= playerCount + queueInfo->players.size())
    {
        selectedGroups.push_back(queueInfo);
        // increase selected players count
        playerCount += queueInfo->players.size();

        return true;
    }

    return playerCount < desiredCount;
}

/*********************************************************/
/***               BATTLEGROUND QUEUES                 ***/
/*********************************************************/

// add group or player (grp == nullptr) to bg queue with the given leader and bg specifications
GroupQueueInfo* BattleGroundQueue::AddGroup(Player* leader, Group* group, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId, bool isPremade)
{
    // create new ginfo
    GroupQueueInfo* queueInfo = new GroupQueueInfo;
    queueInfo->bgTypeId                  = bgTypeId;
    queueInfo->isInvitedToBgInstanceGuid = 0;
    queueInfo->joinTime                  = WorldTimer::getMSTime();
    queueInfo->removeInviteTime          = 0;
    queueInfo->groupTeam                 = leader->GetTeam();

    queueInfo->players.clear();

    // compute index (if group is premade or joined a rated match) to queues
    uint32 index = 0;
    if (!isPremade)
        index += PVP_TEAM_COUNT;                            // BG_QUEUE_PREMADE_* -> BG_QUEUE_NORMAL_*

    if (queueInfo->groupTeam == HORDE)
        ++index;                                            // BG_QUEUE_*_ALLIANCE -> BG_QUEUE_*_HORDE

    DEBUG_LOG("Adding Group to BattleGroundQueue bgTypeId : %u, bracket_id : %u, index : %u", bgTypeId, bracketId, index);

    uint32 lastOnlineTime = WorldTimer::getMSTime();

    // add players from group to ginfo
    {
        // std::lock_guard<std::recursive_mutex> guard(m_Lock);
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->getSource();
                if (!member)
                    continue;   // this should never happen
                PlayerQueueInfo& playerInfo = m_queuedPlayers[member->GetObjectGuid()];
                playerInfo.lastOnlineTime   = lastOnlineTime;
                playerInfo.groupInfo        = queueInfo;
                // add the pinfo to ginfo's list
                queueInfo->players[member->GetObjectGuid()]  = &playerInfo;
            }
        }
        else
        {
            PlayerQueueInfo& playerInfo = m_queuedPlayers[leader->GetObjectGuid()];
            playerInfo.lastOnlineTime   = lastOnlineTime;
            playerInfo.groupInfo        = queueInfo;
            queueInfo->players[leader->GetObjectGuid()]  = &playerInfo;
        }

        // add GroupInfo to m_QueuedGroups
        m_queuedGroups[bracketId][index].push_back(queueInfo);

        // announce to world, this code needs mutex
        if (!isPremade && sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN))
        {
            if (BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(queueInfo->bgTypeId))
            {
                char const* bgName = bg->GetName();
                uint32 minPlayers = bg->GetMinPlayersPerTeam();
                uint32 qHorde = 0;
                uint32 qAlliance = 0;
                uint32 q_min_level = leader->GetMinLevelForBattleGroundBracketId(bracketId, bgTypeId);
                GroupsQueueType::const_iterator itr;
                for (itr = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].begin(); itr != m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].end(); ++itr)
                    if (!(*itr)->isInvitedToBgInstanceGuid)
                        qAlliance += (*itr)->players.size();

                for (itr = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].begin(); itr != m_queuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].end(); ++itr)
                    if (!(*itr)->isInvitedToBgInstanceGuid)
                        qHorde += (*itr)->players.size();

                // Show queue status to player only (when joining queue)
                if (sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN) == 1)
                {
                    ChatHandler(leader).PSendSysMessage(LANG_BG_QUEUE_ANNOUNCE_SELF, bgName, q_min_level, q_min_level + 10,
                                                        qAlliance, (minPlayers > qAlliance) ? minPlayers - qAlliance : (uint32)0, qHorde, (minPlayers > qHorde) ? minPlayers - qHorde : (uint32)0);
                }
                // System message
                else
                {
                    sWorld.SendWorldText(LANG_BG_QUEUE_ANNOUNCE_WORLD, bgName, q_min_level, q_min_level + 10,
                                         qAlliance, (minPlayers > qAlliance) ? minPlayers - qAlliance : (uint32)0, qHorde, (minPlayers > qHorde) ? minPlayers - qHorde : (uint32)0);
                }
            }
        }
        // release mutex
    }

    return queueInfo;
}

/**
  Method that updates average update wait time

  @param    group queue info
  @param    bracket id
*/
void BattleGroundQueue::PlayerInvitedToBgUpdateAverageWaitTime(GroupQueueInfo* queueInfo, BattleGroundBracketId bracketId)
{
    uint32 timeInQueue = WorldTimer::getMSTimeDiff(queueInfo->joinTime, WorldTimer::getMSTime());
    uint8 teamIndex = TEAM_INDEX_ALLIANCE;                    // default set to BG_TEAM_ALLIANCE - or non rated arenas!

    if (queueInfo->groupTeam == HORDE)
        teamIndex = TEAM_INDEX_HORDE;

    // store pointer to arrayindex of player that was added first
    uint32* lastPlayerAddedPointer = &(m_waitTimeLastPlayer[teamIndex][bracketId]);

    // remove his time from sum
    m_sumOfWaitTimes[teamIndex][bracketId] -= m_waitTimes[teamIndex][bracketId][(*lastPlayerAddedPointer)];

    // set average time to new
    m_waitTimes[teamIndex][bracketId][(*lastPlayerAddedPointer)] = timeInQueue;

    // add new time to sum
    m_sumOfWaitTimes[teamIndex][bracketId] += timeInQueue;

    // set index of last player added to next one
    (*lastPlayerAddedPointer)++;
    (*lastPlayerAddedPointer) %= COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME;
}

/**
  Function that returns averate queue wait time

  @param    group queue info
  @param    bracket id
*/
uint32 BattleGroundQueue::GetAverageQueueWaitTime(GroupQueueInfo* queueInfo, BattleGroundBracketId bracketId)
{
    uint8 teamIndex = TEAM_INDEX_ALLIANCE;                    // default set to BG_TEAM_ALLIANCE - or non rated arenas!
    if (queueInfo->groupTeam == HORDE)
        teamIndex = TEAM_INDEX_HORDE;

    // check if there is enought values(we always add values > 0)
    if (m_waitTimes[teamIndex][bracketId][COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME - 1])
        return (m_sumOfWaitTimes[teamIndex][bracketId] / COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME);

    // if there aren't enough values return 0 - not available
    return 0;
}

/**
  Method that removes player from queue and from group info, if group info is empty then remove it too

  @param    guid
  @param    decrease invite count
*/
void BattleGroundQueue::RemovePlayer(ObjectGuid guid, bool decreaseInvitedCount)
{
    // Player *plr = sObjectMgr.GetPlayer(guid);
    // std::lock_guard<std::recursive_mutex> guard(m_Lock);

    int32 bracketId = -1;                                   // signed for proper for-loop finish

    // remove player from map, if he's there
    QueuedPlayersMap::iterator itr = m_queuedPlayers.find(guid);
    if (itr == m_queuedPlayers.end())
    {
        sLog.outError("BattleGroundQueue: couldn't find for remove: %s", guid.GetString().c_str());
        return;
    }

    GroupQueueInfo* group = itr->second.groupInfo;
    GroupsQueueType::iterator group_itr;
    // mostly people with the highest levels are in battlegrounds, thats why
    // we count from MAX_BATTLEGROUND_QUEUES - 1 to 0
    // variable index removes useless searching in other team's queue
    uint32 index = GetTeamIndexByTeamId(group->groupTeam);

    for (int8 bracketIdTmp = MAX_BATTLEGROUND_BRACKETS - 1; bracketIdTmp >= 0 && bracketId == -1; --bracketIdTmp)
    {
        // we must check premade and normal team's queue - because when players from premade are joining bg,
        // they leave groupinfo so we can't use its players size to find out index
        for (uint8 j = index; j < BG_QUEUE_GROUP_TYPES_COUNT; j += BG_QUEUE_NORMAL_ALLIANCE)
        {
            for (GroupsQueueType::iterator group_itr_tmp = m_queuedGroups[bracketIdTmp][j].begin(); group_itr_tmp != m_queuedGroups[bracketIdTmp][j].end(); ++group_itr_tmp)
            {
                if ((*group_itr_tmp) == group)
                {
                    bracketId = bracketIdTmp;
                    group_itr = group_itr_tmp;
                    // we must store index to be able to erase iterator
                    index = j;
                    break;
                }
            }
        }
    }

    // player can't be in queue without group, but just in case
    if (bracketId == -1)
    {
        sLog.outError("BattleGroundQueue: ERROR Cannot find groupinfo for %s", guid.GetString().c_str());
        return;
    }
    DEBUG_LOG("BattleGroundQueue: Removing %s, from bracket_id %u", guid.GetString().c_str(), (uint32)bracketId);

    // ALL variables are correctly set
    // We can ignore leveling up in queue - it should not cause crash
    // remove player from group
    // if only one player there, remove group

    // remove player queue info from group queue info
    GroupQueueInfoPlayers::iterator pitr = group->players.find(guid);
    if (pitr != group->players.end())
        group->players.erase(pitr);

    // if invited to bg, and should decrease invited count, then do it
    if (decreaseInvitedCount && group->isInvitedToBgInstanceGuid)
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(group->isInvitedToBgInstanceGuid, group->bgTypeId);
        if (bg)
            bg->DecreaseInvitedCount(group->groupTeam);
    }

    // remove player queue info
    m_queuedPlayers.erase(itr);

    // remove group queue info if needed
    if (group->players.empty())
    {
        m_queuedGroups[bracketId][index].erase(group_itr);
        delete group;
    }
}

/**
  Function that returns true when player is in queue and is invited to bgInstanceGuid

  @param    player guid
  @param    battleground instance guid
  @param    remove time
*/
bool BattleGroundQueue::IsPlayerInvited(ObjectGuid playerGuid, const uint32 bgInstanceGuid, const uint32 removeTime)
{
    // std::lock_guard<std::recursive_mutex> g(m_Lock);
    QueuedPlayersMap::const_iterator qItr = m_queuedPlayers.find(playerGuid);
    return (qItr != m_queuedPlayers.end()
            && qItr->second.groupInfo->isInvitedToBgInstanceGuid == bgInstanceGuid
            && qItr->second.groupInfo->removeInviteTime == removeTime);
}

/**
  Function that returns player group info data
  - returns true when the player is found in queue

  @param    player guid
  @param    group queue info
*/
bool BattleGroundQueue::GetPlayerGroupInfoData(ObjectGuid guid, GroupQueueInfo* queueInfo)
{
    // std::lock_guard<std::recursive_mutex> g(m_Lock);
    QueuedPlayersMap::const_iterator qItr = m_queuedPlayers.find(guid);
    if (qItr == m_queuedPlayers.end())
        return false;

    *queueInfo = *(qItr->second.groupInfo);
    return true;
}

/**
  Function that invites group to battleground

  @param    group queue info
  @param    battleground
  @param    team
*/
bool BattleGroundQueue::InviteGroupToBg(GroupQueueInfo* queueInfo, BattleGround* bg, Team team)
{
    // set side if needed
    if (team == ALLIANCE || team == HORDE)
        queueInfo->groupTeam = team;

    if (!queueInfo->isInvitedToBgInstanceGuid)
    {
        // not yet invited
        // set invitation
        queueInfo->isInvitedToBgInstanceGuid = bg->GetInstanceId();
        BattleGroundTypeId bgTypeId = bg->GetTypeId();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(bgTypeId);
        BattleGroundBracketId bracket_id = bg->GetBracketId();

        queueInfo->removeInviteTime = WorldTimer::getMSTime() + INVITE_ACCEPT_WAIT_TIME;

        // loop through the players
        for (GroupQueueInfoPlayers::iterator itr = queueInfo->players.begin(); itr != queueInfo->players.end(); ++itr)
        {
            // get the player
            Player* plr = sObjectMgr.GetPlayer(itr->first);
            // if offline, skip him, this should not happen - player is removed from queue when he logs out
            if (!plr)
                continue;

            // invite the player
            PlayerInvitedToBgUpdateAverageWaitTime(queueInfo, bracket_id);
            // sBattleGroundMgr.InvitePlayer(plr, bg, ginfo->Team);

            // set invited player counters
            bg->IncreaseInvitedCount(queueInfo->groupTeam);

            plr->SetInviteForBattleGroundQueueType(bgQueueTypeId, queueInfo->isInvitedToBgInstanceGuid);

            // create remind invite events
            BgQueueInviteEvent* inviteEvent = new BgQueueInviteEvent(plr->GetObjectGuid(), queueInfo->isInvitedToBgInstanceGuid, bgTypeId, queueInfo->removeInviteTime);
            plr->m_events.AddEvent(inviteEvent, plr->m_events.CalculateTime(INVITATION_REMIND_TIME));

            WorldPacket data;

            uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);

            DEBUG_LOG("Battleground: invited %s to BG instance %u queueindex %u bgtype %u, I can't help it if they don't press the enter battle button.", plr->GetGuidStr().c_str(), bg->GetInstanceId(), queueSlot, bg->GetTypeId());

            // send status packet
            sBattleGroundMgr.BuildBattleGroundStatusPacket(data, bg, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0);
            plr->GetSession()->SendPacket(data);
        }
        return true;
    }

    return false;
}

/**
  Method that invites players to an already running battleground
  - invitation is based on config file
  - large groups are disadvantageous, because they will be kicked first if invitation type = 1

  @param    battleground
  @param    bracket id
*/
void BattleGroundQueue::FillPlayersToBg(BattleGround* bg, BattleGroundBracketId bracketId)
{
    int32 hordeFree = bg->GetFreeSlotsForTeam(HORDE);
    int32 aliFree   = bg->GetFreeSlotsForTeam(ALLIANCE);

    // iterator for iterating through bg queue
    GroupsQueueType::const_iterator Ali_itr = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].begin();
    // count of groups in queue - used to stop cycles
    uint32 aliCount = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].size();
    // index to queue which group is current
    uint32 aliIndex = 0;
    for (; aliIndex < aliCount && m_selectionPools[TEAM_INDEX_ALLIANCE].AddGroup((*Ali_itr), aliFree); ++aliIndex)
        ++Ali_itr;

    // the same thing for horde
    GroupsQueueType::const_iterator Horde_itr = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].begin();
    uint32 hordeCount = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].size();
    uint32 hordeIndex = 0;
    for (; hordeIndex < hordeCount && m_selectionPools[TEAM_INDEX_HORDE].AddGroup((*Horde_itr), hordeFree); ++hordeIndex)
        ++Horde_itr;

    // if ofc like BG queue invitation is set in config, then we are happy
    if (sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_INVITATION_TYPE) == 0)
        return;

    /*
    if we reached this code, then we have to solve NP - complete problem called Subset sum problem
    So one solution is to check all possible invitation subgroups, or we can use these conditions:
    1. Last time when BattleGroundQueue::Update was executed we invited all possible players - so there is only small possibility
        that we will invite now whole queue, because only 1 change has been made to queues from the last BattleGroundQueue::Update call
    2. Other thing we should consider is group order in queue
    */

    // At first we need to compare free space in bg and our selection pool
    int32 diffAli   = aliFree   - int32(m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount());
    int32 diffHorde = hordeFree - int32(m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount());
    while (abs(diffAli - diffHorde) > 1 && (m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount() > 0 || m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount() > 0))
    {
        // each cycle execution we need to kick at least 1 group
        if (diffAli < diffHorde)
        {
            // kick alliance group, add to pool new group if needed
            if (m_selectionPools[TEAM_INDEX_ALLIANCE].KickGroup(diffHorde - diffAli))
            {
                for (; aliIndex < aliCount && m_selectionPools[TEAM_INDEX_ALLIANCE].AddGroup((*Ali_itr), (aliFree >= diffHorde) ? aliFree - diffHorde : 0); ++aliIndex)
                    ++Ali_itr;
            }
            // if ali selection is already empty, then kick horde group, but if there are less horde than ali in bg - break;
            if (!m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount())
            {
                if (aliFree <= diffHorde + 1)
                    break;
                m_selectionPools[TEAM_INDEX_HORDE].KickGroup(diffHorde - diffAli);
            }
        }
        else
        {
            // kick horde group, add to pool new group if needed
            if (m_selectionPools[TEAM_INDEX_HORDE].KickGroup(diffAli - diffHorde))
            {
                for (; hordeIndex < hordeCount && m_selectionPools[TEAM_INDEX_HORDE].AddGroup((*Horde_itr), (hordeFree >= diffAli) ? hordeFree - diffAli : 0); ++hordeIndex)
                    ++Horde_itr;
            }
            if (!m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount())
            {
                if (hordeFree <= diffAli + 1)
                    break;
                m_selectionPools[TEAM_INDEX_ALLIANCE].KickGroup(diffAli - diffHorde);
            }
        }

        // count diffs after small update
        diffAli   = aliFree   - int32(m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount());
        diffHorde = hordeFree - int32(m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount());
    }
}

/**
  Method that checks if (premade vs premade) battlegrouns is possible
  - then after 30 mins (default) in queue it moves premade group to normal queue
  - it tries to invite as much players as it can - to maxPlayersPerTeam, because premade groups have more than minPlayersPerTeam players

  @param    bracket id
  @param    min players per team
  @param    max players per team
*/
bool BattleGroundQueue::CheckPremadeMatch(BattleGroundBracketId bracketId, uint32 minPlayersPerTeam, uint32 maxPlayersPerTeam)
{
    // check match
    if (!m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE].empty() && !m_queuedGroups[bracketId][BG_QUEUE_PREMADE_HORDE].empty())
    {
        // start premade match
        // if groups aren't invited
        GroupsQueueType::const_iterator ali_group, horde_group;
        for (ali_group = m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE].begin(); ali_group != m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE].end(); ++ali_group)
            if (!(*ali_group)->isInvitedToBgInstanceGuid)
                break;

        for (horde_group = m_queuedGroups[bracketId][BG_QUEUE_PREMADE_HORDE].begin(); horde_group != m_queuedGroups[bracketId][BG_QUEUE_PREMADE_HORDE].end(); ++horde_group)
            if (!(*horde_group)->isInvitedToBgInstanceGuid)
                break;

        if (ali_group != m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE].end() && horde_group != m_queuedGroups[bracketId][BG_QUEUE_PREMADE_HORDE].end())
        {
            m_selectionPools[TEAM_INDEX_ALLIANCE].AddGroup((*ali_group), maxPlayersPerTeam);
            m_selectionPools[TEAM_INDEX_HORDE].AddGroup((*horde_group), maxPlayersPerTeam);

            // add groups/players from normal queue to size of bigger group
            uint32 maxPlayers = std::max(m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount(), m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount());
            for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
            {
                for (GroupsQueueType::const_iterator itr = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + i].begin(); itr != m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + i].end(); ++itr)
                {
                    // if itr can join BG and player count is less that maxPlayers, then add group to selectionpool
                    if (!(*itr)->isInvitedToBgInstanceGuid && !m_selectionPools[i].AddGroup((*itr), maxPlayers))
                        break;
                }
            }

            // premade selection pools are set
            return true;
        }
    }
    // now check if we can move group from Premade queue to normal queue (timer has expired) or group size lowered!!
    // this could be 2 cycles but i'm checking only first team in queue - it can cause problem -
    // if first is invited to BG and seconds timer expired, but we can ignore it, because players have only 80 seconds to click to enter bg
    // and when they click or after 80 seconds the queue info is removed from queue
    uint32 time_before = WorldTimer::getMSTime() - sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH);
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        if (!m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE + i].empty())
        {
            GroupsQueueType::iterator itr = m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE + i].begin();
            if (!(*itr)->isInvitedToBgInstanceGuid && ((*itr)->joinTime < time_before || (*itr)->players.size() < minPlayersPerTeam))
            {
                // we must insert group to normal queue and erase pointer from premade queue
                m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + i].push_front((*itr));
                m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE + i].erase(itr);
            }
        }
    }

    // selection pools are not set
    return false;
}

/**
  Method that tries to create battleground with minPlayersPerTeam against maxPlayersPerTeam

  @param    battleground
  @param    bracket id
  @param    min players
  @param    max players
*/
bool BattleGroundQueue::CheckNormalMatch(BattleGroundBracketId bracketId, uint32 minPlayers, uint32 maxPlayers)
{
    GroupsQueueType::const_iterator itr_team[PVP_TEAM_COUNT];
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        itr_team[i] = m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + i].begin();
        for (; itr_team[i] != m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + i].end(); ++(itr_team[i]))
        {
            if (!(*(itr_team[i]))->isInvitedToBgInstanceGuid)
            {
                m_selectionPools[i].AddGroup(*(itr_team[i]), maxPlayers);
                if (m_selectionPools[i].GetPlayerCount() >= minPlayers)
                    break;
            }
        }
    }

    // try to invite same number of players - this cycle may cause longer wait time even if there are enough players in queue, but we want ballanced bg
    uint32 j = TEAM_INDEX_ALLIANCE;
    if (m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount() < m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount())
        j = TEAM_INDEX_HORDE;

    if (sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_INVITATION_TYPE) != 0
            && m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount() >= minPlayers && m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount() >= minPlayers)
    {
        // we will try to invite more groups to team with less players indexed by j
        ++(itr_team[j]);                                    // this will not cause a crash, because for cycle above reached break;
        for (; itr_team[j] != m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE + j].end(); ++(itr_team[j]))
        {
            if (!(*(itr_team[j]))->isInvitedToBgInstanceGuid)
                if (!m_selectionPools[j].AddGroup(*(itr_team[j]), m_selectionPools[(j + 1) % PVP_TEAM_COUNT].GetPlayerCount()))
                    break;
        }
        // do not allow to start bg with more than 2 players more on 1 faction
        if (abs((int32)(m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount() - m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount())) > 2)
            return false;
    }

    // allow 1v0 if debug bg
    if (sBattleGroundMgr.IsTesting() && (m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount() || m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount()))
        return true;

    // return true if there are enough players in selection pools - enable to work .debug bg command correctly
    return m_selectionPools[TEAM_INDEX_ALLIANCE].GetPlayerCount() >= minPlayers && m_selectionPools[TEAM_INDEX_HORDE].GetPlayerCount() >= minPlayers;
}

/**
  Method that is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while(true) cycles to invite whole queue
  - it must be called after fully adding the members of a group to ensure group joining
  - should be called from BattleGround::RemovePlayer function in some cases

  @param    bg type id
  @param    bracket id
  @param    arena type
  @param    isRated
  @param    arenaRating
*/
void BattleGroundQueue::Update(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId)
{
    // std::lock_guard<std::recursive_mutex> guard(m_Lock);
    // if no players in queue - do nothing
    if (m_queuedGroups[bracketId][BG_QUEUE_PREMADE_ALLIANCE].empty() &&
            m_queuedGroups[bracketId][BG_QUEUE_PREMADE_HORDE].empty() &&
            m_queuedGroups[bracketId][BG_QUEUE_NORMAL_ALLIANCE].empty() &&
            m_queuedGroups[bracketId][BG_QUEUE_NORMAL_HORDE].empty())
        return;

    // battleground with free slot for player should be always in the beggining of the queue
    // maybe it would be better to create bgfreeslotqueue for each bracket_id
    BgFreeSlotQueueType::iterator next;
    for (BgFreeSlotQueueType::iterator itr = sBattleGroundMgr.BgFreeSlotQueue[bgTypeId].begin(); itr != sBattleGroundMgr.BgFreeSlotQueue[bgTypeId].end(); itr = next)
    {
        next = itr;
        ++next;
        // battleground is running, so if:
        if ((*itr)->GetTypeId() == bgTypeId && (*itr)->GetBracketId() == bracketId &&
                (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
        {
            BattleGround* bg = *itr; // we have to store battleground pointer here, because when battleground is full, it is removed from free queue (not yet implemented!!)
            // and iterator is invalid

            // clear selection pools
            m_selectionPools[TEAM_INDEX_ALLIANCE].Init();
            m_selectionPools[TEAM_INDEX_HORDE].Init();

            // call a function that does the job for us
            FillPlayersToBg(bg, bracketId);

            // now everything is set, invite players
            for (GroupsQueueType::const_iterator citr = m_selectionPools[TEAM_INDEX_ALLIANCE].selectedGroups.begin(); citr != m_selectionPools[TEAM_INDEX_ALLIANCE].selectedGroups.end(); ++citr)
                InviteGroupToBg((*citr), bg, (*citr)->groupTeam);
            for (GroupsQueueType::const_iterator citr = m_selectionPools[TEAM_INDEX_HORDE].selectedGroups.begin(); citr != m_selectionPools[TEAM_INDEX_HORDE].selectedGroups.end(); ++citr)
                InviteGroupToBg((*citr), bg, (*citr)->groupTeam);

            if (!bg->HasFreeSlots())
            {
                // remove BG from BGFreeSlotQueue
                bg->RemoveFromBgFreeSlotQueue();
            }
        }
    }

    // finished iterating through the bgs with free slots, maybe we need to create a new bg

    BattleGround* bgTemplate = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bgTemplate)
    {
        sLog.outError("Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    // get the min. players per team, properly for larger arenas as well.
    uint32 minPlayersPerTeam = bgTemplate->GetMinPlayersPerTeam();
    uint32 maxPlayersPerTeam = bgTemplate->GetMaxPlayersPerTeam();
    if (sBattleGroundMgr.IsTesting())
        minPlayersPerTeam = 1;

    m_selectionPools[TEAM_INDEX_ALLIANCE].Init();
    m_selectionPools[TEAM_INDEX_HORDE].Init();

    {
        // check if there is premade against premade match
        if (CheckPremadeMatch(bracketId, minPlayersPerTeam, maxPlayersPerTeam))
        {
            // create new battleground
            BattleGround* bg2 = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracketId);
            if (!bg2)
            {
                sLog.outError("BattleGroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            // invite those selection pools
            for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
                for (GroupsQueueType::const_iterator citr = m_selectionPools[TEAM_INDEX_ALLIANCE + i].selectedGroups.begin(); citr != m_selectionPools[TEAM_INDEX_ALLIANCE + i].selectedGroups.end(); ++citr)
                    InviteGroupToBg((*citr), bg2, (*citr)->groupTeam);

            // start bg
            bg2->StartBattleGround();
            // clear structures
            m_selectionPools[TEAM_INDEX_ALLIANCE].Init();
            m_selectionPools[TEAM_INDEX_HORDE].Init();
        }
    }

    // now check if there are in queues enough players to start new game of (normal battleground)
    {
        // if there are enough players in pools, start new battleground or non rated arena
        if (CheckNormalMatch(bracketId, minPlayersPerTeam, maxPlayersPerTeam))
        {
            // we successfully created a pool
            BattleGround* bg2 = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracketId);
            if (!bg2)
            {
                sLog.outError("BattleGroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            // invite those selection pools
            for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
                for (GroupsQueueType::const_iterator citr = m_selectionPools[TEAM_INDEX_ALLIANCE + i].selectedGroups.begin(); citr != m_selectionPools[TEAM_INDEX_ALLIANCE + i].selectedGroups.end(); ++citr)
                    InviteGroupToBg((*citr), bg2, (*citr)->groupTeam);

            // start bg
            bg2->StartBattleGround();
        }
    }
}

/*********************************************************/
/***            BATTLEGROUND QUEUE EVENTS              ***/
/*********************************************************/

/**
  Function that executes battleground queue invite event
*/
bool BgQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    sWorld.GetMessager().AddMessage([=](World* world)
    {
        Player* plr = sObjectMgr.GetPlayer(m_playerGuid);
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        if (!plr)
            return;

        BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_bgInstanceGuid, m_bgTypeId);
        // if battleground ended and its instance deleted - do nothing
        if (!bg)
            return;

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(bg->GetTypeId());
        uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
        if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue or in battleground
        {
            // check if player is invited to this bg
            BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];
            if (bgQueue.IsPlayerInvited(m_playerGuid, m_bgInstanceGuid, m_removeTime))
            {
                WorldPacket data;
                // we must send remaining time in queue
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, bg, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME - INVITATION_REMIND_TIME, 0);
                plr->GetSession()->SendPacket(data);
            }
        }
    });
    return true;                                            // event will be deleted
}

void BgQueueInviteEvent::Abort(uint64 /*e_time*/)
{
    // do nothing
}

/**
 Function that executes battleground queue remove event
    this event has many possibilities when it is executed:
    1. player is in battleground ( he clicked enter on invitation window )
    2. player left battleground queue and he isn't there any more
    3. player left battleground queue and he joined it again and IsInvitedToBGInstanceGUID = 0
    4. player left queue and he joined again and he has been invited to same battleground again -> we should not remove him from queue yet
    5. player is invited to bg and he didn't choose what to do and timer expired - only in this condition we should call queue::RemovePlayer
    we must remove player in the 5. case even if battleground object doesn't exist!
*/
bool BgQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    sWorld.GetMessager().AddMessage([=](World* world)
    {
        Player* plr = sObjectMgr.GetPlayer(m_playerGuid);
        if (!plr)
            // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
            return;

        BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_bgInstanceGuid, m_bgTypeId);
        // battleground can be deleted already when we are removing queue info
        // bg pointer can be nullptr! so use it carefully!

        uint32 queueSlot = plr->GetBattleGroundQueueIndex(m_bgQueueTypeId);
        if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue, or in Battleground
        {
            // check if player is in queue for this BG and if we are removing his invite event
            BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[m_bgQueueTypeId];
            if (bgQueue.IsPlayerInvited(m_playerGuid, m_bgInstanceGuid, m_removeTime))
            {
                DEBUG_LOG("Battleground: removing player %u from bg queue for instance %u because of not pressing enter battle in time.", plr->GetGUIDLow(), m_bgInstanceGuid);

                plr->RemoveBattleGroundQueueId(m_bgQueueTypeId);
                bgQueue.RemovePlayer(m_playerGuid, true);

                // update queues if battleground isn't ended
                if (bg && bg->GetStatus() != STATUS_WAIT_LEAVE)
                sBattleGroundMgr.ScheduleQueueUpdate(m_bgQueueTypeId, m_bgTypeId, bg->GetBracketId());

                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, bg, queueSlot, STATUS_NONE, 0, 0);
                plr->GetSession()->SendPacket(data);
            }
        }
    });

    // event will be deleted
    return true;
}

void BgQueueRemoveEvent::Abort(uint64 /*e_time*/)
{
    // do nothing
}

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattleGroundMgr::BattleGroundMgr()
{
    for (uint8 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
        m_battleGrounds[i].clear();
    m_testing = false;
}

BattleGroundMgr::~BattleGroundMgr()
{
    DeleteAllBattleGrounds();
}

/**
  Method that deletes all battlegrounds
*/
void BattleGroundMgr::DeleteAllBattleGrounds()
{
    // will also delete template bgs:
    for (uint8 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
    {
        for (BattleGroundSet::iterator itr = m_battleGrounds[i].begin(); itr != m_battleGrounds[i].end();)
        {
            BattleGround* bg = itr->second;
            ++itr;                                          // step from invalidate iterator pos in result element remove in ~BattleGround call
            delete bg;
        }
    }
}

/**
  Update method

  @param    diff
*/
void BattleGroundMgr::Update(uint32 diff)
{
    // update scheduled queues
    if (!m_queueUpdateScheduler.empty())
    {
        std::vector<uint64> scheduled;
        {
            // create mutex
            // std::lock_guard<std::mutex> guard(SchedulerLock);
            // copy vector and clear the other
            scheduled = std::vector<uint64>(m_queueUpdateScheduler);
            m_queueUpdateScheduler.clear();
            // release lock
        }

        for (uint32 i : scheduled)
        {
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundQueueTypeId(i >> 16 & 255);
            BattleGroundTypeId bgTypeId = BattleGroundTypeId((i >> 8) & 255);
            BattleGroundBracketId bracketId = BattleGroundBracketId(i & 255);
            m_battleGroundQueues[bgQueueTypeId].Update(bgTypeId, bracketId);
        }
    }
}

/**
  Send battleground status update

  @param    packet
  @param    battleground
  @param    queue slot
  @param    status id
  @param    time1
  @param    time2
*/
void BattleGroundMgr::BuildBattleGroundStatusPacket(WorldPacket& data, BattleGround* bg, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2) const
{
    // we can be in 3 queues in same time...

    if (statusId == 0 || !bg)
    {
        data.Initialize(SMSG_BATTLEFIELD_STATUS, 4 * 2);
        data << uint32(queueSlot);                         // queue id (0...2)
        data << uint32(0);
        return;
    }

    data.Initialize(SMSG_BATTLEFIELD_STATUS, (4 + 8 + 4 + 1 + 4 + 4 + 4));
    data << uint32(queueSlot);                             // queue id (0...2) - player can be in 3 queues in time
    // uint64 in client
    data << uint32(bg->GetMapId());                        // MapID
    data << uint8(0);                                      // Unknown
    data << uint32(bg->GetClientInstanceId());
    data << uint32(statusId);                              // status
    switch (statusId)
    {
        case STATUS_WAIT_QUEUE:                             // status_in_queue
            data << uint32(time1);                         // average wait time, milliseconds
            data << uint32(time2);                         // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:                              // status_invite
            data << uint32(time1);                         // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:                            // status_in_progress
            data << uint32(time1);                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            data << uint32(time2);                         // time from bg start, milliseconds
            break;
        default:
            sLog.outError("Unknown BG status!");
            break;
    }
}

/**
  Send battleground log update

  @param    packet
  @param    battleground
*/
void BattleGroundMgr::BuildPvpLogDataPacket(WorldPacket& data, BattleGround* bg) const
{
    data.Initialize(MSG_PVP_LOG_DATA, (1 + 4 + 40 * bg->GetPlayerScoresSize()));

    if (bg->GetStatus() != STATUS_WAIT_LEAVE)
    {
        data << uint8(0);                                  // bg not ended
    }
    else
    {
        data << uint8(1);                                  // bg ended
        data << uint8(bg->GetWinner());                    // who won
    }

    data << (uint32)(bg->GetPlayerScoresSize());

    for (BattleGround::BattleGroundScoreMap::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        const BattleGroundScore* score = itr->second;

        data << ObjectGuid(itr->first);

        Player* plr = sObjectMgr.GetPlayer(itr->first);

        data << uint32(plr ? plr->GetHonorRankInfo().visualRank : 0);
        data << uint32(itr->second->killingBlows);
        data << uint32(itr->second->honorableKills);
        data << uint32(itr->second->dishonorableKills);
        data << uint32(itr->second->deaths);
        data << uint32(itr->second->bonusHonor);

        switch (bg->GetTypeId())                            // battleground specific things
        {
            case BATTLEGROUND_AV:
                data << (uint32)0x00000007;                // count of next fields
                data << (uint32)((BattleGroundAVScore*)score)->graveyardsAssaulted;  // GraveyardsAssaulted
                data << (uint32)((BattleGroundAVScore*)score)->graveyardsDefended;   // GraveyardsDefended
                data << (uint32)((BattleGroundAVScore*)score)->towersAssaulted;      // TowersAssaulted
                data << (uint32)((BattleGroundAVScore*)score)->towersDefended;       // TowersDefended
                data << (uint32)((BattleGroundAVScore*)score)->secondaryObjectives;  // Mines Taken
                data << (uint32)((BattleGroundAVScore*)score)->lieutnantCount;       // Lieutnant kills
                data << (uint32)((BattleGroundAVScore*)score)->secondaryNPC;         // Secondary unit summons
                break;
            case BATTLEGROUND_WS:
                data << (uint32)0x00000002;                // count of next fields
                data << (uint32)((BattleGroundWGScore*)score)->flagCaptures;         // flag captures
                data << (uint32)((BattleGroundWGScore*)score)->flagReturns;          // flag returns
                break;
            case BATTLEGROUND_AB:
                data << (uint32)0x00000002;                // count of next fields
                data << (uint32)((BattleGroundABScore*)score)->basesAssaulted;       // bases asssulted
                data << (uint32)((BattleGroundABScore*)score)->basesDefended;        // bases defended
                break;
            default:
                DEBUG_LOG("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetTypeId());
                data << (uint32)0;
                break;
        }
    }
}

/**
  Send battleground joined packet

  @param    packet
  @param    result
*/
void BattleGroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket& data, BattleGroundTypeId bgTypeId) const
{
    /*bgTypeId is:
    0 - Your group has joined a battleground queue, but you are not eligible
    1 - Your group has joined the queue for AV
    2 - Your group has joined the queue for WS
    3 - Your group has joined the queue for AB*/
    data.Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    data << uint32(bgTypeId);
}

/**
  Send battleground world state packet

  @param    packet
  @param    field
  @param    value
*/
void BattleGroundMgr::BuildUpdateWorldStatePacket(WorldPacket& data, uint32 field, uint32 value) const
{
    data.Initialize(SMSG_UPDATE_WORLD_STATE, 4 + 4);
    data << uint32(field);
    data << uint32(value);
}

/**
  Send battleground sound packet

  @param    packet
  @param    sound id
*/
void BattleGroundMgr::BuildPlaySoundPacket(WorldPacket& data, uint32 soundId) const
{
    data.Initialize(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId);
}

/**
  Send player leave from battleground packet

  @param    packet
  @param    object guid
*/
void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket& data, ObjectGuid guid) const
{
    data.Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    data << ObjectGuid(guid);
}

/**
  Send player join from battleground packet

  @param    packet
  @param    player
*/
void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket& data, Player* player) const
{
    data.Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    data << player->GetObjectGuid();
}

/**
  Function that returns battleground from client instance id and battleground type

  @param    instance id
  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGroundThroughClientInstance(uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    // cause at HandleBattleGroundJoinOpcode the clients sends the instanceid he gets from
    // SMSG_BATTLEFIELD_LIST we need to find the battleground with this clientinstance-id
    BattleGround* bg = GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return nullptr;

    for (auto& itr : m_battleGrounds[bgTypeId])
    {
        if (itr.second->GetClientInstanceId() == instanceId)
            return itr.second;
    }

    return nullptr;
}

/**
  Function that returns battleground from instance id and battleground type

  @param    instance id
  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    // search if needed
    BattleGroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for (uint8 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
        {
            itr = m_battleGrounds[i].find(instanceId);
            if (itr != m_battleGrounds[i].end())
                return itr->second;
        }
        return nullptr;
    }

    itr = m_battleGrounds[bgTypeId].find(instanceId);
    return ((itr != m_battleGrounds[bgTypeId].end()) ? itr->second : nullptr);
}

/**
  Function that returns battleground from template id

  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGroundTemplate(BattleGroundTypeId bgTypeId)
{
    // map is sorted and we can be sure that lowest instance id has only BG template
    return m_battleGrounds[bgTypeId].empty() ? nullptr : m_battleGrounds[bgTypeId].begin()->second;
}

/**
  Function that returns client instance id from battleground type id and bracket id

  @param    battleground type id
  @param    bracket id
*/
uint32 BattleGroundMgr::CreateClientVisibleInstanceId(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId)
{
    // we create here an instanceid, which is just for
    // displaying this to the client and without any other use..
    // the client-instanceIds are unique for each battleground-type
    // the instance-id just needs to be as low as possible, beginning with 1
    // the following works, because std::set is default ordered with "<"
    // the optimalization would be to use as bitmask std::vector<uint32> - but that would only make code unreadable

    uint32 lastId = 0;
    ClientBattleGroundIdSet& ids = m_clientBattleGroundIds[bgTypeId][bracketId];
    for (ClientBattleGroundIdSet::const_iterator itr = ids.begin(); itr != ids.end();)
    {
        if ((++lastId) != *itr)                             // if there is a gap between the ids, we will break..
            break;
        lastId = *itr;
    }
    ids.insert(lastId + 1);

    return lastId + 1;
}

// create a new battleground that will really be used to play
BattleGround* BattleGroundMgr::CreateNewBattleGround(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId)
{
    // get the template BG
    BattleGround* bgTemplate = GetBattleGroundTemplate(bgTypeId);
    if (!bgTemplate)
    {
        sLog.outError("BattleGround: CreateNewBattleGround - bg template not found for %u", bgTypeId);
        return nullptr;
    }

    BattleGround* bg = nullptr;
    // create a copy of the BG template
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV(*(BattleGroundAV*)bgTemplate);
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS(*(BattleGroundWS*)bgTemplate);
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB(*(BattleGroundAB*)bgTemplate);
            break;
        default:
            // error, but it is handled few lines above
            return nullptr;
    }

    // will also set m_bgMap, instanceid
    sMapMgr.CreateBgMap(bg->GetMapId(), bg);

    bg->SetClientInstanceId(CreateClientVisibleInstanceId(bgTypeId, bracketId));

    // reset the new bg (set status to status_wait_queue from status_none)
    bg->Reset();

    // start the joining of the bg
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetBracketId(bracketId);

    return bg;
}

/**
  Function that creates battleground templates

  @param    battleground type id
  @param    min players per team
  @param    max players per team
  @param    level min
  @param    level max
  @param    battleground name
  @param    map id
  @param    team 1 start location X
  @param    team 1 start location Y
  @param    team 1 start location Z
  @param    team 1 start location O
  @param    team 2 start location X
  @param    team 2 start location Y
  @param    team 2 start location Z
  @param    team 2 start location O
  @param    start max distance
*/
uint32 BattleGroundMgr::CreateBattleGround(BattleGroundTypeId bgTypeId, uint32 minPlayersPerTeam, uint32 maxPlayersPerTeam, uint32 levelMin, uint32 levelMax, char const* battleGroundName, uint32 mapId, float team1StartLocX, float team1StartLocY, float team1StartLocZ, float team1StartLocO, float team2StartLocX, float team2StartLocY, float team2StartLocZ, float team2StartLocO, float startMaxDist)
{
    // Create the BG
    BattleGround* bg;
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: bg = new BattleGroundAV; break;
        case BATTLEGROUND_WS: bg = new BattleGroundWS; break;
        case BATTLEGROUND_AB: bg = new BattleGroundAB; break;
        default:              bg = new BattleGround;   break;                           // placeholder for non implemented BG
    }

    bg->SetMapId(mapId);
    bg->SetTypeID(bgTypeId);
    bg->SetMinPlayersPerTeam(minPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(maxPlayersPerTeam);
    bg->SetMinPlayers(minPlayersPerTeam * 2);
    bg->SetMaxPlayers(maxPlayersPerTeam * 2);
    bg->SetName(battleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, team1StartLocX, team1StartLocY, team1StartLocZ, team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    team2StartLocX, team2StartLocY, team2StartLocZ, team2StartLocO);
    bg->SetStartMaxDist(startMaxDist);
    bg->SetLevelRange(levelMin, levelMax);

    // add bg to update list
    AddBattleGround(bg->GetInstanceId(), bg->GetTypeId(), bg);

    // return some not-null value, bgTypeId is good enough for me
    return bgTypeId;
}

/**
  Method that loads battleground data from DB
*/
void BattleGroundMgr::CreateInitialBattleGrounds()
{
    uint32 count = 0;
    //                                                0   1                 2                 3      4      5                6             7 
    QueryResult* result = WorldDatabase.Query("SELECT id, MinPlayersPerTeam,MaxPlayersPerTeam,MinLvl,MaxLvl,AllianceStartLoc,HordeStartLoc,StartMaxDist FROM battleground_template");

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();
        bar.step();

        uint32 resultedBgTypeId = fields[0].GetUInt32();

        BattleGroundTypeId bgTypeId = BattleGroundTypeId(resultedBgTypeId);

        uint32 minPlayersPerTeam = fields[1].GetUInt32();
        uint32 maxPlayersPerTeam = fields[2].GetUInt32();
        uint32 minLvl = fields[3].GetUInt32();
        uint32 maxLvl = fields[4].GetUInt32();

        float allianceStartLoc[4];
        float hordeStartLoc[4];

        uint32 start1 = fields[5].GetUInt32();

        WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(start1);
        if (start)
        {
            allianceStartLoc[0] = start->x;
            allianceStartLoc[1] = start->y;
            allianceStartLoc[2] = start->z;
            allianceStartLoc[3] = start->o;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have nonexistent WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", bgTypeId, start1);
            continue;
        }

        uint32 start2 = fields[6].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(start2);
        if (start)
        {
            hordeStartLoc[0] = start->x;
            hordeStartLoc[1] = start->y;
            hordeStartLoc[2] = start->z;
            hordeStartLoc[3] = start->o;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", bgTypeId, start2);
            continue;
        }

        uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);
        char const* name;

        if (MapEntry const* mapEntry = sMapStore.LookupEntry(mapId))
            name = mapEntry->name[sWorld.GetDefaultDbcLocale()];
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u associated with nonexistent map id %u.", bgTypeId, mapId);
            continue;
        }

        float startMaxDist = fields[7].GetFloat();

        // sLog.outDetail("Creating battleground %s, %u-%u", bl->name[sWorld.GetDBClang()], MinLvl, MaxLvl);
        if (!CreateBattleGround(bgTypeId, minPlayersPerTeam, maxPlayersPerTeam, minLvl, maxLvl, name, mapId, allianceStartLoc[0], allianceStartLoc[1], allianceStartLoc[2], allianceStartLoc[3], hordeStartLoc[0], hordeStartLoc[1], hordeStartLoc[2], hordeStartLoc[3], startMaxDist))
            continue;

        ++count;
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u battlegrounds", count);
    sLog.outString();
}

/**
  Method that builds battleground list data

  @param    packet
  @param    battlemaster guid
  @param    player
  @param    battleground type id
*/
void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket& data, ObjectGuid guid, Player* player, BattleGroundTypeId bgTypeId) const
{
    if (!player)
        return;

    uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);

    data.Initialize(SMSG_BATTLEFIELD_LIST);
    data << guid;                                          // battlemaster guid
    data << uint32(mapId);                                 // battleground id
    data << uint8(0x00);                                   // unk

    size_t count_pos = data.wpos();
    uint32 count = 0;
    data << uint32(0);                                     // number of bg instances

    // battleground
    {
        data << uint8(0x00);                               // unk

        size_t count_pos = data.wpos();
        uint32 count = 0;
        data << uint32(0);                                 // number of bg instances

        uint32 bracket_id = player->GetBattleGroundBracketIdFromLevel(bgTypeId);
        ClientBattleGroundIdSet const& ids = m_clientBattleGroundIds[bgTypeId][bracket_id];
        for (std::set<uint32>::const_iterator itr = ids.begin(); itr != ids.end(); ++itr)
        {
            data << uint32(*itr);
            ++count;
        }
        data.put<uint32>(count_pos, count);
    }
}

/**
  Method that sends player to battleground

  @param    player
  @param    instance id
  @param    battleground type id
*/
void BattleGroundMgr::SendToBattleGround(Player* player, uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    BattleGround* bg = GetBattleGround(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        Team team = player->GetBGTeam();
        if (team == 0)
            team = player->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        DETAIL_LOG("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", player->GetName(), mapid, x, y, z, O);
        player->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog.outError("player %u trying to port to nonexistent bg instance %u", player->GetGUIDLow(), instanceId);
    }
}

/**
  Function that returns the queue type for battleground type id

  @param    battleground type id
  @param    arena type
*/
BattleGroundQueueTypeId BattleGroundMgr::BgQueueTypeId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

/**
  Function that returns the battleground type id from battleground queue id

  @param    battleground queue id
*/
BattleGroundTypeId BattleGroundMgr::BgTemplateId(BattleGroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        default:
            return BattleGroundTypeId(0);                   // used for unknown template (it exist and do nothing)
    }
}

/**
  Method that toggles battleground test mode on / off
*/
void BattleGroundMgr::ToggleTesting()
{
    m_testing = !m_testing;
    if (m_testing)
        sWorld.SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld.SendWorldText(LANG_DEBUG_BG_OFF);
}

/**
  Method that schedules queue update

  @param    arena rating
  @param    arena type
  @param    battleground queue type id
  @param    battleground type id
  @param    bracket id
*/
void BattleGroundMgr::ScheduleQueueUpdate(BattleGroundQueueTypeId bgQueueTypeId, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    // std::lock_guard<std::mutex> guard(SchedulerLock);
    // we will use only 1 number created of bgTypeId and bracket_id
    uint32 schedule_id = (bgQueueTypeId << 16) | (bgTypeId << 8) | bracket_id;
    bool found = false;
    for (unsigned long long i : m_queueUpdateScheduler)
    {
        if (i == schedule_id)
        {
            found = true;
            break;
        }
    }
    if (!found)
        m_queueUpdateScheduler.push_back(schedule_id);
}

uint32 BattleGroundMgr::GetPrematureFinishTime() const
{
    return sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

/**
  Method that loads battlemaster entries from DB
*/
void BattleGroundMgr::LoadBattleMastersEntry()
{
    m_battleMastersMap.clear();                              // need for reload case

    QueryResult* result = WorldDatabase.Query("SELECT entry,bg_template FROM battlemaster_entry");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 battlemaster entries - table is empty!");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        ++count;
        bar.step();

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
        {
            sLog.outErrorDb("Table `battlemaster_entry` contain entry %u for nonexistent battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        m_battleMastersMap[entry] = BattleGroundTypeId(bgTypeId);
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u battlemaster entries", count);
    sLog.outString();
}

/**
  Function that returns the holiday id from battleground type id

  @param    battleground type id
*/
HolidayIds BattleGroundMgr::BgTypeToWeekendHolidayId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_WS: return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_AB: return HOLIDAY_CALL_TO_ARMS_AB;
        default: return HOLIDAY_NONE;
    }
}

/**
  Function that returns the battleground type id from holiday id

  @param    holiday id
*/
BattleGroundTypeId BattleGroundMgr::WeekendHolidayIdToBgType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV: return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_WS: return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_AB: return BATTLEGROUND_AB;
        default: return BATTLEGROUND_TYPE_NONE;
    }
}

/**
  Function that checks if it's an active battleground holiday weekend

  @param    battleground type id
*/
bool BattleGroundMgr::IsBgWeekend(BattleGroundTypeId bgTypeId)
{
    return sGameEventMgr.IsActiveHoliday(BgTypeToWeekendHolidayId(bgTypeId));
}

/**
  Method that loads battleground events used in battleground scripts
*/
void BattleGroundMgr::LoadBattleEventIndexes()
{
    BattleGroundEventIdx events;
    events.event1 = BG_EVENT_NONE;
    events.event2 = BG_EVENT_NONE;
    m_gameObjectBattleEventIndexMap.clear();             // need for reload case
    m_gameObjectBattleEventIndexMap[static_cast<uint32>(-1)] = events;
    m_creatureBattleEventIndexMap.clear();               // need for reload case
    m_creatureBattleEventIndexMap[static_cast<uint32>(-1)] = events;

    uint32 count = 0;

    QueryResult* result =
        //                           0         1           2                3                4              5           6
        WorldDatabase.Query("SELECT data.typ, data.guid1, data.ev1 AS ev1, data.ev2 AS ev2, data.map AS m, data.guid2, description.map, "
                            //                              7                  8                   9
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "RIGHT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            // full outer join doesn't work in mysql :-/ so just UNION-select the same again and add a left outer join
                            "UNION "
                            "SELECT data.typ, data.guid1, data.ev1, data.ev2, data.map, data.guid2, description.map, "
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "LEFT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            "ORDER BY m, ev1, ev2");
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded 0 battleground eventindexes.");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();
        Field* fields = result->Fetch();
        if (fields[2].GetUInt8() == BG_EVENT_NONE || fields[3].GetUInt8() == BG_EVENT_NONE)
            continue;                                       // we don't need to add those to the eventmap

        bool gameobject         = (fields[0].GetUInt8() == 1);
        uint32 dbTableGuidLow   = fields[1].GetUInt32();
        events.event1           = fields[2].GetUInt8();
        events.event2           = fields[3].GetUInt8();
        uint32 map              = fields[4].GetUInt32();

        uint32 desc_map = fields[6].GetUInt32();
        uint8 desc_event1 = fields[7].GetUInt8();
        uint8 desc_event2 = fields[8].GetUInt8();
        const char* description = fields[9].GetString();

        // checking for nullptr - through right outer join this will mean following:
        if (fields[5].GetUInt32() != dbTableGuidLow)
        {
            sLog.outErrorDb("BattleGroundEvent: %s with nonexistent guid %u for event: map:%u, event1:%u, event2:%u (\"%s\")",
                            (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2, description);
            continue;
        }

        // checking for nullptr - through full outer join this can mean 2 things:
        if (desc_map != map)
        {
            // there is an event missing
            if (dbTableGuidLow == 0)
            {
                sLog.outErrorDb("BattleGroundEvent: missing db-data for map:%u, event1:%u, event2:%u (\"%s\")", desc_map, desc_event1, desc_event2, description);
                continue;
            }
            // we have an event which shouldn't exist
            sLog.outErrorDb("BattleGroundEvent: %s with guid %u is registered, for a nonexistent event: map:%u, event1:%u, event2:%u",
                    (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2);
            continue;
        }

        if (gameobject)
            m_gameObjectBattleEventIndexMap[dbTableGuidLow] = events;
        else
            m_creatureBattleEventIndexMap[dbTableGuidLow] = events;

        ++count;
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u battleground eventindexes", count);
    sLog.outString();
    delete result;
}
