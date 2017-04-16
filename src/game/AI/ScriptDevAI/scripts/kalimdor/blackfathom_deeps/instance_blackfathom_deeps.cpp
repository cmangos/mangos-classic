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
SDName: Instance_Blackfathom_Deeps
SD%Complete: 50
SDComment: Quest support: 6921
SDCategory: Blackfathom Deeps
EndScriptData */

#include "precompiled.h"
#include "blackfathom_deeps.h"

/* Encounter 0 = Twilight Lord Kelris
   Encounter 1 = Shrine event
   Must kill twilight lord for shrine event to be possible
   Encounter 2 = Baron Aquanis (spawned by GO use but should only spawn once per instance)
 */

instance_blackfathom_deeps::instance_blackfathom_deeps(Map* pMap) : ScriptedInstance(pMap),
    m_uiWaveCounter(0)
{
    Initialize();
}

void instance_blackfathom_deeps::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    memset(&m_uiSpawnMobsTimer, 0, sizeof(m_uiSpawnMobsTimer));
}

void instance_blackfathom_deeps::OnCreatureCreate(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_KELRIS)
        m_mNpcEntryGuidStore[NPC_KELRIS] = pCreature->GetObjectGuid();
}

void instance_blackfathom_deeps::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_PORTAL_DOOR:
            if (m_auiEncounter[1] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);

            m_mGoEntryGuidStore[GO_PORTAL_DOOR] = pGo->GetObjectGuid();
            break;
        case GO_SHRINE_1:
        case GO_SHRINE_2:
        case GO_SHRINE_3:
        case GO_SHRINE_4:
            if (m_auiEncounter[1] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
    }
}

void instance_blackfathom_deeps::DoSpawnMobs(uint8 uiWaveIndex)
{
    Creature* pKelris = GetSingleCreatureFromStorage(NPC_KELRIS);
    if (!pKelris)
        return;

    float fX_resp, fY_resp, fZ_resp;

    pKelris->GetRespawnCoord(fX_resp, fY_resp, fZ_resp);

    for (uint8 i = 0; i < countof(aWaveSummonInformation); ++i)
    {
        if (aWaveSummonInformation[i].m_uiWaveIndex != uiWaveIndex)
            continue;

        // Summon mobs at positions
        for (uint8 j = 0; j < MAX_COUNT_POS; ++j)
        {
            for (uint8 k = 0; k < aWaveSummonInformation[i].m_aCountAndPos[j].m_uiCount; ++k)
            {
                uint8 uiPos = aWaveSummonInformation[i].m_aCountAndPos[j].m_uiSummonPosition;
                float fPosX = aSpawnLocations[uiPos].m_fX;
                float fPosY = aSpawnLocations[uiPos].m_fY;
                float fPosZ = aSpawnLocations[uiPos].m_fZ;
                float fPosO = aSpawnLocations[uiPos].m_fO;

                // Adapt fPosY slightly in case of higher summon-counts
                if (aWaveSummonInformation[i].m_aCountAndPos[j].m_uiCount > 1)
                    fPosY = fPosY - INTERACTION_DISTANCE / 2 + k * INTERACTION_DISTANCE / aWaveSummonInformation[i].m_aCountAndPos[j].m_uiCount;

                if (Creature* pSummoned = pKelris->SummonCreature(aWaveSummonInformation[i].m_uiNpcEntry, fPosX, fPosY, fPosZ, fPosO, TEMPSUMMON_DEAD_DESPAWN, 0))
                {
                    pSummoned->GetMotionMaster()->MovePoint(0, fX_resp, fY_resp, fZ_resp);
                    m_lWaveMobsGuids[uiWaveIndex].push_back(pSummoned->GetGUIDLow());
                }
            }
        }
    }
}

void instance_blackfathom_deeps::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_KELRIS:                                   // EventAI must set instance data (1,3) at his death
            if (m_auiEncounter[0] != DONE && uiData == DONE)
                m_auiEncounter[0] = uiData;
            break;
        case TYPE_SHRINE:
            m_auiEncounter[1] = uiData;
            if (uiData == IN_PROGRESS)
            {
                m_uiSpawnMobsTimer[m_uiWaveCounter] = 3000;
                ++m_uiWaveCounter;
            }
            else if (uiData == DONE)
                DoUseDoorOrButton(GO_PORTAL_DOOR);
            break;
        case TYPE_AQUANIS:
            m_auiEncounter[2] = uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;

        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2];

        m_strInstData = saveStream.str();
        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_blackfathom_deeps::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_KELRIS: return m_auiEncounter[0];
        case TYPE_SHRINE: return m_auiEncounter[1];
        case TYPE_AQUANIS: return m_auiEncounter[2];
        default:
            return 0;
    }
}

