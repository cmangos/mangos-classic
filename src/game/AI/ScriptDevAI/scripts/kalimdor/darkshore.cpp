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
SDComment: Quest support: 731, 945, 994, 995, 2078, 2118, 4763, 5321
SDCategory: Darkshore
EndScriptData

*/

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "Grids/GridNotifiers.h"
#include "Grids/CellImpl.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/base/follower_ai.h"
#include "Entities/UpdateData.h"

/* ContentData
npc_kerlonian
npc_prospector_remtravel
npc_threshwackonator
npc_volcor
npc_therylune
npc_rabid_bear
npc_corrupted_furbolg
npc_blackwood_warrior
npc_blackwood_totemic
event_purify_food
go_furbolg_food
EndContentData */


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

    void Aggro(Unit* /*attacker*/) override
    {
        m_creature->PlayDistanceSound(6701);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!m_creature->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == NPC_LILADRIS)
        {
            if (m_creature->IsWithinDistInMap(pWho, INTERACTION_DISTANCE * 5))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_SLEEPER_AWAKENED) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_SLEEPER_AWAKENED, m_creature);

                    DoScriptText(SAY_KER_END, m_creature);
                }

                SetFollowComplete();
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 /*uiMiscValue*/) override
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
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
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

UnitAI* GetAI_npc_kerlonian(Creature* pCreature)
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
            pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
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
            case 1:
                DoScriptText(SAY_REM_START, m_creature, pPlayer);
                break;
            case 6:
                DoScriptText(SAY_REM_RAMP1_1, m_creature, pPlayer);
                break;
            case 7:
                m_creature->SummonCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                break;
            case 10:
                DoScriptText(SAY_REM_RAMP1_2, m_creature, pPlayer);
                break;
            case 15:
                // depend quest rewarded?
                DoScriptText(SAY_REM_BOOK, m_creature, pPlayer);
                break;
            case 16:
                DoScriptText(SAY_REM_TENT1_1, m_creature, pPlayer);
                break;
            case 17:
                m_creature->SummonCreature(NPC_GRAVEL_SCOUT, -10.0f, 5.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRAVEL_BONE, -10.0f, 7.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                break;
            case 18:
                DoScriptText(SAY_REM_TENT1_2, m_creature, pPlayer);
                break;
            case 27:
                DoScriptText(SAY_REM_MOSS, m_creature, pPlayer);
                break;
            case 28:
                DoScriptText(EMOTE_REM_MOSS, m_creature, pPlayer);
                break;
            case 29:
                DoScriptText(SAY_REM_MOSS_PROGRESS, m_creature, pPlayer);
                break;
            case 30:
                m_creature->SummonCreature(NPC_GRAVEL_SCOUT, -15.0f, 3.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRAVEL_BONE, -15.0f, 5.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_GRAVEL_GEO, -15.0f, 7.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000);
                break;
            case 32:
                DoScriptText(SAY_REM_PROGRESS, m_creature, pPlayer);
                break;
            case 42:
                DoScriptText(SAY_REM_REMEMBER, m_creature, pPlayer);
                break;
            case 43:
                DoScriptText(EMOTE_REM_END, m_creature, pPlayer);
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ABSENT_MINDED_PT2, m_creature);
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

UnitAI* GetAI_npc_prospector_remtravel(Creature* pCreature)
{
    return new npc_prospector_remtravelAI(pCreature);
}

bool QuestAccept_npc_prospector_remtravel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ABSENT_MINDED_PT2)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

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

        if (!m_creature->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == NPC_GELKAK)
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

