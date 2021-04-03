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
SDName: Feralas
SD%Complete: 100
SDComment: Quest support: 2767, 2845, 2969.
SDCategory: Feralas
EndScriptData */


/* ContentData
npc_oox22fe
npc_shay_leafrunner
go_cage_door
npc_captured_sprite_darter
npc_kindal_moonweaver
EndContentData */

#include "Grids/GridNotifiers.h"
#include "Grids/CellImpl.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/base/follower_ai.h"
#include "AI/ScriptDevAI/scripts/kalimdor/world_kalimdor.h"
#include "Spells/Scripts/SpellScript.h"

/*######
## npc_oox22fe
######*/

enum
{
    SAY_OOX_START           = -1000287,
    SAY_OOX_AGGRO1          = -1000288,
    SAY_OOX_AGGRO2          = -1000289,
    SAY_OOX_AMBUSH          = -1000290,
    SAY_OOX_END             = -1000292,

    NPC_YETI                = 7848,
    NPC_GORILLA             = 5260,
    NPC_WOODPAW_REAVER      = 5255,
    NPC_WOODPAW_BRUTE       = 5253,
    NPC_WOODPAW_ALPHA       = 5258,
    NPC_WOODPAW_MYSTIC      = 5254,

    QUEST_RESCUE_OOX22FE    = 2767
};

struct npc_oox22feAI : public npc_escortAI
{
    npc_oox22feAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    GuidList m_lSummonsList;

    void WaypointReached(uint32 i) override
    {
        switch (i)
        {
            // First Ambush(3 Yetis)
            case 12:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_YETI, -4841.01f, 1593.91f, 73.42f, 3.98f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4837.61f, 1568.58f, 78.21f, 3.13f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4841.89f, 1569.95f, 76.53f, 0.68f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                break;
            // Second Ambush(3 Gorillas)
            case 22:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_GORILLA, -4595.81f, 2005.99f, 53.08f, 3.74f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4597.53f, 2008.31f, 52.70f, 3.78f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4599.37f, 2010.59f, 52.77f, 3.84f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                break;
            // Third Ambush(4 Gnolls)
            case 31:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_WOODPAW_REAVER, -4425.14f, 2075.87f, 47.77f, 3.77f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_BRUTE, -4426.68f, 2077.98f, 47.57f, 3.77f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_MYSTIC, -4428.33f, 2080.24f, 47.43f, 3.87f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_ALPHA, -4430.04f, 2075.54f, 46.83f, 3.81f, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 10000);
                break;
            case 38:
                DoScriptText(SAY_OOX_END, m_creature);
                // Award quest credit
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_RESCUE_OOX22FE, m_creature);
                break;
        }
    }

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    void Aggro(Unit* /*who*/) override
    {
        // For an small probability the npc says something when he get aggro
        switch (urand(0, 9))
        {
            case 0: DoScriptText(SAY_OOX_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_OOX_AGGRO2, m_creature); break;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summoned->AI()->AttackStart(m_creature);
        m_lSummonsList.push_back(summoned->GetObjectGuid());
    }

    void JustDied(Unit* pKiller) override
    {
        for (GuidList::const_iterator itr = m_lSummonsList.begin(); itr != m_lSummonsList.end(); ++itr)
        {
            if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                pSummoned->ForcedDespawn();
        }

        npc_escortAI::JustDied(pKiller);
    }
};

UnitAI* GetAI_npc_oox22fe(Creature* pCreature)
{
    return new npc_oox22feAI(pCreature);
}

