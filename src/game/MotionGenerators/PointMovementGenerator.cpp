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
#include "Entities/Creature.h"
#include "AI/BaseAI/UnitAI.h"
#include "Entities/TemporarySpawn.h"
#include "World/World.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"

//----- Point Movement Generator
template<class T>
void PointMovementGenerator<T>::Initialize(T& unit)
{
    if (unit.hasUnitState(UNIT_STAT_NO_FREE_MOVE | UNIT_STAT_NOT_MOVE))
        return;

    unit.StopMoving();

    unit.addUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);

    Movement::MoveSplineInit init(unit);
    init.MoveTo(i_x, i_y, i_z, m_generatePath);
    if (m_forcedMovement == FORCED_MOVEMENT_WALK)
        init.SetWalk(true);
    if (i_o != 0.f)
        init.SetFacing(i_o);
    init.SetVelocity(i_speed);
    init.Launch();

    m_speedChanged = false;
}

template<class T>
void PointMovementGenerator<T>::Finalize(T& unit)
{
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);

    if (unit.movespline->Finalized())
        MovementInform(unit);
}

template<class T>
void PointMovementGenerator<T>::Interrupt(T& unit)
{
    unit.InterruptMoving();
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
}

template<class T>
void PointMovementGenerator<T>::Reset(T& unit)
{
    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
}

template<class T>
bool PointMovementGenerator<T>::Update(T& unit, const uint32& /*diff*/)
{
    if (unit.hasUnitState(UNIT_STAT_CAN_NOT_MOVE))
    {
        unit.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    if ((!unit.hasUnitState(UNIT_STAT_ROAMING_MOVE) && unit.movespline->Finalized()) || this->m_speedChanged)
        Initialize(unit);

    return !unit.movespline->Finalized();
}

template<>
void PointMovementGenerator<Player>::MovementInform(Player&)
{
}

template <>
void PointMovementGenerator<Creature>::MovementInform(Creature& unit)
{
    MovementGeneratorType const type = GetMovementGeneratorType();
    if (unit.AI())
        unit.AI()->MovementInform(type, m_id);

    if (unit.IsTemporarySummon())
    {
        if (unit.GetSpawnerGuid().IsCreatureOrPet())
            if (Creature* pSummoner = unit.GetMap()->GetAnyTypeCreature(unit.GetSpawnerGuid()))
                if (pSummoner->AI())
                    pSummoner->AI()->SummonedMovementInform(&unit, type, m_id);
    }
}

template void PointMovementGenerator<Player>::Initialize(Player&);
template void PointMovementGenerator<Creature>::Initialize(Creature&);
template void PointMovementGenerator<Player>::Finalize(Player&);
template void PointMovementGenerator<Creature>::Finalize(Creature&);
template void PointMovementGenerator<Player>::Interrupt(Player&);
template void PointMovementGenerator<Creature>::Interrupt(Creature&);
template void PointMovementGenerator<Player>::Reset(Player&);
template void PointMovementGenerator<Creature>::Reset(Creature&);
template bool PointMovementGenerator<Player>::Update(Player&, const uint32& diff);
template bool PointMovementGenerator<Creature>::Update(Creature&, const uint32& diff);

void RetreatMovementGenerator::Initialize(Creature& unit)
{
    if (m_arrived)
        return;

    // Non-client controlled unit with an AI should drop target
    if (unit.AI() && !unit.IsClientControlled())
    {
        unit.SetTarget(nullptr);
        // FIXME: do we need to send attack stop here? verify protocol if possible
    }

    unit.addUnitState(UNIT_STAT_RETREATING);

    PointMovementGenerator::Initialize(unit);

    m_delayTimer.Reset(sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_ASSISTANCE_DELAY));
}

void RetreatMovementGenerator::Finalize(Creature& unit)
{
    unit.clearUnitState(UNIT_STAT_RETREATING);

    PointMovementGenerator::Finalize(unit);

    if (UnitAI* ai = unit.AI())
        ai->RetreatingEnded();
}

void RetreatMovementGenerator::Interrupt(Creature &unit)
{
    PointMovementGenerator::Interrupt(unit);

    if (UnitAI* ai = unit.AI())
        ai->RetreatingEnded();
}

bool RetreatMovementGenerator::Update(Creature& unit, const uint32& diff)
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

void FlyOrLandMovementGenerator::Initialize(Creature& unit)
{
    if (unit.hasUnitState(UNIT_STAT_NO_FREE_MOVE | UNIT_STAT_NOT_MOVE))
        return;

    unit.StopMoving();

    unit.addUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    Movement::MoveSplineInit init(unit);
    init.SetFly();
    init.MoveTo(i_x, i_y, i_z, false);
    init.Launch();
}
