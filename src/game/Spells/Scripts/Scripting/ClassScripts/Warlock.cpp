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

struct CurseOfAgony : public AuraScript
{
    void OnPeriodicCalculateAmount(Aura* aura, uint32& amount) const override
    {
        // 1..4 ticks, 1/2 from normal tick damage
        if (aura->GetAuraTicks() <= 4)
            amount = amount / 2;
        // 9..12 ticks, 3/2 from normal tick damage
        else if (aura->GetAuraTicks() >= 9)
            amount += (amount + 1) / 2; // +1 prevent 0.5 damage possible lost at 1..4 ticks
        // 5..8 ticks have normal tick damage
    }
};

struct LifeTap : public SpellScript
{
    void OnInit(Spell* spell) const override
    {
        float cost = spell->m_currentBasePoints[EFFECT_INDEX_0];

        Unit* caster = spell->GetCaster();
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(spell->m_spellInfo->Id, SPELLMOD_COST, cost);

        int32 dmg = caster->SpellDamageBonusDone(caster, GetSpellSchoolMask(spell->m_spellInfo), spell->m_spellInfo, EFFECT_INDEX_0, uint32(cost > 0 ? cost : 0), SPELL_DIRECT_DAMAGE);
        dmg = caster->SpellDamageBonusTaken(caster, GetSpellSchoolMask(spell->m_spellInfo), spell->m_spellInfo, EFFECT_INDEX_0, dmg, SPELL_DIRECT_DAMAGE);
        spell->SetScriptValue(dmg);
    }

    SpellCastResult OnCheckCast(Spell* spell, bool strict) const override
    {
        if (spell->GetScriptValue() > int32(spell->GetCaster()->GetHealth()))
            return SPELL_FAILED_FIZZLE;

        if (!strict)
        {
            int32 dmg = spell->GetScriptValue();
            if (Aura* aura = spell->GetCaster()->GetAura(28830, EFFECT_INDEX_0))
                dmg += dmg * aura->GetModifier()->m_amount / 100;
            spell->SetPowerCost(dmg);
        }
        return SPELL_CAST_OK;
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* caster = spell->GetCaster();
        int32 mana = spell->GetScriptValue();

        Unit::AuraList const& auraDummy = caster->GetAurasByType(SPELL_AURA_DUMMY);
        for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
        {
            if ((*itr)->isAffectedOnSpell(spell->m_spellInfo))
            {
                switch ((*itr)->GetSpellProto()->Id)
                {
                    case 28830: // Plagueheart Rainment - reduce hp cost
                        break;
                        // Improved Life Tap
                    default: mana = ((*itr)->GetModifier()->m_amount + 100) * mana / 100; break;
                }
            }
        }

        caster->CastCustomSpell(nullptr, 31818, &mana, nullptr, nullptr, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL | TRIGGERED_HIDE_CAST_IN_COMBAT_LOG);

        // Mana Feed
        int32 manaFeedVal = caster->CalculateSpellEffectValue(caster, spell->m_spellInfo, EFFECT_INDEX_1);
        manaFeedVal = manaFeedVal * mana / 100;
        if (manaFeedVal > 0 && caster->GetPet())
            caster->CastCustomSpell(nullptr, 32553, &manaFeedVal, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);            
    }
};

