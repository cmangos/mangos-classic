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
SDName: Darkshore
SD%Complete: 100
SDComment: Quest support: 731, 945, 994, 995, 2078, 2118, 5321
SDCategory: Darkshore
EndScriptData */

/* ContentData
npc_kerlonian
npc_prospector_remtravel
npc_threshwackonator
npc_volcor
npc_therylune
npc_rabid_bear
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*####
# npc_kerlonian
####*/

enum
{
    SAY_KER_START               = -1000434,

    EMOTE_KER_SLEEP_1           = -1000435,
    EMOTE_KER_SLEEP_2           = -1000436,
    EMOTE_KER_SLEEP_3           = -1000437,

    SAY_KER_SLEEP_1             = -1000438,
    SAY_KER_SLEEP_2             = -1000439,
    SAY_KER_SLEEP_3             = -1000440,
    SAY_KER_SLEEP_4             = -1000441,

    EMOTE_KER_AWAKEN            = -1000445,

    SAY_KER_ALERT_1             = -1000442,
    SAY_KER_ALERT_2             = -1000443,

    SAY_KER_END                 = -1000444,

    SPELL_BEAR_FORM             = 18309,
    SPELL_SLEEP_VISUAL          = 25148,
    SPELL_AWAKEN                = 17536,

    QUEST_SLEEPER_AWAKENED      = 5321,

    NPC_LILADRIS                = 11219                     // attackers entries unknown
};

// TODO: make concept similar as "ringo" -escort. Find a way to run the scripted attacks, _if_ player are choosing road.
struct npc_kerlonianAI : public FollowerAI
{
    npc_kerlonianAI(Creature* pCreature) : FollowerAI(pCreature) { Reset(); }

    uint32 m_uiFallAsleepTimer;

    void Reset() override
    {
        m_uiFallAsleepTimer = urand(10000, 45000);

        if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
            DoCastSpellIfCan(m_creature, SPELL_BEAR_FORM);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!m_creature->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == NPC_LILADRIS)
        {
            if (m_creature->IsWithinDistInMap(pWho, INTERACTION_DISTANCE * 5))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_SLEEPER_AWAKENED) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_SLEEPER_AWAKENED, m_creature);

                    DoScriptText(SAY_KER_END, m_creature);
                }

                SetFollowComplete();
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_CUSTOM_A && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            if (HasFollowState(STATE_FOLLOW_INPROGRESS | STATE_FOLLOW_PAUSED))
                ClearSleeping();
        }
    }

    // Function to set npc to sleep mode
    void SetSleeping()
    {
        SetFollowPaused(true);

        switch (urand(0, 2))
        {
            case 0: DoScriptText(EMOTE_KER_SLEEP_1, m_creature); break;
            case 1: DoScriptText(EMOTE_KER_SLEEP_2, m_creature); break;
            case 2: DoScriptText(EMOTE_KER_SLEEP_3, m_creature); break;
        }

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_KER_SLEEP_1, m_creature); break;
            case 1: DoScriptText(SAY_KER_SLEEP_2, m_creature); break;
            case 2: DoScriptText(SAY_KER_SLEEP_3, m_creature); break;
            case 3: DoScriptText(SAY_KER_SLEEP_4, m_creature); break;
        }

        if (DoCastSpellIfCan(m_creature, SPELL_SLEEP_VISUAL) == CAST_OK)
            m_creature->SetStandState(UNIT_STAND_STATE_SLEEP);
    }

    // Function to clear sleep mode
    void ClearSleeping()
    {
        m_creature->RemoveAurasDueToSpell(SPELL_SLEEP_VISUAL);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        DoScriptText(EMOTE_KER_AWAKEN, m_creature);

        SetFollowPaused(false);
    }

    void UpdateFollowerAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
                return;

            if (!HasFollowState(STATE_FOLLOW_PAUSED))
            {
                if (m_uiFallAsleepTimer < uiDiff)
                {
                    SetSleeping();
                    m_uiFallAsleepTimer = urand(25000, 90000);
                }
                else
                    m_uiFallAsleepTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kerlonian(Creature* pCreature)
{
    return new npc_kerlonianAI(pCreature);
}

bool QuestAccept_npc_kerlonian(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SLEEPER_AWAKENED)
    {
        if (npc_kerlonianAI* pKerlonianAI = dynamic_cast<npc_kerlonianAI*>(pCreature->AI()))
        {
            pCreature->RemoveAurasDueToSpell(SPELL_BEAR_FORM);
            pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            DoScriptText(SAY_KER_START, pCreature, pPlayer);
            pKerlonianAI->StartFollow(pPlayer, FACTION_ESCORT_N_FRIEND_PASSIVE, pQuest);
        }
    }

    return true;
}

