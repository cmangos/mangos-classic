/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#include "world_eastern_kingdoms.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/scripts/world/world_map_scripts.h"
#include "World/WorldState.h"
#include "World/WorldStateDefines.h"
#include "GameEvents/GameEventMgr.h"
#include "AI/ScriptDevAI/scripts/world/scourge_invasion.h"

/* *********************************************************
 *                  EASTERN KINGDOMS
 */
struct world_map_eastern_kingdoms : public ScriptedMap
{
    world_map_eastern_kingdoms(Map* pMap) : ScriptedMap(pMap)
    {
        Initialize();
    }

    void Initialize() override
    {
        instance->GetVariableManager().SetVariable(WORLD_STATE_CUSTOM_STV_GRP_01, urand(0, 1));
        instance->GetVariableManager().SetVariable(WORLD_STATE_CUSTOM_STV_GRP_02, urand(0, 1));
    }

    void OnCreatureGroupDespawn(CreatureGroup* creatureGroup, Creature* /*creature*/) override
    {
        switch (creatureGroup->GetGroupEntry().WorldStateCondition)
        {
            case 9900: case 9901: // TODO: Propagate respawn info so they do not instant respawn
                instance->GetVariableManager().SetVariable(WORLD_STATE_CUSTOM_STV_GRP_01, urand(0, 1));
                break;
            case 9902: case 9903:
                instance->GetVariableManager().SetVariable(WORLD_STATE_CUSTOM_STV_GRP_02, urand(0, 1));
                break;
        }
    }

    void OnCreatureCreate(Creature* pCreature) override
    {
        switch (pCreature->GetEntry())
        {
            case NPC_JONATHAN:
            case NPC_WRYNN:
            case NPC_BOLVAR:
            case NPC_PRESTOR:
            case NPC_WINDSOR:
            case NPC_FALSTAD_WILDHAMMER:
            case NPC_SHORT_JOHN_MITHRIL:
                m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;
            case NPC_NECROPOLIS_HEALTH:
                m_npcEntryGuidCollection[pCreature->GetEntry()].push_back(pCreature->GetObjectGuid());
                break;
        }
    }

    void OnCreatureDeath(Creature* pCreature) override
    {
        switch (pCreature->GetEntry())
        {
            case NPC_NECROPOLIS_HEALTH:
                m_npcEntryGuidCollection.erase(pCreature->GetObjectGuid());
                break;
            default:
                break;
        }
    }

    void OnObjectCreate(GameObject* pGo) override
    {
        switch (pGo->GetEntry())
        {
            case GO_SUMMON_CIRCLE:
                m_goEntryGuidCollection[pGo->GetEntry()].push_back(pGo->GetObjectGuid());
                break;
        }
    }

    void OnEventHappened(uint16 event_id, bool activate, bool resume) override
    {
        if (event_id == GAME_EVENT_GURUBASHI_ARENA && activate)
        {
            if (Creature* creature = GetSingleCreatureFromStorage(NPC_SHORT_JOHN_MITHRIL))
                creature->GetMotionMaster()->MoveWaypoint();
        }
    }

    void SetData(uint32 /*uiType*/, uint32 /*uiData*/) override {}
};

InstanceData* GetInstanceData_world_map_eastern_kingdoms(Map* pMap)
{
    return new world_map_eastern_kingdoms(pMap);
}

void AddSC_world_eastern_kingdoms()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "world_map_eastern_kingdoms";
    pNewScript->GetInstanceData = &GetInstanceData_world_map_eastern_kingdoms;
    pNewScript->RegisterSelf();
}
