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

#ifndef _BG_DEFINES_H
#define _BG_DEFINES_H

#include "Common.h"

#define BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY 86400     // seconds in a day
#define COUNT_OF_PLAYERS_TO_AVERAGE_WAIT_TIME 10

enum BattleGroundQueueGroupTypes
{
    BG_QUEUE_PREMADE_ALLIANCE   = 0,
    BG_QUEUE_PREMADE_HORDE      = 1,
    BG_QUEUE_NORMAL_ALLIANCE    = 2,
    BG_QUEUE_NORMAL_HORDE       = 3
};

#define BG_QUEUE_GROUP_TYPES_COUNT 4

enum BattleGroundGroupJoinStatus
{
    BG_GROUP_JOIN_STATUS_TEAM_LEFT_QUEUE        = -7,
    BG_GROUP_JOIN_STATUS_QUEUED_FOR_RATED       = -6,
    BG_GROUP_JOIN_STATUS_CANNOT_QUEUE_FOR_RATED = -5,
    BG_GROUP_JOIN_STATUS_TOO_MANY_QUEUES        = -4,
    BG_GROUP_JOIN_STATUS_NOT_IN_TEAM            = -3,
    BG_GROUP_JOIN_STATUS_DESERTERS              = -2,
    BG_GROUP_JOIN_STATUS_NOT_ELIGIBLE           = -1,
    BG_GROUP_JOIN_STATUS_SUCCESS                = 0,
};

// indexes of BattlemasterList.dbc
enum BattleGroundTypeId
{
    BATTLEGROUND_TYPE_NONE     = 0,
    BATTLEGROUND_AV            = 1,
    BATTLEGROUND_WS            = 2,
    BATTLEGROUND_AB            = 3,
};
#define MAX_BATTLEGROUND_TYPE_ID 4

inline BattleGroundTypeId GetBattleGroundTypeIdByMapId(uint32 mapId)
{
    switch (mapId)
    {
        case 30:    return BATTLEGROUND_AV;
        case 489:   return BATTLEGROUND_WS;
        case 529:   return BATTLEGROUND_AB;
        default:    return BATTLEGROUND_TYPE_NONE;
    }
}

inline uint32 GetBattleGrounMapIdByTypeId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:   return 30;
        case BATTLEGROUND_WS:   return 489;
        case BATTLEGROUND_AB:   return 529;
        default:                return 0;   // none
    }

    // impossible, just make compiler happy
    return 0;
}

enum ArenaType
{
    ARENA_TYPE_NONE = 0,                            // used for mark non-arenas or problematic cases
    ARENA_TYPE_2v2 = 2,
    ARENA_TYPE_3v3 = 3,
    ARENA_TYPE_5v5 = 5
};

inline bool IsArenaTypeValid(ArenaType type) { return type == ARENA_TYPE_2v2 || type == ARENA_TYPE_3v3 || type == ARENA_TYPE_5v5; }

#endif