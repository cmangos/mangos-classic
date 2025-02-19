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
#include "Server/WorldPacket.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
#include "Entities/Player.h"
#include "Globals/ObjectMgr.h"
#include "Server/WorldSession.h"
#include "Entities/Object.h"
#include "Chat/Chat.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGround.h"
#include "Tools/Language.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "World/World.h"

// Sent by client when player talk to the battle master
void WorldSession::HandleBattlemasterHelloOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    DEBUG_LOG("WORLD: Received opcode CMSG_BATTLEMASTER_HELLO from %s", guid.GetString().c_str());

    Creature* pCreature = GetPlayer()->GetMap()->GetCreature(guid);

    if (!pCreature)
        return;

    if (!pCreature->isBattleMaster())                       // it's not battlemaster
        return;

    // Stop the npc if moving
    if (uint32 pauseTimer = pCreature->GetInteractionPauseTimer())
        pCreature->GetMotionMaster()->PauseWaypoints(pauseTimer);

    BattleGroundTypeId bgTypeId = GetPlayer()->GetMap()->GetMapDataContainer().GetBattleMasterBG(pCreature->GetEntry());

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
        return;

    if (!_player->GetBGAccessByLevel(bgTypeId))
    {
        // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattleGroundList(guid, bgTypeId);
}

/**
Send battleground list

@param   guid
@param   bgTypeId
*/
void WorldSession::SendBattleGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId) const
{
    sWorld.GetBGQueue().GetMessager().AddMessage([playerGuid = _player->GetObjectGuid(), masterGuid = guid, playerLevel = _player->GetLevel(), bgTypeId](BattleGroundQueue* queue)
    {
        WorldPacket data;
        queue->BuildBattleGroundListPacket(data, masterGuid, playerLevel, BattleGroundTypeId(bgTypeId));
        sWorld.GetMessager().AddMessage([playerGuid, data](World* /*world*/)
        {
            if (Player* player = sObjectMgr.GetPlayer(playerGuid))
                player->GetSession()->SendPacket(data);
        });
    });
}

