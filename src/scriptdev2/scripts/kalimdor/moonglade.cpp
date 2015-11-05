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
SDName: Moonglade
SD%Complete: 100
SDComment: Quest support: 8736, 10965.
SDCategory: Moonglade
EndScriptData */

/* ContentData
npc_keeper_remulos
boss_eranikus
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "ObjectMgr.h"

/*######
## npc_keeper_remulos
######*/

enum
{
    SPELL_CONJURE_RIFT          = 25813,            // summon Eranikus
    SPELL_HEALING_TOUCH         = 23381,
    SPELL_REGROWTH              = 20665,
    SPELL_REJUVENATION          = 20664,
    SPELL_STARFIRE              = 21668,
    SPELL_ERANIKUS_REDEEMED     = 25846,            // transform Eranikus
    // SPELL_MOONGLADE_TRANQUILITY = unk,            // spell which acts as a spotlight over Eranikus after he is redeemed

    NPC_ERANIKUS_TYRANT         = 15491,
    NPC_NIGHTMARE_PHANTASM      = 15629,            // shadows summoned during the event - should cast 17228 and 21307
    NPC_REMULOS                 = 11832,
    NPC_TYRANDE_WHISPERWIND     = 15633,            // appears with the priestess during the event to help the players - should cast healing spells
    NPC_ELUNE_PRIESTESS         = 15634,

    QUEST_NIGHTMARE_MANIFESTS   = 8736,

    // yells -> in cronological order
    SAY_REMULOS_INTRO_1         = -1000669,        // remulos intro
    SAY_REMULOS_INTRO_2         = -1000670,
    SAY_REMULOS_INTRO_3         = -1000671,
    SAY_REMULOS_INTRO_4         = -1000672,
    SAY_REMULOS_INTRO_5         = -1000673,

    EMOTE_SUMMON_ERANIKUS       = -1000674,        // eranikus spawn - world emote
    SAY_ERANIKUS_SPAWN          = -1000675,

    SAY_REMULOS_TAUNT_1         = -1000676,        // eranikus and remulos chat
    EMOTE_ERANIKUS_LAUGH        = -1000677,
    SAY_ERANIKUS_TAUNT_2        = -1000678,
    SAY_REMULOS_TAUNT_3         = -1000679,
    SAY_ERANIKUS_TAUNT_4        = -1000680,

    EMOTE_ERANIKUS_ATTACK       = -1000681,        // start attack
    SAY_REMULOS_DEFEND_1        = -1000682,
    SAY_REMULOS_DEFEND_2        = -1000683,
    SAY_ERANIKUS_SHADOWS        = -1000684,
    SAY_REMULOS_DEFEND_3        = -1000685,
    SAY_ERANIKUS_ATTACK_1       = -1000686,
    SAY_ERANIKUS_ATTACK_2       = -1000687,
    SAY_ERANIKUS_ATTACK_3       = -1000688,
    SAY_ERANIKUS_KILL           = -1000706,

    SAY_TYRANDE_APPEAR          = -1000689,        // Tyrande appears
    SAY_TYRANDE_HEAL            = -1000690,        // yelled by tyrande when healing is needed
    SAY_TYRANDE_FORGIVEN_1      = -1000691,
    SAY_TYRANDE_FORGIVEN_2      = -1000692,
    SAY_TYRANDE_FORGIVEN_3      = -1000693,
    SAY_ERANIKUS_DEFEAT_1       = -1000694,
    SAY_ERANIKUS_DEFEAT_2       = -1000695,
    SAY_ERANIKUS_DEFEAT_3       = -1000696,
    EMOTE_ERANIKUS_REDEEM       = -1000697,        // world emote before WotLK

    EMOTE_TYRANDE_KNEEL         = -1000698,
    SAY_TYRANDE_REDEEMED        = -1000699,

    SAY_REDEEMED_1              = -1000700,        // eranikus redeemed
    SAY_REDEEMED_2              = -1000701,
    SAY_REDEEMED_3              = -1000702,
    SAY_REDEEMED_4              = -1000703,

    SAY_REMULOS_OUTRO_1         = -1000704,        // remulos outro
    SAY_REMULOS_OUTRO_2         = -1000705,

