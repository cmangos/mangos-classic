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
SDName: Boss_Sapphiron
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    EMOTE_BREATH                = -1533082,
    EMOTE_GENERIC_ENRAGED       = -1000003,

    // All phases spells
    SPELL_DOUBLE_ATTACK         = 19818,
    SPELL_FROST_AURA            = 28529,            // Periodically triggers 28531
    SPELL_BESERK                = 26662,

    // Ground phase spells
    SPELL_CLEAVE                = 19983,
    SPELL_TAIL_SWEEP            = 15847,
    SPELL_LIFE_DRAIN            = 28542,
    SPELL_SUMMON_BLIZZARD_INIT  = 28560,
    SPELL_SUMMON_BLIZZARD       = 28561,

    // Air phase spells
    SPELL_DRAGON_HOVER          = 18430,
    SPELL_ICEBOLT_INIT          = 28526,            // Triggers spell 28522 (Icebolt)
    SPELL_ICEBOLT               = 28522,
    SPELL_ICEBOLT_IMMUNITY      = 31800,
    SPELL_ICEBLOCK_SUMMON       = 28535,
    SPELL_FROST_BREATH_DUMMY    = 30101,
    SPELL_FROST_BREATH          = 28524,            // Triggers spells 29318 (Frost Breath) and 30132 (Despawn Ice Block)
    SPELL_DESPAWN_ICEBLOCK_GO   = 28523,
    SPELL_SUMMON_WING_BUFFET    = 29329,
    SPELL_DESPAWN_WING_BUFFET   = 29330,            // Triggers spell 29336 (Despawn Buffet)
    SPELL_DESPAWN_BUFFET_EFFECT = 29336,

    NPC_BLIZZARD                = 16474,
};

static const float aLiftOffPosition[3] = {3522.386f, -5236.784f, 137.709f};

enum SapphironPhases
{
    PHASE_GROUND        = 1,
    PHASE_LIFT_OFF      = 2,
};

enum SapphironActions
{
    SAPPHIRON_CLEAVE,
    SAPPHIRON_TAIL_SWEEP,
    SAPPHIRON_ICEBOLT,
    SAPPHIRON_FROST_BREATH,
    SAPPHIRON_LIFE_DRAIN,
    SAPPHIRON_BLIZZARD,
    SAPPHIRON_AIR_PHASE,
    SAPPHIRON_LANDING_PHASE,
    SAPPHIRON_GROUND_PHASE,
    SAPPHIRON_BERSERK,
    SAPPHIRON_ACTION_MAX,
};

static const uint32 groundPhaseActions[] = {SAPPHIRON_CLEAVE, SAPPHIRON_TAIL_SWEEP, SAPPHIRON_BLIZZARD, SAPPHIRON_LIFE_DRAIN};

struct boss_sapphironAI : public CombatAI
{
    boss_sapphironAI(Creature* creature) : CombatAI(creature, SAPPHIRON_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(SAPPHIRON_CLEAVE, 5u * IN_MILLISECONDS);
        AddCombatAction(SAPPHIRON_TAIL_SWEEP, 12u * IN_MILLISECONDS);
        AddCombatAction(SAPPHIRON_LIFE_DRAIN, 11u * IN_MILLISECONDS);
        AddCombatAction(SAPPHIRON_BLIZZARD, 15u * IN_MILLISECONDS);
        AddCombatAction(SAPPHIRON_FROST_BREATH, true);
        AddCombatAction(SAPPHIRON_ICEBOLT, true);
        AddCombatAction(SAPPHIRON_BERSERK, 15u * MINUTE * IN_MILLISECONDS);
        AddCustomAction(SAPPHIRON_AIR_PHASE, true, [&]() { HandleAirPhase(); }, TIMER_COMBAT_COMBAT);
        AddCustomAction(SAPPHIRON_LANDING_PHASE, true, [&]() { HandleLandingPhase(); }, TIMER_COMBAT_COMBAT);
        AddCustomAction(SAPPHIRON_GROUND_PHASE, true, [&]() { HandleGroundPhase(); }, TIMER_COMBAT_COMBAT);
    }

    ScriptedInstance* m_instance;

    uint32 m_iceboltCount;
    SapphironPhases m_phase;

