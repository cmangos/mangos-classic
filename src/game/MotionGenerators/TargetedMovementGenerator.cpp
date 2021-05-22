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
#include "Entities/Transports.h"

#define IGNORE_M2 true // simple define for avoiding bugs due to different setting across movechase

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
bool TargetedMovementGeneratorMedium<T, D>::Update(T& owner, const uint32& time_diff)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return !static_cast<D*>(this)->RemoveOnInvalid();

    // Trying to detect error
    if (i_target->GetMap() != owner.GetMap())
    {
        sLog.outCustomLog("TargetedMovementGeneratorMedium::Update(): Target %s left map id %u for map id %u out of order!",
                      i_target->GetGuidStr().c_str(), i_target->GetMapId(), owner.GetMapId());
        return !static_cast<D*>(this)->RemoveOnInvalid();
    }

    if (i_target->GetTransport() != owner.GetTransport())
    {
        HandleMovementFailure(owner);
        return true;
    }

    if (!owner.IsAlive())
        return true;

    // prevent movement while casting spells with cast time or channel time
    if (owner.IsNonMeleeSpellCasted(false, false, true, true))
    {
        if (!owner.movespline->Finalized())
        {
            if (owner.IsClientControlled())
                owner.StopMoving(true);
            else
                owner.InterruptMoving();
        }
        return true;
    }

    if (_hasUnitStateNotMove(owner))
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
bool TargetedMovementGeneratorMedium<T, D>::RequiresNewPosition(T& owner, Position pos) const
{
    float dist = this->GetDynamicTargetDistance(owner, true);
    // More distance let have better performance, less distance let have more sensitive reaction at target move.
    return i_target->GetDistance(pos.x, pos.y, pos.z, DIST_CALC_NONE, owner.GetTransport()) > dist * dist;
}

//-----------------------------------------------//
bool ChaseMovementGenerator::_hasUnitStateNotMove(Unit& u) { return u.hasUnitState(UNIT_STAT_NOT_MOVE | UNIT_STAT_NO_COMBAT_MOVEMENT); }
void ChaseMovementGenerator::_clearUnitStateMove(Unit& u) { u.clearUnitState(UNIT_STAT_CHASE_MOVE); }
void ChaseMovementGenerator::_addUnitStateMove(Unit& u) { u.addUnitState(UNIT_STAT_CHASE_MOVE); }

bool ChaseMovementGenerator::_lostTarget(Unit& u) const
{
    return m_combat && u.GetVictim() != this->GetCurrentTarget();
}

void ChaseMovementGenerator::_reachTarget(Unit& /*owner*/)
{

}

void ChaseMovementGenerator::Initialize(Unit& owner)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;
    if (i_target->GetMap() != owner.GetMap())
    {
        sLog.outCustomLog("ChaseMovementGenerator: Owner and target are not in the same map.");
        return;
    }
    owner.addUnitState(UNIT_STAT_CHASE);                    // _MOVE set in _SetTargetLocation after required checks
    _setLocation(owner);
    i_target->GetPosition(i_lastTargetPos.x, i_lastTargetPos.y, i_lastTargetPos.z);
    m_fanningEnabled = !(owner.GetTypeId() == TYPEID_UNIT && static_cast<Creature&>(owner).IsWorldBoss());
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

// Chase-Movement: These factors depend on combat-reach distance
#define CHASE_DEFAULT_RANGE_FACTOR                        0.5f
#define CHASE_RECHASE_RANGE_FACTOR                        0.75f
#define CHASE_MOVE_CLOSER_FACTOR                          0.875f

