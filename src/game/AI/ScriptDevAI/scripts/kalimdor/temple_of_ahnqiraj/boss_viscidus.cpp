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
SDName: Boss_Viscidus
SD%Complete: 95
SDComment: Viscidus globs miss their speed update while they move to the center.
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Scripts/SpellScript.h"

enum
{
    // emotes
    EMOTE_SLOW                  = -1531041,
    EMOTE_FREEZE                = -1531042,
    EMOTE_FROZEN                = -1531043,
    EMOTE_CRACK                 = -1531044,
    EMOTE_SHATTER               = -1531045,
    EMOTE_EXPLODE               = -1531046,

    // Timer spells
    SPELL_POISON_SHOCK          = 25993,
    SPELL_POISONBOLT_VOLLEY     = 25991,
    SPELL_TOXIN                 = 26575,                    // Triggers toxin cloud - 25989

    // Debuffs gained by the boss on frost damage
    SPELL_VISCIDUS_SLOWED       = 26034,
    SPELL_VISCIDUS_SLOWED_MORE  = 26036,
    SPELL_VISCIDUS_FREEZE       = 25937,

    // Proc spells handling the various stages based on hits
    SPELL_MEMBRANE_VISCIDUS     = 25994,                    // Damage reduction spell
    SPELL_VISCIDUS_WEAKNESS     = 25926,                    // Aura which procs at damage - triggers the slow spells

    // When frost damage exceeds a certain limit, then boss explodes
    SPELL_VISCIDUS_EXPLODE      = 25938,
    SPELL_VISCIDUS_SUICIDE_TRIGGER = 26003,                 // Cast when Viscidus shatters and is below 10% HP - triggers 26002
    SPELL_VISCIDUS_SUICIDE      = 26002,                    // Actual suicide spell
    SPELL_HATE_TO_ZERO          = 20538,

    // Spell for globs
    SPELL_REJOIN_VISCIDUS       = 25896,
    SPELL_SUMMON_GLOBS          = 25885,                    // summons npc 15667 using spells from 25865 to 25884; All spells have target coords
    SPELL_VISCIDUS_SHRINKS      = 25893,
    SPELL_VISCIDUS_SHRINKS_HP   = 27934,
    SPELL_DESPAWN_GLOBS         = 26608,
    SPELL_VISCIDUS_GROWS        = 25897,

//    SPELL_VISCIDUS_TELEPORT     = 25904,                    // Unknown usage
    SPELL_SUMMONT_TRIGGER       = 26564,                    // summons 15992

    // Spell for Toxic Slime (green poisonous clouds)
    SPELL_SUMMON_TOXIC_SLIME    = 26584,
    SPELL_SUMMON_TOXIC_SLIME_2  = 26577,
    SPELL_DESPAWN_TOXIC_SLIMES  = 26585,

    // Make Viscidus invisible and passive
    SPELL_STUN_SELF             = 25900,
    SPELL_INVIS_SELF            = 25905,
    SPELL_INVIS_STALKER         = 27933,

    NPC_GLOB_OF_VISCIDUS        = 15667,

    MAX_VISCIDUS_GLOBS          = 20,                       // there are 20 summoned globs; each glob = 5% hp

    // hitcounts
    HITCOUNT_SLOW               = 100,
    HITCOUNT_SLOW_MORE          = 150,
    HITCOUNT_FREEZE             = 200,
    HITCOUNT_CRACK              = 50,
    HITCOUNT_SHATTER            = 100,
    HITCOUNT_EXPLODE            = 150,

    // phases
    PHASE_NORMAL                = 1,
    PHASE_SLOWED                = 2,
    PHASE_SLOWED_MORE           = 3,
    PHASE_FROZEN                = 4,
    PHASE_EXPLODED              = 5,
};

static const uint32 auiGlobSummonSpells[MAX_VISCIDUS_GLOBS] = { 25865, 25866, 25867, 25868, 25869, 25870, 25871, 25872, 25873, 25874, 25875, 25876, 25877, 25878, 25879, 25880, 25881, 25882, 25883, 25884 };

