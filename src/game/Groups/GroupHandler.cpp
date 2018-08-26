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
#include "Database/DatabaseEnv.h"
#include "Server/Opcodes.h"
#include "Log.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/Player.h"
#include "Groups/Group.h"
#include "Social/SocialMgr.h"
#include "Util.h"

/* differeces from off:
    -you can uninvite yourself - is is useful
    -you can accept invitation even if leader went offline
*/
/* todo:
    -group_destroyed msg is sent but not shown
    -reduce xp gaining when in raid group
    -quest sharing has to be corrected
    -FIX sending PartyMemberStats
*/

void WorldSession::SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res) const
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, (4 + member.size() + 1 + 4));
    data << uint32(operation);
    data << member;                                         // max len 48
    data << uint32(res);

    SendPacket(data);
}

void WorldSession::HandleGroupInviteOpcode(WorldPacket& recv_data)
{
    std::string membername;
    recv_data >> membername;

    // attempt add selected player

    // cheating
    if (!normalizePlayerName(membername))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    Player* initiator = GetPlayer();
    Player* recipient = sObjectMgr.GetPlayer(membername.c_str());

    // no player
    if (!recipient)
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    // can't group with
    if (!GetPlayer()->isGameMaster() && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP) && initiator->GetTeam() != recipient->GetTeam())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_PLAYER_WRONG_FACTION);
        return;
    }

    if (initiator->GetInstanceId() != 0 && recipient->GetInstanceId() != 0 && initiator->GetInstanceId() != recipient->GetInstanceId() && initiator->GetMapId() == recipient->GetMapId())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_ALREADY_IN_GROUP_S); // error message is not so appropriated but no other option for classic
        return;
    }

    // just ignore us
    if (recipient->GetSocial()->HasIgnore(initiator->GetObjectGuid()))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_IGNORING_YOU_S);
        return;
    }

    Group* initiatorGroup = initiator->GetGroup();
    if (initiatorGroup && initiatorGroup->isBattleGroup())
        initiatorGroup = initiator->GetOriginalGroup();
    if (!initiatorGroup)
        initiatorGroup = initiator->GetGroupInvite();

    // player already invited
    if (recipient->GetGroupInvite())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_ALREADY_IN_GROUP_S);
        return;
    }

    Group* recipientGroup = recipient->GetGroup();
    if (recipientGroup && recipientGroup->isBattleGroup())
        recipientGroup = recipient->GetOriginalGroup();

    // player already in another group
    if (recipientGroup)
    {
        SendPartyResult(PARTY_OP_INVITE, membername, ERR_ALREADY_IN_GROUP_S);
        return;
    }

    if (initiatorGroup)
    {
        // not have permissions for invite
        if (!initiatorGroup->IsLeader(initiator->GetObjectGuid()) && !initiatorGroup->IsAssistant(initiator->GetObjectGuid()))
        {
            if (initiatorGroup->IsCreated())
                SendPartyResult(PARTY_OP_INVITE, "", ERR_NOT_LEADER);
            return;
        }
        // not have place
        if (initiatorGroup->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
            return;
        }
    }

    // ok, but group not exist, start a new group
    // but don't create and save the group to the DB until
    // at least one person joins
    if (!initiatorGroup)
    {
        initiatorGroup = new Group();
        // new group: if can't add then delete
        if (!initiatorGroup->AddLeaderInvite(initiator))
        {
            delete initiatorGroup;
            return;
        }
        if (!initiatorGroup->AddInvite(recipient))
        {
            delete initiatorGroup;
            return;
        }
    }
    else
    {
        // already existing group: if can't add then just leave
        if (!initiatorGroup->AddInvite(recipient))
        {
            return;
        }
    }

    // ok, we do it
    WorldPacket data(SMSG_GROUP_INVITE, 10);                // guess size
    data << initiator->GetName();
    recipient->GetSession()->SendPacket(data);

    SendPartyResult(PARTY_OP_INVITE, membername, ERR_PARTY_RESULT_OK);
}

