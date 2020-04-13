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

/* ScriptData
SDName: Instance_Onyxias_Lair
SD%Complete: 80%
SDComment: Quel'Serrar forging event is missing. Whelp summoning would be improved by GO casting.
SDCategory: Onyxia's Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "onyxias_lair.h"

instance_onyxias_lair::instance_onyxias_lair(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_onyxias_lair::Initialize()
{
    m_uiEncounter = NOT_STARTED;
    m_tPhaseTwoStart = time(nullptr);
}

bool instance_onyxias_lair::IsEncounterInProgress() const
{
    return m_uiEncounter == IN_PROGRESS || m_uiEncounter >= DATA_LIFTOFF;
}

void instance_onyxias_lair::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ONYXIA_TRIGGER:
            m_npcEntryGuidStore[NPC_ONYXIA_TRIGGER] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_onyxias_lair::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        // Keep track of all killed Warders to respawn them while Onyxia is in combat
        case NPC_ONYXIAN_WARDER:
            m_lWarderGUIDList.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_onyxias_lair::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_WHELP_SPAWNER:
            if (Creature* pTrigger = GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
            {
                pTrigger->SummonCreature(NPC_ONYXIA_WHELP, pGo->GetPositionX(), pGo->GetPositionY(), pGo->GetPositionZ(), 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, MINUTE * IN_MILLISECONDS);
                // TODO when GO spell casting is added in core, replace above code by (or something similar):
                // pGO->CastSpell(pGO, SPELL_SUMMONWHELP, TRIGGERED_NONE);
            }
            break;
    }
}

void instance_onyxias_lair::SetData(uint32 uiType, uint32 uiData)
{
    if (uiType != TYPE_ONYXIA)
        return;

    m_uiEncounter = uiData;

    switch (uiData)
    {
        case DATA_LIFTOFF:
            m_tPhaseTwoStart = time(nullptr);
            break;
        case IN_PROGRESS:
            // Respawn dead Onyxian Warders
            for (GuidList::const_iterator itr = m_lWarderGUIDList.begin(); itr != m_lWarderGUIDList.end(); ++itr)
            {
                if (Creature* pWarder = instance->GetCreature(*itr))
                {
                    if (!pWarder->IsAlive())
                        pWarder->Respawn();
                }
            }
            break;
        case FAIL:
        case DONE:
            // Despawn Onyxian Warders that were respawned during fight
            for (GuidList::const_iterator itr = m_lWarderGUIDList.begin(); itr != m_lWarderGUIDList.end(); ++itr)
            {
                if (Creature* pWarder = instance->GetCreature(*itr))
                    pWarder->ForcedDespawn();
            }
            break;
    }

    // Currently no reason to save anything
}

InstanceData* GetInstanceData_instance_onyxias_lair(Map* pMap)
{
    return new instance_onyxias_lair(pMap);
}

void AddSC_instance_onyxias_lair()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_onyxias_lair";
    pNewScript->GetInstanceData = &GetInstanceData_instance_onyxias_lair;
    pNewScript->RegisterSelf();
}