bool QuestAccept_npc_oox22fe(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RESCUE_OOX22FE)
    {
        DoScriptText(SAY_OOX_START, pCreature);
        pCreature->SetActiveObjectState(true);
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);
        pCreature->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_ACTIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

        if (npc_oox22feAI* pEscortAI = dynamic_cast<npc_oox22feAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## npc_shay_leafrunner
######*/

enum
{
    SAY_ESCORT_START                    = -1001106,
    SAY_WANDER_1                        = -1001107,
    SAY_WANDER_2                        = -1001108,
    SAY_WANDER_3                        = -1001109,
    SAY_WANDER_4                        = -1001110,
    SAY_WANDER_DONE_1                   = -1001111,
    SAY_WANDER_DONE_2                   = -1001112,
    SAY_WANDER_DONE_3                   = -1001113,
    EMOTE_WANDER                        = -1001114,
    SAY_EVENT_COMPLETE_1                = -1001115,
    SAY_EVENT_COMPLETE_2                = -1001116,

    SPELL_SHAYS_BELL                    = 11402,
    NPC_ROCKBITER                       = 7765,
    QUEST_ID_WANDERING_SHAY             = 2845,
};

struct npc_shay_leafrunnerAI : public FollowerAI
{
    npc_shay_leafrunnerAI(Creature* pCreature) : FollowerAI(pCreature)
    {
        m_uiWanderTimer = 0;
        Reset();
    }

    uint32 m_uiWanderTimer;
    bool m_bIsRecalled;
    bool m_bIsComplete;

    void Reset() override
    {
        m_bIsRecalled = false;
        m_bIsComplete = false;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!m_bIsComplete && pWho->GetEntry() == NPC_ROCKBITER && m_creature->IsWithinDistInMap(pWho, 20.0f))
        {
            Player* pPlayer = GetLeaderForFollower();
            if (!pPlayer)
                return;

            DoScriptText(SAY_EVENT_COMPLETE_1, m_creature);
            DoScriptText(SAY_EVENT_COMPLETE_2, pWho);

            // complete quest
            pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ID_WANDERING_SHAY, m_creature);
            SetFollowComplete(true);
            m_creature->ForcedDespawn(30000);
            m_bIsComplete = true;
            m_uiWanderTimer = 0;

            // move to Rockbiter
            float fX, fY, fZ;
            pWho->GetContactPoint(m_creature, fX, fY, fZ, INTERACTION_DISTANCE);
            m_creature->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
        }
        else if (m_bIsRecalled && pWho->GetTypeId() == TYPEID_PLAYER && pWho->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
        {
            m_uiWanderTimer = 60000;
            m_bIsRecalled = false;

            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_WANDER_DONE_1, m_creature); break;
                case 1: DoScriptText(SAY_WANDER_DONE_2, m_creature); break;
                case 2: DoScriptText(SAY_WANDER_DONE_3, m_creature); break;
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        // start following
        if (eventType == AI_EVENT_START_EVENT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            StartFollow((Player*)pInvoker, 0, GetQuestTemplateStore(uiMiscValue));
            m_uiWanderTimer = 30000;
        }
        else if (eventType == AI_EVENT_CUSTOM_A)
        {
            // resume following
            m_bIsRecalled = true;
            SetFollowPaused(false);
        }
    }

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
        {
            if (m_uiWanderTimer)
            {
                if (m_uiWanderTimer <= uiDiff)
                {
                    // set follow paused and wander in a random point
                    SetFollowPaused(true);
                    DoScriptText(EMOTE_WANDER, m_creature);
                    m_uiWanderTimer = 0;

                    switch (urand(0, 3))
                    {
                        case 0: DoScriptText(SAY_WANDER_1, m_creature); break;
                        case 1: DoScriptText(SAY_WANDER_2, m_creature); break;
                        case 2: DoScriptText(SAY_WANDER_3, m_creature); break;
                        case 3: DoScriptText(SAY_WANDER_4, m_creature); break;
                    }

                    float fX, fY, fZ;
                    m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, frand(25.0f, 40.0f), frand(0, 2 * M_PI_F));
                    m_creature->GetMotionMaster()->MoveRandomAroundPoint(fX, fY, fZ, 20.0f);
                }
                else
                    m_uiWanderTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_shay_leafrunner(Creature* pCreature)
{
    return new npc_shay_leafrunnerAI(pCreature);
}

bool QuestAccept_npc_shay_leafrunner(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_WANDERING_SHAY)
    {
        DoScriptText(SAY_ESCORT_START, pCreature);
        pCreature->AI()->SendAIEvent(AI_EVENT_START_EVENT, pPlayer, pCreature, pQuest->GetQuestId());
    }
    return true;
}

bool EffectDummyCreature_npc_shay_leafrunner(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId == SPELL_SHAYS_BELL && uiEffIndex == EFFECT_INDEX_0)
    {
        if (pCaster->GetTypeId() != TYPEID_PLAYER)
            return true;

        pCreatureTarget->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, pCaster, pCreatureTarget);
        return true;
    }

    return false;
}

