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

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    // Stalagg
    SAY_STAL_AGGRO                  = -1533023,
    SAY_STAL_SLAY                   = -1533024,
    SAY_STAL_DEATH                  = -1533025,

    // Feugen
    SAY_FEUG_AGGRO                  = -1533026,
    SAY_FEUG_SLAY                   = -1533027,
    SAY_FEUG_DEATH                  = -1533028,

    // Thaddus
    SAY_AGGRO_1                     = -1533030,
    SAY_AGGRO_2                     = -1533031,
    SAY_AGGRO_3                     = -1533032,
    SAY_SLAY                        = -1533033,
    SAY_ELECT                       = -1533034,
    SAY_DEATH                       = -1533035,
    EMOTE_TESLA_OVERLOAD            = -1533150,

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
    THADDIUS_POLARITY_SHIFT,
    THADDIUS_CHAIN_LIGHTNING,
    THADDIUS_BALL_LIGHTNING,
    THADDIUS_BERSERK,
    THADDIUS_ACTION_MAX,
};

struct boss_thaddiusAI : public CombatAI
{
    boss_thaddiusAI(Creature* creature) : CombatAI(creature, THADDIUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(THADDIUS_POLARITY_SHIFT, true);
        AddCombatAction(THADDIUS_CHAIN_LIGHTNING, true);
        AddCombatAction(THADDIUS_BALL_LIGHTNING, true);
        AddCombatAction(THADDIUS_BERSERK, true);
    }

    ScriptedInstance* m_instance;

    uint32 GetSubsequentActionTimer(uint32 id)
    {
        switch (id)
        {
            case THADDIUS_CHAIN_LIGHTNING: return 15u * IN_MILLISECONDS;
            case THADDIUS_POLARITY_SHIFT: return 30u * IN_MILLISECONDS;
            case THADDIUS_BALL_LIGHTNING: return 3u * IN_MILLISECONDS;
            case THADDIUS_BERSERK: return 10u * MINUTE * IN_MILLISECONDS;
            default: return 0;
        }
    }

    void Reset() override
    {
        CombatAI::Reset();

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(true);
        DoCastSpellIfCan(m_creature, SPELL_THADIUS_SPAWN, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO_3, m_creature); break;
        }

        ResetCombatAction(THADDIUS_CHAIN_LIGHTNING, 8u * IN_MILLISECONDS);
        ResetCombatAction(THADDIUS_POLARITY_SHIFT, 15u * IN_MILLISECONDS);
        ResetCombatAction(THADDIUS_BERSERK, 6u * MINUTE * IN_MILLISECONDS);
        ResetCombatAction(THADDIUS_BALL_LIGHTNING, 3u * IN_MILLISECONDS);

        // Make Attackable
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(false);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_THADDIUS, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_THADIUS_SPAWN, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() != TYPEID_PLAYER)
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
        {
            m_instance->SetData(TYPE_THADDIUS, DONE);

            // Force Despawn of Adds
            Creature* feugen  = m_instance->GetSingleCreatureFromStorage(NPC_FEUGEN);
            Creature* stalagg = m_instance->GetSingleCreatureFromStorage(NPC_STALAGG);

            if (feugen)
                feugen->ForcedDespawn();
            if (stalagg)
                stalagg->ForcedDespawn();
        }
    }

    void ExecuteAction(uint32 action) override
    {
        if (!m_instance)
            return;

        switch (action)
        {
            case THADDIUS_CHAIN_LIGHTNING:
            {
                Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                if (target && DoCastSpellIfCan(target, SPELL_CHAIN_LIGHTNING) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case THADDIUS_POLARITY_SHIFT:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POLARITY_SHIFT, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                {
                    DoScriptText(SAY_ELECT, m_creature);
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                }
                return;
            }
            case THADDIUS_BALL_LIGHTNING:
            {
                Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE);
                if (!target)
                {
                    if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_BALL_LIGHTNING) == CAST_OK)
                        ResetCombatAction(action, GetSubsequentActionTimer(action));
                }
                return;
            }
            case THADDIUS_BERSERK:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BESERK) == CAST_OK)                  // allow combat movement?
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
        }
    }
};

bool EffectDummyNPC_spell_thaddius_encounter(Unit* /*caster*/, uint32 spellId, SpellEffectIndex effIndex, Creature* creatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    switch (spellId)
    {
        case SPELL_SHOCK_OVERLOAD:
            if (effIndex == EFFECT_INDEX_0)
            {
                // Only do something to Thaddius, and on the first hit.
                if (creatureTarget->GetEntry() != NPC_THADDIUS || !creatureTarget->HasAura(SPELL_THADIUS_SPAWN))
                    return true;
                // remove Stun and then Cast visual starting spell
                creatureTarget->RemoveAurasDueToSpell(SPELL_THADIUS_SPAWN);
                creatureTarget->CastSpell(creatureTarget, SPELL_THADIUS_LIGHTNING_VISUAL, TRIGGERED_NONE);
                // Remove visual effect on Tesla GOs
                if (instance_naxxramas* instance = (instance_naxxramas*)creatureTarget->GetInstanceData())
                {
                    if (GameObject* stalaggTesla = instance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_STALAGG))
                        stalaggTesla->SetGoState(GO_STATE_READY);
                    if (GameObject* feugenTesla = instance->GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_FEUGEN))
                        feugenTesla->SetGoState(GO_STATE_READY);
                }
            }
            return true;
        case SPELL_THADIUS_LIGHTNING_VISUAL:
            if (effIndex == EFFECT_INDEX_0 && creatureTarget->GetEntry() == NPC_THADDIUS)
                creatureTarget->SetInCombatWithZone(false);
            return true;
    }
    return false;
}

