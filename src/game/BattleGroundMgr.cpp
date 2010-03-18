/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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
#include "SharedDefines.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundWS.h"
#include "MapManager.h"
#include "Map.h"
#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include "Chat.h"
#include "World.h"
#include "WorldPacket.h"
#include "ProgressBar.h"
#include "Language.h"

#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( BattleGroundMgr );

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattleGroundQueue::BattleGroundQueue()
{
    //queues are empty, we don't have to call clear()
/*  for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        //m_QueuedPlayers[i].Horde = 0;
        //m_QueuedPlayers[i].Alliance = 0;
        //m_QueuedPlayers[i].AverageTime = 0;
    }*/
}

BattleGroundQueue::~BattleGroundQueue()
{
    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        m_QueuedPlayers[i].clear();
        for(QueuedGroupsList::iterator itr = m_QueuedGroups[i].begin(); itr!= m_QueuedGroups[i].end(); ++itr)
        {
            delete (*itr);
        }
        m_QueuedGroups[i].clear();
    }
}

// initialize eligible groups from the given source matching the given specifications
void BattleGroundQueue::EligibleGroups::Init(BattleGroundQueue::QueuedGroupsList *source, BattleGroundTypeId BgTypeId, uint32 side, uint32 MaxPlayers)
{
    // clear from prev initialization
    clear();
    BattleGroundQueue::QueuedGroupsList::iterator itr, next;
    // iterate through the source
    for(itr = source->begin(); itr!= source->end(); itr = next)
    {
        next = itr;
        ++next;
        if( (*itr)->BgTypeId == BgTypeId &&     // bg type must match
            (*itr)->IsInvitedToBGInstanceGUID == 0 && // leave out already invited groups
            (*itr)->Team == side &&             // match side
            (*itr)->Players.size() <= MaxPlayers)           // the group must fit in the bg
        {
            // the group matches the conditions
            // insert it in order of groupsize, and join time
            uint32 size = (*itr)->Players.size();
            uint32 jointime = (*itr)->JoinTime;
            bool inserted = false;

            for(std::list<GroupQueueInfo *>::iterator elig_itr = begin(); elig_itr != end(); ++elig_itr)
            {
                // if the next one's size is smaller, then insert
                // also insert if the next one's size is equal, but it joined the queue later
                if( ((*elig_itr)->Players.size()<size) ||
                    ((*elig_itr)->Players.size() == size && (*elig_itr)->JoinTime > jointime) )
                {
                    insert(elig_itr,(*itr));
                    inserted = true;
                    break;
                }
            }
            // if not inserted -> this is the smallest group -> push_back
            if(!inserted)
            {
                push_back((*itr));
            }
        }
    }
}

// remove group from eligible groups
// used when building selection pools
void BattleGroundQueue::EligibleGroups::RemoveGroup(GroupQueueInfo * ginfo)
{
    for(std::list<GroupQueueInfo *>::iterator itr = begin(); itr != end(); ++itr)
    {
        if((*itr)==ginfo)
        {
            erase(itr);
            return;
        }
    }
}

// selection pool initialization, used to clean up from prev selection
void BattleGroundQueue::SelectionPool::Init()
{
    SelectedGroups.clear();
    MaxGroup = 0;
    PlayerCount = 0;
}

// get the maximal group from the selection pool
// used when building the pool, and have to remove the largest
GroupQueueInfo * BattleGroundQueue::SelectionPool::GetMaximalGroup()
{
    if(SelectedGroups.empty())
    {
        sLog.outError("Getting max group when selection pool is empty, this should never happen.");
        MaxGroup = NULL;
        return 0;
    }
    // actually select the max group if it's not set
    if(MaxGroup==0 && !SelectedGroups.empty())
    {
        uint32 max_size = 0;
        for(std::list<GroupQueueInfo *>::iterator itr = SelectedGroups.begin(); itr != SelectedGroups.end(); ++itr)
        {
            if(max_size<(*itr)->Players.size())
            {
                MaxGroup =(*itr);
                max_size = MaxGroup->Players.size();
            }
        }
    }
    return MaxGroup;
}

// remove group info from selection pool
// used when building selection pools and have to remove maximal group
void BattleGroundQueue::SelectionPool::RemoveGroup(GroupQueueInfo *ginfo)
{
    // uninitiate max group info if needed
    if(MaxGroup == ginfo)
        MaxGroup = 0;
    // find what to remove
    for(std::list<GroupQueueInfo *>::iterator itr = SelectedGroups.begin(); itr != SelectedGroups.end(); ++itr)
    {
        if((*itr)==ginfo)
        {
            SelectedGroups.erase(itr);
            // decrease selected players count
            PlayerCount -= ginfo->Players.size();
            return;
        }
    }
}

// add group to selection
// used when building selection pools
void BattleGroundQueue::SelectionPool::AddGroup(GroupQueueInfo * ginfo)
{
    SelectedGroups.push_back(ginfo);
    // increase selected players count
    PlayerCount+=ginfo->Players.size();
    if(!MaxGroup || ginfo->Players.size() > MaxGroup->Players.size())
    {
        // update max group info if needed
        MaxGroup = ginfo;
    }
}

/*********************************************************/
/***               BATTLEGROUND QUEUES                 ***/
/*********************************************************/

