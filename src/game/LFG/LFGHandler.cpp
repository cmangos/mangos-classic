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
#include "Tools/Language.h"
#include "DBScripts/ScriptMgr.h"
#include "World/World.h"
#include "Groups/Group.h"
#include "LFG/LFGMgr.h"

void WorldSession::HandleMeetingStoneJoinOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;

    recv_data >> guid;

    DEBUG_LOG("WORLD: Recvd CMSG_MEETINGSTONE_JOIN Message guid: %s", guid.GetString().c_str());

    // ignore for remote control state
    if (!_player->IsSelfMover())
        return;

    GameObject* obj = _player->GetGameObjectIfCanInteractWith(guid);

    if (!obj)
        return;

    // Never expect this opcode for some type GO's
    if (obj->GetGoType() != GAMEOBJECT_TYPE_MEETINGSTONE)
    {
        sLog.outError("HandleMeetingStoneJoinOpcode: CMSG_MEETINGSTONE_JOIN for not allowed GameObject type %u (Entry %u), didn't expect this to happen.", obj->GetGoType(), obj->GetEntry());
        return;
    }

    if (Group* grp = _player->GetGroup())
    {
        if (!grp->IsLeader(_player->GetObjectGuid()))
        {
            SendMeetingstoneFailed(MEETINGSTONE_FAIL_PARTYLEADER);

            return;
        }

        if (grp->IsRaidGroup())
        {
            SendMeetingstoneFailed(MEETINGSTONE_FAIL_RAID_GROUP);
            return;
        }

        if (grp->IsFull())
        {
            SendMeetingstoneFailed(MEETINGSTONE_FAIL_FULL_GROUP);
            return;
        }
    }


   GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(obj->GetEntry());

   sLFGMgr.AddToQueue(_player, gInfo->meetingstone.areaID);
}

void WorldSession::HandleMeetingStoneLeaveOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: Recvd CMSG_MEETINGSTONE_LEAVE");
    if (Group* grp = _player->GetGroup())
    {
        if (grp->IsLeader(_player->GetObjectGuid()) && grp->IsInLFG())
        {
            sWorld.GetLFGQueue().GetMessager().AddMessage([groupId = grp->GetId()](LFGQueue* queue)
            {
                queue->RemoveGroupFromQueue(groupId);
            });
        }
        else
        {
            SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);
        }
    }
    else
    {
        sWorld.GetLFGQueue().GetMessager().AddMessage([playerGuid = _player->GetObjectGuid()](LFGQueue* queue)
        {
            queue->RemovePlayerFromQueue(playerGuid);
        });
    }
}

void WorldSession::HandleMeetingStoneInfoOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: Received CMSG_MEETING_STONE_INFO");

    if (Group* grp = _player->GetGroup())
    {
        if (grp->IsInLFG())
        {
            SendMeetingstoneSetqueue(grp->GetLFGAreaId(), MEETINGSTONE_STATUS_JOINED_QUEUE);
        }
        else
        {
            SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);
        }
    }
    else
    {
        if (!_player || !_player->GetSession())
            return;

        sWorld.GetLFGQueue().GetMessager().AddMessage([playerGuid = _player->GetObjectGuid()](LFGQueue* queue)
        {
            queue->RestoreOfflinePlayer(playerGuid);
        });
    }
}

void WorldSession::SendMeetingstoneFailed(uint8 status)
{
    WorldPacket data(SMSG_MEETINGSTONE_JOINFAILED, 1);
    data << uint8(status);
    SendPacket(data);
}

void WorldSession::SendMeetingstoneSetqueue(uint32 areaid, uint8 status)
{
    WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << uint32(areaid);
    data << uint8(status);
    SendPacket(data);
}
