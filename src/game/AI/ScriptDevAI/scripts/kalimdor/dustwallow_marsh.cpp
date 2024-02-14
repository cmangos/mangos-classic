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
SDName: Dustwallow_Marsh
SD%Complete: 95
SDComment: Quest support: 1173, 1222, 1270, 1273, 1324.
SDCategory: Dustwallow Marsh
EndScriptData

*/

/* ContentData
npc_morokk
npc_ogron
npc_private_hendel
npc_stinky_ignatz
at_nats_landing
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/scripts/kalimdor/world_kalimdor.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include <unordered_map>

/*######
## npc_morokk
######*/

enum
{
    SAY_MOR_CHALLENGE       = -1000499,
    SAY_MOR_SCARED          = -1000500,

    NPC_MOROKK              = 4500,
    QUEST_CHALLENGE_MOROKK  = 1173,

    FACTION_MOR_HOSTILE     = 168,
    FACTION_MOR_RUNNING     = 35,
    FACTION_MOR_SPAWN       = 29
};

struct npc_morokkAI : public npc_escortAI
{
    npc_morokkAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_bIsSuccess = false;
        Reset();
    }

    bool m_bIsSuccess;

    void Reset() override {}

    void JustRespawned() override
    {
        npc_escortAI::JustRespawned();
        m_creature->setFaction(FACTION_MOR_SPAWN);
    }

    void JustReachedHome() override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (!m_bIsSuccess)
            {
                FailQuestForPlayerAndGroup();
                m_creature->ForcedDespawn();
            }
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 1:
                SetEscortPaused(true);
                break;
        }
    }

    void UpdateEscortAI(const uint32 /*uiDiff*/) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    if (pPlayer->IsAlive() && pPlayer->IsInRange(m_creature, 0, 70))
                    {
                        m_bIsSuccess = false;
                        DoScriptText(SAY_MOR_CHALLENGE, m_creature, pPlayer);
                        m_creature->SetImmuneToPlayer(false);
                        SetReactState(REACT_AGGRESSIVE);
                        m_creature->setFaction(FACTION_MOR_HOSTILE);
                        AttackStart(pPlayer);
                    }
                    else
                        SetEscortPaused(false);
                }
                else
                    SetEscortPaused(false);
            }
            return;
        }
        if (m_creature->GetHealthPercent() <= 30.0f)
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_CHALLENGE_MOROKK, m_creature);
                    m_creature->setFaction(FACTION_MOR_RUNNING);
                    m_bIsSuccess = true;
                    m_creature->RemoveAllAurasOnEvade();
                    m_creature->CombatStop(true);
                    m_creature->SetImmuneToPlayer(true);
                    SetEscortPaused(false);
                    SetReactState(REACT_PASSIVE);
                    DoScriptText(SAY_MOR_SCARED, m_creature);
                    m_creature->GetMotionMaster()->Clear(false); // TODO: make whole EscortAI work like this
                }
            }
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_morokk(Creature* pCreature)
{
    return new npc_morokkAI(pCreature);
}

bool QuestAccept_npc_morokk(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHALLENGE_MOROKK)
    {
        if (npc_morokkAI* pEscortAI = dynamic_cast<npc_morokkAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);

        return true;
    }

    return false;
}

/*######
## npc_ogron
######*/

enum
{
    SAY_OGR_START                       = -1000452,
    SAY_OGR_SPOT                        = -1000453,
    SAY_OGR_RET_WHAT                    = -1000454,
    SAY_OGR_RET_SWEAR                   = -1000455,
    SAY_OGR_REPLY_RET                   = -1000456,
    SAY_OGR_RET_TAKEN                   = -1000457,
    SAY_OGR_TELL_FIRE                   = -1000458,
    SAY_OGR_RET_NOCLOSER                = -1000459,
    SAY_OGR_RET_NOFIRE                  = -1000460,
    SAY_OGR_RET_HEAR                    = -1000461,
    SAY_OGR_CAL_FOUND                   = -1000462,
    SAY_OGR_CAL_MERCY                   = -1000463,
    SAY_OGR_HALL_GLAD                   = -1000464,
    EMOTE_OGR_RET_ARROW                 = -1000465,
    SAY_OGR_RET_ARROW                   = -1000466,
    SAY_OGR_CAL_CLEANUP                 = -1000467,
    SAY_OGR_NODIE                       = -1000468,
    SAY_OGR_SURVIVE                     = -1000469,
    SAY_OGR_RET_LUCKY                   = -1000470,
    SAY_OGR_THANKS                      = -1000471,

    QUEST_QUESTIONING                   = 1273,

    FACTION_GENERIC_FRIENDLY            = 35,
    FACTION_THER_HOSTILE                = 151,

    NPC_REETHE                          = 4980,
    NPC_CALDWELL                        = 5046,
    NPC_HALLAN                          = 5045,
    NPC_SKIRMISHER                      = 5044,

    SPELL_FAKE_SHOT                     = 7105,

    PHASE_INTRO                         = 0,
    PHASE_GUESTS                        = 1,
    PHASE_FIGHT                         = 2,
    PHASE_COMPLETE                      = 3
};

static float m_afSpawn[] = { -3383.501953f, -3203.383301f, 36.149f};
static float m_afMoveTo[] = { -3371.414795f, -3212.179932f, 34.210f};