// add group to bg queue with the given leader and bg specifications
GroupQueueInfo * BattleGroundQueue::AddGroup(Player *leader, BattleGroundTypeId BgTypeId, BattleGroundBracketId bracket_id)
{
    // create new ginfo
    // cannot use the method like in addplayer, because that could modify an in-queue group's stats
    // (e.g. leader leaving queue then joining as individual again)
    GroupQueueInfo* ginfo = new GroupQueueInfo;
    ginfo->BgTypeId                  = BgTypeId;
    ginfo->IsInvitedToBGInstanceGUID = 0;                       // maybe this should be modifiable by function arguments to enable selection of running instances?
    ginfo->JoinTime                  = getMSTime();
    ginfo->Team                      = leader->GetTeam();

    ginfo->Players.clear();

    m_QueuedGroups[bracket_id].push_back(ginfo);

    // return ginfo, because it is needed to add players to this group info
    return ginfo;
}

void BattleGroundQueue::AddPlayer(Player *plr, GroupQueueInfo *ginfo)
{
    uint32 bracket_id = plr->GetBattleGroundBracketIdFromLevel();

    //if player isn't in queue, he is added, if already is, then values are overwritten, no memory leak
    PlayerQueueInfo& info = m_QueuedPlayers[bracket_id][plr->GetGUID()];
    info.InviteTime                 = 0;
    info.LastInviteTime             = 0;
    info.LastOnlineTime             = getMSTime();
    info.GroupInfo                  = ginfo;

    // add the pinfo to ginfo's list
    ginfo->Players[plr->GetGUID()]  = &info;
}

void BattleGroundQueue::RemovePlayer(const uint64& guid, bool decreaseInvitedCount)
{
    Player *plr = sObjectMgr.GetPlayer(guid);

    int32 bracket_id = 0;                                   // signed for proper for-loop finish
    QueuedPlayersMap::iterator itr;
    GroupQueueInfo * group;
    QueuedGroupsList::iterator group_itr;
    bool IsSet = false;
    if(plr)
    {
        bracket_id = plr->GetBattleGroundBracketIdFromLevel();

        itr = m_QueuedPlayers[bracket_id].find(guid);
        if(itr != m_QueuedPlayers[bracket_id].end())
            IsSet = true;
    }

    if(!IsSet)
    {
        // either player is offline, or he levelled up to another queue category
        // sLog.outError("Battleground: removing offline player from BG queue - this might not happen, but it should not cause crash");
        for (uint32 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
        {
            itr = m_QueuedPlayers[i].find(guid);
            if(itr != m_QueuedPlayers[i].end())
            {
                bracket_id = i;
                IsSet = true;
                break;
            }
        }
    }

    // couldn't find the player in bg queue, return
    if(!IsSet)
    {
        sLog.outError("Battleground: couldn't find player to remove.");
        return;
    }

    sLog.outDebug("BattleGroundQueue: Removing player GUID %u, from bracket_id %u", GUID_LOPART(guid), (uint32)bracket_id);

    group = itr->second.GroupInfo;

    for(group_itr=m_QueuedGroups[bracket_id].begin(); group_itr != m_QueuedGroups[bracket_id].end(); ++group_itr)
    {
        if(group == (GroupQueueInfo*)(*group_itr))
            break;
    }

    // variables are set (what about leveling up when in queue????)
    // remove player from group
    // if only player there, remove group

    // remove player queue info from group queue info
    std::map<uint64, PlayerQueueInfo*>::iterator pitr = group->Players.find(guid);

    if(pitr != group->Players.end())
        group->Players.erase(pitr);

    // check for iterator correctness
    if (group_itr != m_QueuedGroups[bracket_id].end() && itr != m_QueuedPlayers[bracket_id].end())
    {
        // used when player left the queue, NOT used when porting to bg
        if (decreaseInvitedCount)
        {
            // if invited to bg, and should decrease invited count, then do it
            if(group->IsInvitedToBGInstanceGUID)
            {
                BattleGround* bg = sBattleGroundMgr.GetBattleGround(group->IsInvitedToBGInstanceGUID);
                if (bg)
                    bg->DecreaseInvitedCount(group->Team);
                if (bg && !bg->GetPlayersSize() && !bg->GetInvitedCount(ALLIANCE) && !bg->GetInvitedCount(HORDE))
                {
                    // no more players on battleground, set delete it
                    bg->SetDeleteThis();
                }
            }
            // update the join queue, maybe now the player's group fits in a queue!
            // not yet implemented (should store bgTypeId in group queue info?)
        }
        // remove player queue info
        m_QueuedPlayers[bracket_id].erase(itr);
        // remove group queue info if needed

        //if we left BG queue(not porting) OR if arena team left queue for rated match
        if(decreaseInvitedCount)
            AnnounceWorld(group, guid, false);

        if(group->Players.empty())
        {
            m_QueuedGroups[bracket_id].erase(group_itr);
            delete group;
        }
    }
}

void BattleGroundQueue::AnnounceWorld(GroupQueueInfo *ginfo, const uint64& playerGUID, bool isAddedToQueue)
{
    if( sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN) )
    {
        Player *plr = sObjectMgr.GetPlayer(playerGUID);
        if(!plr)
            return;

        BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(ginfo->BgTypeId);
        if(!bg)
            return;

        BattleGroundBracketId bracket_id = plr->GetBattleGroundBracketIdFromLevel();
        char const* bgName = bg->GetName();

        uint32 q_min_level = Player::GetMinLevelForBattleGroundBracketId(bracket_id);
        uint32 q_max_level = Player::GetMaxLevelForBattleGroundBracketId(bracket_id);

        // replace hardcoded max level by player max level for nice output
        if(q_max_level > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            q_max_level = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

        int8 MinPlayers = bg->GetMinPlayersPerTeam();

        uint8 qHorde = 0;
        uint8 qAlliance = 0;

        BattleGroundTypeId bgTypeId = ginfo->BgTypeId;
        QueuedPlayersMap::iterator itr;
        for(itr = m_QueuedPlayers[bracket_id].begin(); itr!= m_QueuedPlayers[bracket_id].end(); ++itr)
        {
            if(itr->second.GroupInfo->BgTypeId == bgTypeId)
            {
                switch(itr->second.GroupInfo->Team)
                {
                case HORDE:
                    qHorde++; break;
                case ALLIANCE:
                    qAlliance++; break;
                default:
                    break;
                }
            }
        }

        // Show queue status to player only (when joining queue)
        if(sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_QUEUE_ANNOUNCER_JOIN)==1)
        {
            ChatHandler(plr).PSendSysMessage(LANG_BG_QUEUE_ANNOUNCE_SELF,
                bgName, q_min_level, q_max_level, qAlliance, MinPlayers, qHorde, MinPlayers);
        }
        // System message
        else
        {
            sWorld.SendWorldText(LANG_BG_QUEUE_ANNOUNCE_WORLD,
                bgName, q_min_level, q_max_level, qAlliance, MinPlayers, qHorde, MinPlayers);
        }
    }
}

