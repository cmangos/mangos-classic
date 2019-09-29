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

#include "MotionGenerators/TargetedMovementGenerator.h"
#include "PathFinder.h"
#include "Entities/Unit.h"
#include "Entities/Creature.h"
#include "Entities/Player.h"
#include "World/World.h"
#include "Movement/MoveSpline.h"
#include "Maps/MapManager.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

#define IGNORE_M2 true // simple define for avoiding bugs due to different setting across movechase

// Chase-Movement: These factors depend on combat-reach distance
#define CHASE_DEFAULT_RANGE_FACTOR                        0.5f
#define CHASE_RECHASE_RANGE_FACTOR                        0.75f
#define CHASE_MOVE_CLOSER_FACTOR                          0.875f

#define CHASE_CLOSENESS_TIMER                             2000

const char* ChaseModes[] =
{
    "CHASE_MODE_NORMAL",
    "CHASE_MODE_BACKPEDAL",
    "CHASE_MODE_DISTANCING",
    "CHASE_MODE_FANNING",
};

//-----------------------------------------------//
template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::_setTargetLocation(T& owner, bool updateDestination)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    float x, y, z;

    // i_path can be nullptr in case this is the first call for this MMGen (via Update)
    // Can happen for example if no path was created on MMGen-Initialize because of the owner being stunned
    if (updateDestination || !i_path)
    {
        owner.GetPosition(x, y, z);
 
        // prevent redundant micro-movement for pets, other followers.
        if (!RequiresNewPosition(owner, x, y, z))
        {
            if (!owner.movespline->Finalized())
                return;
        }
        // Chase Movement and angle == 0 case: Chase to current angle
        else
        {
            float ori;
            if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
            {
                if (i_angle == 0.f || i_target->getVictim() && i_target->getVictim() == &owner)
                    ori = i_target->GetAngle(&owner); // Need to avoid readjustment when target is attacking owner
                else
                    ori = i_target->GetOrientation() + i_angle;
            }
            else
                ori = i_target->GetOrientation() + i_angle;

            i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), this->GetDynamicTargetDistance(owner, false), ori);
        }
    }
    else
    {
        // the destination has not changed, we just need to refresh the path (usually speed change)
        G3D::Vector3 end = i_path->getEndPosition();
        x = end.x;
        y = end.y;
        z = end.z;
    }

    if (!i_path)
        i_path = new PathFinder(&owner);

    // allow pets following their master to cheat while generating paths
    bool forceDest = (owner.GetTypeId() == TYPEID_UNIT && ((Creature*)&owner)->IsPet()
                      && owner.hasUnitState(UNIT_STAT_FOLLOW));
    i_path->calculate(x, y, z, forceDest);
    if (i_path->getPathType() & PATHFIND_NOPATH)
        return;

    auto& path = i_path->getPath();

    if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        if (float offset = this->i_offset) // need to cut path until most distant viable point
        {
            float dist = this->i_offset * CHASE_MOVE_CLOSER_FACTOR + CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner);
            float tarX, tarY, tarZ;
            i_target->GetPosition(tarX, tarY, tarZ);
            auto iter = std::next(path.begin(), 1); // need to start at index 1, index 0 is start position and filled in init.Launch()
            for (; iter != path.end(); ++iter)
            {
                G3D::Vector3 data = (*iter);
                if (!i_target->IsWithinDist3d(data.x, data.y, data.z, dist))
                    continue;
                if (!owner.GetMap()->IsInLineOfSight(tarX, tarY, tarZ + 2.0f, data.x, data.y, data.z + 2.0f, IGNORE_M2))
                    continue;
                // both in LOS and in range - advance to next and stop
                ++iter;
                break;
            }
            if (iter != path.end())
                path.erase(iter, path.end());
        }
    }

    D::_addUnitStateMove(owner);
    i_targetReached = false;
    i_speedChanged = false;

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(path);
    init.SetWalk(((D*)this)->EnableWalking());
    init.Launch();
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::Update(T& owner, const uint32& time_diff)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return false;

    if (!owner.isAlive())
        return true;

    // prevent movement while casting spells with cast time or channel time
    if (owner.IsNonMeleeSpellCasted(false, false, true, true))
    {
        if (!owner.IsStopped())
            owner.StopMoving();
        return true;
    }

    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
    {
        HandleMovementFailure(owner);
        return true;
    }

    if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE && owner.hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT))
    {
        HandleMovementFailure(owner);
        return true;
    }

    // prevent crash after creature killed pet
    if (static_cast<D*>(this)->_lostTarget(owner))
    {
        HandleMovementFailure(owner);
        return true;
    }

    HandleTargetedMovement(owner, time_diff);

    if (owner.movespline->Finalized() && !i_targetReached)
        HandleFinalizedMovement(owner);

    return true;
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::IsReachable() const
{
    return (i_path) ? (i_path->getPathType() & PATHFIND_NORMAL) : true;
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::RequiresNewPosition(T& owner, float x, float y, float z) const
{
    float dist = this->GetDynamicTargetDistance(owner, true);
    // More distance let have better performance, less distance let have more sensitive reaction at target move.
    return i_target->GetDistance(x, y, z, DIST_CALC_NONE) > dist * dist;
}

//-----------------------------------------------//
void ChaseMovementGenerator::_clearUnitStateMove(Unit& u) { u.clearUnitState(UNIT_STAT_CHASE_MOVE); }
void ChaseMovementGenerator::_addUnitStateMove(Unit& u) { u.addUnitState(UNIT_STAT_CHASE_MOVE); }

bool ChaseMovementGenerator::_lostTarget(Unit& u) const
{
    return m_combat && u.getVictim() != this->GetCurrentTarget();
}

void ChaseMovementGenerator::_reachTarget(Unit& /*owner*/)
{

}

void ChaseMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_CHASE);                    // _MOVE set in _SetTargetLocation after required checks
    _setTargetLocation(owner, true);
    i_target->GetPosition(i_lastTargetPos.x, i_lastTargetPos.y, i_lastTargetPos.z);
}

void ChaseMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_CHASE | UNIT_STAT_CHASE_MOVE);
    if (m_currentMode == CHASE_MODE_DISTANCING) // cleanup in case fanning was removed
        owner.AI()->DistancingEnded();
}

void ChaseMovementGenerator::Interrupt(Unit& owner)
{
    owner.InterruptMoving();
    owner.clearUnitState(UNIT_STAT_CHASE_MOVE);
    if (m_currentMode == CHASE_MODE_DISTANCING)
        owner.AI()->UnitAI::DistancingEnded(); // just remove combat script status
}

void ChaseMovementGenerator::Reset(Unit& owner)
{
    Initialize(owner);
}

void ChaseMovementGenerator::SetOffsetAndAngle(float offset, float angle, bool moveFurther)
{
    i_offset = offset;
    i_angle = angle;
    m_moveFurther = moveFurther;
}

float ChaseMovementGenerator::GetDynamicTargetDistance(Unit& owner, bool forRangeCheck) const
{
    if (m_moveFurther)
    {
        if (!forRangeCheck)
            return this->i_offset + CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, true) ;

        return this->i_offset + CHASE_RECHASE_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, true);
    }
    else
    {
        if (!forRangeCheck) // move to melee range and then cut path to simulate retail
            return CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, true);

        // check against actual max range setting
        return this->i_offset + this->i_target->GetCombinedCombatReach(&owner, this->i_offset == 0.f ? true : false);
    }
}