std::pair<std::string, std::string> ChaseMovementGenerator::GetPrintout() const
{
    std::string output;
    std::string commandOutput;
    auto& path = this->i_path->getPath();
    if (path.size() > 0)
    {
        output += "Start:" + std::to_string(path[0].x) + " " + std::to_string(path[0].y) + " " + std::to_string(path[0].z) + "\n";
        commandOutput += ".go xyz " + std::to_string(path[0].x) + " " + std::to_string(path[0].y) + " " + std::to_string(path[0].z) + "\n";
        for (uint32 i = 1; i < path.size() - 1; ++i)
        {
            output += "Point: " + std::to_string(path[i].x) + " " + std::to_string(path[i].y) + " " + std::to_string(path[i].z) + "\n";
            commandOutput += ".go xyz " + std::to_string(path[i].x) + " " + std::to_string(path[i].y) + " " + std::to_string(path[i].z) + "\n";
        }
        output += "End:" + std::to_string(path[path.size() - 1].x) + " " + std::to_string(path[path.size() - 1].y) + " " + std::to_string(path[path.size() - 1].z) + "\n";
        commandOutput += ".go xyz " + std::to_string(path[path.size() - 1].x) + " " + std::to_string(path[path.size() - 1].y) + " " + std::to_string(path[path.size() - 1].z) + "\n";
    }
    return {output, commandOutput};
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
    this->i_target->GetPosition(currentTargetPos.x, currentTargetPos.y, currentTargetPos.z, owner.GetTransport());
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
    auto vectorDest = owner.movespline->FinalDestination();
    Position dest(vectorDest.x, vectorDest.y, vectorDest.z, 0.f);
    if (dest.x == 0 && dest.y == 0 && dest.z == 0)
        owner.GetPosition(dest.x, dest.y, dest.z, owner.GetTransport());
    if (m_currentMode != CHASE_MODE_DISTANCING)
    {
        targetMoved = this->RequiresNewPosition(owner, dest); // uses transport coordinates

        if ((this->i_speedChanged && !owner.movespline->Finalized()) || targetMoved)
        {
            float x, y, z;

            // i_path can be nullptr in case this is the first call for this MMGen (via Update)
            // Can happen for example if no path was created on MMGen-Initialize because of the owner being stunned
            if (targetMoved || !this->i_path)
                _getLocation(owner, x, y, z);
            else
            {
                // the destination has not changed, we just need to refresh the path (usually speed change)
                G3D::Vector3 end = this->i_path->getEndPosition(); // supplies transport coordinates when on a transport
                x = end.x;
                y = end.y;
                z = end.z;
                if (GenericTransport* transport = owner.GetTransport()) // dispatch spline position needs global coordinates
                    transport->CalculatePassengerPosition(dest.x, dest.y, dest.z);
            }

            if (owner.GetDistance(x, y, z, DIST_CALC_NONE) > 0.3f)
            {
                if (DispatchSplineToPosition(owner, x, y, z, EnableWalking(), true, true))
                {
                    this->i_targetReached = false;
                    this->i_speedChanged = false;
                    /* m_prevTargetPos is updated on making new spline (normal and distancing) and also on reaching target
                    is used for determining if player moved towards target whilst the spline was going on to stop the spline prematurely
                    and prevent it going behind targets back - it will still occur in rare cases due to PF and lag */
                    this->i_target->GetPosition(this->i_lastTargetPos.x, this->i_lastTargetPos.y, this->i_lastTargetPos.z, owner.GetTransport());
                    m_closenessAndFanningTimer = 0;
                    return;
                }
            }
            // if we arrived here something failed in PF dispatch and target is not reachable
            if (this->i_offset == 0.f)
            {
                if (!owner.CanReachWithMeleeAttack(this->i_target.getTarget()))
                    if (!i_target->IsFalling())
                        m_reachable = false;
            }
            else
            {
                if (owner.GetDistance(this->i_target.getTarget(), true, DIST_CALC_COMBAT_REACH) > this->i_offset)
                    if (!i_target->IsFalling())
                        m_reachable = false;
            }
            return;
        }
        else if (!targetMoved) // we do not need new position and we are reachable
            m_reachable = true;
    }
    else if (this->i_speedChanged)
    {
        if (GenericTransport* transport = owner.GetTransport()) // dispatch spline position needs global coordinates
            transport->CalculatePassengerPosition(dest.x, dest.y, dest.z);
        if (DispatchSplineToPosition(owner, dest.x, dest.y, dest.z, false, false, true))
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
                Position ownerPos;
                owner.GetPosition(ownerPos.x, ownerPos.y, ownerPos.z, owner.GetTransport());
                float distFromDestination = owner.GetDistance(dest.x, dest.y, dest.z, DIST_CALC_NONE, owner.GetTransport());
                float distOwnerFromTarget = this->i_target->GetDistance(ownerPos.x, ownerPos.y, ownerPos.z, DIST_CALC_NONE, owner.GetTransport());
                // Explanation of magic: comparing distances between target and destination makes it so we know when mob is between destination and owner
                // - thats when forcible spline stop is needed
                float targetDist = this->i_target->GetCombinedCombatReach(&owner, this->i_offset == 0.f ? true : false);
                if (distFromDestination > distOwnerFromTarget)
                {
                    if (this->i_target->GetDistance(ownerPos.x, ownerPos.y, ownerPos.z, DIST_CALC_NONE, owner.GetTransport()) < targetDist * targetDist)
                    {
                        if (owner.IsClientControlled())
                            owner.StopMoving(true);
                        else
                            owner.InterruptMoving();
                    }
                }
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
    this->i_target->GetPosition(this->i_lastTargetPos.x, this->i_lastTargetPos.y, this->i_lastTargetPos.z, owner.GetTransport());
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
    _clearUnitStateMove(owner);
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
    // do not allow backpedalling during scripts
    if (!owner.AI() || owner.AI()->GetCombatScriptStatus())
        return;

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
    if (!m_fanningEnabled)
        return;

    Unit* collider = nullptr;
    MaNGOS::AnyUnitFulfillingConditionInRangeCheck collisionCheck(&owner, [&](Unit* unit)->bool
    {
        return &owner != unit && unit->GetVictim() && unit->GetVictim() == this->i_target.getTarget() && !unit->IsMoving() && !unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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

bool ChaseMovementGenerator::DispatchSplineToPosition(Unit& owner, float x, float y, float z, bool walk, bool cutPath, bool target)
{
    if (owner.IsDebuggingMovement())
    {
        for (ObjectGuid guid : m_spawns)
            if (Creature* whisp = owner.GetMap()->GetCreature(guid))
                whisp->ForcedDespawn();
        m_spawns.clear();
    }

    if (!owner.movespline->Finalized())
    {
        owner.UpdateSplinePosition();
        if (owner.IsDebuggingMovement())
        {
            Position loc = owner.GetPosition();
            if (Creature* spawn = owner.SummonCreature(1, loc.x, loc.y, loc.z, loc.o, TEMPSPAWN_TIMED_DESPAWN, 5000))
                m_spawns.push_back(spawn->GetObjectGuid()), spawn->SetVisibility(VISIBILITY_ON);
        }
    }

    if (!this->i_path)
        this->i_path = new PathFinder(&owner);

    bool gen = false;
    if (owner.IsWithinDist3d(x, y, z, 200.f) && std::abs(owner.GetPositionZ() - z) < 5.f && owner.IsWithinLOS(x, y, z + i_target->GetCollisionHeight()) && !owner.IsInWater() && !i_target->IsInWater())
    {
        this->i_path->calculate(x, y, z, false, true);
        auto& path = this->i_path->getPath();
        gen = true;
        if (sWorld.getConfig(CONFIG_BOOL_PATH_FIND_NORMALIZE_Z) && (this->i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE)) == 0)
        {
            for (uint32 i = 0; i < path.size() - 1; ++i)
            {
                if (std::abs(path[i].z - path[i + 1].z) > 1.f)
                {
                    gen = false;
                    break;
                }
            }
        }
    }

    if (owner.IsDebuggingMovement())
    {
        if (i_target->IsPlayer())
        {
            Position pos = owner.GetPosition();
            std::string message = "Start X: " + std::to_string(pos.x) + " Y: " + std::to_string(pos.y) + " Z: " + std::to_string(pos.z) + "\n";
            message += "End X: " + std::to_string(x) + " Y: " + std::to_string(y) + " Z: " + std::to_string(z) + "\n";
            message += (owner.IsWithinDist3d(x, y, z, 200.f) ? "Within 200f " : "") + std::string(owner.IsWithinLOS(x, y, z + i_target->GetCollisionHeight()) ? "Within LOS " : "") +
                ((this->i_path->getPathType() & PATHFIND_NOPATH) ? " No straight path" : "") + "\n";
            static_cast<Player*>(i_target.getTarget())->SendMessageToPlayer(message);
            std::ostringstream out;
            out.precision(10);
            out << ".go xyz " << std::fixed << pos.x << " " << pos.y << " " << pos.z << std::endl;
            out << ".go xyz " << std::fixed << x << " " << y << " " << z << std::endl;
            sLog.outCustomLog("%s", out.str().data());
        }
    }

    if (!gen || (this->i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE)))
    {
        this->i_path->calculate(x, y, z);
        if (this->i_path->getPathType() & PATHFIND_NOPATH)
            return false;
    }

    auto& path = this->i_path->getPath();

    if (cutPath)
        CutPath(owner, path);

    if (owner.IsDebuggingMovement())
    {
        for (auto& point : path)
            if (Creature* spawn = owner.SummonCreature(1, point.x, point.y, point.z, 0.f, TEMPSPAWN_TIMED_DESPAWN, 5000))
                m_spawns.push_back(spawn->GetObjectGuid()), spawn->SetVisibility(VISIBILITY_ON);
    }

    _addUnitStateMove(owner);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(path);
    init.SetWalk(walk);
    if (target)
        init.SetFacing(i_target.getTarget());
    init.Launch();

    this->i_target->GetPosition(i_lastTargetPos.x, i_lastTargetPos.y, i_lastTargetPos.z, owner.GetTransport());

    m_reachable = true;
    return true;
}

void ChaseMovementGenerator::CutPath(Unit& owner, PointsArray& path)
{
    if (this->i_offset != 0.f) // need to cut path until most distant viable point
    {
        const float dist = (i_offset * CHASE_MOVE_CLOSER_FACTOR) + (this->i_target->GetCombinedCombatReach(&owner, false) * CHASE_DEFAULT_RANGE_FACTOR);
        const float distSquared = (dist * dist);
        float tarX, tarY, tarZ;
        this->i_target->GetPosition(tarX, tarY, tarZ);

        // Need to start at index 1, index 0 is start position and filled in init.Launch()
        for (size_t i = 1; i < path.size(); ++i)
        {
            const G3D::Vector3& data = path.at(i);
            if (this->i_target->GetDistance(data.x, data.y, data.z, DIST_CALC_NONE) > distSquared)
                continue;
            if (!owner.GetMap()->IsInLineOfSight(tarX, tarY, tarZ + i_target->GetCollisionHeight(), data.x, data.y, data.z + owner.GetCollisionHeight(), IGNORE_M2))
                continue;
            // both in LOS and in range - advance to next and stop
            return path.resize(++i);
        }
    }
}

bool ChaseMovementGenerator::IsReachable() const
{
    return !this->i_targetReached || m_reachable;
}

bool ChaseMovementGenerator::RequiresNewPosition(Unit& owner, Position pos) const
{
    float dist = this->GetDynamicTargetDistance(owner, true);
    dist *= dist;
    float distanceToCoords = i_target->GetDistance(pos.x, pos.y, pos.z, DIST_CALC_NONE, owner.GetTransport()); // raw squared istance
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

bool ChaseMovementGenerator::_getLocation(Unit& owner, float& x, float& y, float& z) const
{
    if (!i_target.isValid())
        return false;

    // Chase Movement and angle == 0 case: Chase to current angle
    // Need to avoid readjustment when target is attacking owner
    const bool currentAngle = (i_angle == 0.f || (i_target->GetVictim() && i_target->GetVictim() == &owner));

    float angle = (currentAngle ? i_target->GetAngle(&owner) : (i_target->GetOrientation() + i_angle));

    owner.GetPosition(x, y, z);

    // TODO: This code would also benefit greatly if "Get Target Position in 250 ms" function existed
    // TODO: When target is moving away, we should choose a point that is much much closer to account for it
    i_target->GetNearPoint(&owner, x, y, z, owner.GetObjectBoundingRadius(), this->GetDynamicTargetDistance(owner, false), angle);

    return true;
}

void ChaseMovementGenerator::_setLocation(Unit& owner)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (_hasUnitStateNotMove(owner))
        return;

    Position pos = owner.GetPosition();
    if (RequiresNewPosition(owner, owner.GetPosition(owner.GetTransport())) && _getLocation(owner, pos.x, pos.y, pos.z))
    {
        if (DispatchSplineToPosition(owner, pos.x, pos.y, pos.z, EnableWalking(), true, true))
            this->i_target->GetPosition(this->i_lastTargetPos.x, this->i_lastTargetPos.y, this->i_lastTargetPos.z, owner.GetTransport());
    }
    else
        return;

    i_targetReached = false;
    i_speedChanged = false;
}

//-----------------------------------------------//
bool FollowMovementGenerator::_hasUnitStateNotMove(Unit& owner) { return owner.hasUnitState(UNIT_STAT_NOT_MOVE); }
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

void FollowMovementGenerator::MarkMovegen()
{
    if (m_possess)
        m_main = false;
}

float FollowMovementGenerator::GetSpeed(Unit& owner) const
{
    const UnitMoveType type = i_target->m_movementInfo.GetSpeedType();
    float speed = owner.GetSpeed(type);

    if (owner.IsInCombat() || !i_target.isValid())
        return speed;

    // Followers sync with master's speed when not in combat
    // Use default speed when a mix of PC and NPC units involved (escorting?)
    if (owner.HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) == i_target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        speed = i_target->GetSpeedInMotion();

    // Catchup boost is not allowed, stop here:
    if (!IsBoostAllowed(owner))
        return speed;

    // Catch-up speed boost if allowed:
    // * When following client-controlled units or overriden: boost up to max hardcoded speed
    // * When following server-controlled units: try to boost up to own run speed
    if (i_target->IsClientControlled() || m_boost)
    {
        const float bonus = (i_target->GetDistance(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ(), DIST_CALC_NONE) / speed);
        return std::max(owner.GetSpeed(MOVE_WALK), std::min((speed + bonus), 40.0f));
    }

    return std::max(speed, owner.GetSpeed(MOVE_RUN));
}

bool FollowMovementGenerator::IsBoostAllowed(Unit& owner) const
{
    // Do not allow boosting when in combat
    if (owner.IsInCombat() || !i_target.isValid())
        return false;

    // Do not allow boosting outside of pet/master relationship by default, unless overriden:
    if (!m_boost && owner.GetMasterGuid() != i_target->GetObjectGuid())
        return false;

    // Boost speed only if follower is too far behind
    if (!RequiresNewPosition(owner, owner.GetPosition(owner.GetTransport())))
        return false;

    // Do not allow speed boosting when in pvp instances
    if (const MapEntry* map = sMapStore.LookupEntry(owner.GetMapId()))
        if (map->IsBattleGround())
            return false;

    // Allow boosting when out of master's line of sight:
    if (!i_target->IsWithinLOSInMap(&owner))
        return true;

    // Do not allow boosting if follower is already in front/back of target:
    return (i_target->HasInArc(&owner) != !i_target->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_BACKWARD)));
}

