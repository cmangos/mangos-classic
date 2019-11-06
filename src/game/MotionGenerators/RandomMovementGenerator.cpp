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

#include "Entities/Creature.h"
#include "Entities/Player.h"
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"
#include "MotionGenerators/RandomMovementGenerator.h"

void AbstractRandomMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(i_stateActive);

    // Client-controlled unit should have control removed
    if (const Player* controllingClientPlayer = owner.GetClientControlling())
        controllingClientPlayer->UpdateClientControl(&owner, false);
    // Non-client controlled unit with an AI should drop target
    else if (owner.AI())
    {
        owner.SetTarget(nullptr);
        owner.SendMeleeAttackStop(owner.getVictim());
    }

    // Stop any previously dispatched splines no matter the source
    if (!owner.movespline->Finalized())
    {
        if (owner.IsClientControlled())
            owner.StopMoving(true);
        else
            owner.InterruptMoving();
    }
}

void AbstractRandomMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(i_stateActive | i_stateMotion);

    // Client-controlled unit should have control restored
    if (const Player* controllingClientPlayer = owner.GetClientControlling())
        controllingClientPlayer->UpdateClientControl(&owner, true);

    // Stop any previously dispatched splines no matter the source
    if (!owner.movespline->Finalized())
    {
        if (owner.IsClientControlled())
            owner.StopMoving(true);
        else
            owner.InterruptMoving();
    }
}

void AbstractRandomMovementGenerator::Interrupt(Unit& owner)
{
    owner.InterruptMoving();

    owner.clearUnitState(i_stateMotion);
}

void AbstractRandomMovementGenerator::Reset(Unit& owner)
{
    i_nextMoveTimer.Reset(0);

    Initialize(owner);
}

bool AbstractRandomMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    if (!owner.isAlive())
        return false;

    if (owner.hasUnitState(UNIT_STAT_NO_FREE_MOVE & ~i_stateActive))
    {
        i_nextMoveTimer.Update(diff);
        owner.clearUnitState(i_stateMotion);
        return true;
    }

    if (owner.movespline->Finalized())
    {
        i_nextMoveTimer.Update(diff);

        if (i_nextMoveTimer.Passed())
        {
            if (_setLocation(owner))
            {
                if (i_nextMoveCount > 1)
                    --i_nextMoveCount;
                else
                {
                    i_nextMoveCount = urand(1, i_nextMoveCountMax);
                    i_nextMoveTimer.Reset(urand(i_nextMoveDelayMin, i_nextMoveDelayMax));
                }
            }
            else
                i_nextMoveTimer.Reset(250);
        }
    }

    return true;
}

bool AbstractRandomMovementGenerator::_getLocation(Unit& owner, float& x, float& y, float& z)
{
    return owner.GetMap()->GetReachableRandomPosition(&owner, x, y, z, i_radius);
}

int32 AbstractRandomMovementGenerator::_setLocation(Unit& owner)
{
    // Look for a random location within certain radius of initial position
    float x = i_x, y = i_y, z = i_z;

    // Require destination to be in LoS for PC units for this movegen hierarchy
    if (!_getLocation(owner, x, y, z) || (owner.HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && !owner.IsWithinLOS(x, y, z)))
        return 0;

    PathFinder pf(&owner);
    if (i_pathLength != 0.0f)
        pf.setPathLengthLimit(i_pathLength);

    pf.calculate(x, y, z);

    if (pf.getPathType() & PATHFIND_NOPATH)
        return 0;

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(pf.getPath());
    init.SetWalk(i_walk);

    int32 duration = init.Launch();

    if (duration)
        owner.addUnitState(i_stateMotion);

    return duration;
}

ConfusedMovementGenerator::ConfusedMovementGenerator(float x, float y, float z) :
    AbstractRandomMovementGenerator(UNIT_STAT_CONFUSED, UNIT_STAT_CONFUSED_MOVE, 500, 1500)
{
    i_radius = 10.0f;
    i_x = x;
    i_y = y;
    i_z = z;
}

ConfusedMovementGenerator::ConfusedMovementGenerator(const Unit& owner) :
    ConfusedMovementGenerator(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ())
{
}

WanderMovementGenerator::WanderMovementGenerator(float x, float y, float z, float radius, float verticalZ) :
    AbstractRandomMovementGenerator(UNIT_STAT_ROAMING, UNIT_STAT_ROAMING_MOVE, 3000, 10000, 3)
{
    i_x = x;
    i_y = y;
    i_z = z;
    i_radius = radius;
    i_verticalZ = verticalZ;
}

WanderMovementGenerator::WanderMovementGenerator(const Creature& npc) :
    AbstractRandomMovementGenerator(UNIT_STAT_ROAMING, UNIT_STAT_ROAMING_MOVE, 3000, 10000, 3)
{
    npc.GetRespawnCoord(i_x, i_y, i_z, nullptr, &i_radius);
}

void WanderMovementGenerator::Finalize(Unit& owner)
{
    AbstractRandomMovementGenerator::Finalize(owner);

    if (owner.GetTypeId() == TYPEID_UNIT)
        static_cast<Creature&>(owner).SetWalk(!owner.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

void WanderMovementGenerator::Interrupt(Unit& owner)
{
    AbstractRandomMovementGenerator::Interrupt(owner);

    if (owner.GetTypeId() == TYPEID_UNIT)
        static_cast<Creature&>(owner).SetWalk(!owner.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

FleeingMovementGenerator::FleeingMovementGenerator(Unit const& source) :
    AbstractRandomMovementGenerator(UNIT_STAT_FLEEING, UNIT_STAT_FLEEING_MOVE, 500, 1500)
{
    source.GetPosition(i_x, i_y, i_z);
    i_pathLength = 30;
    i_walk = false;
}

#define MIN_QUIET_DISTANCE 28.0f
#define MAX_QUIET_DISTANCE 43.0f

bool FleeingMovementGenerator::_getLocation(Unit& owner, float& x, float& y, float& z)
{
    float dist_from_source = owner.GetDistance(i_x, i_y, i_z);

    if (dist_from_source < MIN_QUIET_DISTANCE)
        i_radius = frand(0.4f, 1.3f) * (MIN_QUIET_DISTANCE - dist_from_source);
    else if (dist_from_source > MAX_QUIET_DISTANCE)
        i_radius = frand(0.4f, 1.0f) * (MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);
    else    // we are inside quiet range
        i_radius = frand(0.6f, 1.2f) * (MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);

    owner.GetPosition(x, y, z);

    return owner.GetMap()->GetReachableRandomPosition(&owner, x, y, z, i_radius, false);
}

void PanicMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_PANIC);

    FleeingMovementGenerator::Initialize(owner);
}

void PanicMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_PANIC);

    // Since two fleeing mmgens are mutually exclusive, we are also responsible for the removal of that flag, nobody will clear this for us
    owner.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

    FleeingMovementGenerator::Finalize(owner);

    if (owner.AI())
        owner.AI()->TimedFleeingEnded();
}

void PanicMovementGenerator::Interrupt(Unit& owner)
{
    FleeingMovementGenerator::Interrupt(owner);

    if (owner.AI())
        owner.AI()->TimedFleeingEnded();
}

bool PanicMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    m_fleeingTimer.Update(diff);
    if (m_fleeingTimer.Passed())
        return false;

    return FleeingMovementGenerator::Update(owner, diff);
}