void ChaseMovementGenerator::HandleTargetedMovement(Unit& owner, const uint32& time_diff)
{
    bool targetMoved = false;
    G3D::Vector3 currentTargetPos;
    this->i_target->GetPosition(currentTargetPos.x, currentTargetPos.y, currentTargetPos.z);
    this->i_recheckDistance.Update(time_diff);
    if (m_closenessAndFanningTimer) // here because always need to update timer, cant reuse the timer class because we need a disablable timer
    {
        if (m_closenessAndFanningTimer <= time_diff)
        {
            m_closenessAndFanningTimer = 0;
            m_closenessExpired = true;
        }
        else m_closenessAndFanningTimer -= time_diff;
    }
    if (!this->i_recheckDistance.Passed())
        return;

    this->i_recheckDistance.Reset(250);
    G3D::Vector3 dest = owner.movespline->FinalDestination();
    if (m_currentMode != CHASE_MODE_DISTANCING)
    {
        targetMoved = this->RequiresNewPosition(owner, dest.x, dest.y, dest.z);

        if (this->i_speedChanged || targetMoved)
        {
            float x, y, z;

            // i_path can be nullptr in case this is the first call for this MMGen (via Update)
            // Can happen for example if no path was created on MMGen-Initialize because of the owner being stunned
            if (targetMoved || !this->i_path)
            {
                float ori;
                if (this->i_angle == 0.f || this->i_target->getVictim() && this->i_target->getVictim() == &owner)
                    ori = this->i_target->GetAngle(&owner); // Need to avoid readjustment when target is attacking owner
                else
                    ori = this->i_target->GetOrientation() + i_angle;

                float dist = this->GetDynamicTargetDistance(owner, false);
                // TODO: This code would also benefit greatly if "Get Target Position in 250 ms" function existed
                // TODO: When target is moving away, we should choose a point that is much much closer to account for it
                this->i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), dist, ori);
            }
            else
            {
                // the destination has not changed, we just need to refresh the path (usually speed change)
                G3D::Vector3 end = this->i_path->getEndPosition();
                x = end.x;
                y = end.y;
                z = end.z;
            }

            if (DispatchSplineToPosition(owner, x, y, z, EnableWalking(), true))
            {
                this->i_targetReached = false;
                this->i_speedChanged = false;
                /* m_prevTargetPos is updated on making new spline (normal and distancing) and also on reaching target
                is used for determining if player moved towards target whilst the spline was going on to stop the spline prematurely
                and prevent it going behind targets back - it will still occur in rare cases due to PF and lag */
                this->i_target->GetPosition(this->i_lastTargetPos.x, this->i_lastTargetPos.y, this->i_lastTargetPos.z);
                m_closenessAndFanningTimer = 0;
                return;
            }
            // if we arrived here something failed in PF dispatch and target is not reachable
            m_reachable = false;
            return;
        }
        else if (!targetMoved) // we do not need new position and we are reachable
            m_reachable = true;
    }
    else if (this->i_speedChanged)
    {
        if (DispatchSplineToPosition(owner, dest.x, dest.y, dest.z, false, false))
        {
            this->i_speedChanged = false;
            return;
        }
    }

    // while spline is engaged we have two cases: Running to target or distancing when target was standing in model
    if (!this->i_targetReached)
    {
        if (owner.movespline->Finalized())
            return;
        else if (m_currentMode == CHASE_MODE_NORMAL)
        {
            // TODO: This code would benefit greatly if "Get Target Position in 250 ms" function existed
            if (currentTargetPos != this->i_lastTargetPos)
            {
                G3D::Vector3 ownerPos;
                owner.GetPosition(ownerPos.x, ownerPos.y, ownerPos.z);
                float distFromDestination = owner.GetDistance(dest.x, dest.y, dest.z, DIST_CALC_NONE);
                float distOwnerFromTarget = this->i_target->GetDistance(ownerPos.x, ownerPos.y, ownerPos.z, DIST_CALC_NONE);
                // Explanation of magic: comparing distances between target and destination makes it so we know when mob is between destination and owner
                // - thats when forcible spline stop is needed
                float targetDist = this->i_target->GetCombinedCombatReach(&owner, this->i_offset == 0.f ? true : false);
                if (distFromDestination > distOwnerFromTarget)
                    if (this->i_target->GetDistance(ownerPos.x, ownerPos.y, ownerPos.z, DIST_CALC_NONE) < targetDist * targetDist)
                        owner.StopMoving(true);
            }
        }
    }
    else
    {
        // When creatures use backpedaling, they are caught in an endless cycle of it, its not critical, since they arrive at each other with precision anyhow
        if (this->i_target->GetTypeId() != TYPEID_PLAYER)
            return;

        if (m_closenessExpired)
            Backpedal(owner);
    }
}

void ChaseMovementGenerator::HandleMovementFailure(Unit& owner)
{
    if (m_currentMode == CHASE_MODE_DISTANCING)
        owner.AI()->DistancingEnded();
    m_currentMode = CHASE_MODE_NORMAL;
    _clearUnitStateMove(owner);
}

