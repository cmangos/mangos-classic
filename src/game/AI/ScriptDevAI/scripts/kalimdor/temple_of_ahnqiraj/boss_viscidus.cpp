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
SD%Complete: 90
SDComment: ToDo: Use aura proc to handle freeze event instead of direct function
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
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

    // When frost damage exceeds a certain limit, then boss explodes
    SPELL_REJOIN_VISCIDUS       = 25896,
    SPELL_VISCIDUS_EXPLODE      = 25938,
    SPELL_VISCIDUS_SUICIDE_TRIGGER = 26003,                    // cast when boss explodes and is below 5% Hp - should trigger 26002
    SPELL_VISCIDUS_SUICIDE      = 26002,
    SPELL_DESPAWN_GLOBS         = 26608,

    SPELL_MEMBRANE_VISCIDUS     = 25994,                    // damage reduction spell
    SPELL_VISCIDUS_WEAKNESS     = 25926,                    // aura which procs at damage - should trigger the slow spells
    SPELL_VISCIDUS_SHRINKS      = 25893,
    SPELL_VISCIDUS_SHRINKS_HP   = 27934,
    SPELL_VISCIDUS_GROWS        = 25897,
    SPELL_SUMMON_GLOBS          = 25885,                    // summons npc 15667 using spells from 25865 to 25884; All spells have target coords
    SPELL_VISCIDUS_TELEPORT     = 25904,                    // teleport to room center
    SPELL_SUMMONT_TRIGGER       = 26564,                    // summons 15992

    SPELL_SUMMON_TOXIC_SLIME    = 26584,                    // unk purpose
    SPELL_SUMMON_TOXIC_SLIME_2  = 26577,
    SPELL_INVIS_SELF            = 25905,                    // unconfirmed spell id but fits the purpose

    NPC_GLOB_OF_VISCIDUS        = 15667,
    // NPC_VISCIDUS_TRIGGER        = 15925,                    // handles aura 26575

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
    PHASE_FROZEN                = 2,
    PHASE_EXPLODED              = 3,
};

