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
#ifndef MANGOSSERVER_LFGHANDLER_H
#define MANGOSSERVER_LFGHANDLER_H

struct ObjectQueue 
{

	uint32 zoneid;
	uint32 guid;
};

std::list<ObjectQueue*> LFGList;

void AddGroupToQueue(uint32 id, uint32 zoneid);
void AddPlayerToQueue(Player *plr, uint32 zoneid);
void RemovePlayerFromQueue(uint32 id);
bool GroupHasRole(uint8 role, Group *grp);
bool isPlayerHealer(Player *player);
bool isPlayerDps(Player *plr);
bool isPlayerTank(Player *plr);
enum MeetingstoneQueueStatus
{
    MEETINGSTONE_STATUS_NONE                                = 0,
    MEETINGSTONE_STATUS_JOINED_QUEUE                        = 1,
    MEETINGSTONE_STATUS_PARTY_MEMBER_LEFT_LFG                = 2,        
    MEETINGSTONE_STATUS_PARTY_MEMBER_REMOVED_PARTY_REMOVED  = 3,
    MEETINGSTONE_STATUS_LOOKING_FOR_NEW_PARTY_IN_QUEUE        = 4,
    MEETINGSTONE_STATUS_NONE_UNK                            = 5
};

enum MeetingstoneFailedStatus
{
    MEETINGSTONE_FAIL_NONE                                  = 0,
    MEETINGSTONE_FAIL_PARTYLEADER                           = 1,
    MEETINGSTONE_FAIL_FULL_GROUP                            = 2,
    MEETINGSTONE_FAIL_RAID_GROUP                            = 3,
    MEETINGSTONE_FAIL_NONE_UNK                              = 4
};

enum ClassRoles 
{

	ROLE_HEALER												= 001,
	ROLE_DPS												= 002,
	ROLE_TANK												= 003
};

#endif