void WorldSession::HandleGroupAcceptOpcode(WorldPacket& /*recv_data*/)
{
    Group* group = GetPlayer()->GetGroupInvite();
    if (!group)
        return;

    if (group->GetLeaderGuid() == GetPlayer()->GetObjectGuid())
    {
        sLog.outError("HandleGroupAcceptOpcode: %s tried to accept an invite to his own group",
                      GetPlayer()->GetGuidStr().c_str());
        return;
    }

    // remove in from invites in any case
    group->RemoveInvite(GetPlayer());

    /** error handling **/
    /********************/

    // not have place
    if (group->IsFull())
    {
        SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
        return;
    }

    Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid());

    // forming a new group, create it
    if (!group->IsCreated())
    {
        if (leader)
            group->RemoveInvite(leader);
        if (group->Create(group->GetLeaderGuid(), group->GetLeaderName()))
            sObjectMgr.AddGroup(group);
        else
            return;
    }

    // everything is fine, do it, PLAYER'S GROUP IS SET IN ADDMEMBER!!!
    if (!group->AddMember(GetPlayer()->GetObjectGuid(), GetPlayer()->GetName()))
        return;
}

void WorldSession::HandleGroupDeclineOpcode(WorldPacket& /*recv_data*/)
{
    Group*  group  = GetPlayer()->GetGroupInvite();
    if (!group)
        return;

    // remember leader if online
    Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid());

    // uninvite, group can be deleted
    GetPlayer()->UninviteFromGroup();

    if (!leader || !leader->GetSession())
        return;

    // report
    WorldPacket data(SMSG_GROUP_DECLINE, 10);               // guess size
    data << GetPlayer()->GetName();
    leader->GetSession()->SendPacket(data);
}

void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    // can't uninvite yourself
    if (guid == GetPlayer()->GetObjectGuid())
    {
        sLog.outError("WorldSession::HandleGroupUninviteGuidOpcode: leader %s tried to uninvite himself from the group.", GetPlayer()->GetGuidStr().c_str());
        return;
    }

    PartyResult res = GetPlayer()->CanUninviteFromGroup();
    if (res != ERR_PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_LEAVE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    // raid assistant cannot kick leader
    if (grp->IsLeader(guid))
    {
        SendPartyResult(PARTY_OP_LEAVE, "", ERR_NOT_LEADER);
        return;
    }

    if (grp->IsMember(guid))
    {
        Player::RemoveFromGroup(grp, guid);
        return;
    }

    if (Player* plr = grp->GetInvited(guid))
    {
        plr->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_LEAVE, "", ERR_TARGET_NOT_IN_GROUP_S);
}

void WorldSession::HandleGroupUninviteOpcode(WorldPacket& recv_data)
{
    std::string membername;
    recv_data >> membername;

    // player not found
    if (!normalizePlayerName(membername))
        return;

    // can't uninvite yourself
    if (GetPlayer()->GetName() == membername)
    {
        sLog.outError("WorldSession::HandleGroupUninviteOpcode: leader %s tried to uninvite himself from the group.", GetPlayer()->GetGuidStr().c_str());
        return;
    }

    PartyResult res = GetPlayer()->CanUninviteFromGroup();
    if (res != ERR_PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_LEAVE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    // raid assistant cannot kick leader
    if (grp->GetLeaderName() == membername)
    {
        SendPartyResult(PARTY_OP_LEAVE, "", ERR_NOT_LEADER);
        return;
    }

    if (ObjectGuid guid = grp->GetMemberGuid(membername))
    {
        Player::RemoveFromGroup(grp, guid);
        return;
    }

    if (Player* plr = grp->GetInvited(membername))
    {
        plr->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_LEAVE, membername, ERR_TARGET_NOT_IN_GROUP_S);
}

void WorldSession::HandleGroupSetLeaderOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    Player* player = sObjectMgr.GetPlayer(guid);

    /** error handling **/
    if (!player || !group->IsLeader(GetPlayer()->GetObjectGuid()) || player->GetGroup() != group)
        return;
    /********************/

    // everything is fine, do it
    group->ChangeLeader(guid);
}