void ChaseMovementGenerator::HandleFinalizedMovement(Unit& owner)
{
    this->i_targetReached = true;
    if (!owner.HasInArc(this->i_target.getTarget(), 0.01f))
        owner.SetInFront(this->i_target.getTarget());
    this->m_closenessAndFanningTimer = CHASE_CLOSENESS_TIMER;
    this->i_target->GetPosition(this->i_lastTargetPos.x, this->i_lastTargetPos.y, this->i_lastTargetPos.z);
    switch (m_currentMode)
    {
        case CHASE_MODE_NORMAL:
        case CHASE_MODE_BACKPEDAL:
        case CHASE_MODE_FANNING:
        {
            ChaseMovementGenerator::_reachTarget(owner);
            break;
        }
        case CHASE_MODE_DISTANCING:
        {
            owner.AI()->DistancingEnded();
            break;
        }
    }
    m_currentMode = CHASE_MODE_NORMAL;
    m_reachable = true; // just to be absolutely sure clear reachability here - if its unreachable it will reset on next update
}

void ChaseMovementGenerator::DistanceYourself(Unit& owner, float distance)
{
    float x, y, z;
    i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), distance, i_target->GetAngle(&owner));
    if (DispatchSplineToPosition(owner, x, y, z, false, false))
    {
        this->i_targetReached = false;
        m_currentMode = CHASE_MODE_DISTANCING;
        this->i_speedChanged = false;
        owner.AI()->DistancingStarted();
    }
}

void ChaseMovementGenerator::Backpedal(Unit& owner)
{
    m_closenessExpired = false;
    m_closenessAndFanningTimer = CHASE_CLOSENESS_TIMER; // Just in case path doesnt generate
    float targetDist = std::min(this->i_target->GetCombinedCombatReach(&owner, false), 3.75f);
    float x, y, z;
    owner.GetPosition(x, y, z);
    // TODO: add calculation of circle on ground to core and use that
    if (i_target->GetDistance(x, y, z, DIST_CALC_NONE) < (targetDist * targetDist) * 0.33f) // is too close
    {
        float ori = MapManager::NormalizeOrientation(owner.GetOrientation() + M_PI_F);
        i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), targetDist * 0.75f, ori);
    }
    else
    {
        FanOut(owner); // if already on the radius - check fanning
        return;
    }

    if (DispatchSplineToPosition(owner, x, y, z, true, false))
    {
        this->i_targetReached = false;
        m_currentMode = CHASE_MODE_BACKPEDAL;
        this->i_speedChanged = false;
        m_closenessAndFanningTimer = 0; // restart on reaching target
    }
}

const float fanningRadius = 1.f;
const float fanAngleMin = M_PI_F / 5;
const float fanAngleMax = M_PI_F / 4;

void ChaseMovementGenerator::FanOut(Unit& owner)
{
    Unit* collider = nullptr;
    MaNGOS::AnyUnitFulfillingConditionInRangeCheck collisionCheck(&owner, [&](Unit* unit)->bool
    {
        return &owner != unit && unit->getVictim() && unit->getVictim() == this->i_target.getTarget() && !unit->IsMoving() && !unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }, fanningRadius * fanningRadius, DIST_CALC_NONE);
    MaNGOS::UnitSearcher<MaNGOS::AnyUnitFulfillingConditionInRangeCheck> checker(collider, collisionCheck);
    Cell::VisitAllObjects(&owner, checker, fanningRadius);

    if (collider) // position collision found - need to find a new spot
    {
        int32 direction = irand(0, 1); // blizzlike behaviour
        if (direction == 0) direction = -1;
        float ori = MapManager::NormalizeOrientation(owner.GetOrientation() + M_PI_F + frand(fanAngleMin, fanAngleMax) * direction);
        float x, y, z;
        float targetDist = this->i_target->GetCombinedCombatReach(&owner, false);
        i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), targetDist, ori);
        if (DispatchSplineToPosition(owner, x, y, z, true, false))
        {
            this->i_targetReached = false;
            m_currentMode = CHASE_MODE_FANNING;
            this->i_speedChanged = false;
            m_closenessAndFanningTimer = 0; // restart on reaching target
            return;
        }
        // if first direction failed try second
        ori = MapManager::NormalizeOrientation(owner.GetOrientation() + M_PI_F + frand(fanAngleMin, fanAngleMax) * -direction);
        targetDist = this->i_target->GetCombinedCombatReach(&owner, false);
        i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), targetDist, ori);
        if (DispatchSplineToPosition(owner, x, y, z, true, false))
        {
            this->i_targetReached = false;
            m_currentMode = CHASE_MODE_FANNING;
            this->i_speedChanged = false;
            m_closenessAndFanningTimer = 0; // restart on reaching target
            return;
        }
        // both failed - do nothing
    }
}

