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
SDName: Western_Plaguelands
SD%Complete: 90
SDComment: Quest support: 5216, 5219, 5222, 5225, 5229, 5231, 5233, 5235, 5862, 5944.
SDCategory: Western Plaguelands
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
npc_the_scourge_cauldron
npc_taelan_fordring
npc_isillien
npc_tirion_fordring
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_the_scourge_cauldron
######*/

struct npc_the_scourge_cauldronAI : public ScriptedAI
{
    npc_the_scourge_cauldronAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() override {}

    void DoDie()
    {
        // summoner dies here
        m_creature->DealDamage(m_creature, m_creature->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
        // override any database `spawntimesecs` to prevent duplicated summons
        uint32 rTime = m_creature->GetRespawnDelay();
        if (rTime < 600)
            m_creature->SetRespawnDelay(600);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            switch (m_creature->GetAreaId())
            {
                case 199:                                   // felstone
                    if (((Player*)who)->GetQuestStatus(5216) == QUEST_STATUS_INCOMPLETE ||
                            ((Player*)who)->GetQuestStatus(5229) == QUEST_STATUS_INCOMPLETE)
                    {
                        m_creature->SummonCreature(11075, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
                        DoDie();
                    }
                    break;
                case 200:                                   // dalson
                    if (((Player*)who)->GetQuestStatus(5219) == QUEST_STATUS_INCOMPLETE ||
                            ((Player*)who)->GetQuestStatus(5231) == QUEST_STATUS_INCOMPLETE)
                    {
                        m_creature->SummonCreature(11077, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
                        DoDie();
                    }
                    break;
                case 201:                                   // gahrron
                    if (((Player*)who)->GetQuestStatus(5225) == QUEST_STATUS_INCOMPLETE ||
                            ((Player*)who)->GetQuestStatus(5235) == QUEST_STATUS_INCOMPLETE)
                    {
                        m_creature->SummonCreature(11078, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
                        DoDie();
                    }
                    break;
                case 202:                                   // writhing
                    if (((Player*)who)->GetQuestStatus(5222) == QUEST_STATUS_INCOMPLETE ||
                            ((Player*)who)->GetQuestStatus(5233) == QUEST_STATUS_INCOMPLETE)
                    {
                        m_creature->SummonCreature(11076, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 600000);
                        DoDie();
                    }
                    break;
            }
        }
    }
};

UnitAI* GetAI_npc_the_scourge_cauldron(Creature* pCreature)
{
    return new npc_the_scourge_cauldronAI(pCreature);
}

/*######
## npc_taelan_fordring
######*/

enum
{
    // scarlet end quest texts
    SAY_SCARLET_COMPLETE_1          = -1001076,
    SAY_SCARLET_COMPLETE_2          = -1001077,

    // escort text
    SAY_ESCORT_START                = -1001078,
    SAY_EXIT_KEEP                   = -1001079,
    EMOTE_MOUNT                     = -1001080,
    SAY_REACH_TOWER                 = -1001081,
    SAY_ISILLIEN_1                  = -1001082,
    SAY_ISILLIEN_2                  = -1001083,
    SAY_ISILLIEN_3                  = -1001084,
    SAY_ISILLIEN_4                  = -1001085,
    SAY_ISILLIEN_5                  = -1001086,
    SAY_ISILLIEN_6                  = -1001087,
    EMOTE_ISILLIEN_ATTACK           = -1001088,
    SAY_ISILLIEN_ATTACK             = -1001089,
    SAY_KILL_TAELAN_1               = -1001090,
    EMOTE_ISILLIEN_LAUGH            = -1001091,
    SAY_KILL_TAELAN_2               = -1001092,
    EMOTE_ATTACK_PLAYER             = -1001093,
    SAY_TIRION_1                    = -1001094,
    SAY_TIRION_2                    = -1001095,
    SAY_TIRION_3                    = -1001096,
    SAY_TIRION_4                    = -1001097,
    SAY_TIRION_5                    = -1001098,
    SAY_EPILOG_1                    = -1001099,
    EMOTE_KNEEL                     = -1001100,
    SAY_EPILOG_2                    = -1001101,
    EMOTE_HOLD_TAELAN               = -1001102,
    SAY_EPILOG_3                    = -1001103,
    SAY_EPILOG_4                    = -1001104,
    SAY_EPILOG_5                    = -1001105,

    // spells
    SPELL_DEVOTION_AURA             = 17232,
    SPELL_CRUSADER_STRIKE           = 14518,
    SPELL_HOLY_STRIKE               = 17143,
    SPELL_HOLY_CLEAVE               = 18819,
    SPELL_HOLY_LIGHT                = 15493,
    SPELL_LAY_ON_HANDS              = 17233,
    SPELL_TAELAN_SUFFERING          = 18810,

    // isillen spells
    SPELL_DOMINATE_MIND             = 14515,
    SPELL_FLASH_HEAL                = 10917,
    SPELL_GREATER_HEAL              = 10965,
    SPELL_MANA_BURN                 = 15800,
    SPELL_MIND_BLAST                = 17194,
    SPELL_MIND_FLAY                 = 17165,
    SPELL_TAELAN_DEATH              = 18969,

    // npcs
    NPC_TAELAN_FORDRING             = 1842,
    NPC_SCARLET_CAVALIER            = 1836,
    NPC_ISILLIEN                    = 1840,
    NPC_TIRION_FORDRING             = 12126,                    // Todo: add mount
    NPC_CRIMSON_ELITE               = 12128,

    MODEL_TAELAN_MOUNT              = 2410,                     // ToDo: fix id!

    // quests
    QUEST_ID_SCARLET_SUBTERFUGE     = 5862,
    QUEST_ID_IN_DREAMS              = 5944,
};

static const int32 aCavalierYells[] = { -1001072, -1001073, -1001074, -1001075 };

static const DialogueEntry aScarletDialogue[] =
{
    // Scarlet Subterfuge ending dialogue
    {NPC_SCARLET_CAVALIER,          0,                          3000},
    {QUEST_ID_SCARLET_SUBTERFUGE,   0,                          7000},
    {SAY_SCARLET_COMPLETE_1,        NPC_TAELAN_FORDRING,        2000},
    {QUEST_ID_IN_DREAMS,            0,                          0},
    {SPELL_DEVOTION_AURA,           0,                          3000},
    {SAY_SCARLET_COMPLETE_2,        NPC_TAELAN_FORDRING,        0},
    // In Dreams event dialogue
    {SAY_EXIT_KEEP,                 NPC_TAELAN_FORDRING,        6000},
    {EMOTE_MOUNT,                   NPC_TAELAN_FORDRING,        4000},
    {MODEL_TAELAN_MOUNT,            0,                          0},
    {SAY_REACH_TOWER,               NPC_TAELAN_FORDRING,        4000},
    {SAY_ISILLIEN_1,                NPC_ISILLIEN,               2000},
    {SAY_ISILLIEN_2,                NPC_TAELAN_FORDRING,        3000},
    {SAY_ISILLIEN_3,                NPC_TAELAN_FORDRING,        10000},
    {SAY_ISILLIEN_4,                NPC_ISILLIEN,               7000},
    {SAY_ISILLIEN_5,                NPC_ISILLIEN,               5000},
    {SAY_ISILLIEN_6,                NPC_ISILLIEN,               6000},
    {EMOTE_ISILLIEN_ATTACK,         NPC_ISILLIEN,               3000},
    {SAY_ISILLIEN_ATTACK,           NPC_ISILLIEN,               3000},
    {SPELL_CRUSADER_STRIKE,         0,                          0},
    {SAY_KILL_TAELAN_1,             NPC_ISILLIEN,               1000},
    {EMOTE_ISILLIEN_LAUGH,          NPC_ISILLIEN,               4000},
    {SAY_KILL_TAELAN_2,             NPC_ISILLIEN,               10000},
    {EMOTE_ATTACK_PLAYER,           NPC_ISILLIEN,               0},
    {NPC_TIRION_FORDRING,           0,                          5000},
    {NPC_ISILLIEN,                  0,                          10000},
    {SAY_TIRION_1,                  NPC_TIRION_FORDRING,        4000},
    {SAY_TIRION_2,                  NPC_ISILLIEN,               6000},
    {SAY_TIRION_3,                  NPC_TIRION_FORDRING,        4000},
    {SAY_TIRION_4,                  NPC_TIRION_FORDRING,        3000},
    {SAY_TIRION_5,                  NPC_ISILLIEN,               0},
    {EMOTE_KNEEL,                   NPC_TIRION_FORDRING,        4000},
    {SAY_EPILOG_2,                  NPC_TIRION_FORDRING,        5000},
    {EMOTE_HOLD_TAELAN,             NPC_TIRION_FORDRING,        5000},
    {SAY_EPILOG_3,                  NPC_TIRION_FORDRING,        6000},
    {SAY_EPILOG_4,                  NPC_TIRION_FORDRING,        7000},
    {SAY_EPILOG_5,                  NPC_TIRION_FORDRING,        0},
    {0, 0, 0},
};

struct npc_taelan_fordringAI: public npc_escortAI, private DialogueHelper
{
    npc_taelan_fordringAI(Creature* pCreature): npc_escortAI(pCreature),
        DialogueHelper(aScarletDialogue)
    {
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        m_bScarletComplete = false;
        m_bFightStarted = false;
        m_bTaelanDead = false;
        m_bHasMount = false;
        Reset();
    }

    bool m_bScarletComplete;
    bool m_bFightStarted;
    bool m_bHasMount;
    bool m_bTaelanDead;

    ObjectGuid m_isillenGuid;
    ObjectGuid m_tirionGuid;
    GuidList m_lCavalierGuids;

    uint32 m_uiHolyCleaveTimer;
    uint32 m_uiHolyStrikeTimer;
    uint32 m_uiCrusaderStrike;
    uint32 m_uiHolyLightTimer;

    void Reset() override
    {
        m_uiHolyCleaveTimer = urand(11000, 15000);
        m_uiHolyStrikeTimer = urand(6000, 8000);
        m_uiCrusaderStrike  = urand(1000, 5000);
        m_uiHolyLightTimer  = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_bHasMount)
            m_creature->Unmount();

        DoCastSpellIfCan(m_creature, SPELL_DEVOTION_AURA);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_bTaelanDead)
            return;

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void EnterEvadeMode() override
    {
        if (m_bTaelanDead)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
            m_creature->SetLootRecipient(nullptr);

            m_creature->InterruptNonMeleeSpells(true);
            m_creature->SetHealth(0);
            m_creature->StopMoving();
            m_creature->ClearComboPointHolders();
            m_creature->RemoveAllAurasOnDeath();
            m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->ClearAllReactives();
            m_creature->GetMotionMaster()->Clear();
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

            Reset();
        }
        else
        {
            if (m_bHasMount)
                m_creature->Mount(MODEL_TAELAN_MOUNT);

            npc_escortAI::EnterEvadeMode();
        }
    }

    void JustReachedHome() override
    {
        if (m_bScarletComplete)
        {
            StartNextDialogueText(SPELL_DEVOTION_AURA);
            m_bScarletComplete = false;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue));
            DoScriptText(SAY_ESCORT_START, m_creature);
            m_creature->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
        else if (eventType == AI_EVENT_CUSTOM_A && pInvoker->GetTypeId() == TYPEID_PLAYER && uiMiscValue == QUEST_ID_SCARLET_SUBTERFUGE)
            StartNextDialogueText(NPC_SCARLET_CAVALIER);
        else if (eventType == AI_EVENT_CUSTOM_B && pInvoker->GetEntry() == NPC_ISILLIEN)
        {
            StartNextDialogueText(NPC_TIRION_FORDRING);
            if (Creature* pTirion = m_creature->SummonCreature(NPC_TIRION_FORDRING, 2620.273f, -1920.917f, 74.25f, 0, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                pTirion->SetWalk(false);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 25:
                SetEscortPaused(true);
                StartNextDialogueText(SAY_EXIT_KEEP);
                break;
            case 55:
                SetEscortPaused(true);
                StartNextDialogueText(SAY_REACH_TOWER);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_ISILLIEN:
                SendAIEvent(AI_EVENT_START_ESCORT, m_creature, pSummoned);
                m_isillenGuid = pSummoned->GetObjectGuid();

                // summon additional crimson elites
                float fX, fY, fZ;
                pSummoned->GetNearPoint(pSummoned, fX, fY, fZ, 0, 5.0f, M_PI_F * 1.25f);
                pSummoned->SummonCreature(NPC_CRIMSON_ELITE, fX, fY, fZ, pSummoned->GetOrientation(), TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                pSummoned->GetNearPoint(pSummoned, fX, fY, fZ, 0, 5.0f, 0);
                pSummoned->SummonCreature(NPC_CRIMSON_ELITE, fX, fY, fZ, pSummoned->GetOrientation(), TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                break;
            case NPC_TIRION_FORDRING:
                m_tirionGuid = pSummoned->GetObjectGuid();
                SendAIEvent(AI_EVENT_START_ESCORT, m_creature, pSummoned);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_ISILLIEN)
        {
            if (Creature* pTirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                DoScriptText(SAY_EPILOG_1, pTirion);
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*uiMotionType*/, uint32 uiPointId) override
    {
        if (pSummoned->GetEntry() != NPC_TIRION_FORDRING)
            return;

        if (uiPointId == 100)
        {
            StartNextDialogueText(SAY_TIRION_1);
            if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                pSummoned->SetFacingToObject(pIsillien);
        }
        else if (uiPointId == 200)
            StartNextDialogueText(EMOTE_KNEEL);
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case NPC_SCARLET_CAVALIER:
            {
                // kneel and make everyone worried
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

                CreatureList lCavaliersInRange;
                GetCreatureListWithEntryInGrid(lCavaliersInRange, m_creature, NPC_SCARLET_CAVALIER, 10.0f);

                uint8 uiIndex = 0;
                for (CreatureList::const_iterator itr = lCavaliersInRange.begin(); itr != lCavaliersInRange.end(); ++itr)
                {
                    m_lCavalierGuids.push_back((*itr)->GetObjectGuid());
                    (*itr)->SetFacingToObject(m_creature);
                    DoScriptText(aCavalierYells[uiIndex], (*itr));
                    ++uiIndex;
                }
                break;
            }
            case QUEST_ID_SCARLET_SUBTERFUGE:
            {
                float fX, fY, fZ;
                for (GuidList::const_iterator itr = m_lCavalierGuids.begin(); itr != m_lCavalierGuids.end(); ++itr)
                {
                    if (Creature* pCavalier = m_creature->GetMap()->GetCreature(*itr))
                    {
                        m_creature->GetContactPoint(pCavalier, fX, fY, fZ);
                        pCavalier->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    }
                }
                break;
            }
            case SAY_SCARLET_COMPLETE_1:
                // stand up and knock down effect
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                DoCastSpellIfCan(m_creature, SPELL_TAELAN_SUFFERING);
                break;
            case QUEST_ID_IN_DREAMS:
                // force attack
                for (GuidList::const_iterator itr = m_lCavalierGuids.begin(); itr != m_lCavalierGuids.end(); ++itr)
                {
                    if (Creature* pCavalier = m_creature->GetMap()->GetCreature(*itr))
                        pCavalier->AI()->AttackStart(m_creature);
                }
                m_bScarletComplete = true;
                break;
            case SAY_SCARLET_COMPLETE_2:
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                break;
            case MODEL_TAELAN_MOUNT:
                // mount when outside
                m_bHasMount = true;
                SetEscortPaused(false);
                m_creature->Mount(MODEL_TAELAN_MOUNT);
                break;
            case SAY_REACH_TOWER:
                // start fight event
                m_creature->SummonCreature(NPC_ISILLIEN, 2693.12f, -1943.04f, 72.04f, 2.11f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                break;
            case SAY_ISILLIEN_2:
                m_creature->Unmount();
                if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                    m_creature->SetFacingToObject(pIsillien);
                break;
            case SPELL_CRUSADER_STRIKE:
            {
                float fX, fY, fZ;
                // spawn 3 additional elites
                if (Creature* pElite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2711.32f, -1882.67f, 67.89f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    pElite->SetWalk(false);
                    m_creature->GetContactPoint(pElite, fX, fY, fZ);
                    pElite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
                if (Creature* pElite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2710.93f, -1878.90f, 67.97f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    pElite->SetWalk(false);
                    m_creature->GetContactPoint(pElite, fX, fY, fZ);
                    pElite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
                if (Creature* pElite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2710.53f, -1875.28f, 67.90f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    pElite->SetWalk(false);
                    m_creature->GetContactPoint(pElite, fX, fY, fZ);
                    pElite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }

                // Isillien only attacks Taelan
                if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    pIsillien->AI()->AttackStart(m_creature);
                    AttackStart(pIsillien);
                }

                m_bFightStarted = true;
                break;
            }
            case SAY_KILL_TAELAN_1:
                // kill taelan and attack players
                if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                    SendAIEvent(AI_EVENT_CUSTOM_A, m_creature, pIsillien);
                break;
            case EMOTE_ATTACK_PLAYER:
                // attack players
                if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    if (Player* pPlayer = GetPlayerForEscort())
                        pIsillien->AI()->AttackStart(pPlayer);
                }
                break;
            // tirion event
            case SAY_TIRION_5:
                if (Creature* pIsillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    if (Creature* pTirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    {
                        pTirion->AI()->AttackStart(pIsillien);
                        pIsillien->AI()->AttackStart(pTirion);
                    }
                }
                break;
            // epilog dialogue
            case EMOTE_HOLD_TAELAN:
                if (Creature* pTirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    pTirion->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case SAY_EPILOG_4:
                if (Creature* pTirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    pTirion->SetStandState(UNIT_STAND_STATE_STAND);
                break;
            case SAY_EPILOG_5:
                if (Creature* pTirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                {
                    if (Player* pPlayer = GetPlayerForEscort())
                    {
                        pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ID_IN_DREAMS, m_creature);
                        pTirion->SetFacingToObject(pPlayer);
                    }

                    pTirion->ForcedDespawn(3 * MINUTE * IN_MILLISECONDS);
                    pTirion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                m_creature->ForcedDespawn(3 * MINUTE * IN_MILLISECONDS);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        switch (uiEntry)
        {
            case NPC_TAELAN_FORDRING:   return m_creature;
            case NPC_ISILLIEN:          return m_creature->GetMap()->GetCreature(m_isillenGuid);
            case NPC_TIRION_FORDRING:   return m_creature->GetMap()->GetCreature(m_tirionGuid);

            default:
                return nullptr;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bTaelanDead)
            return;

        // If Taelan is in fight with Isillien and drops below 50% make him "dead"
        if (!m_bTaelanDead && m_bFightStarted && m_creature->GetHealthPercent() < 50.0f)
        {
            StartNextDialogueText(SAY_KILL_TAELAN_1);
            m_bTaelanDead = true;
        }

        if (m_uiHolyCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_CLEAVE) == CAST_OK)
                m_uiHolyCleaveTimer = urand(11000, 13000);
        }
        else
            m_uiHolyCleaveTimer -= uiDiff;

        if (m_uiHolyStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_STRIKE) == CAST_OK)
                m_uiHolyStrikeTimer = urand(9000, 14000);
        }
        else
            m_uiHolyStrikeTimer -= uiDiff;

        if (m_uiCrusaderStrike < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CRUSADER_STRIKE) == CAST_OK)
                m_uiCrusaderStrike = urand(7000, 12000);
        }
        else
            m_uiCrusaderStrike -= uiDiff;

        if (m_creature->GetHealthPercent() < 75.0f)
        {
            if (m_uiHolyLightTimer < uiDiff)
            {
                if (Unit* pTarget = DoSelectLowestHpFriendly(50.0f))
                {
                    if (DoCastSpellIfCan(pTarget, SPELL_HOLY_LIGHT) == CAST_OK)
                        m_uiHolyLightTimer = urand(10000, 15000);
                }
            }
            else
                m_uiHolyLightTimer -= uiDiff;
        }

        if (!m_bFightStarted && m_creature->GetHealthPercent() < 15.0f)
            DoCastSpellIfCan(m_creature, SPELL_LAY_ON_HANDS);

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_taelan_fordring(Creature* pCreature)
{
    return new npc_taelan_fordringAI(pCreature);
}

bool QuestAccept_npc_taelan_fordring(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_IN_DREAMS)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

bool QuestRewarded_npc_taelan_fordring(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_SCARLET_SUBTERFUGE)
        pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

bool EffectDummyCreature_npc_taelan_fordring(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_TAELAN_DEATH && uiEffIndex == EFFECT_INDEX_0 && pCaster->GetEntry() == NPC_ISILLIEN)
    {
        pCreatureTarget->AI()->EnterEvadeMode();
        pCaster->SetFacingToObject(pCreatureTarget);
        ((Creature*)pCaster)->AI()->EnterEvadeMode();

        return true;
    }

    return false;
}

/*######
## npc_isillien
######*/

struct npc_isillienAI: public npc_escortAI
{
    npc_isillienAI(Creature* pCreature): npc_escortAI(pCreature)
    {
        m_bTirionSpawned = false;
        m_bTaelanDead = false;
        m_uiSummonTirionTimer = 0;
        Reset();
    }

    bool m_bTirionSpawned;
    bool m_bTaelanDead;

    ObjectGuid m_taelanGuid;

    uint32 m_uiSummonTirionTimer;
    uint32 m_uiManaBurnTimer;
    uint32 m_uFlashHealTimer;
    uint32 m_uiGreaterHealTimer;
    uint32 m_uiHolyLightTimer;
    uint32 m_uiDominateTimer;
    uint32 m_uiMindBlastTimer;
    uint32 m_uiMindFlayTimer;

    void Reset() override
    {
        m_uiManaBurnTimer   = urand(7000, 12000);
        m_uFlashHealTimer   = urand(10000, 15000);
        m_uiDominateTimer   = urand(10000, 14000);
        m_uiMindBlastTimer  = urand(0, 1000);
        m_uiMindFlayTimer   = urand(3000, 7000);
        m_uiGreaterHealTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*pWho*/) override
    {
        // attack only on request
    }

    void EnterEvadeMode() override
    {
        // evade and keep the same posiiton
        if (m_bTaelanDead)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
            m_creature->SetLootRecipient(nullptr);

            m_creature->GetMotionMaster()->MoveIdle();

            Reset();
        }
        else
            npc_escortAI::EnterEvadeMode();
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* pSender, Unit* pInvoker, uint32 /*uiMiscValue*/) override
    {
        if (pSender->GetEntry() != NPC_TAELAN_FORDRING)
            return;

        // move outside the tower
        if (eventType == AI_EVENT_START_ESCORT)
            Start(false);
        else if (eventType == AI_EVENT_CUSTOM_A)
        {
            // kill Taelan
            DoCastSpellIfCan(pInvoker, SPELL_TAELAN_DEATH, CAST_INTERRUPT_PREVIOUS);
            m_bTaelanDead = true;
            m_uiSummonTirionTimer = 120000;      // 2 minutes wait before Tirion arrives
            m_taelanGuid = pInvoker->GetObjectGuid();
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 2:
                SetEscortPaused(true);
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        // start event epilog
        if (!m_bTirionSpawned && ((m_uiSummonTirionTimer != 0 && m_uiSummonTirionTimer < uiDiff) || m_creature->GetHealthPercent() < 20.0f))
        {
            if (Creature* pTaelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
                SendAIEvent(AI_EVENT_CUSTOM_B, m_creature, pTaelan);
            m_bTirionSpawned = true;
        }
        else if (m_uiSummonTirionTimer)
            m_uiSummonTirionTimer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // combat spells
        if (m_uiMindBlastTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MIND_BLAST) == CAST_OK)
                m_uiMindBlastTimer = urand(3000, 5000);
        }
        else
            m_uiMindBlastTimer -= uiDiff;

        if (m_uiMindFlayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MIND_FLAY) == CAST_OK)
                m_uiMindFlayTimer = urand(9000, 15000);
        }
        else
            m_uiMindFlayTimer -= uiDiff;

        if (m_uiManaBurnTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MANA_BURN) == CAST_OK)
                m_uiManaBurnTimer = urand(8000, 12000);
        }
        else
            m_uiManaBurnTimer -= uiDiff;

        if (m_uiDominateTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_DOMINATE_MIND, SELECT_FLAG_PLAYER))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_DOMINATE_MIND) == CAST_OK)
                    m_uiDominateTimer = urand(25000, 30000);
            }
        }
        else
            m_uiDominateTimer -= uiDiff;

        if (m_uFlashHealTimer < uiDiff)
        {
            if (Unit* pTarget = DoSelectLowestHpFriendly(50.0f))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_FLASH_HEAL) == CAST_OK)
                    m_uFlashHealTimer = urand(10000, 15000);
            }
        }
        else
            m_uFlashHealTimer -= uiDiff;

        if (m_creature->GetHealthPercent() < 50.0f)
        {
            if (m_uiGreaterHealTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_GREATER_HEAL) == CAST_OK)
                    m_uiGreaterHealTimer = urand(15000, 20000);
            }
            else
                m_uiGreaterHealTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_isillien(Creature* pCreature)
{
    return new npc_isillienAI(pCreature);
}

