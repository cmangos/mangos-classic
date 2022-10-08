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

#ifndef _LFG_DEFINES_H
#define _LFG_DEFINES_H

enum LfgRoles
{
    PLAYER_ROLE_NONE = 0x00,
    PLAYER_ROLE_TANK = 0x01,
    PLAYER_ROLE_HEALER = 0x02,
    PLAYER_ROLE_DAMAGE = 0x04
};

enum LfgRolePriority
{
    LFG_PRIORITY_NONE = 0,
    LFG_PRIORITY_LOW = 1,
    LFG_PRIORITY_NORMAL = 2,
    LFG_PRIORITY_HIGH = 3
};

enum PlayerLeaveMethod
{
    PLAYER_CLIENT_LEAVE = 0,
    PLAYER_SYSTEM_LEAVE = 1
};

enum GroupLeaveMethod
{
    GROUP_CLIENT_LEAVE = 0,
    GROUP_SYSTEM_LEAVE = 1
};

enum MeetingstoneQueueStatus
{
    MEETINGSTONE_STATUS_LEAVE_QUEUE                         = 0,
    MEETINGSTONE_STATUS_JOINED_QUEUE                        = 1,
    MEETINGSTONE_STATUS_PARTY_MEMBER_LEFT_LFG               = 2,
    MEETINGSTONE_STATUS_PARTY_MEMBER_REMOVED_PARTY_REMOVED  = 3,
    MEETINGSTONE_STATUS_LOOKING_FOR_NEW_PARTY_IN_QUEUE      = 4,
    MEETINGSTONE_STATUS_NONE                                = 5
};

enum MeetingstoneFailedStatus
{
    MEETINGSTONE_FAIL_NONE                                  = 0, // custom, not to be sent
    MEETINGSTONE_FAIL_PARTYLEADER                           = 1,
    MEETINGSTONE_FAIL_FULL_GROUP                            = 2,
    MEETINGSTONE_FAIL_RAID_GROUP                            = 3,
    //MEETINGSTONE_FAIL_NONE_UNK                              = 4 [-ZERO]
};

#endif