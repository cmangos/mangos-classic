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
SDName: Instance_Scarlet_Monastery
SD%Complete: 50
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "scarlet_monastery.h"

instance_scarlet_monastery::instance_scarlet_monastery(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_scarlet_monastery::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_scarlet_monastery::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_MOGRAINE:
        case NPC_WHITEMANE:
        case NPC_VORREL:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_scarlet_monastery::OnCreatureDeath(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_INTERROGATOR_VISHAS)
    {
        // Any other actions to do with Vorrel? setStandState?
        if (Creature* pVorrel = GetSingleCreatureFromStorage(NPC_VORREL))
            DoScriptText(SAY_TRIGGER_VORREL, pVorrel);
    }
}

void instance_scarlet_monastery::OnObjectCreate(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_WHITEMANE_DOOR)
        m_mGoEntryGuidStore[GO_WHITEMANE_DOOR] = pGo->GetObjectGuid();
}

void instance_scarlet_monastery::SetData(uint32 uiType, uint32 uiData)
{
    if (uiType == TYPE_MOGRAINE_AND_WHITE_EVENT)
    {
        if (uiData == IN_PROGRESS)
            DoUseDoorOrButton(GO_WHITEMANE_DOOR);
        if (uiData == FAIL)
            DoUseDoorOrButton(GO_WHITEMANE_DOOR);

        m_auiEncounter[0] = uiData;
    }
}

uint32 instance_scarlet_monastery::GetData(uint32 uiData) const
{
    if (uiData == TYPE_MOGRAINE_AND_WHITE_EVENT)
        return m_auiEncounter[0];

    return 0;
}

InstanceData* GetInstanceData_instance_scarlet_monastery(Map* pMap)
{
    return new instance_scarlet_monastery(pMap);
}

void AddSC_instance_scarlet_monastery()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_scarlet_monastery";
    pNewScript->GetInstanceData = &GetInstanceData_instance_scarlet_monastery;
    pNewScript->RegisterSelf();
}
