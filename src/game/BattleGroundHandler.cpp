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

void WorldSession::HandleBattleMasterHelloOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid),GuidHigh2TypeId(GUID_HIPART(guid)));

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if(!unit->isBattleMaster())                             // it's not battlemaster
        return;

    // Stop the npc if moving
    unit->StopMoving();

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(unit->GetEntry());

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

void WorldSession::SendBattlegGroundList( uint64 guid, BattleGroundTypeId bgTypeId )
{
    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::HandleBattlemasterJoinOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    uint32 instanceId;
    uint32 mapId;
    uint8 joinAsGroup;
    Group * grp;

    recv_data >> guid;                                      // battlemaster guid
    recv_data >> mapId;                                     // battleground type id (DBC id)
    recv_data >> instanceId;                                // instance id, 0 if First Available selected
    recv_data >> joinAsGroup;                               // join as group

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if(bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype (%u) received. possible cheater? player guid %u",bgTypeId,_player->GetGUIDLow());
        return;
    }

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    // can do this, since it's battleground, not arena
    uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);

    // ignore if player is already in BG
    if(_player->InBattleGround())
        return;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if(!unit->isBattleMaster())                             // it's not battlemaster
        return;

    // get bg instance or bg template if instance not found
    BattleGround *bg = NULL;
    if(instanceId)
        bg = sBattleGroundMgr.GetBattleGround(instanceId);

    if(!bg && !(bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId)))
    {
        sLog.outError("Battleground: no available bg / template found");
        return;
    }

    BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel();

    // check queueing conditions
    if(!joinAsGroup)
    {
        // check Deserter debuff
        if( !_player->CanJoinToBattleground() )
        {
            WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
            data << (uint32) 0xFFFFFFFE;
            _player->GetSession()->SendPacket(&data);
            return;
        }
        // check if already in queue
        if (_player->GetBattleGroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;
        // check if has free queue slots
        if(!_player->HasFreeBattleGroundQueueId())
            return;
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if(!grp)
            return;
        uint32 err = grp->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), 0);
        if (err != BG_JOIN_ERR_OK)
        {
            SendBattleGroundJoinError(err);
            return;
        }
    }
    // if we're here, then the conditions to join a bg are met. We can proceed in joining.

    // _player->GetGroup() was already checked, grp is already initialized
    if(joinAsGroup /* && _player->GetGroup()*/)
    {
        sLog.outDebug("Battleground: the following players are joining as group:");
        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId);
        for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->getSource();
            if(!member) continue;   // this should never happen

            uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId);           // add to queue

            // store entry point coords (same as leader entry point)
            member->SetBattleGroundEntryPoint(_player);

            WorldPacket data;
                                                            // send status packet (in queue)
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, member->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0);
            member->GetSession()->SendPacket(&data);
            sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->GetSession()->SendPacket(&data);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);
            sLog.outDebug("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",bgQueueTypeId,bgTypeId,member->GetGUIDLow(), member->GetName());
        }
        sLog.outDebug("Battleground: group end");
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel());
        if(!ginfo->IsInvitedToBGInstanceGUID)
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AnnounceWorld(ginfo, _player->GetGUID(), true);
    }
    else
    {
        // already checked if queueSlot is valid, now just get it
        uint32 queueSlot = _player->AddBattleGroundQueueId(bgQueueTypeId);
        // store entry point coords
        _player->SetBattleGroundEntryPoint();

        WorldPacket data;
                                                            // send status packet (in queue)
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0);
        SendPacket(&data);

        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId);
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel());
        if(!ginfo->IsInvitedToBGInstanceGUID)
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AnnounceWorld(ginfo, _player->GetGUID(), true);
        sLog.outDebug("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",bgQueueTypeId,bgTypeId,_player->GetGUIDLow(), _player->GetName());
    }
}

void WorldSession::HandleBattleGroundPlayerPositionsOpcode( WorldPacket & /*recv_data*/ )
{
                                                            // empty opcode
    sLog.outDebug("WORLD: Recvd MSG_BATTLEGROUND_PLAYER_POSITIONS Message");

    BattleGround *bg = _player->GetBattleGround();
    if(!bg)                                                 // can't be received if player not in battleground
        return;

    if(bg->GetTypeID() == BATTLEGROUND_WS)
    {
        uint32 count1 = 0;
        uint32 count2 = 0;

        Player *ap = sObjectMgr.GetPlayer(((BattleGroundWS*)bg)->GetAllianceFlagPickerGUID());
        if(ap) ++count2;

        Player *hp = sObjectMgr.GetPlayer(((BattleGroundWS*)bg)->GetHordeFlagPickerGUID());
        if(hp) ++count2;

        WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16*count1+16*count2));
        data << count1;                                     // alliance flag holders count
        /*for(uint8 i = 0; i < count1; i++)
        {
            data << uint64(0);                              // guid
            data << (float)0;                               // x
            data << (float)0;                               // y
        }*/
        data << count2;                                     // horde flag holders count
        if(ap)
        {
            data << (uint64)ap->GetGUID();
            data << (float)ap->GetPositionX();
            data << (float)ap->GetPositionY();
        }
        if(hp)
        {
            data << (uint64)hp->GetGUID();
            data << (float)hp->GetPositionX();
            data << (float)hp->GetPositionY();
        }

        SendPacket(&data);
    }
}

