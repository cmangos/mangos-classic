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
SDName: Boss_Thaddius
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

/* ContentData
boss_thaddius
boss_stalagg
boss_feugen
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    // Stalagg
    SAY_STAL_AGGRO                  = 13083,
    SAY_STAL_SLAY                   = 13085,
    SAY_STAL_DEATH                  = 13084,

    // Feugen
    SAY_FEUG_AGGRO                  = 13023,
    SAY_FEUG_SLAY                   = 13025,
    SAY_FEUG_DEATH                  = 13024,

    // Thaddus
    SAY_AGGRO_1                     = 13086,
    SAY_AGGRO_2                     = 13087,
    SAY_AGGRO_3                     = 13088,
    SAY_SLAY                        = 13096,
    SAY_ELECT                       = 13090,
    SAY_DEATH                       = 13089,
    EMOTE_TESLA_OVERLOAD            = 12178,

    // Thaddius Spells
    SPELL_THADIUS_SPAWN             = 28160,
    SPELL_THADIUS_LIGHTNING_VISUAL  = 28136,
    SPELL_BALL_LIGHTNING            = 28299,
    SPELL_CHAIN_LIGHTNING           = 28167,
    SPELL_POLARITY_SHIFT            = 28089,
    SPELL_POSITIVE_CHARGE           = 28059,
    SPELL_NEGATIVE_CHARGE           = 28084,
    SPELL_POSITIVE_CHARGE_BUFF      = 29659,
    SPELL_NEGATIVE_CHARGE_BUFF      = 29660,
    SPELL_POSITIVE_CHARGE_DAMAGE    = 28062,
    SPELL_NEGATIVE_CHARGE_DAMAGE    = 28085,
    SPELL_BESERK                    = 27680,

    // Stalagg & Feugen Spells
    SPELL_WARSTOMP                  = 28125,
    SPELL_MAGNETIC_PULL_A           = 28338,
    SPELL_MAGNETIC_PULL_B           = 28339,
    SPELL_STATIC_FIELD              = 28135,
    SPELL_POWERSURGE                = 28134,
//    SPELL_FEUGEN_INITIALIZE         = 28138,      // usage unknown
//    SPELL_STALAGG_PASSIVE           = 28139,      // usage unknown

    // Tesla Spells
    SPELL_FEUGEN_CHAIN              = 28111,
    SPELL_STALAGG_CHAIN             = 28096,
    SPELL_FEUGEN_TESLA_PASSIVE      = 28109,
    SPELL_STALAGG_TESLA_PASSIVE     = 28097,
    SPELL_SHOCK_OVERLOAD            = 28159,
//    SPELL_SHOCK                     = 28099,      // Used in SpellEffects.cpp
    SPELL_TRIGGER_TESLAS            = 28359
};

/************
** boss_thaddius
************/

enum ThaddiusActions
{
    THADDIUS_ACTION_MAX,
};

struct boss_thaddiusAI : public BossAI
{
    boss_thaddiusAI(Creature* creature) : BossAI(creature, THADDIUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_THADDIUS);
        AddOnAggroText(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3);
        AddOnKillText(SAY_SLAY);
        AddOnDeathText(SAY_DEATH);
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        BossAI::Reset();
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(true);
        DoCastSpellIfCan(nullptr, SPELL_THADIUS_SPAWN, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Aggro(Unit* who) override
    {
        BossAI::Aggro(who);
        // Make Attackable
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(false);
    }

    void JustReachedHome() override
    {
        BossAI::JustReachedHome();
        DoCastSpellIfCan(nullptr, SPELL_THADIUS_SPAWN, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::Aggro(killer);
        if (m_instance)
        {
            // Force Despawn of Adds
            Creature* feugen  = m_instance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            Creature* stalagg = m_instance->GetSingleCreatureFromStorage(NPC_STALAGG);

            if (feugen)
                feugen->ForcedDespawn();
            if (stalagg)
                stalagg->ForcedDespawn();
        }
    }
};

/************
** boss_thaddiusAddsAI - Superclass for Feugen & Stalagg
************/

// The superclass and add specific structure will use Feugen NPC as the master NPC that will initialise and synchronise both adds.
// This design is hinted in Classic spells DBCs

enum ThaddiusAddActions
{
    THADDIUS_ADD_ACTIONS_MAX,
    THADDIUS_ADD_REVIVE,
    THADDIUS_ADD_SHOCK_OVERLOAD,
    THADDIUS_ADD_HOLD,
};

struct boss_thaddiusAddsAI : public BossAI
{
    boss_thaddiusAddsAI(Creature* creature) : BossAI(creature, THADDIUS_ADD_ACTIONS_MAX), m_instance(dynamic_cast<instance_naxxramas*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_THADDIUS);
        AddCustomAction(THADDIUS_ADD_REVIVE, true, [&]()
        {
            if (!GetOtherAdd())
                return;
            if (boss_thaddiusAddsAI* otherAI = dynamic_cast<boss_thaddiusAddsAI*>(GetOtherAdd()->AI()))
            {
                if (otherAI->IsCountingDead())
                {
                    otherAI->DisableTimer(THADDIUS_ADD_REVIVE);
                    AddCustomAction(THADDIUS_ADD_SHOCK_OVERLOAD, 14s, [&]()
                    {
                        DoCastSpellIfCan(m_creature, SPELL_TRIGGER_TESLAS, TRIGGERED_OLD_TRIGGERED);
                        DisableTimer(THADDIUS_ADD_SHOCK_OVERLOAD);
                    });
                    DisableTimer(THADDIUS_ADD_REVIVE);
                    return;
                }
                Revive();
                m_isFakingDeath = false;
                DisableTimer(THADDIUS_ADD_REVIVE);
            }
        });
    }

    instance_naxxramas* m_instance;

    bool m_isFakingDeath;
    bool m_areBothDead;

    void Reset() override
    {
        BossAI::Reset();
        m_isFakingDeath = false;
        m_areBothDead = false;

        // We might Reset while faking death, so undo this
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_IMMUNE_TO_PLAYER);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        SetDeathPrevention(true);
        SetCombatScriptStatus(false);
        SetCombatMovement(true);
    }

