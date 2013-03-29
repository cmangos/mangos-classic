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
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Object.h"
#include "Chat.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "World.h"
#include "Group.h"
#include "LFGHandler.h"

void WorldSession::HandleMeetingStoneJoinOpcode(WorldPacket & recv_data)
{
	ObjectGuid guid;

	recv_data >> guid;

	DEBUG_LOG("WORLD: Recvd CMSG_MEETINGSTONE_JOIN Message guid: %s", guid.GetString().c_str());

	// ignore for remote control state
	if (!_player->IsSelfMover())
		return;

	GameObject *obj = GetPlayer()->GetMap()->GetGameObject(guid);
	if (!obj)
		return;

	// Never expect this opcode for some type GO's
	if (obj->GetGoType() != GAMEOBJECT_TYPE_MEETINGSTONE)
	{
		sLog.outError("HandleMeetingStoneJoinOpcode: CMSG_MEETINGSTONE_JOIN for not allowed GameObject type %u (Entry %u), didn't expect this to happen.", obj->GetGoType(), obj->GetEntry());
		return;
	}
	GameObjectInfo const *gInfo = ObjectMgr::GetGameObjectInfo(obj->GetEntry());

	if (Group *grp = _player->GetGroup())
	{
		if(!grp->IsLeader(_player->GetGUID()))
		{
			SendMeetingstoneFailed(MEETINGSTONE_FAIL_PARTYLEADER);

			obj->Use(_player);

			return;
		}

		if(grp->isRaidGroup())
		{
			SendMeetingstoneFailed(MEETINGSTONE_FAIL_RAID_GROUP);

			obj->Use(_player);
			return;
		}

		if(grp->IsFull())
		{
			SendMeetingstoneFailed(MEETINGSTONE_FAIL_FULL_GROUP);

			obj->Use(_player);
			return;
		}



	}


	SendMeetingstoneSetqueue(gInfo->meetingstone.areaID, MEETINGSTONE_STATUS_JOINED_QUEUE);

	if (Group *grp = _player->GetGroup())
	{
		AddGroupToQueue(grp->GetId(), gInfo->meetingstone.areaID);
	}
	else {

		AddPlayerToQueue(_player, gInfo->meetingstone.areaID);
	}

	LFGLoop();
	obj->Use(_player);
}



bool GroupHasRole(uint8 role, Group *grp) {

	switch(role) {

	case ROLE_HEALER:
		for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next()) {
			Player* player = itr->getSource();

			if (player->getClass() == 2 || player->getClass() == 5 || player->getClass() == 11 || player->getClass() == 7) {
				return true;
			}
			else {
				return false;
			}
		}

		break;

	case ROLE_TANK:

		for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next()) {
			Player* player = itr->getSource();

			if (player->getClass() == 1 || player->getClass() == 11) {
				return true;
			}
			else {
				return false;
			}
		}

		break;


	}

}

bool isPlayerHealer(Player *player) {
	if (player->getClass() == 2 || player->getClass() == 5 || player->getClass() == 11 || player->getClass() == 7) {
		return true;
	}
	else {
		return false;
	}

}
bool isPlayerDps(Player *plr) {
	if(plr->getClass() == 1 || plr->getClass() == 3 || plr->getClass() == 4 || plr->getClass() == 7 || plr->getClass() == 8) {

		return true;

	}
	else {
		return false;
	}
}
bool isPlayerTank(Player *player) {
	if (player->getClass() == 1 || player->getClass() == 11) {
		return true;
	}
	else {
		return false;
	}

}

void AddPlayerToQueue(Player *plr, uint32 zoneid) {

	CharacterDatabase.PExecute("INSERT INTO lfg_queue (guid,zoneid,is_group,player_name) VALUES ('%u','%u',0, '%s')", plr->GetGUIDLow(), zoneid, plr->GetName());
}
void AddGroupToQueue(uint32 id, uint32 zoneid) {

	CharacterDatabase.PExecute("INSERT INTO lfg_queue (guid,zoneid,is_group) VALUES ('%u','%u',1)", id, zoneid);
}

bool GroupDoesNotExistInQueue(uint32 id) {

	QueryResult *result = CharacterDatabase.PQuery("SELECT FROM lfg_queue WHERE guid = '%u' AND is_group = 1", id);
	if(result->GetRowCount() > 0) {

		return true;
	}
	else {
		return false;
	}
}

void RemovePlayerFromQueue(uint32 id) {
	CharacterDatabase.PExecute("DELETE FROM lfg_queue WHERE guid = '%u' AND is_group = 0", id);
}

void SendMeetingStoneCompleteToParty(Group *grp) {

	for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next()) {
		Player* player = itr->getSource();

		WorldPacket data(SMSG_MEETINGSTONE_COMPLETE, 1);               
		player->GetSession()->SendPacket(&data);

		WorldPacket data2(SMSG_MEETINGSTONE_SETQUEUE, 0);
		data2 << uint32(0);
		data2 << uint8(0);
		player->GetSession()->SendPacket(&data2);
	}

	//remove party from queue
	CharacterDatabase.PExecute("DELETE FROM lfg_queue WHERE guid = '%u' AND is_group = 1", grp->GetId());
}