bool EffectDummyCreature_awaken_kerlonian(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId == SPELL_AWAKEN && uiEffIndex == EFFECT_INDEX_0)
    {
        pCreatureTarget->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, pCaster, pCreatureTarget);
        return true;
    }

    return false;
}

/*####
# npc_prospector_remtravel
####*/

enum
{
    SAY_REM_START               = -1000327,
    SAY_REM_AGGRO               = -1000339,
    SAY_REM_RAMP1_1             = -1000328,
    SAY_REM_RAMP1_2             = -1000329,
    SAY_REM_BOOK                = -1000330,
    SAY_REM_TENT1_1             = -1000331,
    SAY_REM_TENT1_2             = -1000332,
    SAY_REM_MOSS                = -1000333,
    EMOTE_REM_MOSS              = -1000334,
    SAY_REM_MOSS_PROGRESS       = -1000335,
    SAY_REM_PROGRESS            = -1000336,
    SAY_REM_REMEMBER            = -1000337,
    EMOTE_REM_END               = -1000338,

    QUEST_ABSENT_MINDED_PT2     = 731,
    NPC_GRAVEL_SCOUT            = 2158,
    NPC_GRAVEL_BONE             = 2159,
    NPC_GRAVEL_GEO              = 2160
};

struct npc_prospector_remtravelAI : public npc_escortAI
{
    npc_prospector_remtravelAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void WaypointReached(uint32 uiPointId) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
            case 0:
                DoScriptText(SAY_REM_START, m_creature, pPlayer);
                break;
            case 5:
                DoScriptText(SAY_REM_RAMP1_1, m_creature, pPlayer);
                break;
            case 6:
                DoSpawnCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                DoSpawnCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                break;
            case 9:
                DoScriptText(SAY_REM_RAMP1_2, m_creature, pPlayer);
                break;
            case 14:
                // depend quest rewarded?
                DoScriptText(SAY_REM_BOOK, m_creature, pPlayer);
                break;
            case 15:
                DoScriptText(SAY_REM_TENT1_1, m_creature, pPlayer);
                break;
            case 16:
                DoSpawnCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                DoSpawnCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                break;
            case 17:
                DoScriptText(SAY_REM_TENT1_2, m_creature, pPlayer);
                break;
            case 26:
                DoScriptText(SAY_REM_MOSS, m_creature, pPlayer);
                break;
            case 27:
                DoScriptText(EMOTE_REM_MOSS, m_creature, pPlayer);
                break;
            case 28:
                DoScriptText(SAY_REM_MOSS_PROGRESS, m_creature, pPlayer);
                break;
            case 29:
                DoSpawnCreature(NPC_GRAVEL_SCOUT, -15.0f, 3.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                DoSpawnCreature(NPC_GRAVEL_BONE, -15.0f, 5.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                DoSpawnCreature(NPC_GRAVEL_GEO, -15.0f, 7.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 30000);
                break;
            case 31:
                DoScriptText(SAY_REM_PROGRESS, m_creature, pPlayer);
                break;
            case 41:
                DoScriptText(SAY_REM_REMEMBER, m_creature, pPlayer);
                break;
            case 42:
                DoScriptText(EMOTE_REM_END, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_ABSENT_MINDED_PT2, m_creature);
                break;
        }
    }

    void Reset() override { }

    void Aggro(Unit* pWho) override
    {
        if (urand(0, 1))
            DoScriptText(SAY_REM_AGGRO, m_creature, pWho);
    }

    void JustSummoned(Creature* /*pSummoned*/) override
    {
        // unsure if it should be any
        // pSummoned->AI()->AttackStart(m_creature);
    }
};

CreatureAI* GetAI_npc_prospector_remtravel(Creature* pCreature)
{
    return new npc_prospector_remtravelAI(pCreature);
}

bool QuestAccept_npc_prospector_remtravel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ABSENT_MINDED_PT2)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_prospector_remtravelAI* pEscortAI = dynamic_cast<npc_prospector_remtravelAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest, true);
    }

    return true;
}