bool BattleGroundQueue::InviteGroupToBG(GroupQueueInfo * ginfo, BattleGround * bg, uint32 side)
{
    // set side if needed
    if(side)
        ginfo->Team = side;

    if(!ginfo->IsInvitedToBGInstanceGUID)
    {
        // not yet invited
        // set invitation
        ginfo->IsInvitedToBGInstanceGUID = bg->GetInstanceID();
        BattleGroundTypeId bgTypeId = bg->GetTypeID();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);

        // loop through the players
        for(std::map<uint64,PlayerQueueInfo*>::iterator itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            // set status
            itr->second->InviteTime = getMSTime();
            itr->second->LastInviteTime = getMSTime();

            // get the player
            Player* plr = sObjectMgr.GetPlayer(itr->first);
            // if offline, skip him
            if(!plr)
                continue;

            // invite the player
            sBattleGroundMgr.InvitePlayer(plr, bg->GetInstanceID(),ginfo->Team);

            WorldPacket data;

            uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);

            sLog.outDebug("Battleground: invited plr %s (%u) to BG instance %u queueindex %u bgtype %u, I can't help it if they don't press the enter battle button.",plr->GetName(),plr->GetGUIDLow(),bg->GetInstanceID(),queueSlot,bg->GetTypeID());

            // send status packet
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, side?side:plr->GetTeam(), queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0);
            plr->GetSession()->SendPacket(&data);
        }
        return true;
    }

    return false;
}

// this function is responsible for the selection of queued groups when trying to create new battlegrounds
bool BattleGroundQueue::BuildSelectionPool(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint32 MinPlayers, uint32 MaxPlayers,  SelectionPoolBuildMode mode)
{
    uint32 side;
    switch(mode)
    {
        case NORMAL_ALLIANCE:
        case ONESIDE_ALLIANCE_TEAM1:
        case ONESIDE_ALLIANCE_TEAM2:
            side = ALLIANCE;
            break;
        case NORMAL_HORDE:
        case ONESIDE_HORDE_TEAM1:
        case ONESIDE_HORDE_TEAM2:
            side = HORDE;
            break;
        default:
            //unknown mode, return false
            sLog.outDebug("Battleground: unknown selection pool build mode, returning...");
            return false;
    }

    // inititate the groups eligible to create the bg
    m_EligibleGroups.Init(&(m_QueuedGroups[bracket_id]), bgTypeId, side, MaxPlayers);
    // init the selected groups (clear)
    m_SelectionPools[mode].Init();
    while(!(m_EligibleGroups.empty()))
    {
        sLog.outDebug("m_EligibleGroups is not empty, continue building selection pool");
        // in decreasing group size, add groups to join if they fit in the MaxPlayersPerTeam players
        for(EligibleGroups::iterator itr= m_EligibleGroups.begin(); itr!=m_EligibleGroups.end(); ++itr)
        {
            // get the maximal not yet checked group
            GroupQueueInfo * MaxGroup = (*itr);
            // if it fits in the maxplayer size, add it
            if( (m_SelectionPools[mode].GetPlayerCount() + MaxGroup->Players.size()) <= MaxPlayers )
            {
                m_SelectionPools[mode].AddGroup(MaxGroup);
            }
        }
        if(m_SelectionPools[mode].GetPlayerCount()>=MinPlayers)
        {
            // the selection pool is set, return
            sLog.outDebug("pool build succeeded, return true");
            return true;
        }
        // if the selection pool's not set, then remove the group with the highest player count, and try again with the rest.
        GroupQueueInfo * MaxGroup = m_SelectionPools[mode].GetMaximalGroup();
        m_EligibleGroups.RemoveGroup(MaxGroup);
        m_SelectionPools[mode].RemoveGroup(MaxGroup);
    }
    // failed to build a selection pool matching the given values
    return false;
}

