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

#include "PointMovementGenerator.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"
#include "Entities/Creature.h"
#include "Entities/TemporarySpawn.h"
#include "AI/BaseAI/UnitAI.h"

//----- Point Movement Generator

void PointMovementGenerator::Initialize(Unit& unit)
{
    if (unit.hasUnitState(UNIT_STAT_NO_FREE_MOVE | UNIT_STAT_NOT_MOVE))
        return;

    // Stop any previously dispatched splines no matter the source
    if (!unit.movespline->Finalized() && !m_speedChanged)
    {
        if (unit.IsClientControlled())
            unit.StopMoving(true);
        else
            unit.InterruptMoving();
    }

    unit.addUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);

    Move(unit);

    m_speedChanged = false;
}

void PointMovementGenerator::Finalize(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);

    // Stop any previously dispatched splines no matter the source
    if (!unit.movespline->Finalized())
    {
        if (unit.IsClientControlled())
            unit.StopMoving(true);
        else
            unit.InterruptMoving();
    }
    else
        MovementInform(unit);
}

void PointMovementGenerator::Interrupt(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    unit.InterruptMoving();
}

void PointMovementGenerator::Reset(Unit& unit)
{
    unit.addUnitState(UNIT_STAT_ROAMING);
    Initialize(unit);
}

bool PointMovementGenerator::Update(Unit& unit, const uint32&/* diff*/)
{
    if (unit.hasUnitState(UNIT_STAT_CAN_NOT_MOVE))
    {
        unit.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    if ((!unit.hasUnitState(UNIT_STAT_ROAMING_MOVE) && unit.movespline->Finalized()) || m_speedChanged)
        Initialize(unit);

    return !unit.movespline->Finalized();
}

void PointMovementGenerator::Move(Unit& unit)
{
    Movement::MoveSplineInit init(unit);
    init.MoveTo(m_x, m_y, m_z, m_generatePath);
    if (m_forcedMovement == FORCED_MOVEMENT_WALK)
        init.SetWalk(true);
    else if (m_forcedMovement == FORCED_MOVEMENT_RUN)
        init.SetWalk(false);
    else
        init.SetWalk(!unit.hasUnitState(UNIT_STAT_RUNNING));
    if (m_forcedMovement == FORCED_MOVEMENT_FLIGHT)
        init.SetFly();
    if (m_o != 0.f)
        init.SetFacing(m_o);
    init.SetVelocity(m_speed);
    init.Launch();
}

void PointMovementGenerator::MovementInform(Unit& unit)
{
    MovementGeneratorType const type = GetMovementGeneratorType();

    if (UnitAI* ai = unit.AI())
        ai->MovementInform(type, m_id);

    if (unit.GetTypeId() == TYPEID_UNIT && static_cast<Creature&>(unit).IsTemporarySummon())
    {
        if (unit.GetSpawnerGuid().IsCreatureOrPet())
            if (Creature* pSummoner = unit.GetMap()->GetAnyTypeCreature(unit.GetSpawnerGuid()))
            {
                if (UnitAI* ai = pSummoner->AI())
                    ai->SummonedMovementInform(static_cast<Creature*>(&unit), type, m_id);
            }
    }

    if (m_relayId)
        unit.GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, m_relayId, &unit, m_guid ? unit.GetMap()->GetWorldObject(m_guid) : nullptr);
}

void RetreatMovementGenerator::Initialize(Unit& unit)
{
    // Non-client controlled unit with an AI should drop target
    if (unit.AI() && !unit.IsClientControlled())
    {
        unit.SetTarget(nullptr);
        // FIXME: do we need to send attack stop here? verify protocol if possible
    }

    unit.addUnitState(UNIT_STAT_RETREATING);

    if (!m_arrived)
        PointMovementGenerator::Initialize(unit);
}

void RetreatMovementGenerator::Finalize(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_RETREATING);

    PointMovementGenerator::Finalize(unit);

    if (UnitAI* ai = unit.AI())
        ai->RetreatingEnded();
}

void RetreatMovementGenerator::Interrupt(Unit& unit)
{
    PointMovementGenerator::Interrupt(unit);

    if (UnitAI* ai = unit.AI())
        ai->RetreatingEnded();
}

void RetreatMovementGenerator::Reset(Unit& unit)
{
    if (!m_arrived)
        PointMovementGenerator::Reset(unit);
}

bool RetreatMovementGenerator::Update(Unit& unit, const uint32& diff)
{
    if (!PointMovementGenerator::Update(unit, diff))
    {
        if (!m_arrived)
        {
            m_arrived = true;

            if (UnitAI* ai = unit.AI())
                ai->RetreatingArrived();
        }

        m_delayTimer.Update(diff);
        return !m_delayTimer.Passed();
    }

    return true;
}

void StayMovementGenerator::Initialize(Unit &unit)
{
    unit.addUnitState(UNIT_STAT_STAY);
    PointMovementGenerator::Initialize(unit);
}

void StayMovementGenerator::Finalize(Unit &unit)
{
    unit.clearUnitState(UNIT_STAT_STAY);
    PointMovementGenerator::Finalize(unit);
}

void StayMovementGenerator::Interrupt(Unit& unit)
{
    m_arrived = false;
    PointMovementGenerator::Interrupt(unit);
}

bool StayMovementGenerator::Update(Unit& unit, const uint32& diff)
{
    if (!m_arrived && !PointMovementGenerator::Update(unit, diff))
    {
        unit.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        m_arrived = true;
    }
    return true;
}

void PointTOLMovementGenerator::Move(Unit& unit)
{
    Movement::MoveSplineInit init(unit);
    init.MoveTo(m_x, m_y, m_z, false);
    if (m_forcedMovement == FORCED_MOVEMENT_WALK)
        init.SetWalk(true);
    else
        init.SetWalk(!unit.hasUnitState(UNIT_STAT_RUNNING));
    if (m_o != 0.f)
        init.SetFacing(m_o);
    init.SetVelocity(m_speed);
    init.SetFly();
    init.SetWalk(!unit.hasUnitState(UNIT_STAT_RUNNING));
    init.Launch();
}