struct npc_ogronAI : public npc_escortAI
{
    npc_ogronAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        lCreatureList.clear();
        m_uiPhase = 0;
        m_uiPhaseCounter = 0;
        Reset();
    }

    CreatureList lCreatureList;

    uint32 m_uiPhase;
    uint32 m_uiPhaseCounter;
    uint32 m_uiGlobalTimer;

    void Reset() override
    {
        m_uiGlobalTimer = 5000;

        if (HasEscortState(STATE_ESCORT_PAUSED) && m_uiPhase == PHASE_FIGHT)
            m_uiPhase = PHASE_COMPLETE;

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            lCreatureList.clear();
            m_uiPhase = 0;
            m_uiPhaseCounter = 0;
        }
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && pWho->GetEntry() == NPC_REETHE && lCreatureList.empty())
            lCreatureList.push_back((Creature*)pWho);

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    Creature* GetCreature(uint32 uiCreatureEntry)
    {
        if (!lCreatureList.empty())
        {
            for (auto& itr : lCreatureList)
            {
                if (itr->GetEntry() == uiCreatureEntry && itr->IsAlive())
                    return itr;
            }
        }

        return nullptr;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 10:
                DoScriptText(SAY_OGR_SPOT, m_creature);
                break;
            case 11:
                if (Creature* pReethe = GetCreature(NPC_REETHE))
                    DoScriptText(SAY_OGR_RET_WHAT, pReethe);
                break;
            case 12:
                SetEscortPaused(true);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        lCreatureList.push_back(pSummoned);

        pSummoned->setFaction(FACTION_GENERIC_FRIENDLY);

        if (pSummoned->GetEntry() == NPC_CALDWELL)
            pSummoned->GetMotionMaster()->MovePoint(0, m_afMoveTo[0], m_afMoveTo[1], m_afMoveTo[2]);
        else
        {
            if (Creature* pCaldwell = GetCreature(NPC_CALDWELL))
            {
                // will this conversion work without compile warning/error?
                size_t iSize = lCreatureList.size();
                pSummoned->GetMotionMaster()->MoveFollow(pCaldwell, 0.5f, (M_PI / 2) * (int)iSize);
            }
        }
    }

    void DoStartAttackMe()
    {
        if (!lCreatureList.empty())
        {
            for (auto& itr : lCreatureList)
            {
                if (itr->GetEntry() == NPC_REETHE)
                    continue;

                if (itr->IsAlive())
                {
                    itr->setFaction(FACTION_THER_HOSTILE);
                    itr->AI()->AttackStart(m_creature);
                }
            }
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (m_uiGlobalTimer < uiDiff)
                {
                    m_uiGlobalTimer = 5000;

                    switch (m_uiPhase)
                    {
                        case PHASE_INTRO:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 0:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_SWEAR, pReethe);
                                    break;
                                case 1:
                                    DoScriptText(SAY_OGR_REPLY_RET, m_creature);
                                    break;
                                case 2:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_TAKEN, pReethe);
                                    break;
                                case 3:
                                    DoScriptText(SAY_OGR_TELL_FIRE, m_creature);
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_NOCLOSER, pReethe);
                                    break;
                                case 4:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_NOFIRE, pReethe);
                                    break;
                                case 5:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_HEAR, pReethe);

                                    m_creature->SummonCreature(NPC_CALDWELL, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
                                    m_creature->SummonCreature(NPC_HALLAN, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
                                    m_creature->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
                                    m_creature->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);

                                    m_uiPhase = PHASE_GUESTS;
                                    break;
                            }
                            break;
                        }
                        case PHASE_GUESTS:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 6:
                                    if (Creature* pCaldwell = GetCreature(NPC_CALDWELL))
                                        DoScriptText(SAY_OGR_CAL_FOUND, pCaldwell);
                                    break;
                                case 7:
                                    if (Creature* pCaldwell = GetCreature(NPC_CALDWELL))
                                        DoScriptText(SAY_OGR_CAL_MERCY, pCaldwell);
                                    break;
                                case 8:
                                    if (Creature* pHallan = GetCreature(NPC_HALLAN))
                                    {
                                        DoScriptText(SAY_OGR_HALL_GLAD, pHallan);

                                        if (Creature* pReethe = GetCreature(NPC_REETHE))
                                            pHallan->CastSpell(pReethe, SPELL_FAKE_SHOT, TRIGGERED_NONE);
                                    }
                                    break;
                                case 9:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                    {
                                        DoScriptText(EMOTE_OGR_RET_ARROW, pReethe);
                                        DoScriptText(SAY_OGR_RET_ARROW, pReethe);
                                    }
                                    break;
                                case 10:
                                    if (Creature* pCaldwell = GetCreature(NPC_CALDWELL))
                                        DoScriptText(SAY_OGR_CAL_CLEANUP, pCaldwell);

                                    DoScriptText(SAY_OGR_NODIE, m_creature);
                                    break;
                                case 11:
                                    DoStartAttackMe();
                                    m_uiPhase = PHASE_FIGHT;
                                    break;
                            }
                            break;
                        }
                        case PHASE_COMPLETE:
                        {
                            switch (m_uiPhaseCounter)
                            {
                                case 12:
                                    if (Player* pPlayer = GetPlayerForEscort())
                                        pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_QUESTIONING, m_creature);

                                    DoScriptText(SAY_OGR_SURVIVE, m_creature);
                                    break;
                                case 13:
                                    if (Creature* pReethe = GetCreature(NPC_REETHE))
                                        DoScriptText(SAY_OGR_RET_LUCKY, pReethe);
                                    break;
                                case 14:
                                    DoScriptText(SAY_OGR_THANKS, m_creature);
                                    SetRun();
                                    SetEscortPaused(false);
                                    break;
                            }
                            break;
                        }
                    }

                    if (m_uiPhase != PHASE_FIGHT)
                        ++m_uiPhaseCounter;
                }
                else
                    m_uiGlobalTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_ogron(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_QUESTIONING)
    {
        if (npc_ogronAI* pEscortAI = dynamic_cast<npc_ogronAI*>(pCreature->AI()))
        {
            pEscortAI->Start(false, pPlayer, pQuest, true);
            pCreature->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
            DoScriptText(SAY_OGR_START, pCreature, pPlayer);
        }
    }

    return true;
}

