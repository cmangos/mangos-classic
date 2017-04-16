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
SDName: Guard_AI
SD%Complete: 90
SDComment:
SDCategory: Guards
EndScriptData */

#include "precompiled.h"
#include "guard_ai.h"

// This script is for use within every single guard to save coding time

guardAI::guardAI(Creature* pCreature) : ScriptedAI(pCreature),
    m_uiGlobalCooldown(0),
    m_uiBuffTimer(0)
{}

void guardAI::Reset()
{
    m_uiGlobalCooldown = 0;
    m_uiBuffTimer = 0;                                      // Rebuff as soon as we can
}

void guardAI::Aggro(Unit* pWho)
{
    if (m_creature->GetEntry() == NPC_CENARION_INFANTRY)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_GUARD_SIL_AGGRO1, m_creature, pWho); break;
            case 1: DoScriptText(SAY_GUARD_SIL_AGGRO2, m_creature, pWho); break;
            case 2: DoScriptText(SAY_GUARD_SIL_AGGRO3, m_creature, pWho); break;
        }
    }

    if (const SpellEntry* pSpellInfo = m_creature->ReachWithSpellAttack(pWho))
        DoCastSpell(pWho, pSpellInfo);
}

void guardAI::JustDied(Unit* pKiller)
{
    // Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
    if (Player* pPlayer = pKiller->GetCharmerOrOwnerPlayerOrPlayerItself())
        m_creature->SendZoneUnderAttackMessage(pPlayer);
}

void guardAI::UpdateAI(const uint32 uiDiff)
{
    // Always decrease our global cooldown first
    if (m_uiGlobalCooldown > uiDiff)
        m_uiGlobalCooldown -= uiDiff;
    else
        m_uiGlobalCooldown = 0;

    // Buff timer (only buff when we are alive and not in combat
    if (m_creature->isAlive() && !m_creature->isInCombat())
    {
        if (m_uiBuffTimer < uiDiff)
        {
            // Find a spell that targets friendly and applies an aura (these are generally buffs)
            const SpellEntry* pSpellInfo = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_AURA);

            if (pSpellInfo && !m_uiGlobalCooldown)
            {
                // Cast the buff spell
                DoCastSpell(m_creature, pSpellInfo);

                // Set our global cooldown
                m_uiGlobalCooldown = GENERIC_CREATURE_COOLDOWN;

                // Set our timer to 10 minutes before rebuff
                m_uiBuffTimer = 600000;
            }                                               // Try again in 30 seconds
            else
                m_uiBuffTimer = 30000;
        }
        else
            m_uiBuffTimer -= uiDiff;
    }

    // Return since we have no target
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    // Make sure our attack is ready and we arn't currently casting
    if (m_creature->isAttackReady() && !m_creature->IsNonMeleeSpellCasted(false))
    {
        // If we are within range melee the target
        if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
        {
            bool bHealing = false;
            const SpellEntry* pSpellInfo = nullptr;

            // Select a healing spell if less than 30% hp
            if (m_creature->GetHealthPercent() < 30.0f)
                pSpellInfo = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            // No healing spell available, select a hostile spell
            if (pSpellInfo)
                bHealing = true;
            else
                pSpellInfo = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, 0, 0, SELECT_EFFECT_DONTCARE);

            // 20% chance to replace our white hit with a spell
            if (pSpellInfo && !urand(0, 4) && !m_uiGlobalCooldown)
            {
                // Cast the spell
                if (bHealing)
                    DoCastSpell(m_creature, pSpellInfo);
                else
                    DoCastSpell(m_creature->getVictim(), pSpellInfo);

                // Set our global cooldown
                m_uiGlobalCooldown = GENERIC_CREATURE_COOLDOWN;
            }
            else
                m_creature->AttackerStateUpdate(m_creature->getVictim());

            m_creature->resetAttackTimer();
        }
    }
    else
    {
        // Only run this code if we arn't already casting
        if (!m_creature->IsNonMeleeSpellCasted(false))
        {
            bool bHealing = false;
            const SpellEntry* pSpellInfo = nullptr;

            // Select a healing spell if less than 30% hp ONLY 33% of the time
            if (m_creature->GetHealthPercent() < 30.0f && !urand(0, 2))
                pSpellInfo = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            // No healing spell available, See if we can cast a ranged spell (Range must be greater than ATTACK_DISTANCE)
            if (pSpellInfo)
                bHealing = true;
            else
                pSpellInfo = SelectSpell(m_creature->getVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, ATTACK_DISTANCE, 0, SELECT_EFFECT_DONTCARE);

            // Found a spell, check if we arn't on cooldown
            if (pSpellInfo && !m_uiGlobalCooldown)
            {
                // If we are currently moving stop us and set the movement generator
                if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
                {
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MoveIdle();
                }

                // Cast spell
                if (bHealing)
                    DoCastSpell(m_creature, pSpellInfo);
                else
                    DoCastSpell(m_creature->getVictim(), pSpellInfo);

                // Set our global cooldown
                m_uiGlobalCooldown = GENERIC_CREATURE_COOLDOWN;
            }                                               // If no spells available and we arn't moving run to target
            else if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
            {
                // Cancel our current spell and then mutate new movement generator
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->GetMotionMaster()->Clear(false);
                m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            }
        }
    }
}

void guardAI::DoReplyToTextEmote(uint32 uiTextEmote)
{
    switch (uiTextEmote)
    {
        case TEXTEMOTE_KISS:    m_creature->HandleEmote(EMOTE_ONESHOT_BOW);    break;
        case TEXTEMOTE_WAVE:    m_creature->HandleEmote(EMOTE_ONESHOT_WAVE);   break;
        case TEXTEMOTE_SALUTE:  m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE); break;
        case TEXTEMOTE_SHY:     m_creature->HandleEmote(EMOTE_ONESHOT_FLEX);   break;
        case TEXTEMOTE_RUDE:
        case TEXTEMOTE_CHICKEN: m_creature->HandleEmote(EMOTE_ONESHOT_POINT);  break;
    }
}

void guardAI_orgrimmar::ReceiveEmote(Player* pPlayer, uint32 uiTextEmote)
{
    if (pPlayer->GetTeam() == HORDE)
        DoReplyToTextEmote(uiTextEmote);
}

void guardAI_stormwind::ReceiveEmote(Player* pPlayer, uint32 uiTextEmote)
{
    if (pPlayer->GetTeam() == ALLIANCE)
        DoReplyToTextEmote(uiTextEmote);
}
