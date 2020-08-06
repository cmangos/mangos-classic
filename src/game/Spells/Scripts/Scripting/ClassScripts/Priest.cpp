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

struct SpiritOfRedemptionHeal : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
            spell->SetDamage(spell->GetCaster()->GetMaxHealth());
    }
};

struct Blackout : public AuraScript
{
    bool OnCheckProc(Aura* /*aura*/, ProcExecutionData& data) const override
    {
        if (!data.damage || data.isHeal)
            return false;
        return true;
    }
};

void LoadPriestScripts()
{
    RegisterSpellScript<SpiritOfRedemptionHeal>("spell_spirit_of_redemption_heal");
    RegisterAuraScript<Blackout>("spell_blackout");
}
