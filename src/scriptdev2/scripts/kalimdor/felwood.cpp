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
SDName: Felwood
SD%Complete: 95
SDComment: Quest support: 4261, 4506, 5203, 7603, 7603 (Summon Pollo Grande)
SDCategory: Felwood
EndScriptData */

/* ContentData
npc_kitten
npc_niby_the_almighty
npc_kroshius
npc_captured_arkonarin
npc_arei
EndContentData */

#include "precompiled.h"
#include "follower_ai.h"
#include "escort_ai.h"
#include "ObjectMgr.h"

/*####
# npc_kitten
####*/

enum
{
    EMOTE_SAB_JUMP              = -1000541,
    EMOTE_SAB_FOLLOW            = -1000542,

    SPELL_CORRUPT_SABER_VISUAL  = 16510,

    QUEST_CORRUPT_SABER         = 4506,
    NPC_WINNA                   = 9996,
    NPC_CORRUPT_SABER           = 10042
};

#define GOSSIP_ITEM_RELEASE     "I want to release the corrupted saber to Winna."

struct npc_kittenAI : public FollowerAI
{
    npc_kittenAI(Creature* pCreature) : FollowerAI(pCreature)
    {
        if (pCreature->GetOwner() && pCreature->GetOwner()->GetTypeId() == TYPEID_PLAYER)
        {
            StartFollow((Player*)pCreature->GetOwner());
            SetFollowPaused(true);
            DoScriptText(EMOTE_SAB_JUMP, m_creature);

            pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            // find a decent way to move to center of moonwell
        }

        m_uiMoonwellCooldown = 7500;
        Reset();
    }

    uint32 m_uiMoonwellCooldown;

    void Reset() override { }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // should not have npcflag by default, so set when expected
        if (!m_creature->getVictim() && !m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP) && HasFollowState(STATE_FOLLOW_INPROGRESS) && pWho->GetEntry() == NPC_WINNA)
        {
            if (m_creature->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    }

    void UpdateFollowerAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (HasFollowState(STATE_FOLLOW_PAUSED))
            {
                if (m_uiMoonwellCooldown < uiDiff)
                {
                    m_creature->CastSpell(m_creature, SPELL_CORRUPT_SABER_VISUAL, false);
                    SetFollowPaused(false);
                }
                else
                    m_uiMoonwellCooldown -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kitten(Creature* pCreature)
{
    return new npc_kittenAI(pCreature);
}

bool EffectDummyCreature_npc_kitten(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_CORRUPT_SABER_VISUAL && uiEffIndex == EFFECT_INDEX_0)
    {
        // Not nice way, however using UpdateEntry will not be correct.
        if (const CreatureInfo* pTemp = GetCreatureTemplateStore(NPC_CORRUPT_SABER))
        {
            pCreatureTarget->SetEntry(pTemp->Entry);
            pCreatureTarget->SetDisplayId(Creature::ChooseDisplayId(pTemp));
            pCreatureTarget->SetName(pTemp->Name);
            pCreatureTarget->SetFloatValue(OBJECT_FIELD_SCALE_X, pTemp->Scale);
        }

        if (Unit* pOwner = pCreatureTarget->GetOwner())
            DoScriptText(EMOTE_SAB_FOLLOW, pCreatureTarget, pOwner);

        // always return true when we are handling this spell and effect
        return true;
    }
    return false;
}

bool GossipHello_npc_corrupt_saber(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_CORRUPT_SABER) == QUEST_STATUS_INCOMPLETE)
    {
        if (GetClosestCreatureWithEntry(pCreature, NPC_WINNA, INTERACTION_DISTANCE))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_RELEASE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_corrupt_saber(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_kittenAI* pKittenAI = dynamic_cast<npc_kittenAI*>(pCreature->AI()))
            pKittenAI->SetFollowComplete();

        pPlayer->AreaExploredOrEventHappens(QUEST_CORRUPT_SABER);
    }

    return true;
}

/*######
## npc_niby_the_almighty (summons el pollo grande)
######*/

enum
{
    QUEST_KROSHIUS     = 7603,

    NPC_IMPSY          = 14470,

    SPELL_SUMMON_POLLO = 23056,

