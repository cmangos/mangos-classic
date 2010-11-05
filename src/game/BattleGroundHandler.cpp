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
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Object.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGround.h"
#include "Language.h"
#include "ScriptCalls.h"
#include "World.h"

void WorldSession::HandleBattlemasterHelloOpcode(WorldPacket & recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    DEBUG_LOG("WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from %s", guid.GetString().c_str());

    Creature *pCreature = GetPlayer()->GetMap()->GetCreature(guid);

    if (!pCreature)
        return;

    if (!pCreature->isBattleMaster())                       // it's not battlemaster
        return;

    // Stop the npc if moving
    if (!pCreature->IsStopped())
        pCreature->StopMoving();

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(pCreature->GetEntry());

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
        return;

    if (!_player->GetBGAccessByLevel(bgTypeId))
    {
        // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattlegGroundList(guid, bgTypeId);
}

void WorldSession::SendBattlegGroundList( ObjectGuid guid, BattleGroundTypeId bgTypeId )
{
    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::HandleBattlemasterJoinOpcode( WorldPacket & recv_data )
{
    ObjectGuid guid;
    uint32 instanceId;
    uint32 mapId;
    uint8 joinAsGroup;
    bool isPremade = false;
    Group * grp;

    recv_data >> guid;                                      // battlemaster guid
    recv_data >> mapId;
    recv_data >> instanceId;                                // instance id, 0 if First Available selected
    recv_data >> joinAsGroup;                               // join as group

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if(bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype (%u) received. possible cheater? player guid %u",bgTypeId,_player->GetGUIDLow());
        return;
    }

    DEBUG_LOG( "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from %s", guid.GetString().c_str());

    // can do this, since it's battleground, not arena
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);

    // ignore if player is already in BG
    if (_player->InBattleGround())
        return;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isBattleMaster())                            // it's not battlemaster
        return;

    // get bg instance or bg template if instance not found
    BattleGround *bg = NULL;
    if (instanceId)
        bg = sBattleGroundMgr.GetBattleGroundThroughClientInstance(instanceId, bgTypeId);

    if (!bg && !(bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId)))
    {
        sLog.outError("Battleground: no available bg / template found");
        return;
    }

    BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel(bgTypeId);

    // check queue conditions
    if (!joinAsGroup)
    {
        // check Deserter debuff
        if (!_player->CanJoinToBattleground())
        {
            WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
            data << uint32(0xFFFFFFFE);
            _player->GetSession()->SendPacket(&data);
            return;
        }
        // check if already in queue
        if (_player->GetBattleGroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;
        // check if has free queue slots
        if (!_player->HasFreeBattleGroundQueueId())
            return;
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if (!grp)
            return;
        uint32 err = grp->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam());
        isPremade = sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH) &&
            (grp->GetMembersCount() >= bg->GetMinPlayersPerTeam());
        if (err != BG_JOIN_ERR_OK)
        {
            SendBattleGroundJoinError(err);
            return;
        }
    }
    // if we're here, then the conditions to join a bg are met. We can proceed in joining.

    // _player->GetGroup() was already checked, grp is already initialized
    GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId, isPremade);
    uint32 avgTime = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].GetAverageQueueWaitTime(ginfo, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
    if (joinAsGroup)
    {
        DEBUG_LOG("Battleground: the following players are joining as group:");
        for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->getSource();
            if(!member) continue;   // this should never happen

            uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId);           // add to queue

            // store entry point coords (same as leader entry point)
            member->SetBattleGroundEntryPoint(_player);

            WorldPacket data;
                                                            // send status packet (in queue)
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
            member->GetSession()->SendPacket(&data);
            sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->GetSession()->SendPacket(&data);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);
            DEBUG_LOG("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",bgQueueTypeId,bgTypeId,member->GetGUIDLow(), member->GetName());
        }
        DEBUG_LOG("Battleground: group end");
    }
    else
    {
        // already checked if queueSlot is valid, now just get it
        uint32 queueSlot = _player->AddBattleGroundQueueId(bgQueueTypeId);
        // store entry point coords
        _player->SetBattleGroundEntryPoint();

        WorldPacket data;
                                                            // send status packet (in queue)
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
        SendPacket(&data);

        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        DEBUG_LOG("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",bgQueueTypeId,bgTypeId,_player->GetGUIDLow(), _player->GetName());
    }
    sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
    if (!ginfo->IsInvitedToBGInstanceGUID)
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AnnounceWorld(ginfo, _player->GetGUID(), true);
}

