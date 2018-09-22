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
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"

#define IGNORE_M2 true // simple define for avoiding bugs due to different setting across movechase

 // Chase-Movement: These factors depend on combat-reach distance
#define CHASE_DEFAULT_RANGE_FACTOR                        0.5f
#define CHASE_RECHASE_RANGE_FACTOR                        0.75f
#define CHASE_MOVE_CLOSER_FACTOR                          0.875f

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
        else if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE && i_angle == 0.0f)
        {
            i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), this->GetDynamicTargetDistance(owner, false), i_target->GetAngle(&owner));
        }
        // Targeted movement to at i_offset distance from target and i_angle from target facing
        else
        {
            i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), this->GetDynamicTargetDistance(owner, false), i_target->GetOrientation() + i_angle);
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
    init.MovebyPath(i_path->getPath());
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

    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE && owner.hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    // prevent movement while casting spells with cast time or channel time
    if (owner.IsNonMeleeSpellCasted(false, false,  true))
    {
        if (!owner.IsStopped())
            owner.StopMoving();
        return true;
    }

    // prevent crash after creature killed pet
    if (static_cast<D*>(this)->_lostTarget(owner))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    bool targetMoved = false;
    i_recheckDistance.Update(time_diff);
    if (i_recheckDistance.Passed())
    {
        i_recheckDistance.Reset(this->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE ? 50 : 100);
        G3D::Vector3 dest = owner.movespline->FinalDestination();
        targetMoved = RequiresNewPosition(owner, dest.x, dest.y, dest.z);
        if (!targetMoved)
        {
            // This unit is in hitbox of target
            // howewer we have to check if the target not moved a bit to update the orientation
            // client do it automatically 'visually' but it need this new orientation send or it will retrieve old orientation in some case (like stun)
            G3D::Vector3 currTargetPos;
            i_target->GetPosition(currTargetPos.x, currTargetPos.y, currTargetPos.z);
            if (owner.movespline->Finalized() && currTargetPos != m_prevTargetPos)
            {
                // position changed we need to adjust owner orientation to continue facing it
                m_prevTargetPos = currTargetPos;
                owner.SetInFront(i_target.getTarget());         // set movementinfo orientation, needed for next movement if any

                float angle = owner.GetAngle(i_target.getTarget());
                owner.SetFacingTo(angle);                       // inform client that orientation changed
                //sLog.outString("Updating facing to with angle (%3.3f)!!!", angle);
            }
        }
    }

    if (m_speedChanged || targetMoved)
        _setTargetLocation(owner, targetMoved);

    if (owner.movespline->Finalized())
    {
        if (i_angle == 0.f && !owner.HasInArc(i_target.getTarget(), 0.01f))
            owner.SetInFront(i_target.getTarget());

        if (!i_targetReached)
        {
            i_targetReached = true;
            static_cast<D*>(this)->_reachTarget(owner);
        }
    }
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
    if (dist != 0.f && this->GetMovementGeneratorType() == CHASE_MOTION_TYPE) // optimization for los check
        return !i_target->IsWithinDist3d(x, y, z, dist) || !i_target->IsWithinLOSInMap(&owner, IGNORE_M2);
    else
        return !i_target->IsWithinDist3d(x, y, z, dist);
}

template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::SetOffsetAndAngle(float offset, float angle)
{
    i_offset = offset;
    i_angle = angle;
}

//-----------------------------------------------//
template<class T>
void ChaseMovementGenerator<T>::_clearUnitStateMove(T& u) { u.clearUnitState(UNIT_STAT_CHASE_MOVE); }
template<class T>
void ChaseMovementGenerator<T>::_addUnitStateMove(T& u) { u.addUnitState(UNIT_STAT_CHASE_MOVE); }

template<class T>
bool ChaseMovementGenerator<T>::_lostTarget(T& u) const
{
    return m_combat && u.getVictim() != this->GetCurrentTarget();
}

template<class T>
void ChaseMovementGenerator<T>::_reachTarget(T& owner)
{

}

