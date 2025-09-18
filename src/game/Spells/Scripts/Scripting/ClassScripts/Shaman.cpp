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
#include "Entities/Totem.h"
#include "AI/BaseAI/TotemAI.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"

struct SentryTotem : public SpellScript, public AuraScript
{
    void OnRadiusCalculate(Spell* spell, SpellEffectIndex effIdx, bool targetB, float& radius) const override
    {
        if (!targetB && effIdx == EFFECT_INDEX_0)
            radius = 2.f;
    }

    void OnSummon(Spell* spell, Creature* summon) const override
    {
        if (Player* player = dynamic_cast<Player*>(spell->GetCaster()))
            player->GetCamera().SetView(summon);
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        Unit* target = aura->GetTarget();
        if (!target->IsPlayer())
            return;

        Totem* totem = target->GetTotem(TOTEM_SLOT_AIR);

        if (totem && apply)
            static_cast<Player*>(target)->GetCamera().SetView(totem);
        else
            static_cast<Player*>(target)->GetCamera().ResetView();
    }
};

struct SentryTotemAI : public TotemAI
{
    using TotemAI::TotemAI;

    void RemoveAura()
    {
        if (Unit* spawner = m_creature->GetSpawner())
            spawner->RemoveAurasDueToSpell(m_creature->GetUInt32Value(UNIT_CREATED_BY_SPELL));
    }

    void JustDied(Unit* killer) override
    {
        TotemAI::JustDied(killer);
        RemoveAura();
    }

    void OnUnsummon() override
    {
        TotemAI::OnUnsummon();
        RemoveAura();
    }
};

// 29203 - Healing Way
struct HealingWay : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        aura->GetTarget()->RegisterScriptedLocationAura(aura, SCRIPT_LOCATION_SPELL_HEALING_TAKEN, apply);
    }

    void OnDamageCalculate(Aura* aura, Unit* /*attacker*/, Unit* /*victim*/, int32& /*advertisedBenefit*/, float& totalMod) const override
    {
        totalMod *= (aura->GetModifier()->m_amount + 100.0f) / 100.0f;
    }
};

// 8516 - Windfury Totem
struct WindfuryTotemAura : public AuraScript
{
    int32 OnAuraValueCalculate(AuraCalcData& data, int32 value) const override
    {
        if (data.castItem)
            value += (value * data.castItem->GetEnchantmentModifier() / 100);
        return value;
    }
};

// 8017 - Rockbiter Weapon
struct RockbiterWeaponSelector : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_1)
            return;

        Unit* caster = spell->GetCaster();
        uint32 spell_id = 0;
        switch (spell->m_spellInfo->Id) // selection per Elemental Weapons talent
        {
            case 8017:   // Rank 1
                if (caster->HasAura(29080))
                    spell_id = 36496;
                else if (caster->HasAura(29079))
                    spell_id = 36495;
                else if (caster->HasAura(16266))
                    spell_id = 36494;
                else
                    spell_id = 36744;
                break;
            case 8018: // Rank 2
                if (caster->HasAura(29080))
                    spell_id = 36752;
                else if (caster->HasAura(29079))
                    spell_id = 36751;
                else if (caster->HasAura(16266))
                    spell_id = 36750;
                else
                    spell_id = 36753;
                break;
            case 8019: // Rank 3
                if (caster->HasAura(29080))
                    spell_id = 36756;
                else if (caster->HasAura(29079))
                    spell_id = 36755;
                else if (caster->HasAura(16266))
                    spell_id = 36754;
                else
                    spell_id = 36757;
                break;
            case 10399: // Rank 4
                if (caster->HasAura(29080))
                    spell_id = 36760;
                else if (caster->HasAura(29079))
                    spell_id = 36759;
                else if (caster->HasAura(16266))
                    spell_id = 36758;
                else
                    spell_id = 36761;
                break;
            case 16314: // Rank 5
                if (caster->HasAura(29080))
                    spell_id = 36764;
                else if (caster->HasAura(29079))
                    spell_id = 36763;
                else if (caster->HasAura(16266))
                    spell_id = 36762;
                else
                    spell_id = 36765;
                break;
            case 16315: // Rank 6
                if (caster->HasAura(29080))
                    spell_id = 36768;
                else if (caster->HasAura(29079))
                    spell_id = 36767;
                else if (caster->HasAura(16266))
                    spell_id = 36766;
                else
                    spell_id = 36769;
                break;
            case 16316: // Rank 7
                if (caster->HasAura(29080))
                    spell_id = 36772;
                else if (caster->HasAura(29079))
                    spell_id = 36771;
                else if (caster->HasAura(16266))
                    spell_id = 36770;
                else
                    spell_id = 36771;
                break;
            case 25479: // Rank 8
                if (caster->HasAura(29080))
                    spell_id = 36776;
                else if (caster->HasAura(29079))
                    spell_id = 36775;
                else if (caster->HasAura(16266))
                    spell_id = 36774;
                else
                    spell_id = 36777;
                break;
            case 25485: // Rank 9
                if (caster->HasAura(29080))
                    spell_id = 36499;
                else if (caster->HasAura(29079))
                    spell_id = 36498;
                else if (caster->HasAura(16266))
                    spell_id = 36497;
                else
                    spell_id = 36499;
                break;
            default: sLog.outError("Spell::EffectDummy: Spell %u not handled in RW", spell->m_spellInfo->Id); return;
        }

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);

        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i", spell_id);
            return;
        }

        if (!caster->IsPlayer())
            return;

        for (int j = BASE_ATTACK; j <= OFF_ATTACK; ++j)
        {
            if (Item* item = static_cast<Player*>(caster)->GetWeaponForAttack(WeaponAttackType(j)))
            {
                if (item->IsFitToSpellRequirements(spell->m_spellInfo))
                {
                    SpellCastArgs args;
                    args.SetItemTarget(item);

                    caster->CastSpell(args, spellInfo, TRIGGERED_OLD_TRIGGERED);
                }
            }
        }
    }
};

void LoadShamanScripts()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_sentry_totem";
    pNewScript->GetAI = &GetNewAIInstance<SentryTotemAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<SentryTotem>("spell_sentry_totem");
    RegisterSpellScript<HealingWay>("spell_healing_way");
    RegisterSpellScript<WindfuryTotemAura>("spell_windfury_totem_aura");
    RegisterSpellScript<RockbiterWeaponSelector>("spell_rockbiter_weapon_selector");
}