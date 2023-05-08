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
#include "AI/EventAI/CreatureEventAI.h"
#include "AI/EventAI/CreatureEventAIMgr.h"
#include "DBScripts/ScriptMgr.h"
#include "Globals/UnitCondition.h"
#include "World/WorldStateExpression.h"
#include "Globals/CombatCondition.h"

MapDataContainer::MapDataContainer() : m_spellListContainer(sObjectMgr.GetCreatureSpellListContainer()),
    m_spawnGroupContainer(sObjectMgr.GetSpawnGroupContainer()), m_CreatureEventAIEventEntryMap(sEventAIMgr.GetCreatureEventEntryAIMap()),
    m_CreatureEventAIEventGuidMap(sEventAIMgr.GetCreatureEventGuidAIMap()), m_creatureEventAIComputedDataMap(sEventAIMgr.GetEAIComputedDataMap()),
    m_stringIds(sScriptMgr.GetStringIdMap()), m_stringIdsByString(sScriptMgr.GetStringIdByStringMap()),
    m_unitConditions(sObjectMgr.GetUnitConditions()), m_worldStateExpressions(sObjectMgr.GetWorldStateExpressions()), m_combatConditions(sObjectMgr.GetCombatConditions())
{
    for (uint32 i = 0; i < SCRIPT_TYPE_MAX; ++i)
        SetScriptMap(ScriptMapType(i), sScriptMgr.GetScriptMap(ScriptMapType(i)));
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

void MapDataContainer::SetEventAIContainers(std::shared_ptr<CreatureEventAI_Event_Map> entryContainer, std::shared_ptr<CreatureEventAI_Event_Map> guidContainer, std::shared_ptr<CreatureEventAI_EventComputedData_Map> computedContainer)
{
    m_CreatureEventAIEventEntryMap = entryContainer;
    m_CreatureEventAIEventGuidMap = guidContainer;
    m_creatureEventAIComputedDataMap = computedContainer;
}

std::shared_ptr<CreatureEventAI_Event_Map> MapDataContainer::GetCreatureEventEntryAIMap() const
{
    return m_CreatureEventAIEventEntryMap;
}

std::shared_ptr<CreatureEventAI_Event_Map> MapDataContainer::GetCreatureEventGuidAIMap() const
{
    return m_CreatureEventAIEventGuidMap;
}

std::shared_ptr<CreatureEventAI_EventComputedData_Map> MapDataContainer::GetEAIComputedDataMap() const
{
    return m_creatureEventAIComputedDataMap;
}

void MapDataContainer::SetScriptMap(ScriptMapType scriptMapType, std::shared_ptr<ScriptMapMapName> scriptMap)
{
    m_scriptMaps[scriptMapType] = scriptMap;
}

std::shared_ptr<ScriptMapMapName> MapDataContainer::GetScriptMap(ScriptMapType scriptMapType)
{
    return m_scriptMaps[scriptMapType];
}

uint32 MapDataContainer::GetStringId(std::string& stringId) const
{
    auto itr = m_stringIdsByString->find(stringId);
    if (itr == m_stringIdsByString->end())
        return 0;

    return itr->second.Id;
}

void MapDataContainer::SetUnitConditions(std::shared_ptr<std::map<int32, UnitConditionEntry>> unitConditions)
{
    m_unitConditions = unitConditions;
}

void MapDataContainer::SetCombatConditions(std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> worldStateExpressions)
{
    m_worldStateExpressions = worldStateExpressions;
}

void MapDataContainer::SetWorldStateExpressions(std::shared_ptr<std::map<int32, CombatConditionEntry>> combatConditions)
{
    m_combatConditions = combatConditions;
}

std::shared_ptr<std::map<int32, UnitConditionEntry>> MapDataContainer::GetUnitConditions() const
{
    return m_unitConditions;
}

std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> MapDataContainer::GetWorldStateExpressions() const
{
    return m_worldStateExpressions;
}

std::shared_ptr<std::map<int32, CombatConditionEntry>> MapDataContainer::GetCombatConditions() const
{
    return m_combatConditions;
}

void MapDataContainer::SetStringIdMaps(std::shared_ptr<StringIdMap> stringIds, std::shared_ptr<StringIdMapByString> stringIdsByString)
{
    m_stringIds = stringIds;
    m_stringIdsByString = stringIdsByString;
}