void WorldSession::LFGLoop() {

	QueryResult *result = WorldDatabase.Query("SELECT guid,zoneid,is_group FROM lfg_queue");
	if (result)
	{

		do
		{
			Field* fields = result->Fetch();

			//is this entry a group?
			if(fields[2].GetUInt32() == 1) {
				//is there anyone in the queue looking for a group for this groups instance?
				QueryResult *resultLFG = WorldDatabase.PQuery("SELECT guid,zoneid,is_group,player_name FROM lfg_queue WHERE zoneid = '%u' AND is_group = 0", fields[1].GetUInt32());
				if (resultLFG) {
					do
					{
						Field* fieldsLFG = resultLFG->Fetch();
						Player *plr = sObjectAccessor.FindPlayerByName(fieldsLFG[3].GetString());
						Group *grp = sObjectMgr.GetGroupById(fields[0].GetUInt32());

						if(!grp->IsFull()) {
							//check for roles
							if(GroupHasRole(ROLE_TANK, grp) && GroupHasRole(ROLE_HEALER, grp)) {
								//group has both dps & tank, add anyone.
								grp->AddMember(plr->GetGUID(), plr->GetName());
								RemovePlayerFromQueue(plr->GetGUIDLow());

								if(grp->IsFull()) {
									SendMeetingStoneCompleteToParty(grp);
								}
							}
							else if(GroupHasRole(ROLE_TANK, grp) && !GroupHasRole(ROLE_HEALER, grp)) {
								//tank but healers missing
								//is player healer?
								if(isPlayerHealer(plr)) {

									grp->AddMember(plr->GetGUID(), plr->GetName());
									RemovePlayerFromQueue(plr->GetGUIDLow());
									if(grp->IsFull()) {
										SendMeetingStoneCompleteToParty(grp);
									}

								}
								//player is not healer but group can take one more random..
								else if(!isPlayerHealer(plr) && grp->GetMembersCount() < 4) {
									grp->AddMember(plr->GetGUID(), plr->GetName());
									RemovePlayerFromQueue(plr->GetGUIDLow());
									if(grp->IsFull()) {
										SendMeetingStoneCompleteToParty(grp);
									}
								}
							}
							else if(!GroupHasRole(ROLE_TANK, grp) && !GroupHasRole(ROLE_HEALER, grp)) {
								//group has neither tank or healer
								if(isPlayerHealer(plr) || isPlayerTank(plr)) {
									grp->AddMember(plr->GetGUID(), plr->GetName());
									RemovePlayerFromQueue(plr->GetGUIDLow());
									if(grp->IsFull()) {
										SendMeetingStoneCompleteToParty(grp);
									}
								}
								else {
									if(grp->GetMembersCount() < 3) {
										//theres room for 1 more random, let the player join.
										grp->AddMember(plr->GetGUID(), plr->GetName());
										RemovePlayerFromQueue(plr->GetGUIDLow());
										if(grp->IsFull()) {
											SendMeetingStoneCompleteToParty(grp);
										}
									}
								}

							}
						}
					}
					while (resultLFG->NextRow());
				}

			}
			else {
				//its a player, is there any other non-grouped players thats looking for the same instance?

				QueryResult *result_mp = WorldDatabase.PQuery("SELECT guid,zoneid,is_group,player_name FROM lfg_queue WHERE zoneid = '%u' AND is_group = 0", fields[1].GetUInt32());
				if(result_mp->GetRowCount() > 1) {
					int i = 1;
					Group* grp;
					grp = new Group;
					do {
						
						Field* fields_mp = result_mp->Fetch();
						Player *plr = sObjectAccessor.FindPlayerByName(fields_mp[3].GetString());

						if(!grp->IsCreated()) {
							if (grp->Create(plr->GetGUID(), plr->GetName())) {
								sObjectMgr.AddGroup(grp);
								RemovePlayerFromQueue(plr->GetGUIDLow());
							}
						}
						else {
							grp->AddMember(plr->GetGUID(), plr->GetName());
							RemovePlayerFromQueue(plr->GetGUIDLow());
						}


						plr = NULL;

					}
					while (result_mp->NextRow());

					//if this group did not fill up, add it into LFM queue

					if(!grp->IsFull()) {

						AddGroupToQueue(grp->GetId(), fields[1].GetUInt32());
					}
				}



			}
		}
		while (result->NextRow());

	}

}

void WorldSession::HandleMeetingStoneLeaveOpcode(WorldPacket & recv_data)
{
	DEBUG_LOG("WORLD: Recvd CMSG_MEETINGSTONE_LEAVE");

	if (Group *grp = _player->GetGroup()) {

	}
	else {
		RemovePlayerFromQueue(_player->GetGUIDLow());
	}

	SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);
}

void WorldSession::SendMeetingStoneComplete() {

	WorldPacket data(SMSG_MEETINGSTONE_COMPLETE, 1);
	SendPacket(&data);
}

void WorldSession::SendMeetingStoneInProgress(uint32 areaid) {
	WorldPacket data(SMSG_MEETINGSTONE_IN_PROGRESS, 1);
	data << uint8(areaid);
	SendPacket(&data);
}

void WorldSession::SendPartyMemberAdded() {

	WorldPacket data(SMSG_MEETINGSTONE_MEMBER_ADDED, 1);
	SendPacket(&data);

}
void WorldSession::SendMeetingstoneFailed(uint8 status)
{
	WorldPacket data(SMSG_MEETINGSTONE_JOINFAILED, 1);
	data << uint8(status);
	SendPacket(&data);
}

void WorldSession::SendMeetingstoneSetqueue(uint32 areaid, uint8 status)
{
	if(Group *grp = _player->GetGroup()) {

		for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next()) {
			Player* player = itr->getSource();
			WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, status);
			data << uint32(areaid);
			data << uint8(status);
			player->GetSession()->SendPacket(&data);
		}
	}
	else {
		WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, status);
		data << uint32(areaid);
		data << uint8(status);
		SendPacket(&data);
	}
}