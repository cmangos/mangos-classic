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

struct SpiritOfRedemptionHeal : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
            spell->SetDamage(spell->GetCaster()->GetMaxHealth());
    }
};

struct PowerInfusion : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool/* strict*/) const override
    {
        // Patch 1.10.2 (2006-05-02):
        // Power Infusion: This aura will no longer stack with Arcane Power. If you attempt to cast it on someone with Arcane Power, the spell will fail.
        if (Unit* target = spell->m_targets.getUnitTarget())
            if (target->GetAuraCount(12042))
                return SPELL_FAILED_AURA_BOUNCED;

        return SPELL_CAST_OK;
    }
};

struct Blackout : public AuraScript
{
    bool OnCheckProc(Aura* /*aura*/, ProcExecutionData& data) const override
    {
        if (data.isHeal || (!data.damage && data.spellInfo && !IsSpellHaveAura(data.spellInfo, SPELL_AURA_PERIODIC_DAMAGE)))
            return false;
        return true;
    }
};

struct Shadowguard : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        if (!spell->GetTriggeredByAuraSpellInfo())
            return;

        uint32 spellId = 0;
        switch (spell->GetTriggeredByAuraSpellInfo()->Id)
        {
            default:
            case 18137: spellId = 28377; break;   // Rank 1
            case 19308: spellId = 28378; break;   // Rank 2
            case 19309: spellId = 28379; break;   // Rank 3
            case 19310: spellId = 28380; break;   // Rank 4
            case 19311: spellId = 28381; break;   // Rank 5
            case 19312: spellId = 28382; break;   // Rank 6
        }

        if (spellId)
            spell->GetCaster()->CastSpell(spell->GetUnitTarget(), spellId, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL);
    }
};

void LoadPriestScripts()
{
    RegisterSpellScript<PowerInfusion>("spell_power_infusion");
    RegisterSpellScript<SpiritOfRedemptionHeal>("spell_spirit_of_redemption_heal");
    RegisterSpellScript<Blackout>("spell_blackout");
    RegisterSpellScript<Shadowguard>("spell_shadowguard");
}
