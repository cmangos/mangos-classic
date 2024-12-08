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
SDName: Spell_Scripts
SD%Complete: 100
SDComment: Spell scripts for dummy effects (if script need access/interaction with parts of other AI or instance, add it in related source files)
SDCategory: Spell
EndScriptData

*/

/* ContentData
spell 21050
spell 26275
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Spells/Scripts/SpellScript.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

/* When you make a spell effect:
- always check spell id and effect index
- always return true when the spell is handled by script
*/

enum
{
    // npcs that are only interactable while dead
    SPELL_SHROUD_OF_DEATH               = 10848,
    SPELL_SPIRIT_PARTICLES              = 17327,
    NPC_FRANCLORN_FORGEWRIGHT           = 8888,
    NPC_GAERIYAN                        = 9299,

    // quest 6661
    SPELL_MELODIOUS_RAPTURE             = 21050,
    SPELL_MELODIOUS_RAPTURE_VISUAL      = 21051,
    NPC_DEEPRUN_RAT                     = 13016,
    NPC_ENTHRALLED_DEEPRUN_RAT          = 13017,
};

bool EffectAuraDummy_spell_aura_dummy_npc(const Aura* pAura, bool bApply)
{
    switch (pAura->GetId())
    {
        case SPELL_SHROUD_OF_DEATH:
        case SPELL_SPIRIT_PARTICLES:
        {
            Creature* pCreature = (Creature*)pAura->GetTarget();

            if (!pCreature || (pCreature->GetEntry() != NPC_FRANCLORN_FORGEWRIGHT && pCreature->GetEntry() != NPC_GAERIYAN))
                return false;

            if (bApply)
                pCreature->m_AuraFlags |= UNIT_AURAFLAG_ALIVE_INVISIBLE;
            else
                pCreature->m_AuraFlags &= ~UNIT_AURAFLAG_ALIVE_INVISIBLE;

            return false;
        }
    }

    return false;
}

bool EffectDummyCreature_spell_dummy_npc(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    switch (uiSpellId)
    {
        case SPELL_MELODIOUS_RAPTURE:
        {
            if (uiEffIndex == EFFECT_INDEX_0)
            {
                if (pCaster->GetTypeId() != TYPEID_PLAYER && pCreatureTarget->GetEntry() != NPC_DEEPRUN_RAT)
                    return true;

                pCreatureTarget->UpdateEntry(NPC_ENTHRALLED_DEEPRUN_RAT);
                pCreatureTarget->CastSpell(pCreatureTarget, SPELL_MELODIOUS_RAPTURE_VISUAL, TRIGGERED_NONE);
                pCreatureTarget->GetMotionMaster()->MoveFollow(pCaster, frand(0.5f, 3.0f), frand(M_PI_F * 0.8f, M_PI_F * 1.2f));

                ((Player*)pCaster)->KilledMonsterCredit(NPC_ENTHRALLED_DEEPRUN_RAT);
            }
            return true;
        }
    }

    return false;
}

struct GreaterInvisibilityMob : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (apply)
            aura->ForcePeriodicity(1 * IN_MILLISECONDS); // tick every second
    }

    void OnPeriodicTickEnd(Aura* aura) const override
    {
        Unit* target = aura->GetTarget();
        if (!target->IsCreature())
            return;

        Creature* invisible = static_cast<Creature*>(target);
        std::list<Unit*> nearbyTargets;
        MaNGOS::AnyUnitInObjectRangeCheck u_check(invisible, float(invisible->GetDetectionRange()));
        MaNGOS::UnitListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(nearbyTargets, u_check);
        Cell::VisitWorldObjects(invisible, searcher, invisible->GetDetectionRange());
        for (Unit* nearby : nearbyTargets)
        {
            if (invisible->CanAttackOnSight(nearby) && invisible->IsWithinLOSInMap(nearby, true))
            {
                invisible->AI()->AttackStart(nearby);
                return;
            }
        }
    }
};

/* *****************************
*  PX-238 Winter Wondervolt TRAP
*******************************/

struct WondervoltTrap : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            uint32 spells[4] = {26272, 26157, 26273, 26274};    // Four possible transform spells
            if (spell->GetUnitTarget())
            spell->GetUnitTarget()->CastSpell(spell->GetUnitTarget(), spells[urand(0, 3)], TRIGGERED_OLD_TRIGGERED);
        }
    }
};

/* ************************************************************
*  Arcane Cloaking
*  Quests 9121, 9122, 9123, 9378 - Naxxramas, The Dread Citadel
**************************************************************/