void WorldSession::HandlePVPLogDataOpcode( WorldPacket & /*recv_data*/ )
{
    sLog.outDebug( "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    BattleGround *bg = _player->GetBattleGround();
    if(!bg)
        return;

    WorldPacket data;
    sBattleGroundMgr.BuildPvpLogDataPacket(&data, bg);
    SendPacket(&data);

    sLog.outDebug( "WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattlefieldListOpcode( WorldPacket &recv_data )
{
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    uint32 mapId;
    recv_data >> mapId;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if(bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype received.");
        return;
    }

    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, _player->GetGUID(), _player, BattleGroundTypeId(bgTypeId));
    SendPacket( &data );
}

void WorldSession::HandleBattleFieldPortOpcode( WorldPacket &recv_data )
{
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");

    uint8 action;                                           // enter battle 0x1, leave queue 0x0
	uint32 mapId;
    recv_data >> mapId >> action;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if(bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype (%u) received.",bgTypeId);
        // update battleground slots for the player to fix his UI and sent data.
        // this is a HACK, I don't know why the client starts sending invalid packets in the first place.
        // it usually happens with extremely high latency (if debugging / stepping in the code for example)
        if(_player->InBattleGroundQueue())
        {
            // update all queues, send invitation info if player is invited, queue info if queued
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
            {
                uint32 queue_id = _player->GetBattleGroundQueueId(i);
                if(!queue_id)
                    continue;
                BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].find(_player->GetGUID());
                // if the player is not in queue, continue
                if(itrPlayerStatus == sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].end())
                    continue;

                // no group information, this should never happen
                if(!itrPlayerStatus->second.GroupInfo)
                    continue;

                BattleGround * bg = NULL;

                // get possibly needed data from groupinfo
                BattleGroundTypeId bgTypeId = itrPlayerStatus->second.GroupInfo->BgTypeId;
                uint8 status = 0;


                if(!itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID)
                {
                    // not invited to bg, get template
                    bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
                    status = STATUS_WAIT_QUEUE;
                }
                else
                {
                    // get the bg we're invited to
                    bg = sBattleGroundMgr.GetBattleGround(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID);
                    status = STATUS_WAIT_JOIN;
                }

                // if bg not found, then continue
                if(!bg)
                    continue;

                // don't invite if already in the instance
                if(_player->InBattleGround() && _player->GetBattleGround() && _player->GetBattleGround()->GetInstanceID() == bg->GetInstanceID())
                    continue;

                // re - invite player with proper data
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, itrPlayerStatus->second.GroupInfo->Team?itrPlayerStatus->second.GroupInfo->Team:_player->GetTeam(), i, status, INVITE_ACCEPT_WAIT_TIME, 0);
                SendPacket(&data);
            }
        }
        return;
    }

    // get the bg what we were invited to
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId);
    BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus =
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].find(_player->GetGUID());

    if(itrPlayerStatus == sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].end())
    {
        sLog.outError("Battleground: itrplayerstatus not found.");
        return;
    }

    uint32 instanceId = itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID;

    // if action == 1, then instanceId is _required_
    if(!instanceId && action == 1)
    {
        sLog.outError("Battleground: instance not found.");
        return;
    }

    BattleGround *bg = sBattleGroundMgr.GetBattleGround(instanceId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if(!bg && action == 0)
        bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);

    if(!bg)
    {
        sLog.outError("Battleground: bg not found for type id %u.",bgTypeId);
        return;
    }

    bgTypeId = bg->GetTypeID();

    if(_player->InBattleGroundQueue())
    {
        uint32 queueSlot = 0;
        uint32 team = 0;
        // get the team info from the queue
        BattleGroundQueue::QueuedPlayersMap::iterator pitr = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].find(_player->GetGUID());
        if(pitr !=sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].end()
            && pitr->second.GroupInfo )
        {
            team = pitr->second.GroupInfo->Team;
        }
        else
        {
            sLog.outError("Battleground: Invalid player queue info!");
            return;
        }
        WorldPacket data;
        switch(action)
        {
            case 1:                                     // port to battleground
                if(!_player->IsInvitedForBattleGroundQueueType(bgQueueTypeId))
                    return;                                     // cheating?
                // resurrect the player
                if(!_player->isAlive())
                {
                    _player->ResurrectPlayer(1.0f);
                    _player->SpawnCorpseBones();
                }
                // stop taxi flight at port
                if(_player->isInFlight())
                {
                    _player->GetMotionMaster()->MovementExpired();
                    _player->m_taxi.ClearTaxiDestinations();
                }
                _player->RemoveFromGroup();
                queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                _player->GetSession()->SendPacket(&data);
                // remove battleground queue status from BGmgr
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), false);
                // this is still needed here if battleground "jumping" shouldn't add deserter debuff
                // also this required to prevent stuck at old battleground after SetBattleGroundId set to new
                if( BattleGround *currentBg = _player->GetBattleGround() )
                    currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

                // set the destination instance id
                _player->SetBattleGroundId(bg->GetInstanceID());
                // set the destination team
                _player->SetBGTeam(team);
                // bg->HandleBeforeTeleportToBattleGround(_player);
                sBattleGroundMgr.SendToBattleGround(_player, instanceId);
                // add only in HandleMoveWorldPortAck()
                // bg->AddPlayer(_player,team);
                sLog.outDebug("Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.",_player->GetName(),_player->GetGUIDLow(),bg->GetInstanceID(),bg->GetTypeID(),bgQueueTypeId);
                break;
            case 0:                                         // leave queue
                queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
                _player->RemoveBattleGroundQueueId(bgQueueTypeId); // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_NONE, 0, 0);
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), true);
                // player left queue, we should update it, maybe now his group fits in
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId,_player->GetBattleGroundBracketIdFromLevel());
                SendPacket(&data);
                sLog.outDebug("Battleground: player %s (%u) left queue for bgtype %u, queue type %u.",_player->GetName(),_player->GetGUIDLow(),bg->GetTypeID(),bgQueueTypeId);
                break;
            default:
                sLog.outError("Battleground port: unknown action %u", action);
                break;
        }
    }
}

