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
SDName: Instance_Wailing_Caverns
SD%Complete: 90
SDComment:
SDCategory: Wailing Caverns
EndScriptData */

#include "precompiled.h"
#include "wailing_caverns.h"

instance_wailing_caverns::instance_wailing_caverns(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_wailing_caverns::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_wailing_caverns::OnPlayerEnter(Player* pPlayer)
{
    // Respawn the Mysterious chest if one of the players who enter the instance has the quest in his log
    if (pPlayer->GetQuestStatus(QUEST_FORTUNE_AWAITS) == QUEST_STATUS_COMPLETE &&
            !pPlayer->GetQuestRewardStatus(QUEST_FORTUNE_AWAITS))
        DoRespawnGameObject(GO_MYSTERIOUS_CHEST, HOUR);
}

void instance_wailing_caverns::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_NARALEX:
        case NPC_DISCIPLE:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_wailing_caverns::OnObjectCreate(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_MYSTERIOUS_CHEST)
        m_mGoEntryGuidStore[GO_MYSTERIOUS_CHEST] = pGo->GetObjectGuid();
}

void instance_wailing_caverns::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ANACONDRA:
            m_auiEncounter[0] = uiData;
            break;
        case TYPE_COBRAHN:
            m_auiEncounter[1] = uiData;
            break;
        case TYPE_PYTHAS:
            m_auiEncounter[2] = uiData;
            break;
        case TYPE_SERPENTIS:
            m_auiEncounter[3] = uiData;
            break;
        case TYPE_DISCIPLE:
            m_auiEncounter[4] = uiData;
            break;
        case TYPE_MUTANUS:
            m_auiEncounter[5] = uiData;
            break;
    }

    // Set to special in order to start the escort event; only if all four bosses are done
    if (m_auiEncounter[0] == DONE && m_auiEncounter[1] == DONE && m_auiEncounter[2] == DONE && m_auiEncounter[3] == DONE && (m_auiEncounter[4] == NOT_STARTED || m_auiEncounter[4] == FAIL))
    {
        // Yell intro text; only the first time
        if (m_auiEncounter[4] == NOT_STARTED)
        {
            if (Creature* pDisciple = GetSingleCreatureFromStorage(NPC_DISCIPLE))
                DoScriptText(SAY_INTRO, pDisciple);
        }

        m_auiEncounter[4] = SPECIAL;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;

        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5];

        m_strInstData = saveStream.str();
        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_wailing_caverns::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2]
               >> m_auiEncounter[3] >> m_auiEncounter[4] >> m_auiEncounter[5];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_wailing_caverns::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

InstanceData* GetInstanceData_instance_wailing_caverns(Map* pMap)
{
    return new instance_wailing_caverns(pMap);
}

void AddSC_instance_wailing_caverns()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_wailing_caverns";
    pNewScript->GetInstanceData = &GetInstanceData_instance_wailing_caverns;
    pNewScript->RegisterSelf();
}
