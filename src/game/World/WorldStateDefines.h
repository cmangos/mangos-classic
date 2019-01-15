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

#ifndef WORLD_STATE_DEFINES_H
#define WORLD_STATE_DEFINES_H
#include "Platform/Define.h"

// corresponds to clientside data indices
enum WorldStateID : int32
{
    // Eastern Plaguelands Outdoor PVP
    WORLD_STATE_EP_TOWER_COUNT_ALLIANCE = 2327,
    WORLD_STATE_EP_TOWER_COUNT_HORDE = 2328,

    //WORLD_STATE_EP_PLAGUEWOOD_CONTEST_ALLIANCE    = 2366,
    //WORLD_STATE_EP_PLAGUEWOOD_CONTEST_HORDE       = 2367,
    //WORLD_STATE_EP_PLAGUEWOOD_PROGRESS_ALLIANCE   = 2368,
    //WORLD_STATE_EP_PLAGUEWOOD_PROGRESS_HORDE      = 2369,
    WORLD_STATE_EP_PLAGUEWOOD_ALLIANCE = 2370,
    WORLD_STATE_EP_PLAGUEWOOD_HORDE = 2371,
    WORLD_STATE_EP_PLAGUEWOOD_NEUTRAL = 2353,

    //WORLD_STATE_EP_NORTHPASS_CONTEST_ALLIANCE     = 2362,
    //WORLD_STATE_EP_NORTHPASS_CONTEST_HORDE        = 2363,
    //WORLD_STATE_EP_NORTHPASS_PROGRESS_ALLIANCE    = 2364,
    //WORLD_STATE_EP_NORTHPASS_PROGRESS_HORDE       = 2365,
    WORLD_STATE_EP_NORTHPASS_ALLIANCE = 2372,
    WORLD_STATE_EP_NORTHPASS_HORDE = 2373,
    WORLD_STATE_EP_NORTHPASS_NEUTRAL = 2352,

    //WORLD_STATE_EP_EASTWALL_CONTEST_ALLIANCE      = 2359,
    //WORLD_STATE_EP_EASTWALL_CONTEST_HORDE         = 2360,
    //WORLD_STATE_EP_EASTWALL_PROGRESS_ALLIANCE     = 2357,
    //WORLD_STATE_EP_EASTWALL_PROGRESS_HORDE        = 2358,
    WORLD_STATE_EP_EASTWALL_ALLIANCE = 2354,
    WORLD_STATE_EP_EASTWALL_HORDE = 2356,
    WORLD_STATE_EP_EASTWALL_NEUTRAL = 2361,

    //WORLD_STATE_EP_CROWNGUARD_CONTEST_ALLIANCE    = 2374,
    //WORLD_STATE_EP_CROWNGUARD_CONTEST_HORDE       = 2375,
    //WORLD_STATE_EP_CROWNGUARD_PROGRESS_ALLIANCE   = 2376,
    //WORLD_STATE_EP_CROWNGUARD_PROGRESS_HORDE      = 2377,
    WORLD_STATE_EP_CROWNGUARD_ALLIANCE = 2378,
    WORLD_STATE_EP_CROWNGUARD_HORDE = 2379,
    WORLD_STATE_EP_CROWNGUARD_NEUTRAL = 2355,

    // Silithus Outdoor PVP
    // world states
    WORLD_STATE_SI_GATHERED_A = 2313,
    WORLD_STATE_SI_GATHERED_H = 2314,
    WORLD_STATE_SI_SILITHYST_MAX = 2317,
};

#endif