    // For Stalagg returns feugen, for Feugen returns stalagg
    Creature* GetOtherAdd() const
    {
        switch (m_creature->GetEntry())
        {
            case NPC_FEUGEN:  return m_instance->GetSingleCreatureFromStorage(NPC_STALAGG);
            case NPC_STALAGG: return m_instance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            default: return nullptr;
        }
    }

    void Aggro(Unit* who) override
    {
        BossAI::Aggro(who);
        if (Creature* otherAdd = GetOtherAdd())
        {
            if (!otherAdd->IsInCombat())
                otherAdd->AI()->AttackStart(who);
        }
    }

    void Revive()
    {
        SetCombatScriptStatus(false);
        DoResetThreat();
        PauseCombatMovement();
        Reset();
    }

    bool IsCountingDead() const
    {
        return m_isFakingDeath || m_creature->IsDead();
    }

    void PauseCombatMovement()
    {
        SetAIImmobilizedState(true);
        AddCustomAction(THADDIUS_ADD_HOLD, 1s + 500ms, [&]()
        {
            SetAIImmobilizedState(false);
            m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());
            DisableTimer(THADDIUS_ADD_HOLD);
        });
        ResetIfNotStarted(THADDIUS_ADD_HOLD, 1s + 500ms);
    }

    void JustPreventedDeath(Unit* attacker) override
    {
        if (!m_instance)
            return;
        m_isFakingDeath = true;

        m_creature->InterruptNonMeleeSpells(false);
        m_creature->SetHealth(1);
        m_creature->StopMoving();
        m_creature->ClearComboPointHolders();
        m_creature->RemoveAllAurasOnDeath();
        m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_IMMUNE_TO_PLAYER);
        m_creature->ClearAllReactives();
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
        SetCombatScriptStatus(true);

        JustDied(attacker);                                  // Texts
        ResetTimer(THADDIUS_ADD_REVIVE, 10s);
    }
};

/************
** boss_stalagg
************/

struct boss_stalaggAI : public boss_thaddiusAddsAI
{
    boss_stalaggAI(Creature* creature) : boss_thaddiusAddsAI(creature)
    {
        AddOnKillText(SAY_STAL_SLAY);
        AddOnAggroText(SAY_STAL_AGGRO);
        AddOnDeathText(SAY_STAL_DEATH);
    }

    void OnSpellCast(SpellEntry const* spellInfo, Unit* /*target*/) override
    {
        if (spellInfo->Id == SPELL_MAGNETIC_PULL_A)
        {
            if (Creature* feugen = GetOtherAdd())
                feugen->CastSpell(nullptr, SPELL_MAGNETIC_PULL_B, TRIGGERED_OLD_TRIGGERED); // meant to be cast as non triggered
        }
    }
};

/************
** boss_feugen
************/

struct boss_feugenAI : public boss_thaddiusAddsAI
{
    boss_feugenAI(Creature* creature) : boss_thaddiusAddsAI(creature)
    {
        AddOnKillText(SAY_FEUG_SLAY);
        AddOnAggroText(SAY_FEUG_AGGRO);
        AddOnDeathText(SAY_FEUG_DEATH);
    }
};