UnitAI* GetAI_npc_ogron(Creature* pCreature)
{
    return new npc_ogronAI(pCreature);
}

/*######
## npc_private_hendel
######*/

enum
{
    EMOTE_SURRENDER             = -1000415,

    QUEST_MISSING_DIPLO_PT16    = 1324,
    FACTION_HOSTILE             = 168,                      // guessed, may be different

    NPC_SENTRY                  = 5184,                     // helps hendel
    NPC_JAINA                   = 4968,                     // appears once hendel gives up
    NPC_TERVOSH                 = 4967,
    NPC_PAINED                  = 4965,

    SPELL_TELEPORT_VISUAL       = 12980,
    SPELL_TELEPORT              = 7079
};

struct OutroSpawnLocation
{
    float fX, fY, fZ, fO;
    float fDestX, fDestY, fDestZ;
    uint32 uiEntry;
};

const OutroSpawnLocation lOutroSpawns[] =
{
    {
        -2857.604492f, -3354.784912f, 35.369640f, 3.16604f,
        -2881.546631f, -3346.477539f, 34.143719f,
        NPC_TERVOSH
    },
    {
        -2858.120117f, -3358.469971f, 36.086300f, 3.16604f,
        -2879.697998f, -3347.789063f, 34.772892f,
        NPC_JAINA
    },
    {
        -2857.379883f, -3351.370117f, 34.178001f, 3.16604f,
        -2879.959961f, -3344.469971f, 34.670502f,
        NPC_PAINED
    }
};

const float fSentryFleePoint[] {-2917.56f, -3329.90f, 30.37f};

struct npc_private_hendelAI : public ScriptedAI
{
    npc_private_hendelAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    void Reset() override {}

    ObjectGuid guidPlayer;

    void AttackedBy(Unit* pAttacker) override
    {
        if (m_creature->GetVictim())
            return;

        if (!m_creature->CanAttackNow(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        // If Private Hendel is hit by spell Teleport (from DBScript)
        // this means it is time to grant quest credit to the player previously stored to this intend
        if (pSpell->Id == SPELL_TELEPORT)
        {
            if (Player* pPlayer = m_creature->GetMap()->GetPlayer(guidPlayer))
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_MISSING_DIPLO_PT16, m_creature);
        }
    }

    void DamageTaken(Unit* pDoneBy, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (damage > m_creature->GetHealth() || m_creature->GetHealthPercent() < 20.0f)
        {
            if (Player* pPlayer = pDoneBy->GetBeneficiaryPlayer())
            {
                if (pPlayer->GetQuestStatus(QUEST_MISSING_DIPLO_PT16) == QUEST_STATUS_INCOMPLETE)
                    guidPlayer = pPlayer->GetObjectGuid();  // Store the player to give quest credit later
            }

            damage = 0;

            DoScriptText(EMOTE_SURRENDER, m_creature);
            EnterEvadeMode();

            // Make the two sentries flee and despawn
            CreatureList lSentryList;
            GetCreatureListWithEntryInGrid(lSentryList, m_creature, NPC_SENTRY, 40.0f);

            for (CreatureList::const_iterator itr = lSentryList.begin(); itr != lSentryList.end(); ++itr)
            {
                if ((*itr)->IsAlive())
                {
                    (*itr)->RemoveAllAurasOnEvade();
                    (*itr)->CombatStop(true);
                    (*itr)->SetWalk(false);
                    (*itr)->GetMotionMaster()->MovePoint(0, fSentryFleePoint[0], fSentryFleePoint[1], fSentryFleePoint[2]);
                    (*itr)->ForcedDespawn(4000);
                }
            }

            // Summon Jaina Proudmoore, Archmage Tervosh and Pained
            for (const auto& lOutroSpawn : lOutroSpawns)
            {
                Creature* pCreature = m_creature->SummonCreature(lOutroSpawn.uiEntry, lOutroSpawn.fX, lOutroSpawn.fY, lOutroSpawn.fZ, lOutroSpawn.fO, TEMPSPAWN_TIMED_DESPAWN, 3 * MINUTE * IN_MILLISECONDS, false, true);
                if (pCreature)
                {
                    pCreature->CastSpell(pCreature, SPELL_TELEPORT_VISUAL, TRIGGERED_NONE);
                    pCreature->GetMotionMaster()->MovePoint(0, lOutroSpawn.fDestX, lOutroSpawn.fDestY, lOutroSpawn.fDestZ);

                    // Exception case for Archmage Tervosh: the outro event is a simple speech with visual spell cast
                    // so it will be handled by a DBScript held by NPC Archmage Tervosh
                    if (pCreature->GetEntry() == NPC_TERVOSH)
                    {
                        // Remove Gossip and Quest Giver flag from now, they will be re-added later to Archmage Tervosh in DBScript
                        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                        // The DBScript will be done here
                        pCreature->GetMotionMaster()->MoveWaypoint(0);
                    }
                }
            }
        }
    }
};

bool QuestAccept_npc_private_hendel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT16)
    {
        pCreature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_COMBAT_STOP | TEMPFACTION_RESTORE_RESPAWN);
        pCreature->AI()->AttackStart(pPlayer);

        // Find the nearby sentries in order to make them attack
        // The two sentries are linked to Private Hendel in DB to ensure they respawn together
        CreatureList lSentryList;
        GetCreatureListWithEntryInGrid(lSentryList, pCreature, NPC_SENTRY, 40.0f);

        for (CreatureList::const_iterator itr = lSentryList.begin(); itr != lSentryList.end(); ++itr)
        {
            if ((*itr)->IsAlive())
            {
                (*itr)->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_COMBAT_STOP | TEMPFACTION_RESTORE_RESPAWN);
                (*itr)->AI()->AttackStart(pPlayer);
            }
        }
    }

    return true;
}

