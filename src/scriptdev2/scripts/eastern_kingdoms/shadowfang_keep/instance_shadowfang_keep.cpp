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
SDName: Instance_Shadowfang_Keep
SD%Complete: 90
SDComment:
SDCategory: Shadowfang Keep
EndScriptData */

#include "precompiled.h"
#include "shadowfang_keep.h"

instance_shadowfang_keep::instance_shadowfang_keep(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_shadowfang_keep::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_shadowfang_keep::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ASH:
        case NPC_ADA:
        case NPC_FENRUS:
            break;
        case NPC_VINCENT:
            // If Arugal has done the intro, make Vincent dead!
            if (m_auiEncounter[4] == DONE)
                pCreature->SetStandState(UNIT_STAND_STATE_DEAD);
            break;

        default:
            return;
    }
    m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
}

void instance_shadowfang_keep::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_COURTYARD_DOOR:
            if (m_auiEncounter[0] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
            // For this we ignore voidwalkers, because if the server restarts
            // They won't be there, but Fenrus is dead so the door can't be opened!
        case GO_SORCERER_DOOR:
            if (m_auiEncounter[2] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARUGAL_DOOR:
            if (m_auiEncounter[3] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARUGAL_FOCUS:
            break;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_shadowfang_keep::DoSpeech()
{
    Creature* pAda = GetSingleCreatureFromStorage(NPC_ADA);
    Creature* pAsh = GetSingleCreatureFromStorage(NPC_ASH);

    if (pAda && pAda->isAlive() && pAsh && pAsh->isAlive())
    {
        DoScriptText(SAY_BOSS_DIE_AD, pAda);
        DoScriptText(SAY_BOSS_DIE_AS, pAsh);
    }
}

void instance_shadowfang_keep::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_FREE_NPC:
            if (uiData == DONE)
                DoUseDoorOrButton(GO_COURTYARD_DOOR);
            m_auiEncounter[0] = uiData;
            break;
        case TYPE_RETHILGORE:
            if (uiData == DONE)
                DoSpeech();
            m_auiEncounter[1] = uiData;
            break;
        case TYPE_FENRUS:
            if (uiData == DONE)
            {
                if (Creature* pFenrus = GetSingleCreatureFromStorage(NPC_FENRUS))
                    pFenrus->SummonCreature(NPC_ARCHMAGE_ARUGAL, -136.89f, 2169.17f, 136.58f, 2.794f, TEMPSUMMON_TIMED_DESPAWN, 30000);
            }
            m_auiEncounter[2] = uiData;
            break;
        case TYPE_NANDOS:
            if (uiData == DONE)
                DoUseDoorOrButton(GO_ARUGAL_DOOR);
            m_auiEncounter[3] = uiData;
            break;
        case TYPE_INTRO:
            m_auiEncounter[4] = uiData;
            break;
        case TYPE_VOIDWALKER:
            if (uiData == DONE)
            {
                m_auiEncounter[5]++;
                if (m_auiEncounter[5] > 3)
                    DoUseDoorOrButton(GO_SORCERER_DOOR);
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3]
                   << " " << m_auiEncounter[4] << " " << m_auiEncounter[5];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_shadowfang_keep::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_FREE_NPC:   return m_auiEncounter[0];
        case TYPE_RETHILGORE: return m_auiEncounter[1];
        case TYPE_FENRUS:     return m_auiEncounter[2];
        case TYPE_NANDOS:     return m_auiEncounter[3];
        case TYPE_INTRO:      return m_auiEncounter[4];

        default:
            return 0;
    }
}

void instance_shadowfang_keep::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

InstanceData* GetInstanceData_instance_shadowfang_keep(Map* pMap)
{
    return new instance_shadowfang_keep(pMap);
}

void AddSC_instance_shadowfang_keep()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_shadowfang_keep";
    pNewScript->GetInstanceData = &GetInstanceData_instance_shadowfang_keep;
    pNewScript->RegisterSelf();
}