// 28159 - Shock
struct ShockThaddius : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            Unit* target = spell->GetUnitTarget();
            // Only do something to Thaddius, and on the first hit.
            if (target->GetEntry() != NPC_THADDIUS || !target->HasAura(SPELL_THADIUS_SPAWN))
                return;
            // remove Stun and then Cast
            target->RemoveAurasDueToSpell(SPELL_THADIUS_SPAWN);
            target->CastSpell(nullptr, SPELL_THADIUS_LIGHTNING_VISUAL, TRIGGERED_OLD_TRIGGERED);
            // Remove visual effect on Tesla GOs
            if (instance_naxxramas* instance = dynamic_cast<instance_naxxramas*>(target->GetInstanceData()))
            {
                if (GameObject* stalaggTesla = instance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_STALAGG))
                    stalaggTesla->SetGoState(GO_STATE_READY);
                if (GameObject* feugenTesla = instance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_FEUGEN))
                    feugenTesla->SetGoState(GO_STATE_READY);
            }
        }
    }
};

// 28136 - Thadius Lightning Visual
struct ThaddiusLightningVisual : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (effIdx == EFFECT_INDEX_0 && target->IsCreature())
        {
            target->AI()->SetCombatScriptStatus(false);
            static_cast<Creature*>(target)->SetInCombatWithZone(false);
        }
    }
};

/****************
** Polarity Shift
****************/

// 28089 - Polarity Shift
struct PolarityShift : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx ) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* unitTarget = spell->GetUnitTarget())
            {
                unitTarget->RemoveAurasDueToSpell(SPELL_POSITIVE_CHARGE);
                unitTarget->RemoveAurasDueToSpell(SPELL_NEGATIVE_CHARGE);

                uint64 scriptValue = spell->GetScriptValue();

                // 28059 : Positive Charge, 28084 : Negative Charge
                switch (scriptValue)
                {
                    case 0: // first target random
                        scriptValue = urand(0, 1) ? SPELL_POSITIVE_CHARGE : SPELL_NEGATIVE_CHARGE;
                        spell->SetScriptValue(scriptValue);
                        unitTarget->CastSpell(unitTarget, scriptValue, TRIGGERED_INSTANT_CAST);
                        break;
                    case SPELL_POSITIVE_CHARGE: // second target the other
                        spell->SetScriptValue(1);
                        unitTarget->CastSpell(unitTarget, SPELL_NEGATIVE_CHARGE, TRIGGERED_INSTANT_CAST);
                        break;
                    case SPELL_NEGATIVE_CHARGE:
                        spell->SetScriptValue(1);
                        unitTarget->CastSpell(unitTarget, SPELL_POSITIVE_CHARGE, TRIGGERED_INSTANT_CAST);
                        break;
                    default: // third and later random
                        unitTarget->CastSpell(unitTarget, urand(0, 1) ? SPELL_POSITIVE_CHARGE : SPELL_NEGATIVE_CHARGE, TRIGGERED_INSTANT_CAST);
                        break;
                }
            }
        }
    }
};

// 28062 - Positive Charge
// 28085 - Negative Charge
struct ThaddiusChargeDamage : public SpellScript
{
    bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            switch (spell->m_spellInfo->Id)
            {
                case SPELL_POSITIVE_CHARGE_DAMAGE:                // Positive Charge
                    if (target->HasAura(SPELL_POSITIVE_CHARGE))   // Only deal damage to targets without Positive Charge
                        return false;
                    break;
                case SPELL_NEGATIVE_CHARGE_DAMAGE:                // Negative Charge
                    if (target->HasAura(SPELL_NEGATIVE_CHARGE))   // Only deal damage to targets without Negative Charge
                        return false;
                    break;
                default:
                    break;
            }
        }
        return true;
    }
};