bool FollowMovementGenerator::IsUnstuckAllowed(Unit& owner) const
{
    if (!i_target.isValid())
        return false;

    // Do not try to unstuck outside of pet/master relationship
    if (owner.GetMasterGuid() != i_target->GetObjectGuid())
        return false;

    // Do not try to unstuck while target has not landed or stabilized on terrain in some way
    if (i_target->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR | MOVEFLAG_FLYING)))
        return false;

    // Do not try to unstuck if not too far behind:
    return RequiresNewPosition(owner, owner.GetPosition(owner.GetTransport()));
}

void FollowMovementGenerator::Initialize(Unit& owner)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (i_target->GetMap() != owner.GetMap())
    {
        sLog.outCustomLog("FollowMovementGenerator: Owner and target are not in the same map.");
        return;
    }

    owner.addUnitState(UNIT_STAT_FOLLOW);                   // _MOVE set in _SetTargetLocation after required checks
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

bool FollowMovementGenerator::GetResetPosition(Unit& owner, float& x, float& y, float& z, float& o) const
{
    if (!_getLocation(owner, x, y, z, m_targetMoving))
        return false;

    if (!_getOrientation(owner, o))
        o = owner.GetAngle(x, y);

    // must return local coords
    if (GenericTransport* transport = owner.GetTransport())
        transport->CalculatePassengerOffset(x, y, z, &o);

    return true;
}