UnitAI* GetAI_npc_private_hendel(Creature* pCreature)
{
    return new npc_private_hendelAI(pCreature);
}

/*#####
## npc_stinky_ignatz
## TODO Note: Faction change is guessed
#####*/

enum
{
    QUEST_ID_STINKYS_ESCAPE_ALLIANCE    = 1222,
    QUEST_ID_STINKYS_ESCAPE_HORDE       = 1270,

    SAY_STINKY_BEGIN                    = -1000958,
    SAY_STINKY_FIRST_STOP               = -1000959,
    SAY_STINKY_SECOND_STOP              = -1001141,
    SAY_STINKY_THIRD_STOP_1             = -1001142,
    SAY_STINKY_THIRD_STOP_2             = -1001143,
    SAY_STINKY_THIRD_STOP_3             = -1001144,
    SAY_STINKY_PLANT_GATHERED           = -1001145,
    SAY_STINKY_END                      = -1000962,
    SAY_STINKY_END_EMOTE                = -1010032,
    SAY_STINKY_AGGRO_1                  = -1000960,
    SAY_STINKY_AGGRO_2                  = -1000961,
    SAY_STINKY_AGGRO_3                  = -1001146,
    SAY_STINKY_AGGRO_4                  = -1001147,

    GO_BOGBEAN_PLANT                    = 20939,
};

struct npc_stinky_ignatzAI : public npc_escortAI
{
    npc_stinky_ignatzAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        SetReactState(REACT_DEFENSIVE);
        Reset();
    }

    ObjectGuid m_bogbeanPlantGuid;

    void Reset() override {}

    void Aggro(Unit* pWho) override
    {
        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_STINKY_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_STINKY_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_STINKY_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_STINKY_AGGRO_4, m_creature, pWho); break;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_STINKY_BEGIN, m_creature);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue));
            m_creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 6:
                DoScriptText(SAY_STINKY_FIRST_STOP, m_creature);
                break;
            case 11:
                DoScriptText(SAY_STINKY_SECOND_STOP, m_creature);
                break;
            case 25:
                DoScriptText(SAY_STINKY_THIRD_STOP_1, m_creature);
                break;
            case 26:
                DoScriptText(SAY_STINKY_THIRD_STOP_2, m_creature);
                if (GameObject* pBogbeanPlant = GetClosestGameObjectWithEntry(m_creature, GO_BOGBEAN_PLANT, DEFAULT_VISIBILITY_DISTANCE))
                {
                    m_bogbeanPlantGuid = pBogbeanPlant->GetObjectGuid();
                    m_creature->SetFacingToObject(pBogbeanPlant);
                }
                break;
            case 27:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_STINKY_THIRD_STOP_3, m_creature, pPlayer);
                break;
            case 30:
                m_creature->HandleEmote(EMOTE_STATE_USESTANDING);
                break;
            case 31:
                DoScriptText(SAY_STINKY_PLANT_GATHERED, m_creature);
                break;
            case 40:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    pPlayer->RewardPlayerAndGroupAtEventExplored(pPlayer->GetTeam() == ALLIANCE ? QUEST_ID_STINKYS_ESCAPE_ALLIANCE : QUEST_ID_STINKYS_ESCAPE_HORDE, m_creature);
                    DoScriptText(SAY_STINKY_END, m_creature, pPlayer);
                }
                break;
            case 41:
                DoScriptText(SAY_STINKY_END_EMOTE, m_creature);
                break;
        }
    }

    void WaypointStart(uint32 uiPointId) override
    {
        if (uiPointId == 31)
        {
            if (GameObject* pBogbeanPlant = m_creature->GetMap()->GetGameObject(m_bogbeanPlantGuid))
                pBogbeanPlant->Use(m_creature);
            m_bogbeanPlantGuid.Clear();
            m_creature->HandleEmote(EMOTE_ONESHOT_NONE);
        }
    }
};

