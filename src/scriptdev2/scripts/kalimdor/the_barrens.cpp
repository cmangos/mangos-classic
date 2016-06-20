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
SDName: The_Barrens
SD%Complete: 90
SDComment: Quest support: 863, 898, 1719, 2458.
SDCategory: Barrens
EndScriptData */

/* ContentData
npc_gilthares
npc_taskmaster_fizzule
npc_twiggy_flathead
at_twiggy_flathead
npc_wizzlecrank_shredder
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
# npc_gilthares
######*/

enum
{
    SAY_GIL_START               = -1000370,
    SAY_GIL_AT_LAST             = -1000371,
    SAY_GIL_PROCEED             = -1000372,
    SAY_GIL_FREEBOOTERS         = -1000373,
    SAY_GIL_AGGRO_1             = -1000374,
    SAY_GIL_AGGRO_2             = -1000375,
    SAY_GIL_AGGRO_3             = -1000376,
    SAY_GIL_AGGRO_4             = -1000377,
    SAY_GIL_ALMOST              = -1000378,
    SAY_GIL_SWEET               = -1000379,
    SAY_GIL_FREED               = -1000380,

    QUEST_FREE_FROM_HOLD        = 898,
    AREA_MERCHANT_COAST         = 391
};

struct npc_giltharesAI : public npc_escortAI
{
    npc_giltharesAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void Reset() override { }

    void WaypointReached(uint32 uiPointId) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
            case 16:
                DoScriptText(SAY_GIL_AT_LAST, m_creature, pPlayer);
                break;
            case 17:
                DoScriptText(SAY_GIL_PROCEED, m_creature, pPlayer);
                break;
            case 18:
                DoScriptText(SAY_GIL_FREEBOOTERS, m_creature, pPlayer);
                break;
            case 37:
                DoScriptText(SAY_GIL_ALMOST, m_creature, pPlayer);
                break;
            case 47:
                DoScriptText(SAY_GIL_SWEET, m_creature, pPlayer);
                break;
            case 53:
                DoScriptText(SAY_GIL_FREED, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_FREE_FROM_HOLD, m_creature);
                break;
        }
    }

    void Aggro(Unit* pWho) override
    {
        // not always use
        if (urand(0, 3))
            return;

        // only aggro text if not player and only in this area
        if (pWho->GetTypeId() != TYPEID_PLAYER && m_creature->GetAreaId() == AREA_MERCHANT_COAST)
        {
            // appears to be pretty much random (possible only if escorter not in combat with pWho yet?)
            switch (urand(0, 3))
            {
                case 0: DoScriptText(SAY_GIL_AGGRO_1, m_creature, pWho); break;
                case 1: DoScriptText(SAY_GIL_AGGRO_2, m_creature, pWho); break;
                case 2: DoScriptText(SAY_GIL_AGGRO_3, m_creature, pWho); break;
                case 3: DoScriptText(SAY_GIL_AGGRO_4, m_creature, pWho); break;
            }
        }
    }
};

CreatureAI* GetAI_npc_gilthares(Creature* pCreature)
{
    return new npc_giltharesAI(pCreature);
}

