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

#include "Maps/SpawnManager.h"
#include "Maps/Map.h"

bool operator<(SpawnInfo const& lhs, SpawnInfo const& rhs)
{
    return lhs.GetRespawnTime() < rhs.GetRespawnTime();
}

bool SpawnInfo::ConstructForMap(Map& map)
{
    if (GetHighGuid() == HIGHGUID_UNIT)
        return WorldObject::SpawnCreature(GetDbGuid(), &map);
    else if (GetHighGuid() == HIGHGUID_GAMEOBJECT)
        return WorldObject::SpawnGameObject(GetDbGuid(), &map);
    return false;
}

void SpawnManager::AddCreature(uint32 respawnDelay, uint32 dbguid)
{
    m_spawns.emplace_back(m_map.GetCurrentClockTime() + std::chrono::seconds(respawnDelay), dbguid, HIGHGUID_UNIT);
    std::sort(m_spawns.begin(), m_spawns.end());
}

void SpawnManager::RespawnCreature(uint32 dbguid)
{

}

void SpawnManager::Update()
{
    auto& now = m_map.GetCurrentClockTime();
    for (auto itr = m_spawns.begin(); itr != m_spawns.end();)
    {
        auto& spawnInfo = *itr;
        if (spawnInfo.GetRespawnTime() <= now && spawnInfo.ConstructForMap(m_map))
            itr = m_spawns.erase(itr);
        else
            break;
    }
}
