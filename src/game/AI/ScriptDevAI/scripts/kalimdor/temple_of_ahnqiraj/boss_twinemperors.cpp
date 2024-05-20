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
SDName: Boss_Twinemperors
SD%Complete: 95
SDComment: Timers
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Scripts/SpellScript.h"

enum
{
    // yells
    SAY_VEKLOR_AGGRO_1          = -1531019,
    SAY_VEKLOR_AGGRO_2          = -1531020,
    SAY_VEKLOR_AGGRO_3          = -1531021,
    SAY_VEKLOR_AGGRO_4          = -1531022,
    SAY_VEKLOR_SLAY             = 11453,
    SAY_VEKLOR_DEATH            = -1531024,
    SAY_VEKLOR_SPECIAL          = -1531025,

    SAY_VEKNILASH_AGGRO_1       = -1531026,
    SAY_VEKNILASH_AGGRO_2       = -1531027,
    SAY_VEKNILASH_AGGRO_3       = -1531028,
    SAY_VEKNILASH_AGGRO_4       = -1531029,
    SAY_VEKNILASH_SLAY          = -1531030,
    SAY_VEKNILASH_DEATH         = -1531031,
    SAY_VEKNILASH_SPECIAL       = -1531032,

    // common spells
    SPELL_TWIN_TELEPORT         = 799,
    SPELL_TWIN_TELEPORT_STUN    = 800,
    SPELL_HEAL_BROTHER          = 7393,
    SPELL_TWIN_TELEPORT_VISUAL  = 26638,
    SPELL_TWIN_SUICIDE          = 26627,
    SPELL_TWIN_EMPATHY          = 1177,

    // veklor spells
    SPELL_ARCANE_BURST          = 568,
    SPELL_EXPLODE_BUG           = 804,          // targets 15316 or 15317
    SPELL_SHADOW_BOLT           = 26006,
    SPELL_BLIZZARD              = 26607,
    SPELL_FRENZY                = 27897,

    // veknilash spells
    SPELL_MUTATE_BUG            = 802,          // targets 15316 or 15317
    SPELL_VIRULENT_POISON       = 22413,
    SPELL_UPPERCUT              = 26007,
    SPELL_UNBALANCING_STRIKE    = 26613,
    SPELL_BERSERK               = 27680,
    SPELL_DOUBLE_ATTACK         = 18943,

    FACTION_HOSTILE             = 16,
};

enum EmperorActions
{
    EMPEROR_BERSERK,
    EMPEROR_BUG_ABILITY,
    EMPEROR_ACTION_MAX,
    EMPEROR_TELEPORT_DELAY = 10,
};

struct boss_twin_emperorsAI : public CombatAI
{
    boss_twin_emperorsAI(Creature* creature, uint32 actionCount) : CombatAI(creature, actionCount), m_instance(static_cast<instance_temple_of_ahnqiraj*>(creature->GetInstanceData()))
    {
        AddCombatAction(EMPEROR_BERSERK, 15u * MINUTE * IN_MILLISECONDS);
        AddCombatAction(EMPEROR_BUG_ABILITY, urand(7000, 14000));
        AddCustomAction(EMPEROR_TELEPORT_DELAY, true, [&]() { HandleDelayedAttack(); }, TIMER_COMBAT_COMBAT);
    }

    instance_temple_of_ahnqiraj* m_instance;