struct Location
{
    float m_fX, m_fY, m_fZ;
};

static const Location resetPoint = { -7992.0f, 1041.0f, -23.84f };

enum ViscidusActions
{
    VISCIDUS_TOXIN,
    VISCIDUS_POISON_SHOCK,
    VISCIDUS_POISON_BOLT_VOLLEY,
    VISCIDUS_ACTION_MAX,
    VISCIDUS_EXPLODE,
};

struct boss_viscidusAI : public CombatAI
{
    boss_viscidusAI(Creature* creature) : CombatAI(creature, VISCIDUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(VISCIDUS_TOXIN, 30000u);
        AddCombatAction(VISCIDUS_POISON_SHOCK, 7000, 12000);
        AddCombatAction(VISCIDUS_POISON_BOLT_VOLLEY, 10000, 15000);
        AddCustomAction(VISCIDUS_EXPLODE, true, [&]() { HandleExplode(); });
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit* unit, float x, float y, float z) -> bool
        {
            return m_creature->GetDistance(resetPoint.m_fX, resetPoint.m_fY, resetPoint.m_fZ, DIST_CALC_COMBAT_REACH) < 10.0f;
        });
    }

    ScriptedInstance* m_instance;

    uint8 m_phase;

    uint32 m_hitCount;

    GuidList m_lGlobesGuidList;

    void Reset() override
    {
        CombatAI::Reset();
        m_phase    = PHASE_NORMAL;
        m_hitCount = 0;

        SetDeathPrevention(true);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VISCIDUS, IN_PROGRESS);

        DoCastSpellIfCan(nullptr, SPELL_MEMBRANE_VISCIDUS, CAST_TRIGGERED);
        DoCastSpellIfCan(nullptr, SPELL_VISCIDUS_WEAKNESS, CAST_TRIGGERED);
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VISCIDUS, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_GLOBS, CAST_TRIGGERED);
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_TOXIC_SLIMES, CAST_TRIGGERED);

        CombatAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VISCIDUS, DONE);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_GLOB_OF_VISCIDUS)
        {
            summoned->AI()->SetReactState(REACT_PASSIVE);
            float x, y, z;
            m_creature->GetRespawnCoord(x, y, z);
            summoned->GetMotionMaster()->MovePoint(1, x, y, z);
            m_lGlobesGuidList.push_back(summoned->GetObjectGuid());
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_GLOB_OF_VISCIDUS)
        {
            // shrink - modify scale
            DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SHRINKS, CAST_TRIGGERED);

            if (DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SHRINKS_HP, CAST_TRIGGERED) == CAST_OK)
                m_creature->SetHealth(m_creature->GetHealth() - (m_creature->GetMaxHealth() * 0.05f));

            m_lGlobesGuidList.remove(summoned->GetObjectGuid());

            if (m_lGlobesGuidList.empty())
                SetPhase(PHASE_NORMAL);
        }
    }

    void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 pointId) override
    {
        if (summoned->GetEntry() != NPC_GLOB_OF_VISCIDUS || motionType != POINT_MOTION_TYPE || !pointId)
            return;

        DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_GROWS, CAST_TRIGGERED);

        m_lGlobesGuidList.remove(summoned->GetObjectGuid());
        summoned->CastSpell(m_creature, SPELL_REJOIN_VISCIDUS, TRIGGERED_OLD_TRIGGERED);
        summoned->ForcedDespawn(1000);

        if (m_lGlobesGuidList.empty())
            SetPhase(PHASE_NORMAL);
    }

    void SetPhase(uint8 phase)
    {
        // Clean-up any potential phase aura before setting new phase
        m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED);
        m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED_MORE);
        m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE);

        switch (phase)
        {
            case PHASE_NORMAL:
                m_creature->RemoveAurasDueToSpell(SPELL_INVIS_STALKER);
                m_creature->RemoveAurasDueToSpell(SPELL_STUN_SELF);
                m_creature->RemoveAurasDueToSpell(SPELL_INVIS_SELF);
                DoCastSpellIfCan(nullptr, SPELL_MEMBRANE_VISCIDUS, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                DoCastSpellIfCan(nullptr, SPELL_VISCIDUS_WEAKNESS, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                DoCastSpellIfCan(m_creature, SPELL_HATE_TO_ZERO);
                break;
            case PHASE_SLOWED:
            {
                DoScriptText(EMOTE_SLOW, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SLOWED, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                break;
            }
            case PHASE_SLOWED_MORE:
            {
                DoScriptText(EMOTE_FREEZE, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SLOWED_MORE, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                break;
            }
            case PHASE_FROZEN:
            {
                // Reset hit count for frozen phase and shattering
                m_hitCount = 0;
                DoScriptText(EMOTE_FROZEN, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_FREEZE, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
                break;
            }
            case PHASE_EXPLODED:
            {
                m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE);
                // In case there is less than 10% HP left, Viscidus triggers suicide
                if (m_creature->GetHealthPercent() <= 10.0f)
                    DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SUICIDE_TRIGGER, CAST_TRIGGERED);
                // Else, he explodes in small globs that must be destroyed
                else if (DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_EXPLODE, CAST_TRIGGERED|CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                {
                    DoScriptText(EMOTE_EXPLODE, m_creature);
                    ResetTimer(VISCIDUS_EXPLODE, 2 * IN_MILLISECONDS);
                }
                break;
            }
            // Unexpected phase requested : return before setting
            default:
                return;
        }
        m_phase = phase;
    }

    void IncreaseHitCount()
    {
        ++m_hitCount;

        switch (m_phase)
        {
            case PHASE_NORMAL:
            {
                if (m_hitCount == HITCOUNT_SLOW)
                    SetPhase(PHASE_SLOWED);
                break;
            }
            case PHASE_SLOWED:
            {
                if (m_hitCount == HITCOUNT_SLOW_MORE)
                    SetPhase(PHASE_SLOWED_MORE);
                break;
            }
            case PHASE_SLOWED_MORE:
            {
                if (m_hitCount == HITCOUNT_FREEZE)
                    SetPhase(PHASE_FROZEN);
                break;
            }
            case PHASE_FROZEN:
            {
                if (m_hitCount == HITCOUNT_CRACK)
                    DoScriptText(EMOTE_CRACK, m_creature);
                else if (m_hitCount == HITCOUNT_SHATTER)
                    DoScriptText(EMOTE_SHATTER, m_creature);
                else if (m_hitCount == HITCOUNT_EXPLODE)
                    SetPhase(PHASE_EXPLODED);
                break;
            }
            default:
                return;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            if (m_phase == PHASE_EXPLODED)
                return;

            // reset phase if not already exploded
            m_phase = PHASE_NORMAL;
            m_hitCount = 0;
        }
        else if (eventType == AI_EVENT_CUSTOM_B)
        {
            if (m_phase >= PHASE_NORMAL && m_phase < PHASE_FROZEN)
                IncreaseHitCount();
        }
        else if (eventType == AI_EVENT_CUSTOM_C && m_phase == PHASE_FROZEN)
            IncreaseHitCount();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case VISCIDUS_TOXIN:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SUMMON_TOXIC_SLIME) == CAST_OK)
                        ResetCombatAction(action, 30000);
                break;
            }
            case VISCIDUS_POISON_SHOCK:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_POISON_SHOCK) == CAST_OK)
                    ResetCombatAction(action, urand(7000, 12000));
                break;
            }
            case VISCIDUS_POISON_BOLT_VOLLEY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_POISONBOLT_VOLLEY) == CAST_OK)
                    ResetCombatAction(action, urand(10000, 15000));
                break;
            }
            default:
                break;
        }
    }

    void HandleExplode()
    {
        // Reset hit count for normal and freezing phase
        m_hitCount = 0;
        m_creature->RemoveAurasDueToSpell(SPELL_MEMBRANE_VISCIDUS);
        m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_WEAKNESS);
        m_lGlobesGuidList.clear();

        DoCastSpellIfCan(m_creature, SPELL_SUMMON_GLOBS, TRIGGERED_IGNORE_GCD);
    }
};

