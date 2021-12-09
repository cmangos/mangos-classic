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

#ifndef MANGOS_SPAWN_GROUP_H
#define MANGOS_SPAWN_GROUP_H

#include "Platform/Define.h"
#include "Entities/ObjectGuid.h"
#include <map>

class WorldObject;
class Creature;
class GameObject;
struct SpawnGroupEntry;
class Map;
class Unit;

class SpawnGroup
{
    public:
        SpawnGroup(SpawnGroupEntry const& entry, Map& map, uint32 typeId);
        virtual void AddObject(uint32 dbGuid, uint32 entry);
        virtual void RemoveObject(WorldObject* wo);
        uint32 GetGuidEntry(uint32 dbGuid) const;
        virtual void Update();
        uint32 GetEligibleEntry(std::map<uint32, uint32>& existingEntries, std::map<uint32, uint32>& minEntries);
        virtual void Spawn(bool force);
        uint32 GetObjectTypeId() const { return m_objectTypeId; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }
    protected:
        SpawnGroupEntry const& m_entry;
        Map& m_map;
        std::map<uint32, uint32> m_objects;
        uint32 m_objectTypeId;
        bool m_enabled;
};

class CreatureGroup : public SpawnGroup
{
    public:
        CreatureGroup(SpawnGroupEntry const& entry, Map& map);
        void RemoveObject(WorldObject* wo) override;

        void TriggerLinkingEvent(uint32 event, Unit* target);

    private:
        void ClearRespawnTimes();
};

class GameObjectGroup : public SpawnGroup
{
    public:
        GameObjectGroup(SpawnGroupEntry const& entry, Map& map);
        void RemoveObject(WorldObject* wo) override;
};

#endif