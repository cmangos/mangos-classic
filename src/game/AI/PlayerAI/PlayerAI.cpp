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
#include <limits>

enum GenericPlayerAIActions
{
    GENERIC_ACTION_RESET = 1000,
};

PlayerAI::PlayerAI(Player* player) : UnitAI(player), m_player(player), m_spellsDisabled(false)
{
    AddCustomAction(GENERIC_ACTION_RESET, true, [&]() { m_spellsDisabled = false; });
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
    while ((higherRank = sSpellMgr.GetNextSpellInChain(ownedRank)));
    return ownedRank;
}

void PlayerAI::AddPlayerSpellAction(uint32 priority, uint32 spellId, std::function<Unit*()> selector)
{
    m_playerSpellActions.emplace_back(spellId, (selector ? selector : [&]()->Unit* { return m_player->GetVictim(); }));
}

void PlayerAI::ExecuteSpells()
{
    if (m_spellsDisabled)
        return;

    bool success = false;
    for (auto& data : m_playerSpellActions)
        if (Unit* target = data.targetFinder())
            if (DoCastSpellIfCan(target, data.spellId) == CAST_OK)
                success = true;

    if (success)
    {
        m_spellsDisabled = true;
        ResetTimer(GENERIC_ACTION_RESET, urand(5000, 10000));
    }
}

void PlayerAI::JustGotCharmed(Unit* charmer)
{
    m_player->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE, true);
    AttackClosestEnemy();
}

void PlayerAI::EnterEvadeMode()
{
    m_player->CombatStopWithPets(true);
    if (Unit* charmer = m_player->GetCharmer())
        m_player->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE, true);
}

void PlayerAI::AttackClosestEnemy()
{
    float distance = std::numeric_limits<float>::max();
    AttackSpecificEnemy([&](Unit* enemy, Unit*& closestEnemy) mutable
    {
        float curDistance = enemy->GetDistance(m_unit, true, DIST_CALC_NONE);
        if (!closestEnemy || (!closestEnemy->IsPlayer() && enemy->IsPlayer()) || curDistance < distance)
        {
            closestEnemy = enemy;
            distance = curDistance;
        }
    });
}

void PlayerAI::UpdateAI(const uint32 diff)
{
    UpdateTimers(diff);

    // Check if we have a current target
    if (!m_player->SelectHostileTarget() || !m_player->GetVictim())
        return;

    ExecuteSpells();

    DoMeleeAttackIfReady();
}