// Sent by client when player wants to join a battleground
void WorldSession::HandleBattlemasterJoinOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    uint32 instanceId;
    uint32 mapId;
    uint8 joinAsGroup;
    bool isPremade = false;
    Group* group = nullptr;

    recv_data >> guid;                                      // battlemaster guid
    recv_data >> mapId;
    recv_data >> instanceId;                                // instance id, 0 if First Available selected
    recv_data >> joinAsGroup;                               // join as group

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype (%u) received. possible cheater? player guid %u", bgTypeId, _player->GetGUIDLow());
        return;
    }

    DEBUG_LOG("WORLD: Received opcode CMSG_BATTLEMASTER_JOIN from %s", guid.GetString().c_str());

    // can do this, since it's battleground, not arena
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(bgTypeId);

    // ignore if player is already in BG
    if (_player->InBattleGround())
        return;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isBattleMaster())                            // it's not battlemaster
        return;

    // get bg instance or bg template if instance not found
    BattleGround* bg = nullptr;
    if (instanceId)
        bg = sBattleGroundMgr.GetBattleGroundThroughClientInstance(instanceId, bgTypeId);

    if (!bg)
        bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);

    if (!bg)
    {
        sLog.outError("Battleground: no available bg / template found");
        return;
    }

    mapId = bg->GetMapId();

    BattleGroundBracketId bgBracketId = sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, _player->GetLevel());

    // check queue conditions
    if (!joinAsGroup)
    {
        // check Deserter debuff
        if (!_player->CanJoinToBattleground())
        {
            WorldPacket data;
            sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(data, bgTypeId, BG_GROUP_JOIN_STATUS_DESERTERS);
            _player->GetSession()->SendPacket(data);
            return;
        }
        // check if already in queue
        if (_player->GetBattleGroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            // player is already in this queue
            return;
        // check if has free queue slots
        if (!_player->HasFreeBattleGroundQueueId())
            return;
    }
    else
    {
        group = _player->GetGroup();
        // no group found, error
        if (!group)
            return;
        uint32 err = group->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam());
        isPremade = sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH) &&
                    (group->GetMembersCount() >= bg->GetMinPlayersPerTeam());
        if (err != BG_JOIN_ERR_OK)
        {
            SendBattleGroundJoinError(err);
            return;
        }
    }
    // if we're here, then the conditions to join a bg are met. We can proceed in joining.

    // _player->GetGroup() was already checked, group is already initialized
    AddGroupToQueueInfo info;
    info.team = _player->GetTeam();
    info.clientInstanceId = instanceId;
    info.mapId = mapId;
    if (joinAsGroup)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            if (Player* member = itr->getSource())
                info.members.push_back(member->GetObjectGuid());
        DEBUG_LOG("Battleground: the following players are joining as group:");
        sWorld.GetBGQueue().GetMessager().AddMessage([bgQueueTypeId, leaderGuid = group->GetLeaderGuid(), info, bgTypeId, bgBracketId, isPremade, instanceId, mapId](BattleGroundQueue* queue)
        {
            BattleGroundQueueItem& queueItem = queue->GetBattleGroundQueue(bgQueueTypeId);
            GroupQueueInfo* groupInfo = queueItem.AddGroup(leaderGuid, info, bgTypeId, bgBracketId, isPremade, instanceId);
            uint32 avgTime = queueItem.GetAverageQueueWaitTime(groupInfo, bgBracketId);

            sWorld.GetMessager().AddMessage([leaderGuid, members = info.members, bgQueueTypeId, bgTypeId, bgClientInstanceId = instanceId, avgTime, mapId](World* /*world*/)
            {
                Player* leader = sObjectMgr.GetPlayer(leaderGuid);
                for (ObjectGuid guid : members)
                {
                    Player* member = sObjectMgr.GetPlayer(guid);
                    if (!member)
                        continue;

                    uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId);           // add to queue

                    // store entry point coords (same as leader entry point)
                    member->SetBattleGroundEntryPoint(leader);

                    // send status packet (in queue)
                    WorldPacket data;
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, bgTypeId, bgClientInstanceId, mapId, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
                    member->GetSession()->SendPacket(data);
                    sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(data, bgTypeId, BG_GROUP_JOIN_STATUS_SUCCESS);
                    member->GetSession()->SendPacket(data);
                    DEBUG_LOG("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUIDLow(), member->GetName());
                }

                DEBUG_LOG("Battleground: group end");
            });
            queue->ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, bgBracketId);
        });
    }
    else
    {
        sWorld.GetBGQueue().GetMessager().AddMessage([bgQueueTypeId, playerGuid = _player->GetObjectGuid(), info, bgTypeId, bgBracketId, isPremade, instanceId, mapId](BattleGroundQueue* queue)
        {
            BattleGroundQueueItem& queueItem = queue->GetBattleGroundQueue(bgQueueTypeId);
            GroupQueueInfo* groupInfo = queueItem.AddGroup(playerGuid, info, bgTypeId, bgBracketId, isPremade, instanceId);
            uint32 avgTime = queueItem.GetAverageQueueWaitTime(groupInfo, bgBracketId);
            sWorld.GetMessager().AddMessage([playerGuid, bgQueueTypeId, bgTypeId, bgClientInstanceId = instanceId, avgTime, mapId](World* /*world*/)
            {
                if (Player* player = sObjectMgr.GetPlayer(playerGuid))
                {
                    // already checked if queueSlot is valid, now just get it
                    uint32 queueSlot = player->AddBattleGroundQueueId(bgQueueTypeId);
                    // store entry point coords
                    player->SetBattleGroundEntryPoint();

                    WorldPacket data;
                    // send status packet (in queue)
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, bgTypeId, bgClientInstanceId, mapId, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
                    player->GetSession()->SendPacket(data);
                    DEBUG_LOG("Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, player->GetGUIDLow(), player->GetName());
                }
            });
            queue->ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, bgBracketId);
        });
    }
}

