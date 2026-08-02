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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 7622.
SDCategory: Eastern Plaguelands
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_eris_havenfire
EndContentData */



/*######
## npc_eris_havenfire
######*/

enum
{
    SAY_PHASE_HEAL                      = -1000815,
    SAY_EVENT_END                       = -1000816,
    SAY_EVENT_FAIL_1                    = -1000817,
    SAY_EVENT_FAIL_2                    = -1000818,
    SAY_PEASANT_APPEAR_1                = -1000819,
    SAY_PEASANT_APPEAR_2                = -1000820,
    SAY_PEASANT_APPEAR_3                = -1000821,

    // SPELL_DEATHS_DOOR                 = 23127,           // damage spells cast on the peasants
    // SPELL_SEETHING_PLAGUE             = 23072,
    SPELL_ENTER_THE_LIGHT_DND           = 23107,
    SPELL_BLESSING_OF_NORDRASSIL        = 23108,

    NPC_INJURED_PEASANT                 = 14484,
    NPC_PLAGUED_PEASANT                 = 14485,
    NPC_SCOURGE_ARCHER                  = 14489,
    NPC_SCOURGE_FOOTSOLDIER             = 14486,
    NPC_THE_CLEANER                     = 14503,            // can be summoned if the priest has more players in the party for help. requires further research

    QUEST_BALANCE_OF_LIGHT_AND_SHADOW   = 7622,

    BASE_PEASANTS_PER_WAVE              = 11,
    MAX_KILLED_PEASANT                  = 15,
    MAX_SAVED_PEASANT                   = 50,
    MAX_ARCHERS                         = 8,
};

static const float aArcherSpawn[8][4] =
{
    {3327.42f, -3021.11f, 170.57f, 6.01f},
    {3335.4f,  -3054.3f,  173.63f, 0.49f},
    {3351.3f,  -3079.08f, 178.67f, 1.15f},
    {3358.93f, -3076.1f,  174.87f, 1.57f},
    {3371.58f, -3069.24f, 175.20f, 1.99f},
    {3369.46f, -3023.11f, 171.83f, 3.69f},
    {3383.25f, -3057.01f, 181.53f, 2.21f},
    {3380.03f, -3062.73f, 181.90f, 2.31f},
};

static const float aPeasantSpawnLoc[3] = {3360.12f, -3047.79f, 165.26f};
static const float aPeasantMoveLoc[3] = {3335.0f, -2994.04f, 161.14f};

static const int32 aPeasantSpawnYells[3] = {SAY_PEASANT_APPEAR_1, SAY_PEASANT_APPEAR_2, SAY_PEASANT_APPEAR_3};

struct npc_eris_havenfireAI : public ScriptedAI
{
    npc_eris_havenfireAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiEventTimer;
    uint32 m_uiSadEndTimer;
    uint32 m_uiArcherCheckTargetTimer;
    uint8 m_uiPhase;
    uint8 m_uiCurrentWave;
    uint8 m_uiKillCounter;
    uint8 m_uiSaveCounter;
    uint8 m_uiTotalCounter;

    ObjectGuid m_playerGuid;
    GuidList m_lSummonedGuidList;

    void Reset() override
    {
        m_uiEventTimer              = 0;
        m_uiSadEndTimer             = 0;
        m_uiPhase                   = 0;
        m_uiCurrentWave             = 0;
        m_uiKillCounter             = 0;
        m_uiSaveCounter             = 0;
        m_uiArcherCheckTargetTimer  = 0;

        m_playerGuid.Clear();
        m_lSummonedGuidList.clear();
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_INJURED_PEASANT:
            case NPC_PLAGUED_PEASANT:
                float fX, fY, fZ;
                pSummoned->GetRandomPoint(aPeasantMoveLoc[0], aPeasantMoveLoc[1], aPeasantMoveLoc[2], 10.0f, fX, fY, fZ);
                pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                m_uiTotalCounter++;
                break;
            case NPC_SCOURGE_FOOTSOLDIER:
            case NPC_THE_CLEANER:
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pSummoned->AI()->AttackStart(pPlayer);
                break;
            case NPC_SCOURGE_ARCHER:
                break;
        }

