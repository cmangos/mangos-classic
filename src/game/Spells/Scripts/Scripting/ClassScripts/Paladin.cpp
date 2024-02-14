/*
* This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
*
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

#include "Spells/Scripts/SpellScript.h"
#include "Spells/SpellAuras.h"
#include "Spells/SpellMgr.h"

struct SealOfTheCrusader : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (aura->GetEffIndex() != EFFECT_INDEX_1)
            return;

        // Seal of the Crusader damage reduction
        // SotC increases attack speed but reduces damage to maintain the same DPS
        float reduction = (-100.0f * aura->GetModifier()->m_amount) / (aura->GetModifier()->m_amount + 100.0f);
        aura->GetTarget()->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, reduction, apply);
    }
};

// 5373 - Judgement of Light Intermediate
struct JudgementOfLightIntermediate : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        if (spell->GetTriggeredByAuraSpellInfo() == nullptr)
            return;

        uint32 triggerSpell = 0;
        switch (spell->GetTriggeredByAuraSpellInfo()->Id)
        {
            case 20185: triggerSpell = 20267; break; // Rank 1
            case 20344: triggerSpell = 20341; break; // Rank 2
            case 20345: triggerSpell = 20342; break; // Rank 3
            case 20346: triggerSpell = 20343; break; // Rank 4
        }
        if (triggerSpell)
            spell->GetUnitTarget()->CastSpell(nullptr, triggerSpell, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_HIDE_CAST_IN_COMBAT_LOG);
    }
};

// 1826 - Judgement of Wisdom Intermediate
struct JudgementOfWisdomIntermediate : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        if (spell->GetTriggeredByAuraSpellInfo() == nullptr)
            return;

        uint32 triggerSpell = 0;
        switch (spell->GetTriggeredByAuraSpellInfo()->Id)
        {
            case 20186: triggerSpell = 20268; break; // Rank 1
            case 20354: triggerSpell = 20352; break; // Rank 2
            case 20355: triggerSpell = 20353; break; // Rank 3
        }
        if (triggerSpell)
            spell->GetUnitTarget()->CastSpell(nullptr, triggerSpell, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_HIDE_CAST_IN_COMBAT_LOG);
    }
};

struct spell_judgement : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* unitTarget = spell->GetUnitTarget();
        if (!unitTarget || !unitTarget->IsAlive())
            return;

        Unit* caster = spell->GetCaster();

        if (!unitTarget || !unitTarget->IsAlive())
            return;

        uint32 spellId2 = 0;

        // all seals have aura dummy
        Unit::AuraList const& m_dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
        for (Unit::AuraList::const_iterator itr = m_dummyAuras.begin(); itr != m_dummyAuras.end(); ++itr)
        {
            SpellEntry const* spellInfo = (*itr)->GetSpellProto();

            // search seal (all seals have judgement's aura dummy spell id in 2 effect
            if (!spellInfo || !IsSealSpell((*itr)->GetSpellProto()) || (*itr)->GetEffIndex() != 2)
                continue;

            // must be calculated base at raw base points in spell proto, GetModifier()->m_value for S.Righteousness modified by SPELLMOD_DAMAGE
            spellId2 = (*itr)->GetSpellProto()->CalculateSimpleValue(EFFECT_INDEX_2);

            if (spellId2 <= 1)
                continue;

            // found, remove seal
            caster->RemoveAurasDueToSpell((*itr)->GetId());

            break;
        }

        caster->CastSpell(unitTarget, spellId2, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_IGNORE_HIT_CALCULATION);
    }
};

// 19977 - Blessing of Light
struct BlessingOfLight : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        aura->GetTarget()->RegisterScriptedLocationAura(aura, SCRIPT_LOCATION_SPELL_HEALING_TAKEN, apply);
    }

    void OnDamageCalculate(Aura* aura, Unit* attacker, Unit* /*victim*/, int32& advertisedBenefit, float& totalMod) const override
    {
        advertisedBenefit += (aura->GetModifier()->m_amount);  // BoL is penalized since 2.3.0
        // Note: This forces the caster to keep libram equipped, but works regardless if the BOL is his or not
        if (Aura* improved = attacker->GetAura(38320, EFFECT_INDEX_0)) // improved Blessing of light
        {
            if (aura->GetEffIndex() == EFFECT_INDEX_0)
                advertisedBenefit += improved->GetModifier()->m_amount; // holy light gets full amount
            else
                advertisedBenefit += (improved->GetModifier()->m_amount / 2); // flash of light gets half
        }
    }
};

// 19752 - Divine Intervention
struct DivineIntervention : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        Unit* target = spell->m_targets.getUnitTarget();
        if (!target)
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
        if (target->HasAura(23333) || target->HasAura(23335) || target->HasAura(34976)) // possibly SPELL_ATTR_EX_IMMUNITY_TO_HOSTILE_AND_FRIENDLY_EFFECTS
            return SPELL_FAILED_TARGET_AURASTATE;
        return SPELL_CAST_OK;
    }
};

// 20467, 20963, 20964, 20965, 20966, 27171 - Judgement of Command
struct JudgementOfCommand : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        if (!spell->GetUnitTarget()->IsStunned())
            spell->SetDamage(uint32(spell->GetDamage() / 2));
    }
};

void LoadPaladinScripts()
{
    RegisterSpellScript<JudgementOfLightIntermediate>("spell_judgement_of_light_intermediate");
    RegisterSpellScript<JudgementOfWisdomIntermediate>("spell_judgement_of_wisdom_intermediate");
    RegisterSpellScript<DivineIntervention>("spell_divine_intervention");
    RegisterSpellScript<spell_judgement>("spell_judgement");
    RegisterSpellScript<SealOfTheCrusader>("spell_seal_of_the_crusader");
    RegisterSpellScript<BlessingOfLight>("spell_blessing_of_light");
    RegisterSpellScript<JudgementOfCommand>("spell_judgement_of_command");
}