    // Workaround for the shared health pool
    void DamageTaken(Unit* /*dealer*/, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* spellInfo) override
    {
        if (!m_instance || (spellInfo && spellInfo->Id == SPELL_TWIN_EMPATHY))
            return;

        if (Creature* twin = m_instance->GetSingleCreatureFromStorage(m_creature->GetEntry() == NPC_VEKLOR ? NPC_VEKNILASH : NPC_VEKLOR))
        {
            float fDamPercent = ((float)damage) / ((float)m_creature->GetMaxHealth());
            int32 twinDamage = (int32)(fDamPercent * ((float)twin->GetMaxHealth()));
            if (twin->GetHealth() <= uint32(twinDamage))
                twin->CastSpell(nullptr, SPELL_TWIN_SUICIDE, TRIGGERED_OLD_TRIGGERED);
            else
                twin->CastCustomSpell(nullptr, SPELL_TWIN_EMPATHY, &twinDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
    }

    // Workaround for the shared health pool
    void HealedBy(Unit* /*healer*/, uint32& uiHealedAmount) override
    {
        if (!m_instance)
            return;

        if (Creature* twin = m_instance->GetSingleCreatureFromStorage(m_creature->GetEntry() == NPC_VEKLOR ? NPC_VEKNILASH : NPC_VEKLOR))
        {
            float fHealPercent = ((float)uiHealedAmount) / ((float)m_creature->GetMaxHealth());
            uint32 uiTwinHeal = (uint32)(fHealPercent * ((float)twin->GetMaxHealth()));
            uint32 uiTwinHealth = twin->GetHealth() + uiTwinHeal;
            twin->SetHealth(uiTwinHealth < twin->GetMaxHealth() ? uiTwinHealth : twin->GetMaxHealth());
        }
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_TWINS, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_TWINS, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_TWINS, FAIL);
    }

    void StartDelayedAttack()
    {
        SetCombatScriptStatus(true);
        SetMeleeEnabled(false);
        SetCombatMovement(false);
        m_creature->SetTarget(nullptr);
        ResetTimer(EMPEROR_TELEPORT_DELAY, 2000);
    }

    void HandleDelayedAttack()
    {
        SetCombatScriptStatus(false);
        SetMeleeEnabled(true);
        SetCombatMovement(true);
    }

    // Return true, if succeeded
    virtual bool DoHandleBugAbility() = 0;
    virtual bool DoHandleBerserk() = 0;

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case EMPEROR_BERSERK:
            {
                if (DoHandleBerserk())
                    DisableCombatAction(action);
                break;
            }
            case EMPEROR_BUG_ABILITY:
            {
                if (DoHandleBugAbility())
                    ResetCombatAction(action, urand(10000, 17000));
                break;
            }
        }
    }
};

enum VeknilashActions
{
    VEKNILASH_UPPERCUT = EMPEROR_ACTION_MAX,
    VEKNILASH_UNBALANCING_STRIKE,
    VEKNILASH_ACTION_MAX,
};

struct boss_veknilashAI : public boss_twin_emperorsAI
{
    boss_veknilashAI(Creature* creature) : boss_twin_emperorsAI(creature, VEKNILASH_ACTION_MAX)
    {
        AddCombatAction(VEKNILASH_UPPERCUT, urand(14000, 29000));
        AddCombatAction(VEKNILASH_UNBALANCING_STRIKE, urand(8000, 18000));
    }

    void Reset() override
    {
        boss_twin_emperorsAI::Reset();

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (m_instance && m_instance->GetData(TYPE_TWINS) == IN_PROGRESS && who->GetEntry() == NPC_VEKLOR && who->IsWithinDistInMap(m_creature, 60.0f))
            DoCastSpellIfCan(who, SPELL_HEAL_BROTHER);

        boss_twin_emperorsAI::MoveInLineOfSight(who);
    }

    void Aggro(Unit* who) override
    {
        boss_twin_emperorsAI::Aggro(who);

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_VEKNILASH_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_VEKNILASH_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_VEKNILASH_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_VEKNILASH_AGGRO_4, m_creature); break;
        }
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_VEKNILASH_SLAY, m_creature);
    }

    void JustDied(Unit* killer) override
    {
        boss_twin_emperorsAI::JustDied(killer);

        DoScriptText(SAY_VEKNILASH_DEATH, m_creature);
    }

    bool DoHandleBugAbility() override
    {
        return DoCastSpellIfCan(nullptr, SPELL_MUTATE_BUG) == CAST_OK;
    }

    bool DoHandleBerserk() override
    {
        return DoCastSpellIfCan(nullptr, SPELL_BERSERK) == CAST_OK;
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case VEKNILASH_UPPERCUT:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_UPPERCUT, SELECT_FLAG_IN_MELEE_RANGE))
                {
                    if (DoCastSpellIfCan(target, SPELL_UPPERCUT) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 30000));
                }
                break;
            }
            case VEKNILASH_UNBALANCING_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_UNBALANCING_STRIKE) == CAST_OK)
                    ResetCombatAction(action, urand(8000, 20000));
                break;
            }
            default:
                boss_twin_emperorsAI::ExecuteAction(action);
                break;
        }
    }
};

enum VeklorActions
{
    VEKLOR_SHADOWBOLT = EMPEROR_ACTION_MAX,
    VEKLOR_BLIZZARD,
    VEKLOR_ARCANE_BURST,
    VEKLOR_TELEPORT,
    VEKLOR_ACTION_MAX,
};

struct boss_veklorAI : public boss_twin_emperorsAI
{
    boss_veklorAI(Creature* creature) : boss_twin_emperorsAI(creature, VEKLOR_ACTION_MAX)
    {
        AddCombatAction(VEKLOR_SHADOWBOLT, 1000u);
        AddCombatAction(VEKLOR_BLIZZARD, urand(15000, 20000));
        AddCombatAction(VEKLOR_ARCANE_BURST, 1000u);
        AddCombatAction(VEKLOR_TELEPORT, 35000u);
        AddOnKillText(SAY_VEKLOR_SLAY);
    }