// 5699 - Create Healthstone
struct CreateHealthStoneWarlock : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        Unit* caster = spell->GetCaster();
        // check if we already have a healthstone
        uint32 itemType = GetUsableHealthStoneItemType(caster, spell->m_spellInfo);
        if (itemType && caster->IsPlayer() && static_cast<Player*>(caster)->GetItemCount(itemType) > 0)
            return SPELL_FAILED_TOO_MANY_OF_ITEM;
        return SPELL_CAST_OK;
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (!target)
            return;

        uint32 itemType = GetUsableHealthStoneItemType(target, spell->m_spellInfo);
        if (itemType)
            spell->DoCreateItem(effIdx, itemType);
    }

    uint32 GetUsableHealthStoneItemType(Unit* target, SpellEntry const* spellInfo) const
    {
        if (!target || !target->IsPlayer())
            return 0;

        uint32 itemtype = 0;
        uint32 rank = 0;
        Unit::AuraList const& mDummyAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
        for (auto mDummyAura : mDummyAuras)
        {
            if (mDummyAura->GetId() == 18692)
            {
                rank = 1;
                break;
            }
            if (mDummyAura->GetId() == 18693)
            {
                rank = 2;
                break;
            }
        }

        static uint32 const itypes[6][3] =
        {
            { 5512, 19004, 19005},              // Minor Healthstone
            { 5511, 19006, 19007},              // Lesser Healthstone
            { 5509, 19008, 19009},              // Healthstone
            { 5510, 19010, 19011},              // Greater Healthstone
            { 9421, 19012, 19013}               // Major Healthstone
        };

        switch (spellInfo->Id)
        {
            case  6201:
                itemtype = itypes[0][rank]; break; // Minor Healthstone
            case  6202:
                itemtype = itypes[1][rank]; break; // Lesser Healthstone
            case  5699:
                itemtype = itypes[2][rank]; break; // Healthstone
            case 11729:
                itemtype = itypes[3][rank]; break; // Greater Healthstone
            case 11730:
                itemtype = itypes[4][rank]; break; // Major Healthstone
        }
        return itemtype;
    }
};

struct SoulLink : public AuraScript
{
    void OnAuraInit(Aura* aura) const override
    {
        if (aura->GetEffIndex() == EFFECT_INDEX_0)
            aura->GetModifier()->m_auraname = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
    }
};

struct CurseOfDoom : public SpellScript, public AuraScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        // not allow cast at player
        Unit* target = spell->m_targets.getUnitTarget();
        if (!target || target->GetTypeId() == TYPEID_PLAYER)
            return SPELL_FAILED_BAD_TARGETS;
        return SPELL_CAST_OK;
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply && aura->GetRemoveMode() == AURA_REMOVE_BY_DEATH && urand(0, 100) > 95)
            if (Unit* caster = aura->GetCaster())
                caster->CastSpell(nullptr, 18662, TRIGGERED_OLD_TRIGGERED);
    }
};

struct CurseOfDoomEffect : public SpellScript
{
    void OnSummon(Spell* spell, Creature* summon) const override
    {
        summon->CastSpell(nullptr, 42010, TRIGGERED_OLD_TRIGGERED);
    }
};

struct DevourMagic : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool strict) const override
    {
        Unit* target = spell->m_targets.getUnitTarget();
        Unit* caster = spell->GetCaster();
        if (target && caster)
        {
            auto auras = target->GetSpellAuraHolderMap();
            for (auto itr : auras)
            {
                SpellEntry const* spell = itr.second->GetSpellProto();
                if (itr.second->GetTarget()->GetObjectGuid() != caster->GetObjectGuid() && spell->Dispel == DISPEL_MAGIC && (caster->CanAssist(target) ? !IsPositiveSpell(spell) : IsPositiveSpell(spell)) && !IsPassiveSpell(spell))
                    return SPELL_CAST_OK;
            }
        }
        return SPELL_FAILED_NOTHING_TO_DISPEL;
    }
};

void LoadWarlockScripts()
{
    RegisterSpellScript<CurseOfAgony>("spell_curse_of_agony");
    RegisterSpellScript<LifeTap>("spell_life_tap");
    RegisterSpellScript<CreateHealthStoneWarlock>("spell_create_health_stone_warlock");
    RegisterSpellScript<SoulLink>("spell_soul_link");
    RegisterSpellScript<DevourMagic>("spell_devour_magic");
    RegisterSpellScript<CurseOfDoom>("spell_curse_of_doom");
    RegisterSpellScript<CurseOfDoomEffect>("spell_curse_of_doom_effect");
}
