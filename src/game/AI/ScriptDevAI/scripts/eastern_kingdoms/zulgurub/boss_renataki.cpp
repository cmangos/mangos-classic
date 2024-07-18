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
SDName: Boss_Renataki
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SPELL_THOUSAND_BLADES   = 24649,
    SPELL_VANISH            = 24699,
    SPELL_GOUGE             = 24698,
    SPELL_TRASH             = 3391,

    SPELL_VANISH_TELEPORT   = 24700,

    SPELL_SPIRIT_PARTICLES      = 18951,
    SPELL_SPAWN_RED_LIGHTNING   = 24240,
};

enum RenatakiActions
{
    RENATAKI_ACTION_MAX,
    RENATAKI_VANISH_DELAY,
};

struct boss_renatakiAI : public CombatAI
{
    boss_renatakiAI(Creature* creature) : CombatAI(creature, RENATAKI_ACTION_MAX)
    {
        AddCustomAction(RENATAKI_VANISH_DELAY, true, [&]()
        {
            if (Unit* target = m_creature->GetMap()->GetUnit(m_vanishTarget))
                target->CastSpell(m_creature, SPELL_VANISH_TELEPORT, TRIGGERED_OLD_TRIGGERED);
            SetCombatScriptStatus(false);
            SetMeleeEnabled(true);
            SetCombatMovement(true);
        });
    }

    ObjectGuid m_vanishTarget;

    void Reset() override
    {
        CombatAI::Reset();

        SetCombatScriptStatus(false);
        SetMeleeEnabled(true);
        SetCombatMovement(true);
    }

    void JustRespawned() override
    {
        CombatAI::JustRespawned();

        DoCastSpellIfCan(nullptr, SPELL_SPIRIT_PARTICLES);
        DoCastSpellIfCan(nullptr, SPELL_SPAWN_RED_LIGHTNING);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            m_vanishTarget = invoker->GetObjectGuid();
            SetCombatScriptStatus(true);
            SetMeleeEnabled(false);
            SetCombatMovement(false);
        }
    }
};

// 24649 - Thousand Blades
struct ThousandBladesRenataki : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (spell->m_IsTriggeredSpell) // only initial normal cast
            return;

        if (effIdx != EFFECT_INDEX_1)
            return;

        std::vector<Unit*> selectedTargets;
        Unit* caster = spell->GetCaster();
        caster->SelectAttackingTargets(selectedTargets, ATTACKING_TARGET_ALL_SUITABLE, 0, SPELL_THOUSAND_BLADES, SELECT_FLAG_PLAYER);
        // remove current target
        selectedTargets.erase(std::remove(selectedTargets.begin(), selectedTargets.end(), spell->m_targets.getUnitTarget()), selectedTargets.end());
        std::shuffle(selectedTargets.begin(), selectedTargets.end(), *GetRandomGenerator());
        selectedTargets.resize(9);
        for (Unit* target : selectedTargets)
            caster->CastSpell(target, SPELL_THOUSAND_BLADES, TRIGGERED_OLD_TRIGGERED | TRIGGERED_INSTANT_CAST);
    }
};

struct RenatakiVanish : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* caster = spell->GetCaster();
        if (effIdx != EFFECT_INDEX_0 || !caster->AI())
            return;

        caster->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, spell->GetUnitTarget(), caster);
    }
};

struct RenatakiVanishTeleport : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply)
        {
            aura->GetTarget()->CastSpell(aura->GetCaster(), SPELL_TRASH, TRIGGERED_OLD_TRIGGERED);
            aura->GetTarget()->RemoveAurasDueToSpell(SPELL_VANISH);
        }
    }
};

void AddSC_boss_renataki()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_renataki";
    pNewScript->GetAI = &GetNewAIInstance<boss_renatakiAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<ThousandBladesRenataki>("spell_thousand_blades_renataki");
    RegisterSpellScript<RenatakiVanish>("spell_renataki_vanish");
    RegisterSpellScript<RenatakiVanishTeleport>("spell_renataki_vanish_teleport");
}
