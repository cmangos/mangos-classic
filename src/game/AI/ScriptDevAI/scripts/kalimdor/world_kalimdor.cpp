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

#include "AI/ScriptDevAI/include/sc_common.h"
#include "world_kalimdor.h"
#include "AI/ScriptDevAI/scripts/world/world_map_scripts.h"
#include "World/WorldState.h"
#include "World/WorldStateDefines.h"
#include "GameEvents/GameEventMgr.h"
#include "AI/ScriptDevAI/scripts/world/scourge_invasion.h"

/* *********************************************************
 *                     KALIMDOR
 */

struct GhostOPlasmEvent
{
    ObjectGuid guid;
    uint32 despawnTimer;
    uint8 phaseCounter;
    std::vector<ObjectGuid> summonedMagrami;
};

/* *********************************************************
 *                     KALIMDOR
 */
struct world_map_kalimdor : public ScriptedMap
{
    world_map_kalimdor(Map* pMap) : ScriptedMap(pMap) { Initialize(); }

    uint8 m_uiMurkdeepAdds_KilledAddCount;
    std::vector<GhostOPlasmEvent> m_vGOEvents;
    uint32 m_uiOmenResetTimer;
    uint32 m_uiOmenMoonlightTimer;
    uint8 m_uiRocketsCounter;
    uint32 m_encounter[MAX_ENCOUNTER];
    bool b_isOmenSpellCreditDone;
    std::array<std::vector<ObjectGuid>, MAX_ELEMENTS> m_aElementalRiftGUIDs;
    uint32 m_uiDronesTimer;
    uint32 m_freedSpriteDarter;

    void Initialize() override
    {
        m_uiMurkdeepAdds_KilledAddCount = 0;
        m_vGOEvents.clear();
        m_uiOmenResetTimer = 0;
        m_uiOmenMoonlightTimer = 0;
        m_uiRocketsCounter = 0;
        m_freedSpriteDarter = 0;
        b_isOmenSpellCreditDone = false;
        for (auto& riftList : m_aElementalRiftGUIDs)
            riftList.clear();
        m_uiDronesTimer = 0;
        memset(&m_encounter, 0, sizeof(m_encounter));
    }