bool QuestAccept_npc_gilthares(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_FREE_FROM_HOLD)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_H_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        DoScriptText(SAY_GIL_START, pCreature, pPlayer);

        if (npc_giltharesAI* pEscortAI = dynamic_cast<npc_giltharesAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## npc_taskmaster_fizzule
######*/

enum
{
    FACTION_FRIENDLY_F  = 35,
    SPELL_FLARE         = 10113,
    SPELL_FOLLY         = 10137,
};

struct npc_taskmaster_fizzuleAI : public ScriptedAI
{
    npc_taskmaster_fizzuleAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiResetTimer;
    uint8 m_uiFlareCount;

    void Reset() override
    {
        m_uiResetTimer = 0;
        m_uiFlareCount = 0;
    }

    void EnterEvadeMode() override
    {
        if (m_uiResetTimer)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->DeleteThreatList();
            m_creature->CombatStop(true);
            m_creature->LoadCreatureAddon(true);

            m_creature->SetLootRecipient(nullptr);

            m_creature->SetFactionTemporary(FACTION_FRIENDLY_F, TEMPFACTION_RESTORE_REACH_HOME);
            m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE);
        }
        else
            ScriptedAI::EnterEvadeMode();
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster->GetTypeId() == TYPEID_PLAYER && (pSpell->Id == SPELL_FLARE || pSpell->Id == SPELL_FOLLY))
        {
            ++m_uiFlareCount;

            if (m_uiFlareCount >= 2 && m_creature->getFaction() != FACTION_FRIENDLY_F)
                m_uiResetTimer = 120000;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetTimer)
        {
            if (m_uiResetTimer <= uiDiff)
            {
                m_uiResetTimer = 0;
                EnterEvadeMode();
            }
            else
                m_uiResetTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void ReceiveEmote(Player* /*pPlayer*/, uint32 uiTextEmote) override
    {
        if (uiTextEmote == TEXTEMOTE_SALUTE)
        {
            if (m_uiFlareCount >= 2 && m_creature->getFaction() != FACTION_FRIENDLY_F)
                EnterEvadeMode();
        }
    }
};

CreatureAI* GetAI_npc_taskmaster_fizzule(Creature* pCreature)
{
    return new npc_taskmaster_fizzuleAI(pCreature);
}

/*#####
## npc_twiggy_flathead
#####*/

enum
{
    SAY_BIG_WILL_READY                  = -1000123,
    SAY_TWIGGY_BEGIN                    = -1000124,
    SAY_TWIGGY_FRAY                     = -1000125,
    SAY_TWIGGY_DOWN                     = -1000126,
    SAY_TWIGGY_OVER                     = -1000127,

    NPC_TWIGGY                          = 6248,
    NPC_BIG_WILL                        = 6238,
    NPC_AFFRAY_CHALLENGER               = 6240,

    QUEST_AFFRAY                        = 1719,

    FACTION_FRIENDLY                    = 35,
    FACTION_HOSTILE_WILL                = 32,
    FACTION_HOSTILE_CHALLENGER          = 14,

    MAX_CHALLENGERS                     = 6,
};

static const float aAffrayChallengerLoc[8][4] =
{
    { -1683.0f, -4326.0f, 2.79f, 0.00f},
    { -1682.0f, -4329.0f, 2.79f, 0.00f},
    { -1683.0f, -4330.0f, 2.79f, 0.00f},
    { -1680.0f, -4334.0f, 2.79f, 1.49f},
    { -1674.0f, -4326.0f, 2.79f, 3.49f},
    { -1677.0f, -4334.0f, 2.79f, 1.66f},
    { -1713.79f, -4342.09f, 6.05f, 6.15f},          // Big Will spawn loc
    { -1682.31f, -4329.68f, 2.78f, 0.0f},           // Big Will move loc
};

struct npc_twiggy_flatheadAI : public ScriptedAI
{
    npc_twiggy_flatheadAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bIsEventInProgress;

    uint32 m_uiEventTimer;
    uint32 m_uiChallengerCount;
    uint8 m_uiStep;

    ObjectGuid m_playerGuid;
    ObjectGuid m_bigWillGuid;
    GuidVector m_vAffrayChallengerGuidsVector;

    void Reset() override
    {
        m_bIsEventInProgress = false;

        m_uiEventTimer = 1000;
        m_uiChallengerCount = 0;
        m_uiStep = 0;

        m_playerGuid.Clear();
        m_bigWillGuid.Clear();
        m_vAffrayChallengerGuidsVector.clear();
    }

    bool CanStartEvent(Player* pPlayer)
    {
        if (!m_bIsEventInProgress)
        {
            DoScriptText(SAY_TWIGGY_BEGIN, m_creature, pPlayer);
            m_playerGuid = pPlayer->GetObjectGuid();
            m_bIsEventInProgress = true;

            return true;
        }

        debug_log("SD2: npc_twiggy_flathead event already in progress, need to wait.");
        return false;
    }