bool ChaseMovementGenerator::DispatchSplineToPosition(Unit& owner, float x, float y, float z, bool walk, bool cutPath)
{
    if (!this->i_path)
        this->i_path = new PathFinder(&owner);

    this->i_path->calculate(x, y, z, false);
    if (this->i_path->getPathType() & PATHFIND_NOPATH)
        return false;

    auto& path = this->i_path->getPath();
    if (cutPath)
        CutPath(owner, path);

    ChaseMovementGenerator::_addUnitStateMove(owner);
    Movement::MoveSplineInit init(owner);
    init.MovebyPath(path);
    init.SetWalk(walk);
    init.Launch();
    this->i_target->GetPosition(i_lastTargetPos.x, i_lastTargetPos.y, i_lastTargetPos.z);

    m_reachable = true;
    return true;
}

void ChaseMovementGenerator::CutPath(Unit& owner, PointsArray& path)
{
    if (this->i_offset != 0.f) // need to cut path until most distant viable point
    {
        float distSquared = i_offset * CHASE_MOVE_CLOSER_FACTOR + CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, false);
        distSquared *= distSquared; // squared
        float tarX, tarY, tarZ;
        this->i_target->GetPosition(tarX, tarY, tarZ);
        auto iter = std::next(path.begin(), 1); // need to start at index 1, index 0 is start position and filled in init.Launch()
        for (; iter != path.end(); ++iter)
        {
            G3D::Vector3 data = (*iter);
            if (this->i_target->GetDistance(data.x, data.y, data.z, DIST_CALC_NONE) > distSquared)
                continue;
            if (!owner.GetMap()->IsInLineOfSight(tarX, tarY, tarZ + 2.0f, data.x, data.y, data.z + 2.0f, IGNORE_M2))
                continue;
            // both in LOS and in range - advance to next and stop
            ++iter;
            break;
        }
        if (iter != path.end())
            path.erase(iter, path.end());
    }
}

bool ChaseMovementGenerator::IsReachable() const
{
    return !this->i_targetReached || m_reachable;
}

bool ChaseMovementGenerator::RequiresNewPosition(Unit& owner, float x, float y, float z) const
{
    float dist = this->GetDynamicTargetDistance(owner, true);
    dist *= dist;
    float distanceToCoords = i_target->GetDistance(x, y, z, DIST_CALC_NONE); // raw squared istance
    if (m_moveFurther)
    {
        // need a small window for running further/closer
        if (this->i_offset != 0.f) // optimization for los check
            return distanceToCoords > dist * 1.1f || distanceToCoords < dist * 0.9f || !i_target->IsWithinLOSInMap(&owner, IGNORE_M2);
        else
            return distanceToCoords > dist * 1.1f || distanceToCoords < dist * 0.9f;
    }
    else
    {
        if (this->i_offset != 0.f) // optimization for los check
            return distanceToCoords > dist || !i_target->IsWithinLOSInMap(&owner, IGNORE_M2);
        else
            return distanceToCoords > dist;
    }
}

//-----------------------------------------------//
void FollowMovementGenerator::_clearUnitStateMove(Unit& owner) { owner.clearUnitState(UNIT_STAT_FOLLOW_MOVE); }
void FollowMovementGenerator::_addUnitStateMove(Unit& owner) { owner.addUnitState(UNIT_STAT_FOLLOW_MOVE); }

bool FollowMovementGenerator::_lostTarget(Unit&/* owner*/) const
{
    return false;
}

void FollowMovementGenerator::_reachTarget(Unit&/* owner*/)
{
}

bool FollowMovementGenerator::EnableWalking() const
{
    return (i_target.isValid() && i_target->IsWalking());
}

float FollowMovementGenerator::GetAngle(Unit&/* owner*/) const
{
    // FIXME: Crude standin for clientside movement prediction for pets (16.5% of the angle)
    // Requires proper implementation at some point in the future
    if (int32(i_angle * 100) == int32(PET_FOLLOW_ANGLE * 100) && int32(i_offset) == int32(PET_FOLLOW_DIST))
    {
        if (m_targetMoving && i_target->movespline->Finalized())
            return (i_angle / 4.5f);
    }
    return i_angle;
}

