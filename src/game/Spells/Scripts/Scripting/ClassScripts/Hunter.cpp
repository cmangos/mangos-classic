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

// 1515 - Tame Beast
struct TameBeastChannel : public SpellScript, public AuraScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        Unit* target = spell->m_targets.getUnitTarget();
        if (!target)
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
        if (target->GetLevel() > spell->GetCaster()->GetLevel())
            return SPELL_FAILED_HIGHLEVEL;
        return SPELL_CAST_OK;
    }

    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        data.caster = aura->GetCaster();
        data.target = aura->GetCaster();
    }
};

// 13535 - Tame Beast
struct TameBeastDummy : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (Unit* channelTarget = spell->GetCaster()->GetChannelObject())
        {
            if (channelTarget->GetTypeId() == TYPEID_UNIT)
                spell->GetCaster()->CastSpell(channelTarget, 13481, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

// 19572, 19573 - Improved Mend Pet
struct ImprovedMendPet : public AuraScript
{
    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const override
    {
        if (!roll_chance_i(aura->GetModifier()->m_amount))
            return SPELL_AURA_PROC_FAILED;

        procData.triggeredSpellId = 24406;
        procData.triggerTarget = procData.target;
        return SPELL_AURA_PROC_OK;
    }
};

// TODO: some evidence tbc pet growl scales with hunter AP

// 19678 - Tame Adult Plainstrider
struct TamingPetRodAura : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        Unit* caster = aura->GetCaster();
        Unit* target = aura->GetTarget();

        if (!target->IsCreature())
            return;

        Creature* creature = static_cast<Creature*>(target);

        if (apply)
        {
            if (caster && caster->IsPlayer())
                creature->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);   
        }
        else
        {
            if (aura->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                creature->ForcedDespawn();     
        }
    }
};

void LoadHunterScripts()
{
    RegisterSpellScript<TameBeastChannel>("spell_tame_beast_channel");
    RegisterSpellScript<TameBeastDummy>("spell_tame_beast_dummy");
    RegisterSpellScript<ImprovedMendPet>("spell_improved_mend_pet");
    RegisterSpellScript<TamingPetRodAura>("spell_taming_pet_rod");
}