        m_lSummonedGuidList.push_back(pSummoned->GetObjectGuid());
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId)
            return;

        if (uiPointId)
        {
            ++m_uiSaveCounter;
            pSummoned->GetMotionMaster()->Clear();

            pSummoned->RemoveAllAuras();
            pSummoned->CastSpell(pSummoned, SPELL_ENTER_THE_LIGHT_DND, TRIGGERED_NONE);
            pSummoned->ForcedDespawn(10000);

            // Event ended
            if (m_uiSaveCounter >= MAX_SAVED_PEASANT)
                DoBalanceEventEnd();
            // Phase ended
            else if (m_uiSaveCounter + m_uiKillCounter == m_uiTotalCounter)
                DoHandlePhaseEnd();
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_INJURED_PEASANT || pSummoned->GetEntry() == NPC_PLAGUED_PEASANT)
        {
            ++m_uiKillCounter;

            // If more than 15 peasants have died, then fail the quest
            if (m_uiKillCounter == MAX_KILLED_PEASANT)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pPlayer->FailQuest(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

                DoScriptText(SAY_EVENT_FAIL_1, m_creature);
                m_uiSadEndTimer = 4000;
            }
            else if (m_uiSaveCounter + m_uiKillCounter == m_uiTotalCounter)
                DoHandlePhaseEnd();
        }
    }

    void DoSummonWave(uint32 uiSummonId = 0)
    {
        float fX, fY, fZ;

        if (!uiSummonId)
        {
            for (uint8 i = 0; i < BASE_PEASANTS_PER_WAVE + m_uiCurrentWave; ++i)
            {
                uint32 uiSummonEntry = roll_chance_i(70) ? NPC_INJURED_PEASANT : NPC_PLAGUED_PEASANT;
                m_creature->GetRandomPoint(aPeasantSpawnLoc[0], aPeasantSpawnLoc[1], aPeasantSpawnLoc[2], 10.0f, fX, fY, fZ);
                if (Creature* pTemp = m_creature->SummonCreature(uiSummonEntry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0))
                {
                    // Only the first mob needs to yell
                    if (!i)
                        DoScriptText(aPeasantSpawnYells[urand(0, 2)], pTemp);
                }
            }

            ++m_uiCurrentWave;
        }
        else if (uiSummonId == NPC_SCOURGE_FOOTSOLDIER)
        {
            uint8 uiRand = urand(2, 3);
            for (uint8 i = 0; i < uiRand; ++i)
            {
                m_creature->GetRandomPoint(aPeasantSpawnLoc[0], aPeasantSpawnLoc[1], aPeasantSpawnLoc[2], 15.0f, fX, fY, fZ);
                m_creature->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
            }
        }
        else if (uiSummonId == NPC_SCOURGE_ARCHER)
        {
            for (auto i : aArcherSpawn)
                m_creature->SummonCreature(NPC_SCOURGE_ARCHER, i[0], i[1], i[2], i[3], TEMPSPAWN_DEAD_DESPAWN, 0);
        }
    }

    void DoHandlePhaseEnd()
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pPlayer->CastSpell(pPlayer, SPELL_BLESSING_OF_NORDRASSIL, TRIGGERED_OLD_TRIGGERED);

        DoScriptText(SAY_PHASE_HEAL, m_creature);

        // Send next wave
        if (m_uiCurrentWave < 5)
            DoSummonWave();
    }

    void DoStartBalanceEvent(Player* pPlayer)
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        m_playerGuid = pPlayer->GetObjectGuid();
        m_uiEventTimer = 5000;
        m_uiArcherCheckTargetTimer = 2000;
    }

    void DoBalanceEventEnd()
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pPlayer->AreaExploredOrEventHappens(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

        DoScriptText(SAY_EVENT_END, m_creature);
        m_uiArcherCheckTargetTimer = 0;
        DoDespawnSummons(true);
        EnterEvadeMode();
    }

    void DoDespawnSummons(bool bIsEventEnd = false)
    {
        for (GuidList::const_iterator itr = m_lSummonedGuidList.begin(); itr != m_lSummonedGuidList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
            {
                if (bIsEventEnd && (pTemp->GetEntry() == NPC_INJURED_PEASANT || pTemp->GetEntry() == NPC_PLAGUED_PEASANT))
                    continue;

                pTemp->ForcedDespawn();
            }
        }
    }

    // Loop over all Scourge Archer NPCs to pick a target and attack it
    void DoAttackArchersTarget()
    {
        for (GuidList::const_iterator itr = m_lSummonedGuidList.begin(); itr != m_lSummonedGuidList.end(); ++itr)
        {
            Creature* pTemp = m_creature->GetMap()->GetCreature(*itr);
            if (pTemp && pTemp->GetEntry() == NPC_SCOURGE_ARCHER)
            {
                // Archer has a victim and is in range, keep on attacking it
                if (pTemp->GetVictim() && pTemp->IsWithinDistInMap(pTemp->GetVictim(), 30.0f))
                    continue;
                // Else, find a new target in range
                // First look for an Injured Peasant in range (arbitrary choice), if none look for a Plagued Peasant
                Creature* pTarget = GetClosestCreatureWithEntry(pTemp, NPC_INJURED_PEASANT, 30.0f);
                if (!pTarget)
                    pTarget = GetClosestCreatureWithEntry(pTemp, NPC_PLAGUED_PEASANT, 30.0f);

                if (pTarget)
                    pTemp->AI()->AttackStart(pTarget);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiPhase)
                {
                    case 0:
                        DoSummonWave(NPC_SCOURGE_ARCHER);
                        m_uiEventTimer = 5000;
                        break;
                    case 1:
                        DoSummonWave();
                        m_uiEventTimer = urand(60000, 80000);
                        break;
                    default:
                        // The summoning timer of the soldiers isn't very clear
                        DoSummonWave(NPC_SCOURGE_FOOTSOLDIER);
                        m_uiEventTimer = urand(5000, 30000);
                        break;
                }
                ++m_uiPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        // Handle event end in case of fail
        if (m_uiSadEndTimer)
        {
            if (m_uiSadEndTimer <= uiDiff)
            {
                DoScriptText(SAY_EVENT_FAIL_2, m_creature);
                m_creature->ForcedDespawn(5000);
                DoDespawnSummons();
                m_uiSadEndTimer = 0;
            }
            else
                m_uiSadEndTimer -= uiDiff;
        }

        // Timer to force Scourge Archer NPCs to refresh their target
        if (m_uiArcherCheckTargetTimer)
        {
            if (m_uiArcherCheckTargetTimer <= uiDiff)
            {
                // Make Scourge Archer search for a target in range and attack it
                DoAttackArchersTarget();
                m_uiArcherCheckTargetTimer = 2000;
            }
            else
                m_uiArcherCheckTargetTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_npc_eris_havenfire(Creature* pCreature)
{
    return new npc_eris_havenfireAI(pCreature);
}

bool QuestAccept_npc_eris_havenfire(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_BALANCE_OF_LIGHT_AND_SHADOW)
    {
        if (npc_eris_havenfireAI* pErisAI = dynamic_cast<npc_eris_havenfireAI*>(pCreature->AI()))
            pErisAI->DoStartBalanceEvent(pPlayer);
    }

    return true;
}

// 23209 - Terrordale Haunting Spirit #2 (Terrordale Haunting Spirit #2)
struct TerrordaleHauntingSpirit2 : public SpellScript
{
    void OnInit(Spell* spell) const override
    {
        spell->SetEffectChance(55, EFFECT_INDEX_1);
    }
};

// 23253 - Terrordale Haunting Spirit #3 (Terrordale Haunting Spirit #3)
struct TerrordaleHauntingSpirit3 : public SpellScript
{
    void OnInit(Spell* spell) const override
    {
        spell->SetEffectChance(35, EFFECT_INDEX_1);
    }
};

/* Battle of Darrowshire - Quest 5721 (ported from VMangos)
 *
 * Event flow:
 *   1. Player uses Relic Bundle (item 15209) at Darrowshire Town Square
 *   2. Spell 18987 creates GO 177526 (Relic Bundle trigger)
 *   3. GO AI spawns invisible NPC event manager (18200)
 *   4. Event manager scans for nearby player with quest 5721
 *      -> Alliance player: defender faction = 57 (Ironforge)
 *      -> Horde player:    defender faction = 85 (Orgrimmar)
 *      Both factions are hostile to Scourge (974) for natural aggro
 *   5. Battle phases unfold over ~15 minutes:
 *      Phase 0: Darrowshire Defenders spawn (t=6s)
 *      Phase 1: Davil Lightfire spawns (t=2-3min)
 *      Phase 2: Horgus the Ravager spawns, fights Davil (t=60s after Davil)
 *      Phase 3: Horgus dies -> Davil despawns -> Captain Redpath spawns (t=8s after Horgus)
 *      Phase 4: Marduk the Black spawns, kills Redpath -> Corrupted Redpath (t=5-6min)
 *      Phase 5: Player kills Corrupted Redpath -> Joseph Redpath + Davil Crokford spawn
 *   6. Joseph Redpath gives quest credit via gossip, then reunites with Pamela
 *
 * Key design decisions:
 *   - NPC event manager (18200) instead of GO controller: CMaNGOS only routes
 *     JustSummoned/SummonedCreatureJustDied to CreatureAI, not GameObjectAI
 *   - TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN: matches VMangos behavior where despawn
 *     timer resets during combat (prevents key NPCs from despawning mid-fight)
 */

struct DarrowshireMove
{
    float x;
    float y;
    float z;
    float o;
};

enum
{
    MAX_EVENT_SUMMONS  = 200,      // Summon cap: keeps the battle population bounded
    EVENT_MAX_DURATION = 1200000   // Hard lifetime: 20 min of active runtime (nominal event ~12 min worst case)
};

static const DarrowshireMove DarrowshireEvent[] =
{
    {1500.04f, -3662.67f, 82.832f, 3.70805f},       // Attacker spawn 1
    {1506.17f, -3686.72f, 82.8769f, 5.75945f},      // Attacker spawn 2
    {1512.81f, -3724.64f, 87.12099f, 1.64164f},     // Attacker spawn 3
    {1537.6f, -3677.1f, 88.7f, 3.14884f},           // Attacker spawn Bloodletter
    {1484.68f, -3668.74f, 80.6953f, 0.236567f},     // Defender spawn 1
    {1493.53f, -3695.01f, 80.1347f, 0.264055f},     // Defender spawn 2
    {1505.28f, -3718.83f, 83.343f, 1.36911f},       // Defender spawn 3
    {1446.8f, -3694.27f, 76.5966f, 0.401503f}       // Defender spawn Davil / Redpath / Joseph
};

enum DarrowshireTriggerData
{
    // Attacker
    NPC_MARAUDING_CORPSE        = 10951,
    NPC_MARAUDING_SKELETON      = 10952,
    NPC_SERVANT_OF_HORGUS       = 10953,
    NPC_BLOODLETTER             = 10954,
    NPC_HORGUS_THE_RAVAGER      = 10946,
    NPC_MARDUK_THE_BLACK        = 10939,
    NPC_REDPATH_THE_CORRUPTED   = 10938,

    // Defender
    NPC_DARROWSHIRE_DEFENDER    = 10948,
    NPC_SILVERHAND_DISCIPLE     = 10949,
    NPC_REDPATH_MILITIA         = 10950,
    NPC_DAVIL_LIGHTFIRE         = 10944,
    NPC_CAPTAIN_REDPATH         = 10937,
    NPC_JOSEPH_REDPATH          = 10936,
    NPC_DAVIL_CROKFORD          = 10945,

    // Event manager (custom, invisible NPC)
    NPC_DARROWSHIRE_EVENT_MANAGER = 18200,

    GO_DARROWSHIRE_TRIGGER      = 177526,

    // broadcast_text entries (retail IDs - all verified present in classic DB)
    BCT_HORGUS_DIED             = 7368,
    BCT_LIGHTFIRE_DIED          = 7366,
    BCT_REDPATH_DIED            = 7369,
    BCT_SCOURGE_DEFEATED        = 7407,
    BCT_MILITIA_RANDOM_1        = 7347,
    BCT_MILITIA_RANDOM_2        = 7348,
    BCT_MILITIA_RANDOM_3        = 7349,
    BCT_MILITIA_RANDOM_4        = 7350,
    BCT_MILITIA_RANDOM_5        = 7351,
    BCT_MILITIA_RANDOM_6        = 7352,
    BCT_MILITIA_RANDOM_7        = 7353,
    BCT_MILITIA_RANDOM_8        = 7354,
    BCT_DEFENDER_YELL           = 7358,
    BCT_LIGHTFIRE_YELL          = 7343,
    BCT_DAVIL_YELL              = 7346,
    BCT_HORGUS_YELL             = 7344,
    BCT_DAVIL_DESPAWN           = 7227,
    BCT_REDPATH_YELL            = 7355,
    BCT_REDPATH_CORRUPTED       = 7357,
    BCT_MARDUK_YELL             = 7471,

    // Joseph Redpath reunion dialogue (7399-7402 filled by DB PR with
    // the authentic client texts - empty in the base DB)
    BCT_JOSEPH_1                = 7397,
    BCT_PAMELA_1                = 7399,
    BCT_PAMELA_2                = 7400,
    BCT_PAMELA_3                = 7401,
    BCT_JOSEPH_2                = 7398,
    BCT_PAMELA_4                = 7402,
    BCT_JOSEPH_3                = 7403,

    QUEST_BATTLE_DARROWSHIRE    = 5721,

    NPC_PAMELA_REDPATH          = 10926
};

/*######
## go_darrowshire_trigger
######*/

struct go_darrowshire_triggerAI : public GameObjectAI
{
    explicit go_darrowshire_triggerAI(GameObject* go) : GameObjectAI(go), m_spawned(false) {}

    bool m_spawned;

    void UpdateAI(const uint32 /*diff*/) override
    {
        if (!m_spawned)
        {
            // The Relic Bundle spell (18987) spawns this trigger a few yards
            // away from the caster; re-create it at the caster's feet so the
            // bundle appears right where it is used.
            if (Unit* caster = m_go->GetMap()->GetUnit(m_go->GetSpawnerGuid()))
            {
                if (m_go->GetDistance2d(caster->GetPositionX(), caster->GetPositionY()) > 3.0f)
                {
                    float x, y, z;
                    caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

                    m_go->ForcedDespawn();
                    Map* map = caster->GetMap();
                    GameObject* go = new GameObject;
                    uint32 lowGuid = map->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT);
                    if (!go->Create(lowGuid, lowGuid, GO_DARROWSHIRE_TRIGGER, map, x, y, z, caster->GetOrientation()))
                    {
                        delete go;
                        return;
                    }
                    go->SetRespawnTime(0);
                    go->SetSpawnerGuid(caster->GetObjectGuid());
                    map->Add(go);
                    go->AIM_Initialize();
                    // The previous trigger is still despawning in this grid and
                    // would cull the new one via the duplicate check below, so
                    // mark it started and launch the event right away.
                    if (go_darrowshire_triggerAI* triggerAI = dynamic_cast<go_darrowshire_triggerAI*>(go->AI()))
                        triggerAI->SpawnEventManager();
                    return;
                }
            }

            // Duplicate check: prevent overlapping events
            GameObjectList otherTriggers;
            GetGameObjectListWithEntryInGrid(otherTriggers, m_go, GO_DARROWSHIRE_TRIGGER, 100.0f);
            for (GameObject* other : otherTriggers)
            {
                // Keep the first trigger: a later one despawns itself when it
                // finds an existing event, the first one starts it.
                if (other->GetGUIDLow() < m_go->GetGUIDLow())
                {
                    m_go->ForcedDespawn();
                    return;
                }
            }
            SpawnEventManager();
        }
    }

    // Spawn the invisible event manager (cmangos only routes
    // JustSummoned/SummonedCreatureJustDied to Creature AIs).
    // NOT an active object: the event only advances while a player is
    // in range, so an abandoned event freezes instead of simulating
    // unattended for 30 minutes.
    void SpawnEventManager()
    {
        m_spawned = true;
        m_go->SummonCreature(NPC_DARROWSHIRE_EVENT_MANAGER,
            DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, DarrowshireEvent[7].o,
            TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 1800000, false);
    }
};

/*######
## npc_darrowshire_event_manager
######*/

struct npc_darrowshire_event_managerAI : public ScriptedAI
{
    enum
    {
        ACTION_PHASE = 1,          // battle phase machine
        ACTION_SPAWN_ATTACKERS,    // marauding corpses / skeletons
        ACTION_SPAWN_DEFENDERS,    // Darrowshire defenders
        ACTION_SPAWN_SERVANTS,     // servants of Horgus (phase 2 only)
        ACTION_SPAWN_DISCIPLES,    // Silver Hand disciples (phase 3+)
        ACTION_SPAWN_BLOODLETTERS, // bloodletters (phase 4+)
        ACTION_SPAWN_MILITIA,      // Redpath militia (phase 5)
        ACTION_PATROL,             // Davil / Bloodletter / Redpath patrol
        ACTION_FACTION_RESCAN      // defender faction re-scan while unknown
    };

    explicit npc_darrowshire_event_managerAI(Creature* creature) : ScriptedAI(creature)
    {
        m_initialized = false;
        m_cleanupDone = false;
        m_defenderFaction = 0; // set by the player scan below (57 Ironforge / 85 Orgrimmar)

        AddCustomAction(ACTION_PHASE, true, [&]() { HandlePhase(); });
        AddCustomAction(ACTION_SPAWN_ATTACKERS, true, [&]() { HandleSpawnAttackers(); });
        AddCustomAction(ACTION_SPAWN_DEFENDERS, true, [&]() { HandleSpawnDefenders(); });
        AddCustomAction(ACTION_SPAWN_SERVANTS, true, [&]() { HandleSpawnServants(); });
        AddCustomAction(ACTION_SPAWN_DISCIPLES, true, [&]() { HandleSpawnDisciples(); });
        AddCustomAction(ACTION_SPAWN_BLOODLETTERS, true, [&]() { HandleSpawnBloodletters(); });
        AddCustomAction(ACTION_SPAWN_MILITIA, true, [&]() { HandleSpawnMilitia(); });
        AddCustomAction(ACTION_PATROL, true, [&]() { HandlePatrol(); });
        AddCustomAction(ACTION_FACTION_RESCAN, true, [&]() { HandleFactionRescan(); });

        Reset();
    }

    bool m_initialized;
    bool m_cleanupDone;

    uint32 m_phaseStep;
    uint32 m_eventDuration; // hard lifetime counter (EVENT_MAX_DURATION)
    uint32 m_defenderFaction;
    GuidList m_summonedMobsList;

    ObjectGuid m_mardukGuid;
    ObjectGuid m_redpathGuid;
    ObjectGuid m_redpathCorruptedGuid;
    ObjectGuid m_davilGuid;
    ObjectGuid m_horgusGuid;

    void Reset() override
    {
        m_defenderFaction = 0;
        m_phaseStep = 0;
        m_eventDuration = 0;

        // Initial event schedule
        ResetTimer(ACTION_PHASE, 6000);
        ResetTimer(ACTION_SPAWN_ATTACKERS, 15000);
        ResetTimer(ACTION_SPAWN_DEFENDERS, 17000);
        DisableTimer(ACTION_SPAWN_SERVANTS);     // armed when phase 2 begins
        DisableTimer(ACTION_SPAWN_DISCIPLES);    // armed when phase 3 begins
        DisableTimer(ACTION_SPAWN_BLOODLETTERS); // armed when phase 4 begins
        DisableTimer(ACTION_SPAWN_MILITIA);      // armed when phase 5 begins
        DisableTimer(ACTION_PATROL);             // armed when phase 2 begins
        DisableTimer(ACTION_FACTION_RESCAN);     // armed by UpdateAI init
        m_summonedMobsList.clear();
    }

    void DespawnGuid(ObjectGuid& guid)
    {
        if (Creature* c = m_creature->GetMap()->GetCreature(guid))
            c->ForcedDespawn();
        guid.Clear();
    }

    void DespawnAll(const char* reason = "unknown")
    {
        if (m_cleanupDone)
            return;
        m_cleanupDone = true;

        // Stop every event action
        DisableTimer(ACTION_PHASE);
        DisableTimer(ACTION_SPAWN_ATTACKERS);
        DisableTimer(ACTION_SPAWN_DEFENDERS);
        DisableTimer(ACTION_SPAWN_SERVANTS);
        DisableTimer(ACTION_SPAWN_DISCIPLES);
        DisableTimer(ACTION_SPAWN_BLOODLETTERS);
        DisableTimer(ACTION_SPAWN_MILITIA);
        DisableTimer(ACTION_PATROL);
        DisableTimer(ACTION_FACTION_RESCAN);

        for (const auto& guid : m_summonedMobsList)
        {
            if (Creature* creature = m_creature->GetMap()->GetCreature(guid))
            {
                if (creature->IsAlive() &&
                    creature->GetEntry() != NPC_JOSEPH_REDPATH &&
                    creature->GetEntry() != NPC_DAVIL_CROKFORD)
                {
                    creature->ForcedDespawn(5000);
                }
            }
        }

        m_summonedMobsList.clear();
        DespawnGuid(m_mardukGuid);
        DespawnGuid(m_redpathGuid);
        DespawnGuid(m_redpathCorruptedGuid);
        DespawnGuid(m_davilGuid);
        DespawnGuid(m_horgusGuid);

        // The Relic Bundle trigger GO (177526) is summoned by the spell and
        // never despawns on its own; remove it with the event so no bundle
        // is left lying around after a win or a loss.
        GameObjectList triggerGos;
        GetGameObjectListWithEntryInGrid(triggerGos, m_creature, GO_DARROWSHIRE_TRIGGER, 200.0f);
        for (GameObject* go : triggerGos)
            go->ForcedDespawn();

        m_creature->ForcedDespawn(1000);
    }

    // NPCs the scripted sequence depends on: never culled by the summon cap.
    static bool IsScriptCriticalSummon(uint32 entry)
    {
        switch (entry)
        {
            case NPC_JOSEPH_REDPATH:
            case NPC_DAVIL_CROKFORD:
            case NPC_CAPTAIN_REDPATH:
            case NPC_MARDUK_THE_BLACK:
            case NPC_REDPATH_THE_CORRUPTED:
            case NPC_HORGUS_THE_RAVAGER:
            case NPC_DAVIL_LIGHTFIRE:
                return true;
            default:
                return false;
        }
    }

    // Find a nearby quest-holder and set the defender faction (57 Ironforge /
    // 85 Orgrimmar, both hostile to Scourge 974). Re-runnable: called at init
    // and by the faction re-scan action until a player is found.
    void ScanForQuestPlayer()
    {
        Map::PlayerList const& players = m_creature->GetMap()->GetPlayers();
        for (const auto& it : players)
        {
            Player* player = it.getSource();
            if (player && player->IsAlive() && !player->IsGameMaster() &&
                m_creature->IsWithinDist(player, 20.0f, false) &&
                player->GetQuestStatus(QUEST_BATTLE_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
            {
                m_defenderFaction = (player->GetTeam() == ALLIANCE) ? 57 : 85;
                return;
            }
        }
    }

    // Summon `amount` creatures of `entry` at each of the given DarrowshireEvent
    // spawn groups, jittered within `radius` yards. Returns the first summon
    // (used for the militia yell).
    Creature* SpawnWave(uint32 entry, uint32 firstGroup, uint32 lastGroup, uint32 amount, float radius)
    {
        Creature* first = nullptr;
        for (uint32 group = firstGroup; group <= lastGroup; ++group)
            for (uint32 i = 0; i < amount; ++i)
            {
                float x, y, z;
                m_creature->GetRandomPoint(DarrowshireEvent[group].x, DarrowshireEvent[group].y, DarrowshireEvent[group].z, radius, x, y, z);
                if (Creature* summoned = m_creature->SummonCreature(entry, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    if (!first)
                        first = summoned;
            }
        return first;
    }

    void HandleSpawnAttackers()
    {
        // Attackers: marauding corpses and skeletons, 3-6 per wave from the three attack points
        for (uint32 group = 0; group < 3; ++group)
        {
            uint32 amount = urand(1, 2);
            for (uint32 i = 0; i < amount; ++i)
            {
                float x, y, z;
                uint32 entry = urand(0, 1) ? NPC_MARAUDING_CORPSE : NPC_MARAUDING_SKELETON;
                m_creature->GetRandomPoint(DarrowshireEvent[group].x, DarrowshireEvent[group].y, DarrowshireEvent[group].z, 5.0f, x, y, z);
                m_creature->SummonCreature(entry, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
            }
        }
        ResetTimer(ACTION_SPAWN_ATTACKERS, 25000);
    }

    // Spawn one defender-type wave at each of the three defender positions.
    // With `yellTextId` set (contiguous broadcast range), the first unit of
    // the wave rallies with a random shout from that range.
    void HandleSpawnDefenderWave(uint32 entry, float radius, uint32 action, uint32 timer, uint32 yellTextId = 0)
    {
        if (Creature* first = SpawnWave(entry, 4, 6, 1, radius))
        {
            if (yellTextId)
                DoBroadcastText(yellTextId + urand(0, 7), first, nullptr, CHAT_TYPE_ZONE_YELL);
        }
        ResetTimer(action, timer);
    }

    void HandleSpawnDefenders()
    {
        // Defenders: one per wave at each of the three defender positions
        HandleSpawnDefenderWave(NPC_DARROWSHIRE_DEFENDER, 5.0f, ACTION_SPAWN_DEFENDERS, 45000);
    }

    void HandleSpawnServants()
    {
        // Servants of Horgus: reinforce the attackers while phase 2 runs
        for (uint32 group = 0; group < 3; ++group)
            SpawnWave(NPC_SERVANT_OF_HORGUS, group, group, urand(1, 2), 5.0f);
        ResetTimer(ACTION_SPAWN_SERVANTS, 35000);
    }

    void HandleSpawnDisciples()
    {
        // Silver Hand disciples: healers for the defenders (phase 3+)
        HandleSpawnDefenderWave(NPC_SILVERHAND_DISCIPLE, 5.0f, ACTION_SPAWN_DISCIPLES, 45000);
    }

    void HandleSpawnBloodletters()
    {
        // Bloodletters: strike squad of three at the bloodletter point
        SpawnWave(NPC_BLOODLETTER, 3, 3, 3, 5.0f);
        ResetTimer(ACTION_SPAWN_BLOODLETTERS, 35000);
    }

    void HandleSpawnMilitia()
    {
        // Redpath militia: final defenders; the first one rallies with a yell (phase 5)
        HandleSpawnDefenderWave(NPC_REDPATH_MILITIA, 6.0f, ACTION_SPAWN_MILITIA, 45000, BCT_MILITIA_RANDOM_1);
    }

    void HandlePatrol()
    {
        // Patrol the battle captains (Davil, Redpath, Bloodletter) around the square
        for (const auto& guid : m_summonedMobsList)
        {
            if (Creature* creature = m_creature->GetMap()->GetCreature(guid))
            {
                if (creature->GetEntry() != NPC_BLOODLETTER &&
                    creature->GetEntry() != NPC_DAVIL_LIGHTFIRE &&
                    creature->GetEntry() != NPC_CAPTAIN_REDPATH)
                    continue;

                if (creature->IsAlive() && !creature->IsInCombat() &&
                    creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                {
                    // Same cyclic route as SummonedMovementInform
                    // (5 -> 7 -> 4 -> 6 -> 5): spawn, center, rally, flank.
                    uint32 point = urand(0, 3);
                    static const uint8 patrolRoute[] = {5, 7, 4, 6};
                    uint32 rnd = patrolRoute[point];
                    creature->GetMotionMaster()->MovePoint(point, DarrowshireEvent[rnd].x, DarrowshireEvent[rnd].y, DarrowshireEvent[rnd].z, FORCED_MOVEMENT_WALK);
                }
            }
        }
        ResetTimer(ACTION_PATROL, 5000);
    }

    void HandleFactionRescan()
    {
        if (m_defenderFaction)
            return;

        ScanForQuestPlayer();
        if (!m_defenderFaction)
        {
            ResetTimer(ACTION_FACTION_RESCAN, 5000); // keep looking
            return;
        }

        // A late quest-holder was found: defenders already summoned with
        // faction 0 (neutral) must join the battle instead of staying neutral.
        for (const auto& guid : m_summonedMobsList)
        {
            Creature* creature = m_creature->GetMap()->GetCreature(guid);
            if (!creature || !creature->IsAlive())
                continue;
            switch (creature->GetEntry())
            {
                case NPC_DARROWSHIRE_DEFENDER:
                case NPC_SILVERHAND_DISCIPLE:
                case NPC_REDPATH_MILITIA:
                case NPC_DAVIL_LIGHTFIRE:
                case NPC_CAPTAIN_REDPATH:
                    creature->setFaction(m_defenderFaction);
                    break;
                default:
                    break;
            }
        }
    }

    void HandlePhase()
    {
        switch (m_phaseStep)
        {
            case 0: // First defender spawns and rallies the troops (t=6s)
            {
                if (Creature* defender = m_creature->SummonCreature(NPC_DARROWSHIRE_DEFENDER,
                    DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                {
                    DoBroadcastText(BCT_DEFENDER_YELL, defender, nullptr, CHAT_TYPE_ZONE_YELL);
                    defender->SetWalk(false);
                    defender->SetRespawnCoord(DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, DarrowshireEvent[4].o);
                    defender->GetMotionMaster()->MovePoint(0, DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, FORCED_MOVEMENT_RUN);
                    m_phaseStep = 1;
                    ResetTimer(ACTION_PHASE, urand(120000, 180000));
                }
                else
                    ResetTimer(ACTION_PHASE, 5000); // retry
                break;
            }
            case 1: // Davil Lightfire spawns (t=2-3min after Phase 0)
            {
                if (Creature* davil = m_creature->SummonCreature(NPC_DAVIL_LIGHTFIRE,
                    DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                {
                    DoBroadcastText(BCT_LIGHTFIRE_YELL, davil, nullptr, CHAT_TYPE_ZONE_YELL);
                    m_davilGuid = davil->GetObjectGuid();
                    m_phaseStep = 2;
                    ResetTimer(ACTION_SPAWN_SERVANTS, 4000); // servants: phase 2 begins
                    ResetTimer(ACTION_PATROL, 10000);        // patrol: phase 2 begins
                    ResetTimer(ACTION_PHASE, 60000);
                }
                else
                    ResetTimer(ACTION_PHASE, 5000);
                break;
            }
            case 2: // Horgus spawns near Davil and attacks him (t=60s after Davil)
            {
                Creature* davil = m_creature->GetMap()->GetCreature(m_davilGuid);
                if (!davil)
                {
                    ResetTimer(ACTION_PHASE, 5000);
                    break;
                }

                if (m_creature->GetMap()->GetCreature(m_horgusGuid))
                {
                    DoBroadcastText(BCT_DAVIL_YELL, davil, nullptr, CHAT_TYPE_ZONE_YELL);
                    DisableTimer(ACTION_PHASE);
                    break;
                }

                float x, y, z;
                m_creature->GetRandomPoint(davil->GetPositionX(), davil->GetPositionY(), davil->GetPositionZ(), 6.0f, x, y, z);
                if (Creature* horgus = m_creature->SummonCreature(NPC_HORGUS_THE_RAVAGER, x, y, z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                {
                    horgus->AI()->AttackStart(davil);
                    m_horgusGuid = horgus->GetObjectGuid();
                    DoBroadcastText(BCT_HORGUS_YELL, horgus, nullptr, CHAT_TYPE_ZONE_YELL);
                    ResetTimer(ACTION_PHASE, 3000);
                }
                else
                {
                    ResetTimer(ACTION_PHASE, 5000);
                }
                break;
            }
            case 3: // Horgus slain: Davil despawns, Captain Redpath spawns (t=8s after Horgus)
            {
                if (Creature* davil = m_creature->GetMap()->GetCreature(m_davilGuid))
                {
                    davil->ForcedDespawn(2000);
                    DoBroadcastText(BCT_DAVIL_DESPAWN, davil, nullptr, CHAT_TYPE_ZONE_YELL);
                    ResetTimer(ACTION_PHASE, 10000);
                    break;
                }

                if (Creature* redpath = m_creature->SummonCreature(NPC_CAPTAIN_REDPATH,
                    DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                {
                    DoBroadcastText(BCT_REDPATH_YELL, redpath, nullptr, CHAT_TYPE_ZONE_YELL);
                    m_redpathGuid = redpath->GetObjectGuid();
                    m_phaseStep = 4;
                    ResetTimer(ACTION_SPAWN_BLOODLETTERS, 4000); // bloodletters: phase 4 begins
                    ResetTimer(ACTION_PHASE, urand(300000, 350000));
                }
                else
                {
                    ResetTimer(ACTION_PHASE, 10000); // retry
                }
                break;
            }
            case 4: // Marduk spawns, kills Redpath -> Corrupted Redpath appears (t=5-6min)
            {
                Creature* marduk = m_creature->GetMap()->GetCreature(m_mardukGuid);
                Creature* redpath = m_creature->GetMap()->GetCreature(m_redpathGuid);

                if (marduk && redpath)
                {
                    // Marduk kills Redpath, the Corrupted Redpath rises in his place
                    m_phaseStep = 5;
                    ResetTimer(ACTION_SPAWN_MILITIA, 4000); // militia: phase 5 begins
                    DisableTimer(ACTION_PHASE);             // phase machine done
                    Unit::DealDamage(marduk, redpath, redpath->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    if (Creature* redpathCorrupted = m_creature->SummonCreature(NPC_REDPATH_THE_CORRUPTED,
                        redpath->GetPositionX(), redpath->GetPositionY(), redpath->GetPositionZ(), 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        DoBroadcastText(BCT_REDPATH_CORRUPTED, redpathCorrupted, nullptr, CHAT_TYPE_ZONE_YELL);
                        m_redpathCorruptedGuid = redpathCorrupted->GetObjectGuid();
                    }
                }
                else if (redpath)
                {
                    // Marduk missing: summon him near Redpath
                    float x, y, z;
                    m_creature->GetRandomPoint(redpath->GetPositionX(), redpath->GetPositionY(), redpath->GetPositionZ(), 10.0f, x, y, z);
                    if (Creature* mardukNew = m_creature->SummonCreature(NPC_MARDUK_THE_BLACK, x, y, z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        DoBroadcastText(BCT_MARDUK_YELL, mardukNew, nullptr, CHAT_TYPE_ZONE_YELL);
                        m_mardukGuid = mardukNew->GetObjectGuid();
                    }
                    ResetTimer(ACTION_PHASE, 5000);
                }
                else
                {
                    // Redpath missing (likely OOC-despawned after 120s without combat):
                    // re-summon him so the event can still reach its climax instead
                    // of stalling forever on this phase.
                    if (Creature* redpathNew = m_creature->SummonCreature(NPC_CAPTAIN_REDPATH,
                        DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                        m_redpathGuid = redpathNew->GetObjectGuid();
                    ResetTimer(ACTION_PHASE, 5000);
                }
                break;
            }
            default:
                break;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (!summoned)
            return;

        // Summon cap: the list is pruned on despawn, so it counts live
        // summons only. Script-critical NPCs are exempt so the scripted
        // sequence can never be culled.
        if (m_summonedMobsList.size() >= MAX_EVENT_SUMMONS &&
            !IsScriptCriticalSummon(summoned->GetEntry()))
        {
            summoned->ForcedDespawn();
            return;
        }

        m_summonedMobsList.push_back(summoned->GetObjectGuid());

        switch (summoned->GetEntry())
        {
            case NPC_DARROWSHIRE_DEFENDER:
            case NPC_SILVERHAND_DISCIPLE:
            case NPC_REDPATH_MILITIA:
                summoned->setFaction(m_defenderFaction);
                summoned->GetMotionMaster()->MoveRandomAroundPoint(summoned->GetPositionX(), summoned->GetPositionY(), summoned->GetPositionZ(), 60.0f);
                break;
            case NPC_MARAUDING_CORPSE:
            case NPC_MARAUDING_SKELETON:
            case NPC_SERVANT_OF_HORGUS:
                summoned->GetMotionMaster()->MoveRandomAroundPoint(summoned->GetPositionX(), summoned->GetPositionY(), summoned->GetPositionZ(), 60.0f);
                break;
            case NPC_BLOODLETTER:
                summoned->SetWalk(true);
                summoned->SetRespawnCoord(DarrowshireEvent[5].x, DarrowshireEvent[5].y, DarrowshireEvent[5].z, DarrowshireEvent[5].o);
                summoned->GetMotionMaster()->MovePoint(0, DarrowshireEvent[5].x, DarrowshireEvent[5].y, DarrowshireEvent[5].z, FORCED_MOVEMENT_WALK);
                break;
            case NPC_DAVIL_LIGHTFIRE:
            case NPC_CAPTAIN_REDPATH:
                summoned->setFaction(m_defenderFaction);
                summoned->SetWalk(false);
                summoned->SetRespawnCoord(DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, DarrowshireEvent[4].o);
                summoned->GetMotionMaster()->MovePoint(2, DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, FORCED_MOVEMENT_RUN);
                break;
            case NPC_MARDUK_THE_BLACK:
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_IMMUNE_TO_NPC);
                summoned->ForcedDespawn(12000);
                break;
            default:
                break;
        }
    }

    void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE || !summoned)
            return;

        switch (summoned->GetEntry())
        {
            case NPC_DARROWSHIRE_DEFENDER:
            {
                if (pointId == 0)
                    summoned->GetMotionMaster()->MoveRandomAroundPoint(summoned->GetPositionX(), summoned->GetPositionY(), summoned->GetPositionZ(), 60.0f);
                break;
            }
            case NPC_DAVIL_LIGHTFIRE:
            case NPC_CAPTAIN_REDPATH:
            case NPC_BLOODLETTER:
            {
                switch (pointId)
                {
                    case 0: // Move to battle center (position 7)
                        summoned->SetWalk(true);
                        summoned->GetMotionMaster()->MovePoint(1, DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, FORCED_MOVEMENT_WALK);
                        break;
                    case 1: // Move to defender rally point (position 4)
                        summoned->SetWalk(true);
                        summoned->GetMotionMaster()->MovePoint(2, DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, FORCED_MOVEMENT_WALK);
                        break;
                    case 2: // Move to defender flank (position 6)
                        summoned->SetWalk(true);
                        summoned->GetMotionMaster()->MovePoint(3, DarrowshireEvent[6].x, DarrowshireEvent[6].y, DarrowshireEvent[6].z, FORCED_MOVEMENT_WALK);
                        break;
                    case 3: // Return to Bloodletter spawn (position 5)
                        summoned->SetWalk(true);
                        summoned->GetMotionMaster()->MovePoint(0, DarrowshireEvent[5].x, DarrowshireEvent[5].y, DarrowshireEvent[5].z, FORCED_MOVEMENT_WALK);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (!summoned)
            return;

        // Prune: the summon cap counts live summons only
        m_summonedMobsList.remove(summoned->GetObjectGuid());

        switch (summoned->GetEntry())
        {
            case NPC_HORGUS_THE_RAVAGER:
            {
                if (Creature* defender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                    DoBroadcastText(BCT_HORGUS_DIED, defender, nullptr, CHAT_TYPE_ZONE_YELL);
                m_phaseStep = 3;
                DisableTimer(ACTION_SPAWN_SERVANTS);      // servants: phase 2 only
                ResetTimer(ACTION_SPAWN_DISCIPLES, 4000); // disciples: armed when phase 3 begins
                ResetTimer(ACTION_PHASE, 8000);
                break;
            }
            case NPC_DAVIL_LIGHTFIRE:
            {
                if (m_phaseStep < 3)
                {
                    if (Creature* defender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                        DoBroadcastText(BCT_LIGHTFIRE_DIED, defender, nullptr, CHAT_TYPE_ZONE_YELL);
                    DespawnAll("davil died early");
                }
                break;
            }
            case NPC_CAPTAIN_REDPATH:
            {
                if (m_phaseStep < 5)
                {
                    if (Creature* defender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                        DoBroadcastText(BCT_REDPATH_DIED, defender, nullptr, CHAT_TYPE_ZONE_YELL);
                    DespawnAll("redpath died early");
                }
                break;
            }
            case NPC_REDPATH_THE_CORRUPTED:
            {
                if (Creature* defender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                    DoBroadcastText(BCT_SCOURGE_DEFEATED, defender, nullptr, CHAT_TYPE_ZONE_YELL);
                // Active object: the reunion scene must keep playing even if the
                // player is out of the creature's active range (e.g. watching from afar)
                m_creature->SummonCreature(NPC_JOSEPH_REDPATH,
                    DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000, true);
                m_creature->SummonCreature(NPC_DAVIL_CROKFORD,
                    1465.43f, -3678.48f, 78.0816f, 0.0402176f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                                DespawnAll("event won - corrupted slain");
                break;
            }
            default:
                break;
        }
    }

    void SummonedCreatureDespawn(Creature* summoned) override
    {
        if (!summoned)
            return;

        // Temp-spawn expiry also goes through here - keep the cap counting live summons only
        m_summonedMobsList.remove(summoned->GetObjectGuid());
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_initialized)
        {
            m_initialized = true;

            // Duplicate check: keep the first manager, a later one despawns itself
            CreatureList otherManagers;
            GetCreatureListWithEntryInGrid(otherManagers, m_creature, NPC_DARROWSHIRE_EVENT_MANAGER, 100.0f);
            for (Creature* other : otherManagers)
            {
                if (other->GetGUIDLow() < m_creature->GetGUIDLow())
                {
                    m_creature->ForcedDespawn();
                    return;
                }
            }

            // Player scan: defender faction from the nearest quest-holder (57 Ironforge /
            // 85 Orgrimmar, both hostile to Scourge). The re-scan action keeps looking
            // until a quest-holder is found, so a late-arriving player still gives the
            // battle a faction instead of running it with neutral (faction 0) defenders.
            ScanForQuestPlayer();
            ResetTimer(ACTION_FACTION_RESCAN, 5000);
        }

        UpdateTimers(diff, m_creature->IsInCombat());

        // Hard event lifetime: clean everything up after EVENT_MAX_DURATION of
        // active runtime, even if no creature ever dies (prevents an abandoned
        // event from running indefinitely; also covers the temp-spawn expiry
        // path that has no AI hook to trigger cleanup).
        m_eventDuration += diff;
        if (m_eventDuration >= EVENT_MAX_DURATION)
        {
            DespawnAll("hard lifetime expired");
            return;
        }
    }
};

/*######
## npc_joseph_redpath
######*/

struct npc_joseph_redpathAI : public ScriptedAI
{
    enum { ACTION_REUNION_STEP = 1 };

    explicit npc_joseph_redpathAI(Creature* creature) : ScriptedAI(creature)
    {
        m_eventStarted = false;
        m_eventStep = 0;
        Reset();
        AddCustomAction(ACTION_REUNION_STEP, true, [&]() { HandleReunionStep(); });
    }

    bool m_eventStarted;
    uint32 m_eventStep;

    void BeginEvent()
    {
        if (!m_eventStarted)
        {
            ResetTimer(ACTION_REUNION_STEP, 30000);
            m_eventStep = 0;
            m_eventStarted = true;
        }
    }

    void MovementInform(uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case 0: // Joseph walks toward meeting point (30s after event start)
                m_creature->GetMotionMaster()->MovePoint(1, 1434.22f, -3668.756f, 76.671f, FORCED_MOVEMENT_WALK);
                break;
            case 1: // Joseph reaches meeting point, calls for Pamela
                m_creature->GetMotionMaster()->MovePoint(2, 1438.526f, -3632.733f, 78.268f, FORCED_MOVEMENT_WALK);
                DoBroadcastText(BCT_JOSEPH_1, m_creature, nullptr, CHAT_TYPE_ZONE_YELL); // "Pamela? Are you there, honey?"
                ResetTimer(ACTION_REUNION_STEP, 3000);
                break;
            case 2: // Joseph spots Pamela, runs toward her
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_2, pamela, nullptr, CHAT_TYPE_ZONE_YELL); // "Daddy! You're back!"
                    m_creature->SetWalk(false);
                    float x = 0.0f, y = 0.0f, z = 0.0f;
                    pamela->GetContactPoint(m_creature, x, y, z, 1.0f);
                    m_creature->GetMotionMaster()->MovePoint(3, x, y, z, FORCED_MOVEMENT_RUN); // he runs to her
                    DisableTimer(ACTION_REUNION_STEP);
                }
                else
                {
                    ResetTimer(ACTION_REUNION_STEP, 1000);
                }
                break;
            case 3: // Joseph and Pamela face each other (reunion complete)
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 20.0f, true))
                {
                    m_creature->SetFacingToObject(pamela);
                    pamela->SetFacingToObject(m_creature);
                }
                ResetTimer(ACTION_REUNION_STEP, 2000);
                break;
        }
    }

    void HandleReunionStep()
    {
                switch (m_eventStep)
        {
            case 0: // Joseph walks toward meeting point (30s wait ends)
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->GetMotionMaster()->MovePoint(0, 1431.501f, -3684.229f, 75.726f, FORCED_MOVEMENT_WALK);
                ++m_eventStep;
                DisableTimer(ACTION_REUNION_STEP);
                break;
            case 1: // Pamela appears and runs toward Joseph
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_1, pamela, nullptr, CHAT_TYPE_ZONE_YELL); // "Daddy!"
                    pamela->GetMotionMaster()->MovePoint(0, 1450.733f, -3599.974f, 85.621f, FORCED_MOVEMENT_WALK);
                    ++m_eventStep;
                    DisableTimer(ACTION_REUNION_STEP);
                }
                else
                {
                    // Do not lose Pamela's line: retry until she is found.
                    ResetTimer(ACTION_REUNION_STEP, 1000);
                }
                break;
            case 2: // Pamela sees Joseph
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_3, pamela, nullptr, CHAT_TYPE_ZONE_YELL); // "Let's go play! No, tell me a story, Daddy!..."
                }
                ++m_eventStep;
                ResetTimer(ACTION_REUNION_STEP, 5000);
                break;
            case 3: // Joseph explains his return
                DoBroadcastText(BCT_JOSEPH_2, m_creature, nullptr, CHAT_TYPE_ZONE_YELL); // "Hahah!"
                ++m_eventStep;
                ResetTimer(ACTION_REUNION_STEP, 3000);
                break;
            case 4: // Pamela's farewell
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_4, pamela, nullptr, CHAT_TYPE_ZONE_YELL); // "I missed you so much, Daddy!"
                }
                ++m_eventStep;
                ResetTimer(ACTION_REUNION_STEP, 4000);
                break;
            case 5: // Joseph's final farewell; Pamela walks back home - she is a
                // permanent world spawn AND quest 5721's turn-in NPC, despawning
                // her would block the turn-in for ~6 minutes for everyone.
                DoBroadcastText(BCT_JOSEPH_3, m_creature, nullptr, CHAT_TYPE_ZONE_YELL); // "I missed you too, honey. And I'm finally home..."
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->ForcedDespawn(6000);
                if (Creature* pamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    pamela->GetMotionMaster()->MovePoint(0, 1456.32f, -3596.44f, 86.963f, FORCED_MOVEMENT_WALK);
                }
                DisableTimer(ACTION_REUNION_STEP);
                break;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        UpdateTimers(diff, m_creature->IsInCombat());
    }
};

bool GossipHello_npc_joseph_redpath(Player* player, Creature* creature)
{
    // Always show Joseph's spirit dialogue (npc_text 4778, the vanilla quest
    // text): the gossip_menu 3861 -> 4778 row is gated by 'quest 5721 taken',
    // so it would fall back to the client default greeting once the quest
    // completes. The quest credit is handled below.
    player->SEND_GOSSIP_MENU(4778, creature->GetObjectGuid());
    if (player->GetQuestStatus(QUEST_BATTLE_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
    {
        player->KilledMonsterCredit(NPC_JOSEPH_REDPATH, creature->GetObjectGuid());
        creature->HandleEmote(EMOTE_ONESHOT_BEG);
        if (npc_joseph_redpathAI* josephAI = dynamic_cast<npc_joseph_redpathAI*>(creature->AI()))
            josephAI->BeginEvent();
    }
    return true;
}
void AddSC_eastern_plaguelands()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_eris_havenfire";
    pNewScript->GetAI = &GetAI_npc_eris_havenfire;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_eris_havenfire;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_darrowshire_trigger";
    pNewScript->GetGameObjectAI = &GetNewAIInstance<go_darrowshire_triggerAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_darrowshire_event_manager";
    pNewScript->GetAI = &GetNewAIInstance<npc_darrowshire_event_managerAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_joseph_redpath";
    pNewScript->GetAI = &GetNewAIInstance<npc_joseph_redpathAI>;
    pNewScript->pGossipHello = &GossipHello_npc_joseph_redpath;
    pNewScript->RegisterSelf();

    RegisterSpellScript<TerrordaleHauntingSpirit2>("spell_terrordale_haunting_spirit2");
    RegisterSpellScript<TerrordaleHauntingSpirit3>("spell_terrordale_haunting_spirit3");
}