float FollowMovementGenerator::GetOffset(Unit&/* owner*/) const
{
    // FIXME: Crude standin for clientside movement prediction for pets (375% of the offset)
    // Requires proper implementation at some point in the future
    if (int32(i_angle * 100) == int32(PET_FOLLOW_ANGLE * 100) && int32(i_offset) == int32(PET_FOLLOW_DIST))
    {
        if (m_targetMoving && i_target->movespline->Finalized())
            return (i_offset * 3.75f);
    }
    return i_offset;
}

float FollowMovementGenerator::GetVelocity(Unit& owner, bool allowCatchup/* = false*/) const
{
    float speed = 0.0f;  // Use default speed

    if (i_target.isValid() && !owner.isInCombat() && owner.GetMasterGuid() == i_target->GetObjectGuid())
    {
        // Followers sync with master's speed when not in combat and able to boost speed in attempt catch up with master
        if (!i_target->movespline->Finalized())
            speed = i_target->movespline->Velocity();
        else
        {
            MovementFlags movementFlags = i_target->m_movementInfo.GetMovementFlags();
            UnitMoveType movementType = MOVE_RUN;

            if (movementFlags & MOVEFLAG_SWIMMING)
                movementType = (movementFlags & MOVEFLAG_BACKWARD ? MOVE_SWIM_BACK : MOVE_SWIM);
            else if (movementFlags & MOVEFLAG_WALK_MODE)
                movementType = MOVE_WALK;
            else if (movementFlags & MOVEFLAG_BACKWARD)
                movementType = MOVE_RUN_BACK;

            speed = i_target->GetSpeed(movementType);

            // Catch-up speed bonus if follower is too far behind
            // FIXME: Add catchup for units with spline movement
            if (allowCatchup)
            {
                float treshold = GetDynamicTargetDistance(owner, true);
                float dist = i_target->GetDistance(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ(), DIST_CALC_NONE);

                if (dist > (treshold * treshold))
                    speed += (dist / speed);
            }
        }
    }
    return speed;
}

void FollowMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_FOLLOW);                   // _MOVE set in _SetTargetLocation after required checks
    HandleTargetedMovement(owner, 0);
}

void FollowMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_FOLLOW | UNIT_STAT_FOLLOW_MOVE);
}

void FollowMovementGenerator::Interrupt(Unit& owner)
{
    _clearUnitStateMove(owner);
    owner.InterruptMoving();
}

void FollowMovementGenerator::Reset(Unit& owner)
{
    Initialize(owner);
}

bool FollowMovementGenerator::Move(Unit& owner, float x, float y, float z)
{
    if (!i_path)
        i_path = new PathFinder(&owner);

    i_path->calculate(x, y, z, false);

    if (i_path->getPathType() & PATHFIND_NOPATH)
        return false;

    auto& path = i_path->getPath();

    return _move(owner, path);
}

bool FollowMovementGenerator::Unstuck(Unit& owner, float x, float y, float z)
{
    // Attempts to move permanent pet back on the same terrain with owner when out of combat
    // Should not be available to temporary pets/charms/other units following
    if (i_target.isValid() && !owner.isInCombat() && owner.GetOwnerGuid() == i_target->GetObjectGuid())
    {
        // Wait until landing on terrain
        if (!i_target->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR)))
        {
            owner.Relocate(x, y, z);
            return true;
        }
    }
    return false;
}

void FollowMovementGenerator::_setOrientation(Unit& owner)
{
    // Final facing adjustment once target is reached
    if (i_target.isValid())
    {
        m_targetFaced = true;
        const float o = (i_faceTarget ? owner.GetAngle(i_target.getTarget()) : i_target->GetOrientation());
        owner.SetOrientation(o);
        owner.SetFacingTo(o);
    }
}

void FollowMovementGenerator::_setLocation(Unit& owner, bool updateDestination)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    float x, y, z;

    // i_path can be nullptr in case this is the first call for this MMGen (via Update)
    // Can happen for example if no path was created on MMGen-Initialize because of the owner being stunned
    if (updateDestination || !i_path)
    {
        float o = (i_target->GetOrientation() + GetAngle(owner));
        owner.GetPosition(x, y, z);
        i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), GetDynamicTargetDistance(owner, false), o);

        if (!Move(owner, x, y, z))
            Unstuck(owner, x, y, z);
    }
    else    // the destination has not changed, we just need to refresh the path (usually speed change)
        _move(owner, i_path->getPath(), owner.movespline->currentPathIdx());

    i_targetReached = false;
    i_speedChanged = false;
    m_targetFaced = false;
}