void WorldSession::HandleGroupDisbandOpcode(WorldPacket& /*recv_data*/)
{
    Player* player = GetPlayer();
    Group* group = player->GetGroup();
    Group* groupPending = player->GetGroupInvite();
    if (!group && !groupPending)
        return;

    if (player->InBattleGround())
    {
        SendPartyResult(PARTY_OP_INVITE, "", ERR_NOT_LEADER);  // error message is not so appropriated but no other option for classic
        return;
    }

    /** error handling **/
    /********************/

    // everything is fine, do it
    if (group)
    {
        SendPartyResult(PARTY_OP_LEAVE, player->GetName(), ERR_PARTY_RESULT_OK);
        player->RemoveFromGroup();
    }
    else if (groupPending && groupPending->GetLeaderGuid() == player->GetObjectGuid())
    {
        // pending group creation being cancelled
        SendPartyResult(PARTY_OP_LEAVE, player->GetName(), ERR_PARTY_RESULT_OK);
        groupPending->Disband();
    }
}

void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
{
    float x, y;
    recv_data >> x;
    recv_data >> y;

    if (!GetPlayer()->GetGroup())
        return;

    // DEBUG_LOG("Received opcode MSG_MINIMAP_PING X: %f, Y: %f", x, y);

    /** error handling **/
    /********************/

    // everything is fine, do it
    WorldPacket data(MSG_MINIMAP_PING, (8 + 4 + 4));
    data << GetPlayer()->GetObjectGuid();
    data << float(x);
    data << float(y);
    GetPlayer()->GetGroup()->BroadcastPacket(data, true, -1, GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    uint32 minimum, maximum;
    recv_data >> minimum;
    recv_data >> maximum;

    /** error handling **/
    if (minimum > maximum || maximum > 10000)               // < 32768 for urand call
        return;
    /********************/

    // everything is fine, do it
    uint32 roll = urand(minimum, maximum);

    // DEBUG_LOG("ROLL: MIN: %u, MAX: %u, ROLL: %u", minimum, maximum, roll);

    WorldPacket data(MSG_RANDOM_ROLL, 4 + 4 + 4 + 8);
    data << uint32(minimum);
    data << uint32(maximum);
    data << uint32(roll);
    data << GetPlayer()->GetObjectGuid();
    if (GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->BroadcastPacket(data, false);
    else
        SendPacket(data);
}

void WorldSession::HandleRaidTargetUpdateOpcode(WorldPacket& recv_data)
{
    uint8  x;
    recv_data >> x;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    /********************/

    // everything is fine, do it
    if (x == 0xFF)                                          // target icon request
    {
        group->SendTargetIconList(this);
    }
    else                                                    // target icon update
    {
        if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
                !group->IsAssistant(GetPlayer()->GetObjectGuid()))
            return;

        ObjectGuid guid;
        recv_data >> guid;
        group->SetTargetIcon(x, guid);
    }
}

void WorldSession::HandleGroupRaidConvertOpcode(WorldPacket& /*recv_data*/)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (_player->InBattleGround())
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) || group->GetMembersCount() < group->GetMembersMinCount())
        return;
    /********************/

    // everything is fine, do it (is it 0 (PARTY_OP_INVITE) correct code)
    SendPartyResult(PARTY_OP_INVITE, "", ERR_PARTY_RESULT_OK);
    group->ConvertToRaid();
}

void WorldSession::HandleGroupChangeSubGroupOpcode(WorldPacket& recv_data)
{
    std::string name;
    uint8 groupNr;
    recv_data >> name;

    recv_data >> groupNr;

    if (groupNr >= MAX_RAID_SUBGROUPS)
        return;

    // we will get correct pointer for group here, so we don't have to check if group is BG raid
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
            !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;

    if (!group->HasFreeSlotSubGroup(groupNr))
        return;
    /********************/

    // everything is fine, do it
    if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
        group->ChangeMembersGroup(player, groupNr);
    else
    {
        if (ObjectGuid guid = sObjectMgr.GetPlayerGuidByName(name))
            group->ChangeMembersGroup(guid, groupNr);
    }
}

void WorldSession::HandleGroupAssistantLeaderOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    uint8 flag;
    recv_data >> guid;
    recv_data >> flag;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // everything is fine, do it
    group->SetAssistant(guid, (flag != 0));
}

