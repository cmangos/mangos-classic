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
 *   1. Player uses Relic Bundle (item 15209) at Darrowshire Town Square (GO 177528)
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

enum { MAX_MOB_SLOTS = 7 };

static DarrowshireMove DarrowshireEvent[] =
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
    NPC_DARROWSHIRE_BETRAYER    = 10947,

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

    // broadcast_text entries
    BCT_HORGUS_DIED             = 2000001,
    BCT_LIGHTFIRE_DIED          = 2000002,
    BCT_REDPATH_DIED            = 2000003,
    BCT_SCOURGE_DEFEATED        = 2000004,
    BCT_MILITIA_RANDOM_1        = 2000005,
    BCT_MILITIA_RANDOM_2        = 2000006,
    BCT_MILITIA_RANDOM_3        = 2000007,
    BCT_MILITIA_RANDOM_4        = 2000008,
    BCT_MILITIA_RANDOM_5        = 2000009,
    BCT_MILITIA_RANDOM_6        = 2000010,
    BCT_MILITIA_RANDOM_7        = 2000011,
    BCT_MILITIA_RANDOM_8        = 2000012,
    BCT_DEFENDER_YELL           = 2000013,
    BCT_LIGHTFIRE_YELL          = 2000014,
    BCT_DAVIL_YELL              = 2000015,
    BCT_HORGUS_YELL             = 2000016,
    BCT_DAVIL_DESPAWN           = 2000017,
    BCT_REDPATH_YELL            = 2000018,
    BCT_REDPATH_CORRUPTED       = 2000019,
    BCT_MARDUK_YELL             = 2000020,

    // Joseph Redpath dialogue
    BCT_JOSEPH_1                = 2000021,
    BCT_PAMELA_1                = 2000022,
    BCT_PAMELA_2                = 2000023,
    BCT_PAMELA_3                = 2000024,
    BCT_JOSEPH_2                = 2000025,
    BCT_PAMELA_4                = 2000026,
    BCT_JOSEPH_3                = 2000027,
    BCT_JOSEPH_DESPAWN          = 2000028,

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

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        // Duplicate check: prevent overlapping events
        if (!m_spawned)
        {
            GameObjectList otherTriggers;
            GetGameObjectListWithEntryInGrid(otherTriggers, m_go, GO_DARROWSHIRE_TRIGGER, 100.0f);
            if (otherTriggers.size() > 1)
            {
                m_go->ForcedDespawn();
                return;
            }
            m_spawned = true;

            // Spawn NPC event manager (cmangos only routes
            // JustSummoned/SummonedCreatureJustDied to Creature AIs)
            m_go->SummonCreature(NPC_DARROWSHIRE_EVENT_MANAGER,
                DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, DarrowshireEvent[7].o,
                TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 1800000, true);
        }
    }
};

/*######
## npc_darrowshire_event_manager
######*/

struct npc_darrowshire_event_managerAI : public ScriptedAI
{
    explicit npc_darrowshire_event_managerAI(Creature* creature) : ScriptedAI(creature)
    {
        m_initialized = false;
        m_cleanupDone = false;
        m_defenderFaction = 0; // Set by player scan in UpdateAI init (57 Ironforge / 85 Orgrimmar)
        Reset();
    }

    bool m_initialized;
    bool m_cleanupDone;

    uint32 m_phaseStep;
    uint32 m_phaseTimer;
    uint32 m_mobTimer[MAX_MOB_SLOTS];
    uint32 m_defenderFaction;
    GuidList m_summonedMobsList;

    ObjectGuid m_mardukGuid;
    ObjectGuid m_redpathGuid;
    ObjectGuid m_redpathCorruptedGuid;
    ObjectGuid m_davilGuid;
    ObjectGuid m_horgusGuid;

