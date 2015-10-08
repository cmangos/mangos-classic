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
SDName: instance_razorfen_downs
SD%Complete: 90%
SDComment: Spawns coords can be improved
SDCategory: Razorfen Downs
EndScriptData */

#include "precompiled.h"
#include "razorfen_downs.h"

instance_razorfen_downs::instance_razorfen_downs(Map* pMap) : ScriptedInstance(pMap),
    m_uiWaveCounter(0)
{
    Initialize();
}

void instance_razorfen_downs::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_razorfen_downs::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_TOMB_FIEND:
        case NPC_TOMB_REAVER:
            m_lSpawnedMobsList.push_back(pCreature->GetObjectGuid());
            return;
    }
}

void instance_razorfen_downs::OnObjectCreate(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_GONG)
        m_mGoEntryGuidStore[GO_GONG] = pGo->GetObjectGuid();
}

void instance_razorfen_downs::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_TUTEN_KASH:
            m_auiEncounter[uiType] = uiData;
            if (uiData == IN_PROGRESS)
                ++m_uiWaveCounter;
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

uint32 instance_razorfen_downs::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_razorfen_downs::Load(const char* chrIn)
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

void instance_razorfen_downs::OnCreatureDeath(Creature* pCreature)
{
    // Only use this function if gong event is in progress
    if (GetData(TYPE_TUTEN_KASH) != IN_PROGRESS)
        return;

    switch (pCreature->GetEntry())
    {
        case NPC_TOMB_FIEND:
        case NPC_TOMB_REAVER:
            m_lSpawnedMobsList.remove(pCreature->GetObjectGuid());

            // No more wave-mobs around, enable the gong for the next wave
            if (m_lSpawnedMobsList.empty())
            {
                if (GameObject* pGo = GetSingleGameObjectFromStorage(GO_GONG))
                {
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);

                    // Workaround - GO need to be respawned - requires core fix, GO shouldn't despawn in the first place
                    pGo->Respawn();
                }
            }
            break;
        case NPC_TUTEN_KASH:
            SetData(TYPE_TUTEN_KASH, DONE);
            break;
    }
}

void instance_razorfen_downs::DoSpawnWaveIfCan(GameObject* pGo)
{
    // safety checks
    if (GetData(TYPE_TUTEN_KASH) == DONE)
        return;

    if (!m_lSpawnedMobsList.empty())
        return;

    if (m_uiWaveCounter >= MAX_WAVES)
        return;

    for (uint8 i = 0; i < aWaveSummonInformation[m_uiWaveCounter].m_uiNPCperWave; ++i)
    {
        uint8 uiPos = i % 2;                                        // alternate spawn between the left and right corridor
        float fPosX, fPosY, fPosZ;
        float fTargetPosX, fTargetPosY, fTargetPosZ;

        pGo->GetRandomPoint(aSpawnLocations[uiPos].m_fX, aSpawnLocations[uiPos].m_fY, aSpawnLocations[uiPos].m_fZ, 5.0f, fPosX, fPosY, fPosZ);

        // move the summoned NPC toward the gong
        if (Creature* pSummoned = pGo->SummonCreature(aWaveSummonInformation[m_uiWaveCounter].m_uiNpcEntry, fPosX, fPosY, fPosZ, aSpawnLocations[uiPos].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            pSummoned->SetWalk(false);
            pGo->GetContactPoint(pSummoned, fTargetPosX, fTargetPosY, fTargetPosZ);
            pSummoned->GetMotionMaster()->MovePoint(0, fTargetPosX, fTargetPosY, fTargetPosZ);
        }
    }

    // Will increase m_uiWaveCounter, hence after the wave is summoned
    SetData(TYPE_TUTEN_KASH, IN_PROGRESS);
    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
}

InstanceData* GetInstanceData_instance_razorfen_downs(Map* pMap)
{
    return new instance_razorfen_downs(pMap);
}

bool ProcessEventId_event_go_tutenkash_gong(uint32 /*uiEventId*/, Object* pSource, Object* pTarget, bool /*bIsStart*/)
{
    if (pSource->GetTypeId() == TYPEID_PLAYER && pTarget->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        instance_razorfen_downs* pInstance = (instance_razorfen_downs*)((Player*)pSource)->GetInstanceData();
        if (!pInstance)
            return true;

        pInstance->DoSpawnWaveIfCan((GameObject*)pTarget);
        return true;
    }

    return false;
}

void AddSC_instance_razorfen_downs()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_razorfen_downs";
    pNewScript->GetInstanceData = &GetInstanceData_instance_razorfen_downs;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_go_tutenkash_gong";
    pNewScript->pProcessEventId = &ProcessEventId_event_go_tutenkash_gong;
    pNewScript->RegisterSelf();
}