    POINT_ID_ERANIKUS_FLIGHT    = 0,
    POINT_ID_ERANIKUS_COMBAT    = 1,
    POINT_ID_ERANIKUS_REDEEMED  = 2,

    MAX_SHADOWS                 = 3,                // the max shadows summoned per turn
    MAX_SUMMON_TURNS            = 10,               // There are about 10 summoned shade waves
};

static const DialogueEntry aIntroDialogue[] =
{
    {NPC_REMULOS,           0,                   14000},    // target player
    {SAY_REMULOS_INTRO_4,   NPC_REMULOS,         12000},
    {SAY_REMULOS_INTRO_5,   NPC_REMULOS,         5000},
    {SPELL_CONJURE_RIFT,    0,                   13000},    // conjure rift spell
    {SAY_ERANIKUS_SPAWN,    NPC_ERANIKUS_TYRANT, 11000},
    {SAY_REMULOS_TAUNT_1,   NPC_REMULOS,         5000},
    {EMOTE_ERANIKUS_LAUGH,  NPC_ERANIKUS_TYRANT, 3000},
    {SAY_ERANIKUS_TAUNT_2,  NPC_ERANIKUS_TYRANT, 10000},
    {SAY_REMULOS_TAUNT_3,   NPC_REMULOS,         12000},
    {SAY_ERANIKUS_TAUNT_4,  NPC_ERANIKUS_TYRANT, 6000},
    {EMOTE_ERANIKUS_ATTACK, NPC_ERANIKUS_TYRANT, 7000},
    {NPC_ERANIKUS_TYRANT,   0,                   0},        // target player - restart the escort and move Eranikus above the village
    {SAY_REMULOS_DEFEND_2,  NPC_REMULOS,         6000},     // face Eranikus
    {SAY_ERANIKUS_SHADOWS,  NPC_ERANIKUS_TYRANT, 4000},
    {SAY_REMULOS_DEFEND_3,  NPC_REMULOS,         0},
    {0, 0, 0},
};

struct EventLocations
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static EventLocations aEranikusLocations[] =
{
    {7881.72f, -2651.23f, 493.29f, 0.40f},          // eranikus spawn loc
    {7929.86f, -2574.88f, 505.35f},                 // eranikus flight move loc
    {7912.98f, -2568.99f, 488.71f},                 // eranikus combat move loc
    {7906.57f, -2565.63f, 488.39f},                 // eranikus redeemed loc
};

static EventLocations aTyrandeLocations[] =
{
    // Tyrande should appear along the pathway, but because of the missing pathfinding we'll summon here closer to Eranikus
    {7948.89f, -2575.58f, 490.05f, 3.03f},          // tyrande spawn loc
    {7888.32f, -2566.25f, 487.02f},                 // tyrande heal loc
    {7901.83f, -2565.24f, 488.04f},                 // tyrande eranikus loc
};

static EventLocations aShadowsLocations[] =
{
    // Inside the house shades - first wave only
    {7832.78f, -2604.57f, 489.29f},
    {7826.68f, -2538.46f, 489.30f},
    {7811.48f, -2573.20f, 488.49f},
    // Outside shade points - basically only the first set of coords is used for the summoning; there is no solid proof of using the other coords
    {7888.32f, -2566.25f, 487.02f},
    {7946.12f, -2577.10f, 489.97f},
    {7963.00f, -2492.03f, 487.84f}
};