    SAY_NIBY_1         = -1000566,
    SAY_NIBY_2         = -1000567,
    EMOTE_IMPSY_1      = -1000568,
    SAY_IMPSY_1        = -1000569,
    SAY_NIBY_3         = -1000570
};

struct npc_niby_the_almightyAI : public ScriptedAI
{
    npc_niby_the_almightyAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiSummonTimer;
    uint8  m_uiSpeech;

    bool m_bEventStarted;

    void Reset() override
    {
        m_uiSummonTimer = 500;
        m_uiSpeech = 0;

        m_bEventStarted = false;
    }

    void StartEvent()
    {
        Reset();
        m_bEventStarted = true;
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bEventStarted)
        {
            if (m_uiSummonTimer <= uiDiff)
            {
                switch (m_uiSpeech)
                {
                    case 1:
                        m_creature->GetMotionMaster()->Clear();
                        m_creature->GetMotionMaster()->MovePoint(0, 5407.19f, -753.00f, 350.82f);
                        m_uiSummonTimer = 6200;
                        break;
                    case 2:
                        m_creature->SetFacingTo(1.2f);
                        DoScriptText(SAY_NIBY_1, m_creature);
                        m_uiSummonTimer = 3000;
                        break;
                    case 3:
                        DoScriptText(SAY_NIBY_2, m_creature);
                        DoCastSpellIfCan(m_creature, SPELL_SUMMON_POLLO);
                        m_uiSummonTimer = 2000;
                        break;
                    case 4:
                        if (Creature* pImpsy = GetClosestCreatureWithEntry(m_creature, NPC_IMPSY, 20.0))
                        {
                            DoScriptText(EMOTE_IMPSY_1, pImpsy);
                            DoScriptText(SAY_IMPSY_1, pImpsy);
                            m_uiSummonTimer = 2500;
                        }
                        else
                        {
                            // Skip Speech 5
                            m_uiSummonTimer = 40000;
                            ++m_uiSpeech;
                        }
                        break;
                    case 5:
                        DoScriptText(SAY_NIBY_3, m_creature);
                        m_uiSummonTimer = 40000;
                        break;
                    case 6:
                        m_creature->GetMotionMaster()->MoveTargetedHome();
                        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        m_bEventStarted = false;
                        break;
                }
                ++m_uiSpeech;
            }
            else
                m_uiSummonTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_niby_the_almighty(Creature* pCreature)
{
    return new npc_niby_the_almightyAI(pCreature);
}

bool QuestRewarded_npc_niby_the_almighty(Player* /*pPlayer*/, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_KROSHIUS)
    {
        if (npc_niby_the_almightyAI* pNibyAI = dynamic_cast<npc_niby_the_almightyAI*>(pCreature->AI()))
        {
            pNibyAI->StartEvent();
        }
    }
    return true;
}

/*######
## npc_kroshius
######*/

enum
{
    NPC_KROSHIUS            = 14467,
    SPELL_KNOCKBACK         = 10101,
    SAY_KROSHIUS_REVIVE     = -1000589,
    EVENT_KROSHIUS_REVIVE   = 8328,
    FACTION_HOSTILE         = 16,
};

struct npc_kroshiusAI : public ScriptedAI
{
    npc_kroshiusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiPhase = 0;
        Reset();
    }

    ObjectGuid m_playerGuid;
    uint32 m_uiKnockBackTimer;
    uint32 m_uiPhaseTimer;

    uint8 m_uiPhase;

    void Reset() override
    {
        m_uiKnockBackTimer = urand(5000, 8000);
        m_playerGuid.Clear();

        if (!m_uiPhase)
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    void DoRevive(Player* pSource)
    {
        if (m_uiPhase)
            return;

        m_uiPhase = 1;
        m_uiPhaseTimer = 2500;
        m_playerGuid = pSource->GetObjectGuid();

        // TODO: A visual Flame Circle around the mob still missing
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        m_uiPhase = 0;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_uiPhase)
            return;

        if (m_uiPhase < 4)
        {
            if (m_uiPhaseTimer < uiDiff)
            {
                switch (m_uiPhase)
                {
                    case 1:                                 // Revived
                        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                        m_uiPhaseTimer = 1000;
                        break;
                    case 2:
                        DoScriptText(SAY_KROSHIUS_REVIVE, m_creature);
                        m_uiPhaseTimer = 3500;
                        break;
                    case 3:                                 // Attack
                        m_creature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_COMBAT_STOP | TEMPFACTION_RESTORE_RESPAWN |  TEMPFACTION_TOGGLE_OOC_NOT_ATTACK | TEMPFACTION_TOGGLE_PASSIVE);
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                        {
                            if (m_creature->IsWithinDistInMap(pPlayer, 30.0f))
                                AttackStart(pPlayer);
                        }
                        break;
                }
                ++m_uiPhase;
            }
            else
                m_uiPhaseTimer -= uiDiff;
        }
        else
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