bool FollowMovementGenerator::Move(Unit& owner, float x, float y, float z)
{
    if (!owner.movespline->Finalized())
        owner.UpdateSplinePosition(true);

    if (!i_path)
        i_path = new PathFinder(&owner);

    bool unstuck = false;

    i_path->calculate(x, y, z);

    auto& path = i_path->getPath();

    if (i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_SHORTCUT))
    {
        if (!IsUnstuckAllowed(owner))
            return false;

        unstuck = true;
    }
    else if (i_target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        // Trim path according to LoS when target is a PC unit
        // Follower needs to be able to see predicted location to prevent issues and exploits
        const Map* map = owner.GetMap();
        const float height = owner.GetCollisionHeight();
        const GenericTransport* transport = owner.GetTransport();

        for (size_t i = 1; i < path.size(); ++i)
        {
            float lx = path[i - 1].x, ly = path[i - 1].y, lz = path[i - 1].z;
            float nx = path[i].x, ny = path[i].y, nz = path[i].z;

            // Convert local transport spline to global spline for visibility check
            if (transport)
            {
                transport->CalculatePassengerPosition(lx, ly, lz);
                transport->CalculatePassengerPosition(nx, ny, nz);
            }

            if (map->IsInLineOfSight(lx, ly, (lz + height), nx, ny, (nz + height), true))
                continue;

            if (i != 1)
                path.resize(i + 1);
            else if (!IsUnstuckAllowed(owner))
                return false;
            else
                unstuck = true;
            break;
        }
    }

    if (unstuck)
    {
        float o;
        _getOrientation(owner, o);
        _getLocation(owner, x, y, z, false);

        // Do a final sanity LoS check when unstucking self to prevent landing on a wrong walkable surface
        // Target needs to be able to see this new location to prevent issues and exploits
        if (i_target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            if (!i_target->IsWithinLOS(x, y, (z + owner.GetCollisionHeight()), true))
                i_target->GetPosition(x, y, z);

        if (owner.GetTypeId() == TYPEID_PLAYER)
            owner.NearTeleportTo(x, y, z, o);
        else
        {
            owner.GetMap()->CreatureRelocation(static_cast<Creature*>(&owner), x, y, z, o);
            owner.SendHeartBeat();
        }

        return false;
    }

    _addUnitStateMove(owner);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(path);
    init.SetWalk(EnableWalking());
    init.SetVelocity(GetSpeed(owner));
    init.Launch();

    return true;
}

