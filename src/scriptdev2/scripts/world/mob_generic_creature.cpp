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
SDName: Generic_Creature
SD%Complete: 80
SDComment: Should be replaced with core based AI
SDCategory: Creatures
EndScriptData */

#include "precompiled.h"

#define GENERIC_CREATURE_COOLDOWN   5000

struct generic_creatureAI : public ScriptedAI
{
    generic_creatureAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 GlobalCooldown;      // This variable acts like the global cooldown that players have (1.5 seconds)
    uint32 BuffTimer;           // This variable keeps track of buffs
    bool IsSelfRooted;

    void Reset() override
    {
        GlobalCooldown = 0;
        BuffTimer = 0;          // Rebuff as soon as we can
        IsSelfRooted = false;
    }

    void Aggro(Unit* who) override
    {
        if (!m_creature->CanReachWithMeleeAttack(who))
        {
            IsSelfRooted = true;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        // Always decrease our global cooldown first
        if (GlobalCooldown > diff)
            GlobalCooldown -= diff;
        else GlobalCooldown = 0;

        // Buff timer (only buff when we are alive and not in combat
        if (!m_creature->isInCombat() && m_creature->isAlive())
        {
            if (BuffTimer < diff)
            {
                // Find a spell that targets friendly and applies an aura (these are generally buffs)
                SpellEntry const* info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_AURA);

                if (info && !GlobalCooldown)
                {
                    // Cast the buff spell
                    DoCastSpell(m_creature, info);

                    // Set our global cooldown
                    GlobalCooldown = GENERIC_CREATURE_COOLDOWN;

                    // Set our timer to 10 minutes before rebuff
                    BuffTimer = 600000;
                }// Try agian in 30 seconds
                else BuffTimer = 30000;
            }
            else BuffTimer -= diff;
        }

        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Return if we already cast a spell
        if (m_creature->IsNonMeleeSpellCasted(false))
            return;

        // If we are within range melee the target
        if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
        {
            // Make sure our attack is ready
            if (m_creature->isAttackReady())
            {
                bool Healing = false;
                SpellEntry const* info = NULL;

                // Select a healing spell if less than 30% hp
                if (m_creature->GetHealthPercent() < 30.0f)
                    info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

                // No healing spell available, select a hostile spell
                if (info) Healing = true;
                else info = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, 0, 0, SELECT_EFFECT_DONTCARE);

                // 50% chance if elite or higher, 20% chance if not, to replace our white hit with a spell
                if (info && (rand() % (m_creature->GetCreatureInfo()->Rank > 1 ? 2 : 5) == 0) && !GlobalCooldown)
                {
                    // Cast the spell
                    if (Healing)DoCastSpell(m_creature, info);
                    else DoCastSpell(m_creature->getVictim(), info);

                    // Set our global cooldown
                    GlobalCooldown = GENERIC_CREATURE_COOLDOWN;
                }
                else m_creature->AttackerStateUpdate(m_creature->getVictim());

                m_creature->resetAttackTimer();
            }
        }
        else
        {
            bool Healing = false;
            SpellEntry const* info = NULL;

            // Select a healing spell if less than 30% hp ONLY 33% of the time
            if (m_creature->GetHealthPercent() < 30.0f && !urand(0, 2))
                info = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            // No healing spell available, See if we can cast a ranged spell (Range must be greater than ATTACK_DISTANCE)
            if (info) Healing = true;
            else info = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, ATTACK_DISTANCE, 0, SELECT_EFFECT_DONTCARE);

            // Found a spell, check if we arn't on cooldown
            if (info && !GlobalCooldown)
            {
                // If we are currently moving stop us and set the movement generator
                if (!IsSelfRooted)
                {
                    IsSelfRooted = true;
                }

                // Cast spell
                if (Healing) DoCastSpell(m_creature, info);
                else DoCastSpell(m_creature->getVictim(), info);

                // Set our global cooldown
                GlobalCooldown = GENERIC_CREATURE_COOLDOWN;
            }// If no spells available and we arn't moving run to target
            else if (IsSelfRooted)
            {
                // Cancel our current spell and then allow movement agian
                m_creature->InterruptNonMeleeSpells(false);
                IsSelfRooted = false;
            }
        }
    }
};

CreatureAI* GetAI_generic_creature(Creature* pCreature)
{
    return new generic_creatureAI(pCreature);
}

void AddSC_generic_creature()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "generic_creature";
    pNewScript->GetAI = &GetAI_generic_creature;
    pNewScript->RegisterSelf(false);
}