/*######
## Quest Freedom for all creatures
######*/

enum {
    SAY_KINDAL_QUEST_START      = -1010023,
    SAY_KINDAL_INITIAL_AGGRO    = -1010024,
    SAY_KINDAL_AGGRO            = -1010025,
    SAY_KINDAL_QUEST_COMPLETE   = -1010026,

    NPC_CAPTURED_SPRITE_DARTER  = 7997,
    NPC_GRIMTOTEM_RAIDER        = 7725,
    NPC_KINDAL_MOONWEAVER       = 7956,

    QUEST_FREEDOM_ALL_CREATURES = 2969,

    SPELL_MANA_BURN             = 11981,
    SPELL_SHOOT                 = 14443,
    SPELL_MULTI_SHOT            = 6660
};

static const float raiderSpawnPos[4] = { -4515.35f, 811.36f, 62.99f, 3.60244f };

/*######
## npc_captured_sprite_darter
######*/

struct npc_captured_sprite_darterAI : public npc_escortAI
{
    npc_captured_sprite_darterAI(Creature* creature) : npc_escortAI(creature)
    {
        m_map = (ScriptedMap*)creature->GetInstanceData();
        Reset();
    }

    ScriptedMap* m_map;
    uint32 m_manaBurnTimer;

    void Reset() override
    {
        m_manaBurnTimer = urand(3, 6) * IN_MILLISECONDS;
        // Randomly pick a pre-set flight path amongst 4
        SetEscortWaypoints(urand(0, 3));
    }

    void WaypointReached(uint32 pointId) override
    {
        // When we reach the end of the path, despawn and inform map script that we are free
        if (pointId == 8)
        {
            m_map->SetData(TYPE_FREEDOM_CREATURES, SPECIAL);
            m_creature->ForcedDespawn();
        }
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_manaBurnTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MANA_BURN, SELECT_FLAG_POWER_MANA))
            {
                if(DoCastSpellIfCan(target, SPELL_MANA_BURN) == CAST_OK)
                    m_manaBurnTimer = urand(7, 10) * IN_MILLISECONDS;
            }
        }
        else
            m_manaBurnTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_captured_sprite_darter(Creature* pCreature)
{
    return new npc_captured_sprite_darterAI(pCreature);
}

/*######
## go_cage_door
######*/