UnitAI* GetAI_npc_stinky_ignatz(Creature* pCreature)
{
    return new npc_stinky_ignatzAI(pCreature);
}

bool QuestAccept_npc_stinky_ignatz(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_STINKYS_ESCAPE_ALLIANCE || pQuest->GetQuestId() == QUEST_ID_STINKYS_ESCAPE_HORDE)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

/*######
## at_sentry_point
######*/

enum SentryPoint
{
    QUEST_MISSING_DIPLO_PT14    = 1265,
    SPELL_TELEPORT_VISUAL_2     = 799,  // TODO Find the correct spell
    NPC_SENTRY_POINT_GUARD      = 5085
};

bool AreaTrigger_at_sentry_point(Player* pPlayer, const AreaTriggerEntry* /*pAt*/)
{
    QuestStatus quest_status = pPlayer->GetQuestStatus(QUEST_MISSING_DIPLO_PT14);
    if (pPlayer->IsDead() || quest_status == QUEST_STATUS_NONE || quest_status == QUEST_STATUS_COMPLETE)
        return false;

    if (!GetClosestCreatureWithEntry(pPlayer, NPC_TERVOSH, 100.0f))
    {
        if (Creature* pTervosh = pPlayer->SummonCreature(NPC_TERVOSH, -3476.51f, -4105.94f, 17.1f, 5.3816f, TEMPSPAWN_TIMED_DESPAWN, 60000))
        {
            pTervosh->CastSpell(pTervosh, SPELL_TELEPORT_VISUAL_2, TRIGGERED_OLD_TRIGGERED);

            if (Creature* pGuard = GetClosestCreatureWithEntry(pTervosh, NPC_SENTRY_POINT_GUARD, 15.0f))
            {
                pGuard->SetFacingToObject(pTervosh);
                pGuard->HandleEmote(EMOTE_ONESHOT_SALUTE);
            }
        }
    }

    return true;
};

/*######
## npc_theramore_combatant
######*/

enum
{
    NPC_GUARD_NARRISHA = 5093,
    NPC_GUARD_KAHIL = 5091,
    NPC_GUARD_EDWARD = 4922,
    NPC_GUARD_TARK = 5094,
    NPC_GUARD_LANA = 5092,
    NPC_GUARD_JARAD = 4923,

    NPC_MEDIC_HELAINA = 5200, //Red healer
    NPC_MEDIC_TAMBERLYN = 5199, //Blue healer

    NPC_MASTER_SZIGETI = 5090,
    NPC_MASTER_CRITON = 4924,

    NPC_BLUE_CAPTAIN = 5096,
    NPC_RED_CAPTAIN = 5095,

    SPELL_FIRST_AID = 7162,

    BLUE_TEAM = 1621,
    RED_TEAM = 1622,

    FACTION_THERAMORE_FRIENDLY = 151,

    SAY_RED_WINNER = 1794, //Szigeti
    SAY_BLUE_WINNER = 1795, //Szigeti
    SAY_RED_WINNER_OVERALL = 1773, //Criton
    SAY_BLUE_WINNER_OVERALL = 1774, //Criton

    SAY_BACK_TO_CORNERS = 1636, //Criton

    SAY_FOR_THE_RED = 1778,
    SAY_FOR_THE_BLUE = 1777,

    SAY_FIRST_COMBATANTS = 1771,
    SAY_FIGHT_INTRO = 1643,
    SAY_FIGHT_START = 1641,

    SAY_EDWARD_LOSS = 1760,
    SAY_KAHIL_LOSS = 1762,
    SAY_NARRISHA_LOSS = 1763,

    SAY_JARAD_LOSS = 1766,
    SAY_LANA_LOSS = 1767,
    SAY_TARK_LOSS = 1768,

    SAY_RED_MOTIVATION_WIN = 1769,
    SAY_RED_MOTIVATION_LOSS = 1770,

    SAY_BLUE_MOTIVATION_LOSS = 1765,
    SAY_BLUE_MOTIVATION_WIN = 1764,

    SAY_RED_FIRST = 1776,
    SAY_BLUE_FIRST = 1775,
};

enum CombatantActions
{
    COMBATANT_CHECK_HP,
    COMBATANT_BLUE_WIN,
    COMBATANT_RED_WIN,
    COMBATANT_START_POSITION,
    COMBATANT_START_SPAR,
    COMBATANT_GET_ALL,
    COMBATANT_SALUTE,
    COMBATANT_SWITCH,
    COMBATANT_SEPARATE,
    COMBATANT_INTRO,
    COMBATANT_MOTIVATION,
    COMBATANT_ACTION_MAX,
};

static std::unordered_map<uint32, const Position> startingPositions ({
    {BLUE_TEAM, {-3643.574707f, -4505.717773f, 9.462863f, 3.855592f}},
    {RED_TEAM, {-3652.405273f, -4514.126953f, 9.462863f, 0.717925f}}
});

static const uint32 theramore_blue_team[] = {NPC_GUARD_EDWARD, NPC_GUARD_KAHIL, NPC_GUARD_NARRISHA};
static const uint32 theramore_red_team[] = {NPC_GUARD_JARAD, NPC_GUARD_LANA, NPC_GUARD_TARK};

