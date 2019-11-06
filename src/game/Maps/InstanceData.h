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

#ifndef MANGOS_INSTANCE_DATA_H
#define MANGOS_INSTANCE_DATA_H

#include "Common.h"
#include "Entities/ObjectGuid.h"
#include "ByteBuffer.h"

class Map;
class Unit;
class Player;
class GameObject;
class Creature;
class WorldObject;

enum InstanceConditionIDs                                   // Suggested values used with CONDITION_INSTANCE_SCRIPT for some generic uses
{
    // for hard-mode loot (0 normal; 1,2... hard,harder... mode)
    INSTANCE_CONDITION_ID_NORMAL_MODE       = 0,
    INSTANCE_CONDITION_ID_HARD_MODE         = 1,
    INSTANCE_CONDITION_ID_HARD_MODE_2       = 2,
    INSTANCE_CONDITION_ID_HARD_MODE_3       = 3,
    INSTANCE_CONDITION_ID_HARD_MODE_4       = 4,
    INSTANCE_CONDITION_ID_HARD_MODE_5       = 5,
    INSTANCE_CONDITION_ID_HARD_MODE_6       = 6,

    // to check for which team the instance is doing scripts
    INSTANCE_CONDITION_ID_TEAM_HORDE        = 67,
    INSTANCE_CONDITION_ID_TEAM_ALLIANCE     = 469,
};

class InstanceData
{
    public:

        explicit InstanceData(Map* map) : instance(map) {}
        virtual ~InstanceData() {}

        Map* instance;

        // On creation, NOT load.
        virtual void Initialize() {}

        // On load
        virtual void Load(const char* /*data*/) {}

        // When save is needed, this function generates the data
        virtual const char* Save() const { return ""; }

        void SaveToDB() const;

        // Called every map update
        virtual void Update(const uint32 /*diff*/) {}

        // This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const { return false; };

        // Called when a player successfully enters the instance (after really added to map)
        virtual void OnPlayerEnter(Player*) {}

        // Called when a player dies inside instance
        virtual void OnPlayerDeath(Player*) {}

        // Called when a player rezzurects inside instance
        virtual void OnPlayerResurrect(Player*) {}

        // Called when a player leaves the instance (before really removed from map (or possibly world))
        virtual void OnPlayerLeave(Player*) {}

        // Called when a gameobject is created
        virtual void OnObjectCreate(GameObject*) {}

        // called on creature creation
        virtual void OnCreatureCreate(Creature* /*creature*/) {}

        // called on creature enter combat
        virtual void OnCreatureEnterCombat(Creature* /*creature*/) {}

        // called on creature evade
        virtual void OnCreatureEvade(Creature* /*creature*/) {}

        // called on creature death
        virtual void OnCreatureDeath(Creature* /*creature*/) {}

        // called on creature despawn
        virtual void OnCreatureDespawn(Creature* /*creature*/) {}

        // called on game event
        virtual void OnEventHappened(uint16 /*event_id*/, bool /*activate*/, bool /*resume*/) {}

        // All-purpose data storage 64 bit
        virtual uint64 GetData64(uint32 /*Data*/) const { return 0; }
        virtual void SetData64(uint32 /*Data*/, uint64 /*Value*/) { }

        // Guid data storage (wrapper for set/get from uint64 storage
        ObjectGuid GetGuid(uint32 dataIdx) const { return ObjectGuid(GetData64(dataIdx)); }
        void SetGuid(uint32 dataIdx, ObjectGuid value) { SetData64(dataIdx, value.GetRawValue()); }

        // All-purpose data storage 32 bit
        virtual uint32 GetData(uint32 /*Type*/) const { return 0; }
        virtual void SetData(uint32 /*Type*/, uint32 /*Data*/) {}

        // Condition criteria additional requirements check
        // This is used for such things are heroic loot
        // See ObjectMgr.h enum ConditionSource for possible values of conditionSourceType
        virtual bool CheckConditionCriteriaMeet(Player const* source, uint32 instance_condition_id, WorldObject const* conditionSource, uint32 conditionSourceType) const;

        virtual void FillInitialWorldStates(ByteBuffer& /*data*/, uint32& /*count*/, uint32 /*zoneId*/, uint32 /*areaId*/) {}

        // helper functions for world state list fill
        inline void FillInitialWorldStateData(ByteBuffer& data, uint32& count, uint32 state, uint32 value)
        {
            data << uint32(state);
            data << uint32(value);
            ++count;
        }

        inline void FillInitialWorldStateData(ByteBuffer& data, uint32& count, uint32 state, int32 value)
        {
            data << uint32(state);
            data << int32(value);
            ++count;
        }
};

#endif