// used to remove the Enter Battle window if the battle has already, but someone still has it
// (this can happen in arenas mainly, since the preparation is shorter than the timer for the bgqueueremove event
void BattleGroundQueue::BGEndedRemoveInvites(BattleGround *bg)
{
    BattleGroundBracketId bracket_id = bg->GetBracketId();
    uint32 bgInstanceId = bg->GetInstanceID();
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID());
    QueuedGroupsList::iterator itr, next;
    for(itr = m_QueuedGroups[bracket_id].begin(); itr != m_QueuedGroups[bracket_id].end(); itr = next)
    {
        // must do this way, because the groupinfo will be deleted when all playerinfos are removed
        GroupQueueInfo * ginfo = (*itr);
        next = itr;
        ++next;
        // if group was invited to this bg instance, then remove all references
        if(ginfo->IsInvitedToBGInstanceGUID == bgInstanceId)
        {
            // after removing this much playerinfos, the ginfo will be deleted, so we'll use a for loop
            uint32 to_remove = ginfo->Players.size();
            uint32 team = ginfo->Team;
            for(uint32 i = 0; i < to_remove; ++i)
            {
                // always remove the first one in the group
                std::map<uint64, PlayerQueueInfo * >::iterator itr2 = ginfo->Players.begin();
                if(itr2 == ginfo->Players.end())
                {
                    sLog.outError("Empty Players in ginfo, this should never happen!");
                    return;
                }

                // get the player
                Player * plr = sObjectMgr.GetPlayer(itr2->first);
                if(!plr)
                {
                    sLog.outError("Player offline when trying to remove from GroupQueueInfo, this should never happen.");
                    continue;
                }

                // get the queueslot
                uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
                if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
                {
                    plr->RemoveBattleGroundQueueId(bgQueueTypeId);
                    // remove player from queue, this might delete the ginfo as well! don't use that pointer after this!
                    RemovePlayer(itr2->first, true);
                    // this is probably unneeded, since this player was already invited -> does not fit when initing eligible groups
                    // but updateing the queue can't hurt
                    Update(bg->GetTypeID(), bg->GetBracketId());
                    // send info to client
                    WorldPacket data;
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, team, queueSlot, STATUS_NONE, 0, 0);
                    plr->GetSession()->SendPacket(&data);
                }
            }
        }
    }
}

/*
this method is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while(true) cycles to invite whole queue
it must be called after fully adding the members of a group to ensure group joining
should be called after removeplayer functions in some cases
*/
void BattleGroundQueue::Update(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    if(bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
    {
        sLog.outError("BattleGroundQueue::Update() non existing BG Type %u", bgTypeId);
        return;
    }

    if (bracket_id >= MAX_BATTLEGROUND_BRACKETS)
    {
        //this is error, that caused crashes (not in , but now it shouldn't)
        sLog.outError("BattleGroundQueue::Update() called for non existing queue type - this can cause crash, pls report problem, if this is the last line of error log before crash");
        return;
    }

    //if no players in queue ... do nothing
    if (m_QueuedGroups[bracket_id].empty())
        return;

    uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);

    //battleground with free slot for player should be always the last in this queue
    BGFreeSlotQueueType::iterator itr, next;
    for (itr = sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].end(); itr = next)
    {
        next = itr;
        ++next;
        // battleground is running, so if:
        if ((*itr)->GetTypeID() == bgTypeId && (*itr)->GetBracketId() == bracket_id && (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
        {
            //we must check both teams
            BattleGround* bg = *itr; //we have to store battleground pointer here, because when battleground is full, it is removed from free queue (not yet implemented!!)
            // and iterator is invalid

            for(QueuedGroupsList::iterator itr2 = m_QueuedGroups[bracket_id].begin(); itr2 != m_QueuedGroups[bracket_id].end(); ++itr2)
            {
                // did the group join for this bg type?
                if((*itr2)->BgTypeId != bgTypeId)
                    continue;
                // if so, check if fits in
                if(bg->GetFreeSlotsForTeam((*itr2)->Team) >= (*itr2)->Players.size())
                {
                    // if group fits in, invite it
                    InviteGroupToBG((*itr2),bg,(*itr2)->Team);
                }
            }

            if (!bg->HasFreeSlots())
            {
                //remove BG from BGFreeSlotQueue
                bg->RemoveFromBGFreeSlotQueue();
            }
        }
    }

    // finished iterating through the bgs with free slots, maybe we need to create a new bg

    BattleGround * bg_template = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if(!bg_template)
    {
        sLog.outError("Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    // get the min. players per team
    uint32 MinPlayersPerTeam = bg_template->GetMinPlayersPerTeam();
    uint32 MaxPlayersPerTeam = bg_template->GetMaxPlayersPerTeam();

    if(sBattleGroundMgr.isTesting())
    {
        MinPlayersPerTeam = 1;
    }

    // try to build the selection pools
    bool bAllyOK = BuildSelectionPool(bgTypeId, bracket_id, MinPlayersPerTeam, MaxPlayersPerTeam, NORMAL_ALLIANCE);
    if(bAllyOK)
        sLog.outDebug("Battleground: ally pool succesfully build");
    else
        sLog.outDebug("Battleground: ally pool wasn't created");
    bool bHordeOK = BuildSelectionPool(bgTypeId, bracket_id, MinPlayersPerTeam, MaxPlayersPerTeam, NORMAL_HORDE);
    if(bHordeOK)
        sLog.outDebug("Battleground: horde pool succesfully built");
    else
        sLog.outDebug("Battleground: horde pool wasn't created");

    // if selection pools are ready, create the new bg
    if ((bAllyOK && bHordeOK) || ( sBattleGroundMgr.isTesting() && (bAllyOK || bHordeOK)))
    {
        // create new battleground
        BattleGround * bg2 = sBattleGroundMgr.CreateNewBattleGround(bgTypeId);
        if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_QUEUE_ANNOUNCER_START))
        {
            char const* bgName = bg2->GetName();
            uint32 q_min_level = Player::GetMinLevelForBattleGroundBracketId(bracket_id);
            uint32 q_max_level = Player::GetMaxLevelForBattleGroundBracketId(bracket_id);
            if(q_max_level > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
                q_max_level = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);
            sWorld.SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, bgName, q_min_level, q_max_level);
        }

        if(!bg2)
        {
            sLog.outError("Battleground: couldn't create bg %u",bgTypeId);
            return;
        }

        // start the joining of the bg
        bg2->SetStatus(STATUS_WAIT_JOIN);
        bg2->SetBracketId(bracket_id);

        std::list<GroupQueueInfo* >::iterator itr;

        // invite groups from horde selection pool
        for(itr = m_SelectionPools[NORMAL_HORDE].SelectedGroups.begin(); itr != m_SelectionPools[NORMAL_HORDE].SelectedGroups.end(); ++itr)
        {
            InviteGroupToBG((*itr),bg2,HORDE);
        }

        // invite groups from ally selection pools
        for(itr = m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.begin(); itr != m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.end(); ++itr)
        {
            InviteGroupToBG((*itr),bg2,ALLIANCE);
        }

        // start the battleground
        bg2->StartBattleGround();
    }

    // there weren't enough players for a "normal" match
}