    void SetChallengers()
    {
        m_vAffrayChallengerGuidsVector.reserve(MAX_CHALLENGERS);

        for (uint8 i = 0; i < MAX_CHALLENGERS; ++i)
            m_creature->SummonCreature(NPC_AFFRAY_CHALLENGER, aAffrayChallengerLoc[i][0], aAffrayChallengerLoc[i][1], aAffrayChallengerLoc[i][2], aAffrayChallengerLoc[i][3], TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
    }

    void SetChallengerReady(Creature* pChallenger)
    {
        pChallenger->setFaction(FACTION_HOSTILE_CHALLENGER);
        pChallenger->HandleEmote(EMOTE_ONESHOT_ROAR);
        ++m_uiChallengerCount;

        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
            pChallenger->AI()->AttackStart(pPlayer);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_BIG_WILL)
        {
            m_bigWillGuid = pSummoned->GetObjectGuid();
            pSummoned->setFaction(FACTION_FRIENDLY);
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(1, aAffrayChallengerLoc[7][0], aAffrayChallengerLoc[7][1], aAffrayChallengerLoc[7][2]);
        }
        else
        {
            pSummoned->setFaction(FACTION_FRIENDLY);
            pSummoned->HandleEmote(EMOTE_ONESHOT_ROAR);
            m_vAffrayChallengerGuidsVector.push_back(pSummoned->GetObjectGuid());
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMoveType, uint32 uiPointId) override
    {
        if (uiMoveType != POINT_MOTION_TYPE || !uiPointId || pSummoned->GetEntry() != NPC_BIG_WILL)
            return;

        pSummoned->setFaction(FACTION_HOSTILE_WILL);

        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
        {
            DoScriptText(SAY_BIG_WILL_READY, pSummoned, pPlayer);
            pSummoned->SetFacingToObject(pPlayer);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_BIG_WILL)
        {
            DoScriptText(SAY_TWIGGY_OVER, m_creature);
            EnterEvadeMode();
        }
        else
        {
            DoScriptText(SAY_TWIGGY_DOWN, m_creature);
            m_uiEventTimer = 15000;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_bIsEventInProgress)
            return;

        if (m_uiEventTimer < uiDiff)
        {
            Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

            if (!pPlayer || !pPlayer->isAlive())
                EnterEvadeMode();

            switch (m_uiStep)
            {
                case 0:
                    SetChallengers();
                    m_uiEventTimer = 5000;
                    ++m_uiStep;
                    break;
                case 1:
                    DoScriptText(SAY_TWIGGY_FRAY, m_creature);
                    if (Creature* pChallenger = m_creature->GetMap()->GetCreature(m_vAffrayChallengerGuidsVector[m_uiChallengerCount]))
                        SetChallengerReady(pChallenger);
                    else
                        EnterEvadeMode();

                    if (m_uiChallengerCount == MAX_CHALLENGERS)
                    {
                        ++m_uiStep;
                        m_uiEventTimer = 5000;
                    }
                    else
                        m_uiEventTimer = 25000;
                    break;
                case 2:
                    m_creature->SummonCreature(NPC_BIG_WILL, aAffrayChallengerLoc[6][0], aAffrayChallengerLoc[6][1], aAffrayChallengerLoc[6][2], aAffrayChallengerLoc[6][3], TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
                    m_uiEventTimer = 15000;
                    ++m_uiStep;
                    break;
                default:
                    m_uiEventTimer = 5000;
                    break;
            }
        }
        else
            m_uiEventTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_twiggy_flathead(Creature* pCreature)
{
    return new npc_twiggy_flatheadAI(pCreature);
}

bool AreaTrigger_at_twiggy_flathead(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    if (pPlayer->isAlive() && !pPlayer->isGameMaster() && pPlayer->GetQuestStatus(QUEST_AFFRAY) == QUEST_STATUS_INCOMPLETE)
    {
        Creature* pCreature = GetClosestCreatureWithEntry(pPlayer, NPC_TWIGGY, 30.0f);
        if (!pCreature)
            return true;

        if (npc_twiggy_flatheadAI* pTwiggyAI = dynamic_cast<npc_twiggy_flatheadAI*>(pCreature->AI()))
        {
            if (pTwiggyAI->CanStartEvent(pPlayer))
                return false;                               // ok to let mangos process further
        }

        return true;
    }
    return true;
}

/*#####
## npc_wizzlecranks_shredder
#####*/

enum
{
    SAY_START           = -1000298,
    SAY_STARTUP1        = -1000299,
    SAY_STARTUP2        = -1000300,
    SAY_MERCENARY       = -1000301,
    SAY_PROGRESS_1      = -1000302,
    SAY_PROGRESS_2      = -1000303,
    SAY_PROGRESS_3      = -1000304,
    SAY_END             = -1000305,

    QUEST_ESCAPE        = 863,
    FACTION_RATCHET     = 637,
    NPC_PILOT_WIZZ      = 3451,
    NPC_MERCENARY       = 3282
};

struct npc_wizzlecranks_shredderAI : public npc_escortAI
{
    npc_wizzlecranks_shredderAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_bIsPostEvent = false;
        m_uiPostEventTimer = 1000;
        m_uiPostEventCount = 0;
        Reset();
    }

    bool m_bIsPostEvent;
    uint32 m_uiPostEventTimer;
    uint32 m_uiPostEventCount;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (m_creature->getStandState() == UNIT_STAND_STATE_DEAD)
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);

            m_bIsPostEvent = false;
            m_uiPostEventTimer = 1000;
            m_uiPostEventCount = 0;
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 0:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_STARTUP1, m_creature, pPlayer);
                break;
            case 9:
                SetRun(false);
                break;
            case 17:
                if (Creature* pTemp = m_creature->SummonCreature(NPC_MERCENARY, 1128.489f, -3037.611f, 92.701f, 1.472f, TEMPSUMMON_TIMED_OOC_DESPAWN, 120000))
                {
                    DoScriptText(SAY_MERCENARY, pTemp);
                    m_creature->SummonCreature(NPC_MERCENARY, 1160.172f, -2980.168f, 97.313f, 3.690f, TEMPSUMMON_TIMED_OOC_DESPAWN, 120000);
                }
                break;
            case 24:
                m_bIsPostEvent = true;
                break;
        }
    }

    void WaypointStart(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 9:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_STARTUP2, m_creature, pPlayer);
                break;
            case 18:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_PROGRESS_1, m_creature, pPlayer);
                SetRun();
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_PILOT_WIZZ)
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

        if (pSummoned->GetEntry() == NPC_MERCENARY)
            pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (m_bIsPostEvent)
            {
                if (m_uiPostEventTimer < uiDiff)
                {
                    switch (m_uiPostEventCount)
                    {
                        case 0:
                            DoScriptText(SAY_PROGRESS_2, m_creature);
                            break;
                        case 1:
                            DoScriptText(SAY_PROGRESS_3, m_creature);
                            break;
                        case 2:
                            DoScriptText(SAY_END, m_creature);
                            break;
                        case 3:
                            if (Player* pPlayer = GetPlayerForEscort())
                            {
                                pPlayer->GroupEventHappens(QUEST_ESCAPE, m_creature);
                                m_creature->SummonCreature(NPC_PILOT_WIZZ, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
                            }
                            break;
                    }

                    ++m_uiPostEventCount;
                    m_uiPostEventTimer = 5000;
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_wizzlecranks_shredder(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ESCAPE)
    {
        DoScriptText(SAY_START, pCreature);
        pCreature->SetFactionTemporary(FACTION_RATCHET, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_wizzlecranks_shredderAI* pEscortAI = dynamic_cast<npc_wizzlecranks_shredderAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_wizzlecranks_shredder(Creature* pCreature)
{
    return new npc_wizzlecranks_shredderAI(pCreature);
}

void AddSC_the_barrens()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_gilthares";
    pNewScript->GetAI = &GetAI_npc_gilthares;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_gilthares;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_taskmaster_fizzule";
    pNewScript->GetAI = &GetAI_npc_taskmaster_fizzule;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_twiggy_flathead";
    pNewScript->GetAI = &GetAI_npc_twiggy_flathead;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_twiggy_flathead";
    pNewScript->pAreaTrigger = &AreaTrigger_at_twiggy_flathead;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_wizzlecranks_shredder";
    pNewScript->GetAI = &GetAI_npc_wizzlecranks_shredder;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_wizzlecranks_shredder;
    pNewScript->RegisterSelf();
}
