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
    m_speedChanged = false;

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
    i_target->GetPosition(m_prevTargetPos.x, m_prevTargetPos.y, m_prevTargetPos.z);
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

        if (this->m_speedChanged || targetMoved)
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
                this->m_speedChanged = false;
                /* m_prevTargetPos is updated on making new spline (normal and distancing) and also on reaching target
                is used for determining if player moved towards target whilst the spline was going on to stop the spline prematurely
                and prevent it going behind targets back - it will still occur in rare cases due to PF and lag */
                this->i_target->GetPosition(this->m_prevTargetPos.x, this->m_prevTargetPos.y, this->m_prevTargetPos.z);
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
    else if (this->m_speedChanged)
    {
        if (DispatchSplineToPosition(owner, dest.x, dest.y, dest.z, false, false))
        {
            this->m_speedChanged = false;
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
            if (currentTargetPos != this->m_prevTargetPos)
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
    this->i_target->GetPosition(this->m_prevTargetPos.x, this->m_prevTargetPos.y, this->m_prevTargetPos.z);
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
        this->m_speedChanged = false;
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
        this->m_speedChanged = false;
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
            this->m_speedChanged = false;
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
            this->m_speedChanged = false;
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
    this->i_target->GetPosition(m_prevTargetPos.x, m_prevTargetPos.y, m_prevTargetPos.z);

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
template<class T>
void FollowMovementGenerator<T>::_clearUnitStateMove(T& u) { u.clearUnitState(UNIT_STAT_FOLLOW_MOVE); }
template<class T>
void FollowMovementGenerator<T>::_addUnitStateMove(T& u) { u.addUnitState(UNIT_STAT_FOLLOW_MOVE); }

template<>
bool FollowMovementGenerator<Creature>::EnableWalking() const
{
    return i_target.isValid() && i_target->IsWalking();
}

template<>
bool FollowMovementGenerator<Player>::EnableWalking() const
{
    return false;
}

template<>
void FollowMovementGenerator<Player>::_updateSpeed(Player& /*u*/)
{
    // nothing to do for Player
}

template<>
void FollowMovementGenerator<Creature>::_updateSpeed(Creature& u)
{
    // pet only sync speed with owner
    if (!((Creature&)u).IsPet() || !i_target.isValid() || i_target->GetObjectGuid() != u.GetOwnerGuid())
        return;

    u.UpdateSpeed(MOVE_RUN, true);
    u.UpdateSpeed(MOVE_WALK, true);
    u.UpdateSpeed(MOVE_SWIM, true);
}

template<>
void FollowMovementGenerator<Player>::Initialize(Player& owner)
{
    owner.addUnitState(UNIT_STAT_FOLLOW);                   // _MOVE set in _SetTargetLocation after required checks
    _updateSpeed(owner);
    _setTargetLocation(owner, true);
}

template<>
void FollowMovementGenerator<Creature>::Initialize(Creature& owner)
{
    owner.addUnitState(UNIT_STAT_FOLLOW);                   // _MOVE set in _SetTargetLocation after required checks
    _updateSpeed(owner);
    _setTargetLocation(owner, true);
    if (owner.IsPet())
        i_faceTarget = false;
}

template<class T>
void FollowMovementGenerator<T>::Finalize(T& owner)
{
    owner.clearUnitState(UNIT_STAT_FOLLOW | UNIT_STAT_FOLLOW_MOVE);
    _updateSpeed(owner);
}

template<class T>
void FollowMovementGenerator<T>::Interrupt(T& owner)
{
    owner.InterruptMoving();
    owner.clearUnitState(UNIT_STAT_FOLLOW_MOVE);
    _updateSpeed(owner);
}

template<class T>
void FollowMovementGenerator<T>::Reset(T& owner)
{
    Initialize(owner);
}

// This factor defines how much of the bounding-radius (as measurement of size) will be used for recalculating a new following position
//   The smaller, the more micro movement, the bigger, possibly no proper movement updates
#define FOLLOW_RECALCULATE_FACTOR                         1.0f
// This factor defines when the distance of a follower will have impact onto following-position updates
#define FOLLOW_DIST_GAP_FOR_DIST_FACTOR                   1.0f
// This factor defines how much of the follow-distance will be used as sloppyness value (if the above distance is exceeded)
#define FOLLOW_DIST_RECALCULATE_FACTOR                    1.0f

template<class T>
float FollowMovementGenerator<T>::GetDynamicTargetDistance(T& owner, bool forRangeCheck) const
{
    if (!forRangeCheck)
        return this->i_offset + owner.GetObjectBoundingRadius() + this->i_target->GetObjectBoundingRadius();

    float allowed_dist = sWorld.getConfig(CONFIG_FLOAT_RATE_TARGET_POS_RECALCULATION_RANGE) - this->i_target->GetObjectBoundingRadius();
    allowed_dist += FOLLOW_RECALCULATE_FACTOR * (owner.GetObjectBoundingRadius() + this->i_target->GetObjectBoundingRadius());
    if (this->i_offset > FOLLOW_DIST_GAP_FOR_DIST_FACTOR)
        allowed_dist += FOLLOW_DIST_RECALCULATE_FACTOR * this->i_offset;

    return allowed_dist;
}

// keep same code for Follow
template<class T>
void FollowMovementGenerator<T>::HandleTargetedMovement(T& owner, const uint32& time_diff)
{
    bool targetMoved = false;
    this->i_recheckDistance.Update(time_diff);
    if (this->i_recheckDistance.Passed())
    {
        this->i_recheckDistance.Reset(this->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE ? 50 : 100);
        G3D::Vector3 dest = owner.movespline->FinalDestination();
        targetMoved = this->RequiresNewPosition(owner, dest.x, dest.y, dest.z);

        if (!targetMoved)
        {
            // This unit is in hitbox of target
            // howewer we have to check if the target has not moved to update the orientation
            // client will do it automatically 'visually' but it needs the new orientation to send 
            // or it will retrieve old orientation in some cases (like stun and sap spells)
            G3D::Vector3 currTargetPos;
            this->i_target->GetPosition(currTargetPos.x, currTargetPos.y, currTargetPos.z);
            if (owner.movespline->Finalized() && currTargetPos != this->m_prevTargetPos)
            {
                // position changed we need to adjust owner orientation to continue facing it
                this->m_prevTargetPos = currTargetPos;
                if (this->i_faceTarget)
                {
                    owner.SetInFront(this->i_target.getTarget());         // set movementinfo orientation, needed for next movement if any
                    float angle = owner.GetAngle(this->i_target.getTarget());
                    owner.SetFacingTo(angle);                       // inform client that orientation changed
                }
                else
                {
                    float ori = this->i_target.getTarget()->GetOrientation();
                    owner.SetOrientation(ori);         // set movementinfo orientation, needed for next movement if any
                    owner.SetFacingTo(ori);            // inform client that orientation changed
                }
            }
        }
    }

    if (this->m_speedChanged || targetMoved)
        this->_setTargetLocation(owner, targetMoved);
}

template<class T>
void FollowMovementGenerator<T>::HandleMovementFailure(T& owner)
{
    _clearUnitStateMove(owner);
}

template<class T>
void FollowMovementGenerator<T>::HandleFinalizedMovement(T& owner)
{
    this->i_targetReached = true;
    if (!owner.HasInArc(this->i_target.getTarget(), 0.01f))
        owner.SetInFront(this->i_target.getTarget());
    FollowMovementGenerator<T>::_reachTarget(owner);
}

//-----------------------------------------------//
template void TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::_setTargetLocation(Unit&, bool);
template void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::_setTargetLocation(Player&, bool);
template void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::_setTargetLocation(Creature&, bool);
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::Update(Unit&, const uint32&);
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::Update(Player&, const uint32&);
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::Update(Creature&, const uint32&);
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::RequiresNewPosition(Unit& owner, float x, float y, float z) const;
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::RequiresNewPosition(Player& owner, float x, float y, float z) const;
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::RequiresNewPosition(Creature& owner, float x, float y, float z) const;

template void FollowMovementGenerator<Player>::_clearUnitStateMove(Player& u);
template void FollowMovementGenerator<Creature>::_addUnitStateMove(Creature& u);
template void FollowMovementGenerator<Player>::Finalize(Player&);
template void FollowMovementGenerator<Creature>::Finalize(Creature&);
template void FollowMovementGenerator<Player>::Interrupt(Player&);
template void FollowMovementGenerator<Creature>::Interrupt(Creature&);
template void FollowMovementGenerator<Player>::Reset(Player&);
template void FollowMovementGenerator<Creature>::Reset(Creature&);
template float FollowMovementGenerator<Creature>::GetDynamicTargetDistance(Creature&, bool) const;
template float FollowMovementGenerator<Player>::GetDynamicTargetDistance(Player&, bool) const;
template void FollowMovementGenerator<Creature>::HandleTargetedMovement(Creature&, const uint32&);
template void FollowMovementGenerator<Player>::HandleTargetedMovement(Player&, const uint32&);
template void FollowMovementGenerator<Creature>::HandleFinalizedMovement(Creature&);
template void FollowMovementGenerator<Player>::HandleFinalizedMovement(Player&);
template void FollowMovementGenerator<Creature>::HandleMovementFailure(Creature&);
template void FollowMovementGenerator<Player>::HandleMovementFailure(Player&);