/*********************************************************/
/***            BATTLEGROUND QUEUE EVENTS              ***/
/*********************************************************/

bool BGQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = sObjectMgr.GetPlayer( m_PlayerGuid );

    // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
    if (!plr)
        return true;

    // Player can be in another BG queue and must be removed in normal way in any case
    // // player is already in battleground ... do nothing (battleground queue status is deleted when player is teleported to BG)
    // if (plr->GetBattleGroundId() > 0)
    //    return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID);
    if (!bg)
        return true;

    uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID());
    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue
    {
        // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
        BattleGroundQueue::QueuedPlayersMap const& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattleGroundBracketIdFromLevel()];
        BattleGroundQueue::QueuedPlayersMap::const_iterator qItr = qpMap.find(m_PlayerGuid);
        if (qItr != qpMap.end() && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
        {
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, qItr->second.GroupInfo->Team, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME/2, 0);
            plr->GetSession()->SendPacket(&data);
        }
    }
    return true;                                            //event will be deleted
}

void BGQueueInviteEvent::Abort(uint64 /*e_time*/)
{
    //this should not be called
    sLog.outError("Battleground invite event ABORTED!");
}

bool BGQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = sObjectMgr.GetPlayer( m_PlayerGuid );
    if (!plr)
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID);
    if (!bg)
        return true;

    sLog.outDebug("Battleground: removing player %u from bg queue for instance %u because of not pressing enter battle in time.",plr->GetGUIDLow(),m_BgInstanceGUID);

    uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID());
    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
    {
        // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
        BattleGroundQueue::QueuedPlayersMap::iterator qMapItr = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattleGroundBracketIdFromLevel()].find(m_PlayerGuid);
        if (qMapItr != sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattleGroundBracketIdFromLevel()].end() && qMapItr->second.GroupInfo && qMapItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
        {
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(m_PlayerGuid, true);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bg->GetTypeID(),bg->GetBracketId());
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, m_PlayersTeam, queueSlot, STATUS_NONE, 0, 0);
            plr->GetSession()->SendPacket(&data);
        }
    }
    else
        sLog.outDebug("Battleground: Player was already removed from queue");

    //event will be deleted
    return true;
}

void BGQueueRemoveEvent::Abort(uint64 /*e_time*/)
{
    //this should not be called
    sLog.outError("Battleground remove event ABORTED!");
}

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattleGroundMgr::BattleGroundMgr()
{
    m_BattleGrounds.clear();
    m_Testing=false;
}

BattleGroundMgr::~BattleGroundMgr()
{
    DeleteAlllBattleGrounds();
}

void BattleGroundMgr::DeleteAlllBattleGrounds()
{
    for(BattleGroundSet::iterator itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end();)
    {
        BattleGround * bg = itr->second;
        m_BattleGrounds.erase(itr++);
        delete bg;
    }

    // destroy template battlegrounds that listed only in queues (other already terminated)
    for(uint32 bgTypeId = 0; bgTypeId < MAX_BATTLEGROUND_TYPE_ID; ++bgTypeId)
    {
        // ~BattleGround call unregistring BG from queue
        while(!BGFreeSlotQueue[bgTypeId].empty())
            delete BGFreeSlotQueue[bgTypeId].front();
    }
}

// used to update running battlegrounds, and delete finished ones
void BattleGroundMgr::Update(uint32 diff)
{
    BattleGroundSet::iterator itr, next;
    for(itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end(); itr = next)
    {
        next = itr;
        ++next;
        itr->second->Update(diff);
        // use the SetDeleteThis variable
        // direct deletion caused crashes
        if(itr->second->m_SetDeleteThis)
        {
            BattleGround * bg = itr->second;
            m_BattleGrounds.erase(itr);
            delete bg;
        }
    }
}

