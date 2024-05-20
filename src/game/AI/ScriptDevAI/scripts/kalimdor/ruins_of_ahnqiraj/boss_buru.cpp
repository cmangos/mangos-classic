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
SDName: Boss_Buru
SD%Complete: 70
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "ruins_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    EMOTE_TARGET            = -1509002,

    // boss spells
    SPELL_CREEPING_PLAGUE   = 20512,
    SPELL_DISMEMBER         = 96,
    SPELL_GATHERING_SPEED   = 1834,
    SPELL_FULL_SPEED        = 1557,
    SPELL_THORNS            = 25640,
    SPELL_BURU_TRANSFORM    = 24721,
    SPELL_CREATURE_SPECIAL  = 7155, // unk purpose

    // egg spells
    SPELL_SUMMON_HATCHLING  = 1881,
    SPELL_EXPLODE           = 19593,
    SPELL_BURU_EGG_TRIGGER  = 26646,
    SPELL_EXPLOSION         = 5255,

    // npcs
    NPC_BURU_EGG_TRIGGER    = 15964,
    NPC_BURU_EGG            = 15514,
    NPC_HATCHLING           = 15521,

    PHASE_EGG               = 1,
    PHASE_TRANSFORM         = 2,
};

enum BuruActions
{
    BURU_PHASE_2_TRANSITION,
    BURU_NEW_TARGET,
    BURU_DISMEMBER,
    BURU_CREEPING_PLAGUE,
    BURU_GATHERING_SPEED,
    BURU_FULL_SPEED,
    BURU_ACTION_MAX,
    BURU_PHASE_2_TRANSITION_STEP,
};

struct boss_buruAI : public CombatAI
{
    boss_buruAI(Creature* creature) :
        CombatAI(creature, BURU_ACTION_MAX),
        m_uiPhase(0),
        m_transitionStep(0)
    {
        AddTimerlessCombatAction(BURU_PHASE_2_TRANSITION, true);
        AddTimerlessCombatAction(BURU_NEW_TARGET, false);
        AddCombatAction(BURU_DISMEMBER, 5000u);
        AddCombatAction(BURU_CREEPING_PLAGUE, true);
        AddCombatAction(BURU_GATHERING_SPEED, 9000u);
        AddCombatAction(BURU_FULL_SPEED, 60000u);
        AddCustomAction(BURU_PHASE_2_TRANSITION_STEP, true, [&]() { HandlePhaseTwo(); }, TIMER_COMBAT_COMBAT);
    }

    uint8 m_uiPhase;
    uint32 m_transitionStep;

    void Reset() override
    {
        CombatAI::Reset();
        m_uiPhase = PHASE_EGG;
        m_transitionStep = 0;
        SetCombatMovement(true);
        SetCombatScriptStatus(false);
        SetMeleeEnabled(true);
    }

    void Aggro(Unit* who) override
    {
        DoScriptText(EMOTE_TARGET, m_creature, who);
        DoCastSpellIfCan(nullptr, SPELL_THORNS);
        DoAttackNewTarget();
    }

    void KilledUnit(Unit* victim) override
    {
        // Attack a new random target when a player is killed
        if (victim == m_creature->GetVictim())
            ScheduleNewTarget();
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo, SpellMissInfo /*missInfo*/) override
    {
        if (spellInfo->Id == SPELL_CREATURE_SPECIAL)
            DoAttackNewTarget();
    }

    void ScheduleNewTarget()
    {
        SetActionReadyStatus(BURU_NEW_TARGET, true);
    }

    // Wrapper to attack a new target and remove the speed gathering buff
    void DoAttackNewTarget()
    {
        if (m_uiPhase == PHASE_TRANSFORM)
            return;

        SetCombatMovement(true);
        DoResetThreat();

        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
        {
            m_creature->AddThreat(target, 1000000.f);
            AttackStart(target);
            DoScriptText(EMOTE_TARGET, m_creature, target);
        }

        ResetCombatAction(BURU_FULL_SPEED, 60000);
        ResetCombatAction(BURU_GATHERING_SPEED, 9000);
    }