/*####
# npc_threshwackonator
####*/

enum
{
    EMOTE_START             = -1000325,
    SAY_AT_CLOSE            = -1000326,
    QUEST_GYROMAST_REV      = 2078,
    NPC_GELKAK              = 6667,
    FACTION_HOSTILE         = 14,

    TEXT_ID_THRESH_DEFAULT  = 718,
    TEXT_ID_KEY_READY       = 758,

    GOSSIP_ITEM_TURN_KEY    = -3000111,
};

struct npc_threshwackonatorAI : public FollowerAI
{
    npc_threshwackonatorAI(Creature* pCreature) : FollowerAI(pCreature) { Reset(); }

    void Reset() override {}

    void MoveInLineOfSight(Unit* pWho) override
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!m_creature->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == NPC_GELKAK)
        {
            if (m_creature->IsWithinDistInMap(pWho, 10.0f))
            {
                DoScriptText(SAY_AT_CLOSE, pWho);
                DoAtEnd();
            }
        }
    }

    void DoAtEnd()
    {
        m_creature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN);

        if (Player* pHolder = GetLeaderForFollower())
            m_creature->AI()->AttackStart(pHolder);

        SetFollowComplete();
    }
};

CreatureAI* GetAI_npc_threshwackonator(Creature* pCreature)
{
    return new npc_threshwackonatorAI(pCreature);
}

bool GossipHello_npc_threshwackonator(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_GYROMAST_REV) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TURN_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->SEND_GOSSIP_MENU(TEXT_ID_KEY_READY, pCreature->GetObjectGuid());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(TEXT_ID_THRESH_DEFAULT, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_threshwackonator(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_threshwackonatorAI* pThreshAI = dynamic_cast<npc_threshwackonatorAI*>(pCreature->AI()))
        {
            DoScriptText(EMOTE_START, pCreature);
            pThreshAI->StartFollow(pPlayer);
        }
    }

    return true;
}

/*######
# npc_volcor
######*/

enum
{
    SAY_START                       = -1000789,
    SAY_END                         = -1000790,
    SAY_FIRST_AMBUSH                = -1000791,
    SAY_AGGRO_1                     = -1000792,
    SAY_AGGRO_2                     = -1000793,
    SAY_AGGRO_3                     = -1000794,

    SAY_ESCAPE                      = -1000195,

    NPC_BLACKWOOD_SHAMAN            = 2171,
    NPC_BLACKWOOD_URSA              = 2170,

    SPELL_MOONSTALKER_FORM          = 10849,

    WAYPOINT_ID_QUEST_STEALTH       = 16,
    FACTION_FRIENDLY                = 35,