    void Reset() override
    {
        boss_twin_emperorsAI::Reset();

        m_meleeEnabled = false;
        m_attackDistance = 40.f;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (m_instance && m_instance->GetData(TYPE_TWINS) == IN_PROGRESS && who->GetEntry() == NPC_VEKNILASH && who->IsWithinDistInMap(m_creature, 60.0f))
            DoCastSpellIfCan(who, SPELL_HEAL_BROTHER);

        boss_twin_emperorsAI::MoveInLineOfSight(who);
    }

    void Aggro(Unit* who) override
    {
        boss_twin_emperorsAI::Aggro(who);

        switch (urand(0, 3))
        {
            case 0: DoScriptText(SAY_VEKLOR_AGGRO_1, m_creature); break;
            case 1: DoScriptText(SAY_VEKLOR_AGGRO_2, m_creature); break;
            case 2: DoScriptText(SAY_VEKLOR_AGGRO_3, m_creature); break;
            case 3: DoScriptText(SAY_VEKLOR_AGGRO_4, m_creature); break;
        }
    }

    void JustDied(Unit* killer) override
    {
        boss_twin_emperorsAI::JustDied(killer);

        DoScriptText(SAY_VEKLOR_DEATH, m_creature);
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_TWIN_TELEPORT)
        {

            if (Creature* veknilash = m_instance->GetSingleCreatureFromStorage(NPC_VEKNILASH))
            {
                float twinX, twinY, twinZ;
                m_creature->GetPosition(twinX, twinY, twinZ);
                float twinOri = m_creature->GetOrientation();

                TeleportTwin(m_creature, veknilash->GetPositionX(), veknilash->GetPositionY(), veknilash->GetPositionZ(), veknilash->GetOrientation());
                TeleportTwin(veknilash, twinX, twinY, twinZ, twinOri);
            }
        }
    }

    void TeleportTwin(Creature* twin, float x, float y, float z, float ori)
    {
        twin->AI()->DoResetThreat();
        twin->AI()->DoCastSpellIfCan(nullptr, SPELL_TWIN_TELEPORT_VISUAL, CAST_TRIGGERED);
        twin->AI()->DoCastSpellIfCan(nullptr, SPELL_TWIN_TELEPORT_STUN, CAST_TRIGGERED);
        twin->NearTeleportTo(x, y, z, ori, true);
        static_cast<boss_twin_emperorsAI*>(twin->AI())->StartDelayedAttack();
    }

    bool DoHandleBugAbility() override
    {
        return DoCastSpellIfCan(nullptr, SPELL_EXPLODE_BUG) == CAST_OK;
    }

    bool DoHandleBerserk() override
    {
        return DoCastSpellIfCan(nullptr, SPELL_FRENZY) == CAST_OK;
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case VEKLOR_SHADOWBOLT:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHADOW_BOLT) == CAST_OK)
                    ResetCombatAction(action, 2000);
                break;
            }
            case VEKLOR_BLIZZARD:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_BLIZZARD, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_BLIZZARD) == CAST_OK)
                        ResetCombatAction(action, urand(15000, 30000));
                break;
            }
            case VEKLOR_ARCANE_BURST:
            {
                if (m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, SPELL_ARCANE_BURST, SELECT_FLAG_PLAYER | SELECT_FLAG_USE_EFFECT_RADIUS))
                    if (DoCastSpellIfCan(nullptr, SPELL_ARCANE_BURST) == CAST_OK)
                        ResetCombatAction(action, 5000);
                break;
            }
            case VEKLOR_TELEPORT:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_TWIN_TELEPORT) == CAST_OK)
                    ResetCombatAction(action, 35000);
                break;
            }
            default:
                boss_twin_emperorsAI::ExecuteAction(action);
                break;
        }
    }
};

struct MutateBug : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (aura->GetEffIndex() == EFFECT_INDEX_0 && apply)
        {
            aura->GetTarget()->AI()->DoCastSpellIfCan(nullptr, SPELL_VIRULENT_POISON, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
            if (aura->GetTarget()->IsCreature())
            {
                aura->GetTarget()->setFaction(FACTION_HOSTILE);
                static_cast<Creature*>(aura->GetTarget())->SetInCombatWithZone();
                aura->GetTarget()->AI()->AttackClosestEnemy();
            }
        }
    }
};

void AddSC_boss_twinemperors()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_veknilash";
    pNewScript->GetAI = &GetNewAIInstance<boss_veknilashAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_veklor";
    pNewScript->GetAI = &GetNewAIInstance<boss_veklorAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<MutateBug>("spell_mutate_bug");
}