    void HandlePhaseTwo()
    {
        switch (m_transitionStep)
        {
            case 0:
            {
                DoCastSpellIfCan(nullptr, SPELL_BURU_TRANSFORM);
                m_creature->RemoveAurasDueToSpell(SPELL_THORNS);
                ResetTimer(BURU_PHASE_2_TRANSITION_STEP, 5000);
                break;
            }
            case 1:
            {
                DoCastSpellIfCan(nullptr, SPELL_FULL_SPEED, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                SetCombatScriptStatus(false);
                SetMeleeEnabled(true);
                SetCombatMovement(true, true);
                break;
            }
        }
        ++m_transitionStep;
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case BURU_PHASE_2_TRANSITION:
            {
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    SetActionReadyStatus(action, false);
                    m_uiPhase = PHASE_TRANSFORM;
                    DisableCombatAction(BURU_DISMEMBER);
                    SetActionReadyStatus(BURU_NEW_TARGET, false);
                    DisableCombatAction(BURU_GATHERING_SPEED);
                    DisableCombatAction(BURU_FULL_SPEED);
                    ResetCombatAction(BURU_CREEPING_PLAGUE, 0);
                    DoResetThreat();
                    ResetTimer(BURU_PHASE_2_TRANSITION_STEP, 2000);
                    SetCombatScriptStatus(true);
                    SetMeleeEnabled(false);
                    SetCombatMovement(false, true);
                    m_creature->SetTarget(nullptr);
                }
                break;
            }
            case BURU_NEW_TARGET:
            {
                m_creature->RemoveAurasDueToSpell(SPELL_FULL_SPEED);
                m_creature->RemoveAurasDueToSpell(SPELL_GATHERING_SPEED);
                if (DoCastSpellIfCan(nullptr, SPELL_CREATURE_SPECIAL) == CAST_OK)
                {
                    SetCombatMovement(false, true);
                    SetActionReadyStatus(action, false);
                }
                break;
            }
            case BURU_DISMEMBER:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_DISMEMBER) == CAST_OK)
                    ResetCombatAction(action, 5000);
                break;
            }
            case BURU_CREEPING_PLAGUE:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_CREEPING_PLAGUE) == CAST_OK)
                    ResetCombatAction(action, 6000);
                break;
            }
            case BURU_GATHERING_SPEED:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_GATHERING_SPEED) == CAST_OK)
                    ResetCombatAction(action, 9000);
                break;
            }
            case BURU_FULL_SPEED:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_FULL_SPEED) == CAST_OK)
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_GATHERING_SPEED);
                    DisableCombatAction(action);
                    DisableCombatAction(BURU_GATHERING_SPEED);
                }
                break;
            }
        }
    }
};

struct npc_buru_eggAI : public Scripted_NoMovementAI
{
    npc_buru_eggAI(Creature* creature) : Scripted_NoMovementAI(creature), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        m_creature->SetCorpseDelay(5);
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        SetCombatMovement(false);
        SetReactState(REACT_PASSIVE);
    }

    void JustSummoned(Creature* summoned) override
    {
        // The purpose of this is unk for the moment
        if (summoned->GetEntry() == NPC_BURU_EGG_TRIGGER)
            summoned->CastSpell(summoned, SPELL_BURU_EGG_TRIGGER, TRIGGERED_OLD_TRIGGERED);
        // The Hatchling should attack a random target
        else if (summoned->GetEntry() == NPC_HATCHLING)
        {
            if (m_instance)
            {
                if (Creature* buru = m_instance->GetSingleCreatureFromStorage(NPC_BURU))
                {
                    if (Unit* pTarget = buru->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                        summoned->AI()->AttackStart(pTarget);
                }
            }
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo, SpellMissInfo /*missInfo*/) override
    {
        if (spellInfo->Id == SPELL_EXPLODE)
        {
            int32 damage;
            if (target->IsPlayer()) // damage from 100 - 500 based on proximity - max range 25
                damage = 100 + ((25 - std::min(m_creature->GetDistance(target, true, DIST_CALC_COMBAT_REACH), 25.f)) / 25.f) * 400;
            else if (target->GetEntry() == NPC_BURU)
            {
                damage = target->GetMaxHealth() * 15 / 100; // 15% hp for buru
                static_cast<boss_buruAI*>(target->AI())->ScheduleNewTarget();
            }
            m_creature->CastCustomSpell(target, SPELL_EXPLOSION, &damage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_BURU_TRANSFORM)
        {
            DoCastSpellIfCan(nullptr, SPELL_SUMMON_HATCHLING, CAST_TRIGGERED);
            m_creature->ForcedDespawn();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Explode and Summon hatchling
        DoCastSpellIfCan(nullptr, SPELL_EXPLODE);
        DoCastSpellIfCan(nullptr, SPELL_SUMMON_HATCHLING, CAST_TRIGGERED);
    }

    void UpdateAI(const uint32 /*uiDiff*/) override { }
};

void AddSC_boss_buru()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_buru";
    pNewScript->GetAI = &GetNewAIInstance<boss_buruAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_buru_egg";
    pNewScript->GetAI = &GetNewAIInstance<npc_buru_eggAI>;
    pNewScript->RegisterSelf();
}
