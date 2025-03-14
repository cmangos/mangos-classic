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

#include "Globals/ObjectMgr.h"
#include "Maps/SpawnManager.h"
#include "Maps/Map.h"
#include "Maps/SpawnGroupDefines.h"
#include "Maps/MapPersistentStateMgr.h"

bool operator<(SpawnInfo const& lhs, SpawnInfo const& rhs)
{
    return lhs.GetRespawnTime() < rhs.GetRespawnTime();
}

bool SpawnInfo::ConstructForMap(Map& map)
{
    m_inUse = true;
    bool result = false;
    if (GetHighGuid() == HIGHGUID_UNIT)
        result = WorldObject::SpawnCreature(GetDbGuid(), &map);
    else if (GetHighGuid() == HIGHGUID_GAMEOBJECT)
        result = WorldObject::SpawnGameObject(GetDbGuid(), &map);
    if (result)
        SetUsed();
    m_inUse = false;
    return result;
}

SpawnManager::~SpawnManager()
{
    for (auto& groupData : m_spawnGroups)
        delete groupData.second;
}

void SpawnManager::Initialize()
{
    time_t now = time(nullptr);
    std::vector<uint32>* dynGuidCreatures = sObjectMgr.GetCreatureDynGuidForMap(m_map.GetId());
    if (dynGuidCreatures)
    {
        for (uint32 dbGuid : *dynGuidCreatures)
        {
            if (m_map.GetPersistentState()->GetCreatureRespawnTime(dbGuid) < now)
            {
                auto data = sObjectMgr.GetCreatureData(dbGuid);
                if (m_map.GetCreatureLinkingHolder()->CanSpawn(dbGuid, &m_map, nullptr, 0.f, 0.f)) // copy of Map::CanSpawn
                    m_map.GetPersistentState()->AddCreatureToGrid(dbGuid, data);
            }
            else
                AddCreature(dbGuid);
        }
    }
    std::vector<uint32>* dynGuidGameObjects = sObjectMgr.GetGameObjectDynGuidForMap(m_map.GetId());
    if (dynGuidGameObjects)
    {
        for (uint32 dbGuid : *dynGuidGameObjects)
        {
            if (m_map.GetPersistentState()->GetGORespawnTime(dbGuid) < now)
            {
                auto data = sObjectMgr.GetGOData(dbGuid);
                if (data->spawnMask != 0) // copy of Map::CanSpawn
                    m_map.GetPersistentState()->AddGameobjectToGrid(dbGuid, data);
            }
            else
                AddGameObject(dbGuid);
        }
    }

    auto spawnGroupData = m_map.GetMapDataContainer().GetSpawnGroups();
    for (auto& groupData : spawnGroupData->spawnGroupMap)
    {
        SpawnGroupEntry const& entry = groupData.second;
        if (entry.DbGuids.empty())
            continue;
        if (entry.Type == SPAWN_GROUP_CREATURE)
        {
            if (m_map.GetId() != sObjectMgr.GetCreatureData(entry.DbGuids[0].DbGuid)->mapid)
                continue;
        }
        else
        {
            if (m_map.GetId() != sObjectMgr.GetGOData(entry.DbGuids[0].DbGuid)->mapid)
                continue;
        }

        SpawnGroup* spawnGroup = nullptr;
        if (entry.Type == SPAWN_GROUP_CREATURE)
            spawnGroup = new CreatureGroup(entry, m_map);
        else
            spawnGroup = new GameObjectGroup(entry, m_map);
        m_spawnGroups.emplace(entry.Id, spawnGroup);
    }
}

void SpawnManager::AddCreature(uint32 dbguid)
{
    time_t respawnTime = m_map.GetPersistentState()->GetCreatureRespawnTime(dbguid);
    if (m_updated)
        m_deferredSpawns.emplace_back(TimePoint(std::chrono::seconds(respawnTime)), dbguid, HIGHGUID_UNIT);
    else
        m_spawns.emplace_back(TimePoint(std::chrono::seconds(respawnTime)), dbguid, HIGHGUID_UNIT);
}