struct ViscidusFreeze : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (aura->GetEffIndex() == EFFECT_INDEX_1 && !apply)
            if (Unit* target = aura->GetTarget())
                target->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, target, target);
    }

    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const override
    {
        if (Unit* target = aura->GetTarget())
        {
            if (procData.attType == BASE_ATTACK || procData.attType == OFF_ATTACK)
                target->AI()->SendAIEvent(AI_EVENT_CUSTOM_C, target, target);
        }
        return SPELL_AURA_PROC_OK;
    }
};

struct SummonToxicSlime : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (!spell->GetUnitTarget())
            return;
        spell->GetUnitTarget()->CastSpell(nullptr, SPELL_SUMMON_TOXIC_SLIME_2, TRIGGERED_OLD_TRIGGERED);
    }
};

struct ViscidusDespawnAdds : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (!spell->GetUnitTarget())
            return;

        if (spell->GetUnitTarget()->GetTypeId() != TYPEID_UNIT)
            return;

        ((Creature*)spell->GetUnitTarget())->ForcedDespawn();
    }
};

struct ViscidusSuicideTrigger : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (!spell->GetUnitTarget())
            return;

        if (spell->GetUnitTarget()->GetTypeId() != TYPEID_UNIT)
            return;

        spell->GetUnitTarget()->CastSpell(nullptr, SPELL_VISCIDUS_SUICIDE, TRIGGERED_OLD_TRIGGERED);
    }
};