void WorldSession::HandleLeaveBattlefieldOpcode( WorldPacket& recv_data )
{
    sLog.outDebug( "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");

    recv_data.read_skip<uint8>();                           // unk1
    recv_data.read_skip<uint8>();                           // BattleGroundTypeId-1 ?
    recv_data.read_skip<uint16>();                          // unk2 0

    //if(bgTypeId >= MAX_BATTLEGROUND_TYPES)                  // cheating? but not important in this case
    //    return;

    // not allow leave battleground in combat
    if(_player->isInCombat())
        if(BattleGround* bg = _player->GetBattleGround())
            if(bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

    _player->LeaveBattleground();
}

void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & /*recv_data*/ )
{
    // empty opcode
    sLog.outDebug( "WORLD: Battleground status" );

    WorldPacket data;

    // TODO: we must put player back to battleground in case disconnect (< 5 minutes offline time) or teleport player on login(!) from battleground map to entry point
    if(_player->InBattleGround())
    {
        BattleGround *bg = _player->GetBattleGround();
        if(bg)
        {
            uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID());
            uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
            if((bg->GetStatus() <= STATUS_IN_PROGRESS))
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                SendPacket(&data);
            }
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
            {
                uint32 queue_id = _player->GetBattleGroundQueueId(i);       // battlegroundqueueid stores the type id, not the instance id, so this is definitely wrong
                if (i == queueSlot || !queue_id)                            // we need to get the instance ids
                    continue;
                BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].find(_player->GetGUID());
                if(itrPlayerStatus == sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].end())
                    continue;
                BattleGround *bg2 = sBattleGroundMgr.GetBattleGroundTemplate(BattleGroundMgr::BGTemplateId(queue_id)); //  try this
                if(bg2)
                {
                    //in this call is small bug, this call should be filled by player's waiting time in queue
                    //this call nulls all timers for client :
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg2, _player->GetTeam(), i, STATUS_WAIT_QUEUE, 0, 0);
                    SendPacket(&data);
                }
            }
        }
    }
    else
    {
        // we should update all queues? .. i'm not sure if this code is correct
        for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            uint32 queue_id = _player->GetBattleGroundQueueId(i);
            if(!queue_id)
                continue;

            BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(queue_id);
            BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].find(_player->GetGUID());
            if(itrPlayerStatus == sBattleGroundMgr.m_BattleGroundQueues[queue_id].m_QueuedPlayers[_player->GetBattleGroundBracketIdFromLevel()].end())
                continue;
            if(bg && queue_id)
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), i, STATUS_WAIT_QUEUE, 0, 0);
                SendPacket(&data);
            }
        }
    }
/*    else              // not sure if it needed...
    {
        for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, NULL, _player->GetTeam(),i , STATUS_NONE, 0, 0);
            SendPacket(&data);
        }
    }*/
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");

    BattleGround *bg = _player->GetBattleGround();
    if(!bg)
        return;

    uint64 guid;
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
    sLog.outDebug("WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");

    BattleGround *bg = _player->GetBattleGround();
    if(!bg)
        return;

    uint64 guid;
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
        case BG_JOIN_ERR_MIXED_ARENATEAM:
        default:
            return;
            break;
    }
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, GetMangosString(msg), NULL);
    SendPacket(&data);
    return;
}