void BattleGroundMgr::BuildBattleGroundStatusPacket(WorldPacket *data, BattleGround *bg, uint32 team, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2)
{
    // we can be in 3 queues in same time...
    if(StatusID == 0)
    {
        data->Initialize(SMSG_BATTLEFIELD_STATUS, 4*2);
        *data << uint32(QueueSlot);                         // queue id (0...2)
        *data << uint32(0);
        return;
    }

    data->Initialize(SMSG_BATTLEFIELD_STATUS, (4+1+1+4+2+4+1+4+4+4));
    *data << uint32(QueueSlot);                             // queue id (0...2) - player can be in 3 queues in time
    // uint64 in client
    *data << uint32(bg->GetMapId());                          // MapID
    *data << uint8(0);                                        // Unknown
    *data << uint32(bg->GetInstanceID());                     // Instance ID
    *data << uint32(StatusID);                                // Status ID
    switch(StatusID)
    {
        case STATUS_WAIT_QUEUE:                             // status_in_queue
            *data << uint32(Time1);                         // average wait time, milliseconds
            *data << uint32(Time2);                         // time in queue, updated every minute?
            break;
        case STATUS_WAIT_JOIN:                              // status_invite
            *data << uint32(Time1);                         // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:                            // status_in_progress
            *data << uint32(Time1);                         // 0 at bg start, 120000 after bg end, time to bg auto leave, milliseconds
            *data << uint32(Time2);                         // time from bg start, milliseconds
            break;
        default:
            sLog.outError("Unknown BG status!");
            break;
    }
}

void BattleGroundMgr::BuildPvpLogDataPacket(WorldPacket *data, BattleGround *bg)
{
    data->Initialize(MSG_PVP_LOG_DATA, (1+4+40*bg->GetPlayerScoresSize()));

    if(bg->GetWinner() == 2)
    {
        *data << uint8(0);                                  // bg in progress
    }
    else
    {
        *data << uint8(1);                                  // bg ended
        *data << uint8(bg->GetWinner());                    // who win
    }

    *data << (uint32)(bg->GetPlayerScoresSize());

    for(BattleGround::BattleGroundScoreMap::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        *data << uint64(itr->first);

        Player *plr = sObjectMgr.GetPlayer(itr->first);

        *data << uint32(plr ? plr->GetHonorRank() : 0);
        *data << uint32(itr->second->KillingBlows);
        *data << uint32(itr->second->HonorableKills);
        *data << uint32(itr->second->Deaths);
        *data << uint32(itr->second->BonusHonor);

        switch(bg->GetTypeID())                              // battleground specific things
        {
            case BATTLEGROUND_AV:
                *data << (uint32)0x00000007;                // count of next fields
                *data << (uint32)((BattleGroundAVScore*)itr->second)->GraveyardsAssaulted;  // GraveyardsAssaulted
                *data << (uint32)((BattleGroundAVScore*)itr->second)->GraveyardsDefended;   // GraveyardsDefended
                *data << (uint32)((BattleGroundAVScore*)itr->second)->TowersAssaulted;      // TowersAssaulted
                *data << (uint32)((BattleGroundAVScore*)itr->second)->TowersDefended;       // TowersDefended
                *data << (uint32)((BattleGroundAVScore*)itr->second)->SecondaryObjectives;  // Mines Taken
                *data << (uint32)((BattleGroundAVScore*)itr->second)->LieutnantCount;       // Lieutnant kills
                *data << (uint32)((BattleGroundAVScore*)itr->second)->SecondaryNPC;         // Secondary unit summons
                break;
            case BATTLEGROUND_WS:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundWGScore*)itr->second)->FlagCaptures;         // flag captures
                *data << (uint32)((BattleGroundWGScore*)itr->second)->FlagReturns;          // flag returns
                break;
            case BATTLEGROUND_AB:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundABScore*)itr->second)->BasesAssaulted;       // bases asssulted
                *data << (uint32)((BattleGroundABScore*)itr->second)->BasesDefended;        // bases defended
                break;
            default:
                sLog.outDebug("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetTypeID());
                *data << (uint32)0;
                break;
        }
    }
}

void BattleGroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket *data, BattleGroundTypeId bgTypeId)
{
    /*bgTypeId is:
    0 - Your group has joined a battleground queue, but you are not eligible
    1 - Your group has joined the queue for AV
    2 - Your group has joined the queue for WS
    3 - Your group has joined the queue for AB
    4 - Your group has joined the queue for NA
    5 - Your group has joined the queue for BE Arena
    6 - Your group has joined the queue for All Arenas
    7 - Your group has joined the queue for EotS*/
    data->Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    *data << uint32(bgTypeId);
}

void BattleGroundMgr::BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4+4);
    *data << uint32(field);
    *data << uint32(value);
}

void BattleGroundMgr::BuildPlaySoundPacket(WorldPacket *data, uint32 soundid)
{
    data->Initialize(SMSG_PLAY_SOUND, 4);
    *data << uint32(soundid);
}

void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket *data, Player *plr)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    *data << uint64(plr->GetGUID());
}

void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket *data, Player *plr)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    *data << uint64(plr->GetGUID());
}

void BattleGroundMgr::InvitePlayer(Player* plr, uint32 bgInstanceGUID, uint32 team)
{
    // set invited player counters:
    BattleGround* bg = GetBattleGround(bgInstanceGUID);
    if(!bg)
        return;
    bg->IncreaseInvitedCount(team);

    plr->SetInviteForBattleGroundQueueType(BGQueueTypeId(bg->GetTypeID()), bgInstanceGUID);

    // create invite events:
    //add events to player's counters ---- this is not good way - there should be something like global event processor, where we should add those events
    BGQueueInviteEvent* inviteEvent = new BGQueueInviteEvent(plr->GetGUID(), bgInstanceGUID);
    plr->m_Events.AddEvent(inviteEvent, plr->m_Events.CalculateTime(INVITE_ACCEPT_WAIT_TIME/2));
    BGQueueRemoveEvent* removeEvent = new BGQueueRemoveEvent(plr->GetGUID(), bgInstanceGUID, team);
    plr->m_Events.AddEvent(removeEvent, plr->m_Events.CalculateTime(INVITE_ACCEPT_WAIT_TIME));
}