void WorldSession::HandleBattleGroundPlayerPositionsOpcode( WorldPacket & /*recv_data*/ )
{
                                                            // empty opcode
    DEBUG_LOG("WORLD: Recvd MSG_BATTLEGROUND_PLAYER_POSITIONS Message");

    BattleGround *bg = _player->GetBattleGround();
    if(!bg)                                                 // can't be received if player not in battleground
        return;

    switch( bg->GetTypeID() )
    {
        case BATTLEGROUND_WS:
            {
                uint32 count1 = 0;                                  //always constant zero?
                uint32 count2 = 0;                                  //count of next fields

                Player *ali_plr = sObjectMgr.GetPlayer(((BattleGroundWS*)bg)->GetAllianceFlagPickerGUID());
                if (ali_plr)
                    ++count2;

                Player *horde_plr = sObjectMgr.GetPlayer(((BattleGroundWS*)bg)->GetHordeFlagPickerGUID());
                if (horde_plr)
                    ++count2;

                WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16*count1+16*count2));
                data << count1;                                     // alliance flag holders count - obsolete, now always 0
                /*for(uint8 i = 0; i < count1; ++i)
                {
                    data << uint64(0);                              // guid
                    data << (float)0;                               // x
                    data << (float)0;                               // y
                }*/
                data << count2;                                     // horde flag holders count - obsolete, now count of next fields
                if (ali_plr)
                {
                    data << (uint64)ali_plr->GetGUID();
                    data << (float)ali_plr->GetPositionX();
                    data << (float)ali_plr->GetPositionY();
                }
                if (horde_plr)
                {
                    data << (uint64)horde_plr->GetGUID();
                    data << (float)horde_plr->GetPositionX();
                    data << (float)horde_plr->GetPositionY();
                }

                SendPacket(&data);
            }
            break;
        case BATTLEGROUND_AB:
        case BATTLEGROUND_AV:
            {
                //for other BG types - send default
                WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4));
                data << uint32(0);
                data << uint32(0);
                SendPacket(&data);
            }
            break;
        default:
            //maybe it is sent also in arena - do nothing
            break;
    }
}

