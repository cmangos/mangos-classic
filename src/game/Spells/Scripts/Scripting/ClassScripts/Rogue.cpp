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

struct spell_preparation : public SpellScript
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
struct Stealth : public AuraScript
{
    bool OnCheckProc(Aura* /*aura*/, ProcExecutionData& data) const override // per 1.12.0 patch notes - no other indication of how it works
    {
        if (data.spell)
        {
            switch (data.spell->m_spellInfo->Id)
            {
                case SPELL_DISTRACT:
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
    SpellEntry const* stealthSpellEntry = nullptr;
    const PlayerSpellMap& sp_list = static_cast<Player*>(caster)->GetSpellMap();
    for (const auto& itr : sp_list)
    {
        // only highest rank is shown in spell book, so simply check if shown in spell book
        if (!itr.second.active || itr.second.disabled || itr.second.state == PLAYERSPELL_REMOVED)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr.first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsFitToFamily(SPELLFAMILY_ROGUE, uint64(0x0000000000400000)))
        {
            stealthSpellEntry = spellInfo;
            break;
        }
    }

    // no Stealth spell found
    if (!stealthSpellEntry)
        return;

    // reset cooldown on it if needed
    if (!caster->IsSpellReady(*stealthSpellEntry))
        caster->RemoveSpellCooldown(*stealthSpellEntry);

    caster->CastSpell(nullptr, stealthSpellEntry, TRIGGERED_OLD_TRIGGERED);
}

struct VanishRogue : public SpellScript
{
    void OnCast(Spell* spell) const override
    {
        CastHighestStealthRank(spell->GetCaster());
    }
};

struct ImprovedSap : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        CastHighestStealthRank(spell->GetCaster());
    }
};

void LoadRogueScripts()
{
    RegisterSpellScript<spell_preparation>("spell_preparation");
    RegisterAuraScript<Stealth>("spell_stealth");
    RegisterSpellScript<VanishRogue>("spell_vanish");
    RegisterSpellScript<ImprovedSap>("spell_improved_sap");
}