            if (m_uiKnockBackTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCKBACK);
                m_uiKnockBackTimer = urand(9000, 12000);
            }
            else
                m_uiKnockBackTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_npc_kroshius(Creature* pCreature)
{
    return new npc_kroshiusAI(pCreature);
}

bool ProcessEventId_npc_kroshius(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
    if (uiEventId == EVENT_KROSHIUS_REVIVE)
    {
        if (pSource->GetTypeId() == TYPEID_PLAYER)
        {
            if (Creature* pKroshius = GetClosestCreatureWithEntry((Player*)pSource, NPC_KROSHIUS, 20.0f))
            {
                if (npc_kroshiusAI* pKroshiusAI = dynamic_cast<npc_kroshiusAI*>(pKroshius->AI()))
                    pKroshiusAI->DoRevive((Player*)pSource);
            }
        }

        return true;
    }
    return false;
}

/*####
# npc_captured_arkonarin
####*/

enum
{
    SAY_ESCORT_START                = -1001148,
    SAY_FIRST_STOP                  = -1001149,
    SAY_SECOND_STOP                 = -1001150,
    SAY_AGGRO                       = -1001151,
    SAY_FOUND_EQUIPMENT             = -1001152,
    SAY_ESCAPE_DEMONS               = -1001153,
    SAY_FRESH_AIR                   = -1001154,
    SAY_TREY_BETRAYER               = -1001155,
    SAY_TREY                        = -1001156,
    SAY_TREY_ATTACK                 = -1001157,
    SAY_ESCORT_COMPLETE             = -1001158,

    SPELL_STRENGTH_ARKONARIN        = 18163,
    SPELL_MORTAL_STRIKE             = 16856,
    SPELL_CLEAVE                    = 15496,

    QUEST_ID_RESCUE_JAEDENAR        = 5203,
    NPC_JAEDENAR_LEGIONNAIRE        = 9862,
    NPC_SPIRT_TREY                  = 11141,
    NPC_ARKONARIN                   = 11018,
    GO_ARKONARIN_CHEST              = 176225,
    GO_ARKONARIN_CAGE               = 176306,
};

struct npc_captured_arkonarinAI : public npc_escortAI
{
    npc_captured_arkonarinAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    ObjectGuid m_treyGuid;

    bool m_bCanAttack;

    uint32 m_uiMortalStrikeTimer;
    uint32 m_uiCleaveTimer;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_bCanAttack = false;