    QUEST_ESCAPE_THROUGH_FORCE      = 994,
    QUEST_ESCAPE_THROUGH_STEALTH    = 995,
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

// Spawn locations
static const SummonLocation aVolcorSpawnLocs[] =
{
    {4630.2f, 22.6f, 70.1f, 2.4f},
    {4603.8f, 53.5f, 70.4f, 5.4f},
    {4627.5f, 100.4f, 62.7f, 5.8f},
    {4692.8f, 75.8f, 56.7f, 3.1f},
    {4747.8f, 152.8f, 54.6f, 2.4f},
    {4711.7f, 109.1f, 53.5f, 2.4f},
};

struct npc_volcorAI : public npc_escortAI
{
    npc_volcorAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    uint32 m_uiQuestId;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_uiQuestId = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        // shouldn't always use text on agro
        switch (urand(0, 4))
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO_3, m_creature); break;
        }
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // No combat for this quest
        if (m_uiQuestId == QUEST_ESCAPE_THROUGH_STEALTH)
            return;

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    // Wrapper to handle start function for both quests
    void StartEscort(Player* pPlayer, const Quest* pQuest)
    {
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        m_creature->SetFacingToObject(pPlayer);
        m_uiQuestId = pQuest->GetQuestId();

        if (pQuest->GetQuestId() == QUEST_ESCAPE_THROUGH_STEALTH)
        {
            // Note: faction may not be correct, but only this way works fine
            m_creature->SetFactionTemporary(FACTION_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN);

            Start(true, pPlayer, pQuest);
            SetEscortPaused(true);
            SetCurrentWaypoint(WAYPOINT_ID_QUEST_STEALTH);
            SetEscortPaused(false);
        }
        else
            Start(false, pPlayer, pQuest);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 2:
                DoScriptText(SAY_START, m_creature);
                break;
            case 5:
                m_creature->SummonCreature(NPC_BLACKWOOD_SHAMAN, aVolcorSpawnLocs[0].m_fX, aVolcorSpawnLocs[0].m_fY, aVolcorSpawnLocs[0].m_fZ, aVolcorSpawnLocs[0].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[1].m_fX, aVolcorSpawnLocs[1].m_fY, aVolcorSpawnLocs[1].m_fZ, aVolcorSpawnLocs[1].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
                break;
            case 6:
                DoScriptText(SAY_FIRST_AMBUSH, m_creature);
                break;
            case 11:
                m_creature->SummonCreature(NPC_BLACKWOOD_SHAMAN, aVolcorSpawnLocs[2].m_fX, aVolcorSpawnLocs[2].m_fY, aVolcorSpawnLocs[2].m_fZ, aVolcorSpawnLocs[2].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[3].m_fX, aVolcorSpawnLocs[3].m_fY, aVolcorSpawnLocs[3].m_fZ, aVolcorSpawnLocs[3].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
            case 13:
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[4].m_fX, aVolcorSpawnLocs[4].m_fY, aVolcorSpawnLocs[4].m_fZ, aVolcorSpawnLocs[4].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[5].m_fX, aVolcorSpawnLocs[5].m_fY, aVolcorSpawnLocs[5].m_fZ, aVolcorSpawnLocs[5].m_fO, TEMPSUMMON_TIMED_OOC_DESPAWN, 20000);
                break;
            case 15:
                DoScriptText(SAY_END, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ESCAPE_THROUGH_FORCE, m_creature);
                SetEscortPaused(true);
                m_creature->ForcedDespawn(10000);
                break;
                // Quest 995 waypoints
            case 16:
                m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                break;
            case 17:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_ESCAPE, m_creature, pPlayer);
                break;
            case 18:
                DoCastSpellIfCan(m_creature, SPELL_MOONSTALKER_FORM);
                break;
            case 24:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ESCAPE_THROUGH_STEALTH, m_creature);
                break;
        }
    }
};

CreatureAI* GetAI_npc_volcor(Creature* pCreature)
{
    return new npc_volcorAI(pCreature);
}

bool QuestAccept_npc_volcor(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ESCAPE_THROUGH_FORCE || pQuest->GetQuestId() == QUEST_ESCAPE_THROUGH_STEALTH)
    {
        if (npc_volcorAI* pEscortAI = dynamic_cast<npc_volcorAI*>(pCreature->AI()))
            pEscortAI->StartEscort(pPlayer, pQuest);
    }

    return true;
}

/*####
# npc_therylune
####*/

enum
{
    SAY_THERYLUNE_START              = -1000905,
    SAY_THERYLUNE_FINISH             = -1000906,

    NPC_THERYSIL                     = 3585,

    QUEST_ID_THERYLUNE_ESCAPE        = 945,
};

struct npc_theryluneAI : public npc_escortAI
{
    npc_theryluneAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }


    void Reset() override {}

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 17:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ID_THERYLUNE_ESCAPE, m_creature);
                break;
            case 19:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_THERYLUNE_FINISH, m_creature, pPlayer);
                SetRun();
                break;
        }
    }
};

CreatureAI* GetAI_npc_therylune(Creature* pCreature)
{
    return new npc_theryluneAI(pCreature);
}

bool QuestAccept_npc_therylune(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_THERYLUNE_ESCAPE)
    {
        if (npc_theryluneAI* pEscortAI = dynamic_cast<npc_theryluneAI*>(pCreature->AI()))
        {
            pEscortAI->Start(false, pPlayer, pQuest);
            DoScriptText(SAY_THERYLUNE_START, pCreature, pPlayer);
        }
    }

    return true;
}

/*######
## npc_rabid_bear
######*/

enum
{
    QUEST_PLAGUED_LANDS             = 2118,

    NPC_CAPTURED_RABID_THISTLE_BEAR = 11836,
    NPC_THARNARIUN_TREETENDER       = 3701,                 // Npc related to quest-outro
    GO_NIGHT_ELVEN_BEAR_TRAP        = 111148,               // This is actually the (visual) spell-focus GO