    void Reset() override
    {
        // Faction set by player scan; 57 = Ironforge, 85 = Orgrimmar, both hostile to Scourge
        m_defenderFaction = 0;
        m_phaseStep = 0;
        m_phaseTimer = 6000;

        m_mobTimer[0] = 15000;
        m_mobTimer[1] = 17000;
        m_mobTimer[2] = m_mobTimer[3] = m_mobTimer[4] = m_mobTimer[5] = m_mobTimer[6] = 0;
        m_summonedMobsList.clear();
    }

    void DespawnGuid(ObjectGuid& guid)
    {
        if (Creature* c = m_creature->GetMap()->GetCreature(guid))
            c->ForcedDespawn();
        guid.Clear();
    }

    void DespawnAll()
    {
        if (m_cleanupDone)
            return;
        m_cleanupDone = true;
        for (uint32& timer : m_mobTimer)
            timer = 0;
        m_phaseTimer = 0;

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

        CreatureList betrayerList;
        GetCreatureListWithEntryInGrid(betrayerList, m_creature, NPC_DARROWSHIRE_BETRAYER, 150.0f);
        for (Creature* betrayer : betrayerList)
            betrayer->ForcedDespawn();

        m_creature->ForcedDespawn(1000);
    }
    void JustSummoned(Creature* summoned) override
    {
        if (!summoned)
            return;

        m_summonedMobsList.push_back(summoned->GetObjectGuid());
        // Set faction for defenders, movement for attackers, following VMangos JustSummoned pattern

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

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (!pSummoned)
            return;

        switch (pSummoned->GetEntry())
        {
            case NPC_HORGUS_THE_RAVAGER:
            {
                if (Creature* pDefender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                    DoBroadcastText(BCT_HORGUS_DIED, pDefender);
                m_phaseStep = 3;
                m_phaseTimer = 8000;
                break;
            }
            case NPC_DAVIL_LIGHTFIRE:
            {
                if (m_phaseStep < 3)
                {
                    if (Creature* pDefender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                        DoBroadcastText(BCT_LIGHTFIRE_DIED, pDefender);
                    DespawnAll();
                }
                break;
            }
            case NPC_CAPTAIN_REDPATH:
            {
                if (m_phaseStep < 5)
                {
                    if (Creature* pDefender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                        DoBroadcastText(BCT_REDPATH_DIED, pDefender);
                    DespawnAll();
                }
                break;
            }
            case NPC_REDPATH_THE_CORRUPTED:
            {
                if (Creature* pDefender = GetClosestCreatureWithEntry(m_creature, NPC_DARROWSHIRE_DEFENDER, 100.0f, true))
                    DoBroadcastText(BCT_SCOURGE_DEFEATED, pDefender);
                m_creature->SummonCreature(NPC_JOSEPH_REDPATH,
                    DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
                m_creature->SummonCreature(NPC_DAVIL_CROKFORD,
                    1465.43f, -3678.48f, 78.0816f, 0.0402176f,
                    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                DespawnAll();
                break;
            }
            default:
                break;
        }
    }
    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_initialized)
        {
            m_initialized = true;
            // Player scan: determine defender faction based on quest holder team (same as VMangos Reset)

            // Duplicate check: prevent overlapping events
            CreatureList otherManagers;
            GetCreatureListWithEntryInGrid(otherManagers, m_creature, NPC_DARROWSHIRE_EVENT_MANAGER, 100.0f);
            if (otherManagers.size() > 1)
            {
                m_creature->ForcedDespawn();
                return;
            }

            // Scan nearby players for quest 5721 to determine faction (same as VMangos Reset)
            Map::PlayerList const& players = m_creature->GetMap()->GetPlayers();
            for (const auto& it : players)
            {
                Player* pPlayer = it.getSource();
                if (pPlayer && pPlayer->IsAlive() && !pPlayer->IsGameMaster() &&
                    m_creature->IsWithinDist(pPlayer, 20.0f, false) &&
                    pPlayer->GetQuestStatus(QUEST_BATTLE_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
                {
                    m_defenderFaction = (pPlayer->GetTeam() == ALLIANCE) ? 57 : 85;
                    // 57 = Ironforge, 85 = Orgrimmar — both hostile to Scourge (974) for natural aggro
                    break;
                }
            }
        }

        for (int mobIndex = 0; mobIndex < MAX_MOB_SLOTS; ++mobIndex)
        {
            if (m_mobTimer[mobIndex] && m_mobTimer[mobIndex] <= uiDiff)
            {
                switch (mobIndex)
                {
                    case 0: // NPC_MARAUDING_CORPSE / NPC_MARAUDING_SKELETON
                    {
                        for (int spawnGroupIndex = 0; spawnGroupIndex < 3; ++spawnGroupIndex)
                        {
                            int amount = urand(1, 2);
                            for (int spawnIndex = 0; spawnIndex < amount; ++spawnIndex)
                            {
                                float x, y, z;
                                uint32 entry = urand(0, 1) ? NPC_MARAUDING_CORPSE : NPC_MARAUDING_SKELETON;
                                m_creature->GetRandomPoint(DarrowshireEvent[spawnGroupIndex].x, DarrowshireEvent[spawnGroupIndex].y, DarrowshireEvent[spawnGroupIndex].z, 5.0f, x, y, z);
                                m_creature->SummonCreature(entry, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                            }
                        }
                        m_mobTimer[mobIndex] = 25000;
                        break;
                    }
                    case 1: // NPC_DARROWSHIRE_DEFENDER
                    {
                        for (int spawnGroupIndex = 4; spawnGroupIndex < MAX_MOB_SLOTS; ++spawnGroupIndex)
                        {
                            float x, y, z;
                            m_creature->GetRandomPoint(DarrowshireEvent[spawnGroupIndex].x, DarrowshireEvent[spawnGroupIndex].y, DarrowshireEvent[spawnGroupIndex].z, 5.0f, x, y, z);
                            m_creature->SummonCreature(NPC_DARROWSHIRE_DEFENDER, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                        }
                        m_mobTimer[mobIndex] = 45000;
                        break;
                    }
                    case 2: // NPC_SERVANT_OF_HORGUS
                    {
                        if (m_phaseStep != 2)
                        {
                            m_mobTimer[mobIndex] = 0;
                            break;
                        }

                        float x, y, z;
                        for (int spawnGroupIndex = 0; spawnGroupIndex < 3; ++spawnGroupIndex)
                        {
                            int amount = urand(1, 2);
                            for (int spawnIndex = 0; spawnIndex < amount; ++spawnIndex)
                            {
                                m_creature->GetRandomPoint(DarrowshireEvent[spawnGroupIndex].x, DarrowshireEvent[spawnGroupIndex].y, DarrowshireEvent[spawnGroupIndex].z, 5.0f, x, y, z);
                                m_creature->SummonCreature(NPC_SERVANT_OF_HORGUS, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                            }
                        }
                        m_mobTimer[mobIndex] = 35000;
                        break;
                    }
                    case 3: // NPC_SILVERHAND_DISCIPLE
                    {
                        if (m_phaseStep <= 2)
                        {
                            m_mobTimer[mobIndex] = 0;
                            break;
                        }

                        for (int spawnGroupIndex = 4; spawnGroupIndex < MAX_MOB_SLOTS; ++spawnGroupIndex)
                        {
                            float x, y, z;
                            m_creature->GetRandomPoint(DarrowshireEvent[spawnGroupIndex].x, DarrowshireEvent[spawnGroupIndex].y, DarrowshireEvent[spawnGroupIndex].z, 5.0f, x, y, z);
                            m_creature->SummonCreature(NPC_SILVERHAND_DISCIPLE, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                        }
                        m_mobTimer[mobIndex] = 45000;
                        break;
                    }
                    case 4: // NPC_BLOODLETTER
                    {
                        for (int spawnGroupIndex = 0; spawnGroupIndex < 3; ++spawnGroupIndex)
                        {
                            float x, y, z;
                            m_creature->GetRandomPoint(DarrowshireEvent[3].x, DarrowshireEvent[3].y, DarrowshireEvent[3].z, 5.0f, x, y, z);
                            m_creature->SummonCreature(NPC_BLOODLETTER, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000);
                        }
                        m_mobTimer[mobIndex] = 35000;
                        break;
                    }
                    case 5: // NPC_REDPATH_MILITIA
                    {
                        if (m_phaseStep <= 4)
                        {
                            m_mobTimer[mobIndex] = 0;
                            break;
                        }

                        bool yelled = false;
                        for (int spawnGroupIndex = 4; spawnGroupIndex < MAX_MOB_SLOTS; ++spawnGroupIndex)
                        {
                            float x, y, z;
                            m_creature->GetRandomPoint(DarrowshireEvent[spawnGroupIndex].x, DarrowshireEvent[spawnGroupIndex].y, DarrowshireEvent[spawnGroupIndex].z, 6.0f, x, y, z);
                            if (Creature* pMilitia = m_creature->SummonCreature(NPC_REDPATH_MILITIA, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                            {
                                if (!yelled)
                                {
                                    DoBroadcastText(BCT_MILITIA_RANDOM_1 + (urand(0, 7)), pMilitia);
                                    yelled = true;
                                }
                            }
                        }
                        m_mobTimer[mobIndex] = 45000;
                        break;
                    }
                    case 6: // Patrol management: Davil, Bloodletter, Redpath
                    {
                        for (const auto& guid : m_summonedMobsList)
                        {
                            if (Creature* pCrea = m_creature->GetMap()->GetCreature(guid))
                            {
                                if (pCrea->GetEntry() != NPC_BLOODLETTER &&
                                    pCrea->GetEntry() != NPC_DAVIL_LIGHTFIRE &&
                                    pCrea->GetEntry() != NPC_CAPTAIN_REDPATH)
                                    continue;

                                if (pCrea->IsAlive() && !pCrea->IsInCombat() &&
                                    pCrea->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                                {
                                    int point = urand(0, 3);
                                    static const int patrolMap[] = {5, 7, 4, 6};
                                    int rnd = patrolMap[point];
                                    pCrea->GetMotionMaster()->MovePoint(point, DarrowshireEvent[rnd].x, DarrowshireEvent[rnd].y, DarrowshireEvent[rnd].z, FORCED_MOVEMENT_WALK);
                                }
                            }
                        }

                        m_mobTimer[mobIndex] = 5000;
                        break;
                    }
                    default:
                        break;
                }
            }
            else if (m_mobTimer[mobIndex])
                m_mobTimer[mobIndex] -= uiDiff;
        }

        if (m_phaseTimer && m_phaseTimer <= uiDiff)
        {
            switch (m_phaseStep)
            {
                case 0: // First defender spawns and rallies the troops (t=6s)
                {
                    if (Creature* pDefender = m_creature->SummonCreature(NPC_DARROWSHIRE_DEFENDER,
                        DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        DoBroadcastText(BCT_DEFENDER_YELL, pDefender);
                        pDefender->SetWalk(false);
                        pDefender->SetRespawnCoord(DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, DarrowshireEvent[4].o);
                        pDefender->GetMotionMaster()->MovePoint(0, DarrowshireEvent[4].x, DarrowshireEvent[4].y, DarrowshireEvent[4].z, FORCED_MOVEMENT_RUN);
                        m_phaseTimer = urand(120000, 180000);
                        m_phaseStep = 1;
                    }
                    break;
                }
                case 1: // Davil Lightfire spawns (t=2-3min after Phase 0)
                {
                    if (Creature* pDavil = m_creature->SummonCreature(NPC_DAVIL_LIGHTFIRE,
                        DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        DoBroadcastText(BCT_LIGHTFIRE_YELL, pDavil);
                        m_davilGuid = pDavil->GetObjectGuid();
                        m_phaseTimer = 60000;
                        m_mobTimer[2] = 4000;
                        m_mobTimer[3] = 6000;
                        m_mobTimer[6] = 10000;
                        m_phaseStep = 2;
                    }
                    break;
                }
                case 2: // Horgus spawns near Davil and attacks him (t=60s after Davil)
                {
                    Creature* pDavil = m_creature->GetMap()->GetCreature(m_davilGuid);
                    if (!pDavil)
                        break;

                    if (m_creature->GetMap()->GetCreature(m_horgusGuid))
                    {
                        DoBroadcastText(BCT_DAVIL_YELL, pDavil);
                        m_phaseTimer = 0;
                        break;
                    }

                    float x, y, z;
                    m_creature->GetRandomPoint(pDavil->GetPositionX(), pDavil->GetPositionY(), pDavil->GetPositionZ(), 6.0f, x, y, z);
                    if (Creature* pHorgus = m_creature->SummonCreature(NPC_HORGUS_THE_RAVAGER, x, y, z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        pHorgus->AI()->AttackStart(pDavil);
                        m_horgusGuid = pHorgus->GetObjectGuid();
                        DoBroadcastText(BCT_HORGUS_YELL, pHorgus);
                        m_phaseTimer = 3000;
                    }
                    break;
                }
                case 3: // Horgus slain: Davil despawns, Captain Redpath spawns (t=8s after Horgus)
                {
                    if (Creature* pDavil = m_creature->GetMap()->GetCreature(m_davilGuid))
                    {
                        pDavil->ForcedDespawn(2000);
                        DoBroadcastText(BCT_DAVIL_DESPAWN, pDavil);
                        m_phaseTimer = 10000;
                        break;
                    }

                    if (Creature* pRedpath = m_creature->SummonCreature(NPC_CAPTAIN_REDPATH,
                        DarrowshireEvent[7].x, DarrowshireEvent[7].y, DarrowshireEvent[7].z, 0.0f,
                        TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                    {
                        DoBroadcastText(BCT_REDPATH_YELL, pRedpath);
                        m_redpathGuid = pRedpath->GetObjectGuid();
                        m_phaseTimer = urand(300000, 350000);
                        m_phaseStep = 4;
                        m_mobTimer[4] = 4000;
                        m_mobTimer[5] = 6000;
                    }
                    break;
                }
                case 4: // Marduk spawns, kills Redpath -> Corrupted Redpath appears (t=5-6min)
                {
                    Creature* pMarduk = m_creature->GetMap()->GetCreature(m_mardukGuid);
                    if (pMarduk)
                    {
                        if (Creature* pRedpath = m_creature->GetMap()->GetCreature(m_redpathGuid))
                        {
                            m_phaseStep = 5;
                            m_phaseTimer = 0;
                            Unit::DealDamage(pMarduk, pRedpath, pRedpath->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                            if (Creature* pRedpathCorrupted = m_creature->SummonCreature(NPC_REDPATH_THE_CORRUPTED,
                                pRedpath->GetPositionX(), pRedpath->GetPositionY(), pRedpath->GetPositionZ(), 0.0f,
                                TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                            {
                                DoBroadcastText(BCT_REDPATH_CORRUPTED, pRedpathCorrupted);
                                m_redpathCorruptedGuid = pRedpathCorrupted->GetObjectGuid();
                            }
                        }
                        break;
                    }

                    if (Creature* pRedpath = m_creature->GetMap()->GetCreature(m_redpathGuid))
                    {
                        float x, y, z;
                        m_creature->GetRandomPoint(pRedpath->GetPositionX(), pRedpath->GetPositionY(), pRedpath->GetPositionZ(), 10.0f, x, y, z);
                        if (Creature* pMardukNew = m_creature->SummonCreature(NPC_MARDUK_THE_BLACK, x, y, z, 0.0f,
                            TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                        {
                            DoBroadcastText(BCT_MARDUK_YELL, pMardukNew);
                            m_mardukGuid = pMardukNew->GetObjectGuid();
                            m_phaseTimer = 5000;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        else if (m_phaseTimer)
            m_phaseTimer -= uiDiff;
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
        m_bEventStarted = false;
        m_uiEventStep = 0;
        Reset();
        AddCustomAction(ACTION_REUNION_STEP, true, [&]() { HandleReunionStep(); });
    }

    bool m_bEventStarted;
    uint32 m_uiEventStep;

    void BeginEvent()
    {
        if (!m_bEventStarted)
        {
            ResetTimer(ACTION_REUNION_STEP, 30000);
            m_uiEventStep = 0;
            m_bEventStarted = true;
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (uiPointId)
        {
            case 0: // Joseph walks toward meeting point (30s after event start)
                m_creature->GetMotionMaster()->MovePoint(1, 1434.22f, -3668.756f, 76.671f, FORCED_MOVEMENT_WALK);
                break;
            case 1: // Joseph reaches meeting point, calls for Pamela
                m_creature->GetMotionMaster()->MovePoint(2, 1438.526f, -3632.733f, 78.268f, FORCED_MOVEMENT_WALK);
                DoBroadcastText(BCT_JOSEPH_1, m_creature);
                ResetTimer(ACTION_REUNION_STEP, 3000);
                break;
            case 2: // Joseph spots Pamela, runs toward her
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_2, pPamela);
                    m_creature->SetWalk(false);
                    float x, y, z = 0;
                    pPamela->GetContactPoint(m_creature, x, y, z, 1.0f);
                    m_creature->GetMotionMaster()->MovePoint(3, x, y, z, FORCED_MOVEMENT_WALK);
                    DisableTimer(ACTION_REUNION_STEP);
                }
                else
                {
                    ResetTimer(ACTION_REUNION_STEP, 1);
                }
                break;
            case 3: // Joseph and Pamela face each other (reunion complete)
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 20.0f, true))
                {
                    m_creature->SetFacingToObject(pPamela);
                    pPamela->SetFacingToObject(m_creature);
                }
                ResetTimer(ACTION_REUNION_STEP, 2000);
                break;
        }
    }

    void HandleReunionStep()
    {
        switch (m_uiEventStep)
        {
            case 0: // Joseph walks toward meeting point (30s wait ends)
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->GetMotionMaster()->MovePoint(0, 1431.501f, -3684.229f, 75.726f, FORCED_MOVEMENT_WALK);
                ++m_uiEventStep;
                DisableTimer(ACTION_REUNION_STEP);
                break;
            case 1: // Pamela appears and runs toward Joseph
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_1, pPamela);
                    pPamela->GetMotionMaster()->MovePoint(0, 1450.733f, -3599.974f, 85.621f, FORCED_MOVEMENT_WALK);
                }
                ++m_uiEventStep;
                DisableTimer(ACTION_REUNION_STEP);
                break;
            case 2: // Pamela sees Joseph
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_3, pPamela);
                }
                ++m_uiEventStep;
                ResetTimer(ACTION_REUNION_STEP, 5000);
                break;
            case 3: // Joseph explains his return
                DoBroadcastText(BCT_JOSEPH_2, m_creature);
                ++m_uiEventStep;
                ResetTimer(ACTION_REUNION_STEP, 3000);
                break;
            case 4: // Pamela's farewell
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    DoBroadcastText(BCT_PAMELA_4, pPamela);
                }
                ++m_uiEventStep;
                ResetTimer(ACTION_REUNION_STEP, 4000);
                break;
            case 5: // Joseph's final farewell, both despawn
                DoBroadcastText(BCT_JOSEPH_3, m_creature);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->ForcedDespawn(6000);
                if (Creature* pPamela = GetClosestCreatureWithEntry(m_creature, NPC_PAMELA_REDPATH, 150.0f, true))
                {
                    pPamela->ForcedDespawn(4000);
                }
                DisableTimer(ACTION_REUNION_STEP);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        UpdateTimers(uiDiff, m_creature->IsInCombat());
    }
};

bool GossipHello_npc_joseph_redpath(Player* player, Creature* creature)
{
    player->SEND_GOSSIP_MENU(3861, creature->GetObjectGuid());
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