void SpawnManager::AddGameObject(uint32 dbguid)
{
    time_t respawnTime = m_map.GetPersistentState()->GetGORespawnTime(dbguid);
    if (m_updated)
        m_deferredSpawns.emplace_back(TimePoint(std::chrono::seconds(respawnTime)), dbguid, HIGHGUID_GAMEOBJECT);
    else
        m_spawns.emplace_back(TimePoint(std::chrono::seconds(respawnTime)), dbguid, HIGHGUID_GAMEOBJECT);
}

void SpawnManager::RespawnCreature(uint32 dbguid, uint32 respawnDelay)
{
    bool found = false;
    auto itr = m_spawns.begin();
    for (; itr != m_spawns.end(); )
    {
        auto& spawnInfo = *itr;
        if (!spawnInfo.IsUsed() && spawnInfo.GetDbGuid() == dbguid && spawnInfo.GetHighGuid() == HIGHGUID_UNIT)
        {
            found = true;
            m_map.GetPersistentState()->SaveCreatureRespawnTime(dbguid, time(nullptr) + respawnDelay);
            if (respawnDelay > 0)
                spawnInfo.SetRespawnTime(m_map.GetCurrentClockTime() + std::chrono::seconds(respawnDelay));
            break;
        }
        ++itr;
    }
    if (!found)
    {
        m_map.GetPersistentState()->SaveCreatureRespawnTime(dbguid, time(nullptr) + respawnDelay);
        AddCreature(dbguid);
    }
    else if (respawnDelay == 0)
        (*itr).ConstructForMap(m_map);
}

void SpawnManager::RespawnGameObject(uint32 dbguid, uint32 respawnDelay)
{
    bool found = false;
    auto itr = m_spawns.begin();
    for (; itr != m_spawns.end(); )
    {
        auto& spawnInfo = *itr;
        if (!spawnInfo.IsUsed() && spawnInfo.GetDbGuid() == dbguid && spawnInfo.GetHighGuid() == HIGHGUID_GAMEOBJECT)
        {
            found = true;
            m_map.GetPersistentState()->SaveGORespawnTime(dbguid, time(nullptr) + respawnDelay);
            if (respawnDelay > 0)
                spawnInfo.SetRespawnTime(m_map.GetCurrentClockTime() + std::chrono::seconds(respawnDelay));
            break;
        }
        ++itr;
    }
    if (!found)
    {
        m_map.GetPersistentState()->SaveGORespawnTime(dbguid, time(nullptr) + respawnDelay);
        AddGameObject(dbguid);
    }
    else if (respawnDelay == 0)
        (*itr).ConstructForMap(m_map);
}

void SpawnManager::RemoveSpawns(std::vector<uint32> const& creatureDbGuids, std::vector<uint32> const& goDbGuids)
{
    for (auto& spawnInfo : m_spawns)
    {
        switch (spawnInfo.GetHighGuid())
        {
            case HIGHGUID_GAMEOBJECT:
                if (std::find(goDbGuids.begin(), goDbGuids.end(), spawnInfo.GetDbGuid()) != goDbGuids.end())
                    spawnInfo.SetUsed(); // will be erased on next manager update
                break;
            case HIGHGUID_UNIT:
                if (std::find(creatureDbGuids.begin(), creatureDbGuids.end(), spawnInfo.GetDbGuid()) != creatureDbGuids.end())
                    spawnInfo.SetUsed(); // will be erased on next manager update
                break;
            default: break;
        }
    }
}

void SpawnManager::RemoveSpawn(uint32 dbguid, HighGuid high)
{
    for (auto& spawnInfo : m_spawns)
    {
        if (spawnInfo.GetHighGuid() == high && spawnInfo.GetDbGuid() == dbguid)
        {
            spawnInfo.SetUsed(); // will be erased on next manager update
            break;
        }
    }
}

void SpawnManager::AddEventGuid(uint32 dbguid, HighGuid high)
{
    switch (high)
    {
        case HIGHGUID_GAMEOBJECT: m_eventGoDbGuids.insert(dbguid); break;
        case HIGHGUID_UNIT: m_eventCreatureDbGuids.insert(dbguid); break;
        default: break;
    }
}

