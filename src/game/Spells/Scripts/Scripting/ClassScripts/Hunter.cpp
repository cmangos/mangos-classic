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

struct TameBeastChannel : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        data.caster = aura->GetCaster();
        data.target = aura->GetCaster();
    }
};

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

void LoadHunterScripts()
{
    RegisterSpellScript<TameBeastChannel>("spell_tame_beast_channel");
    RegisterSpellScript<TameBeastDummy>("spell_tame_beast_dummy");
}
