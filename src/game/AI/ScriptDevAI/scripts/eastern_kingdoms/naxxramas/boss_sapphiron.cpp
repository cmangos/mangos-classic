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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    EMOTE_BREATH                = -1533082,
    EMOTE_GENERIC_ENRAGED       = -1000003,

    // All phases spells
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

enum Phases
{
    PHASE_GROUND        = 1,
    PHASE_LIFT_OFF      = 2,
    PHASE_AIR_BOLTS     = 3,
    PHASE_AIR_BREATH    = 4,
    PHASE_LANDING       = 5,
};

struct boss_sapphironAI : public ScriptedAI
{
    boss_sapphironAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_cleaveTimer;
    uint32 m_tailSweepTimer;
    uint32 m_iceboltTimer;
    uint32 m_frostBreathTimer;
    uint32 m_lifeDrainTimer;
    uint32 m_blizzardTimer;
    uint32 m_flyTimer;
    uint32 m_berserkTimer;
    uint32 m_landTimer;

    uint32 m_iceboltCount;
    Phases m_phase;

    void Reset() override
    {
        m_cleaveTimer         = 5 * IN_MILLISECONDS;
        m_tailSweepTimer      = 12 * IN_MILLISECONDS;
        m_frostBreathTimer    = 7 * IN_MILLISECONDS;
        m_lifeDrainTimer      = 11 * IN_MILLISECONDS;
        m_blizzardTimer       = 15 * IN_MILLISECONDS;
        m_flyTimer            = 46 * IN_MILLISECONDS;
        m_iceboltTimer        = 7 * IN_MILLISECONDS;
        m_landTimer           = 0;
        m_berserkTimer        = 15 * MINUTE * IN_MILLISECONDS;
        m_phase                 = PHASE_GROUND;
        m_iceboltCount        = 0;

        SetCombatMovement(true);
        SetDeathPrevention(false);
        SetMeleeEnabled(true);
        m_creature->SetHover(false);
    }

    void Aggro(Unit* /*who*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_FROST_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_instance)
            m_instance->SetData(TYPE_SAPPHIRON, IN_PROGRESS);
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
            m_phase = PHASE_AIR_BOLTS;

            m_frostBreathTimer = 5 * IN_MILLISECONDS;
            m_iceboltTimer = 5 * IN_MILLISECONDS;
            m_iceboltCount = 0;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_phase)
        {
            case PHASE_GROUND:
                if (m_cleaveTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                        m_cleaveTimer = urand(5, 10) * IN_MILLISECONDS;
                }
                else
                    m_cleaveTimer -= diff;

                if (m_tailSweepTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TAIL_SWEEP) == CAST_OK)
                        m_tailSweepTimer = urand(7, 10) * IN_MILLISECONDS;
                }
                else
                    m_tailSweepTimer -= diff;

                if (m_lifeDrainTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_LIFE_DRAIN) == CAST_OK)
                        m_lifeDrainTimer = 24 * IN_MILLISECONDS;
                }
                else
                    m_lifeDrainTimer -= diff;

                if (m_blizzardTimer < diff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_BLIZZARD_INIT) == CAST_OK)
                        m_blizzardTimer = urand(10, 30) * IN_MILLISECONDS;
                }
                else
                    m_blizzardTimer -= diff;

                if (m_creature->GetHealthPercent() > 10.0f)
                {
                    if (m_flyTimer < diff)
                    {
                        m_phase = PHASE_LIFT_OFF;
                        m_iceboltTimer = 7 * IN_MILLISECONDS;

                        SetDeathPrevention(true);
                        m_creature->InterruptNonMeleeSpells(false);
                        SetCombatMovement(false);
                        SetMeleeEnabled(false);
                        m_creature->SetTarget(nullptr);
                        m_creature->GetMotionMaster()->MovePoint(1, aLiftOffPosition[0], aLiftOffPosition[1], aLiftOffPosition[2]);

                        return;
                    }
                    m_flyTimer -= diff;
                }

                // Only Phase in which we have melee attack!
                DoMeleeAttackIfReady();
                break;
            case PHASE_LIFT_OFF:
                break;
            case PHASE_AIR_BOLTS:
                if (m_iceboltCount == 5)
                {
                    if (m_frostBreathTimer < diff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH) == CAST_OK)
                        {
                            DoCastSpellIfCan(m_creature, SPELL_FROST_BREATH_DUMMY, CAST_TRIGGERED);
                            DoScriptText(EMOTE_BREATH, m_creature);
                            m_phase = PHASE_AIR_BREATH;
                            m_frostBreathTimer = 4 * IN_MILLISECONDS;
                            m_landTimer = 11 * IN_MILLISECONDS;
                        }
                    }
                    else
                        m_frostBreathTimer -= diff;
                }
                else
                {
                    if (m_iceboltTimer < diff)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_ICEBOLT_INIT) == CAST_OK)
                        {
                            ++m_iceboltCount;
                            m_iceboltTimer = 3 * IN_MILLISECONDS;
                        }
                    }
                    else
                        m_iceboltTimer -= diff;
                }

                break;
            case PHASE_AIR_BREATH:
                if (m_landTimer)
                {
                    if (m_landTimer <= diff)
                    {
                        // Begin Landing
                        m_creature->HandleEmote(EMOTE_ONESHOT_LAND);
                        m_phase = PHASE_LANDING;
                        m_landTimer = 2 * IN_MILLISECONDS;
                    }
                    else
                        m_landTimer -= diff;
                }

                break;
            case PHASE_LANDING:
                if (m_landTimer < diff)
                {
                    m_phase = PHASE_GROUND;
                    SetDeathPrevention(false);
                    SetCombatMovement(true);
                    SetMeleeEnabled(true);
                    m_creature->RemoveAurasDueToSpell(SPELL_DRAGON_HOVER);
                    m_creature->SetHover(false);
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());

                    m_flyTimer = 67 * IN_MILLISECONDS;
                    m_landTimer = 0;
                }
                else
                    m_landTimer -= diff;

                break;
        }

        // Enrage can happen in any phase
        if (m_berserkTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BESERK) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_ENRAGED, m_creature);
                m_berserkTimer = 300 * IN_MILLISECONDS;
            }
        }
        else
            m_berserkTimer -= diff;
    }
};

UnitAI* GetAI_boss_sapphiron(Creature* creature)
{
    return new boss_sapphironAI(creature);
}

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
            data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_DESPAWN_BUFFET_EFFECT); // Summon Ice Block
    }
};

void AddSC_boss_sapphiron()
{
    Script* newScript = new Script;
    newScript->Name = "boss_sapphiron";
    newScript->GetAI = &GetAI_boss_sapphiron;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "go_sapphiron_birth";
    newScript->pGOUse = &GOUse_go_sapphiron_birth;
    newScript->RegisterSelf();

    RegisterAuraScript<PeriodicIceBolt>("spell_sapphiron_icebolt_aura");
    RegisterSpellScript<IceBolt>("spell_sapphiron_icebolt");
    RegisterSpellScript<SummonBlizzard>("spell_sapphiron_blizzard");
    RegisterSpellScript<DespawnIceBlock>("spell_sapphiron_iceblock");
    RegisterAuraScript<DespawnBuffet>("spell_sapphiron_despawn_buffet");
}