/************
** boss_thaddiusAddsAI - Superclass for Feugen & Stalagg
************/

// The superclass and add specific structure will use Feugen NPC as the master NPC that will initialise and synchronise both adds.
// This design is hinted in Classic spells DBCs

struct boss_thaddiusAddsAI : public ScriptedAI
{
    boss_thaddiusAddsAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    bool m_isFakingDeath;
    bool m_areBothDead;

    uint32 m_holdTimer;
    uint32 m_warStompTimer;
    uint32 m_reviveTimer;
    uint32 m_shockOverloadTimer;

    void Reset() override
    {
        m_isFakingDeath = false;
        m_areBothDead = false;

        m_reviveTimer = 5 * IN_MILLISECONDS;
        m_holdTimer = 2 * IN_MILLISECONDS;
        m_warStompTimer = urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        m_shockOverloadTimer = 0;

        // We might Reset while faking death, so undo this
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(false);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    Creature* GetOtherAdd(bool hasToBeAlive = false) const
    // For Stalagg returns feugen, for Feugen returns stalagg
    {
        Creature* otherAdd = nullptr;
        switch (m_creature->GetEntry())
        {
            case NPC_FEUGEN:  otherAdd = m_instance->GetSingleCreatureFromStorage(NPC_STALAGG); break;
            case NPC_STALAGG: otherAdd = m_instance->GetSingleCreatureFromStorage(NPC_FEUGEN); break;
            default:
                return nullptr;
        }

        // Check if the other add needs to be alive and return nullptr if dead of faking death
        if (hasToBeAlive && otherAdd)
        {
            if (boss_thaddiusAddsAI* otherAddAI = dynamic_cast<boss_thaddiusAddsAI*>(otherAdd->AI()))
            {
                if (otherAddAI->IsCountingDead())
                    return nullptr;
            }
        }

        return otherAdd;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (!m_instance)
            return;

        m_instance->SetData(TYPE_THADDIUS, IN_PROGRESS);

        if (Creature* otherAdd = GetOtherAdd())
        {
            if (!otherAdd->IsInCombat())
                otherAdd->SetInCombatWithZone();
        }
    }

    void JustReachedHome() override
    {
        if (!m_instance)
            return;

        m_instance->SetData(TYPE_THADDIUS, FAIL);
    }

    bool DoMagneticPullIfCan()
    {
        if (!m_isFakingDeath && GetOtherAdd())
        {
            if (DoCastSpellIfCan(GetOtherAdd(), (m_creature->GetEntry() == NPC_FEUGEN ? SPELL_MAGNETIC_PULL_A : SPELL_MAGNETIC_PULL_B), TRIGGERED_OLD_TRIGGERED))
            {
                DoStopAttack();
                DoResetThreat();
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE))
                {
                    AttackStart(target);
                    m_creature->SetInFront(target);
                }
                return true;
            }
        }
        return false;
    }

    bool IsCountingDead() const
    {
        return m_isFakingDeath || m_creature->IsDead();
    }


    virtual void UpdateAddAI(const uint32 /*diff*/) {}        // Used for Add-specific spells

    void UpdateAI(const uint32 diff) override
    {
        if (m_areBothDead)                                    // This is the case while fighting Thaddius
        {
            if (m_shockOverloadTimer)                         // Timer before triggering phase 2 with spell Shock Overload
            {
                if (m_shockOverloadTimer <= diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TRIGGER_TESLAS) == CAST_OK)
                        m_shockOverloadTimer = 0;
                }
                else
                    m_shockOverloadTimer -= diff;
            }
            return;
        }

        if (m_isFakingDeath)
        {
            if (m_reviveTimer < diff)
            {
                if (Creature* pOther = GetOtherAdd())
                {
                    if (boss_thaddiusAddsAI* otherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
                    {
                        if (!otherAI->IsCountingDead())     // Raid was to slow to kill the second add
                        {
                            DoResetThreat();
                            SetCombatMovement(false);
                            m_holdTimer = 1500;
                            Reset();
                        }
                        else
                        {
                            m_areBothDead = true;           // Now both adds are counting dead
                            otherAI->m_areBothDead = true;
                            // Set both Teslas to overload
                            m_shockOverloadTimer = 14 * IN_MILLISECONDS;
                        }
                    }
                }
            }
            else
                m_reviveTimer -= diff;
            return;
        }

        // If no target do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_holdTimer)                      // A short timer preventing combat movement after revive
        {
            if (m_holdTimer <= diff)
            {
                SetCombatMovement(true);
                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());
                m_holdTimer = 0;
            }
            else
                m_holdTimer -= diff;
        }

        if (m_warStompTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WARSTOMP) == CAST_OK)
                m_warStompTimer = urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
        }
        else
            m_warStompTimer -= diff;

        UpdateAddAI(diff);                    // For Add Specific Abilities

        DoMeleeAttackIfReady();
    }

    void DamageTaken(Unit* dealer, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (damage < m_creature->GetHealth())
            return;

        // Prevent glitch if in fake death
        if (m_isFakingDeath)
        {
            damage = std::min(damage, m_creature->GetHealth() - 1);
            return;
        }

        // prevent death
        damage = std::min(damage, m_creature->GetHealth() - 1);
        m_isFakingDeath = true;

        m_creature->InterruptNonMeleeSpells(false);
        m_creature->StopMoving();
        m_creature->ClearComboPointHolders();
        m_creature->RemoveAllAurasOnDeath();
        m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetImmuneToPlayer(true);
        m_creature->ClearAllReactives();
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

        JustDied(dealer);                                  // Texts
    }
};