void WorldSession::HandlePartyAssignmentOpcode(WorldPacket& recv_data)
{
    uint8 flag1, flag2;
    ObjectGuid guid;
    recv_data >> flag1 >> flag2;
    recv_data >> guid;

    DEBUG_LOG("MSG_PARTY_ASSIGNMENT");

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    // if(flag1) Main Assist
    //     0x4
    // if(flag2) Main Tank
    //     0x2

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // everything is fine, do it
    if (flag1 == 1)
        group->SetMainAssistant(guid);
    if (flag2 == 1)
        group->SetMainTank(guid);
}

void WorldSession::HandleRaidReadyCheckOpcode(WorldPacket& recv_data)
{
    if (recv_data.empty())                                  // request
    {
        Group* group = GetPlayer()->GetGroup();
        if (!group)
            return;

        /** error handling **/
        if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
                !group->IsAssistant(GetPlayer()->GetObjectGuid()))
            return;
        /********************/

        // everything is fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK, 8);
        data << ObjectGuid(GetPlayer()->GetObjectGuid());
        group->BroadcastPacket(data, true, -1);

        group->OfflineReadyCheck();
    }
    else                                                    // answer
    {
        uint8 state;
        recv_data >> state;

        Group* group = GetPlayer()->GetGroup();
        if (!group)
            return;

        // everything is fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
        data << GetPlayer()->GetObjectGuid();
        data << uint8(state);
        group->BroadcastReadyCheck(data);
    }
}

void WorldSession::HandleRaidReadyCheckFinishedOpcode(WorldPacket& /*recv_data*/)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) && !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;

    // Broadcast finish:
    WorldPacket data(MSG_RAID_READY_CHECK_FINISHED, 0);
    group->BroadcastPacket(data, true, -1);
}