bool FollowMovementGenerator::_getOrientation(Unit& owner, float& o) const
{
    if (!i_target.isValid())
        return false;

    o = (i_faceTarget ? owner.GetAngle(i_target.getTarget()) : i_target->GetOrientation());
    return true;
}

bool FollowMovementGenerator::_getLocation(Unit& owner, float& x, float& y, float& z, bool movingNow) const
{
    if (!i_target.isValid())
        return false;

    owner.GetPosition(x, y, z);

    float tx, ty, tz;
    i_target->GetPosition(tx, ty, tz);

    float to = i_target->GetOrientation();

    // Server-controlled moving unit: use destination
    if (!i_target->movespline->Finalized() && movingNow)
    {
        auto const& dest = i_target->movespline->CurrentDestination();
        tx = dest.x;
        ty = dest.y;
        tz = dest.z;

        if (GenericTransport* transport = i_target->GetTransport())
            transport->CalculatePassengerPosition(tx, ty, tz);
    }
    // Client-controlled moving unit: use simple prediction, unless on transport
    else if (movingNow)
    {
        const float speed = i_target->GetSpeedInMotion();
        const float o = i_target->m_movementInfo.GetOrientationInMotion(to);
        float dx = (speed * cos(o)), dy = (speed * sin(o));
        float nx = (tx + dx), ny = (ty + dy), nz = tz;
        i_target->UpdateAllowedPositionZ(nx, ny, nz);

        const float height = owner.GetCollisionHeight();

        // Attempt to trim predicted path according to LoS when following client controlled unit
        // Target needs to be able to see predicted location to prevent issues and exploits
        if (i_target->GetMap()->IsInLineOfSight(x, y, (z + height), nx, ny, (nz + height), true))
        {
            tx = nx;
            ty = ny;
            tz = nz;
        }
    }

    const float radius = owner.GetObjectBoundingRadius();
    const float range = GetDynamicTargetDistance(owner, false);
    const float angle = (to + GetAngle());

    i_target->GetNearPointAt(tx, ty, tz, &owner, x, y, z, radius, range, angle);
    return true;
}

