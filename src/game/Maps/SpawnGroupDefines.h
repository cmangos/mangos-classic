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

#ifndef MANGOS_SPAWN_GROUP_DEFINE_H
#define MANGOS_SPAWN_GROUP_DEFINE_H

#include "Platform/Define.h"
#include <string>
#include <vector>
#include <map>

struct SpawnGroupRandomEntry
{
    uint32 Id;
    uint32 Entry;
    uint32 MinCount; // Minimum of given entry before randomization is used
    uint32 MaxCount; // Maximum of given entry alive spawned in world, 0 is infinite
    uint32 Chance;
};

// TODO: Formations struct
// TODO: Linkage struct

struct SpawnGroupDbGuids
{
    uint32 Id;
    uint32 DbGuid;
};

enum SpawnGroupType
{
    SPAWN_GROUP_CREATURE = 0,
    SPAWN_GROUP_GAMEOBJECT = 1,
};

enum CreatureGroupFlags
{
    CREATURE_GROUP_AGGRO_TOGETHER   = 0x01,
    CREATURE_GROUP_RESPAWN_TOGETHER = 0x02,
    CREATURE_GROUP_EVADE_TOGETHER   = 0x04,
};

enum CreatureGroupEvent : uint32
{
    CREATURE_GROUP_EVENT_AGGRO,
    CREATURE_GROUP_EVENT_EVADE,
    CREATURE_GROUP_EVENT_HOME,
    CREATURE_GROUP_EVENT_RESPAWN,
};

struct SpawnGroupEntry
{
    uint32 Id;
    std::string Name;
    SpawnGroupType Type;
    uint32 MaxCount; // Maximum active alive entities spawned in world
    int32 WorldStateId; // Worldstate value when set to 1 enables spawning of given group and 0 disables spawning
    uint32 Flags;
    bool Active;
    bool EnabledByDefault;
    std::vector<SpawnGroupDbGuids> DbGuids;
    std::vector<SpawnGroupRandomEntry> RandomEntries;
    std::vector<SpawnGroupRandomEntry*> EquallyChanced;
    std::vector<SpawnGroupRandomEntry*> ExplicitlyChanced;
    std::vector<uint32> LinkedGroups;
};

struct SpawnGroupEntryContainer
{
    std::map<uint32, SpawnGroupEntry> spawnGroupMap;
    std::map<std::pair<uint32, uint32>, SpawnGroupEntry*> spawnGroupByGuidMap;
};

#endif