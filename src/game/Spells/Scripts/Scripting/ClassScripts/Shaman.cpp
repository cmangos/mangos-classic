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

    void AttackStart(Unit* who) override
    {
        TotemAI::AttackStart(who);
        // Sentry totem sends ping on attack
        if (Player* owner = dynamic_cast<Player*>(m_creature->GetSpawner()))
        {
            WorldPacket data(MSG_MINIMAP_PING, (8 + 4 + 4));
            data << m_creature->GetObjectGuid();
            data << m_creature->GetPositionX();
            data << m_creature->GetPositionY();
            owner->SendDirectMessage(data);
        }
    }

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

    void OnDamageCalculate(Aura* aura, Unit* /*victim*/, int32& /*advertisedBenefit*/, float& totalMod) const override
    {
        totalMod *= (aura->GetModifier()->m_amount + 100.0f) / 100.0f;
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
}