BattleGround * BattleGroundMgr::GetBattleGroundTemplate(BattleGroundTypeId bgTypeId)
{
    return BGFreeSlotQueue[bgTypeId].empty() ? NULL : BGFreeSlotQueue[bgTypeId].back();
}

// create a new battleground that will really be used to play
BattleGround * BattleGroundMgr::CreateNewBattleGround(BattleGroundTypeId bgTypeId)
{
    // get the template BG
    BattleGround *bg_template = GetBattleGroundTemplate(bgTypeId);
    if(!bg_template)
    {
        sLog.outError("BattleGround: CreateNewBattleGround - bg template not found for %u", bgTypeId);
        return NULL;
    }

    BattleGround *bg = NULL;

    // create a copy of the BG template
    switch(bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV(*(BattleGroundAV*)bg_template);
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS(*(BattleGroundWS*)bg_template);
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB(*(BattleGroundAB*)bg_template);
            break;
        default:
            //bg = new BattleGround;
            return 0;
            break;             // placeholder for non implemented BG
    }

    // generate a new instance id
    bg->SetInstanceID(sMapMgr.GenerateInstanceId()); // set instance id

    // reset the new bg (set status to status_wait_queue from status_none)
    bg->Reset();

    /*   will be setup in BG::Update() when the first player is ported in
    if(!(bg->SetupBattleGround()))
    {
        sLog.outError("BattleGround: CreateNewBattleGround: SetupBattleGround failed for bg %u", bgTypeId);
        delete bg;
        return 0;
    }
    */

    // add BG to free slot queue
    bg->AddToBGFreeSlotQueue();

    // add bg to update list
    AddBattleGround(bg->GetInstanceID(), bg);

    return bg;
}

// used to create the BG templates
uint32 BattleGroundMgr::CreateBattleGround(BattleGroundTypeId bgTypeId, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO)
{
    // Create the BG
    BattleGround *bg = NULL;

    switch(bgTypeId)
    {
        case BATTLEGROUND_AV: 
            bg = new BattleGroundAV; 
            bg->SetName("Alterac Valley");
            bg->SetMapId(30);
        break;
        case BATTLEGROUND_WS: 
            bg = new BattleGroundWS;
            bg->SetName("Warsong Gulch");
            bg->SetMapId(489);
        break;
        case BATTLEGROUND_AB: 
            bg = new BattleGroundAB;
            bg->SetName("Arathi Basin");
            bg->SetMapId(529);
        break;

        default:bg = new BattleGround;   break;             // placeholder for non implemented BG
    }

    bg->Reset();

    bg->SetTypeID(bgTypeId);
    bg->SetInstanceID(0);                               // template bg, instance id is 0
    bg->SetMinPlayersPerTeam(MinPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(MaxPlayersPerTeam);
    bg->SetMinPlayers(MinPlayersPerTeam*2);
    bg->SetMaxPlayers(MaxPlayersPerTeam*2);
    bg->SetTeamStartLoc(ALLIANCE, Team1StartLocX, Team1StartLocY, Team1StartLocZ, Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    Team2StartLocX, Team2StartLocY, Team2StartLocZ, Team2StartLocO);
    bg->SetLevelRange(LevelMin, LevelMax);

    //add BattleGround instance to FreeSlotQueue (.back() will return the template!)
    bg->AddToBGFreeSlotQueue();

    // do NOT add to update list, since this is a template battleground!

    // return some not-null value, bgTypeId is good enough for me
    return bgTypeId;
}

void BattleGroundMgr::CreateInitialBattleGrounds()
{
    float AStartLoc[4];
    float HStartLoc[4];
    uint32 MaxPlayersPerTeam, MinPlayersPerTeam, MinLvl, MaxLvl, start1, start2;
    WorldSafeLocsEntry const *start;

    uint32 count = 0;

    //                                                0   1                 2                 3      4      5                6              7             8
    QueryResult *result = WorldDatabase.Query("SELECT id, MinPlayersPerTeam,MaxPlayersPerTeam,MinLvl,MaxLvl,AllianceStartLoc,AllianceStartO,HordeStartLoc,HordeStartO FROM battleground_template");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        return;
    }

    barGoLink bar((int)result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 bgTypeID_ = fields[0].GetUInt32();

        BattleGroundTypeId bgTypeID = BattleGroundTypeId(bgTypeID_);

        MaxPlayersPerTeam = 0;
        MinPlayersPerTeam = 0;
        MinLvl = 0;
        MaxLvl = 0;

        if(fields[1].GetUInt32())
            MinPlayersPerTeam = fields[1].GetUInt32();

        if(fields[2].GetUInt32())
            MaxPlayersPerTeam = fields[2].GetUInt32();

        if(fields[3].GetUInt32())
            MinLvl = fields[3].GetUInt32();

        if(fields[4].GetUInt32())
            MaxLvl = fields[4].GetUInt32();

        start1 = fields[5].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry(start1);
        if(start)
        {
            AStartLoc[0] = start->x;
            AStartLoc[1] = start->y;
            AStartLoc[2] = start->z;
            AStartLoc[3] = fields[6].GetFloat();
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.",bgTypeID,start1);
            continue;
        }

        start2 = fields[7].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry(start2);
        if(start)
        {
            HStartLoc[0] = start->x;
            HStartLoc[1] = start->y;
            HStartLoc[2] = start->z;
            HStartLoc[3] = fields[8].GetFloat();
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.",bgTypeID,start2);
            continue;
        }

        //sLog.outDetail("Creating battleground %s, %u-%u", bl->name[sWorld.GetDBClang()], MinLvl, MaxLvl);
        if(!CreateBattleGround(bgTypeID, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]))
            continue;

        ++count;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u battlegrounds", count );
}