void FollowMovementGenerator::_setOrientation(Unit& owner)
{
    // Final facing adjustment once target is reached
    float o;

    if (_getOrientation(owner, o))
    {
        m_targetFaced = true;
        owner.SetOrientation(o);
        owner.SetFacingTo(o);
    }
}

void FollowMovementGenerator::_setLocation(Unit& owner, bool movingNow)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (_hasUnitStateNotMove(owner))
        return;

    float x, y, z;

    _getLocation(owner, x, y, z, movingNow);

    i_targetReached = !Move(owner, x, y, z);
    i_speedChanged = false;
    m_targetFaced = false;
}

uint32 FollowMovementGenerator::_getPollRateMultiplier(Unit& owner, bool/* targetMovingNow*/, bool/* targetMovedBefore = true*/) const
{
    uint32 multiplier = (owner.HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) ? 1 : 2);
    return multiplier;
}

uint32 FollowMovementGenerator::_getPollRate(Unit& owner, bool movingNow, bool movingBefore/* = true*/) const
{
    const uint32 multiplier = _getPollRateMultiplier(owner, movingNow, movingBefore);
    const uint32 rate = (_getPollRateBase() * uint32(owner.GetSpeedRateInMotion()));
    const uint32 max = (_getPollRateMax() * uint32(owner.GetSpeedRateInMotion()));
    return std::min(std::max(rate, (rate * multiplier)), max);
}

