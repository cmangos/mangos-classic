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
SDName: Boss_Vaelastrasz
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackwing_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_LINE_1                      = -1469026,
    SAY_LINE_2                      = -1469027,
    SAY_LINE_3                      = -1469028,
    SAY_HALFLIFE                    = -1469029,
    SAY_KILLTARGET                  = -1469030,
    SAY_NEFARIUS_CORRUPT_1          = -1469006,                 // When he corrupts Vaelastrasz
    SAY_NEFARIUS_CORRUPT_2          = -1469032,
    SAY_TECHNICIAN_RUN              = -1469034,

    NPC_BLACKWING_TECHNICIAN        = 13996,                    // Flees at Vael intro event

    SPELL_ESSENCE_OF_THE_RED        = 23513,                    // and 55740 in wotlk
    SPELL_FLAME_BREATH              = 23461,
    SPELL_FIRE_NOVA                 = 23462,
    SPELL_TAIL_SWEEP                = 15847,
    SPELL_BURNING_ADRENALINE_TANK   = 18173,
    SPELL_BURNING_ADRENALINE        = 23620,
    SPELL_CLEAVE                    = 19983,

    SPELL_NEFARIUS_CORRUPTION       = 23642,
    SPELL_RED_LIGHTNING             = 19484,

    GOSSIP_ITEM_VAEL_1              = -3469003,
    GOSSIP_ITEM_VAEL_2              = -3469004,

    GOSSIP_TEXT_VAEL_1              = 7156,
    GOSSIP_TEXT_VAEL_2              = 7256,

    FACTION_HOSTILE                 = 14,

    AREATRIGGER_VAEL_INTRO          = 3626,

    QUEST_NEFARIUS_CORRUPTION       = 8730,
};

enum VaelaastraszActions
{
    VAEL_LOW_HP_YELL,
    VAEL_BURNING_ADRENALINE_TANK,
    VAEL_BURNING_ADRENALINE_TARGET,
    VAEL_FLAME_BREATH,
    VAEL_FIRE_NOVA,
    VAEL_CLEAVE,
    VAEL_TAIL_SWEEP,
    VAEL_ACTION_MAX,
    VAEL_INTRO,
    VAEL_SPEECH,
};

struct boss_vaelastraszAI : public CombatAI
{
    boss_vaelastraszAI(Creature* creature) : CombatAI(creature, VAEL_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(VAEL_LOW_HP_YELL, true);
        AddCombatAction(VAEL_BURNING_ADRENALINE_TANK, 45000u);
        AddCombatAction(VAEL_BURNING_ADRENALINE_TARGET, 15000u);
        AddCombatAction(VAEL_FLAME_BREATH, 11000u);
        AddCombatAction(VAEL_FIRE_NOVA, 5000u);
        AddCombatAction(VAEL_CLEAVE, 8000u);
        AddCombatAction(VAEL_TAIL_SWEEP, 20000u);
        AddCustomAction(VAEL_INTRO, true, [&]() { HandleIntro(); });
        AddCustomAction(VAEL_SPEECH, true, [&]() { HandleSpeech(); });
        Reset();

        // Set stand state to dead before the intro event
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    ScriptedInstance* m_instance;

    ObjectGuid m_nefariusGuid;
    uint8 m_uiIntroPhase;

    uint8 m_uiSpeechNum;

    void Reset() override
    {
        CombatAI::Reset();

        m_uiIntroPhase                   = 0;
        m_uiSpeechNum                    = 0;

        // Creature should have only 30% of hp
        m_creature->SetHealth(uint32(m_creature->GetMaxHealth()*.3));
    }

    void BeginIntro()
    {
        // Start Intro delayed
        ResetTimer(VAEL_INTRO, 1000);

        if (m_instance)
            m_instance->SetData(TYPE_VAELASTRASZ, SPECIAL);
    }

    void BeginSpeech(Player* target)
    {
        // 10 seconds
        DoScriptText(SAY_LINE_1, m_creature);

        // Make boss stand
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        ResetTimer(VAEL_SPEECH, 10000);
        m_uiSpeechNum = 0;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_KILLTARGET, m_creature, pVictim);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VAELASTRASZ, IN_PROGRESS);