/************
** boss_stalagg
************/

struct boss_stalaggAI : public boss_thaddiusAddsAI
{
    boss_stalaggAI(Creature* creature) : boss_thaddiusAddsAI(creature)
    {
        Reset();
    }
    uint32 m_powerSurgeTimer;

    void Reset() override
    {
        boss_thaddiusAddsAI::Reset();
        m_powerSurgeTimer = urand(10, 15) * IN_MILLISECONDS;
    }

    void Aggro(Unit* who) override
    {
        DoScriptText(SAY_STAL_AGGRO, m_creature);
        boss_thaddiusAddsAI::Aggro(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_STAL_DEATH, m_creature);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_STAL_SLAY, m_creature);
    }

    void UpdateAddAI(const uint32 diff) override
    {
        if (m_powerSurgeTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POWERSURGE) == CAST_OK)
                m_powerSurgeTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_powerSurgeTimer -= diff;
    }
};

UnitAI* GetAI_boss_stalagg(Creature* creature)
{
    return new boss_stalaggAI(creature);
}

/************
** boss_feugen
************/

struct boss_feugenAI : public boss_thaddiusAddsAI
{
    boss_feugenAI(Creature* creature) : boss_thaddiusAddsAI(creature)
    {
        Reset();
    }
    uint32 m_staticFieldTimer;
    uint32 m_magneticPullTimer;

    void Reset() override
    {
        boss_thaddiusAddsAI::Reset();
        m_staticFieldTimer = urand(10, 15) * IN_MILLISECONDS;
        m_magneticPullTimer = 20 * IN_MILLISECONDS;
    }

    void Aggro(Unit* who) override
    {
        DoScriptText(SAY_FEUG_AGGRO, m_creature);
        boss_thaddiusAddsAI::Aggro(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_FEUG_DEATH, m_creature);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_FEUG_SLAY, m_creature);
    }

    void UpdateAddAI(const uint32 diff) override
    {
        if (m_magneticPullTimer < diff)
        {
            if (!m_isFakingDeath && GetOtherAdd(true))      // No need to check if we have a victim that Stalagg can pull: this is already done in super class
            {
                if (DoMagneticPullIfCan())                  // This will return false if Stalagg has no victim we can pull
                {
                    if (boss_thaddiusAddsAI* stalaggAI = dynamic_cast<boss_thaddiusAddsAI*>(GetOtherAdd()->AI()))
                    {
                        stalaggAI->DoMagneticPullIfCan();
                        m_magneticPullTimer = 20 * IN_MILLISECONDS;
                    }
                }
            }
        }
        else
            m_magneticPullTimer -= diff;

        if (m_staticFieldTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STATIC_FIELD) == CAST_OK)
                m_staticFieldTimer = urand(10, 15) * IN_MILLISECONDS;
        }
        else
            m_staticFieldTimer -= diff;
    }
};

UnitAI* GetAI_boss_feugen(Creature* creature)
{
    return new boss_feugenAI(creature);
}

/****************
** Polarity Shift
****************/

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

void AddSC_boss_thaddius()
{
    Script* newScript = new Script;
    newScript->Name = "boss_thaddius";
    newScript->GetAI = &GetNewAIInstance<boss_thaddiusAI>;
    newScript->pEffectDummyNPC = &EffectDummyNPC_spell_thaddius_encounter;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_stalagg";
    newScript->GetAI = &GetAI_boss_stalagg;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_feugen";
    newScript->GetAI = &GetAI_boss_feugen;
    newScript->RegisterSelf();

    RegisterSpellScript<PolarityShift>("spell_thaddius_polarity_shift");
    RegisterSpellScript<ThaddiusChargeDamage>("spell_thaddius_charge_damage");
    RegisterSpellScript<ThaddiusCharge>("spell_thaddius_charge_buff");
}
