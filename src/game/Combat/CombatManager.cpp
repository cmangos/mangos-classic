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

#include "CombatManager.h"

#include "Entities/Pet.h"
#include "Maps/Map.h"

CombatManager::CombatManager(Unit* owner) : m_owner(owner), m_evadeTimer(0), m_evadeState(EVADE_NONE), m_combatTimer(0), m_leashingDisabled(false), m_leashingCheck(nullptr)
{

}

/*
    Creature Evade Timer & Leash range

    1) Leash timer 15 sec if creature can reach you
    - Timer will be reset upon if player deals damage or if creature does damage to player
    - Only direct damage will reset damage

    2) There is no timer for creature that cannot reach you
    - Creature appears to be evading while it cannot reach you so player cannot deal any damage
    - After 90yd it will drop combat because player is too far.

    3) Special creatures have leash range and timer
    - No way to find out range as of now on WoD
*/

void CombatManager::Update(const uint32 diff)
{
    if (m_owner->isInCombat())
    {
        if (m_evadeTimer)
        {
            if (m_evadeTimer <= diff)
            {
                m_evadeTimer = 0;
                m_owner->EvadeTimerExpired();
            }
            else
                m_evadeTimer -= diff;
        }

        if (!m_owner->GetMap()->IsDungeon() || m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            if (m_combatTimer)
            {
                if (m_combatTimer <= diff)
                    m_combatTimer = 0;
                else
                    m_combatTimer -= diff;
            }
            else
            {
                bool check = !m_owner->HasMaster();
                if (!check)
                {
                    Unit* master = m_owner->GetMaster();
                    if (!master || !master->isAlive()) // if charmer alive, he will evade this charm
                        check = true;
                }
                if (check)
                {
                    if (m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    {
                        if (m_owner->getHostileRefManager().getSize() == 0)
                            m_owner->HandleExitCombat();
                    }
                    else // if timer ran out and we are far away from homebox, evade
                    {
                        Creature* creatureOwner = static_cast<Creature*>(m_owner);
                        float x, y, z, o;
                        creatureOwner->GetCombatStartPosition(x, y, z, o);
                        // homebox not confirmed on classic
                        if (creatureOwner->GetDistance2d(x, y) > 30.0f)
                            creatureOwner->HandleExitCombat();
                    }
                }
            }
        }
        if (m_owner->IsCreature() && !m_owner->HasCharmer()) // charmer should have leashing check or leash set
        {
            Creature* creatureOwner = static_cast<Creature*>(m_owner);
            // If creature is within 30yd from combat start do not exit combat
            if (!creatureOwner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            {
                if (m_leashingCheck)
                {
                    float x, y, z;
                    creatureOwner->GetPosition(x, y, z);
                    if (m_leashingCheck(creatureOwner, x, y, z))
                        creatureOwner->HandleExitCombat();
                }
                else if (creatureOwner->GetCreatureInfo()->Leash) // If creature has set maximum leashing distance
                {
                    float x, y, z, o;
                    creatureOwner->GetCombatStartPosition(x, y, z, o);
                    if (creatureOwner->GetDistance2d(x, y) > creatureOwner->GetCreatureInfo()->Leash)
                        creatureOwner->HandleExitCombat();
                }
            }
        }
    }
}

void CombatManager::StopEvade()
{
    if (m_evadeTimer)
    {
        m_evadeTimer = 0;
        return;
    }
    SetEvadeState(EVADE_NONE);
}

struct SetEvadeHelper
{
    explicit SetEvadeHelper(EvadeState _state) : state(_state) {}
    void operator()(Unit* unit) const { unit->GetCombatManager().SetEvadeState(state); }
    EvadeState state;
};

void CombatManager::SetEvadeState(EvadeState state)
{
    if (m_evadeState == state)
        return;

    if (state == EVADE_NONE && m_evadeState == EVADE_HOME)
        m_owner->AI()->SetAIOrder(ORDER_NONE);
    else if (state == EVADE_HOME)
        m_owner->AI()->SetAIOrder(ORDER_EVADE);
    m_evadeState = state;

    // Do not propagate during charm
    if (!m_owner->HasCharmer())
        m_owner->CallForAllControlledUnits(SetEvadeHelper(state), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

void CombatManager::TriggerCombatTimer(Unit* target)
{
    if (target->CanHaveThreatList()) // From attackers PoV if we have victim and victim can have a threat list then set his timer
        target->GetCombatManager().TriggerCombatTimer(target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));
}

void CombatManager::TriggerCombatTimer(bool pvp)
{
    m_combatTimer = pvp ? 5000 : 15000;
}
