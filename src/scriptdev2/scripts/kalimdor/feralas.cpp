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
SDComment: Quest support: 2767, 2845.
SDCategory: Feralas
EndScriptData */

/* ContentData
npc_oox22fe
npc_shay_leafrunner
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

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

    void WaypointReached(uint32 i) override
    {
        switch (i)
        {
                // First Ambush(3 Yetis)
            case 11:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_YETI, -4841.01f, 1593.91f, 73.42f, 3.98f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4837.61f, 1568.58f, 78.21f, 3.13f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4841.89f, 1569.95f, 76.53f, 0.68f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
                // Second Ambush(3 Gorillas)
            case 21:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_GORILLA, -4595.81f, 2005.99f, 53.08f, 3.74f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4597.53f, 2008.31f, 52.70f, 3.78f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4599.37f, 2010.59f, 52.77f, 3.84f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
                // Third Ambush(4 Gnolls)
            case 30:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_WOODPAW_REAVER, -4425.14f, 2075.87f, 47.77f, 3.77f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_BRUTE , -4426.68f, 2077.98f, 47.57f, 3.77f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_MYSTIC, -4428.33f, 2080.24f, 47.43f, 3.87f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_ALPHA , -4430.04f, 2075.54f, 46.83f, 3.81f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
            case 37:
                DoScriptText(SAY_OOX_END, m_creature);
                // Award quest credit
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_RESCUE_OOX22FE, m_creature);
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
    }
};

CreatureAI* GetAI_npc_oox22fe(Creature* pCreature)
{
    return new npc_oox22feAI(pCreature);
}

bool QuestAccept_npc_oox22fe(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RESCUE_OOX22FE)
    {
        DoScriptText(SAY_OOX_START, pCreature);
        // change that the npc is not lying dead on the ground
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (pPlayer->GetTeam() == HORDE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

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
            pPlayer->GroupEventHappens(QUEST_ID_WANDERING_SHAY, m_creature);
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

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        // start following
        if (eventType == AI_EVENT_START_EVENT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
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
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
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

CreatureAI* GetAI_npc_shay_leafrunner(Creature* pCreature)
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

void AddSC_feralas()
{
    Script* pNewScript;

    pNewScript = new Script;
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
}