struct npc_keeper_remulosAI : public npc_escortAI, private DialogueHelper
{
    npc_keeper_remulosAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        Reset();
    }

    uint32 m_uiHealTimer;
    uint32 m_uiStarfireTimer;
    uint32 m_uiShadesummonTimer;
    uint32 m_uiOutroTimer;

    ObjectGuid m_eranikusGuid;

    uint8 m_uiOutroPhase;
    uint8 m_uiSummonCount;

    bool m_bIsFirstWave;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiOutroTimer          = 0;
            m_uiOutroPhase          = 0;
            m_uiSummonCount         = 0;

            m_eranikusGuid.Clear();

            m_uiShadesummonTimer    = 0;
            m_uiHealTimer           = 10000;
            m_uiStarfireTimer       = 25000;

            m_bIsFirstWave          = true;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_ERANIKUS_TYRANT:
                m_eranikusGuid = pSummoned->GetObjectGuid();
                // Make Eranikus unattackable first
                // ToDo: uncomment the fly effect when it will be possible to cancel it properly
                // pSummoned->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_UNK_2);
                pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pSummoned->SetLevitate(true);
                break;
            case NPC_NIGHTMARE_PHANTASM:
                // ToDo: set faction to DB
                pSummoned->setFaction(14);
                pSummoned->AI()->AttackStart(m_creature);
                break;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE || pSummoned->GetEntry() != NPC_ERANIKUS_TYRANT)
            return;

        switch (uiPointId)
        {
            case POINT_ID_ERANIKUS_FLIGHT:
                // Set Eranikus to face Remulos
                pSummoned->SetFacingToObject(m_creature);
                break;
            case POINT_ID_ERANIKUS_COMBAT:
                // Start attack
                pSummoned->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pSummoned->AI()->AttackStart(m_creature);
                DoScriptText(SAY_ERANIKUS_ATTACK_2, pSummoned);
                break;
        }
    }

    void JustDied(Unit* pKiller) override
    {
        // Make Eranikus evade in order to despawn all the summons
        if (Creature* pEranikus = m_creature->GetMap()->GetCreature(m_eranikusGuid))
            pEranikus->AI()->EnterEvadeMode();

        npc_escortAI::JustDied(pKiller);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 0:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_REMULOS_INTRO_1, m_creature, pPlayer);
                break;
            case 1:
                DoScriptText(SAY_REMULOS_INTRO_2, m_creature);
                break;
            case 13:
                StartNextDialogueText(NPC_REMULOS);
                SetEscortPaused(true);
                break;
            case 17:
                StartNextDialogueText(SAY_REMULOS_DEFEND_2);
                SetEscortPaused(true);
                break;
            case 18:
                SetEscortPaused(true);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        switch (uiEntry)
        {
            case NPC_REMULOS:         return m_creature;
            case NPC_ERANIKUS_TYRANT: return m_creature->GetMap()->GetCreature(m_eranikusGuid);

            default:
                return nullptr;
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case NPC_REMULOS:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_REMULOS_INTRO_3, m_creature, pPlayer);
                break;
            case SPELL_CONJURE_RIFT:
                DoCastSpellIfCan(m_creature, SPELL_CONJURE_RIFT);
                break;
            case SAY_ERANIKUS_SPAWN:
                // This big yellow emote was removed at some point in WotLK
                // DoScriptText(EMOTE_SUMMON_ERANIKUS, pEranikus);
                break;
            case NPC_ERANIKUS_TYRANT:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_REMULOS_DEFEND_1, m_creature, pPlayer);
                if (Creature* pEranikus = m_creature->GetMap()->GetCreature(m_eranikusGuid))
                    pEranikus->GetMotionMaster()->MovePoint(POINT_ID_ERANIKUS_FLIGHT, aEranikusLocations[1].m_fX, aEranikusLocations[1].m_fY, aEranikusLocations[1].m_fZ);
                SetEscortPaused(false);
                break;
            case SAY_REMULOS_DEFEND_2:
                if (Creature* pEranikus = m_creature->GetMap()->GetCreature(m_eranikusGuid))
                    m_creature->SetFacingToObject(pEranikus);
                break;
            case SAY_REMULOS_DEFEND_3:
                SetEscortPaused(true);
                m_uiShadesummonTimer = 5000;
                break;
        }
    }

    void DoHandleOutro(Creature* pTarget)
    {
        if (Player* pPlayer = GetPlayerForEscort())
            pPlayer->GroupEventHappens(QUEST_NIGHTMARE_MANIFESTS, pTarget);

        m_uiOutroTimer = 3000;
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (m_uiOutroTimer)
        {
            if (m_uiOutroTimer <= uiDiff)
            {
                switch (m_uiOutroPhase)
                {
                    case 0:
                        DoScriptText(SAY_REMULOS_OUTRO_1, m_creature);
                        m_uiOutroTimer = 3000;
                        break;
                    case 1:
                        // Despawn Remulos after the outro is finished - he will respawn automatically at his home position after a few min
                        DoScriptText(SAY_REMULOS_OUTRO_2, m_creature);
                        m_creature->SetRespawnDelay(1 * MINUTE);
                        m_creature->ForcedDespawn(3000);
                        m_uiOutroTimer = 0;
                        break;
                }
                ++m_uiOutroPhase;
            }
            else
                m_uiOutroTimer -= uiDiff;
        }

        // during the battle
        if (m_uiShadesummonTimer)
        {
            if (m_uiShadesummonTimer <= uiDiff)
            {
                // do this yell only first time
                if (m_bIsFirstWave)
                {
                    // summon 3 shades inside the house
                    for (uint8 i = 0; i < MAX_SHADOWS; ++i)
                        m_creature->SummonCreature(NPC_NIGHTMARE_PHANTASM, aShadowsLocations[i].m_fX, aShadowsLocations[i].m_fY, aShadowsLocations[i].m_fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0);

                    if (Creature* pEranikus = m_creature->GetMap()->GetCreature(m_eranikusGuid))
                        DoScriptText(SAY_ERANIKUS_ATTACK_1, pEranikus);

                    ++m_uiSummonCount;
                    SetEscortPaused(false);
                    m_bIsFirstWave = false;
                }

                // Summon 3 shades per turn until the maximum summon turns are reached
                float fX, fY, fZ;
                // Randomize the summon point
                uint8 uiSummonPoint = roll_chance_i(70) ? uint32(MAX_SHADOWS) : urand(MAX_SHADOWS + 1, MAX_SHADOWS + 2);

                if (m_uiSummonCount < MAX_SUMMON_TURNS)
                {
                    for (uint8 i = 0; i < MAX_SHADOWS; ++i)
                    {
                        m_creature->GetRandomPoint(aShadowsLocations[uiSummonPoint].m_fX, aShadowsLocations[uiSummonPoint].m_fY, aShadowsLocations[uiSummonPoint].m_fZ, 10.0f, fX, fY, fZ);
                        m_creature->SummonCreature(NPC_NIGHTMARE_PHANTASM, fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }

                    ++m_uiSummonCount;
                }

                // If all the shades were summoned then set Eranikus in combat
                // We don't count the dead shades, because the boss is usually set in combat before all shades are dead
                if (m_uiSummonCount == MAX_SUMMON_TURNS)
                {
                    m_uiShadesummonTimer = 0;

                    if (Creature* pEranikus = m_creature->GetMap()->GetCreature(m_eranikusGuid))
                    {
                        pEranikus->SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0);
                        pEranikus->SetLevitate(false);
                        pEranikus->GetMotionMaster()->MovePoint(POINT_ID_ERANIKUS_COMBAT, aEranikusLocations[2].m_fX, aEranikusLocations[2].m_fY, aEranikusLocations[2].m_fZ);
                    }
                }
                else
                    m_uiShadesummonTimer = urand(20000, 30000);
            }
            else
                m_uiShadesummonTimer -= uiDiff;
        }

        // Combat spells
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiHealTimer < uiDiff)
        {
            if (Unit* pTarget = DoSelectLowestHpFriendly(DEFAULT_VISIBILITY_DISTANCE))
            {
                switch (urand(0, 2))
                {
                    case 0: DoCastSpellIfCan(pTarget, SPELL_HEALING_TOUCH); break;
                    case 1: DoCastSpellIfCan(pTarget, SPELL_REJUVENATION);  break;
                    case 2: DoCastSpellIfCan(pTarget, SPELL_REGROWTH);      break;
                }
            }
            m_uiHealTimer = 10000;
        }
        else
            m_uiHealTimer -= uiDiff;

        if (m_uiStarfireTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_STARFIRE) == CAST_OK)
                    m_uiStarfireTimer = 20000;
            }
        }
        else
            m_uiStarfireTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_keeper_remulos(Creature* pCreature)
{
    return new npc_keeper_remulosAI(pCreature);
}