// Sent by client while inside battleground; depends on the battleground type
void WorldSession::HandleBattleGroundPlayerPositionsOpcode(WorldPacket& /*recv_data*/)
{
    // empty opcode
    DEBUG_LOG("WORLD: Received opcode MSG_BATTLEGROUND_PLAYER_POSITIONS");

    BattleGround* bg = _player->GetBattleGround();
    if (!bg)                                                // can't be received if player not in battleground
        return;

    switch (bg->GetTypeId())
    {
        case BATTLEGROUND_WS:
        {
            WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS);
            size_t countPos = data.wpos();
            uint32 count = 0;
            data << uint32(0);
            for (auto const& itr : bg->GetPlayers())
            {
                if (_player->GetTeam() == itr.second.playerTeam)
                {
                    if (Player const* player = sObjectMgr.GetPlayer(itr.first))
                    {
                        data << ObjectGuid(player->GetObjectGuid());
                        data << float(player->GetPositionX());
                        data << float(player->GetPositionY());
                        count++;
                    }
                }
            }
            data.put<uint32>(countPos, count);

            Player* flagCarrier;
            if (_player->GetTeam() == ALLIANCE)
                flagCarrier = sObjectMgr.GetPlayer(static_cast<BattleGroundWS*>(bg)->GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE));
            else
                flagCarrier = sObjectMgr.GetPlayer(static_cast<BattleGroundWS*>(bg)->GetFlagCarrierGuid(TEAM_INDEX_HORDE));

            data << uint8(flagCarrier ? 1 : 0);

            if (flagCarrier)
            {
                data << ObjectGuid(flagCarrier->GetObjectGuid());
                data << float(flagCarrier->GetPositionX());
                data << float(flagCarrier->GetPositionY());
            }

            SendPacket(data);
            break;
        }
        case BATTLEGROUND_AB:
        case BATTLEGROUND_AV:
        {
            // for other BG types - send default
            WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, 4 + 4);
            data << uint32(0);
            data << uint8(0);
            SendPacket(data);
            break;
        }
        case BATTLEGROUND_TYPE_NONE: break;
    }
}

// Sent by client when requesting the PvP match log
void WorldSession::HandlePVPLogDataOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: Received opcode MSG_PVP_LOG_DATA");

    BattleGround* bg = _player->GetBattleGround();
    if (!bg)
        return;

    WorldPacket data;
    sBattleGroundMgr.BuildPvpLogDataPacket(data, bg);
    SendPacket(data);

    DEBUG_LOG("WORLD: Sent MSG_PVP_LOG_DATA Message");
}

// Sent by client when requesting available battleground list
void WorldSession::HandleBattlefieldListOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_BATTLEFIELD_LIST");

    uint32 mapId;
    recv_data >> mapId;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("Battleground: invalid bgtype received.");
        return;
    }
   
    sWorld.GetBGQueue().GetMessager().AddMessage([playerGuid = _player->GetObjectGuid(), masterGuid = _player->GetObjectGuid(), playerLevel = _player->GetLevel(), bgTypeId](BattleGroundQueue* queue)
    {
        WorldPacket data;
        queue->BuildBattleGroundListPacket(data, masterGuid, playerLevel, BattleGroundTypeId(bgTypeId));
        sWorld.GetMessager().AddMessage([playerGuid, data](World* /*world*/)
        {
            if (Player* player = sObjectMgr.GetPlayer(playerGuid))
                player->GetSession()->SendPacket(data);
        });
    });
}

