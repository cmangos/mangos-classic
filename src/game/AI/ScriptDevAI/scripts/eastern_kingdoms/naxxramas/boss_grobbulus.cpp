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
SDName: Boss_Grobbulus
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

/*Poison Cloud 26590
Slime Spray 28157
Fallout slime 28218
Mutating Injection 28169
Enrages 26527*/

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    SPELL_SLIME_STREAM              = 28137,
    SPELL_MUTATING_INJECTION        = 28169,
    SPELL_EMBALMING_CLOUD           = 28322,
    SPELL_MUTAGEN_EXPLOSION         = 28206,
    SPELL_POISON_CLOUD              = 28240,
    SPELL_SLIME_SPRAY               = 28157,
    SPELL_SUMMON_FALLOUT_SLIME      = 28218,
    SPELL_BERSERK                   = 26662,

    SPELL_DESPAWN_SUMMONS           = 30134,            // Spell ID unsure: there are several spells doing the same thing in DBCs within Naxxramas spell IDs range
                                                        // but it is not always clear which one is for each boss so some are assigned randomly like this one
};

enum GrobbulusActions
{
    GROBBULUS_SLIME_STREAM,
    GROBBULUS_SLIME_SPRAY,
    GROBBULUS_INJECTION,
    GROBBULUS_POISON_CLOUD,
    GROBBULUS_BERSERK,
    GROBBULUS_ACTION_MAX,
};

struct boss_grobbulusAI : public CombatAI
{
    boss_grobbulusAI(Creature* creature) : CombatAI(creature, GROBBULUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(GROBBULUS_SLIME_STREAM, 5u * IN_MILLISECONDS);
        AddCombatAction(GROBBULUS_SLIME_SPRAY, 20u * IN_MILLISECONDS, 30u * IN_MILLISECONDS);
        AddCombatAction(GROBBULUS_INJECTION, 13u * IN_MILLISECONDS);
        AddCombatAction(GROBBULUS_POISON_CLOUD, 20u * IN_MILLISECONDS, 25u * IN_MILLISECONDS);
        AddCombatAction(GROBBULUS_BERSERK,  (uint32)(12 * MINUTE * IN_MILLISECONDS));
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, IN_PROGRESS);

        ResetIfNotStarted(GROBBULUS_SLIME_STREAM, 5u * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, DONE);
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, FAIL);

        // Clean-up stage
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_SUMMONS, CAST_TRIGGERED);

        CombatAI::EnterEvadeMode();
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell) override
    {
        // Summon a Fallout Slime for every player hit by Slime Spray
        if ((spell->Id == SPELL_SLIME_SPRAY) && target->GetTypeId() == TYPEID_PLAYER)
            DoCastSpellIfCan(target, SPELL_SUMMON_FALLOUT_SLIME, CAST_TRIGGERED);
    }

    uint32 GetSubsequentActionTimer(uint32 id)
    {
        switch (id)
        {
            // Mutagen Injection is used more often when below 30% HP
            case GROBBULUS_INJECTION: return m_creature->GetHealthPercent() > 30.0f ? urand(7, 13) * IN_MILLISECONDS : urand(3, 7) * IN_MILLISECONDS;
            case GROBBULUS_POISON_CLOUD: return 15u * IN_MILLISECONDS;
            case GROBBULUS_SLIME_SPRAY: return urand(20, 30) * IN_MILLISECONDS;
            default: return 0;
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case GROBBULUS_SLIME_STREAM:
            {
                if (!m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
                    DoCastSpellIfCan(m_creature, SPELL_SLIME_STREAM, CAST_TRIGGERED);
                ResetCombatAction(action, 3u * IN_MILLISECONDS);
                break;
            }
            case GROBBULUS_INJECTION:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MUTATING_INJECTION, SELECT_FLAG_PLAYER | SELECT_FLAG_NOT_AURA))
                {
                    if (DoCastSpellIfCan(target, SPELL_MUTATING_INJECTION, CAST_TRIGGERED) == CAST_OK)
                        ResetCombatAction(action, GetSubsequentActionTimer(action));
                }
                break;
            }
            case GROBBULUS_POISON_CLOUD:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POISON_CLOUD) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                break;
            }
            case GROBBULUS_SLIME_SPRAY:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SLIME_SPRAY) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                break;
            }
            case GROBBULUS_BERSERK:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                    DisableCombatAction(action);
                break;
            }
        }
    }
};

struct MutatingInjection : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply)
        {
            Unit* target = aura->GetTarget();
            if (!target)
                return;

            if (aura->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                // Embalming Cloud
                target->CastSpell(target, SPELL_EMBALMING_CLOUD, TRIGGERED_OLD_TRIGGERED, nullptr, aura);
            else // Removed by dispel
                // Mutagen Explosion
                target->CastSpell(target, SPELL_MUTAGEN_EXPLOSION, TRIGGERED_OLD_TRIGGERED, nullptr, aura);
            // Poison Cloud
            target->CastSpell(target, SPELL_POISON_CLOUD, TRIGGERED_OLD_TRIGGERED, nullptr, aura);
        }
    }
};

void AddSC_boss_grobbulus()
{
    Script* newScript = new Script;
    newScript->Name = "boss_grobbulus";
    newScript->GetAI = &GetNewAIInstance<boss_grobbulusAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<MutatingInjection>("spell_grobbulus_mutating_injection");
}