// 28006 - Arcane Cloaking
struct ArcaneCloaking : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            Unit* caster = spell->GetCaster();
            Unit* target = spell->GetUnitTarget();
            // Naxxramas Entry Flag Effect DND
            if (target && target->IsPlayer())
                caster->CastSpell(target, 29296, TRIGGERED_OLD_TRIGGERED);  // Cast Naxxramas Entry Flag Trigger DND
        }
    }
};

enum SpellVisualKitFoodOrDrink
{
    SPELL_VISUAL_KIT_FOOD = 406,
    SPELL_VISUAL_KIT_DRINK = 438
};

struct FoodAnimation : public AuraScript
{
    void OnHeartbeat(Aura* aura) const override
    {
        aura->GetTarget()->PlaySpellVisual(SPELL_VISUAL_KIT_FOOD);
    }
};

struct DrinkAnimation : public AuraScript
{
    void OnHeartbeat(Aura* aura) const override
    {
        aura->GetTarget()->PlaySpellVisual(SPELL_VISUAL_KIT_DRINK);
    }
};

struct spell_effect_summon_no_follow_movement : public SpellScript
{
    void OnSummon(Spell* /*spell*/, Creature* summon) const override
    {
        summon->AI()->SetFollowMovement(false);
    }
};

struct IncreasedHealingDoneDummy : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        aura->GetTarget()->RegisterScriptedLocationAura(aura, SCRIPT_LOCATION_SPELL_HEALING_DONE, apply);
    }

    void OnDamageCalculate(Aura* aura, Unit* /*attacker*/, Unit* /*victim*/, int32& advertisedBenefit, float& /*totalMod*/) const override
    {
        advertisedBenefit += aura->GetModifier()->m_amount;
    }
};

struct IncreasedSpellDamageDoneDummy : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        aura->GetTarget()->RegisterScriptedLocationAura(aura, SCRIPT_LOCATION_SPELL_DAMAGE_DONE, apply);
    }

    void OnDamageCalculate(Aura* aura, Unit* /*attacker*/, Unit* /*victim*/, int32& advertisedBenefit, float& /*totalMod*/) const override
    {
        advertisedBenefit += aura->GetModifier()->m_amount;
    }
};

struct spell_scourge_strike : public SpellScript
{
    bool OnCheckTarget(const Spell* /*spell*/, Unit* target, SpellEffectIndex /*eff*/) const override
    {
        if (target->IsPlayer() || (target->IsPlayerControlled()))
            return false;

        return true;
    }
};

enum
{
    SPELL_THISTLEFUR_DEATH = 8603,
    SPELL_RIVERPAW_DEATH   = 8655,
    SPELL_STROMGARDE_DEATH = 8894,
    SPELL_CRUSHRIDGE_DEATH = 9144,

    SAY_RAGE_FALLEN        = 1151,
};

struct TribalDeath : public SpellScript
{
    bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex /*eff*/) const override
    {
        uint32 entry = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_THISTLEFUR_DEATH: entry = 3925; break; // Thistlefur Avenger
            case SPELL_RIVERPAW_DEATH: entry = 0; break; // Unk
            case SPELL_STROMGARDE_DEATH: entry = 2585; break; // Stromgarde Vindicator
            case SPELL_CRUSHRIDGE_DEATH: entry = 2287; break; // Crushridge Warmonger
        }
        if (target->GetEntry() != entry)
            return false;
        return true;
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        uint32 spellId = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_THISTLEFUR_DEATH: spellId = 8602; break;
            case SPELL_RIVERPAW_DEATH: spellId = 0; break; // Unk
            case SPELL_STROMGARDE_DEATH: spellId = 8602; break;
            case SPELL_CRUSHRIDGE_DEATH: spellId = 8269; break;
        }
        Unit* target = spell->GetUnitTarget();
        Unit* caster = spell->GetCaster();
        target->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
        if (!target->IsInCombat())
            if (Unit* killer = target->GetMap()->GetUnit(static_cast<Creature*>(target)->GetKillerGuid()))
                target->AI()->AttackStart(killer);

        if (spell->m_spellInfo->Id == SPELL_CRUSHRIDGE_DEATH)
            DoBroadcastText(SAY_RAGE_FALLEN, target, caster);
    }
};