// Sent by client when requesting teleport to the battleground location
void WorldSession::HandleBattlefieldPortOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_BATTLEFIELD_PORT");

    uint8 action;                                           // enter battle 0x1, leave queue 0x0
    uint32 mapId;

    recv_data >> mapId >> action;

    BattleGroundTypeId bgTypeId = GetBattleGroundTypeIdByMapId(mapId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        sLog.outError("BattlegroundHandler: invalid bg map (%u) received.", mapId);
        return;
    }
    if (!_player->InBattleGroundQueue())
    {
        sLog.outError("BattlegroundHandler: Invalid CMSG_BATTLEFIELD_PORT received from player (%u), he is not in bg_queue.", _player->GetGUIDLow());
        return;
    }

    // get GroupQueueInfo from BattleGroundQueue
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(bgTypeId);
    bool canJoinToBg = _player->CanJoinToBattleground();
    uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);

    sWorld.GetBGQueue().GetMessager().AddMessage([bgQueueTypeId, playerGuid = _player->GetObjectGuid(), actionTemp = action, canJoinToBg, bgTypeId, playerLevel = _player->GetLevel(), queueSlot](BattleGroundQueue* queue)
    {
        uint8 action = actionTemp;
        BattleGroundQueueItem& queueItem = queue->GetBattleGroundQueue(bgQueueTypeId);
        GroupQueueInfo queueInfo;
        if (!queueItem.GetPlayerGroupInfoData(playerGuid, &queueInfo))
        {
            sLog.outError("BattlegroundHandler: itrplayerstatus not found.");
            return;
        }

        if (!queueInfo.isInvitedToBgInstanceGuid && action == 1)
        {
            sLog.outError("BattlegroundHandler: instance not found.");
            return;
        }

        // some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
        if (action == 1)
        {
            // if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
            if (!canJoinToBg)
            {
                // send bg command result to show nice message
                WorldPacket data2;
                sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(data2, bgTypeId, BG_GROUP_JOIN_STATUS_DESERTERS);
                sWorld.GetMessager().AddMessage([playerGuid, data2](World* /*world*/)
                {
                    if (Player* player = sObjectMgr.GetPlayer(playerGuid))
                    {
                        player->GetSession()->SendPacket(data2);
                    }
                });

                action = 0;
            }
        }

        switch (action)
        {
            case 1:                                         // port to battleground
            {
                BattleGroundInQueueInfo* bgInQueue = queue->GetFreeSlotInstance(bgTypeId, queueInfo.isInvitedToBgInstanceGuid);
                MANGOS_ASSERT(bgInQueue); // at this point must always exist

                // remove battleground queue status from BGmgr
                queueItem.RemovePlayer(*queue, playerGuid, false);

                sWorld.GetMessager().AddMessage([playerGuid, invitedTo = queueInfo.isInvitedToBgInstanceGuid, bgTypeId, bgQueueTypeId, groupTeam = queueInfo.groupTeam, queueSlot, bgClientInstanceId = bgInQueue->GetClientInstanceId(), mapId = bgInQueue->GetMapId()](World* /*world*/)
                {
                    Player* player = sObjectMgr.GetPlayer(playerGuid);
                    if (!player)
                        return;

                    // resurrect the player
                    if (!player->IsAlive())
                    {
                        player->ResurrectPlayer(1.0f);
                        player->SpawnCorpseBones();
                    }

                    player->TaxiFlightInterrupt();

                    uint32 startTime = 0;
                    if (BattleGround* bg = sBattleGroundMgr.GetBattleGround(invitedTo, bgTypeId))
                        startTime = bg->GetStartTime();

                    WorldPacket data;
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, bgTypeId, bgClientInstanceId, mapId, queueSlot, STATUS_IN_PROGRESS, 0, startTime);
                    player->GetSession()->SendPacket(data);

                    // this is still needed here if battleground "jumping" shouldn't add deserter debuff
                    // also this is required to prevent stuck at old battleground after SetBattleGroundId set to new
                    if (BattleGround* currentBg = player->GetBattleGround())
                        currentBg->RemovePlayerAtLeave(player->GetObjectGuid(), false, true);

                    // set the destination instance id
                    player->SetBattleGroundId(invitedTo, bgTypeId);
                    // set the destination team
                    player->SetBGTeam(groupTeam);

                    sBattleGroundMgr.SendToBattleGround(player, invitedTo, bgTypeId);

                    DEBUG_LOG("Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", player->GetName(), player->GetGUIDLow(), invitedTo, bgTypeId, bgQueueTypeId);
                });

                break;
            }
            case 0:                                         // leave queue
                queueItem.RemovePlayer(*queue, playerGuid, true);
                sWorld.GetMessager().AddMessage([playerGuid, bgQueueTypeId, queueSlot, bgTypeId, bgClientInstanceId = queueInfo.clientInstanceId, mapId = queueInfo.mapId](World* /*world*/)
                {
                    Player* player = sObjectMgr.GetPlayer(playerGuid);
                    if (!player)
                        return;
                    player->RemoveBattleGroundQueueId(bgQueueTypeId);
                    WorldPacket data;
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, bgTypeId, bgClientInstanceId, mapId, queueSlot, STATUS_NONE, 0, 0);
                    player->GetSession()->SendPacket(data);
                });

                queue->ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, queueInfo.bgBracketId);
                break;
            default:
                sLog.outError("Battleground port: unknown action %u", action);
                break;
        }
    });
}