    SPELL_RABIES                    = 3150,                 // Spell used in comabt
};

struct npc_rabid_bearAI : public ScriptedAI
{
    npc_rabid_bearAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
        JustRespawned();
    }

    uint32 m_uiCheckTimer;
    uint32 m_uiRabiesTimer;
    uint32 m_uiDespawnTimer;

    void Reset() override
    {
        m_uiRabiesTimer = urand(12000, 18000);
    }

    void JustRespawned() override
    {
        m_uiCheckTimer = 1000;
        m_uiDespawnTimer = 0;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (!m_uiCheckTimer && pWho->GetTypeId() == TYPEID_UNIT && pWho->GetEntry() == NPC_THARNARIUN_TREETENDER &&
                pWho->IsWithinDist(m_creature, 2 * INTERACTION_DISTANCE, false))
        {
            // Possible related spell: 9455 9372
            m_creature->ForcedDespawn(1000);
            m_creature->GetMotionMaster()->MoveIdle();

            return;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiCheckTimer && m_creature->isInCombat())
        {
            if (m_uiCheckTimer <= uiDiff)
            {
                // Trap nearby?
                if (GameObject* pTrap = GetClosestGameObjectWithEntry(m_creature, GO_NIGHT_ELVEN_BEAR_TRAP, 0.5f))
                {
                    // Despawn trap
                    pTrap->Use(m_creature);
                    // "Evade"
                    m_creature->RemoveAllAurasOnEvade();
                    m_creature->DeleteThreatList();
                    m_creature->CombatStop(true);
                    m_creature->SetLootRecipient(nullptr);
                    Reset();
                    // Update Entry and start following player
                    m_creature->UpdateEntry(NPC_CAPTURED_RABID_THISTLE_BEAR);
                    // get player
                    Unit* pTrapOwner = pTrap->GetOwner();
                    if (pTrapOwner && pTrapOwner->GetTypeId() == TYPEID_PLAYER &&
                            ((Player*)pTrapOwner)->GetQuestStatus(QUEST_PLAGUED_LANDS) == QUEST_STATUS_INCOMPLETE)
                    {
                        ((Player*)pTrapOwner)->KilledMonsterCredit(m_creature->GetEntry(), m_creature->GetObjectGuid());
                        m_creature->GetMotionMaster()->MoveFollow(pTrapOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                    }
                    else                                // Something unexpected happened
                        m_creature->ForcedDespawn(1000);

                    // No need to check any more
                    m_uiCheckTimer = 0;
                    // Despawn after a while (delay guesswork)
                    m_uiDespawnTimer = 3 * MINUTE * IN_MILLISECONDS;

                    return;
                }
                else
                    m_uiCheckTimer = 1000;
            }
            else
                m_uiCheckTimer -= uiDiff;
        }

        if (m_uiDespawnTimer && !m_creature->isInCombat())
        {
            if (m_uiDespawnTimer <= uiDiff)
            {
                m_creature->ForcedDespawn();
                return;
            }
            else
                m_uiDespawnTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiRabiesTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, SPELL_RABIES))
                DoCastSpellIfCan(pTarget, SPELL_RABIES);
            m_uiRabiesTimer = urand(12000, 18000);
        }
        else
            m_uiRabiesTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_rabid_bear(Creature* pCreature)
{
    return new npc_rabid_bearAI(pCreature);
}

void AddSC_darkshore()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_kerlonian";
    pNewScript->GetAI = &GetAI_npc_kerlonian;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kerlonian;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_awaken_kerlonian;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_prospector_remtravel";
    pNewScript->GetAI = &GetAI_npc_prospector_remtravel;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_prospector_remtravel;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_threshwackonator";
    pNewScript->GetAI = &GetAI_npc_threshwackonator;
    pNewScript->pGossipHello = &GossipHello_npc_threshwackonator;
    pNewScript->pGossipSelect = &GossipSelect_npc_threshwackonator;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_volcor";
    pNewScript->GetAI = &GetAI_npc_volcor;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_volcor;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_therylune";
    pNewScript->GetAI = &GetAI_npc_therylune;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_therylune;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_rabid_bear";
    pNewScript->GetAI = &GetAI_npc_rabid_bear;
    pNewScript->RegisterSelf();
}