// 28059 - Positive Charge
// 28084 - Negative Charge
struct ThaddiusCharge : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (apply)
        {
            Unit* target = aura->GetTarget();
            if (!target)
                return;

            // On Polarity Shift, remove the previous damage buffs
            uint32 buffAuraIds[2] = { SPELL_POSITIVE_CHARGE_BUFF , SPELL_NEGATIVE_CHARGE_BUFF };
            for (auto buffAura: buffAuraIds)
                target->RemoveAurasDueToSpell(buffAura);
        }
    }

    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& /*data*/) const override
    {
        if (Unit* target = aura->GetTarget())
        {

            uint32 buffAuraId = aura->GetId() == SPELL_POSITIVE_CHARGE ? SPELL_POSITIVE_CHARGE_BUFF : SPELL_NEGATIVE_CHARGE_BUFF;
            float range = 13.f; // Static value from DBC files. As the value is the same for both spells we can hardcode it instead of accessing is through sSpellRadiusStore

            uint32 curCount = 0;
            PlayerList playerList;
            GetPlayerListWithEntryInWorld(playerList, target, range);
            for (Player* player : playerList)
                if (target != player && player->HasAura(aura->GetId()))
                    ++curCount;

            // Remove previous buffs in case we have less targets of the same charge near use than in previous tick
            target->RemoveAurasDueToSpell(buffAuraId);
                for (uint32 i = 0; i < curCount; i++)
                    target->CastSpell(target, buffAuraId, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

// 28338, 28339 - Magnetic Pull
struct MagneticPullThaddius : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* target = spell->GetUnitTarget();
        Unit* caster = spell->GetCaster();
        if (target && caster->GetVictim())
            target->CastSpell(caster->GetVictim(), 28337, TRIGGERED_OLD_TRIGGERED); // target cast actual Magnetic Pull on caster's victim
        // ToDo research if target should also get the threat of the caster for caster's victim.
        // This is the case in WotLK version but we have no proof of this in Classic/TBC
        // and it was common at these times to let players manage threat and tank transitions by themselves
    }
};

// 28096 - Stalagg Chain
// 28111 - Feugen Chain
struct FeugenStalaggChain : public AuraScript
{
    void OnPeriodicTrigger(Aura* /*aura*/, PeriodicTriggerData& /*data*/) const override
    {
        
    }
};

// 28098 - Stalagg Tesla Effect
// 28110 - Feugen Tesla Effect
struct FeugenStalaggTeslaEffect : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* unitTarget = spell->GetUnitTarget();
        Unit* caster = spell->GetCaster();
        if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->IsAlive())
        {
            if (!caster->hasUnitState(UNIT_STAT_ROOT))    // This state is found in sniffs and is probably caused by another aura like 23973
                caster->addUnitState(UNIT_STAT_ROOT);     // but as we are not sure (the aura does not show up in sniffs), we handle the state here

            // Cast chain (Stalagg Chain or Feugen Chain)
            uint32 chainSpellId = spell->m_spellInfo->Id == 28098 ? 28096 : 28111;
            // Only cast if not already present and in range
            if (!unitTarget->HasAura(chainSpellId) && caster->IsWithinDistInMap(unitTarget, 60.0f))
            {
                if (!caster->IsImmuneToPlayer())
                    caster->SetImmuneToPlayer(true);
                caster->CastSpell(unitTarget, chainSpellId, TRIGGERED_OLD_TRIGGERED);
            }
            // Not in range and fight in progress: remove aura and cast Shock onto players
            else if (!caster->IsWithinDistInMap(unitTarget, 60.0f) && caster)
            {
                unitTarget->RemoveAurasDueToSpell(chainSpellId);
                caster->SetImmuneToPlayer(false);

                if (Unit* target = caster->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    caster->CastSpell(target, 28099, TRIGGERED_NONE);
            }
            // else: in range and already have aura: do nothing
        }
    }
};

// 28359 - Trigger Teslas
struct TriggerTeslas : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* unitTarget = spell->GetUnitTarget();
        if (unitTarget)
        {
            DoBroadcastText(EMOTE_TESLA_OVERLOAD, unitTarget, unitTarget);
            unitTarget->RemoveAllAuras();
            unitTarget->CastSpell(nullptr, SPELL_SHOCK_OVERLOAD, TRIGGERED_NONE); // Shock
        }
    }
};

void AddSC_boss_thaddius()
{
    Script* newScript = new Script;
    newScript->Name = "boss_thaddius";
    newScript->GetAI = &GetNewAIInstance<boss_thaddiusAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_stalagg";
    newScript->GetAI = &GetNewAIInstance<boss_stalaggAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_feugen";
    newScript->GetAI = &GetNewAIInstance<boss_feugenAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<ShockThaddius>("spell_shock_thaddius");
    RegisterSpellScript<ThaddiusLightningVisual>("spell_thaddius_lightning_visual");
    RegisterSpellScript<PolarityShift>("spell_thaddius_polarity_shift");
    RegisterSpellScript<ThaddiusChargeDamage>("spell_thaddius_charge_damage");
    RegisterSpellScript<ThaddiusCharge>("spell_thaddius_charge_buff");
    RegisterSpellScript<MagneticPullThaddius>("spell_magnetic_pull_thaddius");
    RegisterSpellScript<FeugenStalaggChain>("spell_feugen_stalagg_chain");
    RegisterSpellScript<FeugenStalaggTeslaEffect>("spell_feugen_stalagg_tesla_effect");
    RegisterSpellScript<TriggerTeslas>("spell_trigger_teslas");
}