void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket *data, const uint64& guid, Player* plr, BattleGroundTypeId bgTypeId)
{
    if (!plr)
        return;

    uint32 PlayerLevel = plr->getLevel();

    uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    *data << uint64(guid);                              // battlemaster guid
    *data << uint32(mapId);                             // battleground id
    *data << uint8(0x00);                               // unk

    size_t count_pos = data->wpos();
    uint32 count = 0;
    *data << uint32(0x00);                              // number of bg instances

    for(std::map<uint32, BattleGround*>::iterator itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end(); ++itr)
    {
        if(itr->second->GetTypeID() == bgTypeId && (PlayerLevel >= itr->second->GetMinLevel()) && (PlayerLevel <= itr->second->GetMaxLevel()))
        {
            *data << uint32(itr->second->GetInstanceID());
            ++count;
        }
    }
    data->put<uint32>( count_pos , count);
}

void BattleGroundMgr::SendToBattleGround(Player *pl, uint32 instanceId)
{
    BattleGround *bg = GetBattleGround(instanceId);
    if(bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        uint32 team = pl->GetBGTeam();
        if(team==0)
            team = pl->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        sLog.outDetail("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", pl->GetName(), mapid, x, y, z, O);
        pl->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog.outError("player %u trying to port to non-existent bg instance %u",pl->GetGUIDLow(), instanceId);
    }
}

BattleGroundQueueTypeId BattleGroundMgr::BGQueueTypeId(BattleGroundTypeId bgTypeId)
{
    switch(bgTypeId)
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

BattleGroundTypeId BattleGroundMgr::BGTemplateId(uint32 bgQueueTypeId)
{
    switch(bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        default:
            return BattleGroundTypeId(0);                   // used for unknown template (it existed and do nothing)
    }
}

void BattleGroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if(m_Testing)
        sWorld.SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld.SendWorldText(LANG_DEBUG_BG_OFF);
}

uint32 BattleGroundMgr::GetPrematureFinishTime() const
{
    return sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

void BattleGroundMgr::LoadBattleMastersEntry()
{
    mBattleMastersMap.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.Query( "SELECT entry,bg_template FROM battlemaster_entry" );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 battlemaster entries - table is empty!" );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
        {
            sLog.outErrorDb("Table `battlemaster_entry` contain entry %u for not existed battleground type %u, ignored.",entry,bgTypeId);
            continue;
        }

        mBattleMastersMap[entry] = BattleGroundTypeId(bgTypeId);

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u battlemaster entries", count );
}

void BattleGroundMgr::LoadBattleEventIndexes()
{
    BattleGroundEventIdx events;
    events.event1 = BG_EVENT_NONE;
    events.event2 = BG_EVENT_NONE;
    m_GameObjectBattleEventIndexMap.clear();             // need for reload case
    m_GameObjectBattleEventIndexMap[-1] = events;
    m_CreatureBattleEventIndexMap.clear();               // need for reload case
    m_CreatureBattleEventIndexMap[-1] = events;

    uint32 count = 0;

    QueryResult *result =
        //                              0            1           2             3                     4           5           6
        WorldDatabase.PQuery( "SELECT data.typ, data.guid1, data.ev1 AS ev1, data.ev2 AS ev2, data.map AS m, data.guid2, description.map, "
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
                              "ORDER BY m, ev1, ev2" );
    if(!result)
    {
        barGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 battleground eventindexes.");
        return;
    }

    barGoLink bar((int)result->GetRowCount());

    do
    {
        bar.step();
        Field *fields = result->Fetch();
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
        const char *description = fields[9].GetString();

        // checking for NULL - through right outer join this will mean following:
        if (fields[5].GetUInt32() != dbTableGuidLow)
        {
            sLog.outErrorDb("BattleGroundEvent: %s with nonexistant guid %u for event: map:%u, event1:%u, event2:%u (\"%s\")",
                (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2, description);
            continue;
        }

        // checking for NULL - through full outer join this can mean 2 things:
        if (desc_map != map)
        {
            // there is an event missing
            if (dbTableGuidLow == 0)
            {
                sLog.outErrorDb("BattleGroundEvent: missing db-data for map:%u, event1:%u, event2:%u (\"%s\")", desc_map, desc_event1, desc_event2, description);
                continue;
            }
            // we have an event which shouldn't exist
            else
            {
                sLog.outErrorDb("BattleGroundEvent: %s with guid %u is registered, for a nonexistant event: map:%u, event1:%u, event2:%u",
                    (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2);
                continue;
            }
        }

        if (gameobject)
            m_GameObjectBattleEventIndexMap[dbTableGuidLow] = events;
        else
            m_CreatureBattleEventIndexMap[dbTableGuidLow] = events;

        ++count;

    } while(result->NextRow());

    sLog.outString();
    sLog.outString( ">> Loaded %u battleground eventindexes", count);
    delete result;
}