// Max distance from movement target point (+moving unit size) and targeted object (+size) for target to be considered too far away.
//      Suggested max: melee attack range (5), suggested min: contact range (0.5)
//      Less distance let have more sensitive reaction at target movement digressions.
#define FOLLOW_RECALCULATE_RANGE                          2.5f
// This factor defines how much of the bounding-radius (as measurement of size) will be used for recalculating a new following position
//      The smaller, the more micro movement, the bigger, possibly no proper movement updates
#define FOLLOW_RECALCULATE_FACTOR                         1.0f
// This factor defines when the distance of a follower will have impact onto following-position updates
#define FOLLOW_DIST_GAP_FOR_DIST_FACTOR                   1.0f
// This factor defines how much of the follow-distance will be used as sloppyness value (if the above distance is exceeded)
#define FOLLOW_DIST_RECALCULATE_FACTOR                    1.0f

float FollowMovementGenerator::GetDynamicTargetDistance(Unit& owner, bool forRangeCheck) const
{
    if (!forRangeCheck)
        return (GetOffset() + owner.GetObjectBoundingRadius() + i_target->GetObjectBoundingRadius());

    float allowed_dist = (FOLLOW_RECALCULATE_RANGE - i_target->GetObjectBoundingRadius());
    allowed_dist += FOLLOW_RECALCULATE_FACTOR * (owner.GetObjectBoundingRadius() + i_target->GetObjectBoundingRadius());
    if (i_offset > FOLLOW_DIST_GAP_FOR_DIST_FACTOR)
        allowed_dist += FOLLOW_DIST_RECALCULATE_FACTOR * i_offset;

    // Additional leeway for wandering units
    if (i_target->GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
        allowed_dist *= 2;

    return allowed_dist;
}

void FollowMovementGenerator::HandleTargetedMovement(Unit& owner, const uint32& time_diff)
{
    static const MovementFlags detected = MovementFlags(MOVEFLAG_MASK_MOVING_FORWARD | MOVEFLAG_BACKWARD | MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN);
    static const MovementFlags ignored = MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR);

    const bool followerMoving = owner.m_movementInfo.HasMovementFlag(detected);

    // Detect target movement and relocation (ignore jumping in place and long falls)
    const bool targetMovementIgnored = i_target->m_movementInfo.HasMovementFlag(ignored);
    const bool targetMovingLast = m_targetMoving;
    m_targetMoving = (!targetMovementIgnored && i_target->m_movementInfo.HasMovementFlag(detected));

    bool targetRelocation = false;
    bool targetOrientation = false;
    bool targetSpeedChanged = (i_speedChanged && m_targetMoving && targetMovingLast && followerMoving);
    i_speedChanged = false;

    if (m_targetMoving && !targetMovingLast)                        // Movement just started: force update
        targetRelocation = !followerMoving;
    else if (!m_targetMoving && targetMovingLast && followerMoving) // Movement just ended: delay update further
        i_recheckDistance.Reset(_getPollRate(owner, m_targetMoving, targetMovingLast));
    else                                                            // Periodic dist poll: fast when moving, slow when stationary
    {
        i_recheckDistance.Update(time_diff);

        if (!targetMovementIgnored && (i_recheckDistance.Passed() || targetSpeedChanged))
        {
            i_recheckDistance.Reset(_getPollRate(owner, m_targetMoving, targetMovingLast));

            G3D::Vector3 currentTargetPos;
            i_target->GetPosition(currentTargetPos.x, currentTargetPos.y, currentTargetPos.z, owner.GetTransport());

            Position actualPos = owner.GetPosition(owner.GetTransport());

            // Tracking PC units requires additional orientation tick
            if (i_target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            {
                targetRelocation = (currentTargetPos != i_lastTargetPos || RequiresNewPosition(owner, actualPos));
                targetOrientation = (!targetRelocation && !m_targetMoving && !m_targetFaced);
            }
            // Tracking NPC units has no orientation tick and relaxed tracking for wandering targets
            else
            {
                if (i_target->GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
                    targetRelocation = RequiresNewPosition(owner, actualPos);
                else
                    targetRelocation = (currentTargetPos != i_lastTargetPos || RequiresNewPosition(owner, actualPos));
            }

            targetSpeedChanged = (targetSpeedChanged && targetRelocation);
            i_lastTargetPos = currentTargetPos;
        }
    }

    // Decide whether it's suitable time to update position or orientation
    if (targetRelocation || targetSpeedChanged)
    {
        i_recheckDistance.Reset(_getPollRate(owner, m_targetMoving, targetMovingLast));
        _setLocation(owner, m_targetMoving);
    }
    else if (targetOrientation && !i_faceTarget && i_targetReached)
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
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::Update(Unit&, const uint32&);
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::Update(Unit&, const uint32&);
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::IsReachable() const;
template bool TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::RequiresNewPosition(Unit& owner, Position pos) const;
template bool TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>::RequiresNewPosition(Unit& owner, Position pos) const;