bool GOUse_go_cage_door(Player* player, GameObject* go)
{
    ScriptedInstance* mapScript = (ScriptedInstance*)go->GetInstanceData();

    if (!mapScript)
        return false;

    // Omen encounter is set to NOT_STARTED every time the GO cluster is used
    // This increases an internal counter that handles the event in the map script
    mapScript->SetData(TYPE_FREEDOM_CREATURES, IN_PROGRESS);

    // Make all Captured Sprite Darters flee
    CreatureList spriteDarts;
    MaNGOS::AllCreaturesOfEntryInRangeCheck checkerForWaypoint(go, NPC_CAPTURED_SPRITE_DARTER, 20.0f);
    MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(spriteDarts, checkerForWaypoint);
    Cell::VisitGridObjects(go, searcher, 20.0f);

    for (auto& spriteDart : spriteDarts)
    {
        spriteDart->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
        if (auto* escortAI = dynamic_cast<npc_captured_sprite_darterAI*>(spriteDart->AI()))
            escortAI->Start(true, player, nullptr);
    }
    // Spawn a Grimtotem Raider
    if (Creature* grimtotemRaider = player->SummonCreature(NPC_GRIMTOTEM_RAIDER, raiderSpawnPos[0], raiderSpawnPos[1], raiderSpawnPos[2], raiderSpawnPos[3],TEMPSPAWN_DEAD_DESPAWN, 10 * MINUTE * IN_MILLISECONDS))
    {
        grimtotemRaider->AI()->AttackStart(player);
        if (Creature* kindal = GetClosestCreatureWithEntry(player, NPC_KINDAL_MOONWEAVER, 20.0f))
        {
            DoScriptText(SAY_KINDAL_INITIAL_AGGRO, kindal, grimtotemRaider);
            kindal->AI()->AttackStart(grimtotemRaider);
        }
    }
    return true;
}

/*######
## npc_kindal_moonweaver
######*/

struct npc_kindal_moonweaverAI : public FollowerAI
{
    npc_kindal_moonweaverAI(Creature* creature) : FollowerAI(creature)
    {
        m_map = (ScriptedMap*)creature->GetInstanceData();
        Reset();
    }

    ScriptedMap* m_map;
    uint32 m_checkCompletionTimer;
    uint32 m_shootTimer;
    uint32 m_multiShotTimer;
    uint32 m_despawnTimer;
    bool m_hasFleed;

    void Reset() override
    {
        m_checkCompletionTimer = 1 * IN_MILLISECONDS;
        m_despawnTimer = 0;
        m_shootTimer = urand(2, 4) * IN_MILLISECONDS;
        m_multiShotTimer = urand(8, 11) * IN_MILLISECONDS;
        m_hasFleed = false;
    }

    void Aggro(Unit* target) override
    {
        if (!urand(0, 3))
            DoScriptText(SAY_KINDAL_AGGRO, m_creature, target);
    }

    void UpdateFollowerAI(const uint32 diff) override
    {
        if (HasFollowState(STATE_FOLLOW_INPROGRESS))
        {
            // Periodically ask map script if quest is completed or not
            if (m_checkCompletionTimer < diff)
            {
                if (m_map->GetData(TYPE_FREEDOM_CREATURES) == DONE)
                {
                    m_map->SetData(TYPE_FREEDOM_CREATURES, NOT_STARTED);
                    if (Player* player = GetLeaderForFollower())
                    {
                        if (player->GetQuestStatus(QUEST_FREEDOM_ALL_CREATURES) == QUEST_STATUS_INCOMPLETE)
                            player->RewardPlayerAndGroupAtEventExplored(QUEST_FREEDOM_ALL_CREATURES, m_creature);
                        DoScriptText(SAY_KINDAL_QUEST_COMPLETE, m_creature, player);
                    }
                    m_despawnTimer = 5 * IN_MILLISECONDS;
                    SetFollowComplete(true);
                }
            }
            else
                m_checkCompletionTimer -= diff;
        }

        if (m_despawnTimer)
        {
            if (m_despawnTimer < diff)
            {
                m_creature->SetImmuneToNPC(true);
                m_creature->ForcedDespawn();
                m_creature->Respawn();
                m_despawnTimer = 0;
            }
            else
                m_despawnTimer -= diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Flee at 15% HP
        if (m_creature->GetHealthPercent() <= 15.0f && !m_hasFleed)
        {
            if (m_creature->AI()->DoFlee())
                m_hasFleed = true;
        }

        // Shoot
        if (m_shootTimer < diff)
        {
            // If we are in shooting range: shoot!
            if (m_creature->IsInRange(m_creature->GetVictim(), 5.0f, 30.0f, true, true))
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHOOT) == CAST_OK)
                {
                    m_shootTimer = urand(2, 4) * IN_MILLISECONDS;
                    return;
                }
            }
        }
        else
            m_shootTimer -= diff;

        // Multishot
        if (m_multiShotTimer < diff)
        {
            // If we are in shooting range: shoot!
            if (m_creature->IsInRange(m_creature->GetVictim(), 5.0f, 30.0f, true, true))
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MULTI_SHOT) == CAST_OK)
                {
                    m_shootTimer = urand(8, 11) * IN_MILLISECONDS;
                    return;
                }
            }
        }
        else
            m_multiShotTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_kindal_moonweaver(Creature* pCreature)
{
    return new npc_kindal_moonweaverAI(pCreature);
}

