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

#ifndef MAP_DATA_CONTAINER_H
#define MAP_DATA_CONTAINER_H

#include "Platform/Define.h"
#include "DBScripts/ScriptMgrDefines.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

struct CreatureSpellListContainer;
struct CreatureSpellList;
struct SpawnGroupEntry;
struct SpawnGroupEntryContainer;
struct CreatureEventAI_Event;
struct CreatureEventAI_EventComputedData;
struct ScriptInfo;
struct UnitConditionEntry;
struct WorldStateExpressionEntry;
struct CombatConditionEntry;

// Event_Map
typedef std::vector<CreatureEventAI_Event> CreatureEventAI_Event_Vec;
typedef std::unordered_map<uint32, CreatureEventAI_Event_Vec> CreatureEventAI_Event_Map;
typedef std::unordered_map<uint32, CreatureEventAI_EventComputedData> CreatureEventAI_EventComputedData_Map;

// Scripts
typedef std::multimap < uint32 /*delay*/, std::shared_ptr<ScriptInfo>> ScriptMap;
typedef std::map < uint32 /*id*/, ScriptMap > ScriptMapMap;
typedef std::pair<const char*, ScriptMapMap> ScriptMapMapName;

class MapDataContainer
{
    public:
        MapDataContainer();
        void SetCreatureSpellListContainer(std::shared_ptr<CreatureSpellListContainer> container);
        CreatureSpellList* GetCreatureSpellList(uint32 Id) const;
        SpawnGroupEntry* GetSpawnGroup(uint32 Id) const;
        SpawnGroupEntry* GetSpawnGroupByGuid(uint32 dbGuid, uint32 high) const;
        std::shared_ptr<SpawnGroupEntryContainer> GetSpawnGroups() const;

        void SetEventAIContainers(std::shared_ptr<CreatureEventAI_Event_Map> entryContainer, std::shared_ptr<CreatureEventAI_Event_Map> guidContainer, std::shared_ptr<CreatureEventAI_EventComputedData_Map> computedContainer);
        std::shared_ptr<CreatureEventAI_Event_Map> GetCreatureEventEntryAIMap() const;
        std::shared_ptr<CreatureEventAI_Event_Map> GetCreatureEventGuidAIMap() const;
        std::shared_ptr<CreatureEventAI_EventComputedData_Map> GetEAIComputedDataMap() const;

        void SetScriptMap(ScriptMapType scriptMapType, std::shared_ptr<ScriptMapMapName> scriptMap);
        std::shared_ptr<ScriptMapMapName> GetScriptMap(ScriptMapType scriptMapType);

        void SetStringIdMaps(std::shared_ptr<StringIdMap> stringIds, std::shared_ptr<StringIdMapByString> stringIdsByString);
        uint32 GetStringId(std::string& stringId) const;

        void SetUnitConditions(std::shared_ptr<std::map<int32, UnitConditionEntry>> unitConditions);
        void SetCombatConditions(std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> worldStateExpressions);
        void SetWorldStateExpressions(std::shared_ptr<std::map<int32, CombatConditionEntry>> combatConditions);
        std::shared_ptr<std::map<int32, UnitConditionEntry>> GetUnitConditions() const;
        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> GetWorldStateExpressions() const;
        std::shared_ptr<std::map<int32, CombatConditionEntry>> GetCombatConditions() const;
    private:
        std::shared_ptr<CreatureSpellListContainer> m_spellListContainer;
        std::shared_ptr<SpawnGroupEntryContainer> m_spawnGroupContainer;

        std::shared_ptr<CreatureEventAI_Event_Map>  m_CreatureEventAIEventEntryMap;
        std::shared_ptr<CreatureEventAI_Event_Map>  m_CreatureEventAIEventGuidMap;
        std::shared_ptr<CreatureEventAI_EventComputedData_Map>  m_creatureEventAIComputedDataMap;

        std::shared_ptr<ScriptMapMapName> m_scriptMaps[SCRIPT_TYPE_MAX];

        // SCRIPT_ID
        std::shared_ptr<StringIdMap> m_stringIds;
        std::shared_ptr<StringIdMapByString> m_stringIdsByString;

        std::shared_ptr<std::map<int32, UnitConditionEntry>> m_unitConditions;
        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> m_worldStateExpressions;
        std::shared_ptr<std::map<int32, CombatConditionEntry>> m_combatConditions;
};

#endif