    void Reset() override
    {
        CombatAI:: Reset();

        m_iceboltCount = 0;
        m_phase = PHASE_GROUND;

        SetCombatMovement(true);
        SetDeathPrevention(false);
        SetMeleeEnabled(true);
        m_creature->SetHover(false);

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    uint32 GetSubsequentActionTimer(uint32 action)
    {
        switch (action)
        {
            case SAPPHIRON_CLEAVE: return urand(5, 10) * IN_MILLISECONDS;
            case SAPPHIRON_TAIL_SWEEP: return urand(7, 10) * IN_MILLISECONDS;
            case SAPPHIRON_LIFE_DRAIN: return 24u * IN_MILLISECONDS;
            case SAPPHIRON_BLIZZARD: return urand(10, 30) * IN_MILLISECONDS;
            case SAPPHIRON_ICEBOLT: return 3u * IN_MILLISECONDS;
            case SAPPHIRON_BERSERK: return 300u * IN_MILLISECONDS;
            case SAPPHIRON_AIR_PHASE: return 46u * IN_MILLISECONDS;
            default: return 0;
        }
    }

    void Aggro(Unit* /*who*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FROST_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_instance)
            m_instance->SetData(TYPE_SAPPHIRON, IN_PROGRESS);

        ResetTimer(SAPPHIRON_AIR_PHASE, GetSubsequentActionTimer(SAPPHIRON_AIR_PHASE));
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SAPPHIRON, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SAPPHIRON, FAIL);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_BLIZZARD)
        {
            if (Unit* pEnemy = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                summoned->AI()->AttackStart(pEnemy);
        }
    }

    void MovementInform(uint32 type, uint32 /*pointId*/) override
    {
        if (type == POINT_MOTION_TYPE && m_phase == PHASE_LIFT_OFF)
        {
            // Summon the Wing Buffet NPC and cast the triggered aura to despawn it
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_WING_BUFFET) == CAST_OK)
                DoCastSpellIfCan(m_creature, SPELL_DESPAWN_WING_BUFFET);

            // Actual take off
            m_creature->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
            m_creature->SetHover(true);
            m_creature->CastSpell(nullptr, SPELL_DRAGON_HOVER, TRIGGERED_OLD_TRIGGERED);
            ResetCombatAction(SAPPHIRON_ICEBOLT, 8u * IN_MILLISECONDS);
        }
    }

    void HandleAirPhase()
    {
        if (m_creature->GetHealthPercent() > 10.f)
        {
            m_phase = PHASE_LIFT_OFF;
            for (uint32 action : groundPhaseActions)
                DisableCombatAction(action);
            SetDeathPrevention(true);
            m_creature->InterruptNonMeleeSpells(false);
            SetCombatMovement(false);
            SetMeleeEnabled(false);
            m_creature->SetTarget(nullptr);
            m_creature->GetMotionMaster()->MovePoint(1, aLiftOffPosition[0], aLiftOffPosition[1], aLiftOffPosition[2]);
        }
        else
            DisableTimer(SAPPHIRON_AIR_PHASE);
    }

    void HandleLandingPhase()
    {
        m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
        ResetTimer(SAPPHIRON_GROUND_PHASE, 2u * IN_MILLISECONDS);
    }

    void HandleGroundPhase()
    {
        m_phase = PHASE_GROUND;
        for (uint32 action : groundPhaseActions)
            ResetCombatAction(action, GetSubsequentActionTimer(action));
        SetDeathPrevention(false);
        SetCombatMovement(true);
        SetMeleeEnabled(true);
        m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
        m_creature->SetHover(false);
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());

        ResetTimer(SAPPHIRON_AIR_PHASE, GetSubsequentActionTimer(SAPPHIRON_AIR_PHASE));
    }

    void ExecuteAction(uint32 action) override
    {
        switch(action)
        {
            case SAPPHIRON_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case SAPPHIRON_TAIL_SWEEP:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_TAIL_SWEEP) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case SAPPHIRON_LIFE_DRAIN:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_LIFE_DRAIN) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case SAPPHIRON_BLIZZARD:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_BLIZZARD_INIT) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case SAPPHIRON_ICEBOLT:
            {
                if (m_iceboltCount < 5)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_ICEBOLT_INIT) == CAST_OK)
                    {
                        ++m_iceboltCount;
                        ResetCombatAction(action, GetSubsequentActionTimer(action));
                    }
                }
                else
                {
                    m_iceboltCount = 0;
                    DisableCombatAction(action);
                    ResetCombatAction(SAPPHIRON_FROST_BREATH, 100u);    // Five Icebolt were cast, switch to Frost Breath (Ice Bomb) after that
                }
                return;
            }
            case SAPPHIRON_FROST_BREATH:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH) == CAST_OK)
                {
                    DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH_DUMMY, CAST_TRIGGERED);
                    DoScriptText(EMOTE_BREATH, m_creature);
                    DisableCombatAction(action);
                    ResetTimer(SAPPHIRON_LANDING_PHASE, 10u * IN_MILLISECONDS);
                }
                return;
            }
            case SAPPHIRON_BERSERK:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BESERK) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                    DisableCombatAction(action);
                }
                return;
            }
        }
    }
};

