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
SDName: Boss_Ragnaros
SD%Complete: 99
SDComment: Submerge emote is not played
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_ARRIVAL5_RAG            = -1409012,
    SAY_REINFORCEMENTS_1        = -1409013,
    SAY_REINFORCEMENTS_2        = -1409014,
    SAY_HAMMER                  = -1409015,
    SAY_WRATH                   = -1409016,
    SAY_KILL                    = 7626,
    SAY_MAGMABURST              = -1409018,

    SPELL_WRATH_OF_RAGNAROS     = 20566,
    SPELL_ELEMENTAL_FIRE        = 20563,                    // Aura, proc spell 20564 when doing melee damage
    SPELL_MAGMA_BLAST           = 20565,                    // Ranged attack if nobody is in melee range
    SPELL_MELT_WEAPON           = 21387,
    SPELL_RAGNA_SUBMERGE        = 21107,                    // Stealth aura
    SPELL_RAGNA_SUBMERGE_VISUAL = 20567,                    // Visual for submerging into lava
    SPELL_RAGNA_EMERGE          = 20568,                    // Emerge from lava
    SPELL_SUBMERGE_EFFECT       = 21859,                    // Make Ragnaros immune and passive while submerged
    SPELL_ELEMENTAL_FIRE_KILL   = 19773,
    SPELL_MIGHT_OF_RAGNAROS     = 21154,
    SPELL_INTENSE_HEAT          = 21155,
    SPELL_LAVA_BURST            = 21908,                    // Randomly trigger one of spells 21886, 21900 - 21907 which summons Go 178088
    SPELL_SUMMON_SONS_FLAME     = 21108,                    // Trigger the eight spells summoning the Son of Flame adds

    // add spells
    SPELL_DOUBLE_ATTACK         = 19818,
    SPELL_LAVA_SHIELD           = 21857,

    NB_ADDS_IN_SUBMERGE         = 8,
    NPC_SON_OF_FLAME            = 12143,
    NPC_FLAME_OF_RAGNAROS       = 13148,

    PHASE_EMERGING              = 0,
    PHASE_EMERGED               = 1,
    PHASE_SUBMERGING            = 2,
    PHASE_SUBMERGED             = 3
};

enum RagnarosActions
{
    RAGNAROS_PHASE_TRANSITION,
    RAGNAROS_WRATH_OF_RAGNAROS,
    RAGNAROS_MIGHT_OF_RAGNAROS,
    RAGNAROS_MAGMA_BLAST,
    RAGNAROS_LAVA_BURST,
    RAGNAROS_ACTION_MAX,
    RAGNAROS_ENTER_COMBAT,
};

struct boss_ragnarosAI : public CombatAI
{
    boss_ragnarosAI(Creature* creature) : CombatAI(creature, RAGNAROS_ACTION_MAX), m_instance(static_cast<instance_molten_core*>(creature->GetInstanceData()))
    {
        AddCustomAction(RAGNAROS_ENTER_COMBAT, true, [&]() { HandleEnterCombat(); });
        AddCombatAction(RAGNAROS_PHASE_TRANSITION, uint32(3 * MINUTE * IN_MILLISECONDS));
        AddCombatAction(RAGNAROS_WRATH_OF_RAGNAROS, 30000u);
        AddCombatAction(RAGNAROS_MIGHT_OF_RAGNAROS, 11000u);
        AddCombatAction(RAGNAROS_MAGMA_BLAST, 2000u);
        AddCombatAction(RAGNAROS_LAVA_BURST, uint32(20 * IN_MILLISECONDS));
        AddOnKillText(SAY_KILL);
        m_bHasAggroYelled = false;
    }

    instance_molten_core* m_instance;

    uint32 m_uiAddCount;
    uint32 m_phase;
    int32 m_rangeCheckState;

    bool m_bHasAggroYelled;
    bool m_bHasYelledMagmaBurst;
    bool m_bHasSubmergedOnce;

    GuidVector m_spawns;

