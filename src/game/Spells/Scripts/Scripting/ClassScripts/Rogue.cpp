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

// 14185 - Preparation
struct Preparation : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (spell->GetCaster()->IsPlayer())
        {
            // immediately finishes the cooldown on certain Rogue abilities
            auto cdCheck = [](SpellEntry const& spellEntry) -> bool { return (spellEntry.SpellFamilyName == SPELLFAMILY_ROGUE && spellEntry.Id != 14185); };
            static_cast<Player*>(spell->GetCaster())->RemoveSomeCooldown(cdCheck);
        }
    }
};

enum
{
    SPELL_DISTRACT      = 1725,
    SPELL_EARTHBIND     = 3600,
};

// Warning: Also currently used by Prowl
// 1784 - Stealth
struct Stealth : public AuraScript
{
    bool OnCheckProc(Aura* /*aura*/, ProcExecutionData& data) const override // per 1.12.0 patch notes - no other indication of how it works
    {
        if (data.spell)
        {
            switch (data.spell->m_spellInfo->Id)
            {
                case SPELL_EARTHBIND:
                    return false;
            }
        }
        return true;
    }
};

void CastHighestStealthRank(Unit* caster)
{
    if (!caster->IsPlayer())
        return;

    // get highest rank of the Stealth spell
    uint32 spellId = static_cast<Player*>(caster)->LookupHighestLearnedRank(1784);
    if (!spellId)
        return;
    SpellEntry const* stealthSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    // no Stealth spell found
    if (!stealthSpellEntry)
        return;

    // reset cooldown on it if needed
    if (!caster->IsSpellReady(*stealthSpellEntry))
        caster->RemoveSpellCooldown(*stealthSpellEntry);

    caster->CastSpell(nullptr, stealthSpellEntry, TRIGGERED_OLD_TRIGGERED);
}

// 1856 - Vanish
struct VanishRogue : public SpellScript
{
    void OnCast(Spell* spell) const override
    {
        CastHighestStealthRank(spell->GetCaster());
        // meant to be hooked like override scripts but we dont have that functionality yet
        if (spell->GetCaster()->HasAura(23582, EFFECT_INDEX_0)) // amount has trigger chance 100
            spell->GetCaster()->CastSpell(nullptr, 23583, TRIGGERED_OLD_TRIGGERED);
    }
};

// 6770 - Sap
struct SapRogue : public SpellScript
{
    // SPELL_ATTR_EX3_SUPPRESS_TARGET_PROCS prevents sap to proc stealth normally
    void OnHit(Spell* spell, SpellMissInfo missInfo) const override
    {
        if (missInfo == SPELL_MISS_NONE && spell->GetUnitTarget())
            spell->GetUnitTarget()->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
    }
};

// 14076 - Improved Sap
struct ImprovedSap : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        CastHighestStealthRank(spell->GetCaster());
    }
};

// 13983 - Setup
struct SetupRogue : public AuraScript
{
    bool OnCheckProc(Aura* /*aura*/, ProcExecutionData& data) const override
    {
        return data.victim->GetTarget() == data.attacker;
    }
};

void LoadRogueScripts()
{
    RegisterSpellScript<Preparation>("spell_preparation");
    RegisterSpellScript<Stealth>("spell_stealth");
    RegisterSpellScript<VanishRogue>("spell_vanish");
    RegisterSpellScript<SapRogue>("spell_sap");
    RegisterSpellScript<ImprovedSap>("spell_improved_sap");
    RegisterSpellScript<SetupRogue>("spell_setup_rogue");
}