// Sent by client when leaving the battleground
void WorldSession::HandleLeaveBattlefieldOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_LEAVE_BATTLEFIELD");
    uint64 guid;

    uint32 mapId;
    recv_data >> mapId;
    if (_player->GetMapId() != mapId)               // cheating? but not important in this case
        return;

    // not allow leave battleground in combat
    if (_player->IsInCombat())
        if (BattleGround* bg = _player->GetBattleGround())
            if (bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

    _player->LeaveBattleground();
}

// Sent by client when requesting battleground status
void WorldSession::HandleBattlefieldStatusOpcode(WorldPacket& /*recv_data*/)
{
    // empty opcode
    DEBUG_LOG("WORLD: Battleground status");

    WorldPacket data;
    // we must update all queues here
    BattleGround* bg;
    std::vector<std::pair<uint32, BattleGroundQueueTypeId>> idsToCheck;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;

        BattleGroundTypeId bgTypeId = BattleGroundMgr::BgTemplateId(bgQueueTypeId);
        if (bgTypeId == _player->GetBattleGroundTypeId())
        {
            bg = _player->GetBattleGround();
            // so i must use bg pointer to get that information
            if (bg)
            {
                // this line is checked, i only don't know if GetStartTime is changing itself after bg end!
                // send status in BattleGround
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, bg, bg->GetTypeId(), bg->GetClientInstanceId(), bg->GetMapId(), i, STATUS_IN_PROGRESS, bg->GetEndTime(), bg->GetStartTime());
                SendPacket(data);
                continue;
            }
        }

        idsToCheck.push_back({i, bgQueueTypeId});
        // we are sending update to player about queue - he can be invited there!
    }

    sWorld.GetBGQueue().GetMessager().AddMessage([idsToCheck, playerGuid = _player->GetObjectGuid(), playerLevel = _player->GetLevel()](BattleGroundQueue* queue)
    {
        for (auto [queueSlot, bgQueueTypeId] : idsToCheck)
        {
            BattleGroundQueueItem& queueItem = queue->GetBattleGroundQueue(bgQueueTypeId);
            GroupQueueInfo queueInfo;
            if (!queueItem.GetPlayerGroupInfoData(playerGuid, &queueInfo))
                continue;

            WorldPacket data;
            if (queueInfo.isInvitedToBgInstanceGuid)
            {
                uint32 remainingTime = WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), queueInfo.removeInviteTime);
                // send status invited to BattleGround
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, queueInfo.bgTypeId, queueInfo.clientInstanceId, queueInfo.mapId, queueSlot, STATUS_WAIT_JOIN, remainingTime, 0);
            }
            else
            {
                uint32 avgTime = queueItem.GetAverageQueueWaitTime(&queueInfo, queueInfo.bgBracketId);
                // send status in BattleGround Queue
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, queueInfo.bgTypeId, queueInfo.clientInstanceId, queueInfo.mapId, queueSlot, STATUS_WAIT_QUEUE, avgTime, WorldTimer::getMSTimeDiff(queueInfo.joinTime, WorldTimer::getMSTime()));
            }
            sWorld.GetMessager().AddMessage([playerGuid, data](World* /*world*/)
            {
                if (Player* player = sObjectMgr.GetPlayer(playerGuid))
                {
                    player->GetSession()->SendPacket(data);
                }
            });
        }
    });

}

// Sent by client when requesting the spirit healer
void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");

    BattleGround* bg = _player->GetBattleGround();
    if (!bg)
        return;

    ObjectGuid guid;
    recv_data >> guid;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                           // it's not spirit service
        return;

    unit->SendAreaSpiritHealerQueryOpcode(GetPlayer());
}

// Sent by client when requesting spirit healer queue
void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");

    BattleGround* bg = _player->GetBattleGround();
    if (!bg)
        return;

    ObjectGuid guid;
    recv_data >> guid;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                           // it's not spirit service
        return;

    sScriptDevAIMgr.OnGossipHello(GetPlayer(), unit);
}

void WorldSession::SendBattleGroundJoinError(uint8 err) const
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
        // case BG_JOIN_ERR_MIXED_ARENATEAM:
        default:
            return;
            break;
    }
    ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, GetMangosString(msg), LANG_UNIVERSAL);
    SendPacket(data);
    return;
}
