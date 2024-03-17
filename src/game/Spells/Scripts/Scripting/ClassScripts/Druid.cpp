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

// 18562 - Swiftmend
struct Swiftmend : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        Unit* target = spell->m_targets.getUnitTarget();
        if (!target || !target->GetAura(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, uint64(0x50))) // Rejuvenation or Regrowth
            return SPELL_FAILED_TARGET_AURASTATE;
        return SPELL_CAST_OK;
    }
};

enum
{
    SPELL_BLESSING_OF_THE_CLAW = 28750,
};

struct Regrowth : public AuraScript
{
    void OnPeriodicTickEnd(Aura* aura) const override
    {
        if (Unit* caster = aura->GetCaster())
            if (caster->HasOverrideScript(4537))
                caster->CastSpell(aura->GetTarget(), SPELL_BLESSING_OF_THE_CLAW, TRIGGERED_OLD_TRIGGERED);
    }
};

struct FormScalingAttackPowerAuras : public AuraScript
{
    int32 OnAuraValueCalculate(AuraCalcData& data, int32 value) const override
    {
        if (data.spellProto->EffectApplyAuraName[data.effIdx] == SPELL_AURA_MOD_ATTACK_POWER)
        {
            // Predatory Strikes
            Aura* predatoryStrikes = nullptr;
            if (Aura* aura = data.target->GetAura(16975, EFFECT_INDEX_0)) // rank 3
                predatoryStrikes = aura;
            else if (Aura* aura = data.target->GetAura(16974, EFFECT_INDEX_0)) // rank 2
                predatoryStrikes = aura;
            else if (Aura* aura = data.target->GetAura(16972, EFFECT_INDEX_0)) // rank 1
                predatoryStrikes = aura;
            if (predatoryStrikes)
                value += data.target->GetLevel() * predatoryStrikes->GetAmount() / 100;
        }
        return value;
    }
};

void LoadDruidScripts()
{
    RegisterSpellScript<Swiftmend>("spell_swiftmend");
    RegisterSpellScript<Regrowth>("spell_regrowth");
    RegisterSpellScript<FormScalingAttackPowerAuras>("spell_druid_form_scaling_ap_auras");
}