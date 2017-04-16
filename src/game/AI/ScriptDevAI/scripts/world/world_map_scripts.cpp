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
SDName: world_map_scripts
SD%Complete: 100
SDComment: Quest support: 4740, 11538
SDCategory: World Map Scripts
EndScriptData */

#include "precompiled.h"
#include "world_map_scripts.h"

/* *********************************************************
 *                  EASTERN KINGDOMS
 */
struct world_map_eastern_kingdoms : public ScriptedMap
{
    world_map_eastern_kingdoms(Map* pMap) : ScriptedMap(pMap) {}

    void OnCreatureCreate(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            case NPC_JONATHAN:
            case NPC_WRYNN:
            case NPC_BOLVAR:
            case NPC_PRESTOR:
            case NPC_WINDSOR:
                m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
        }
    }

    void SetData(uint32 /*uiType*/, uint32 /*uiData*/) {}
};

InstanceData* GetInstanceData_world_map_eastern_kingdoms(Map* pMap)
{
    return new world_map_eastern_kingdoms(pMap);
}

/* *********************************************************
 *                     KALIMDOR
 */
struct world_map_kalimdor : public ScriptedMap
{
    world_map_kalimdor(Map* pMap) : ScriptedMap(pMap) { Initialize(); }

    uint8 m_uiMurkdeepAdds_KilledAddCount;

    void Initialize()
    {
        m_uiMurkdeepAdds_KilledAddCount = 0;
    }

    void OnCreatureCreate(Creature* pCreature)
    {
        if (pCreature->GetEntry() == NPC_MURKDEEP)
            m_mNpcEntryGuidStore[NPC_MURKDEEP] = pCreature->GetObjectGuid();
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            case NPC_GREYMIST_COASTRUNNNER:
                if (pCreature->IsTemporarySummon())         // Only count the ones summoned for Murkdeep quest
                {
                    ++m_uiMurkdeepAdds_KilledAddCount;

                    // If all 3 coastrunners are killed, summon 2 warriors
                    if (m_uiMurkdeepAdds_KilledAddCount == 3)
                    {
                        float fX, fY, fZ;
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            pCreature->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][0], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][1], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][2], 5.0f, fX, fY, fZ);

                            if (Creature* pTemp = pCreature->SummonCreature(NPC_GREYMIST_WARRIOR, fX, fY, fZ, aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][3], TEMPSUMMON_DEAD_DESPAWN, 0))
                            {
                                pTemp->SetWalk(false);
                                pTemp->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_MOVE][0], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][1], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][2], 5.0f, fX, fY, fZ);
                                pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                            }
                        }

                        m_uiMurkdeepAdds_KilledAddCount = 0;
                    }
                }
                break;
            case NPC_GREYMIST_WARRIOR:
                if (pCreature->IsTemporarySummon())         // Only count the ones summoned for Murkdeep quest
                {
                    ++m_uiMurkdeepAdds_KilledAddCount;

                    // After the 2 warriors are killed, Murkdeep spawns, along with a hunter
                    if (m_uiMurkdeepAdds_KilledAddCount == 2)
                    {
                        float fX, fY, fZ;
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            pCreature->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][0], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][1], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][2], 5.0f, fX, fY, fZ);

                            if (Creature* pTemp = pCreature->SummonCreature(!i ? NPC_MURKDEEP : NPC_GREYMIST_HUNTER, fX, fY, fZ, aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][3], TEMPSUMMON_DEAD_DESPAWN, 0))
                            {
                                pTemp->SetWalk(false);
                                pTemp->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_MOVE][0], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][1], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][2], 5.0f, fX, fY, fZ);
                                pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                            }
                        }

                        m_uiMurkdeepAdds_KilledAddCount = 0;
                    }
                }
                break;
        }
    }

    void SetData(uint32 /*uiType*/, uint32 /*uiData*/) {}
};

InstanceData* GetInstanceData_world_map_kalimdor(Map* pMap)
{
    return new world_map_kalimdor(pMap);
}

void AddSC_world_map_scripts()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "world_map_eastern_kingdoms";
    pNewScript->GetInstanceData = &GetInstanceData_world_map_eastern_kingdoms;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "world_map_kalimdor";
    pNewScript->GetInstanceData = &GetInstanceData_world_map_kalimdor;
    pNewScript->RegisterSelf();
}
