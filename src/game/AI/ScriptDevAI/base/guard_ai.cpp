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

guardAI::guardAI(Creature* creature) : CombatAI(creature, 0),
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
}

void guardAI::JustDied(Unit* killer)
{
    if (!killer)
        return;

    // Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
    if (Player* pPlayer = killer->GetBeneficiaryPlayer())
        m_creature->SendZoneUnderAttackMessage(pPlayer);
}

void guardAI::JustRespawned()
{
    CombatAI::JustRespawned();
    const auto& spellList = GetSpellList();
    if (spellList.ChanceRangedAttack)
    {
        for (auto& spell : spellList.Spells)
        {
            if (spell.second.Flags & SPELL_LIST_FLAG_RANGED_ACTION)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell.second.SpellId);
                if (!spellInfo)
                    continue;
                SetRangedMode(true, CalculateSpellRange(spellInfo) * 0.8f, TYPE_PROXIMITY);
                break;
            }
        }
    }
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