        // Buff players on aggro
        DoCastSpellIfCan(nullptr, SPELL_ESSENCE_OF_THE_RED);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VAELASTRASZ, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VAELASTRASZ, FAIL);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_LORD_VICTOR_NEFARIUS)
        {
            // Set not selectable, so players won't interact with it
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_nefariusGuid = summoned->GetObjectGuid();
        }
    }

    void HandleIntro()
    {
        uint32 timer = 0;
        switch (m_uiIntroPhase)
        {
            case 0:
            {
                // Summon Lord Victor Nefarius in front of the Throne
                m_creature->SummonCreature(NPC_LORD_VICTOR_NEFARIUS, aNefariusSpawnLoc[0], aNefariusSpawnLoc[1], aNefariusSpawnLoc[2], aNefariusSpawnLoc[3], TEMPSPAWN_TIMED_DESPAWN, 25000);

                bool bHasYelled = false;
                CreatureList lTechniciansList;
                // Search for the Blackwing Technicians tormeting Vaelastrasz to make them flee to the next room above the stairs
                GetCreatureListWithEntryInGrid(lTechniciansList, m_creature, NPC_BLACKWING_TECHNICIAN, 40.0f);
                for (CreatureList::const_iterator itr = lTechniciansList.begin(); itr != lTechniciansList.end(); ++itr)
                {
                    // Ignore Blackwing Technicians on upper floors and dead ones
                    if (!((*itr)->IsAlive()) || (*itr)->GetPositionZ() > m_creature->GetPositionZ() + 1)
                        continue;

                    // Each fleeing part and despawn is handled in DB, we only need to make them run
                    (*itr)->SetWalk(false);

                    // The technicians will behave differently depending on they are on the right or left side of
                    // Vaelastrasz. We compare their X position to Vaelastrasz X position to sort them out
                    if ((*itr)->GetPositionX() > m_creature->GetPositionX())
                    {
                        // Left side
                        if (!bHasYelled)
                        {
                            DoScriptText(SAY_TECHNICIAN_RUN, (*itr));
                            bHasYelled = true;
                        }
                        (*itr)->GetMotionMaster()->MoveWaypoint(0);
                    }
                    else
                        // Right side
                        (*itr)->GetMotionMaster()->MoveWaypoint(1);
                }
                timer = 1000;
                break;
            }
            case 1:
                if (Creature* nefarius = m_creature->GetMap()->GetCreature(m_nefariusGuid))
                {
                    nefarius->CastSpell(m_creature, SPELL_NEFARIUS_CORRUPTION, TRIGGERED_OLD_TRIGGERED);
                    DoScriptText(SAY_NEFARIUS_CORRUPT_1, nefarius);
                }
                timer = 14000;
                break;
            case 2:
                if (Creature* nefarius = m_creature->GetMap()->GetCreature(m_nefariusGuid))
                    DoScriptText(SAY_NEFARIUS_CORRUPT_2, nefarius);

                timer = 2000;
                break;
            case 3:
                if (Creature* nefarius = m_creature->GetMap()->GetCreature(m_nefariusGuid))
                    nefarius->CastSpell(m_creature, SPELL_RED_LIGHTNING, TRIGGERED_NONE);

                // Set npc flags now
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                m_creature->RemoveAurasDueToSpell(SPELL_NEFARIUS_CORRUPTION);
                break;
        }
        ++m_uiIntroPhase;
        if (timer)
            ResetTimer(VAEL_INTRO, timer);
    }

    void HandleSpeech()
    {
        uint32 timer = 0;
        switch (m_uiSpeechNum)
        {
            case 0:
                // 16 seconds till next line
                DoScriptText(SAY_LINE_2, m_creature);
                timer = 16000;
                ++m_uiSpeechNum;
                break;
            case 1:
                // This one is actually 16 seconds but we only go to 10 seconds because he starts attacking after he says "I must fight this!"
                DoScriptText(SAY_LINE_3, m_creature);
                timer = 10000;
                ++m_uiSpeechNum;
                break;
            case 2:
                m_creature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN);
                m_creature->SetInCombatWithZone();
                AttackClosestEnemy();
                break;
        }
        if (timer)
            ResetTimer(VAEL_SPEECH, timer);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case VAEL_LOW_HP_YELL:
            {
                if (m_creature->GetHealthPercent() < 15.0f)
                {
                    DoScriptText(SAY_HALFLIFE, m_creature);
                    SetActionReadyStatus(action, false);
                }
                break;
            }
            case VAEL_BURNING_ADRENALINE_TANK:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_BURNING_ADRENALINE_TANK) == CAST_OK)
                    ResetCombatAction(action, 45000);
                break;
            }
            case VAEL_BURNING_ADRENALINE_TARGET:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_BURNING_ADRENALINE, SELECT_FLAG_PLAYER | SELECT_FLAG_POWER_MANA))
                    if (DoCastSpellIfCan(target, SPELL_BURNING_ADRENALINE) == CAST_OK)
                        ResetCombatAction(action, 15000);
                break;
            }
            case VAEL_FLAME_BREATH:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FLAME_BREATH) == CAST_OK)
                    ResetCombatAction(action, urand(4000, 8000));
                break;
            }
            case VAEL_FIRE_NOVA:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FIRE_NOVA) == CAST_OK)
                    ResetCombatAction(action, 5000);
                break;
            }
            case VAEL_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, 15000);
                break;
            }
            case VAEL_TAIL_SWEEP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_TAIL_SWEEP) == CAST_OK)
                    ResetCombatAction(action, 20000);
                break;
            }
        }
    }
};

