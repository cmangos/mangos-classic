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

#include "AI/ScriptDevAI/include/sc_common.h"
#include "guard_ai.h"

// This script is for use within every single guard to save coding time

guardAI::guardAI(Creature* creature) : ScriptedAI(creature),
    m_globalCooldown(0),
    m_buffTimer(0)
{}

void guardAI::Reset()
{
    m_globalCooldown = 0;
    m_buffTimer = 0;                                      // Rebuff as soon as we can
}

void guardAI::Aggro(Unit* who)
{
    if (m_creature->GetEntry() == NPC_CENARION_INFANTRY)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_GUARD_SIL_AGGRO1, m_creature, who); break;
            case 1: DoScriptText(SAY_GUARD_SIL_AGGRO2, m_creature, who); break;
            case 2: DoScriptText(SAY_GUARD_SIL_AGGRO3, m_creature, who); break;
        }
    }

    if (const SpellEntry* spellInfo = m_creature->ReachWithSpellAttack(who))
        m_creature->CastSpell(who, spellInfo, TRIGGERED_NONE);
}

void guardAI::JustDied(Unit* killer)
{
    // Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
    if (Player* pPlayer = killer->GetBeneficiaryPlayer())
        m_creature->SendZoneUnderAttackMessage(pPlayer);
}

void guardAI::UpdateAI(const uint32 diff)
{
    // Always decrease our global cooldown first
    if (m_globalCooldown > diff)
        m_globalCooldown -= diff;
    else
        m_globalCooldown = 0;

    // Buff timer (only buff when we are alive and not in combat
    if (m_creature->IsAlive() && !m_creature->IsInCombat())
    {
        if (m_buffTimer < diff)
        {
            // Find a spell that targets friendly and applies an aura (these are generally buffs)
            const SpellEntry* spellInfo = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_AURA);

            if (spellInfo && !m_globalCooldown)
            {
                // Cast the buff spell
                m_creature->CastSpell(m_creature, spellInfo, TRIGGERED_NONE);

                // Set our global cooldown
                m_globalCooldown = GENERIC_CREATURE_COOLDOWN;

                // Set our timer to 10 minutes before rebuff
                m_buffTimer = 600000;
            }                                               // Try again in 30 seconds
            else
                m_buffTimer = 30000;
        }
        else
            m_buffTimer -= diff;
    }

    // Return since we have no target
    if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
        return;

    // Make sure our attack is ready and we arn't currently casting
    if (!m_creature->IsNonMeleeSpellCasted(false))
    {
        // If we are within range melee the target
        if (m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
        {
            bool bHealing = false;
            const SpellEntry* spellInfo = nullptr;

            // Select a healing spell if less than 30% hp
            if (m_creature->GetHealthPercent() < 30.0f && !urand(0, 2))
                spellInfo = SelectSpell(m_creature, -1, -1, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            // No healing spell available, select a hostile spell
            if (spellInfo)
                bHealing = true;
            else
                spellInfo = SelectSpell(m_creature->GetVictim(), -1, -1, SELECT_TARGET_ANY_ENEMY, 0, 0, 0, 0, SELECT_EFFECT_DONTCARE);

            // 20% chance to replace our white hit with a spell
            if (spellInfo && !urand(0, 4) && !m_globalCooldown)
            {
                // Cast spell
                DoCastSpellIfCan(bHealing ? m_creature : m_creature->GetVictim(), spellInfo->Id);

                // Set our global cooldown
                m_globalCooldown = GENERIC_CREATURE_COOLDOWN;
            }
        }
    }

    DoMeleeAttackIfReady();
}

void guardAI::DoReplyToTextEmote(uint32 textEmote)
{
    switch (textEmote)
    {
        case TEXTEMOTE_KISS:    m_creature->HandleEmote(EMOTE_ONESHOT_BOW);    break;
        case TEXTEMOTE_WAVE:    m_creature->HandleEmote(EMOTE_ONESHOT_WAVE);   break;
        case TEXTEMOTE_SALUTE:  m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE); break;
        case TEXTEMOTE_SHY:     m_creature->HandleEmote(EMOTE_ONESHOT_FLEX);   break;
        case TEXTEMOTE_RUDE:
        case TEXTEMOTE_CHICKEN: m_creature->HandleEmote(EMOTE_ONESHOT_POINT);  break;
    }
}

void guardAI_orgrimmar::ReceiveEmote(Player* player, uint32 textEmote)
{
    if (player->GetTeam() == HORDE)
        DoReplyToTextEmote(textEmote);
}