struct RetaliationCreature : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        if (!spell->m_targets.getUnitTarget() || !spell->GetCaster()->HasInArc(spell->m_targets.getUnitTarget()))
            return SPELL_FAILED_CASTER_AURASTATE;

        return SPELL_CAST_OK;
    }
};

// 11920 - Net Guard
struct NetGuard : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0)
            return;

        spell->GetCaster()->getThreatManager().modifyThreatPercent(spell->GetUnitTarget(), -50);
    }
};

struct HateToHalf : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0)
            return;

        spell->GetCaster()->getThreatManager().modifyThreatPercent(spell->GetUnitTarget(), -50);
    }
};

struct HateToZero : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx != EFFECT_INDEX_0)
            return;

        spell->GetCaster()->getThreatManager().modifyThreatPercent(spell->GetUnitTarget(), -100);
    }
};

struct InstillLordValthalaksSpirit : public SpellScript
{
    void OnSummon(Spell* spell, Creature* summon) const override
    {
        spell->GetCaster()->AddCreature(spell->m_spellInfo->Id, summon);
    }
};

struct Stoned : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        Unit* target = aura->GetTarget();
        if (apply)
        {
            if (target->GetTypeId() != TYPEID_UNIT)
                return;

            if (target->GetEntry() == 25507)
                return;

            target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_UNINTERACTIBLE);
            target->addUnitState(UNIT_STAT_ROOT);
        }
        else
        {
            if (target->GetTypeId() != TYPEID_UNIT)
                return;

            if (target->GetEntry() == 25507)
                return;

            // see dummy effect of spell 10254 for removal of flags etc
            target->CastSpell(nullptr, 10254, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

struct GameobjectCallForHelpOnUsage : public SpellScript
{
    void OnSuccessfulStart(Spell* spell) const
    {
        UnitList targets;
        MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck check(spell->GetCaster(), 12.f);
        MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, check);
        Cell::VisitAllObjects(spell->GetCaster(), searcher, 12.f);
        for (Unit* attacker : targets)
        {
            if (attacker->IsCreature() && static_cast<Creature*>(attacker)->IsCritter())
                continue;

            if (!spell->GetCaster()->IsEnemy(attacker))
                continue;

            if (attacker->AI())
                attacker->AI()->AttackStart(spell->GetCaster());
        }
    }
};

// s.7131 - npc 2638,4785,5097,6493,6932,11027,11263 - might be different delay per npc!
struct IllusionPassive : public AuraScript
{
    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& /*procData*/) const override
    {
        if (Unit* caster = aura->GetCaster())
            if (caster->IsCreature())
                static_cast<Creature*>(caster)->ForcedDespawn(1000);
        return SPELL_AURA_PROC_OK;
    }
};

void AddSC_spell_scripts()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "spell_dummy_npc";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_dummy_npc;
    pNewScript->pEffectAuraDummy = &EffectAuraDummy_spell_aura_dummy_npc;
    pNewScript->RegisterSelf();

    RegisterSpellScript<WondervoltTrap>("spell_wondervolt_trap");
    RegisterSpellScript<ArcaneCloaking>("spell_arcane_cloaking");
    RegisterSpellScript<GreaterInvisibilityMob>("spell_greater_invisibility_mob");
    RegisterSpellScript<FoodAnimation>("spell_food_animation");
    RegisterSpellScript<DrinkAnimation>("spell_drink_animation");
    RegisterSpellScript<spell_effect_summon_no_follow_movement>("spell_effect_summon_no_follow_movement");
    RegisterSpellScript<IncreasedHealingDoneDummy>("spell_increased_healing_done_dummy");
    RegisterSpellScript<IncreasedSpellDamageDoneDummy>("spell_increased_spell_damage_done_dummy");
    RegisterSpellScript<spell_scourge_strike>("spell_scourge_strike");
    RegisterSpellScript<TribalDeath>("spell_tribal_death");
    RegisterSpellScript<InstillLordValthalaksSpirit>("spell_instill_lord_valthalaks_spirit");
    RegisterSpellScript<RetaliationCreature>("spell_retaliation_creature");
    RegisterSpellScript<NetGuard>("spell_net_guard");
    RegisterSpellScript<HateToHalf>("spell_hate_to_half");
    RegisterSpellScript<HateToZero>("spell_hate_to_zero");
    RegisterSpellScript<Stoned>("spell_stoned");
    RegisterSpellScript<GameobjectCallForHelpOnUsage>("spell_gameobject_call_for_help_on_usage");
    RegisterSpellScript<IllusionPassive>("spell_illusion_passive");
}