        m_uiMortalStrikeTimer = urand(5000, 7000);
        m_uiCleaveTimer = urand(1000, 4000);
    }

    void Aggro(Unit* pWho) override
    {
        if (pWho->GetEntry() == NPC_SPIRT_TREY)
            DoScriptText(SAY_TREY_ATTACK, m_creature);
        else if (roll_chance_i(25))
            DoScriptText(SAY_AGGRO, m_creature, pWho);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_JAEDENAR_LEGIONNAIRE)
            pSummoned->AI()->AttackStart(m_creature);
        else if (pSummoned->GetEntry() == NPC_SPIRT_TREY)
        {
            DoScriptText(SAY_TREY_BETRAYER, pSummoned);
            m_treyGuid = pSummoned->GetObjectGuid();
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue));

            if (GameObject* pCage = GetClosestGameObjectWithEntry(m_creature, GO_ARKONARIN_CAGE, 5.0f))
                pCage->Use(m_creature);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 0:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_ESCORT_START, m_creature, pPlayer);
                break;
            case 14:
                DoScriptText(SAY_FIRST_STOP, m_creature);
                break;
            case 34:
                DoScriptText(SAY_SECOND_STOP, m_creature);
                SetRun();
                break;
            case 38:
                if (GameObject* pChest = GetClosestGameObjectWithEntry(m_creature, GO_ARKONARIN_CHEST, 5.0f))
                    pChest->Use(m_creature);
                m_creature->HandleEmote(EMOTE_ONESHOT_KNEEL);
                break;
            case 39:
                DoCastSpellIfCan(m_creature, SPELL_STRENGTH_ARKONARIN);
                break;
            case 40:
                m_creature->UpdateEntry(NPC_ARKONARIN);
                if (Player* pPlayer = GetPlayerForEscort())
                    m_creature->SetFacingToObject(pPlayer);
                m_bCanAttack = true;
                DoScriptText(SAY_FOUND_EQUIPMENT, m_creature);
                break;
            case 41:
                DoScriptText(SAY_ESCAPE_DEMONS, m_creature);
                m_creature->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5082.068f, -490.084f, 296.856f, 5.15f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5084.135f, -489.187f, 296.832f, 5.15f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5085.676f, -488.518f, 296.824f, 5.15f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
                break;
            case 43:
                SetRun(false);
                break;
            case 104:
                DoScriptText(SAY_FRESH_AIR, m_creature);
                break;
            case 105:
                m_creature->SummonCreature(NPC_SPIRT_TREY, 4844.839f, -395.763f, 350.603f, 6.25f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
                break;
            case 106:
                DoScriptText(SAY_TREY, m_creature);
                break;
            case 107:
                if (Creature* pTrey = m_creature->GetMap()->GetCreature(m_treyGuid))
                    AttackStart(pTrey);
                break;
            case 108:
                if (Player* pPlayer = GetPlayerForEscort())
                    m_creature->SetFacingToObject(pPlayer);
                DoScriptText(SAY_ESCORT_COMPLETE, m_creature);
                break;
            case 109:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ID_RESCUE_JAEDENAR, m_creature);
                SetRun();
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bCanAttack)
        {
            if (m_uiMortalStrikeTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_STRIKE) == CAST_OK)
                    m_uiMortalStrikeTimer = urand(7000, 10000);
            }
            else
                m_uiMortalStrikeTimer -= uiDiff;

            if (m_uiCleaveTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                    m_uiCleaveTimer = urand(3000, 6000);
            }
            else
                m_uiCleaveTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captured_arkonarin(Creature* pCreature)
{
    return new npc_captured_arkonarinAI(pCreature);
}

bool QuestAccept_npc_captured_arkonarin(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_RESCUE_JAEDENAR)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

/*####
# npc_arei
####*/

enum
{
    SAY_AREI_ESCORT_START           = -1001159,
    SAY_ATTACK_IRONTREE             = -1001160,
    SAY_ATTACK_TOXIC_HORROR         = -1001161,
    SAY_EXIT_WOODS                  = -1001162,
    SAY_CLEAR_PATH                  = -1001163,
    SAY_ASHENVALE                   = -1001164,
    SAY_TRANSFORM                   = -1001165,
    SAY_LIFT_CURSE                  = -1001166,
    SAY_AREI_ESCORT_COMPLETE        = -1001167,

    SPELL_WITHER_STRIKE             = 5337,
    SPELL_AREI_TRANSFORM            = 14888,

    NPC_AREI                        = 9598,
    NPC_TOXIC_HORROR                = 7132,
    NPC_IRONTREE_WANDERER           = 7138,
    NPC_IRONTREE_STOMPER            = 7139,
    QUEST_ID_ANCIENT_SPIRIT         = 4261,
};

static const DialogueEntry aEpilogDialogue[] =
{
    {SAY_CLEAR_PATH,            NPC_AREI,   4000},
    {SPELL_WITHER_STRIKE,       0,          5000},
    {SAY_TRANSFORM,             NPC_AREI,   3000},
    {SPELL_AREI_TRANSFORM,      0,          7000},
    {QUEST_ID_ANCIENT_SPIRIT,   0,          0},
    {0, 0, 0},
};