bool QuestAccept_npc_kindal_moonweaver(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_FREEDOM_ALL_CREATURES)
    {
        if (npc_kindal_moonweaverAI* kindal = dynamic_cast<npc_kindal_moonweaverAI*>(creature->AI()))
            kindal->StartFollow(player, FACTION_ESCORT_A_NEUTRAL_ACTIVE, quest);
        DoScriptText(SAY_KINDAL_QUEST_START, creature);
        creature->SetStandState(UNIT_STAND_STATE_STAND);
        creature->SetImmuneToNPC(false);
    }

    return true;
}

struct SpecificTargetScript : public SpellScript
{
    virtual std::set<uint32> const& GetRequiredTargets() const = 0;

    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        Unit* target = spell->m_targets.getUnitTarget();
        if (!target)
            return SPELL_FAILED_BAD_TARGETS;
        auto& targets = GetRequiredTargets();
        if (targets.find(target->GetObjectGuid().GetEntry()) == targets.end())
            return SPELL_FAILED_BAD_TARGETS;
        return SPELL_CAST_OK;
    }
};

struct CaptureWildkin : public SpecificTargetScript // 11886
{
    std::set<uint32> m_targets = { 2927, 2928, 7808 };

    std::set<uint32> const& GetRequiredTargets() const override { return m_targets; }
};

struct CaptureHippogryph : public SpecificTargetScript // 11887
{
    std::set<uint32> m_targets = { 5300, 5304, 5305, 5306 };

    std::set<uint32> const& GetRequiredTargets() const override { return m_targets; }
};

struct CaptureFaerieDragon : public SpecificTargetScript // 11888
{
    std::set<uint32> m_targets = { 5276, 5278 };

    std::set<uint32> const& GetRequiredTargets() const override { return m_targets; }
};

struct CaptureTreant : public SpecificTargetScript // 11885
{
    std::set<uint32> m_targets = { 7584 };

    std::set<uint32> const& GetRequiredTargets() const override { return m_targets; }
};

struct CaptureMountainGiant : public SpecificTargetScript // 11889
{
    std::set<uint32> m_targets = { 5357, 5358 };

    std::set<uint32> const& GetRequiredTargets() const override { return m_targets; }
};

void AddSC_feralas()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_oox22fe";
    pNewScript->GetAI = &GetAI_npc_oox22fe;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_oox22fe;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_shay_leafrunner";
    pNewScript->GetAI = &GetAI_npc_shay_leafrunner;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_shay_leafrunner;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_npc_shay_leafrunner;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captured_sprite_darter";
    pNewScript->GetAI = &GetAI_npc_captured_sprite_darter;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kindal_moonweaver";
    pNewScript->GetAI = &GetAI_npc_kindal_moonweaver;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kindal_moonweaver;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_cage_door";
    pNewScript->pGOUse = &GOUse_go_cage_door;
    pNewScript->RegisterSelf();

    RegisterSpellScript<CaptureWildkin>("spell_capture_wildkin");
    RegisterSpellScript<CaptureHippogryph>("spell_capture_hippogryph");
    RegisterSpellScript<CaptureFaerieDragon>("spell_capture_faerie_dragon");
    RegisterSpellScript<CaptureTreant>("spell_capture_treant");
    RegisterSpellScript<CaptureMountainGiant>("spell_capture_mountain_giant");
}