UnitAI* GetAI_npc_threshwackonator(Creature* pCreature)
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
            m_creature->SetFactionTemporary(FACTION_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

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
                m_creature->SummonCreature(NPC_BLACKWOOD_SHAMAN, aVolcorSpawnLocs[0].m_fX, aVolcorSpawnLocs[0].m_fY, aVolcorSpawnLocs[0].m_fZ, aVolcorSpawnLocs[0].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[1].m_fX, aVolcorSpawnLocs[1].m_fY, aVolcorSpawnLocs[1].m_fZ, aVolcorSpawnLocs[1].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                break;
            case 6:
                DoScriptText(SAY_FIRST_AMBUSH, m_creature);
                break;
            case 11:
                m_creature->SummonCreature(NPC_BLACKWOOD_SHAMAN, aVolcorSpawnLocs[2].m_fX, aVolcorSpawnLocs[2].m_fY, aVolcorSpawnLocs[2].m_fZ, aVolcorSpawnLocs[2].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[3].m_fX, aVolcorSpawnLocs[3].m_fY, aVolcorSpawnLocs[3].m_fZ, aVolcorSpawnLocs[3].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
            case 13:
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[4].m_fX, aVolcorSpawnLocs[4].m_fY, aVolcorSpawnLocs[4].m_fZ, aVolcorSpawnLocs[4].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_BLACKWOOD_URSA, aVolcorSpawnLocs[5].m_fX, aVolcorSpawnLocs[5].m_fY, aVolcorSpawnLocs[5].m_fZ, aVolcorSpawnLocs[5].m_fO, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                break;
            case 15:
                DoScriptText(SAY_END, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ESCAPE_THROUGH_FORCE, m_creature);
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
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ESCAPE_THROUGH_STEALTH, m_creature);
                break;
        }
    }
};

UnitAI* GetAI_npc_volcor(Creature* pCreature)
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

    void JustRespawned() override
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        npc_escortAI::JustRespawned();
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 18:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ID_THERYLUNE_ESCAPE, m_creature);
                break;
            case 20:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_THERYLUNE_FINISH, m_creature, pPlayer);
                SetRun();
                break;
        }
    }
};

