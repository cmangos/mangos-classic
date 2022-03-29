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

#include "MapDataContainer.h"
#include "Globals/ObjectMgr.h"

MapDataContainer::MapDataContainer() : m_spellListContainer(sObjectMgr.GetCreatureSpellListContainer()),
    m_spawnGroupContainer(sObjectMgr.GetSpawnGroupContainer())
{
}

void MapDataContainer::SetCreatureSpellListContainer(std::shared_ptr<CreatureSpellListContainer> container)
{
    m_spellListContainer = container;
}

CreatureSpellList* MapDataContainer::GetCreatureSpellList(uint32 Id) const
{
    auto itr = m_spellListContainer->spellLists.find(Id);
    if (itr == m_spellListContainer->spellLists.end())
        return nullptr;

    return &(*itr).second;
}

SpawnGroupEntry* MapDataContainer::GetSpawnGroup(uint32 Id) const
{
    auto itr = m_spawnGroupContainer->spawnGroupMap.find(Id);
    if (itr == m_spawnGroupContainer->spawnGroupMap.end())
        return nullptr;

    return &(*itr).second;
}

SpawnGroupEntry* MapDataContainer::GetSpawnGroupByGuid(uint32 dbGuid, uint32 high) const
{
    auto itr = m_spawnGroupContainer->spawnGroupByGuidMap.find(std::make_pair(dbGuid, uint32(high)));
    if (itr == m_spawnGroupContainer->spawnGroupByGuidMap.end())
        return nullptr;

    return (*itr).second;
}

std::shared_ptr<SpawnGroupEntryContainer> MapDataContainer::GetSpawnGroups() const
{
    return m_spawnGroupContainer;
}