    void OnCreatureCreate(Creature* pCreature) override
    {
        switch (pCreature->GetEntry())
        {
            case NPC_HORNIZZ:
            case NPC_MURKDEEP:
            case NPC_OMEN:
            case NPC_AVALANCHION:
            case NPC_PRINCESS_TEMPESTRIA:
            case NPC_THE_WINDREAVER:
            case NPC_BARON_CHARR:
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

                            if (Creature* pTemp = pCreature->SummonCreature(NPC_GREYMIST_WARRIOR, fX, fY, fZ, aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][3], TEMPSPAWN_DEAD_DESPAWN, 0))
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

                            if (Creature* pTemp = pCreature->SummonCreature(!i ? NPC_MURKDEEP : NPC_GREYMIST_HUNTER, fX, fY, fZ, aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][3], TEMPSPAWN_DEAD_DESPAWN, 0))
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
            case NPC_OMEN:
                SetData(TYPE_OMEN, DONE);
                break;
            case NPC_THE_WINDREAVER:
                DoDespawnElementalRifts(ELEMENTAL_AIR);
                break;
            case NPC_PRINCESS_TEMPESTRIA:
                DoDespawnElementalRifts(ELEMENTAL_WATER);
                break;
            case NPC_BARON_CHARR:
                DoDespawnElementalRifts(ELEMENTAL_FIRE);
                break;
            case NPC_AVALANCHION:
                DoDespawnElementalRifts(ELEMENTAL_EARTH);
                break;
            case NPC_COLOSSUS_OF_ZORA:
                WorldObject::SpawnCreature(155122, instance);
                break;
            case NPC_COLOSSUS_OF_REGAL:
                WorldObject::SpawnCreature(155124, instance);
                break;
            case NPC_COLOSSUS_OF_ASHI:
                WorldObject::SpawnCreature(155123, instance);
                break;
            case NPC_NECROPOLIS_HEALTH:
                m_npcEntryGuidCollection.erase(pCreature->GetObjectGuid());
                break;
        }
    }

    void OnObjectCreate(GameObject* pGo) override
    {
        switch (pGo->GetEntry())
        {
            case GO_GHOST_MAGNET:
                m_vGOEvents.push_back({ pGo->GetObjectGuid(), 0, 0, {} }); // insert new event with 0 timer
                pGo->SetActiveObjectState(true);
                break;
            case GO_ROCKET_CLUSTER:
                m_goEntryGuidStore[GO_ROCKET_CLUSTER] = pGo->GetObjectGuid();
                break;
            case GO_EARTH_RIFT:
                m_aElementalRiftGUIDs[ELEMENTAL_EARTH].push_back(pGo->GetObjectGuid());
                break;
            case GO_WATER_RIFT:
                m_aElementalRiftGUIDs[ELEMENTAL_WATER].push_back(pGo->GetObjectGuid());
                break;
            case GO_FIRE_RIFT:
                m_aElementalRiftGUIDs[ELEMENTAL_FIRE].push_back(pGo->GetObjectGuid());
                break;
            case GO_AIR_RIFT:
                m_aElementalRiftGUIDs[ELEMENTAL_AIR].push_back(pGo->GetObjectGuid());
                break;
            case GO_SUMMON_CIRCLE:
                m_goEntryGuidCollection[pGo->GetEntry()].push_back(pGo->GetObjectGuid());
                break;
        }
    }

    void DoDespawnElementalRifts(uint8 index)
    {
        // Despawn all GO rifts for a given element type, erase the GUIDs for the GOs
        for (auto guid : m_aElementalRiftGUIDs[index])
        {
            if (GameObject* pRift = instance->GetGameObject(guid))
                pRift->SetLootState(GO_JUST_DEACTIVATED);
        }
        m_aElementalRiftGUIDs[index].clear();
    }

    bool GhostOPlasmEventStep(GhostOPlasmEvent& eventData)
    {
        if (eventData.despawnTimer > 180000)
        {
            for (auto guid : eventData.summonedMagrami)
            {
                if (Creature * magrami = instance->GetCreature(guid))
                {
                    if (magrami->IsAlive()) // dont despawn corpses with loot
                    {
                        magrami->CastSpell(nullptr, SPELL_SPIRIT_SPAWN_OUT, TRIGGERED_OLD_TRIGGERED);
                        magrami->ForcedDespawn(1000);
                    }
                }
            }

            if (GameObject* go = instance->GetGameObject(eventData.guid))
                go->AddObjectToRemoveList(); // TODO: Establish rules for despawning temporary GOs that were used in their lifetime (buttons for example)

            return false;
        }


        if (GameObject* go = instance->GetGameObject(eventData.guid))
        {
            if (eventData.despawnTimer / 15000 >= eventData.phaseCounter)
            {
                float x, y, z;
                go->GetPosition(x, y, z); // do some urand radius shenanigans to spawn it further and make it walk to go using doing X and Y yourself and using function in MAP to get proper Z
                uint32 random = urand(0, 35);
                float xR = x + random, yR = y + (40 - random), zR = z;
                instance->GetHeightInRange(xR, yR, zR);
                Creature* creature = go->SummonCreature(NPC_MAGRAMI_SPECTRE, xR, yR, zR, 0, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 180000); // add more timed logic here
                instance->GetReachableRandomPointOnGround(x, y, z, 10.0f); // get position to which spectre will walk
                eventData.phaseCounter++;
                eventData.summonedMagrami.push_back(creature->GetObjectGuid());
                creature->GetMotionMaster()->MovePoint(1, x, y, z);
            }
            return true;
        }
        return false;
    }

    void Update(uint32 diff) override
    {
        if (!m_vGOEvents.empty())
        {
            for (auto iter = m_vGOEvents.begin(); iter != m_vGOEvents.end();)
            {
                iter->despawnTimer += diff;
                if (!GhostOPlasmEventStep((*iter)))
                    iter = m_vGOEvents.erase(iter);
                else
                    ++iter;
            }
        }

        if (GetData(TYPE_OMEN) == DONE)
        {
            // Timer before Omen event reset (OOC)
            if (m_uiOmenResetTimer < diff)
            {
                if (Creature* pOmen = GetSingleCreatureFromStorage(NPC_OMEN))
                {
                    // Return is Omen is in fight
                    if (pOmen->IsInCombat())
                        return;
                    pOmen->ForcedDespawn();
                }
                m_encounter[TYPE_OMEN] = NOT_STARTED;
                m_uiOmenResetTimer = 0;
                m_uiRocketsCounter = 0;
                m_uiOmenMoonlightTimer = 0;
                b_isOmenSpellCreditDone = false;
            }
            else
                m_uiOmenResetTimer -= diff;

            // Spell summoning GO traps for buff and quest credit for Omen
            if (!b_isOmenSpellCreditDone)
            {
                if (m_uiOmenMoonlightTimer < diff)
                {
                    if (Creature* pOmen = GetSingleCreatureFromStorage(NPC_OMEN))
                    {
                        pOmen->CastSpell(pOmen, SPELL_OMEN_MOONLIGHT, TRIGGERED_OLD_TRIGGERED);
                        b_isOmenSpellCreditDone = true;
                    }
                    m_uiOmenMoonlightTimer = 0;
                }
                else
                    m_uiOmenMoonlightTimer -= diff;
            }
        }
        // Used for Hive Tower area trigger in Silithus
        if (m_uiDronesTimer)
        {
            if (m_uiDronesTimer <= diff)
            {
                SetData(TYPE_HIVE, NOT_STARTED);
                m_uiDronesTimer = 0;
            }
            else
                m_uiDronesTimer -= diff;
        }
    }

    void SetData(uint32 uiType, uint32 uiData) override
    {
        switch (uiType)
        {
            case TYPE_OMEN:
            {
                switch (uiData)
                {
                    case NOT_STARTED:
                        // Count another rocket cluster launched
                        m_uiRocketsCounter++;
                        if (m_uiRocketsCounter < MAX_ROCKETS)
                        {
                            // 25% chance of spawning Minions of Omen (guessed), only if not already spawned (encounter is set to FAIL in that case)
                            if (GetData(TYPE_OMEN) == NOT_STARTED && urand(0, 1) < 1)
                                SetData(TYPE_OMEN, SPECIAL); // This will notify the GO to summon Omen's minions
                        }
                        // Set the event in motion and notify the GO to summon Omen
                        else if (GetData(TYPE_OMEN) != IN_PROGRESS && GetData(TYPE_OMEN) != DONE)   // Check that Omen is not already spawned and event is reset
                            SetData(TYPE_OMEN, IN_PROGRESS);

                        return; // Don't store NOT_STARTED data unless explicitly told so: we use it to count rockets
                    case SPECIAL:
                        if (GameObject* pRocketCluster = GetSingleGameObjectFromStorage(GO_ROCKET_CLUSTER))
                        {
                            for (uint8 i = POS_IDX_MINION_OMEN_START; i <= POS_IDX_MINION_OMEN_STOP; i++)
                                pRocketCluster->SummonCreature(NPC_MINION_OMEN, aSpawnLocations[i][0], aSpawnLocations[i][1], aSpawnLocations[i][2], aSpawnLocations[i][3], TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                        }
                        break;
                    case IN_PROGRESS:
                        if (GameObject* pRocketCluster = GetSingleGameObjectFromStorage(GO_ROCKET_CLUSTER))
                        {
                            if (Creature* pOmen = pRocketCluster->SummonCreature(NPC_OMEN, aSpawnLocations[POS_IDX_OMEN_SPAWN][0], aSpawnLocations[POS_IDX_OMEN_SPAWN][1], aSpawnLocations[POS_IDX_OMEN_SPAWN][2], aSpawnLocations[POS_IDX_OMEN_SPAWN][3], TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                            {
                                // Moving him to the lake bank
                                pOmen->SetWalk(true);
                                pOmen->GetMotionMaster()->MovePoint(1, aSpawnLocations[POS_IDX_OMEN_MOVE][0], aSpawnLocations[POS_IDX_OMEN_MOVE][1], aSpawnLocations[POS_IDX_OMEN_MOVE][2]);
                                m_uiOmenResetTimer = 15 * MINUTE * IN_MILLISECONDS; // Reset after 15 minutes if not engaged or defeated
                            }
                        }
                        break;
                    case DONE:
                        m_uiOmenMoonlightTimer = 5 * IN_MILLISECONDS;            // Timer before casting the end quest spell
                        m_uiOmenResetTimer = 5 * MINUTE * IN_MILLISECONDS;       // Prevent another summoning of Omen for 5 minutes (based on spell duration)
                        break;
                }
                break;
            }
            case TYPE_GONG_TIME:
                // TODO: Handle initial first gong only
                break;
            case TYPE_HIVE:
                if (uiData == IN_PROGRESS)
                    m_uiDronesTimer = 5 * MINUTE * IN_MILLISECONDS;
                break;
            case TYPE_FREEDOM_CREATURES:
                if (uiData == IN_PROGRESS)
                    m_freedSpriteDarter = 0;
                else if (uiData == SPECIAL)
                {
                    ++m_freedSpriteDarter;
                    if (m_freedSpriteDarter >= 6)
                        uiData = DONE;
                }
                break;
        }
        m_encounter[uiType] = uiData;
    }

    uint32 GetData(uint32 uiType) const override { return m_encounter[uiType]; }
};

InstanceData* GetInstanceData_world_map_kalimdor(Map* pMap)
{
    return new world_map_kalimdor(pMap);
}

void AddSC_world_kalimdor()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "world_map_kalimdor";
    pNewScript->GetInstanceData = &GetInstanceData_world_map_kalimdor;
    pNewScript->RegisterSelf();
}