bool FollowMovementGenerator::_move(Unit& owner, const Movement::PointsArray& path, int32 offset/* = 0*/) const
{
    if (path.empty())
        return false;

    _addUnitStateMove(owner);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(path, offset);
    init.SetWalk(EnableWalking());
    init.SetVelocity(GetVelocity(owner, true));
    init.Launch();

    return true;
}

// This factor defines how much of the bounding-radius (as measurement of size) will be used for recalculating a new following position
//   The smaller, the more micro movement, the bigger, possibly no proper movement updates
#define FOLLOW_RECALCULATE_FACTOR                         1.0f
// This factor defines when the distance of a follower will have impact onto following-position updates
#define FOLLOW_DIST_GAP_FOR_DIST_FACTOR                   1.0f
// This factor defines how much of the follow-distance will be used as sloppyness value (if the above distance is exceeded)
#define FOLLOW_DIST_RECALCULATE_FACTOR                    1.0f

float FollowMovementGenerator::GetDynamicTargetDistance(Unit& owner, bool forRangeCheck) const
{
    if (!forRangeCheck)
        return (GetOffset(owner) + owner.GetObjectBoundingRadius() + i_target->GetObjectBoundingRadius());

    float allowed_dist = (1.5f - i_target->GetObjectBoundingRadius());
    allowed_dist += FOLLOW_RECALCULATE_FACTOR * (owner.GetObjectBoundingRadius() + i_target->GetObjectBoundingRadius());
    if (i_offset > FOLLOW_DIST_GAP_FOR_DIST_FACTOR)
        allowed_dist += FOLLOW_DIST_RECALCULATE_FACTOR * i_offset;

    return allowed_dist;
}

void FollowMovementGenerator::HandleTargetedMovement(Unit& owner, const uint32& time_diff)
{
    // Detect target movement and relocation

    const bool targetMovingLast = m_targetMoving;
    // If moving in any direction (not count jumping in place)
    m_targetMoving = i_target->m_movementInfo.HasMovementFlag(MovementFlags(movementFlagsMask & ~(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR)));
    bool targetRelocation = false;
    bool targetOrientation = false;

    if (m_targetMoving && !targetMovingLast)        // Movement just started: just force update
        targetRelocation = true;
    else if (!m_targetMoving && targetMovingLast)   // Movement just ended: delay update further
        i_recheckDistance.Reset(1000);
    else                                            // Periodic dist poll: fast when moving, slow when stationary
    {
        i_recheckDistance.Update(time_diff);

        if (i_recheckDistance.Passed())
        {
            i_recheckDistance.Reset(m_targetMoving ? 250 : 500);

            G3D::Vector3 currentTargetPos;

            // FIXME: Add a less resource intensive follow for units with splines with static point picking
            /*
            if (!i_target->movespline->Finalized()) // If moved serverside via spline: check nearest destination change
                currentTargetPos = i_target->movespline->CurrentDestination();
            else                                    // If moved clientside or some other way
            */
                i_target->GetPosition(currentTargetPos.x, currentTargetPos.y, currentTargetPos.z);

            targetRelocation = (currentTargetPos != i_lastTargetPos);
            targetOrientation = (!targetRelocation && !m_targetMoving && !m_targetFaced);
            i_lastTargetPos = currentTargetPos;
        }
    }

    if ((i_speedChanged && !i_targetReached) || targetRelocation)
    {
        i_recheckDistance.Reset(m_targetMoving ? 250 : 500);
        _setLocation(owner, targetRelocation);
    }

    if (!i_faceTarget && i_targetReached && targetOrientation)
        _setOrientation(owner);
}

void FollowMovementGenerator::HandleMovementFailure(Unit& owner)
{
    _clearUnitStateMove(owner);
}

void FollowMovementGenerator::HandleFinalizedMovement(Unit& owner)
{
    i_targetReached = true;
    _reachTarget(owner);
}

//-----------------------------------------------//
template void TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::_setTargetLocation(Unit&, bool);
template void TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::_setTargetLocation(Unit&, bool);
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::Update(Unit&, const uint32&);
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::Update(Unit&, const uint32&);
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::RequiresNewPosition(Unit& owner, float x, float y, float z) const;
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::RequiresNewPosition(Unit& owner, float x, float y, float z) const;