void instance_blackfathom_deeps::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_blackfathom_deeps::OnCreatureDeath(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_BARON_AQUANIS)
        SetData(TYPE_AQUANIS, DONE);

    // Only use this function if shrine event is in progress
    if (m_auiEncounter[1] != IN_PROGRESS)
        return;

    switch (pCreature->GetEntry())
    {
        case NPC_AKUMAI_SERVANT:
            m_lWaveMobsGuids[1].remove(pCreature->GetGUIDLow());
            break;
        case NPC_AKUMAI_SNAPJAW:
            m_lWaveMobsGuids[0].remove(pCreature->GetGUIDLow());
            break;
        case NPC_BARBED_CRUSTACEAN:
            m_lWaveMobsGuids[2].remove(pCreature->GetGUIDLow());
            break;
        case NPC_MURKSHALLOW_SOFTSHELL:
            m_lWaveMobsGuids[3].remove(pCreature->GetGUIDLow());
            break;
    }

    if (IsWaveEventFinished())
        SetData(TYPE_SHRINE, DONE);
}

// Check if all the summoned event mobs are dead
bool instance_blackfathom_deeps::IsWaveEventFinished()
{
    // If not all fires are lighted return
    if (m_uiWaveCounter < MAX_FIRES)
        return false;

    // Check if all mobs are dead
    for (uint8 i = 0; i < MAX_FIRES; ++i)
    {
        if (!m_lWaveMobsGuids[i].empty())
            return false;
    }

    return true;
}

void instance_blackfathom_deeps::Update(uint32 uiDiff)
{
    // Only use this function if shrine event is in progress
    if (m_auiEncounter[1] != IN_PROGRESS)
        return;

    for (uint8 i = 0; i < MAX_FIRES; ++i)
    {
        if (m_uiSpawnMobsTimer[i])
        {
            if (m_uiSpawnMobsTimer[i] <= uiDiff)
            {
                DoSpawnMobs(i);
                m_uiSpawnMobsTimer[i] = 0;
            }
            else
                m_uiSpawnMobsTimer[i] -= uiDiff;
        }
    }
}

InstanceData* GetInstanceData_instance_blackfathom_deeps(Map* pMap)
{
    return new instance_blackfathom_deeps(pMap);
}

bool GOUse_go_fire_of_akumai(Player* /*pPlayer*/, GameObject* pGo)
{
    instance_blackfathom_deeps* pInstance = (instance_blackfathom_deeps*)pGo->GetInstanceData();

    if (!pInstance)
        return true;

    if (pInstance->GetData(TYPE_SHRINE) == DONE)
        return true;

    if (pInstance->GetData(TYPE_KELRIS) == DONE)
    {
        pInstance->SetData(TYPE_SHRINE, IN_PROGRESS);
        return false;
    }

    return true;
}

bool GOUse_go_fathom_stone(Player* pPlayer, GameObject* pGo)
{
    instance_blackfathom_deeps* pInstance = (instance_blackfathom_deeps*)pGo->GetInstanceData();
    if (!pInstance)
        return true;

    if (pInstance->GetData(TYPE_AQUANIS) == NOT_STARTED)
    {
        pPlayer->SummonCreature(NPC_BARON_AQUANIS, afAquanisPos[0], afAquanisPos[1], afAquanisPos[2], afAquanisPos[3], TEMPSUMMON_DEAD_DESPAWN, 0);
        pInstance->SetData(TYPE_AQUANIS, IN_PROGRESS);
    }

    return false;
}

void AddSC_instance_blackfathom_deeps()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_blackfathom_deeps";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackfathom_deeps;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_fire_of_akumai";
    pNewScript->pGOUse = &GOUse_go_fire_of_akumai;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_fathom_stone";
    pNewScript->pGOUse = &GOUse_go_fathom_stone;
    pNewScript->RegisterSelf();
}