UnitAI* GetAI_npc_therylune(Creature* pCreature)
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
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
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
    SPELL_BEAR_CAPTURED             = 9439
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

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster->GetTypeId() == TYPEID_PLAYER && pSpell->Id == SPELL_BEAR_CAPTURED)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
            m_creature->SetLootRecipient(nullptr);
            Reset();
            // Update Entry and start following player
            m_creature->UpdateEntry(NPC_CAPTURED_RABID_THISTLE_BEAR);
            // get player
            if (((Player*)pCaster)->GetQuestStatus(QUEST_PLAGUED_LANDS) == QUEST_STATUS_INCOMPLETE)
            {
                ((Player*)pCaster)->KilledMonsterCredit(m_creature->GetEntry(), m_creature->GetObjectGuid());
                m_creature->GetMotionMaster()->MoveFollow(pCaster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }
            else                                // Something unexpected happened
                m_creature->ForcedDespawn(1000);

            // No need to check any more
            m_uiCheckTimer = 0;
            // Despawn after a while (delay guesswork)
            m_uiDespawnTimer = 3 * MINUTE * IN_MILLISECONDS;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiDespawnTimer && !m_creature->IsInCombat())
        {
            if (m_uiDespawnTimer <= uiDiff)
            {
                m_creature->ForcedDespawn();
                return;
            }
            m_uiDespawnTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
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

UnitAI* GetAI_npc_rabid_bear(Creature* pCreature)
{
    return new npc_rabid_bearAI(pCreature);
}

/*######
## npc_corrupted_furbolg
######*/

enum {
    NPC_BLACKWOOD_WARRIOR    = 2168,
    NPC_BLACKWOOD_TOTEMIC    = 2169,

    GO_PURIFIED_FOOD         = 175336,

    FACTION_BLACKWOOD        = 35,         // Faction guessed

    EMOTE_LURED              = -1010027,
    EMOTE_PURIFIED           = -1010028,
    EMOTE_GENERIC_FLEE       = -1000007,

    SPELL_BATTLE_STANCE      = 7165,
    SPELL_THUNDERCLAP        = 8078,
    SPELL_HEALING_WARD       = 5605,



    EVENT_BECOME_PURIFIED    = 0,
    EVENT_FURBOLG_RESET      = 1,
    EVENT_START_PURIFICATION = 2,
    ACTION_FLEE              = 3,
    ACTION_THUNDERCLAP       = 4,
    ACTION_HEALING_WARD      = 5,
    ACTION_MAX               = 6,

    BOWL_DISTANCE            = 5,
};

static const std::vector<uint32> furbolgList = { NPC_BLACKWOOD_WARRIOR, NPC_BLACKWOOD_TOTEMIC };

struct npc_corrupted_furbolgAI : public CombatAI
{
    npc_corrupted_furbolgAI(Creature* creature) : CombatAI(creature, ACTION_MAX)
    {
        AddCustomAction(EVENT_BECOME_PURIFIED, true, [&]()
        {
            if (m_isFirst)
                DoScriptText(EMOTE_PURIFIED, m_creature);
            m_creature->SetFactionTemporary(FACTION_BLACKWOOD, TEMPFACTION_RESTORE_COMBAT_STOP);
            m_creature->GetMotionMaster()->MoveRandomAroundPoint(m_bowlCoords.GetPositionX(), m_bowlCoords.GetPositionY(), m_bowlCoords.GetPositionZ(), 40.f);
            m_creature->SetWalk(true);
            ResetTimer(EVENT_FURBOLG_RESET, 1.5 * MINUTE * IN_MILLISECONDS);
        });
        AddCustomAction(EVENT_FURBOLG_RESET, true, [&]()
        {
            m_creature->GetMotionMaster()->Initialize();
            EnterEvadeMode();
            if (m_creature->GetDefaultMovementType() == IDLE_MOTION_TYPE)
                m_creature->GetMotionMaster()->MoveTargetedHome(false);
        });
        AddCustomAction(EVENT_START_PURIFICATION, true, [&]()
        {
            if (!m_bowlCoords.IsEmpty() && m_bowlCoords.GetDistance(m_creature->GetPosition()) <= 3600.f)
            {
                if (m_isFirst)
                    DoScriptText(EMOTE_LURED, m_creature);
                SetCombatScriptStatus(true);
                SetReactState(REACT_PASSIVE);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
                m_creature->CombatStop();

                float x, y, z, angle;
                angle = m_creature->GetAngle(m_bowlCoords.GetPositionX(), m_bowlCoords.GetPositionY());
                m_creature->GetNearPointAt(m_bowlCoords.GetPositionX(), m_bowlCoords.GetPositionY(), m_bowlCoords.GetPositionZ(),
                    m_creature, x, y, z, m_creature->GetObjectBoundingRadius(), CONTACT_DISTANCE * 2.f, angle + M_PI);
                m_creature->SetWalk(false);
                m_creature->GetMotionMaster()->MovePoint(1, x, y, z);
            }
        });
        AddTimerlessCombatAction(ACTION_FLEE, true);
        switch (m_creature->GetEntry())
        {
            case NPC_BLACKWOOD_WARRIOR: AddCombatAction(ACTION_THUNDERCLAP, 16000, 21000); break;
            case NPC_BLACKWOOD_TOTEMIC: AddCombatAction(ACTION_HEALING_WARD, 6000, 12000); break;
        }
    }

    bool m_isFirst;
    bool m_hasFled;
    Position m_bowlCoords;

    void Reset() override
    {
        CombatAI::Reset();
        
        switch (m_creature->GetEntry())
        {
            case NPC_BLACKWOOD_WARRIOR: DoCastSpellIfCan(m_creature, SPELL_BATTLE_STANCE); break;
            case NPC_BLACKWOOD_TOTEMIC: break;
        }
        
        SetCombatScriptStatus(false);
        m_hasFled           = false;
        m_isFirst           = false;
        SetReactState(REACT_AGGRESSIVE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
    }

    uint32 GetSubsequentActionTimer(uint32 actionID)
    {
        switch (actionID)
        {
            case ACTION_THUNDERCLAP: return urand(16, 21) * IN_MILLISECONDS;
            case ACTION_HEALING_WARD: return urand(30, 36) * IN_MILLISECONDS;
        }
        return 0;
    }

    void SeekPurification(bool isFirst, Position bowlCoords)
    {
        m_isFirst = isFirst;
        m_bowlCoords = bowlCoords;
        if(m_isFirst)
            ResetTimer(EVENT_START_PURIFICATION, 0);
        else
            ResetTimer(EVENT_START_PURIFICATION, urand(0, 5 * IN_MILLISECONDS));
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != POINT_MOTION_TYPE || !pointId)
            return;

        m_creature->GetMotionMaster()->MoveIdle();
        ResetTimer(EVENT_BECOME_PURIFIED, 5 * IN_MILLISECONDS);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ACTION_FLEE:
            {
                if (m_creature->GetHealthPercent() <= 15.f)
                {
                    DoScriptText(EMOTE_GENERIC_FLEE, m_creature);
                    if (DoFlee())
                        m_hasFled = true;
                    DisableCombatAction(action);
                }
                break;
            }
            case ACTION_THUNDERCLAP:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE))
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_THUNDERCLAP) == CAST_OK)
                        ResetCombatAction(action, GetSubsequentActionTimer(action));
                }
                break;
            }
            case ACTION_HEALING_WARD:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_HEALING_WARD) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                break;
            }
        }
    }
};

