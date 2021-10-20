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
    GENERIC_THREAT_CHANGE = 1001,
};

PlayerAI::PlayerAI(Player* player) : UnitAI(player), m_player(player), m_spellsDisabled(false)
{
    AddCustomAction(GENERIC_ACTION_RESET, true, [&]() { m_spellsDisabled = false; });
    AddCustomAction(GENERIC_THREAT_CHANGE, true, [&]() { m_executeTargetChange = true; });
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

void PlayerAI::AddPlayerSpellAction(uint32 spellId, std::function<Unit*()> selector)
{
    if (!selector)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (spellInfo)
        {
            if (IsNextMeleeSwingSpell(spellInfo))
                selector = [&]()->Unit* { return m_player->GetVictim(); };
            else if (HasSpellTarget(spellInfo, TARGET_UNIT_ENEMY) || (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)) // always random
                selector = [&, spellId = spellInfo->Id]()->Unit* { return m_player->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, spellId, SELECT_FLAG_PLAYER); };
            if (HasSpellTarget(spellInfo, TARGET_UNIT_FRIEND) || HasSpellTarget(spellInfo, TARGET_UNIT_FRIEND_CHAIN_HEAL)) // heals only target self
                selector = [&]()->Unit* { return m_player; };
        }
        if (!selector) // fallback
            selector = [&]()->Unit* { return nullptr; };
    }
    m_playerSpellActions.emplace_back(spellId, selector);
}

void PlayerAI::ExecuteSpells()
{
    if (m_executeTargetChange && CanExecuteCombatAction())
    {
        if (Unit* target = m_player->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
            AttackStart(target);

        ResetTimer(GENERIC_THREAT_CHANGE, urand(10000, 20000));
        m_executeTargetChange = false;
    }

    if (m_spellsDisabled)
        return;

    std::shuffle(m_playerSpellActions.begin(), m_playerSpellActions.end(), *GetRandomGenerator());

    bool success = false;
    for (auto& data : m_playerSpellActions)
        if (DoCastSpellIfCan(data.targetFinder(), data.spellId) == CAST_OK)
            success = true;            

    if (success)
    {
        uint32 randomInterval = urand(0, 5);
        if (randomInterval != 0)
        {
            m_spellsDisabled = true;
            ResetTimer(GENERIC_ACTION_RESET, urand(6000, 12000));
        }
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
