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
SDName: Zul'Gurub
SD%Complete: 0
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"

/* ContentData
EndContentData */

enum
{
    // npc_soulflayer
    SPELL_SOUL_TAP               = 24619,
    SPELL_LIGHTNING_BREATH       = 20543,
    SPELL_THRASH                 = 12787,
    SPELL_KNOCKDOWN              = 20276,
    SPELL_FEAR                   = 22678,
    SPELL_FRENZY                 = 28371,
    SPELL_ENRAGE                 = 8269
};

/*######
## npc_soulflayer
######*/

struct npc_soulflayerAI : public ScriptedAI
{
    npc_soulflayerAI(Creature* pCreature) : ScriptedAI(pCreature), m_uiRandomBuffAbility(0) { Reset(); }

    uint32 m_uiSoulTapTimer;
    uint32 m_uiLightingBreathTimer;
    uint32 m_uiRandomCcAbility;
    uint32 m_uiRandomCcAbilityTimer;
    uint32 m_uiRandomBuffAbility;

    const uint32 spell_list_cc[2] = { SPELL_FEAR, SPELL_KNOCKDOWN };
    const uint32 spell_list_buff[3] = { SPELL_ENRAGE, SPELL_FRENZY, SPELL_THRASH };

    void JustRespawned() override
    {
        // Set random time based CC ability on spawn (Fear or Knockdown)
        m_uiRandomCcAbility = spell_list_cc[urand(0, 1)];

        // Set a random buff HP based ability (Frenzy or Enrage) or Thrash on spawn
        m_uiRandomBuffAbility = spell_list_buff[urand(0, 2)];

        // Cast Thrash on spawn, since a passive ability
        if (m_uiRandomBuffAbility == SPELL_THRASH)
            DoCastSpellIfCan(m_creature, m_uiRandomBuffAbility, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Reset() override
    {
        m_uiRandomCcAbilityTimer = urand(2000, 5000);
        m_uiSoulTapTimer = urand(m_uiRandomCcAbilityTimer, 7000);
        m_uiLightingBreathTimer = urand(m_uiSoulTapTimer, 9000);

        if (m_uiRandomBuffAbility == SPELL_THRASH)
            DoCastSpellIfCan(m_creature, m_uiRandomBuffAbility, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Check if we have a valid target, otherwise do nothing
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Check if we are casting / channelling, return to not interrupt process and pause CDs
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        // Soul Tap
        if (m_uiSoulTapTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SOUL_TAP, SELECT_TARGET_ANY_ENEMY))
                if (DoCastSpellIfCan(pTarget, SPELL_SOUL_TAP) == CAST_OK)
                    m_uiSoulTapTimer = urand(10000, 15000);
        }
        else
            m_uiSoulTapTimer -= uiDiff;

        // Lightning Breath
        if (m_uiLightingBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_LIGHTNING_BREATH) == CAST_OK)
                m_uiLightingBreathTimer = urand(10000, 15000);
        }
        else
            m_uiLightingBreathTimer -= uiDiff;

        // Random time based CC ability
        // Either Fear or Knockdown
        if (m_uiRandomCcAbilityTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), m_uiRandomCcAbility) == CAST_OK)
                m_uiRandomCcAbilityTimer = urand(8000, 10000);
        }
        else
            m_uiRandomCcAbilityTimer -= uiDiff;

        // Random HP based ability
        // Either Frenzy or Enrage
        if (m_uiRandomBuffAbility != SPELL_THRASH)
            if (m_creature->GetHealthPercent() <= 30.0f)
                if (!m_creature->HasAura(m_uiRandomBuffAbility))
                    DoCastSpellIfCan(m_creature->GetVictim(), m_uiRandomBuffAbility);

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_soulflayer(Creature* pCreature)
{
    return new npc_soulflayerAI(pCreature);
}

enum // leftover hazzarah enum for future docu
{
    SPELL_CHAIN_BURN            = 24684,
    SPELL_SLEEP                 = 24664,
    SPELL_EARTH_SHOCK           = 24685,
    SPELL_SUMMON_ILLUSION_1     = 24681,
    SPELL_SUMMON_ILLUSION_2     = 24728, // main spell with script effect
    SPELL_SUMMON_ILLUSION_3     = 24729,
};

struct SummonNightmareIllusion : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
        {
            Unit* caster = spell->GetCaster();
            caster->CastSpell(nullptr, SPELL_SUMMON_ILLUSION_1, TRIGGERED_OLD_TRIGGERED);
            caster->CastSpell(nullptr, SPELL_SUMMON_ILLUSION_3, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

struct DelusionsOfJindo : public SpellScript
{
    void OnHit(Spell* spell, SpellMissInfo /*missInfo*/) const
    {
        Unit* caster = spell->GetCaster();
        caster->CastSpell(nullptr, 24308, TRIGGERED_OLD_TRIGGERED);
    }
};

struct SummonShadeOfJindo : public SpellScript
{
    void OnSummon(Spell* /*spell*/, Creature* summon) const
    {
        summon->CastSpell(nullptr, 24307, TRIGGERED_NONE);
        summon->CastSpell(nullptr, 23878, TRIGGERED_NONE);
        summon->CastSpell(nullptr, 24313, TRIGGERED_NONE);
    }
};

struct RandomAggro : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* caster = spell->GetCaster();
        if (effIdx != EFFECT_INDEX_0 || !caster->AI())
            return;

        spell->GetCaster()->AI()->AttackStart(spell->GetUnitTarget());
    }
};

void AddSC_zulgurub()
{
    Script* pNewScript = new Script();
    pNewScript->Name = "npc_soulflayer";
    pNewScript->GetAI = &GetAI_npc_soulflayer;
    pNewScript->RegisterSelf();

    RegisterSpellScript<SummonNightmareIllusion>("spell_summon_nightmare_illusion_hazzarah");
    RegisterSpellScript<DelusionsOfJindo>("spell_delusions_of_jindo");
    RegisterSpellScript<SummonShadeOfJindo>("spell_summon_shade_of_jindo");
    RegisterSpellScript<RandomAggro>("spell_random_aggro");
}