struct ViscidusSummonGlobs : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (!spell->GetUnitTarget())
            return;

        uint8 globeCount = floor(spell->GetUnitTarget()->GetHealthPercent() / 5.0f);

        for (uint8 i = 0; i <= globeCount; ++i)
            spell->GetUnitTarget()->CastSpell(spell->GetUnitTarget(), auiGlobSummonSpells[i], TRIGGERED_IGNORE_GCD);

        // Make invisible and stun self
        spell->GetUnitTarget()->CastSpell(nullptr, SPELL_INVIS_SELF, TRIGGERED_OLD_TRIGGERED);
        spell->GetUnitTarget()->CastSpell(nullptr, SPELL_INVIS_STALKER, TRIGGERED_OLD_TRIGGERED);
        spell->GetUnitTarget()->CastSpell(nullptr, SPELL_STUN_SELF, TRIGGERED_OLD_TRIGGERED);

    }
};

struct ViscidusFrostWeakness : public AuraScript
{
    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const override
    {
        if (Unit* target = aura->GetTarget())
        {
            if (procData.procSpell)
            {
                if (GetSchoolMask(procData.procSpell->School) == SPELL_SCHOOL_MASK_FROST)
                    target->AI()->SendAIEvent(AI_EVENT_CUSTOM_B, target, target);
            }
        }
        return SPELL_AURA_PROC_OK;
    }
};

void AddSC_boss_viscidus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_viscidus";
    pNewScript->GetAI = &GetNewAIInstance<boss_viscidusAI>;
    pNewScript->RegisterSelf();

    RegisterAuraScript<ViscidusFreeze>("spell_viscidus_freeze");
    RegisterSpellScript<SummonToxicSlime>("spell_summon_toxic_slime");
    RegisterSpellScript<ViscidusDespawnAdds>("spell_viscidus_despawn_adds");
    RegisterSpellScript<ViscidusSuicideTrigger>("spell_viscidus_suicide");
    RegisterSpellScript<ViscidusSummonGlobs>("spell_viscidus_summon_globs");
    RegisterAuraScript<ViscidusFrostWeakness>("spell_viscidus_frost_weakness");
}
