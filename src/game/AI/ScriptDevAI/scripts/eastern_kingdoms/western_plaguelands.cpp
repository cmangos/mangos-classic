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

/* ContentData
npc_the_scourge_cauldron
npc_taelan_fordring
npc_isillien
npc_tirion_fordring
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "Spells/Scripts/SpellScript.h"

/*######
## npc_the_scourge_cauldron
######*/

struct npc_the_scourge_cauldronAI : public ScriptedAI
{
    npc_the_scourge_cauldronAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() override {}

    void DoDie()
    {
        // summoner dies here - TODO: Check suicide spell
        m_creature->Suicide();
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

UnitAI* GetAI_npc_the_scourge_cauldron(Creature* creature)
{
    return new npc_the_scourge_cauldronAI(creature);
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
    NPC_TIRION_FORDRING             = 12126,
    NPC_CRIMSON_ELITE               = 12128,

    MODEL_TAELAN_MOUNT              = 239,

    // quests
    QUEST_ID_SCARLET_SUBTERFUGE     = 5862,
    QUEST_ID_IN_DREAMS              = 5944,

    TIRION_KNEELS_1                 = 1,
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
    {SAY_REACH_TOWER,               NPC_TAELAN_FORDRING,        2000},
    {SAY_ISILLIEN_1,                NPC_ISILLIEN,               5000},
    {SAY_ISILLIEN_2,                NPC_TAELAN_FORDRING,        4000},
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
    {SAY_EPILOG_1,                  NPC_TIRION_FORDRING,        3000},
    {TIRION_KNEELS_1,               0,                          3000},
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
    npc_taelan_fordringAI(Creature* creature): npc_escortAI(creature), DialogueHelper(aScarletDialogue)
    {
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);

        m_scarletComplete = false;
        m_fightStarted = false;
        m_taelanDead = false;
        m_hasMount = false;
        Reset();
    }

    bool m_scarletComplete;
    bool m_fightStarted;
    bool m_hasMount;
    bool m_taelanDead;

    ObjectGuid m_isillenGuid;
    ObjectGuid m_tirionGuid;
    GuidList m_lCavalierGuids;

    uint32 m_holyCleaveTimer;
    uint32 m_holyStrikeTimer;
    uint32 m_crusaderStrike;
    uint32 m_holyLightTimer;

    void Reset() override
    {
        m_holyCleaveTimer = urand(11000, 15000);
        m_holyStrikeTimer = urand(6000, 8000);
        m_crusaderStrike  = urand(1000, 5000);
        m_holyLightTimer  = 0;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_hasMount)
            m_creature->Unmount();

        DoCastSpellIfCan(m_creature, SPELL_DEVOTION_AURA);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (m_taelanDead)
            return;

        npc_escortAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode() override
    {
        if (m_taelanDead)
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
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_creature->ClearAllReactives();
            m_creature->SetImmobilizedState(true, true);
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

            Reset();
        }
        else
        {
            if (m_hasMount)
                m_creature->Mount(MODEL_TAELAN_MOUNT);

            npc_escortAI::EnterEvadeMode();
        }
    }