bool GOUse_go_sapphiron_birth(Player* /*player*/, GameObject* go)
{
    ScriptedInstance* instance = (ScriptedInstance*)go->GetInstanceData();

    if (!instance)
        return true;

    if (instance->GetData(TYPE_SAPPHIRON) != NOT_STARTED)
        return true;

    // If already summoned return (safety check)
    if (instance->GetSingleCreatureFromStorage(NPC_SAPPHIRON, true))
        return true;

    // Set data to special and allow the Go animation to proceed
    instance->SetData(TYPE_SAPPHIRON, SPECIAL);
    return false;
}

struct IceBolt : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* caster = spell->GetCaster())
        {
            if (Unit* target = ((Creature*)caster)->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_ICEBOLT, SELECT_FLAG_PLAYER | SELECT_FLAG_NOT_AURA))
                caster->CastSpell(target, SPELL_ICEBOLT, TRIGGERED_NONE); // Icebolt
        }
    }
};

struct PeriodicIceBolt : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        if (Unit* target =  aura->GetTarget())
        {
            if (target->IsAlive() && !target->HasAura(SPELL_ICEBOLT_IMMUNITY))
            {
                target->CastSpell(target, SPELL_ICEBOLT_IMMUNITY, TRIGGERED_OLD_TRIGGERED);     // Icebolt which causes immunity to frost dmg
                data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_ICEBLOCK_SUMMON); // Summon Ice Block
            }
        }
    }
};

struct SummonBlizzard : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
            unitTarget->CastSpell(unitTarget, SPELL_SUMMON_BLIZZARD, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetCaster()->GetObjectGuid());
    }
};

struct DespawnIceBlock : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                unitTarget->RemoveAurasDueToSpell(SPELL_ICEBOLT_IMMUNITY);                          // Icebolt immunity spell
                unitTarget->RemoveAurasDueToSpell(SPELL_ICEBOLT);                                   // Icebolt stun/damage spell
                unitTarget->CastSpell(nullptr, SPELL_DESPAWN_ICEBLOCK_GO, TRIGGERED_OLD_TRIGGERED); // Despawn Ice Block (targets Ice Block GOs)
            }
        }
    }
};

struct DespawnBuffet : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        if (Unit* target =  aura->GetTarget())
            data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_DESPAWN_BUFFET_EFFECT); // Despawn Ice Block
    }
};

void AddSC_boss_sapphiron()
{
    Script* newScript = new Script;
    newScript->Name = "boss_sapphiron";
    newScript->GetAI = &GetNewAIInstance<boss_sapphironAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "go_sapphiron_birth";
    newScript->pGOUse = &GOUse_go_sapphiron_birth;
    newScript->RegisterSelf();

    RegisterSpellScript<PeriodicIceBolt>("spell_sapphiron_icebolt_aura");
    RegisterSpellScript<IceBolt>("spell_sapphiron_icebolt");
    RegisterSpellScript<SummonBlizzard>("spell_sapphiron_blizzard");
    RegisterSpellScript<DespawnIceBlock>("spell_sapphiron_iceblock");
    RegisterSpellScript<DespawnBuffet>("spell_sapphiron_despawn_buffet");
}