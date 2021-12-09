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

#include "Maps/SpawnGroup.h"

#include "Entities/Creature.h"
#include "Entities/GameObject.h"
#include "Maps/Map.h"
#include "Maps/SpawnGroupDefines.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Globals/ObjectMgr.h"

SpawnGroup::SpawnGroup(SpawnGroupEntry const& entry, Map& map, uint32 typeId) : m_entry(entry), m_map(map), m_objectTypeId(typeId), m_enabled(m_entry.EnabledByDefault)
{
}

void SpawnGroup::AddObject(uint32 dbGuid, uint32 entry)
{
    m_objects[dbGuid] = entry;
}

void SpawnGroup::RemoveObject(WorldObject* wo)
{
    m_objects.erase(wo->GetDbGuid());
}

uint32 SpawnGroup::GetGuidEntry(uint32 dbGuid) const
{
    auto itr = m_objects.find(dbGuid);
    if (itr == m_objects.end())
        return 0; // should never happen
    return (*itr).second;
}

void SpawnGroup::Update()
{
    Spawn(false);
}

uint32 SpawnGroup::GetEligibleEntry(std::map<uint32, uint32>& existingEntries, std::map<uint32, uint32>& minEntries)
{
    if (m_entry.RandomEntries.empty())
        return 0;

    if (minEntries.size() > 0)
    {
        auto itr = minEntries.begin();
        std::advance(itr, urand(0, minEntries.size() - 1));
        uint32 entry = (*itr).first;
        --(*itr).second;
        if ((*itr).second == 0)
            minEntries.erase(itr);
        return entry;
    }

    if (m_entry.ExplicitlyChanced.size())
    {
        int32 roll = urand(1, 100);
        for (auto explicitly : m_entry.ExplicitlyChanced)
        {
            if (existingEntries[explicitly->Entry] > 0)
            {
                if (roll < explicitly->Chance)
                    return explicitly->Entry;

                roll -= int32(explicitly->Chance);
            }
        }
    }

    if (m_entry.EquallyChanced.empty())
        return 0;

    auto equallyCopy = m_entry.EquallyChanced;
    std::shuffle(equallyCopy.begin(), equallyCopy.end(), *GetRandomGenerator());

    for (auto equally : equallyCopy)
        if (existingEntries[equally->Entry] > 0)
            return equally->Entry;

    return 0;
}

void SpawnGroup::Spawn(bool force)
{
    if (!m_enabled && !force)
        return;

    if (m_objects.size() >= m_entry.MaxCount || (m_entry.WorldStateId && m_map.GetVariableManager().GetVariable(m_entry.WorldStateId) == 0))
        return;

    std::vector<uint32> eligibleGuids;
    std::map<uint32, uint32> validEntries;
    std::map<uint32, uint32> minEntries;

    for (auto& randomEntry : m_entry.RandomEntries)
    {
        validEntries[randomEntry.Entry] = randomEntry.MaxCount > 0 ? randomEntry.MaxCount : std::numeric_limits<uint32>::max();
        if (randomEntry.MinCount > 0)
            minEntries.emplace(randomEntry.Entry, randomEntry.MinCount);
    }

    for (auto& guid : m_entry.DbGuids)
        eligibleGuids.push_back(guid.DbGuid);

    for (auto& data : m_objects)
    {
        eligibleGuids.erase(std::remove(eligibleGuids.begin(), eligibleGuids.end(), data.first), eligibleGuids.end());
        if (validEntries.size() > 0)
        {
            uint32 curCount = validEntries[data.second];
            validEntries[data.second] = curCount > 0 ? curCount - 1 : 0;
        }
        if (minEntries.size() > 0)
        {
            auto itr = minEntries.find(data.second);
            if (itr != minEntries.end())
            {
                --(*itr).second;
                if ((*itr).second == 0)
                    minEntries.erase(itr);
            }
        }
    }

    time_t now = time(nullptr);
    for (auto itr = eligibleGuids.begin(); itr != eligibleGuids.end();)
    {
        if (m_map.GetPersistentState()->GetObjectRespawnTime(GetObjectTypeId(), *itr) > now)
        {
            if (!force)
            {
                if (m_entry.MaxCount == 1) // rare mob case - prevent respawn until all are off CD
                    return;
                itr = eligibleGuids.erase(itr);
                continue;
            }
            else
                m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), *itr, now);
        }
        ++itr;
    }

    // on tbc+ spawnmask block located here

    std::shuffle(eligibleGuids.begin(), eligibleGuids.end(), *GetRandomGenerator());

    for (auto itr = eligibleGuids.begin(); itr != eligibleGuids.end() && !eligibleGuids.empty() && m_objects.size() < m_entry.MaxCount; ++itr)
    {
        uint32 dbGuid = *itr;
        uint32 entry = GetEligibleEntry(validEntries, minEntries);
        float x, y;
        if (GetObjectTypeId() == TYPEID_UNIT)
        {
            auto data = sObjectMgr.GetCreatureData(*itr);
            x = data->posX; y = data->posY;
            m_map.GetPersistentState()->AddCreatureToGrid(dbGuid, data);
        }
        else
        {
            auto data = sObjectMgr.GetGOData(*itr);
            x = data->posX; y = data->posY;
            m_map.GetPersistentState()->AddGameobjectToGrid(dbGuid, data);
        }
        AddObject(dbGuid, entry);
        if (force || m_entry.Active || m_map.IsLoaded(x, y))
        {
            if (GetObjectTypeId() == TYPEID_UNIT)
                WorldObject::SpawnCreature(dbGuid, &m_map, entry);
            else
                WorldObject::SpawnGameObject(dbGuid, &m_map, entry);
        }
        if (entry && validEntries[entry])
            --validEntries[entry];
    }
}

