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
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
npc_gilthares
npc_taskmaster_fizzule
npc_twiggy_flathead
at_twiggy_flathead
npc_wizzlecrank_shredder
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

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

    void JustReachedHome() override 
    {
        npc_escortAI::JustReachedHome();
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

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
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_FREE_FROM_HOLD, m_creature);
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

UnitAI* GetAI_npc_gilthares(Creature* pCreature)
{
    return new npc_giltharesAI(pCreature);
}

bool QuestAccept_npc_gilthares(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_FREE_FROM_HOLD)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_H_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
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

    FIZZULE_FLARE_1             = 1,
    FIZZULE_FLARE_2             = 2,
    FIZZULE_SALUTE              = 3,
    FIZZULE_WHISTLE             = 4,
};

struct npc_taskmaster_fizzuleAI : public ScriptedAI
{
    npc_taskmaster_fizzuleAI(Creature* creature) : ScriptedAI(creature)
    { 
        resetTimer = 0;
        flareCount = 0;
    }

    uint32 resetTimer;
    uint8 flareCount;

    void Reset() override {}

    void EnterEvadeMode() override
    {
        if (resetTimer)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
        }
        else
            ScriptedAI::EnterEvadeMode();
    }

    void SpellHit(Unit* caster, const SpellEntry* spell) override
    {
        if (caster->GetTypeId() != TYPEID_PLAYER)
            return;

        switch (spell->Id)
        {
            case SPELL_FLARE:
            {
                if (flareCount < 2)
                {
                    ++flareCount;
                    HandleFlareOrKneel();
                }
                break;
            }
            case SPELL_FOLLY:
            {
                flareCount = 4;
                HandleFlareOrKneel();
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (resetTimer)
        {
            if (resetTimer <= diff)
            {
                resetTimer = 0;
                flareCount = 0;
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                m_creature->SetImmuneToPlayer(false);
                EnterEvadeMode();
            }
            else
                resetTimer -= diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void HandleFlareOrKneel()
    {
        switch (flareCount)
        {
            case FIZZULE_FLARE_1:
                resetTimer = 120000;
                m_creature->SetImmuneToPlayer(true);
                break;
            case FIZZULE_FLARE_2:
                m_creature->SetFactionTemporary(FACTION_FRIENDLY_F, TEMPFACTION_RESTORE_REACH_HOME);
                break;
            case FIZZULE_SALUTE:
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE);
                break;
            case FIZZULE_WHISTLE:
                resetTimer = 120000;
                m_creature->SetImmuneToPlayer(true);
                m_creature->SetFactionTemporary(FACTION_FRIENDLY_F, TEMPFACTION_RESTORE_REACH_HOME);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE);
                break;
        }
    }

    void ReceiveEmote(Player* /*player*/, uint32 textEmote) override
    {
        if (textEmote == TEXTEMOTE_SALUTE)
        {
            if (flareCount == 2)
            {
                flareCount++;
                HandleFlareOrKneel();
            }
        }
    }
};

UnitAI* GetAI_npc_taskmaster_fizzule(Creature* creature)
{
    return new npc_taskmaster_fizzuleAI(creature);
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

    void FailEvent()
    {
        if (Creature* bigWill = m_creature->GetMap()->GetCreature(m_bigWillGuid))
            if (bigWill->isAlive())
                bigWill->ForcedDespawn();

        for (ObjectGuid guid : m_vAffrayChallengerGuidsVector)
            if (Creature* creature = m_creature->GetMap()->GetCreature(guid))
                if (creature->isAlive())
                    creature->ForcedDespawn();

        Reset();
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
            m_creature->SummonCreature(NPC_AFFRAY_CHALLENGER, aAffrayChallengerLoc[i][0], aAffrayChallengerLoc[i][1], aAffrayChallengerLoc[i][2], aAffrayChallengerLoc[i][3], TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
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
            Reset();
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
            {
                FailEvent();
                return;
            }

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
                    {
                        FailEvent(); // this should never happen
                        return;
                    }

                    if (m_uiChallengerCount == MAX_CHALLENGERS)
                    {
                        ++m_uiStep;
                        m_uiEventTimer = 5000;
                    }
                    else
                        m_uiEventTimer = 25000;
                    break;
                case 2:
                    m_creature->SummonCreature(NPC_BIG_WILL, aAffrayChallengerLoc[6][0], aAffrayChallengerLoc[6][1], aAffrayChallengerLoc[6][2], aAffrayChallengerLoc[6][3], TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
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

UnitAI* GetAI_npc_twiggy_flathead(Creature* pCreature)
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
                if (Creature* pTemp = m_creature->SummonCreature(NPC_MERCENARY, 1128.489f, -3037.611f, 92.701f, 1.472f, TEMPSPAWN_TIMED_OOC_DESPAWN, 120000))
                {
                    DoScriptText(SAY_MERCENARY, pTemp);
                    m_creature->SummonCreature(NPC_MERCENARY, 1160.172f, -2980.168f, 97.313f, 3.690f, TEMPSPAWN_TIMED_OOC_DESPAWN, 120000);
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
                                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ESCAPE, m_creature);
                                m_creature->SummonCreature(NPC_PILOT_WIZZ, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 180000);
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

UnitAI* GetAI_npc_wizzlecranks_shredder(Creature* pCreature)
{
    return new npc_wizzlecranks_shredderAI(pCreature);
}

struct npc_gallywixAI : public ScriptedAI
{
    npc_gallywixAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset() override {}

    void DamageTaken(Unit* /*pDealer*/, uint32& uiDamage, DamageEffectType /*damagetype*/, SpellEntry const* spellInfo) override
    {
        if (spellInfo && spellInfo->IsFitToFamilyMask(0x0000000000800200)) // on Ambush
            uiDamage = (m_creature->GetHealth() * 0.5); // Ambush should do 50% health in damage to this creature
    }
};

UnitAI* GetAI_npc_gallywix(Creature* pCreature)
{
    return new npc_gallywixAI(pCreature);
}

/*#####
## npc_regthar_deathgate
#####*/

enum
{
    QUEST_COUNTERATTACK        = 4021,

    SAY_START_REGTHAR          = -1000892,
    SAY_DEFENDER               = -1000893,
    YELL_RETREAT               = -1000894,

    NPC_REGTHAR_DEATHGATE      = 3389,
    NPC_WARLORD_KROMZAR        = 9456,
    NPC_HORDE_DEFENDER         = 9457,
    NPC_HORDE_AXE_THROWER      = 9458,
    NPC_KOLKAR_INVADER         = 9524,
    NPC_KOLKAR_STORMSEER       = 9523,

    KILL_LIMIT                 = 20
};

// respawn time in seconds
#define COUNTERATTACK_RESPAWN_TIME_HORDE      12 // normal respawn time
#define COUNTERATTACK_RESPAWN_TIME_HORDE_INC  1  // increased respawn time to have enough defender everytime
#define COUNTERATTACK_RESPAWN_TIME_KOLKAR     10

struct SpawnPoint
{
    float fX, fY, fZ, fO;
};

typedef std::vector<SpawnPoint> SpawnPointVector;

static const SpawnPointVector spawnPointsHorde
{
    { -307.23f, -1919.84f, 91.66f, 0.64f},
    { -281.15f, -1906.39f, 91.66f, 1.88f},
    { -270.09f, -1901.58f, 91.66f, 1.88f},
    { -275.24f, -1901.96f, 91.66f, 1.90f},
    { -295.84f, -1913.58f, 91.66f, 1.86f},
    { -221.61f, -1936.54f, 94.00f, 0.41f},
    { -226.65f, -1927.87f, 93.24f, 0.41f},
};

static const SpawnPointVector spawnPointsKromzar
{
    { -281.19f, -1855.54f, 92.58f, 4.85f},
    { -283.66f, -1858.45f, 92.47f, 4.85f},
    { -286.50f, -1856.18f, 92.44f, 4.85f}
};

static const SpawnPointVector spawnPointsKolkar
{
    { -290.26f, -1860.85f, 92.48f, 3.68f},
    { -311.46f, -1871.16f, 92.64f, 6.06f},
    { -321.75f, -1868.48f, 93.73f, 0.30f},
    { -338.12f, -1852.10f, 94.09f, 0.01f},
    { -311.46f, -1847.78f, 94.93f, 6.15f},
    { -292.02f, -1840.00f, 93.06f, 3.51f},
    { -267.14f, -1853.97f, 93.24f, 3.52f},
    { -267.68f, -1832.77f, 92.69f, 3.52f},
    { -279.70f, -1845.67f, 92.81f, 3.83f},
    { -338.99f, -1868.68f, 93.50f, 0.17f},
    { -210.28f, -1916.22f, 92.87f, 4.89f},
    { -209.09f, -1922.44f, 93.26f, 3.87f},
    { -206.64f, -1924.83f, 93.78f, 2.41f},
    { -283.57f, -1883.91f, 92.60f, 3.53f},
    { -197.50f, -1929.31f, 94.03f, 3.51f}
};

struct npc_regthar_deathgateAI : public ScriptedAI
{
private:
    struct KilledSlot
    {
        KilledSlot(ObjectGuid guid, TimePoint& rTime) : KilledGuid(guid), respawnTime(rTime) {}
        ObjectGuid KilledGuid;
        TimePoint respawnTime;
    };

    typedef std::map<ObjectGuid, uint32> SpawnPointIndexMap;
    typedef std::list<KilledSlot> FullGuidList;

    // keep track of spawned creature by saving their guid and assigned slot
    SpawnPointIndexMap m_hordeCreatureMap;
    SpawnPointIndexMap m_kolkarCreatureMap;

    // keep track of killed creature by saving their respawntime those lists are sorted, the first one should be respawned first
    FullGuidList m_killedHordeGuid;
    FullGuidList m_killedKolkarGuid;

    uint64 m_eventTimer;
    uint32 m_waitSummonTimerKolkar;
    uint32 m_waitSummonTimerHorde;
    uint32 m_kolkarKillCount;
    bool   m_eventStarted;
    bool   m_isKromzarSpawned;

    // Spawn one creature if there is at least one killed in killedGuidList (DB waypoint for all spawnpoint index less or equal given moveToWP argument)
    bool SpawnCreature(SpawnPointVector const& spawnPoints, SpawnPointIndexMap& creatureSlotMap, FullGuidList& killedGuidList, uint32 entry1, uint32 entry2, int32 moveToWP = -1)
    {
        if (killedGuidList.empty())
            return false;

        bool result = false;
        KilledSlot const& spawnSlot = killedGuidList.front();
        auto itr = creatureSlotMap.find(spawnSlot.KilledGuid);
        if (itr == creatureSlotMap.end())
        {
            error_log("SD2::npc_regthar_deathgateAI::SpawnCreature> Invalid GUID was present in guidList container!");
            killedGuidList.erase(killedGuidList.begin());
            return false;
        }

        if (spawnSlot.respawnTime < m_creature->GetMap()->GetCurrentClockTime())
        {
            killedGuidList.erase(killedGuidList.begin());
            uint32 summonEntry = urand(0, 1) ? entry1 : entry2;
            int32 spawnPointIdx = int32(itr->second);
            SpawnPoint const& spawnPoint = spawnPoints[spawnPointIdx];
            if (Creature * creature = m_creature->SummonCreature(summonEntry, spawnPoint.fX, spawnPoint.fY, spawnPoint.fZ, spawnPoint.fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 40 * IN_MILLISECONDS))
            {
                // Force Horde NPCs spawned in main point to move towards kolkar invaders. We give them WP movement to ensure they always move there event after evade/reset. WP are handled in DB
                if (spawnPointIdx < moveToWP)
                    creature->GetMotionMaster()->MoveWaypoint();

                // update creature map
                creatureSlotMap[creature->GetObjectGuid()] = spawnPointIdx;
                creatureSlotMap.erase(itr);
                result = true;
            }
        }
        return result;
    }

    // spawn all slot with given entry1/entry2 randomly and store them to SpawnPointIndexMap storage (moveTOWP is max slot index that should use DB waypoint)
    void SpawnAllSlot(SpawnPointVector const& spawnPoints, SpawnPointIndexMap& creatureSlotMap, uint32 entry1, uint32 entry2, int32 moveToWP = -1)
    {
        int32 counter = 0;
        for (auto& spawnPoint : spawnPoints)
        {
            uint32 summonEntry = urand(0, 1) ? entry1 : entry2;
            if (Creature * creature = m_creature->SummonCreature(summonEntry, spawnPoint.fX, spawnPoint.fY, spawnPoint.fZ, spawnPoint.fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 40 * IN_MILLISECONDS))
            {
                if (counter < moveToWP)
                {
                    // Force Horde NPCs spawned in main point to move towards kolkar invaders. We give them WP movement to ensure they always move there event after evade/reset. WP are handled in DB
                    creature->GetMotionMaster()->MoveWaypoint();
                }

                creatureSlotMap[creature->GetObjectGuid()] = uint32(counter);
                counter++;
            }
        }
    }

    // despawn all creature in provided SpawnPointIndexMap if they are still exist
    void ForceDespawn(SpawnPointIndexMap& spawnPoints)
    {
        for (auto spawnItr : spawnPoints)
        {
            if (Creature * creature = m_creature->GetMap()->GetCreature(spawnItr.first))
            {
                if (creature->isInCombat())
                {
                    // Give one minute to the NPCs still fighting before despawning them
                    creature->ForcedDespawn(MINUTE * IN_MILLISECONDS);
                }
                else
                    creature->ForcedDespawn();
            }
        }
    }

public:
    npc_regthar_deathgateAI(Creature* creature) : ScriptedAI(creature) {Reset();}

    void Reset()
    {
        m_waitSummonTimerKolkar = 0;
        m_waitSummonTimerHorde  = 0;
        m_kolkarKillCount       = 0;
        m_eventTimer            = 0;
        m_isKromzarSpawned      = false;
        m_eventStarted          = false;
        m_killedKolkarGuid.clear();
        m_killedHordeGuid.clear();
        m_hordeCreatureMap.clear();
        m_kolkarCreatureMap.clear();
    }

    void JustRespawned()
    {
        FinishEvent();
        Reset();
    }

    void StartEvent()
    {
        if(m_eventStarted)
            return;

        m_eventStarted         = true;
        m_eventTimer           = 20 * MINUTE * IN_MILLISECONDS;
 
        // Initial summon of NPCs, they will be resummon when killed based on their spawn point
        SpawnAllSlot(spawnPointsHorde, m_hordeCreatureMap, NPC_HORDE_DEFENDER, NPC_HORDE_AXE_THROWER, 5);
        SpawnAllSlot(spawnPointsKolkar, m_kolkarCreatureMap, NPC_KOLKAR_INVADER, NPC_KOLKAR_STORMSEER);
    }

    void FinishEvent()
    {
        // despawn all remaining creature
        DoScriptText(YELL_RETREAT, m_creature);
        ForceDespawn(m_hordeCreatureMap);
        ForceDespawn(m_kolkarCreatureMap);

        m_eventStarted = false;
        m_kolkarKillCount = 0;
    }

    void SummonedCreatureJustDied(Creature* killed)
    {
        TimePoint respTime = m_creature->GetMap()->GetCurrentClockTime();
        switch (killed->GetEntry())
        {
            case NPC_HORDE_DEFENDER:
            case NPC_HORDE_AXE_THROWER:
            {
                // handle respawn time
                if (m_killedHordeGuid.size() > 3)
                {
                    // speed up summon if too many defenders are dead
                    respTime += std::chrono::seconds(COUNTERATTACK_RESPAWN_TIME_HORDE_INC);
                }
                else
                    respTime += std::chrono::seconds(COUNTERATTACK_RESPAWN_TIME_HORDE);

                // save the killed guid and respawn time for later use in SpawnCreature
                m_killedHordeGuid.emplace_back(killed->GetObjectGuid(), respTime);

                if (urand(0,1))
                    DoScriptText(SAY_DEFENDER, m_creature);
                break;
            }

            case NPC_KOLKAR_INVADER:
            case NPC_KOLKAR_STORMSEER:
            {
                // handle respawn time
                respTime += std::chrono::seconds(COUNTERATTACK_RESPAWN_TIME_KOLKAR);

                // save the killed guid and respawn time for later use in SpawnCreature
                m_killedKolkarGuid.emplace_back(killed->GetObjectGuid(), respTime);

                // increment killed creature count
                ++m_kolkarKillCount;
                break;
            }

            case NPC_WARLORD_KROMZAR:
            {
                FinishEvent();
                break;
            }

            default:
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_eventStarted)
            return;

        if (!m_isKromzarSpawned && m_eventTimer < uiDiff)
            FinishEvent();

        if (!m_isKromzarSpawned)
        {
            if (m_kolkarKillCount >= KILL_LIMIT)  // Kill count reached: spawn Warlord Kromzar and his bodyguards
            {
                m_creature->SummonCreature(NPC_KOLKAR_INVADER,  spawnPointsKromzar[0].fX, spawnPointsKromzar[0].fY, spawnPointsKromzar[0].fZ, spawnPointsKromzar[0].fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 40 * IN_MILLISECONDS);
                m_creature->SummonCreature(NPC_WARLORD_KROMZAR,  spawnPointsKromzar[1].fX, spawnPointsKromzar[1].fY, spawnPointsKromzar[1].fZ, spawnPointsKromzar[1].fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 40 * IN_MILLISECONDS);
                m_creature->SummonCreature(NPC_KOLKAR_STORMSEER,  spawnPointsKromzar[2].fX, spawnPointsKromzar[2].fY, spawnPointsKromzar[2].fZ, spawnPointsKromzar[2].fO, TEMPSPAWN_CORPSE_TIMED_DESPAWN, 40 * IN_MILLISECONDS);
                m_isKromzarSpawned = true;
                return;
            }

            // Summon Horde defender if need
            SpawnCreature(spawnPointsHorde, m_hordeCreatureMap, m_killedHordeGuid, NPC_HORDE_DEFENDER, NPC_HORDE_AXE_THROWER, 5);

            // Summon kolkar if need
            SpawnCreature(spawnPointsKolkar, m_kolkarCreatureMap, m_killedKolkarGuid, NPC_KOLKAR_STORMSEER, NPC_KOLKAR_INVADER);
        }
        else    // Nothing to spawn: update all timers
            m_eventTimer -= uiDiff;
    }
};

UnitAI* GetAI_npc_regthar_deathgate(Creature* creature)
{
    return new npc_regthar_deathgateAI(creature);
}

bool GossipHello_npc_regthar_deathgate(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetObjectGuid());

    if (player->GetQuestStatus(QUEST_COUNTERATTACK) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Where is Warlord Krom'zar?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(2533, creature->GetObjectGuid());
        return true;
    }
    else
        player->SEND_GOSSIP_MENU(2533, creature->GetObjectGuid());
    return true;
}

bool QuestAccept_npc_regthar_deathgate(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_COUNTERATTACK)
    {
        DoScriptText(SAY_START_REGTHAR, creature, player);
        player->CLOSE_GOSSIP_MENU();
        if (npc_regthar_deathgateAI* regtharAI = dynamic_cast<npc_regthar_deathgateAI*>(creature->AI()))
            regtharAI->StartEvent();
    }
    return true;
}

bool GossipSelect_npc_regthar_deathgate(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        DoScriptText(SAY_START_REGTHAR, creature, player);
        player->CLOSE_GOSSIP_MENU();
        if (npc_regthar_deathgateAI* regtharAI = dynamic_cast<npc_regthar_deathgateAI*>(creature->AI()))
            regtharAI->StartEvent();
    }
    return true;
}

void AddSC_the_barrens()
{
    Script* pNewScript = new Script;
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

    pNewScript = new Script;
    pNewScript->Name = "npc_gallywix";
    pNewScript->GetAI = &GetAI_npc_gallywix;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_regthar_deathgate";
    pNewScript->pGossipHello = &GossipHello_npc_regthar_deathgate;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_regthar_deathgate;
    pNewScript->pGossipSelect = &GossipSelect_npc_regthar_deathgate;
    pNewScript->GetAI = &GetAI_npc_regthar_deathgate;
    pNewScript->RegisterSelf();
}
