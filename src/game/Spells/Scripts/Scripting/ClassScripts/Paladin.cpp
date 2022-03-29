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
    void OnApply(Aura* aura, bool apply) const
    {
        if (aura->GetEffIndex() != EFFECT_INDEX_1)
            return;

        // Seal of the Crusader damage reduction
        // SotC increases attack speed but reduces damage to maintain the same DPS
        float reduction = (-100.0f * aura->GetModifier()->m_amount) / (aura->GetModifier()->m_amount + 100.0f);
        aura->GetTarget()->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, reduction, apply);
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

void LoadPaladinScripts()
{
    RegisterSpellScript<spell_judgement>("spell_judgement");
    RegisterSpellScript<SealOfTheCrusader>("spell_seal_of_the_crusader");
}