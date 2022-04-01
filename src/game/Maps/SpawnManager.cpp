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

void SpawnManager::AddCreature(uint32 respawnDelay, uint32 dbguid)
{
    m_spawns.emplace_back(m_map.GetCurrentClockTime() + std::chrono::seconds(respawnDelay), dbguid, HIGHGUID_UNIT);
    std::sort(m_spawns.begin(), m_spawns.end());
}

void SpawnManager::AddGameObject(uint32 respawnDelay, uint32 dbguid)
{
    m_spawns.emplace_back(m_map.GetCurrentClockTime() + std::chrono::seconds(respawnDelay), dbguid, HIGHGUID_GAMEOBJECT);
    std::sort(m_spawns.begin(), m_spawns.end());
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
        AddCreature(respawnDelay, dbguid);
    else if (respawnDelay == 0)
        (*itr).ConstructForMap(m_map);
    if (respawnDelay > 0)
        std::sort(m_spawns.begin(), m_spawns.end());
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
        AddGameObject(respawnDelay, dbguid);
    else if (respawnDelay == 0)
        (*itr).ConstructForMap(m_map);
    if (respawnDelay > 0)
        std::sort(m_spawns.begin(), m_spawns.end());
}

void SpawnManager::RespawnAll()
{
    for (auto itr = m_spawns.begin(); itr != m_spawns.end();)
    {
        auto& spawnInfo = *itr;
        if (spawnInfo.GetHighGuid() == HIGHGUID_GAMEOBJECT)
            m_map.GetPersistentState()->SaveGORespawnTime(spawnInfo.GetDbGuid(), 0);
        if (spawnInfo.GetHighGuid() == HIGHGUID_UNIT)
            m_map.GetPersistentState()->SaveCreatureRespawnTime(spawnInfo.GetDbGuid(), 0);
        spawnInfo.ConstructForMap(m_map);
    }
}

void SpawnManager::Update()
{
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