bool QuestAccept_npc_keeper_remulos(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_NIGHTMARE_MANIFESTS)
    {
        if (npc_keeper_remulosAI* pEscortAI = dynamic_cast<npc_keeper_remulosAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);

        return true;
    }

    // Return false for other quests in order to handle DB scripts. Example: quest 8447
    return false;
}

bool EffectDummyCreature_conjure_rift(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* /*pCreatureTarget*/, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_CONJURE_RIFT && uiEffIndex == EFFECT_INDEX_0)
    {
        pCaster->SummonCreature(NPC_ERANIKUS_TYRANT, aEranikusLocations[0].m_fX, aEranikusLocations[0].m_fY, aEranikusLocations[0].m_fZ, aEranikusLocations[0].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);

        // always return true when we are handling this spell and effect
        return true;
    }

    return false;
}

/*######
## boss_eranikus
######*/

enum
{
    NPC_KEEPER_REMULOS          = 11832,

    SPELL_ACID_BREATH           = 24839,
    SPELL_NOXIOUS_BREATH        = 24818,
    SPELL_SHADOWBOLT_VOLLEY     = 25586,
    SPELL_ARCANE_CHANNELING     = 23017,        // used by Tyrande - not sure if it's the right id

    FACTION_FRIENDLY            = 35,
    MAX_PRIESTESS               = 7,