    void JustReachedHome() override
    {
        if (m_scarletComplete)
        {
            StartNextDialogueText(SPELL_DEVOTION_AURA);
            m_scarletComplete = false;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 miscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && invoker->GetTypeId() == TYPEID_PLAYER)
        {
            Start(false, (Player*)invoker, GetQuestTemplateStore(miscValue));
            DoScriptText(SAY_ESCORT_START, m_creature);
            m_creature->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
        else if (eventType == AI_EVENT_CUSTOM_A && invoker->GetTypeId() == TYPEID_PLAYER && miscValue == QUEST_ID_SCARLET_SUBTERFUGE)
            StartNextDialogueText(NPC_SCARLET_CAVALIER);
        else if (eventType == AI_EVENT_CUSTOM_B && invoker->GetEntry() == NPC_ISILLIEN)
        {
            StartNextDialogueText(NPC_TIRION_FORDRING);
            if (Creature* tirion = m_creature->SummonCreature(NPC_TIRION_FORDRING, 2620.273f, -1920.917f, 74.25f, 0, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
            {
                tirion->SetWalk(false);
                tirion->SetImmuneToNPC(true);  // Temporary make Tirion immune to nearby Scarlet NPCs so the script can run smoothly
            }
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 26:
                SetEscortPaused(true);
                StartNextDialogueText(SAY_EXIT_KEEP);
                break;
            case 56:
                SetEscortPaused(true);
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveIdle();
                StartNextDialogueText(SAY_REACH_TOWER);
                break;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            case NPC_ISILLIEN:
                SendAIEvent(AI_EVENT_START_ESCORT, m_creature, summoned);
                m_isillenGuid = summoned->GetObjectGuid();

                // summon additional crimson elites
                float fX, fY, fZ;
                summoned->GetNearPoint(summoned, fX, fY, fZ, 0, 5.0f, M_PI_F * 1.25f);
                summoned->SummonCreature(NPC_CRIMSON_ELITE, fX, fY, fZ, summoned->GetOrientation(), TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                summoned->GetNearPoint(summoned, fX, fY, fZ, 0, 5.0f, 0);
                summoned->SummonCreature(NPC_CRIMSON_ELITE, fX, fY, fZ, summoned->GetOrientation(), TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                break;
            case NPC_TIRION_FORDRING:
                m_tirionGuid = summoned->GetObjectGuid();
                SendAIEvent(AI_EVENT_START_ESCORT, m_creature, summoned);
                break;
        }
    }

    void SummonedMovementInform(Creature* summoned, uint32 /*motionType*/, uint32 pointId) override
    {
        if (summoned->GetEntry() != NPC_TIRION_FORDRING)
            return;

        if (pointId == 100)
        {
            StartNextDialogueText(NPC_ISILLIEN);
            summoned->SetFacingToObject(m_creature);
            summoned->SetStandState(UNIT_STAND_STATE_KNEEL);
        }
        else if (pointId == 200)
            StartNextDialogueText(SAY_EPILOG_1);
    }

    void JustDidDialogueStep(int32 entry) override
    {
        switch (entry)
        {
            case NPC_SCARLET_CAVALIER:
            {
                // kneel and make everyone worried
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

                CreatureList lCavaliersInRange;
                GetCreatureListWithEntryInGrid(lCavaliersInRange, m_creature, NPC_SCARLET_CAVALIER, 10.0f);

                uint8 index = 0;
                for (auto* cavalier : lCavaliersInRange)
                {
                    if (cavalier->IsAlive() && !cavalier->IsInCombat() && cavalier->GetPositionZ() > 164.0f) // Prevent picking NPC from another floor
                    {
                        m_lCavalierGuids.push_back(cavalier->GetObjectGuid());
                        cavalier->SetFacingToObject(m_creature);
                        DoScriptText(aCavalierYells[index], cavalier);
                        ++index;
                    }
                }
                break;
            }
            case QUEST_ID_SCARLET_SUBTERFUGE:
            {
                float fX, fY, fZ;
                for (GuidList::const_iterator itr = m_lCavalierGuids.begin(); itr != m_lCavalierGuids.end(); ++itr)
                {
                    if (Creature* cavalier = m_creature->GetMap()->GetCreature(*itr))
                    {
                        m_creature->GetContactPoint(cavalier, fX, fY, fZ);
                        cavalier->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
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
                m_creature->SetFactionTemporary(FACTION_ESCORT_N_FRIEND_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
                for (GuidList::const_iterator itr = m_lCavalierGuids.begin(); itr != m_lCavalierGuids.end(); ++itr)
                {
                    if (Creature* cavalier = m_creature->GetMap()->GetCreature(*itr))
                        cavalier->AI()->AttackStart(m_creature);
                }
                m_scarletComplete = true;
                break;
            case SAY_SCARLET_COMPLETE_2:
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                break;
            case MODEL_TAELAN_MOUNT:
                // mount when outside
                m_hasMount = true;
                SetEscortPaused(false);
                m_creature->Mount(MODEL_TAELAN_MOUNT);
                break;
            case SAY_REACH_TOWER:
                // start fight event
                if (Player* player = GetPlayerForEscort())
                    m_creature->SetFacingToObject(player);
                m_creature->SummonCreature(NPC_ISILLIEN, 2693.12f, -1943.04f, 72.04f, 2.11f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
                break;
            case SAY_ISILLIEN_2:
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                    m_creature->SetFacingToObject(isillien);
                break;
            case SAY_ISILLIEN_3:
                m_creature->Unmount();
                break;
            case SPELL_CRUSADER_STRIKE:
            {
                float fX, fY, fZ;
                // spawn 3 additional elites
                if (Creature* elite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2711.32f, -1882.67f, 67.89f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    elite->SetWalk(false);
                    m_creature->GetContactPoint(elite, fX, fY, fZ);
                    elite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
                if (Creature* elite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2710.93f, -1878.90f, 67.97f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    elite->SetWalk(false);
                    m_creature->GetContactPoint(elite, fX, fY, fZ);
                    elite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
                if (Creature* elite = m_creature->SummonCreature(NPC_CRIMSON_ELITE, 2710.53f, -1875.28f, 67.90f, 3.2f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS))
                {
                    elite->SetWalk(false);
                    m_creature->GetContactPoint(elite, fX, fY, fZ);
                    elite->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }

                // Isillien only attacks Taelan
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    isillien->AI()->AttackStart(m_creature);
                    AttackStart(isillien);
                }

                m_fightStarted = true;
                break;
            }
            case SAY_KILL_TAELAN_1:
                // Kill Taelan and attack players
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                    SendAIEvent(AI_EVENT_CUSTOM_A, m_creature, isillien);
                break;
            case EMOTE_ATTACK_PLAYER:
                // Attack players
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    if (Player* player = GetPlayerForEscort())
                        isillien->AI()->AttackStart(player);
                }
                break;
            // Tirion event
            case SAY_TIRION_1:
                if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                {
                    tirion->SetStandState(UNIT_STAND_STATE_STAND);
                    if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                    {
                        tirion->SetFacingToObject(isillien);
                        isillien->CombatStop();
                    }
                }
                break;
            case SAY_TIRION_5:
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    {
                        tirion->SetImmuneToNPC(false);
                        tirion->AI()->AttackStart(isillien);
                        isillien->AI()->AttackStart(tirion);
                    }
                }
                break;
            // Epilogue dialogue
            case SAY_EPILOG_1:
                if (Creature* isillien = m_creature->GetMap()->GetCreature(m_isillenGuid))
                {
                    if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                        tirion->SetFacingToObject(isillien);
                }
                break;
            case TIRION_KNEELS_1:
                if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    tirion->SetFacingToObject(m_creature);
                break;
            case EMOTE_HOLD_TAELAN:
                if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    tirion->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case SAY_EPILOG_4:
                if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                    tirion->SetStandState(UNIT_STAND_STATE_STAND);
                break;
            case SAY_EPILOG_5:
                if (Creature* tirion = m_creature->GetMap()->GetCreature(m_tirionGuid))
                {
                    if (Player* player = GetPlayerForEscort())
                        player->RewardPlayerAndGroupAtEventExplored(QUEST_ID_IN_DREAMS, m_creature);

                    tirion->ForcedDespawn(3 * MINUTE * IN_MILLISECONDS);
                    tirion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                m_creature->ForcedDespawn(3 * MINUTE * IN_MILLISECONDS);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 entry) override
    {
        switch (entry)
        {
            case NPC_TAELAN_FORDRING:   return m_creature;
            case NPC_ISILLIEN:          return m_creature->GetMap()->GetCreature(m_isillenGuid);
            case NPC_TIRION_FORDRING:   return m_creature->GetMap()->GetCreature(m_tirionGuid);

            default:
                return nullptr;
        }
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        DialogueUpdate(diff);

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_taelanDead)
            return;

        // If Taelan is in fight with Isillien and drops below 50% make him "dead"
        if (!m_taelanDead && m_fightStarted && m_creature->GetHealthPercent() < 50.0f)
        {
            StartNextDialogueText(SAY_KILL_TAELAN_1);
            m_taelanDead = true;
        }

        if (m_holyCleaveTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLY_CLEAVE) == CAST_OK)
                m_holyCleaveTimer = urand(11000, 13000);
        }
        else
            m_holyCleaveTimer -= diff;

        if (m_holyStrikeTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLY_STRIKE) == CAST_OK)
                m_holyStrikeTimer = urand(9000, 14000);
        }
        else
            m_holyStrikeTimer -= diff;

        if (m_crusaderStrike < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CRUSADER_STRIKE) == CAST_OK)
                m_crusaderStrike = urand(7000, 12000);
        }
        else
            m_crusaderStrike -= diff;

        if (m_creature->GetHealthPercent() < 75.0f)
        {
            if (m_holyLightTimer < diff)
            {
                if (Unit* target = DoSelectLowestHpFriendly(50.0f))
                {
                    if (DoCastSpellIfCan(target, SPELL_HOLY_LIGHT) == CAST_OK)
                        m_holyLightTimer = urand(10000, 15000);
                }
            }
            else
                m_holyLightTimer -= diff;
        }

        if (!m_fightStarted && m_creature->GetHealthPercent() < 15.0f)
            DoCastSpellIfCan(m_creature, SPELL_LAY_ON_HANDS);

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_taelan_fordring(Creature* creature)
{
    return new npc_taelan_fordringAI(creature);
}

bool QuestAccept_npc_taelan_fordring(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_ID_IN_DREAMS)
        creature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, player, creature, quest->GetQuestId());

    return true;
}

bool QuestRewarded_npc_taelan_fordring(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_ID_SCARLET_SUBTERFUGE)
        creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, player, creature, quest->GetQuestId());

    return true;
}

bool EffectDummyCreature_npc_taelan_fordring(Unit* caster, uint32 spellId, SpellEffectIndex effIndex, Creature* creatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (spellId == SPELL_TAELAN_DEATH && effIndex == EFFECT_INDEX_0 && caster->GetEntry() == NPC_ISILLIEN)
    {
        creatureTarget->AI()->EnterEvadeMode();
        caster->SetFacingToObject(creatureTarget);
        ((Creature*)caster)->AI()->EnterEvadeMode();

        return true;
    }

    return false;
}

/*######
## npc_isillien
######*/

struct npc_isillienAI: public npc_escortAI
{
    npc_isillienAI(Creature* creature): npc_escortAI(creature)
    {
        m_tirionSpawned = false;
        m_taelanDead = false;
        m_summonTirionTimer = 0;
        Reset();
    }

    bool m_tirionSpawned;
    bool m_taelanDead;

    ObjectGuid m_taelanGuid;

    uint32 m_summonTirionTimer;
    uint32 m_manaBurnTimer;
    uint32 m_flashHealTimer;
    uint32 m_greaterHealTimer;
    uint32 m_holyLightTimer;
    uint32 m_dominateTimer;
    uint32 m_mindBlastTimer;
    uint32 m_mindFlayTimer;

    void Reset() override
    {
        m_manaBurnTimer   = urand(7000, 12000);
        m_flashHealTimer   = urand(10000, 15000);
        m_dominateTimer   = urand(10000, 14000);
        m_mindBlastTimer  = urand(0, 1000);
        m_mindFlayTimer   = urand(3000, 7000);
        m_greaterHealTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*who*/) override
    {
        // attack only on request
    }

    void EnterEvadeMode() override
    {
        // evade and keep the same posiiton
        if (m_taelanDead)
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

    void ReceiveAIEvent(AIEventType eventType, Unit* sender, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (sender->GetEntry() != NPC_TAELAN_FORDRING)
            return;

        // move outside the tower
        if (eventType == AI_EVENT_START_ESCORT)
            Start(false);
        else if (eventType == AI_EVENT_CUSTOM_A)
        {
            // kill Taelan
            DoCastSpellIfCan(invoker, SPELL_TAELAN_DEATH, CAST_INTERRUPT_PREVIOUS);
            m_taelanDead = true;
            m_summonTirionTimer = 120000;      // 2 minutes wait before Tirion arrives
            m_taelanGuid = invoker->GetObjectGuid();
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 3:
                SetEscortPaused(true);
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        // Start event epilogue
        if (!m_tirionSpawned && ((m_summonTirionTimer != 0 && m_summonTirionTimer < diff) || m_creature->GetHealthPercent() < 20.0f))
        {
            if (Creature* taelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
                SendAIEvent(AI_EVENT_CUSTOM_B, m_creature, taelan);
            m_tirionSpawned = true;
        }
        else if (m_summonTirionTimer)
            m_summonTirionTimer -= diff;

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Combat spells
        if (m_mindBlastTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MIND_BLAST) == CAST_OK)
                m_mindBlastTimer = urand(3000, 5000);
        }
        else
            m_mindBlastTimer -= diff;

        if (m_mindFlayTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MIND_FLAY) == CAST_OK)
                m_mindFlayTimer = urand(9000, 15000);
        }
        else
            m_mindFlayTimer -= diff;

        if (m_manaBurnTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MANA_BURN) == CAST_OK)
                m_manaBurnTimer = urand(8000, 12000);
        }
        else
            m_manaBurnTimer -= diff;

