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

#include "ConfusedMovementGenerator.h"
#include "Creature.h"
#include "Player.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

template<class T>
void ConfusedMovementGenerator<T>::Initialize(T& unit)
{
    unit.addUnitState(UNIT_STAT_CONFUSED);

    // set initial position
    unit.GetPosition(i_x, i_y, i_z);

    if (!unit.isAlive() || unit.hasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED_MOVE);
}

template<class T>
void ConfusedMovementGenerator<T>::Interrupt(T& unit)
{
    unit.InterruptMoving();
    // confused state still applied while movegen disabled
    unit.clearUnitState(UNIT_STAT_CONFUSED_MOVE);
}

template<class T>
void ConfusedMovementGenerator<T>::Reset(T& unit)
{
    i_nextMoveTime.Reset(0);

    if (!unit.isAlive() || unit.hasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_CONFUSED_MOVE);
}

template<class T>
bool ConfusedMovementGenerator<T>::Update(T& unit, const uint32& diff)
{
    // ignore in case other no reaction state
    if (unit.hasUnitState(UNIT_STAT_CAN_NOT_REACT & ~UNIT_STAT_CONFUSED))
        return true;

    if (i_nextMoveTime.Passed())
    {
        // currently moving, update location
        unit.addUnitState(UNIT_STAT_CONFUSED_MOVE);

        if (unit.movespline->Finalized())
            i_nextMoveTime.Reset(urand(800, 1500));
    }
    else
    {
        // waiting for next move
        i_nextMoveTime.Update(diff);
        if (i_nextMoveTime.Passed())
        {
            // start moving
            unit.addUnitState(UNIT_STAT_CONFUSED_MOVE);

            float destX = i_x;
            float destY = i_y;
            float destZ = i_z;

            // check if new random position is assigned, GetReachableRandomPosition may fail
            if (unit.GetMap()->GetReachableRandomPosition(&unit, destX, destY, destZ, 10.0f))
            {
                Movement::MoveSplineInit init(unit);
                init.MoveTo(destX, destY, destZ, true);
                init.SetWalk(true);
                init.Launch();
                i_nextMoveTime.Reset(urand(800, 1000));             // Keep a short wait time
            }
            else
                i_nextMoveTime.Reset(50);                           // Retry later
        }
    }

    return true;
}

template<>
void ConfusedMovementGenerator<Player>::Finalize(Player& unit)
{
    unit.clearUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_CONFUSED_MOVE);
    unit.StopMoving(true);
}

template<>
void ConfusedMovementGenerator<Creature>::Finalize(Creature& unit)
{
    unit.clearUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_CONFUSED_MOVE);
}

template void ConfusedMovementGenerator<Player>::Initialize(Player& player);
template void ConfusedMovementGenerator<Creature>::Initialize(Creature& creature);
template void ConfusedMovementGenerator<Player>::Interrupt(Player& player);
template void ConfusedMovementGenerator<Creature>::Interrupt(Creature& creature);
template void ConfusedMovementGenerator<Player>::Reset(Player& player);
template void ConfusedMovementGenerator<Creature>::Reset(Creature& creature);
template bool ConfusedMovementGenerator<Player>::Update(Player& player, const uint32& diff);
template bool ConfusedMovementGenerator<Creature>::Update(Creature& creature, const uint32& diff);