template<>
void ChaseMovementGenerator<Player>::Initialize(Player& owner)
{
    owner.addUnitState(UNIT_STAT_CHASE);                    // _MOVE set in _SetTargetLocation after required checks
    _setTargetLocation(owner, true);

    i_target->GetPosition(m_prevTargetPos.x, m_prevTargetPos.y, m_prevTargetPos.z);
}

template<>
void ChaseMovementGenerator<Creature>::Initialize(Creature& owner)
{
    owner.SetWalk(m_walk, false);                            // Chase movement is running
    owner.addUnitState(UNIT_STAT_CHASE);                    // _MOVE set in _SetTargetLocation after required checks
    _setTargetLocation(owner, true);

    i_target->GetPosition(m_prevTargetPos.x, m_prevTargetPos.y, m_prevTargetPos.z);
}

template<class T>
void ChaseMovementGenerator<T>::Finalize(T& owner)
{
    owner.clearUnitState(UNIT_STAT_CHASE | UNIT_STAT_CHASE_MOVE);
}

template<class T>
void ChaseMovementGenerator<T>::Interrupt(T& owner)
{
    owner.InterruptMoving();
    owner.clearUnitState(UNIT_STAT_CHASE_MOVE);
}

template<class T>
void ChaseMovementGenerator<T>::Reset(T& owner)
{
    Initialize(owner);
}

template<class T>
void ChaseMovementGenerator<T>::SetMovementParameters(float offset, float angle, bool moveFurther)
{
    TargetedMovementGeneratorMedium<T, ChaseMovementGenerator>::SetOffsetAndAngle(offset, angle);
    m_moveFurther = moveFurther;
}

template<class T>
float ChaseMovementGenerator<T>::GetDynamicTargetDistance(T& owner, bool forRangeCheck) const
{
    if (m_moveFurther)
    {
        if (!forRangeCheck)
            return this->i_offset + CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, false);

        return CHASE_RECHASE_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, false);
    }
    else
    {
        if (!forRangeCheck) // move to melee range and then cut path to simulate retail
            return CHASE_DEFAULT_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, false);

        // check against actual max range setting
        return this->i_offset + CHASE_RECHASE_RANGE_FACTOR * this->i_target->GetCombinedCombatReach(&owner, false);
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
#define FOLLOW_DIST_GAP_FOR_DIST_FACTOR                   3.0f
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

//-----------------------------------------------//
template void TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::_setTargetLocation(Player&, bool);
template void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::_setTargetLocation(Player&, bool);
template void TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::_setTargetLocation(Creature&, bool);
template void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::_setTargetLocation(Creature&, bool);
template bool TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::Update(Player&, const uint32&);
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::Update(Player&, const uint32&);
template bool TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::Update(Creature&, const uint32&);
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::Update(Creature&, const uint32&);
template bool TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::IsReachable() const;

template void ChaseMovementGenerator<Player>::_clearUnitStateMove(Player& u);
template void ChaseMovementGenerator<Creature>::_addUnitStateMove(Creature& u);
template bool ChaseMovementGenerator<Player>::_lostTarget(Player& u) const;
template bool ChaseMovementGenerator<Creature>::_lostTarget(Creature& u) const;
template void ChaseMovementGenerator<Player>::_reachTarget(Player&);
template void ChaseMovementGenerator<Creature>::_reachTarget(Creature&);
template void ChaseMovementGenerator<Player>::Finalize(Player&);
template void ChaseMovementGenerator<Creature>::Finalize(Creature&);
template void ChaseMovementGenerator<Player>::Interrupt(Player&);
template void ChaseMovementGenerator<Creature>::Interrupt(Creature&);
template void ChaseMovementGenerator<Player>::Reset(Player&);
template void ChaseMovementGenerator<Creature>::Reset(Creature&);
template float ChaseMovementGenerator<Creature>::GetDynamicTargetDistance(Creature&, bool) const;
template float ChaseMovementGenerator<Player>::GetDynamicTargetDistance(Player&, bool) const;
template void ChaseMovementGenerator<Creature>::SetMovementParameters(float, float, bool);
template void ChaseMovementGenerator<Player>::SetMovementParameters(float, float, bool);

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