void WorldSession::HandlePVPLogDataOpcode( WorldPacket & /*recv_data*/ )
{
    DEBUG_LOG( "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    WorldPacket data;
    sBattleGroundMgr.BuildPvpLogDataPacket(&data, bg);
    SendPacket(&data);

    DEBUG_LOG( "WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattlefieldListOpcode( WorldPacket &recv_data )
{
    DEBUG_LOG( "WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    uint32 mapId;
    recv_data >> mapId;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype received.");
        return;
    }

    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, _player->GetObjectGuid(), _player, BattleGroundTypeId(bgTypeId));
    SendPacket( &data );
}

void WorldSession::HandleBattleFieldPortOpcode( WorldPacket &recv_data )
{
    DEBUG_LOG( "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");

    uint8 action;                                           // enter battle 0x1, leave queue 0x0
    uint32 mapId;

    recv_data >> mapId >> action;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype (%u) received.",bgTypeId);
        // update battleground slots for the player to fix his UI and sent data.
        // this is a HACK, I don't know why the client starts sending invalid packets in the first place.
        // it usually happens with extremely high latency (if debugging / stepping in the code for example)
        if (_player->InBattleGroundQueue())
        {
            // update all queues, send invitation info if player is invited, queue info if queued
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
            {
                BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
                if (!bgQueueTypeId)
                    continue;

                BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
                BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
                BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
                // if the player is not in queue, continue or no group information - this should never happen
                if (itrPlayerStatus == qpMap.end() || !itrPlayerStatus->second.GroupInfo)
                    continue;

                BattleGround * bg = NULL;
                // get possibly needed data from groupinfo
                uint8 status = 0;

                if (!itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID)
                {
                    // not invited to bg, get template
                    bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
                    status = STATUS_WAIT_QUEUE;
                }
                else
                {
                    // get the bg we're invited to
                    bg = sBattleGroundMgr.GetBattleGround(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID, bgTypeId);
                    status = STATUS_WAIT_JOIN;
                }

                // if bg not found, then continue, don't invite if already in the instance
                if (!bg || (_player->InBattleGround() && _player->GetBattleGround() && _player->GetBattleGround()->GetInstanceID() == bg->GetInstanceID()))
                    continue;

                // re - invite player with proper data
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, i, status, INVITE_ACCEPT_WAIT_TIME, 0);
                SendPacket(&data);
            }
        }
        return;
    }

    // get the bg what we were invited to
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);
    BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
    BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
    if (itrPlayerStatus == qpMap.end())
    {
        sLog.outError("Battleground: itrplayerstatus not found.");
        return;
    }

    uint32 instanceId = itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID;
    // if action == 1, then instanceId is required
    if (!instanceId && action == 1)
    {
        sLog.outError("Battleground: instance not found.");
        return;
    }

    BattleGround *bg = sBattleGroundMgr.GetBattleGround(instanceId, bgTypeId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if (!bg && action == 0)
        bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
    {
        sLog.outError("BattlegroundHandler: bg_template not found for type id %u.", bgTypeId);
        return;
    }

    if (_player->InBattleGroundQueue())
    {
        //we must use temporary variables, because GroupQueueInfo pointer can be deleted in BattleGroundQueue::RemovePlayer() function!
        uint32 team = itrPlayerStatus->second.GroupInfo->Team;
        //some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
        if (action == 1)
        {
            //if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
            if (!_player->CanJoinToBattleground())
            {
                //send bg command result to show nice message
                WorldPacket data2(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
                data2 << uint32(0xFFFFFFFE);
                _player->GetSession()->SendPacket(&data2);
                action = 0;
                sLog.outDebug("Battleground: player %s (%u) has a deserter debuff, do not port him to battleground!", _player->GetName(), _player->GetGUIDLow());
            }
            //if player don't match battleground max level, then do not allow him to enter! (this might happen when player leveled up during his waiting in queue
            if (_player->getLevel() > bg->GetMaxLevel())
            {
                sLog.outError("Battleground: Player %s (%u) has level higher than maxlevel of battleground! Do not port him to battleground!", _player->GetName(), _player->GetGUIDLow());
                action = 0;
            }
        }
        uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
        WorldPacket data;
        switch( action )
        {
            case 1:                                         // port to battleground
                if (!_player->IsInvitedForBattleGroundQueueType(bgQueueTypeId))
                    return;                                 // cheating?
                // resurrect the player
                if (!_player->isAlive())
                {
                    _player->ResurrectPlayer(1.0f);
                    _player->SpawnCorpseBones();
                }
                // stop taxi flight at port
                if (_player->IsTaxiFlying())
                {
                    _player->GetMotionMaster()->MovementExpired();
                    _player->m_taxi.ClearTaxiDestinations();
                }

                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                _player->GetSession()->SendPacket(&data);
                // remove battleground queue status from BGmgr
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), false);
                // this is still needed here if battleground "jumping" shouldn't add deserter debuff
                // also this is required to prevent stuck at old battleground after SetBattleGroundId set to new
                if (BattleGround *currentBg = _player->GetBattleGround())
                    currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

                // set the destination instance id
                _player->SetBattleGroundId(bg->GetInstanceID(), bgTypeId);
                // set the destination team
                _player->SetBGTeam(team);
                // bg->HandleBeforeTeleportToBattleGround(_player);
                sBattleGroundMgr.SendToBattleGround(_player, instanceId, bgTypeId);
                // add only in HandleMoveWorldPortAck()
                // bg->AddPlayer(_player,team);
                DEBUG_LOG("Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetInstanceID(), bg->GetTypeID(), bgQueueTypeId);
                break;
            case 0:                                         // leave queue
                _player->RemoveBattleGroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, queueSlot, STATUS_NONE, 0, 0);
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), true);
                // player left queue, we should update it
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
                SendPacket(&data);
                DEBUG_LOG("Battleground: player %s (%u) left queue for bgtype %u, queue type %u.",_player->GetName(),_player->GetGUIDLow(),bg->GetTypeID(),bgQueueTypeId);
                break;
            default:
                sLog.outError("Battleground port: unknown action %u", action);
                break;
        }
    }
}

