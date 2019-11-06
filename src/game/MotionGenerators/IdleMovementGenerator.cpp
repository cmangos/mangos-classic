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

#include "IdleMovementGenerator.h"
#include "AI/BaseAI/CreatureAI.h"
#include "Entities/Creature.h"

IdleMovementGenerator si_idleMovement;

bool IdleMovementGenerator::Update(Unit&, const uint32&)
{
    return true;
}

void DistractMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_DISTRACTED);
}

void DistractMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_DISTRACTED);
}

void DistractMovementGenerator::Reset(Unit& owner)
{
    Initialize(owner);
}

bool DistractMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    m_timer.Update(diff);
    return (owner.hasUnitState(UNIT_STAT_DISTRACTED) && !m_timer.Passed());
}