static const uint32 auiGlobSummonSpells[MAX_VISCIDUS_GLOBS] = { 25865, 25866, 25867, 25868, 25869, 25870, 25871, 25872, 25873, 25874, 25875, 25876, 25877, 25878, 25879, 25880, 25881, 25882, 25883, 25884 };

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
    }

    ScriptedInstance* m_instance;

    uint8 m_uiPhase;

    uint32 m_uiHitCount;

    GuidList m_lGlobesGuidList;

    void Reset() override
    {
        CombatAI::Reset();
        m_uiPhase                 = PHASE_NORMAL;
        m_uiHitCount              = 0;

        DoCastSpellIfCan(nullptr, SPELL_MEMBRANE_VISCIDUS, CAST_TRIGGERED);
        DoCastSpellIfCan(nullptr, SPELL_VISCIDUS_WEAKNESS, CAST_TRIGGERED);

        SetCombatScriptStatus(false);
        SetMeleeEnabled(true);
        SetCombatMovement(true);
        SetDeathPrevention(true);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VISCIDUS, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VISCIDUS, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_GLOBS, CAST_TRIGGERED);
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
        else if (summoned->GetEntry() == NPC_VISCIDUS_TRIGGER) // forwarded from map
            summoned->CastSpell(nullptr, SPELL_TOXIN, TRIGGERED_OLD_TRIGGERED);
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

            // suicide if required
            if (m_creature->GetHealthPercent() < 5.0f)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_INVIS_SELF);

                if (DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SUICIDE_TRIGGER, CAST_TRIGGERED) == CAST_OK)
                    m_creature->CastSpell(nullptr, SPELL_VISCIDUS_SUICIDE, TRIGGERED_OLD_TRIGGERED);
            }
            else if (m_lGlobesGuidList.empty())
            {
                m_creature->RemoveAurasDueToSpell(SPELL_INVIS_SELF);
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                m_uiPhase = PHASE_NORMAL;

                SetCombatScriptStatus(false);
                SetMeleeEnabled(true);
                SetCombatMovement(true, true);
            }
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
        {
            m_creature->RemoveAurasDueToSpell(SPELL_INVIS_SELF);
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_uiPhase = PHASE_NORMAL;

            SetCombatScriptStatus(false);
            SetMeleeEnabled(true);
            SetCombatMovement(true, true);
        }
    }

    void DamageTaken(Unit* dealer, uint32& damage, DamageEffectType damagetype, SpellEntry const* spellInfo) override
    {
        if (m_uiPhase == PHASE_FROZEN)
        {
            if (!spellInfo || spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE) // only melee attacks - TODO: do through proc of SPELL_VISCIDUS_FREEZE
            {
                ++m_uiHitCount;

                // only count melee attacks
                if (m_uiHitCount >= HITCOUNT_EXPLODE)
                {
                    if (m_creature->GetHealthPercent() <= 5.0f)
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SUICIDE_TRIGGER, CAST_TRIGGERED) == CAST_OK)
                            m_creature->CastSpell(nullptr, SPELL_VISCIDUS_SUICIDE, TRIGGERED_OLD_TRIGGERED);
                    }
                    else if (DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_EXPLODE, CAST_TRIGGERED | CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                    {
                        DoScriptText(EMOTE_EXPLODE, m_creature);
                        m_uiPhase = PHASE_EXPLODED;
                        m_uiHitCount = 0;
                        m_lGlobesGuidList.clear();
                        uint32 uiGlobeCount = m_creature->GetHealthPercent() / 5.0f;

                        DoCastSpellIfCan(m_creature, SPELL_SUMMON_GLOBS, CAST_TRIGGERED);

                        for (uint8 i = 0; i < uiGlobeCount; ++i)
                            DoCastSpellIfCan(m_creature, auiGlobSummonSpells[i], CAST_TRIGGERED);

                        m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE);
                        ResetTimer(VISCIDUS_EXPLODE, 2000);

                        SetCombatScriptStatus(true);
                        SetCombatMovement(false, true);
                        SetMeleeEnabled(false);
                        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                    }
                }
                else if (m_uiHitCount == HITCOUNT_SHATTER)
                    DoScriptText(EMOTE_SHATTER, m_creature);
                else if (m_uiHitCount == HITCOUNT_CRACK)
                    DoScriptText(EMOTE_CRACK, m_creature);
            }
        }
        CombatAI::DamageTaken(dealer, damage, damagetype, spellInfo);
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (m_uiPhase != PHASE_NORMAL)
            return;

        // only count frost damage
        if (GetSchoolMask(pSpell->School) == SPELL_SCHOOL_MASK_FROST) // - TODO: do through proc of SPELL_VISCIDUS_WEAKNESS
        {
            ++m_uiHitCount;

            if (m_uiHitCount >= HITCOUNT_FREEZE)
            {
                m_uiPhase = PHASE_FROZEN;
                m_uiHitCount = 0;

                if (m_uiHitCount == HITCOUNT_FREEZE)
                    DoScriptText(EMOTE_FROZEN, m_creature);
                m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED_MORE);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_FREEZE, CAST_TRIGGERED);
            }
            else if (m_uiHitCount >= HITCOUNT_SLOW_MORE)
            {
                if (m_uiHitCount == HITCOUNT_SLOW_MORE)
                    DoScriptText(EMOTE_FREEZE, m_creature);
                m_creature->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SLOWED_MORE, CAST_TRIGGERED);
            }
            else if (m_uiHitCount >= HITCOUNT_SLOW)
            {
                if (m_uiHitCount == HITCOUNT_SLOW)
                    DoScriptText(EMOTE_SLOW, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_VISCIDUS_SLOWED, CAST_TRIGGERED);
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* /*pInvoker*/, uint32 /*uiMiscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            if (m_uiPhase == PHASE_EXPLODED)
                return;

            // reset phase if not already exploded
            m_uiPhase = PHASE_NORMAL;
            m_uiHitCount = 0;
        }
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
        }
    }

    void HandleExplode()
    {
        // Make invisible
        m_creature->CastSpell(nullptr, SPELL_INVIS_SELF, TRIGGERED_OLD_TRIGGERED);
        DoCastSpellIfCan(nullptr, SPELL_VISCIDUS_TELEPORT, CAST_TRIGGERED);
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

void AddSC_boss_viscidus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_viscidus";
    pNewScript->GetAI = &GetNewAIInstance<boss_viscidusAI>;
    pNewScript->RegisterSelf();

    RegisterAuraScript<ViscidusFreeze>("spell_viscidus_freeze");
    RegisterSpellScript<SummonToxicSlime>("spell_summon_toxic_slime");
}