/*######
## EventId_event_purify_food
######*/

bool ProcessEventId_event_purify_food(uint32 /*eventId*/, Object* source, Object* target, bool /*isStart*/)
{
    if (source->GetTypeId() == TYPEID_PLAYER && target->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        GameObject* bonfire = (GameObject*)target;
        Player* player = (Player*)source;
        CreatureList furbolgs;
        Position bowlCoords;
        // Get all nearby Blackwood Furbolgs
        GetCreatureListWithEntryInGrid(furbolgs, bonfire, furbolgList, 40.f);
        player->GetFirstCollisionPosition(bowlCoords, float(BOWL_DISTANCE), player->GetOrientation());

        // Spawn two extra Blackwood Warriors
        float x, y, z;
        for (int8 i = 0; i < 2; ++i)
        {
            bonfire->GetRandomPoint(bonfire->GetPositionX(), bonfire->GetPositionY(), bonfire->GetPositionZ(), 30.0f, x, y, z);
            if (Creature* warrior = bonfire->SummonCreature(NPC_BLACKWOOD_WARRIOR, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
                furbolgs.push_back(warrior);
        }

        // Activate the purification sequence for all hostile furbolgs (prevent attracting already purified ones)
        for (auto& furbolg : furbolgs)
        {
            if (furbolg->IsInCombat() || !furbolg->IsAlive() || !furbolg->CanAttack((Unit*)source))
                continue;

            if (auto* furbolgAI = dynamic_cast<npc_corrupted_furbolgAI*>(furbolg->AI()))
                furbolgAI->SeekPurification(furbolg == furbolgs.back(), bowlCoords);
        }

        return false;
    }

    return true;
}

/*######
## go_furbolg_food
######*/

bool GOUse_go_furbolg_food(Player* player, GameObject* go)
{
    // Search for a nearby Blackwood Warrior and make it attack
    if (Creature* warrior = GetClosestCreatureWithEntry(go, NPC_BLACKWOOD_WARRIOR, 20.0f))
    {
        if (!warrior->IsInCombat() && warrior->IsAlive() && warrior->CanAttack(player))
            warrior->AI()->AttackStart(player);
    }
    // Else spawn a new one (and make it attack)
    else
    {
        float x, y, z;
        go->GetRandomPoint(go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 20.0f, x, y, z);
        if (Creature* warrior = go->SummonCreature(NPC_BLACKWOOD_WARRIOR, x, y, z, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 120000))
            warrior->AI()->AttackStart(player);
    }

    return true;
}

void AddSC_darkshore()
{
    Script* pNewScript = new Script;
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

    pNewScript = new Script;
    pNewScript->Name = "npc_blackwood_warrior";
    pNewScript->GetAI = &GetNewAIInstance<npc_corrupted_furbolgAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_blackwood_totemic";
    pNewScript->GetAI = &GetNewAIInstance<npc_corrupted_furbolgAI>;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "event_purify_food";
    pNewScript->pProcessEventId = &ProcessEventId_event_purify_food;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_furbolg_food";
    pNewScript->pGOUse = &GOUse_go_furbolg_food;
    pNewScript->RegisterSelf();
}