/*######
## npc_tirion_fordring
######*/

struct npc_tirion_fordringAI: public npc_escortAI
{
    npc_tirion_fordringAI(Creature* pCreature): npc_escortAI(pCreature) { Reset(); }

    ObjectGuid m_taelanGuid;

    uint32 m_uiHolyCleaveTimer;
    uint32 m_uiHolyStrikeTimer;
    uint32 m_uiCrusaderStrike;

    void Reset() override
    {
        m_uiHolyCleaveTimer = urand(11000, 15000);
        m_uiHolyStrikeTimer = urand(6000, 8000);
        m_uiCrusaderStrike  = urand(1000, 5000);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_DEVOTION_AURA);
    }

    void MoveInLineOfSight(Unit* /*pWho*/) override
    {
        // attack only on request
    }

    void EnterEvadeMode() override
    {
        m_creature->RemoveAllAurasOnEvade();
        m_creature->CombatStop(true);
        m_creature->SetLootRecipient(nullptr);

        // on evade go to Taelan
        if (Creature* pTaelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
        {
            float fX, fY, fZ;
            pTaelan->GetContactPoint(m_creature, fX, fY, fZ);
            m_creature->GetMotionMaster()->MovePoint(200, fX, fY, fZ);
        }

        Reset();
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* pSender, Unit* pInvoker, uint32 /*uiMiscValue*/) override
    {
        if (pSender->GetEntry() != NPC_TAELAN_FORDRING)
            return;

        if (eventType == AI_EVENT_START_ESCORT)
        {
            Start(true);
            m_taelanGuid = pInvoker->GetObjectGuid();
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 2:
                SetEscortPaused(true);

                // unmount and go to Taelan
                m_creature->Unmount();
                if (Creature* pTaelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
                {
                    float fX, fY, fZ;
                    pTaelan->GetContactPoint(m_creature, fX, fY, fZ);
                    m_creature->GetMotionMaster()->MovePoint(100, fX, fY, fZ);
                }
                break;
        }
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId) override
    {
        // custom points; ignore in escort AI
        if (uiPointId == 100 || uiPointId == 200)
            return;

        npc_escortAI::MovementInform(uiMoveType, uiPointId);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // combat spells
        if (m_uiHolyCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_CLEAVE) == CAST_OK)
                m_uiHolyCleaveTimer = urand(12000, 15000);
        }
        else
            m_uiHolyCleaveTimer -= uiDiff;

        if (m_uiHolyStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_STRIKE) == CAST_OK)
                m_uiHolyStrikeTimer = urand(8000, 11000);
        }
        else
            m_uiHolyStrikeTimer -= uiDiff;

        if (m_uiCrusaderStrike < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CRUSADER_STRIKE) == CAST_OK)
                m_uiCrusaderStrike = urand(7000, 9000);
        }
        else
            m_uiCrusaderStrike -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_tirion_fordring(Creature* pCreature)
{
    return new npc_tirion_fordringAI(pCreature);
}

void AddSC_western_plaguelands()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_the_scourge_cauldron";
    pNewScript->GetAI = &GetAI_npc_the_scourge_cauldron;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_taelan_fordring";
    pNewScript->GetAI = &GetAI_npc_taelan_fordring;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_taelan_fordring;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_taelan_fordring;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_npc_taelan_fordring;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_isillien";
    pNewScript->GetAI = &GetAI_npc_isillien;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_tirion_fordring";
    pNewScript->GetAI = &GetAI_npc_tirion_fordring;
    pNewScript->RegisterSelf();
}