bool GossipSelect_boss_vaelastrasz(Player* pPlayer, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VAEL_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_VAEL_2, creature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (boss_vaelastraszAI* pVaelAI = dynamic_cast<boss_vaelastraszAI*>(creature->AI()))
                pVaelAI->BeginSpeech(pPlayer);
            break;
    }

    return true;
}

bool GossipHello_boss_vaelastrasz(Player* pPlayer, Creature* creature)
{
    if (creature->isQuestGiver())
        pPlayer->PrepareQuestMenu(creature->GetObjectGuid());

    pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VAEL_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_VAEL_1, creature->GetObjectGuid());

    return true;
}

bool QuestAccept_boss_vaelastrasz(Player* pPlayer, Creature* /*creature*/, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_NEFARIUS_CORRUPTION)
    {
        if (instance_blackwing_lair* pInstance = (instance_blackwing_lair*)pPlayer->GetInstanceData())
            pInstance->SetData(TYPE_QUEST_SCEPTER, IN_PROGRESS);
    }

    return true;
}

bool AreaTrigger_at_vaelastrasz(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (pAt->id == AREATRIGGER_VAEL_INTRO)
    {
        if (pPlayer->IsGameMaster() || pPlayer->IsDead())
            return false;

        if (instance_blackwing_lair* pInstance = (instance_blackwing_lair*)pPlayer->GetInstanceData())
        {
            // Handle intro event
            if (pInstance->GetData(TYPE_VAELASTRASZ) == NOT_STARTED)
            {
                if (Creature* pVaelastrasz = pInstance->GetSingleCreatureFromStorage(NPC_VAELASTRASZ))
                    if (boss_vaelastraszAI* pVaelAI = dynamic_cast<boss_vaelastraszAI*>(pVaelastrasz->AI()))
                        pVaelAI->BeginIntro();
            }
        }
    }

    return false;
}

void AddSC_boss_vaelastrasz()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_vaelastrasz";
    pNewScript->GetAI = &GetNewAIInstance<boss_vaelastraszAI>;
    pNewScript->pGossipHello = &GossipHello_boss_vaelastrasz;
    pNewScript->pGossipSelect = &GossipSelect_boss_vaelastrasz;
    pNewScript->pQuestAcceptNPC = &QuestAccept_boss_vaelastrasz;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_vaelastrasz";
    pNewScript->pAreaTrigger = &AreaTrigger_at_vaelastrasz;
    pNewScript->RegisterSelf();
}
