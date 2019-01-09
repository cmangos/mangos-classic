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

#include "GuardianAI.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

GuardianAI::GuardianAI(Creature* creature) : CreatureEventAI(creature)
{
    Unit* owner = creature->GetOwner();
    MANGOS_ASSERT(owner);
    m_defaultMovement = FOLLOW_MOTION_TYPE;
}

void GuardianAI::JustRespawned()
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return;

    CreatureEventAI::JustRespawned();

    if (GetDefaultMovement() == FOLLOW_MOTION_TYPE)
        m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
}

void GuardianAI::UpdateAI(const uint32 diff)
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return;

    switch (m_reactState)
    {
        case REACT_AGGRESSIVE:
        case REACT_DEFENSIVE:
            if (!m_creature->isInCombat() && owner->isInCombat() && !(m_creature->IsPet() && ((Pet*)m_creature)->getPetType() == MINI_PET))
                AttackStart(owner->getAttackerForHelper());   // check for getAttackerForHelper() == nullpter in AttackStart()
            break;
        default:
            break;
    }

    CreatureEventAI::UpdateAI(diff);
}

int GuardianAI::Permissible(const Creature* creature)
{
    if (creature->IsPet() && ((Pet*)creature)->getPetType() == GUARDIAN_PET)
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void GuardianAI::CombatStop()
{
    Unit* owner = m_creature->GetOwner();

    if (!owner || !m_creature->IsInWorld())
        return;

    // only alive creatures that are not on transport can return to home position
    if (m_creature->isAlive() && GetDefaultMovement() == FOLLOW_MOTION_TYPE)
        m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
}

void GuardianAI::EnterEvadeMode()
{
    m_creature->RemoveAllAurasOnEvade();
    m_creature->CombatStopWithPets(true);

    m_creature->TriggerEvadeEvents();

    // Handle Evade events
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_EVADE)
            CheckAndReadyEventForExecution(i);
    }
    ProcessEvents();
}