    void Reset() override
    {
        CombatAI::Reset();
        m_uiAddCount = 0;
        m_phase = PHASE_EMERGED;
        m_rangeCheckState       = -1;

        m_bHasYelledMagmaBurst = false;
        m_bHasSubmergedOnce = false;

        m_creature->SetImmobilizedState(true);

        m_creature->HandleEmote(EMOTE_STATE_STAND);
        m_creature->RemoveAurasDueToSpell(SPELL_RAGNA_SUBMERGE);
        m_creature->RemoveAurasDueToSpell(SPELL_SUBMERGE_EFFECT);

        DoCastSpellIfCan(nullptr, SPELL_MELT_WEAPON, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(nullptr, SPELL_ELEMENTAL_FIRE, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        DespawnGuids(m_spawns);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_RAGNAROS, DONE);
    }

    void Aggro(Unit* who) override
    {
        if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == NPC_MAJORDOMO)
            return;

        if (m_instance)
            m_instance->SetData(TYPE_RAGNAROS, IN_PROGRESS);
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_RAGNAROS, FAIL);

        // Reset flag if Ragnaros had been submerged
        if (m_phase != PHASE_EMERGED)
            DoCastSpellIfCan(m_creature, SPELL_RAGNA_EMERGE);
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);

        ScriptedAI::EnterEvadeMode();
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        // If all Sons of Flame are dead, trigger emerge
        if (summoned->GetEntry() == NPC_SON_OF_FLAME)
        {
            m_uiAddCount--;

            // If last add killed then emerge soonish
            if (m_uiAddCount == 0)
                ReduceTimer(RAGNAROS_PHASE_TRANSITION, 1000);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SON_OF_FLAME)
        {
            summoned->AI()->DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
            summoned->AI()->DoCastSpellIfCan(nullptr, SPELL_LAVA_SHIELD, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
            summoned->SetInCombatWithZone();
            summoned->AI()->AttackClosestEnemy();
        }
        else if (summoned->GetEntry() == NPC_FLAME_OF_RAGNAROS)
            summoned->CastSpell(nullptr, SPELL_INTENSE_HEAT, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());

        m_spawns.push_back(summoned->GetObjectGuid());
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo, SpellMissInfo /*missInfo*/) override
    {
        if (spellInfo->Id == SPELL_WRATH_OF_RAGNAROS)
            m_creature->getThreatManager().modifyThreatPercent(target, -100);
        // As Majordomo is now killed, the last timer (until attacking) must be handled with ragnaros script
        else if (spellInfo->Id == SPELL_ELEMENTAL_FIRE_KILL && target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == NPC_MAJORDOMO)
            ResetTimer(RAGNAROS_ENTER_COMBAT, 10000);
    }

    void HandleEnterCombat()
    {
        // Introduction
        if (!m_bHasAggroYelled)
        {
            ResetTimer(RAGNAROS_ENTER_COMBAT, 3000);
            m_bHasAggroYelled = true;
            DoScriptText(SAY_ARRIVAL5_RAG, m_creature);
        }
        else
        {
            // If we don't remove this passive flag, he will be unattackable after evading, this way he will enter combat
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            if (m_instance)
            {
                m_creature->SetInCombatWithZone();
                AttackClosestEnemy();
            }
        }

    }

    void HandlePhaseTransition()
    {
        switch (m_phase)
        {
            case PHASE_EMERGED:
            {
                // Submerge and attack again after 90 secs
                DoCastSpellIfCan(m_creature, SPELL_RAGNA_SUBMERGE_VISUAL, CAST_TRIGGERED);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);

                // Say dependend if first time or not
                DoScriptText(!m_bHasSubmergedOnce ? SAY_REINFORCEMENTS_1 : SAY_REINFORCEMENTS_2, m_creature);
                m_bHasSubmergedOnce = true;

                // Summon 8 elementals around the boss
                if (DoCastSpellIfCan(nullptr, SPELL_SUMMON_SONS_FLAME) == CAST_OK)
                    m_uiAddCount = NB_ADDS_IN_SUBMERGE;

                ResetCombatAction(RAGNAROS_PHASE_TRANSITION, 1000);
                m_phase = PHASE_SUBMERGING;
                DisableCombatAction(RAGNAROS_WRATH_OF_RAGNAROS);
                DisableCombatAction(RAGNAROS_MIGHT_OF_RAGNAROS);
                DisableCombatAction(RAGNAROS_MAGMA_BLAST);
                DisableCombatAction(RAGNAROS_LAVA_BURST);
                SetMeleeEnabled(false);
                break;
            }
            case PHASE_SUBMERGING:
            {
                m_creature->HandleEmote(EMOTE_STATE_SUBMERGED);
                DoCastSpellIfCan(nullptr, SPELL_RAGNA_SUBMERGE, CAST_TRIGGERED);
                DoCastSpellIfCan(nullptr, SPELL_SUBMERGE_EFFECT, CAST_TRIGGERED);
                ResetCombatAction(RAGNAROS_PHASE_TRANSITION, 90 * IN_MILLISECONDS);
                m_phase = PHASE_SUBMERGED;
                break;
            }
            case PHASE_SUBMERGED:
            {
                // Become emerged again
                m_creature->HandleEmote(EMOTE_STATE_STAND);
                m_creature->RemoveAurasDueToSpell(SPELL_RAGNA_SUBMERGE);
                m_creature->RemoveAurasDueToSpell(SPELL_SUBMERGE_EFFECT);
                m_creature->RemoveAurasDueToSpell(SPELL_RAGNA_SUBMERGE_VISUAL);
                ResetCombatAction(RAGNAROS_PHASE_TRANSITION, 500);
                m_phase = PHASE_EMERGING;
                break;
            }
            case PHASE_EMERGING:
            {
                DoCastSpellIfCan(m_creature, SPELL_RAGNA_EMERGE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
                m_phase = PHASE_EMERGED;
                ResetCombatAction(RAGNAROS_PHASE_TRANSITION, 3 * MINUTE * IN_MILLISECONDS);
                ResetCombatAction(RAGNAROS_WRATH_OF_RAGNAROS, 20000);
                ResetCombatAction(RAGNAROS_MIGHT_OF_RAGNAROS, urand(11000, 30000));
                ResetCombatAction(RAGNAROS_MAGMA_BLAST, 3000);
                ResetCombatAction(RAGNAROS_LAVA_BURST, urand(5000, 25000));
                SetMeleeEnabled(true);
                break;
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case RAGNAROS_PHASE_TRANSITION:
            {
                HandlePhaseTransition();
                break;
            }
            case RAGNAROS_WRATH_OF_RAGNAROS:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_WRATH_OF_RAGNAROS) == CAST_OK)
                {
                    DoScriptText(SAY_WRATH, m_creature);
                    ResetCombatAction(action, 25000);
                }
                break;
            }
            case RAGNAROS_MIGHT_OF_RAGNAROS:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MIGHT_OF_RAGNAROS, SELECT_FLAG_POWER_MANA))
                {
                    if (DoCastSpellIfCan(target, SPELL_MIGHT_OF_RAGNAROS) == CAST_OK)
                    {
                        DoScriptText(SAY_HAMMER, m_creature);
                        ResetCombatAction(action, urand(11000, 30000));
                    }
                }
                break;
            }
            case RAGNAROS_MAGMA_BLAST:
            {
                uint32 timer = 500;
                // If victim exists we have a target in melee range
                if (m_creature->GetVictim() && m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
                    m_rangeCheckState = -1;
                // Spam Waterbolt spell when not tanked
                else
                {
                    ++m_rangeCheckState;
                    if (m_rangeCheckState > 1)
                    {
                        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MAGMA_BLAST, SELECT_FLAG_PLAYER))
                        {
                            if (DoCastSpellIfCan(target, SPELL_MAGMA_BLAST) == CAST_OK)
                            {
                                //if (!m_bHasYelledMagmaBurst)
                                //{
                                //    DoScriptText(SAY_MAGMABURST, m_creature);
                                //    m_bHasYelledMagmaBurst = true;
                                //}
                                timer = 2500;
                            }
                        }
                    }
                }
                ResetCombatAction(action, timer);
                break;
            }
            case RAGNAROS_LAVA_BURST:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_LAVA_BURST) == CAST_OK)
                    ResetCombatAction(action, urand(5000, 25000)); // Upper value is guess work based on videos. It could be up to 45 secs
                break;
            }
        }
    }
};

UnitAI* GetAI_boss_ragnaros(Creature* creature)
{
    return new boss_ragnarosAI(creature);
}

void AddSC_boss_ragnaros()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ragnaros";
    pNewScript->GetAI = &GetAI_boss_ragnaros;
    pNewScript->RegisterSelf();
}