    POINT_ID_TYRANDE_HEAL       = 0,
    POINT_ID_TYRANDE_ABSOLUTION = 1,
};

struct boss_eranikusAI : public ScriptedAI
{
    boss_eranikusAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiAcidBreathTimer;
    uint32 m_uiNoxiousBreathTimer;
    uint32 m_uiShadowboltVolleyTimer;
    uint32 m_uiEventTimer;
    uint32 m_uiTyrandeMoveTimer;

    uint8 m_uiEventPhase;
    uint8 m_uiTyrandeMovePoint;
    uint8 m_uiHealthCheck;

    ObjectGuid m_remulosGuid;
    ObjectGuid m_tyrandeGuid;
    GuidList m_lPriestessList;

    void Reset() override
    {
        m_uiAcidBreathTimer         = 10000;
        m_uiNoxiousBreathTimer      = 3000;
        m_uiShadowboltVolleyTimer   = 5000;
        m_uiTyrandeMoveTimer        = 0;

        m_remulosGuid.Clear();
        m_tyrandeGuid.Clear();

        m_uiHealthCheck             = 85;
        m_uiEventPhase              = 0;
        m_uiEventTimer              = 0;

        // For some reason the boss doesn't move in combat
        SetCombatMovement(false);
    }

    void EnterEvadeMode() override
    {
        if (m_creature->GetHealthPercent() < 20.0f)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->DeleteThreatList();
            m_creature->CombatStop(true);
            m_creature->LoadCreatureAddon(true);

            m_creature->SetLootRecipient(nullptr);

            // Get Remulos guid and make him stop summoning shades
            if (Creature* pRemulos = GetClosestCreatureWithEntry(m_creature, NPC_REMULOS, 50.0f))
            {
                m_remulosGuid = pRemulos->GetObjectGuid();
                pRemulos->AI()->EnterEvadeMode();
            }

            // Despawn the priestess
            DoDespawnSummoned();

            // redeem eranikus
            m_uiEventTimer = 5000;
            m_creature->setFaction(FACTION_FRIENDLY);
        }
        else
        {
            // There may be a core issue related to the reached home function for summoned creatures so we are cleaning things up here
            // if the creature evades while the event is in progress then we despawn all the summoned, including himself
            m_creature->ForcedDespawn();
            DoDespawnSummoned();

            if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                pTyrande->ForcedDespawn();
        }
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        DoScriptText(SAY_ERANIKUS_KILL, m_creature);
    }

    void DoSummonHealers()
    {
        float fX, fY, fZ;
        for (uint8 j = 0; j < MAX_PRIESTESS; ++j)
        {
            m_creature->GetRandomPoint(aTyrandeLocations[0].m_fX, aTyrandeLocations[0].m_fY, aTyrandeLocations[0].m_fZ, 10.0f, fX, fY, fZ);
            m_creature->SummonCreature(NPC_ELUNE_PRIESTESS, fX, fY, fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_TYRANDE_WHISPERWIND:
                m_tyrandeGuid = pSummoned->GetObjectGuid();
                pSummoned->SetWalk(false);
                pSummoned->GetMotionMaster()->MovePoint(POINT_ID_TYRANDE_HEAL, aTyrandeLocations[1].m_fX, aTyrandeLocations[1].m_fY, aTyrandeLocations[1].m_fZ);
                break;
            case NPC_ELUNE_PRIESTESS:
                m_lPriestessList.push_back(pSummoned->GetObjectGuid());
                float fX, fY, fZ;
                pSummoned->SetWalk(false);
                m_creature->GetRandomPoint(aTyrandeLocations[1].m_fX, aTyrandeLocations[1].m_fY, aTyrandeLocations[1].m_fZ, 10.0f, fX, fY, fZ);
                pSummoned->GetMotionMaster()->MovePoint(POINT_ID_TYRANDE_HEAL, fX, fY, fZ);
                break;
        }
    }

    void DoDespawnSummoned()
    {
        for (GuidList::const_iterator itr = m_lPriestessList.begin(); itr != m_lPriestessList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                pTemp->ForcedDespawn();
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (uiPointId)
        {
            case POINT_ID_TYRANDE_HEAL:
                if (pSummoned->GetEntry() == NPC_TYRANDE_WHISPERWIND)
                {
                    // Unmont, yell and prepare to channel the spell on Eranikus
                    DoScriptText(SAY_TYRANDE_HEAL, pSummoned);
                    pSummoned->Unmount();
                    m_uiTyrandeMoveTimer = 5000;
                }
                // Unmount the priestess - unk what is their exact purpose (maybe healer)
                else if (pSummoned->GetEntry() == NPC_ELUNE_PRIESTESS)
                    pSummoned->Unmount();
                break;
            case POINT_ID_TYRANDE_ABSOLUTION:
                if (pSummoned->GetEntry() == NPC_TYRANDE_WHISPERWIND)
                {
                    pSummoned->CastSpell(pSummoned, SPELL_ARCANE_CHANNELING, false);
                    DoScriptText(SAY_TYRANDE_FORGIVEN_1, pSummoned);
                }
                break;
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE || uiPointId != POINT_ID_ERANIKUS_REDEEMED)
            return;

        DoScriptText(SAY_REDEEMED_1, m_creature);
        m_uiEventTimer = 11000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiEventPhase)
                {
                    case 0:
                        // Eranikus is redeemed - make Tyrande kneel and stop casting
                        if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                        {
                            pTyrande->InterruptNonMeleeSpells(false);
                            pTyrande->SetStandState(UNIT_STAND_STATE_KNEEL);
                            DoScriptText(EMOTE_TYRANDE_KNEEL, pTyrande);
                        }
                        if (Creature* pRemulos = m_creature->GetMap()->GetCreature(m_remulosGuid))
                            pRemulos->SetFacingToObject(m_creature);
                        // Note: this emote was a world wide yellow emote before WotLK
                        DoScriptText(EMOTE_ERANIKUS_REDEEM, m_creature);
                        // DoCastSpellIfCan(m_creature, SPELL_MOONGLADE_TRANQUILITY);        // spell id unk for the moment
                        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                        m_uiEventTimer = 5000;
                        break;
                    case 1:
                        if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                            DoScriptText(SAY_TYRANDE_REDEEMED, pTyrande);
                        m_uiEventTimer = 6000;
                        break;
                    case 2:
                        // Transform Eranikus into elf
                        DoCastSpellIfCan(m_creature, SPELL_ERANIKUS_REDEEMED);
                        m_uiEventTimer = 5000;
                        break;
                    case 3:
                        // Move Eranikus in front of Tyrande
                        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                        m_creature->SetWalk(true);
                        m_creature->GetMotionMaster()->MovePoint(POINT_ID_ERANIKUS_REDEEMED, aEranikusLocations[3].m_fX, aEranikusLocations[3].m_fY, aEranikusLocations[3].m_fZ);
                        m_uiEventTimer = 0;
                        break;
                    case 4:
                        DoScriptText(SAY_REDEEMED_2, m_creature);
                        m_uiEventTimer = 11000;
                        break;
                    case 5:
                        DoScriptText(SAY_REDEEMED_3, m_creature);
                        m_uiEventTimer = 13000;
                        break;
                    case 6:
                        DoScriptText(SAY_REDEEMED_4, m_creature);
                        m_uiEventTimer = 7000;
                        break;
                    case 7:
                        // Complete Quest and end event
                        if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                        {
                            pTyrande->SetStandState(UNIT_STAND_STATE_STAND);
                            pTyrande->ForcedDespawn(9000);
                        }
                        if (Creature* pRemulos = m_creature->GetMap()->GetCreature(m_remulosGuid))
                            ((npc_keeper_remulosAI*)pRemulos->AI())->DoHandleOutro(m_creature);
                        m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                        m_creature->ForcedDespawn(2000);
                        break;
                }
                ++m_uiEventPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Move Tyrande after she is summoned
        if (m_uiTyrandeMoveTimer)
        {
            if (m_uiTyrandeMoveTimer <= uiDiff)
            {
                if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                    pTyrande->GetMotionMaster()->MovePoint(POINT_ID_TYRANDE_ABSOLUTION, aTyrandeLocations[2].m_fX, aTyrandeLocations[2].m_fY, aTyrandeLocations[2].m_fZ);
                m_uiTyrandeMoveTimer = 0;
            }
            else
                m_uiTyrandeMoveTimer -= uiDiff;
        }

        // Not sure if this should be handled by health percent, but this is the only reasonable way
        if (m_creature->GetHealthPercent() < m_uiHealthCheck)
        {
            switch (m_uiHealthCheck)
            {
                case 85:
                    DoScriptText(SAY_ERANIKUS_ATTACK_3, m_creature);
                    // Here Tyrande only yells but she doesn't appear anywhere - we summon here for 1 second just to handle the yell
                    if (Creature* pTyrande = m_creature->SummonCreature(NPC_TYRANDE_WHISPERWIND, aTyrandeLocations[0].m_fX, aTyrandeLocations[0].m_fY, aTyrandeLocations[0].m_fZ, 0, TEMPSUMMON_TIMED_DESPAWN, 1000))
                        DoScriptText(SAY_TYRANDE_APPEAR, pTyrande);
                    m_uiHealthCheck = 75;
                    break;
                case 75:
                    // Eranikus yells again
                    DoScriptText(SAY_ERANIKUS_ATTACK_3, m_creature);
                    m_uiHealthCheck = 50;
                    break;
                case 50:
                    // Summon Tyrande - she enters the fight this time
                    m_creature->SummonCreature(NPC_TYRANDE_WHISPERWIND, aTyrandeLocations[0].m_fX, aTyrandeLocations[0].m_fY, aTyrandeLocations[0].m_fZ, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    m_uiHealthCheck = 35;
                    break;
                case 35:
                    // Summon the priestess
                    DoSummonHealers();
                    DoScriptText(SAY_ERANIKUS_DEFEAT_1, m_creature);
                    m_uiHealthCheck = 31;
                    break;
                case 31:
                    if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                        DoScriptText(SAY_TYRANDE_FORGIVEN_2, pTyrande);
                    m_uiHealthCheck = 27;
                    break;
                case 27:
                    if (Creature* pTyrande = m_creature->GetMap()->GetCreature(m_tyrandeGuid))
                        DoScriptText(SAY_TYRANDE_FORGIVEN_3, pTyrande);
                    m_uiHealthCheck = 25;
                    break;
                case 25:
                    DoScriptText(SAY_ERANIKUS_DEFEAT_2, m_creature);
                    m_uiHealthCheck = 20;
                    break;
                case 20:
                    // Eranikus is redeemed - stop the fight
                    DoScriptText(SAY_ERANIKUS_DEFEAT_3, m_creature);
                    m_creature->AI()->EnterEvadeMode();
                    m_uiHealthCheck = 0;
                    break;
            }
        }

        // Combat spells
        if (m_uiAcidBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ACID_BREATH) == CAST_OK)
                m_uiAcidBreathTimer = 15000;
        }
        else
            m_uiAcidBreathTimer -= uiDiff;

        if (m_uiNoxiousBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_NOXIOUS_BREATH) == CAST_OK)
                m_uiNoxiousBreathTimer = 30000;
        }
        else
            m_uiNoxiousBreathTimer -= uiDiff;

        if (m_uiShadowboltVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBOLT_VOLLEY) == CAST_OK)
                m_uiShadowboltVolleyTimer = 25000;
        }
        else
            m_uiShadowboltVolleyTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_eranikus(Creature* pCreature)
{
    return new boss_eranikusAI(pCreature);
}

void AddSC_moonglade()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_keeper_remulos";
    pNewScript->GetAI = &GetAI_npc_keeper_remulos;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_keeper_remulos;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_conjure_rift;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_eranikus";
    pNewScript->GetAI = &GetAI_boss_eranikus;
    pNewScript->RegisterSelf();
}