struct npc_theramore_spar_controller : public CombatAI
{
    npc_theramore_spar_controller(Creature* creature) : CombatAI(creature, COMBATANT_ACTION_MAX)
    {
        AddCustomAction(COMBATANT_CHECK_HP, true, [&](){ CheckCombatantHP(); });
        AddCustomAction(COMBATANT_BLUE_WIN, true, [&](){ HandleBlueWin(); });
        AddCustomAction(COMBATANT_RED_WIN, true, [&](){ HandleRedWin(); });
        AddCustomAction(COMBATANT_START_SPAR, true, [&](){ StartSparring(); });
        AddCustomAction(COMBATANT_START_POSITION, 10000u, [&](){ MoveIntoPosition(); });
        AddCustomAction(COMBATANT_SALUTE, true, [&](){ CombatantSalute(); });
        AddCustomAction(COMBATANT_INTRO, true, [&](){ CombatantIntro(); });
        AddCustomAction(COMBATANT_SWITCH, true, [&](){ SwitchMembers(); });
        AddCustomAction(COMBATANT_SEPARATE, true, [&](){ CombatantSeparate(); });
        AddCustomAction(COMBATANT_GET_ALL, 2000u, [&](){ GetCombatants(); });
        AddCustomAction(COMBATANT_MOTIVATION, true, [&](){ HandleMotivation(); });
    }

    std::unordered_map<uint32, GuidList> teams;
    std::unordered_map<uint32, ObjectGuid> medics;
    std::unordered_map<uint32, ObjectGuid> active;

    uint32 m_uiWinningTeam;

    void GetCombatants()
    {
        teams.clear();
        active.clear();
        for (uint32 red : theramore_red_team)
        {
            Creature* redMember = GetClosestCreatureWithEntry(m_creature, red, 20.f);
            if (redMember && redMember->IsAlive())
            {
                redMember->setFaction(FACTION_THERAMORE_FRIENDLY);
                teams[RED_TEAM].push_back(redMember->GetObjectGuid());
            }
        }

        for (uint32 blue : theramore_blue_team)
        {
            Creature* blueMember = GetClosestCreatureWithEntry(m_creature, blue, 20.f);
            if (blueMember && blueMember->IsAlive())
            {
                blueMember->setFaction(FACTION_THERAMORE_FRIENDLY);
                teams[BLUE_TEAM].push_back(blueMember->GetObjectGuid());
            }
        }

        if (Creature* redMedic = GetClosestCreatureWithEntry(m_creature, NPC_MEDIC_HELAINA, 20.f))
            medics[RED_TEAM] = redMedic->GetObjectGuid();
        if (Creature* blueMedic = GetClosestCreatureWithEntry(m_creature, NPC_MEDIC_TAMBERLYN, 20.f))
            medics[BLUE_TEAM] = blueMedic->GetObjectGuid();
    }

    void CheckCombatantHP()
    {
        Unit* redCombatant = m_creature->GetMap()->GetUnit(active[RED_TEAM]);
        Unit* blueCombatant = m_creature->GetMap()->GetUnit(active[BLUE_TEAM]);

        if (redCombatant && blueCombatant)
        {
            float redHealth = redCombatant->GetHealthPercent();
            float blueHealth = blueCombatant->GetHealthPercent();
            if (redHealth <= 30 || blueHealth <= 30)
            {
                ResetTimer(redHealth < blueHealth ? COMBATANT_BLUE_WIN : COMBATANT_RED_WIN, 100);

                redCombatant->RestoreOriginalFaction();
                blueCombatant->RestoreOriginalFaction();
                redCombatant->CombatStop(true);
                blueCombatant->CombatStop(true);
                
                Unit* redMedic = m_creature->GetMap()->GetUnit(medics[RED_TEAM]);
                Unit* blueMedic = m_creature->GetMap()->GetUnit(medics[BLUE_TEAM]);

                if (redMedic && redMedic->IsAlive() && redCombatant->IsAlive())
                    redMedic->AI()->DoCastSpellIfCan(redCombatant, SPELL_FIRST_AID);
                if (blueMedic && blueMedic->IsAlive() && blueCombatant->IsAlive())
                    blueMedic->AI()->DoCastSpellIfCan(blueCombatant, SPELL_FIRST_AID);
                DisableTimer(COMBATANT_CHECK_HP);
                return;
            }
        }
        ResetTimer(COMBATANT_CHECK_HP, 1000u);
    }