struct npc_areiAI : public npc_escortAI, private DialogueHelper
{
    npc_areiAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aEpilogDialogue)
    {
        m_bAggroIrontree = false;
        m_bAggroHorror = false;
        Reset();
    }

    uint32 m_uiWitherStrikeTimer;

    bool m_bAggroIrontree;
    bool m_bAggroHorror;

    GuidList m_lSummonsGuids;

    void Reset() override
    {
        m_uiWitherStrikeTimer = urand(1000, 4000);
    }

    void Aggro(Unit* pWho) override
    {
        if ((pWho->GetEntry() == NPC_IRONTREE_WANDERER || pWho->GetEntry() == NPC_IRONTREE_STOMPER) && !m_bAggroIrontree)
        {
            DoScriptText(SAY_ATTACK_IRONTREE, m_creature);
            m_bAggroIrontree = true;
        }
        else if (pWho->GetEntry() == NPC_TOXIC_HORROR && ! m_bAggroHorror)
        {
            if (Player* pPlayer = GetPlayerForEscort())
                DoScriptText(SAY_ATTACK_TOXIC_HORROR, m_creature, pPlayer);
            m_bAggroHorror = true;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_IRONTREE_STOMPER:
                DoScriptText(SAY_EXIT_WOODS, m_creature, pSummoned);
                // no break;
            case NPC_IRONTREE_WANDERER:
                pSummoned->AI()->AttackStart(m_creature);
                m_lSummonsGuids.push_back(pSummoned->GetObjectGuid());
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_IRONTREE_STOMPER || pSummoned->GetEntry() == NPC_IRONTREE_WANDERER)
        {
            m_lSummonsGuids.remove(pSummoned->GetObjectGuid());

            if (m_lSummonsGuids.empty())
                StartNextDialogueText(SAY_CLEAR_PATH);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_AREI_ESCORT_START, m_creature, pInvoker);

            m_creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
            Start(true, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue));
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        if (uiPointId == 36)
        {
            SetEscortPaused(true);

            m_creature->SummonCreature(NPC_IRONTREE_STOMPER, 6573.321f, -1195.213f, 442.489f, 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
            m_creature->SummonCreature(NPC_IRONTREE_WANDERER, 6573.240f, -1213.475f, 443.643f, 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
            m_creature->SummonCreature(NPC_IRONTREE_WANDERER, 6583.354f, -1209.811f, 444.769f, 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        if (uiEntry == NPC_AREI)
            return m_creature;

        return nullptr;
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case SPELL_WITHER_STRIKE:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_ASHENVALE, m_creature, pPlayer);
                break;
            case SPELL_AREI_TRANSFORM:
                DoCastSpellIfCan(m_creature, SPELL_AREI_TRANSFORM);
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_LIFT_CURSE, m_creature, pPlayer);
                break;
            case QUEST_ID_ANCIENT_SPIRIT:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_AREI_ESCORT_COMPLETE, m_creature, pPlayer);
                    pPlayer->GroupEventHappens(QUEST_ID_ANCIENT_SPIRIT, m_creature);
                    m_creature->ForcedDespawn(10000);
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiWitherStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WITHER_STRIKE) == CAST_OK)
                m_uiWitherStrikeTimer = urand(3000, 6000);
        }
        else
            m_uiWitherStrikeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_arei(Creature* pCreature)
{
    return new npc_areiAI(pCreature);
}

bool QuestAccept_npc_arei(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_ANCIENT_SPIRIT)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

void AddSC_felwood()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_kitten";
    pNewScript->GetAI = &GetAI_npc_kitten;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_npc_kitten;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_corrupt_saber";
    pNewScript->pGossipHello = &GossipHello_npc_corrupt_saber;
    pNewScript->pGossipSelect = &GossipSelect_npc_corrupt_saber;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_niby_the_almighty";
    pNewScript->GetAI = &GetAI_npc_niby_the_almighty;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_niby_the_almighty;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kroshius";
    pNewScript->GetAI = &GetAI_npc_kroshius;
    pNewScript->pProcessEventId = &ProcessEventId_npc_kroshius;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captured_arkonarin";
    pNewScript->GetAI = &GetAI_npc_captured_arkonarin;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_captured_arkonarin;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_arei";
    pNewScript->GetAI = &GetAI_npc_arei;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_arei;
    pNewScript->RegisterSelf();
}