void SpawnManager::RemoveEventGuid(uint32 dbguid, HighGuid high)
{
    switch (high)
    {
        case HIGHGUID_GAMEOBJECT: m_eventGoDbGuids.erase(dbguid); break;
        case HIGHGUID_UNIT: m_eventCreatureDbGuids.erase(dbguid); break;
        default: break;
    }
}

bool SpawnManager::IsEventGuid(uint32 dbguid, HighGuid high) const
{
    switch (high)
    {
        case HIGHGUID_GAMEOBJECT: return m_eventGoDbGuids.find(dbguid) != m_eventGoDbGuids.end();
        case HIGHGUID_UNIT: return m_eventCreatureDbGuids.find(dbguid) != m_eventCreatureDbGuids.end();
        default: break;
    }
    return false;
}

void SpawnManager::RespawnAll()
{
    for (auto itr = m_spawns.begin(); itr != m_spawns.end(); )
    {
        auto& spawnInfo = *itr;
        if (spawnInfo.GetHighGuid() == HIGHGUID_GAMEOBJECT)
            m_map.GetPersistentState()->SaveGORespawnTime(spawnInfo.GetDbGuid(), 0);
        if (spawnInfo.GetHighGuid() == HIGHGUID_UNIT)
            m_map.GetPersistentState()->SaveCreatureRespawnTime(spawnInfo.GetDbGuid(), 0);
        if (spawnInfo.ConstructForMap(m_map))
            itr = m_spawns.erase(itr);
        else
            ++itr;
    }
}

void SpawnManager::Update()
{
    m_updated = true;
    if (!m_deferredSpawns.empty()) // cannot insert during update
    {
        m_spawns.reserve(m_spawns.size() + m_deferredSpawns.size());
        std::move(std::begin(m_deferredSpawns), std::end(m_deferredSpawns), std::back_inserter(m_spawns));
        m_deferredSpawns.clear();
    }
    auto now = m_map.GetCurrentClockTime();
    for (auto itr = m_spawns.begin(); itr != m_spawns.end();)
    {
        auto& spawnInfo = *itr;
        if (spawnInfo.IsUsed() ||
            (spawnInfo.GetRespawnTime() <= now && spawnInfo.ConstructForMap(m_map)))
            itr = m_spawns.erase(itr);
        else
            ++itr;
    }
    m_updated = false;

    // spawn groups are safe from this
    for (auto& group : m_spawnGroups)
        group.second->Update();
}

std::string SpawnManager::GetRespawnList()
{
    std::string output = "";
    for (auto& data : m_spawns)
    {
        output += "DBGuid: " + std::to_string(data.GetDbGuid()) + "HighGuid: " + (data.GetHighGuid() == HIGHGUID_UNIT ? "Creature" : "GameObject") + "Respawn Time ";
        auto diff = (data.GetRespawnTime() - m_map.GetCurrentClockTime()).count();
        if (auto hours = diff / (HOUR * IN_MILLISECONDS))
        {
            output += std::to_string(hours) + "h ";
            diff -= hours * HOUR * IN_MILLISECONDS;
        }
        if (auto minutes = diff / (MINUTE * IN_MILLISECONDS))
        {
            output += std::to_string(minutes) + "m ";
            diff -= minutes * MINUTE * IN_MILLISECONDS;
        }
        if (auto seconds = diff / (IN_MILLISECONDS))
        {
            output += std::to_string(seconds) + "s ";
            diff -= seconds * IN_MILLISECONDS;
        }
    }
    return output;
}

SpawnGroup* SpawnManager::GetSpawnGroup(uint32 Id)
{
    auto itr = m_spawnGroups.find(Id);
    if (itr == m_spawnGroups.end())
        return nullptr;

    return (*itr).second;
}

void SpawnManager::RespawnSpawnGroupsInVicinity(Position pos, float range)
{
    for (auto& data : m_spawnGroups)
        data.second->RespawnIfInVicinity(pos, range);
}
