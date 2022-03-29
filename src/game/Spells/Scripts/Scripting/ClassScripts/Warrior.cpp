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

struct WarriorExecute : public SpellScript
{
    void OnCast(Spell* spell) const override // confirmed main spell can not hit and child still hits
    {
        int32 basePoints0 = spell->GetCaster()->CalculateSpellEffectValue(spell->m_targets.getUnitTarget(), spell->m_spellInfo, SpellEffectIndex(0))
            + int32((spell->GetCaster()->GetPower(POWER_RAGE)) * spell->m_spellInfo->DmgMultiplier[0]);
        SpellCastResult result = spell->GetCaster()->CastCustomSpell(spell->m_targets.getUnitTarget(), 20647, &basePoints0, nullptr, nullptr, TRIGGERED_IGNORE_CURRENT_CASTED_SPELL);
    }
};

struct WarriorExecuteDamage : public SpellScript
{
    void OnHit(Spell* spell, SpellMissInfo missInfo) const override
    {
        if (missInfo == SPELL_MISS_NONE)
            spell->GetCaster()->SetPower(POWER_RAGE, 0);
    }
};

struct Bloodrage : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (apply)
            aura->GetTarget()->SetInCombatState(false);
        aura->GetTarget()->GetCombatManager().SetForcedCombat(apply);
    }
};

struct RetaliationWarrior : public AuraScript
{
    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const override
    {
        // check attack comes not from behind
        if (procData.victim->IsFacingTargetsBack(procData.attacker))
            return SPELL_AURA_PROC_FAILED;

        procData.victim->CastSpell(procData.attacker, 20240, TRIGGERED_IGNORE_HIT_CALCULATION | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_COSTS);
        return SPELL_AURA_PROC_OK;
    }
};

void LoadWarriorScripts()
{
    RegisterSpellScript<WarriorExecute>("spell_warrior_execute");
    RegisterSpellScript<WarriorExecuteDamage>("spell_warrior_execute_damage");
    RegisterSpellScript<Bloodrage>("spell_bloodrage");
    RegisterSpellScript<RetaliationWarrior>("spell_retaliation_warrior");
}