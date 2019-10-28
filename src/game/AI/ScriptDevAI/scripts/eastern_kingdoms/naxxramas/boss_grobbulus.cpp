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
SDName: Boss_Grobbulus
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/*Poison Cloud 26590
Slime Spray 28157
Fallout slime 28218
Mutating Injection 28169
Enrages 26527*/


#include "naxxramas.h"

enum
{
    SPELL_SLIME_STREAM              = 28137,
    SPELL_MUTATING_INJECTION        = 28169,
    SPELL_POISON_CLOUD              = 28240,
    SPELL_SLIME_SPRAY               = 28157,
    SPELL_SUMMON_FALLOUT_SLIME      = 28218,
    SPELL_BERSERK                   = 26662,

    SPELL_DESPAWN_SUMMONS           = 30134,            // Spell ID unsure: there are several spells doing the same thing in DBCs within Naxxramas spell IDs range
                                                        // but it is not always clear which one is for each boss so some are assigned randomly like this one
};

struct boss_grobbulusAI : public ScriptedAI
{
    boss_grobbulusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiInjectionTimer;
    uint32 m_uiPoisonCloudTimer;
    uint32 m_uiSlimeSprayTimer;
    uint32 m_uiBerserkTimer;
    uint32 m_uiSlimeStreamTimer;

    void Reset() override
    {
        m_uiInjectionTimer = 13 * IN_MILLISECONDS;
        m_uiPoisonCloudTimer = urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
        m_uiSlimeSprayTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS);
        m_uiBerserkTimer = 12 * MINUTE * IN_MILLISECONDS;
        m_uiSlimeStreamTimer = 5 * IN_MILLISECONDS;         // The first few secs it is ok to be out of range
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GROBBULUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GROBBULUS, DONE);
    }

    void EnterEvadeMode() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GROBBULUS, FAIL);

        // Clean-up stage
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_SUMMONS, CAST_TRIGGERED);
        
        ScriptedAI::EnterEvadeMode();
    }

    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell) override
    {
        // Summon a Fallout Slime for every player hit by Slime Spray
        if ((pSpell->Id == SPELL_SLIME_SPRAY) && pTarget->GetTypeId() == TYPEID_PLAYER)
            DoCastSpellIfCan(pTarget, SPELL_SUMMON_FALLOUT_SLIME, CAST_TRIGGERED);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Slime Stream
        if (!m_uiSlimeStreamTimer)
        {
            if (!m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SLIME_STREAM) == CAST_OK)
                    // Give some time, to re-reach Grobbulus
                    m_uiSlimeStreamTimer = 3 * IN_MILLISECONDS;
            }
        }
        else
        {
            if (m_uiSlimeStreamTimer < uiDiff)
                m_uiSlimeStreamTimer = 0;
            else
                m_uiSlimeStreamTimer -= uiDiff;
        }

        // Berserk
        if (m_uiBerserkTimer)
        {
            if (m_uiBerserkTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                    m_uiBerserkTimer = 0;
            }
            else
                m_uiBerserkTimer -= uiDiff;
        }

        // SlimeSpray
        if (m_uiSlimeSprayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SLIME_SPRAY) == CAST_OK)
                m_uiSlimeSprayTimer = urand(20, 30) * IN_MILLISECONDS;
        }
        else
            m_uiSlimeSprayTimer -= uiDiff;

        // Mutagen Injection
        if (m_uiInjectionTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MUTATING_INJECTION, SELECT_FLAG_PLAYER | SELECT_FLAG_NOT_AURA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MUTATING_INJECTION, CAST_TRIGGERED) == CAST_OK)
                {
                    // Mutagen Injection is used more often when below 30% HP
                    if (m_creature->GetHealthPercent() > 30.0f)
                        m_uiInjectionTimer = urand(7, 13) * IN_MILLISECONDS;
                    else
                        m_uiInjectionTimer = urand(3, 7) * IN_MILLISECONDS;
                }
            }
        }
        else
            m_uiInjectionTimer -= uiDiff;

        // Poison Cloud
        if (m_uiPoisonCloudTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISON_CLOUD) == CAST_OK)
                m_uiPoisonCloudTimer = 15 * IN_MILLISECONDS;
        }
        else
            m_uiPoisonCloudTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_grobbulus(Creature* pCreature)
{
    return new boss_grobbulusAI(pCreature);
}

void AddSC_boss_grobbulus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_grobbulus";
    pNewScript->GetAI = &GetAI_boss_grobbulus;
    pNewScript->RegisterSelf();
}