    void HandleBlueWin()
    {
        Unit* szigeti = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_SZIGETI, 15.f);
        Creature* redCaptain = GetClosestCreatureWithEntry(m_creature, NPC_RED_CAPTAIN, 20.f);
        if (szigeti && szigeti->IsAlive())
        {
            szigeti->HandleEmote(EMOTE_ONESHOT_POINT);
            DoBroadcastText(SAY_BLUE_WINNER, szigeti);
        }
        for (uint32 member : theramore_blue_team)
        {
            Unit* combatant = GetClosestCreatureWithEntry(m_creature, member, 20.f);
            if (combatant && combatant->GetEntry() != active[BLUE].GetEntry() && combatant->IsAlive())
            {
                combatant->HandleEmote(EMOTE_ONESHOT_CHEER);
            }
        }
        if (redCaptain && redCaptain->IsAlive())
        {
            switch (active[RED_TEAM].GetEntry())
            {
                case NPC_GUARD_JARAD:
                    DoBroadcastText(SAY_JARAD_LOSS, redCaptain);
                    break;
                case NPC_GUARD_LANA:
                    DoBroadcastText(SAY_LANA_LOSS, redCaptain);
                    break;
                case NPC_GUARD_TARK:
                    DoBroadcastText(SAY_TARK_LOSS, redCaptain);
                    break;
                default:
                    break;
            }
        }
        HandleWin(BLUE_TEAM);
        m_uiWinningTeam = BLUE_TEAM;
    }

    void HandleRedWin()
    {
        Unit* szigeti = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_SZIGETI, 15.f);
        Creature* blueCaptain = GetClosestCreatureWithEntry(m_creature, NPC_BLUE_CAPTAIN, 20.f);
        if (szigeti && szigeti->IsAlive())
        {
            szigeti->HandleEmote(EMOTE_ONESHOT_POINT);
            DoBroadcastText(SAY_RED_WINNER, szigeti);
        }
        for (uint32 member : theramore_red_team)
        {
            Unit* combatant = GetClosestCreatureWithEntry(m_creature, member, 20.f);
            if (combatant && combatant->GetEntry() != active[RED_TEAM].GetEntry() && combatant->IsAlive())
            {
                combatant->HandleEmote(EMOTE_ONESHOT_CHEER);
            }
        }
        if (blueCaptain && blueCaptain->IsAlive())
        {
            switch (active[BLUE_TEAM].GetEntry())
            {
                case NPC_GUARD_EDWARD:
                    DoBroadcastText(SAY_EDWARD_LOSS, blueCaptain);
                    break;
                case NPC_GUARD_KAHIL:
                    DoBroadcastText(SAY_KAHIL_LOSS, blueCaptain);
                    break;
                case NPC_GUARD_NARRISHA:
                    DoBroadcastText(SAY_NARRISHA_LOSS, blueCaptain);
                    break;
                default:
                    break;
            }
        }
        HandleWin(RED_TEAM);
        m_uiWinningTeam = RED_TEAM;
    }

    void HandleMotivation()
    {
        uint32 winningTeam = m_uiWinningTeam;
        Creature* redCaptain = GetClosestCreatureWithEntry(m_creature, NPC_RED_CAPTAIN, 20.f);
        Creature* blueCaptain = GetClosestCreatureWithEntry(m_creature, NPC_BLUE_CAPTAIN, 20.f);

        if (!redCaptain || !blueCaptain || !redCaptain->IsAlive() || !blueCaptain->IsAlive())
            return;

        switch (winningTeam)
        {
            case RED_TEAM:
                DoBroadcastText(SAY_RED_MOTIVATION_WIN, redCaptain);
                DoBroadcastText(SAY_BLUE_MOTIVATION_LOSS, blueCaptain);
                break;
            case BLUE_TEAM:
                DoBroadcastText(SAY_BLUE_MOTIVATION_WIN, blueCaptain);
                DoBroadcastText(SAY_RED_MOTIVATION_LOSS, redCaptain);
                break;
            default:
                break;
        }

    }

    void HandleWin(uint32 winningTeam)
    {
        Unit* criton = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_CRITON, 15.f);
        uint32 losingTeam = (winningTeam == BLUE_TEAM ? RED_TEAM : BLUE_TEAM);

        if (criton && criton->IsAlive())
        {
            DoBroadcastText(SAY_BACK_TO_CORNERS, criton);
            if (teams[losingTeam].empty())
            {
                DoBroadcastText(winningTeam == BLUE_TEAM ? SAY_BLUE_WINNER_OVERALL : SAY_RED_WINNER_OVERALL, criton);
                ResetTimer(COMBATANT_MOTIVATION, 8000u);
            }
        }
        ResetTimer(COMBATANT_SEPARATE, 100u);
    }

    void CombatantSalute()
    {
        Creature* redMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[RED_TEAM]));
        Creature* blueMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[BLUE_TEAM]));
        if (redMember && blueMember && redMember->IsAlive() && blueMember->IsAlive())
        {
            redMember->HandleEmote(EMOTE_ONESHOT_SALUTE);
            DoBroadcastText(SAY_FOR_THE_RED, redMember);
            blueMember->HandleEmote(EMOTE_ONESHOT_SALUTE);
            DoBroadcastText(SAY_FOR_THE_BLUE, blueMember);
        }
        ResetTimer(COMBATANT_INTRO, 3000u);
    }

    void CombatantIntro()
    {
        Unit* criton = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_CRITON, 15.f);
        if (criton && criton->IsAlive())
        {
            DoBroadcastText(SAY_FIGHT_INTRO, criton);
        }
        ResetTimer(COMBATANT_START_SPAR, 3000u);
    }

    void SwitchMembers()
    {
        uint32 winningTeam = m_uiWinningTeam;
        uint32 losingTeam = (winningTeam == BLUE_TEAM ? RED_TEAM : BLUE_TEAM);
        Creature* loser = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[losingTeam]));
        Creature* winner = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[winningTeam]));

        if (loser && loser->IsAlive())
        {
            Position respawnCoords = loser->GetRespawnPosition();
            loser->AI()->SetReactState(REACT_PASSIVE);
            loser->GetMotionMaster()->Clear(true, true);
            loser->GetMotionMaster()->MovePoint(0, respawnCoords, FORCED_MOVEMENT_RUN);
        }
        if (winner && winner->IsAlive())
        {
            winner->GetMotionMaster()->MovePoint(1, startingPositions[winningTeam], FORCED_MOVEMENT_WALK);
        }
        if (!teams[losingTeam].empty()){
            active[losingTeam] = teams[losingTeam].front();
            teams[losingTeam].pop_front();
            ResetTimer(COMBATANT_START_POSITION, 5000u);
        }
        else
        {
            if (winner && winner->IsAlive())
            {
                Position respawnCoords = winner->GetRespawnPosition();
                winner->AI()->SetReactState(REACT_PASSIVE);
                winner->GetMotionMaster()->Clear(true, true);
                winner->GetMotionMaster()->MovePoint(0, respawnCoords, FORCED_MOVEMENT_RUN);
            }
            GetCombatants();
            ResetTimer(COMBATANT_START_POSITION, 20000u);
        }
    }

    void CombatantSeparate()
    {
        Creature* redMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[RED_TEAM]));
        Creature* blueMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[BLUE_TEAM]));

        if (redMember && blueMember)
        {
            redMember->GetMotionMaster()->Clear(true, true);
            redMember->GetMotionMaster()->MovePoint(1, startingPositions[RED_TEAM], FORCED_MOVEMENT_RUN);
            redMember->AI()->SetReactState(REACT_PASSIVE);
            blueMember->GetMotionMaster()->Clear(true, true);
            blueMember->GetMotionMaster()->MovePoint(1, startingPositions[BLUE_TEAM], FORCED_MOVEMENT_RUN);
            blueMember->AI()->SetReactState(REACT_PASSIVE);
        }
        ResetTimer(COMBATANT_SWITCH, 5000u);
    }

    void StartSparring()
    {
        Creature* redMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[RED_TEAM]));
        Creature* blueMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[BLUE_TEAM]));

        Unit* criton = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_CRITON, 15.f);
        if (criton && criton->IsAlive())
        {
            DoBroadcastText(SAY_FIGHT_START, criton);
        }

        if (redMember && blueMember)
        {
            redMember->SetFactionTemporary(RED_TEAM, TEMPFACTION_RESTORE_COMBAT_STOP);
            redMember->AI()->SetReactState(REACT_DEFENSIVE);
            blueMember->SetFactionTemporary(BLUE_TEAM, TEMPFACTION_RESTORE_COMBAT_STOP);
            blueMember->AI()->SetReactState(REACT_DEFENSIVE);
            redMember->AI()->AttackStart(blueMember);
            blueMember->AI()->AttackStart(redMember);

            ResetTimer(COMBATANT_CHECK_HP, 1000u);
        }
    }

    void MoveIntoPosition()
    {
        if (active.empty() && !teams.empty())
        {
            Creature* redCaptain = GetClosestCreatureWithEntry(m_creature, NPC_RED_CAPTAIN, 20.f);
            Creature* blueCaptain = GetClosestCreatureWithEntry(m_creature, NPC_BLUE_CAPTAIN, 20.f);
            active[RED_TEAM] = teams[RED_TEAM].front();
            teams[RED_TEAM].pop_front();
            active[BLUE_TEAM] = teams[BLUE_TEAM].front();
            teams[BLUE_TEAM].pop_front();
            Unit* criton = GetClosestCreatureWithEntry(m_creature, NPC_MASTER_CRITON, 15.f);
            if (criton && criton->IsAlive())
            {
                DoBroadcastText(SAY_FIRST_COMBATANTS, criton);
            }
            if (redCaptain && redCaptain->IsAlive())
            {
                DoBroadcastText(SAY_RED_FIRST, redCaptain);
            }
            if (blueCaptain && blueCaptain->IsAlive())
            {
                DoBroadcastText(SAY_BLUE_FIRST, blueCaptain);
            }
        }
        Creature* redMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[RED_TEAM]));
        Creature* blueMember = dynamic_cast<Creature*>(m_creature->GetMap()->GetUnit(active[BLUE_TEAM]));

        if (redMember && blueMember)
        {
            redMember->GetMotionMaster()->Clear(true, true);
            redMember->GetMotionMaster()->MovePoint(1, startingPositions[RED_TEAM], FORCED_MOVEMENT_WALK);
            blueMember->GetMotionMaster()->Clear(true, true);
            blueMember->GetMotionMaster()->MovePoint(1, startingPositions[BLUE_TEAM], FORCED_MOVEMENT_WALK);
        }
        ResetTimer(COMBATANT_SALUTE, 5000u);
    }
};

void AddSC_dustwallow_marsh()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_morokk";
    pNewScript->GetAI = &GetAI_npc_morokk;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_morokk;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ogron";
    pNewScript->GetAI = &GetAI_npc_ogron;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_ogron;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_private_hendel";
    pNewScript->GetAI = &GetAI_npc_private_hendel;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_private_hendel;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_stinky_ignatz";
    pNewScript->GetAI = &GetAI_npc_stinky_ignatz;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_stinky_ignatz;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_sentry_point";
    pNewScript->pAreaTrigger = &AreaTrigger_at_sentry_point;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_theramore_spar_controller";
    pNewScript->GetAI = &GetNewAIInstance<npc_theramore_spar_controller>;
    pNewScript->RegisterSelf();
}