CreatureGroup::CreatureGroup(SpawnGroupEntry const& entry, Map& map) : SpawnGroup(entry, map, uint32(TYPEID_UNIT))
{
}

void CreatureGroup::RemoveObject(WorldObject* wo)
{
    SpawnGroup::RemoveObject(wo);
    CreatureData const* data = sObjectMgr.GetCreatureData(wo->GetDbGuid());
    m_map.GetPersistentState()->RemoveCreatureFromGrid(wo->GetDbGuid(), data);
}

void CreatureGroup::TriggerLinkingEvent(uint32 event, Unit* target)
{
    switch (event)
    {
        case CREATURE_GROUP_EVENT_AGGRO:
            if ((m_entry.Flags & CREATURE_GROUP_AGGRO_TOGETHER) == 0)
                return;

            for (auto& data : m_objects)
            {
                uint32 dbGuid = data.first;
                if (Creature* creature = m_map.GetCreature(dbGuid))
                {
                    creature->AddThreat(target);
                    target->AddThreat(creature);
                    target->SetInCombatWith(creature);
                    target->GetCombatManager().TriggerCombatTimer(creature);
                }
            }

            for (uint32 linkedGroup : m_entry.LinkedGroups)
            {
                // ensured on db load that it will be valid fetch
                CreatureGroup* group = static_cast<CreatureGroup*>(m_map.GetSpawnManager().GetSpawnGroup(linkedGroup));
                group->TriggerLinkingEvent(event, target);
            }
            break;
        case CREATURE_GROUP_EVENT_EVADE:
            if ((m_entry.Flags & CREATURE_GROUP_EVADE_TOGETHER) != 0)
            {
                for (auto& data : m_objects)
                {
                    uint32 dbGuid = data.first;
                    if (Creature* creature = m_map.GetCreature(dbGuid))
                        if (!creature->GetCombatManager().IsEvadingHome())
                            creature->AI()->EnterEvadeMode();
                }
            }
            break;
        case CREATURE_GROUP_EVENT_HOME:
        case CREATURE_GROUP_EVENT_RESPAWN:
            if ((m_entry.Flags & CREATURE_GROUP_RESPAWN_TOGETHER) == 0)
                return;

            ClearRespawnTimes();
            break;
    }
}

void CreatureGroup::ClearRespawnTimes()
{
    time_t now = time(nullptr);
    for (auto& data : m_entry.DbGuids)
        m_map.GetPersistentState()->SaveObjectRespawnTime(GetObjectTypeId(), data.DbGuid, now);
}

GameObjectGroup::GameObjectGroup(SpawnGroupEntry const& entry, Map& map) : SpawnGroup(entry, map, uint32(TYPEID_GAMEOBJECT))
{
}

void GameObjectGroup::RemoveObject(WorldObject* wo)
{
    SpawnGroup::RemoveObject(wo);
    GameObjectData const* data = sObjectMgr.GetGOData(wo->GetDbGuid());
    m_map.GetPersistentState()->RemoveGameobjectFromGrid(wo->GetDbGuid(), data);
}
