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
    boss_grobbulusAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_injectionTimer;
    uint32 m_poisonCloudTimer;
    uint32 m_slimeSprayTimer;
    uint32 m_berserkTimer;
    uint32 m_slimeStreamTimer;

    void Reset() override
    {
        m_injectionTimer = 13 * IN_MILLISECONDS;
        m_poisonCloudTimer = urand(20 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
        m_slimeSprayTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS);
        m_berserkTimer = 12 * MINUTE * IN_MILLISECONDS;
        m_slimeStreamTimer = 5 * IN_MILLISECONDS;         // The first few secs it is ok to be out of range
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, DONE);
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_GROBBULUS, FAIL);

        // Clean-up stage
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_SUMMONS, CAST_TRIGGERED);

        ScriptedAI::EnterEvadeMode();
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell) override
    {
        // Summon a Fallout Slime for every player hit by Slime Spray
        if ((spell->Id == SPELL_SLIME_SPRAY) && target->GetTypeId() == TYPEID_PLAYER)
            DoCastSpellIfCan(target, SPELL_SUMMON_FALLOUT_SLIME, CAST_TRIGGERED);
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Slime Stream
        if (!m_slimeStreamTimer)
        {
            if (!m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SLIME_STREAM) == CAST_OK)
                    // Give some time, to re-reach Grobbulus
                    m_slimeStreamTimer = 3 * IN_MILLISECONDS;
            }
        }
        else
        {
            if (m_slimeStreamTimer < diff)
                m_slimeStreamTimer = 0;
            else
                m_slimeStreamTimer -= diff;
        }

        // Berserk
        if (m_berserkTimer)
        {
            if (m_berserkTimer <= diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                    m_berserkTimer = 0;
            }
            else
                m_berserkTimer -= diff;
        }

        // SlimeSpray
        if (m_slimeSprayTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SLIME_SPRAY) == CAST_OK)
                m_slimeSprayTimer = urand(20, 30) * IN_MILLISECONDS;
        }
        else
            m_slimeSprayTimer -= diff;

        // Mutagen Injection
        if (m_injectionTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MUTATING_INJECTION, SELECT_FLAG_PLAYER | SELECT_FLAG_NOT_AURA))
            {
                if (DoCastSpellIfCan(target, SPELL_MUTATING_INJECTION, CAST_TRIGGERED) == CAST_OK)
                {
                    // Mutagen Injection is used more often when below 30% HP
                    if (m_creature->GetHealthPercent() > 30.0f)
                        m_injectionTimer = urand(7, 13) * IN_MILLISECONDS;
                    else
                        m_injectionTimer = urand(3, 7) * IN_MILLISECONDS;
                }
            }
        }
        else
            m_injectionTimer -= diff;

        // Poison Cloud
        if (m_poisonCloudTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISON_CLOUD) == CAST_OK)
                m_poisonCloudTimer = 15 * IN_MILLISECONDS;
        }
        else
            m_poisonCloudTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_grobbulus(Creature* creature)
{
    return new boss_grobbulusAI(creature);
}

void AddSC_boss_grobbulus()
{
    Script* newScript = new Script;
    newScript->Name = "boss_grobbulus";
    newScript->GetAI = &GetAI_boss_grobbulus;
    newScript->RegisterSelf();
}