void WorldSession::HandleLeaveBattlefieldOpcode( WorldPacket& recv_data )
{
    DEBUG_LOG( "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");

    recv_data.read_skip<uint8>();                           // unk1
    recv_data.read_skip<uint8>();                           // BattleGroundTypeId-1 ?
    recv_data.read_skip<uint16>();                          // unk2 0

    //if(bgTypeId >= MAX_BATTLEGROUND_TYPES)                  // cheating? but not important in this case
    //    return;

    // not allow leave battleground in combat
    if (_player->isInCombat())
        if (BattleGround* bg = _player->GetBattleGround())
            if (bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

    _player->LeaveBattleground();
}

void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & /*recv_data*/ )
{
    // empty opcode
    DEBUG_LOG( "WORLD: Battleground status" );

    WorldPacket data;
    // we must update all queues here
    BattleGround *bg = NULL;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;

        BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
        if (bgTypeId == _player->GetBattleGroundTypeId())
        {
            bg = _player->GetBattleGround();
            //so i must use bg pointer to get that information
            if (bg)
            {
                // this line is checked, i only don't know if GetStartTime is changing itself after bg end!
                // send status in BattleGround
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, i, STATUS_IN_PROGRESS, bg->GetEndTime(), bg->GetStartTime());
                SendPacket(&data);
                continue;
            }
        }
        //we are sending update to player about queue - he can be invited there!
        //get GroupQueueInfo for queue status
        BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
        BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
        if (itrPlayerStatus == qpMap.end())
            continue;
        if (itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID)
        {
            bg = sBattleGroundMgr.GetBattleGround(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID, bgTypeId);
            if (!bg)
                continue;

            uint32 remainingTime = getMSTimeDiff(getMSTime(), itrPlayerStatus->second.GroupInfo->RemoveInviteTime);
            // send status invited to BattleGround
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, i, STATUS_WAIT_JOIN, remainingTime, 0);
            SendPacket(&data);
        }
        else
        {
            bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            if (!bg)
                continue;
            uint32 avgTime = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].GetAverageQueueWaitTime(itrPlayerStatus->second.GroupInfo, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
            // send status in BattleGround Queue
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, i, STATUS_WAIT_QUEUE, avgTime, getMSTimeDiff(itrPlayerStatus->second.GroupInfo->JoinTime, getMSTime()));
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    ObjectGuid guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if(!unit->isSpiritService())                            // it's not spirit service
        return;

    unit->SendAreaSpiritHealerQueryOpcode(GetPlayer());
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    ObjectGuid guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if(!unit->isSpiritService())                            // it's not spirit service
        return;

    Script->GossipHello(GetPlayer(), unit);
}

void WorldSession::SendBattleGroundJoinError(uint8 err)
{
    WorldPacket data;
    int32 msg;
    switch (err)
    {
        case BG_JOIN_ERR_OFFLINE_MEMBER:
            msg = LANG_BG_GROUP_OFFLINE_MEMBER;
            break;
        case BG_JOIN_ERR_GROUP_TOO_MANY:
            msg = LANG_BG_GROUP_TOO_LARGE;
            break;
        case BG_JOIN_ERR_MIXED_FACTION:
            msg = LANG_BG_GROUP_MIXED_FACTION;
            break;
        case BG_JOIN_ERR_MIXED_LEVELS:
            msg = LANG_BG_GROUP_MIXED_LEVELS;
            break;
        case BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE:
            msg = LANG_BG_GROUP_MEMBER_ALREADY_IN_QUEUE;
            break;
        case BG_JOIN_ERR_GROUP_DESERTER:
            msg = LANG_BG_GROUP_MEMBER_DESERTER;
            break;
        case BG_JOIN_ERR_ALL_QUEUES_USED:
            msg = LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS;
            break;
        case BG_JOIN_ERR_GROUP_NOT_ENOUGH:
        //case BG_JOIN_ERR_MIXED_ARENATEAM:
        default:
            return;
            break;
    }
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, GetMangosString(msg), NULL);
    SendPacket(&data);
    return;
}