        if (m_dominateTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_DOMINATE_MIND, SELECT_FLAG_PLAYER))
            {
                if (DoCastSpellIfCan(target, SPELL_DOMINATE_MIND) == CAST_OK)
                    m_dominateTimer = urand(25000, 30000);
            }
        }
        else
            m_dominateTimer -= diff;

        if (m_flashHealTimer < diff)
        {
            if (Unit* target = DoSelectLowestHpFriendly(50.0f))
            {
                if (DoCastSpellIfCan(target, SPELL_FLASH_HEAL) == CAST_OK)
                    m_flashHealTimer = urand(10000, 15000);
            }
        }
        else
            m_flashHealTimer -= diff;

        if (m_creature->GetHealthPercent() < 50.0f)
        {
            if (m_greaterHealTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_GREATER_HEAL) == CAST_OK)
                    m_greaterHealTimer = urand(15000, 20000);
            }
            else
                m_greaterHealTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_isillien(Creature* creature)
{
    return new npc_isillienAI(creature);
}

/*######
## npc_tirion_fordring
######*/

struct npc_tirion_fordringAI: public npc_escortAI
{
    npc_tirion_fordringAI(Creature* creature): npc_escortAI(creature) { Reset(); }

    ObjectGuid m_taelanGuid;

    uint32 m_holyCleaveTimer;
    uint32 m_holyStrikeTimer;
    uint32 m_crusaderStrike;

