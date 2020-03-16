/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

/* ScriptData
SDName: Item_Scripts
SD%Complete: 100
SDComment: quest 4743
SDCategory: Items
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Spells/Spell.h"
#include "Spells/Scripts/SpellScript.h"
/* ContentData
EndContentData */

/*#####
# item_orb_of_draconic_energy
#####*/

enum
{
    SPELL_DOMINION_SOUL     = 16053,
    NPC_EMBERSTRIFE         = 10321
};

bool ItemUse_item_orb_of_draconic_energy(Player* pPlayer, Item* pItem, const SpellCastTargets& /*pTargets*/)
{
    Creature* pEmberstrife = GetClosestCreatureWithEntry(pPlayer, NPC_EMBERSTRIFE, 20.0f);
    if (!pEmberstrife)
        return false;

    // If Emberstrife already mind controled or above 10% HP: force spell cast failure
    if (pEmberstrife->HasAura(SPELL_DOMINION_SOUL) || pEmberstrife->GetHealthPercent() > 10.0f)
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DOMINION_SOUL))
            Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_TARGET_AURASTATE);

        return true;
    }

    return false;
}

enum
{
    SPELL_ASHBRINGER_EFFECT_001 = 28442,
};

struct AshbringerItemAura : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const
    {
        if (apply)
        {
            Unit* target = aura->GetTarget();
            int32 basepoints = ReputationRank(REP_FRIENDLY);
            target->CastCustomSpell(nullptr, SPELL_ASHBRINGER_EFFECT_001, &basepoints, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
        else
            aura->GetTarget()->RemoveAurasDueToSpell(SPELL_ASHBRINGER_EFFECT_001);
    }
};

enum
{
    SPELL_GDR_PERIODIC_DAMAGE = 13493,
    SPELL_GDR_DAMAGE_HIT      = 13279,
};

struct GDRChannel : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
            spell->GetCaster()->CastSpell(nullptr, SPELL_GDR_PERIODIC_DAMAGE, TRIGGERED_OLD_TRIGGERED);
    }
};

struct GDRPeriodicDamage : public AuraScript
{
    int32 OnAuraValueCalculate(Aura* /*aura*/, Unit* /*caster*/, int32 value) const override
    {
        return urand(100, 500);
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply && aura->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            int32 dmg = (int32)aura->GetScriptValue();
            aura->GetTarget()->CastCustomSpell(aura->GetTarget()->GetTarget(), SPELL_GDR_DAMAGE_HIT, &dmg, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

void AddSC_item_scripts()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "item_orb_of_draconic_energy";
    pNewScript->pItemUse = &ItemUse_item_orb_of_draconic_energy;
    pNewScript->RegisterSelf();

    RegisterAuraScript<AshbringerItemAura>("spell_ashbringer_item");

    RegisterSpellScript<GDRChannel>("spell_gdr_channel");
    RegisterAuraScript<GDRPeriodicDamage>("spell_gdr_periodic");
}
