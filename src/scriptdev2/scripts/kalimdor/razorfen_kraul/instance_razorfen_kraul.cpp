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
SDName: instance_razorfen_kraul
SD%Complete: 50
SDComment:
SDCategory: Razorfen Kraul
EndScriptData */

#include "precompiled.h"
#include "razorfen_kraul.h"

instance_razorfen_kraul::instance_razorfen_kraul(Map* pMap) : ScriptedInstance(pMap),
    m_uiWardKeepersRemaining(0)
{
    Initialize();
}

void instance_razorfen_kraul::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_razorfen_kraul::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_AGATHELOS_WARD:
            m_mGoEntryGuidStore[GO_AGATHELOS_WARD] = pGo->GetObjectGuid();
            if (m_auiEncounter[0] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
    }
}

void instance_razorfen_kraul::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_WARD_KEEPER:
            ++m_uiWardKeepersRemaining;
            break;
    }
}

void instance_razorfen_kraul::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_AGATHELOS:
            --m_uiWardKeepersRemaining;
            if (!m_uiWardKeepersRemaining)
            {
                m_auiEncounter[0] = uiData;
                DoUseDoorOrButton(GO_AGATHELOS_WARD);
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;

        saveStream << m_auiEncounter[0];
        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_razorfen_kraul::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_razorfen_kraul::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_AGATHELOS:
            return m_auiEncounter[0];
    }
    return 0;
}

InstanceData* GetInstanceData_instance_razorfen_kraul(Map* pMap)
{
    return new instance_razorfen_kraul(pMap);
}

void AddSC_instance_razorfen_kraul()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_razorfen_kraul";
    pNewScript->GetInstanceData = &GetInstanceData_instance_razorfen_kraul;
    pNewScript->RegisterSelf();
}