void WorldSession::BuildPartyMemberStatsChangedPacket(Player* player, WorldPacket& data)
{
    uint32 mask = player->GetGroupUpdateFlag();

    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)                // if update power type, update current/max power also
        mask |= (GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER);

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)            // same for pets
        mask |= (GROUP_UPDATE_FLAG_PET_CUR_POWER | GROUP_UPDATE_FLAG_PET_MAX_POWER);

    uint32 byteCount = 0;
    for (int i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];

    data.Initialize(SMSG_PARTY_MEMBER_STATS, 8 + 4 + byteCount);
    data << player->GetPackGUID();
    data << uint32(mask);

    if (mask & GROUP_UPDATE_FLAG_STATUS)
        data << uint8(GetGroupMemberStatus(player));

    if (mask & GROUP_UPDATE_FLAG_CUR_HP)
        data << uint16(player->GetHealth());

    if (mask & GROUP_UPDATE_FLAG_MAX_HP)
        data << uint16(player->GetMaxHealth());

    Powers powerType = player->GetPowerType();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
        data << uint8(powerType);

    if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
        data << uint16(player->GetPower(powerType));

    if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
        data << uint16(player->GetMaxPower(powerType));

    if (mask & GROUP_UPDATE_FLAG_LEVEL)
        data << uint16(player->getLevel());

    if (mask & GROUP_UPDATE_FLAG_ZONE)
        data << uint16(player->GetZoneId());

    if (mask & GROUP_UPDATE_FLAG_POSITION)
        data << uint16(player->GetPositionX()) << uint16(player->GetPositionY());

    if (mask & GROUP_UPDATE_FLAG_AURAS)
    {
        const uint64& auramask = player->GetAuraUpdateMask();
        data << uint32(auramask);
        // In all checked pre-2.x data of packets included only positive auras
        for (uint32 i = 0; i < MAX_POSITIVE_AURAS; ++i)
        {
            if (auramask & (uint64(1) << i))
            {
                data << uint16(player->GetUInt32Value(UNIT_FIELD_AURA + i));
            }
        }
    }

    Pet* pet = player->GetPet();
    if (mask & GROUP_UPDATE_FLAG_PET_GUID)
        data << (pet ? pet->GetObjectGuid() : ObjectGuid());

    if (mask & GROUP_UPDATE_FLAG_PET_NAME)
    {
        if (pet)
            data << pet->GetName();
        else
            data << uint8(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MODEL_ID)
    {
        if (pet)
            data << uint16(pet->GetDisplayId());
        else
            data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_HP)
    {
        if (pet)
            data << uint16(pet->GetHealth());
        else
            data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_HP)
    {
        if (pet)
            data << uint16(pet->GetMaxHealth());
        else
            data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)
    {
        if (pet)
            data << uint8(pet->GetPowerType());
        else
            data << uint8(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_POWER)
    {
        if (pet)
            data << uint16(pet->GetPower(pet->GetPowerType()));
        else
            data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_POWER)
    {
        if (pet)
            data << uint16(pet->GetMaxPower(pet->GetPowerType()));
        else
            data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_AURAS)
    {
        if (pet)
        {
            const uint64& auramask = pet->GetAuraUpdateMask();
            data << uint32(auramask);
            // In all checked pre-2.x data of packets included only positive auras
            for (uint32 i = 0; i < MAX_POSITIVE_AURAS; ++i)
            {
                if (auramask & (uint64(1) << i))
                {
                    data << uint16(pet->GetUInt32Value(UNIT_FIELD_AURA + i));
                }
            }
        }
        else
            data << uint32(0);
    }
}

/*this procedure handles clients CMSG_REQUEST_PARTY_MEMBER_STATS request*/
void WorldSession::HandleRequestPartyMemberStatsOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_REQUEST_PARTY_MEMBER_STATS");
    ObjectGuid guid;
    recv_data >> guid;

    Player* player = ObjectAccessor::FindPlayer(guid, false);
    if (!player)
    {
        WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 3 + 4 + 2);
        data << guid.WriteAsPacked();
        data << uint32(GROUP_UPDATE_FLAG_STATUS);
        data << uint8(MEMBER_STATUS_OFFLINE);
        SendPacket(data);
        return;
    }

    Pet* pet = player->GetPet();

    WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 4 + 2 + 2 + 2 + 1 + 2 * 6 + 8 + 1 + 8);
    data << player->GetPackGUID();

    uint32 mask1 = 0x00040BFF;                              // common mask, real flags used 0x000040BFF
    if (pet)
        mask1 = 0x7FFFFFFF;                                 // for hunters and other classes with pets

    Powers powerType = player->GetPowerType();
    data << uint32(mask1);                                  // group update mask
    data << uint8(GetGroupMemberStatus(player));            // member's online status
    data << uint16(player->GetHealth());                    // GROUP_UPDATE_FLAG_CUR_HP
    data << uint16(player->GetMaxHealth());                 // GROUP_UPDATE_FLAG_MAX_HP
    data << uint8(powerType);                               // GROUP_UPDATE_FLAG_POWER_TYPE
    data << uint16(player->GetPower(powerType));            // GROUP_UPDATE_FLAG_CUR_POWER
    data << uint16(player->GetMaxPower(powerType));         // GROUP_UPDATE_FLAG_MAX_POWER
    data << uint16(player->getLevel());                     // GROUP_UPDATE_FLAG_LEVEL

    // verify player coordinates and zoneid to send to teammates
    uint16 iZoneId = 0;
    uint16 iCoordX = 0;
    uint16 iCoordY = 0;

    if (player->IsInWorld())
    {
        iZoneId = player->GetZoneId();
        iCoordX = player->GetPositionX();
        iCoordY = player->GetPositionY();
    }
    else if (player->IsBeingTeleported())               // Player is in teleportation
    {
        WorldLocation& loc = player->GetTeleportDest(); // So take teleportation destination
        iZoneId = sTerrainMgr.GetZoneId(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z);
        iCoordX = loc.coord_x;
        iCoordY = loc.coord_y;
    }
    else
    {
        // unknown player status.
    }

    data << uint16(iZoneId);                              // GROUP_UPDATE_FLAG_ZONE
    data << uint16(iCoordX);                              // GROUP_UPDATE_FLAG_POSITION
    data << uint16(iCoordY);                              // GROUP_UPDATE_FLAG_POSITION

    uint32 auramask = 0;
    size_t maskPos = data.wpos();
    data << uint32(auramask);                               // placeholder
    for (uint8 i = 0; i < MAX_AURAS; ++i)
    {
        if (uint32 aura = player->GetUInt32Value(UNIT_FIELD_AURA + i))
        {
            auramask |= (uint32(1) << i);
            data << uint16(aura);
        }
    }
    data.put<uint32>(maskPos, auramask);                    // GROUP_UPDATE_FLAG_AURAS

    if (pet)
    {
        Powers petpowertype = pet->GetPowerType();
        data << pet->GetObjectGuid();                       // GROUP_UPDATE_FLAG_PET_GUID
        data << pet->GetName();                             // GROUP_UPDATE_FLAG_PET_NAME
        data << uint16(pet->GetDisplayId());                // GROUP_UPDATE_FLAG_PET_MODEL_ID
        data << uint16(pet->GetHealth());                   // GROUP_UPDATE_FLAG_PET_CUR_HP
        data << uint16(pet->GetMaxHealth());                // GROUP_UPDATE_FLAG_PET_MAX_HP
        data << uint8(petpowertype);                        // GROUP_UPDATE_FLAG_PET_POWER_TYPE
        data << uint16(pet->GetPower(petpowertype));        // GROUP_UPDATE_FLAG_PET_CUR_POWER
        data << uint16(pet->GetMaxPower(petpowertype));     // GROUP_UPDATE_FLAG_PET_MAX_POWER

        uint32 petauramask = 0;
        size_t petMaskPos = data.wpos();
        data << uint32(petauramask);                        // placeholder
        for (uint8 i = 0; i < MAX_AURAS; ++i)
        {
            if (uint32 petaura = pet->GetUInt32Value(UNIT_FIELD_AURA + i))
            {
                petauramask |= (uint32(1) << i);
                data << uint16(petaura);
            }
        }
        data.put<uint32>(petMaskPos, petauramask);          // GROUP_UPDATE_FLAG_PET_AURAS
    }
    else
    {
        data << uint8(0);                                   // GROUP_UPDATE_FLAG_PET_NAME
        data << uint32(0);                                  // GROUP_UPDATE_FLAG_PET_AURAS
    }

    SendPacket(data);
}

