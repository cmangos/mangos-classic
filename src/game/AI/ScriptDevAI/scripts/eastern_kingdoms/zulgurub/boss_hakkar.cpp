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
SDName: Boss_Hakkar
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulgurub.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_AGGRO                   = -1309020,
    SAY_FLEEING                 = -1309021,

    SPELL_BLOOD_SIPHON          = 24324,                    // triggers 24322 or 24323 on caster
    SPELL_CORRUPTED_BLOOD       = 24328,
    SPELL_CAUSE_INSANITY        = 24327,
    SPELL_WILL_OF_HAKKAR        = 24178,
    SPELL_ENRAGE                = 24318,
    SPELL_DOUBLE_ATTACK         = 19818,

    // The Aspects of all High Priests
    SPELL_ASPECT_OF_JEKLIK      = 24687, // assumed spell list index 5
    SPELL_ASPECT_OF_VENOXIS     = 24688, // assumed spell list index 6
    SPELL_ASPECT_OF_MARLI       = 24686, // assumed spell list index 7
    SPELL_ASPECT_OF_THEKAL      = 24689, // assumed spell list index 8
    SPELL_ASPECT_OF_ARLOKK      = 24690  // assumed spell list index 9
};

enum HakkarActions
{
    HAKKAR_ENRAGE_LOW,
    HAKKAR_ACTION_MAX,
};

struct boss_hakkarAI : public CombatAI
{
    boss_hakkarAI(Creature* creature) : CombatAI(creature, HAKKAR_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(HAKKAR_ENRAGE_LOW, true);
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        CombatAI::Reset();

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        InitiateHakkarPowerStacks();
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        // check if the priest encounters are done
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_JEKLIK) == DONE)
                m_creature->UpdateSpell(5, 0);
            if (m_instance->GetData(TYPE_VENOXIS) == DONE)
                m_creature->UpdateSpell(6, 0);
            if (m_instance->GetData(TYPE_MARLI) == DONE)
                m_creature->UpdateSpell(7, 0);
            if (m_instance->GetData(TYPE_THEKAL) == DONE)
                m_creature->UpdateSpell(8, 0);
            if (m_instance->GetData(TYPE_ARLOKK) == DONE)
                m_creature->UpdateSpell(9, 0);
        }
    }

    // For each of the High Priests that is alive, update Hakkar's Power Stacks (updating Hakkar's HP)
    void InitiateHakkarPowerStacks()
    {
        m_creature->RemoveAurasDueToSpell(SPELL_HAKKAR_POWER);
        for (uint8 i = 0; i < MAX_PRIESTS; i++)
        {
            if (m_instance->GetData(i) != DONE)
                m_creature->CastSpell(m_creature, SPELL_HAKKAR_POWER, TRIGGERED_NONE);
        }
    }

    void ExecuteAction(uint32 action) override
    {
        if (action == HAKKAR_ENRAGE_LOW)
            if (m_creature->GetHealthPercent() < 5.f)
                if (DoCastSpellIfCan(nullptr, SPELL_ENRAGE) == CAST_OK)
                    DisableCombatAction(action);
    }
};

struct HakkarPowerDown : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* target = spell->GetUnitTarget())
            {
                if (target->HasAura(SPELL_HAKKAR_POWER))
                    target->RemoveAuraStack(SPELL_HAKKAR_POWER);
            }
        }
    }
};

struct BloodSiphon : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0)
            return;

        Unit* target = spell->GetUnitTarget();
        target->CastSpell(spell->GetCaster(), target->HasAura(24321) ? 24323 : 24322, TRIGGERED_OLD_TRIGGERED);
    }
};

void AddSC_boss_hakkar()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_hakkar";
    pNewScript->GetAI = &GetNewAIInstance<boss_hakkarAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<HakkarPowerDown>("spell_hakkar_power_down");
    RegisterSpellScript<BloodSiphon>("spell_hakkar_blood_siphon");
}
