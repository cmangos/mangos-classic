/*
* This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
*
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

#include "PlayerAI.h"
#include "Entities/Player.h"
#include "Spells/SpellMgr.h"

PlayerAI::PlayerAI(Player* player, uint32 maxSpells) : UnitAI(player), m_player(player), m_playerSpellActions(maxSpells)
{

}

uint32 PlayerAI::LookupHighestLearnedRank(uint32 spellId)
{
    uint32 ownedRank = 0;
    uint32 higherRank = spellId;
    do
    {
        if (m_player->HasSpell(higherRank))
            ownedRank = higherRank;
        else
            break;
    }
    while (higherRank = sSpellMgr.GetNextSpellInChain(ownedRank));
    return ownedRank;
}

void PlayerAI::AddPlayerSpellAction(uint32 priority, uint32 spellId, std::function<Unit*()> selector)
{
    m_playerSpellActions[priority] = { spellId, selector ? selector : [&]()->Unit* { return m_player->getVictim(); } };
}

void PlayerAI::ExecuteSpells()
{
    for (auto& data : m_playerSpellActions)
        if (Unit* target = data.second())
            DoCastSpellIfCan(target, data.first);
}

void PlayerAI::JustGotCharmed(Unit* charmer)
{
    m_player->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE, true);
    AttackClosestEnemy();
}

void PlayerAI::UpdateAI(const uint32 diff)
{
    // Check if we have a current target
    if (!m_player->SelectHostileTarget() || !m_player->getVictim())
        return;

    ExecuteSpells();

    DoMeleeAttackIfReady();
}