void WorldSession::HandleRequestRaidInfoOpcode(WorldPacket& /*recv_data*/)
{
    // every time the player checks the character screen
    _player->SendRaidInfo();
}

void WorldSession::HandleOptOutOfLootOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_OPT_OUT_OF_LOOT");

    uint32 unkn;
    recv_data >> unkn;

    // ignore if player not loaded
    if (!GetPlayer())                                       // needed because STATUS_AUTHED
    {
        if (unkn != 0)
            sLog.outError("CMSG_GROUP_PASS_ON_LOOT value<>0 for not-loaded character!");
        return;
    }

    if (unkn != 0)
        sLog.outError("CMSG_GROUP_PASS_ON_LOOT: activation not implemented!");
}

void WorldSession::HandleGroupSwapSubGroupOpcode(WorldPacket& recv_data)
{
    std::string playerName1, playerName2;

    recv_data >> playerName1;
    recv_data >> playerName2;

    Player* player = GetPlayer();

    Group* group = player->GetGroup();
    if (!group || !group->isRaidGroup())
        return;

    ObjectGuid const& guid = player->GetObjectGuid();
    if (!group->IsLeader(guid) && !group->IsAssistant(guid))
        return;

    auto getMemberSlotInfo = [&group](std::string const& playerName, uint8& subgroup, ObjectGuid& guid)
    {
        auto slots = group->GetMemberSlots();
        for (auto& slot : slots)
        {
            if (slot.guid && slot.name == playerName)
            {
                subgroup = slot.group;
                guid = slot.guid;
                return true;
            }
        }
        return false;
    };

    ObjectGuid guid1, guid2;
    uint8 subgroup1, subgroup2;

    if (!getMemberSlotInfo(playerName1, subgroup1, guid1) || !getMemberSlotInfo(playerName2, subgroup2, guid2))
        return;

    if (subgroup1 == subgroup2)
        return;

    group->ChangeMembersGroup(guid1, subgroup2);
    group->ChangeMembersGroup(guid2, subgroup1);
}