    void Reset() override
    {
        m_holyCleaveTimer = urand(11000, 15000);
        m_holyStrikeTimer = urand(6000, 8000);
        m_crusaderStrike  = urand(1000, 5000);
    }

    void Aggro(Unit* /*who*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_DEVOTION_AURA);
    }

    void MoveInLineOfSight(Unit* /*who*/) override
    {
        // attack only on request
    }

    void EnterEvadeMode() override
    {
        m_creature->RemoveAllAurasOnEvade();
        m_creature->CombatStop(true);
        m_creature->SetLootRecipient(nullptr);

        // on evade go to Taelan
        if (Creature* taelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
        {
            float fX, fY, fZ;
            taelan->GetContactPoint(m_creature, fX, fY, fZ);
            m_creature->GetMotionMaster()->MovePoint(200, fX, fY, fZ);
        }

        Reset();
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* sender, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (sender->GetEntry() != NPC_TAELAN_FORDRING)
            return;

        if (eventType == AI_EVENT_START_ESCORT)
        {
            Start(true);
            m_taelanGuid = invoker->GetObjectGuid();
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 3:
                SetEscortPaused(true);

                // unmount and go to Taelan
                m_creature->Unmount();
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveIdle();
                if (Creature* taelan = m_creature->GetMap()->GetCreature(m_taelanGuid))
                {
                    float fX, fY, fZ;
                    taelan->GetContactPoint(m_creature, fX, fY, fZ);
                    m_creature->GetMotionMaster()->MovePoint(100, fX, fY, fZ);
                }
                break;
        }
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        // custom points; ignore in escort AI
        if (pointId == 100 || pointId == 200)
            return;

        npc_escortAI::MovementInform(moveType, pointId);
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // combat spells
        if (m_holyCleaveTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLY_CLEAVE) == CAST_OK)
                m_holyCleaveTimer = urand(12000, 15000);
        }
        else
            m_holyCleaveTimer -= diff;

        if (m_holyStrikeTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLY_STRIKE) == CAST_OK)
                m_holyStrikeTimer = urand(8000, 11000);
        }
        else
            m_holyStrikeTimer -= diff;

        if (m_crusaderStrike < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CRUSADER_STRIKE) == CAST_OK)
                m_crusaderStrike = urand(7000, 9000);
        }
        else
            m_crusaderStrike -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_tirion_fordring(Creature* creature)
{
    return new npc_tirion_fordringAI(creature);
}

/*######
## spell_placing_beacon_torch
######*/

struct spell_placing_beacon_torch : public SpellScript
{
    void OnRadiusCalculate(Spell* /*spell*/, SpellEffectIndex effIdx, bool /*targetB*/, float& radius) const override
    {
        if (effIdx == EFFECT_INDEX_0)
            radius = 0.f;
        if (effIdx == EFFECT_INDEX_1)
            radius = 10.f;
    }
};


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

    RegisterSpellScript<spell_placing_beacon_torch>("spell_placing_beacon_torch");
}
