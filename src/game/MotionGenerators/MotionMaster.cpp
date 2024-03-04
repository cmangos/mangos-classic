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

#include "MotionMaster.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "MotionGenerators/PathMovementGenerator.h"
#include "MotionGenerators/PointMovementGenerator.h"
#include "MotionGenerators/RandomMovementGenerator.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "MotionGenerators/WaypointMovementGenerator.h"
#include "MotionGenerators/WrapperMovementGenerator.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"
#include "Maps/GridDefines.h"
#include "Maps/Map.h"
#include "AI/CreatureAISelector.h"
#include "Entities/Creature.h"
#include "Entities/CreatureLinkingMgr.h"
#include "Entities/Player.h"
#include "Entities/Pet.h"
#include "Server/DBCStores.h"
#include "Log/Log.h"

#ifdef BUILD_METRICS
 #include "Metric/Metric.h"
#endif

#include <cassert>

namespace
{
    // Minimum falling distance required to launch a FallMovement generator.
    constexpr float g_moveFallMinFallDistance = 0.5f;
}

inline bool isStatic(MovementGenerator* mv)
{
    return (mv == &si_idleMovement);
}

void MotionMaster::Initialize()
{
#ifdef BUILD_METRICS
    metric::duration<std::chrono::microseconds> meas("motionmaster.initialize", {
        { "entry", std::to_string(m_owner->GetEntry()) },
        { "guid", std::to_string(m_owner->GetGUIDLow()) },
        { "unit_type", std::to_string(m_owner->GetGUIDHigh()) },
        { "map_id", std::to_string(m_owner->GetMapId()) },
        { "instance_id", std::to_string(m_owner->GetInstanceId()) }
    }, 1000);
#endif
    // stop current move
    m_owner->StopMoving();

    // clear ALL movement generators (including default)
    Clear(false, true);

    // set new default movement generator
    if (m_owner->IsCreature() && !m_owner->hasUnitState(UNIT_STAT_POSSESSED))
    {
        auto creature = static_cast<Creature*>(m_owner);
        m_currentPathId = m_defaultPathId;
        MovementGenerator* movement = FactorySelector::selectMovementGenerator(creature);
        push(movement == nullptr ? &si_idleMovement : movement);
        top()->Initialize(*m_owner);

        if (top()->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE || top()->GetMovementGeneratorType() == LINEAR_WP_MOTION_TYPE)
        {
            // check if creature is part of formation and use waypoint path as path origin
            WaypointPathOrigin pathOrigin = WaypointPathOrigin::PATH_NO_PATH;
            auto creatureGroup = creature->GetCreatureGroup();
            if (creatureGroup && creatureGroup->GetFormationEntry() && creatureGroup->GetGroupEntry().GetFormationSlotId(m_owner->GetDbGuid()) == 0)
            {
                m_currentPathId = creatureGroup->GetFormationEntry()->MovementIdOrWander;
                pathOrigin = WaypointPathOrigin::PATH_FROM_WAYPOINT_PATH;
            }

            (static_cast<WaypointMovementGenerator<Creature>*>(top()))->InitializeWaypointPath(*creature, m_currentPathId, pathOrigin, 0);
        }
    }
    else
        push(&si_idleMovement);
}

MotionMaster::~MotionMaster()
{
    // just deallocate movement generator, but do not Finalize since it may access to already deallocated owner's memory
    while (!empty())
    {
        MovementGenerator* m = top();
        pop();
        if (!isStatic(m))
            delete m;
    }
}

void MotionMaster::UpdateMotion(uint32 diff)
{
    if (m_owner->hasUnitState(UNIT_STAT_CAN_NOT_MOVE))
        return;
#ifdef BUILD_METRICS
    metric::duration<std::chrono::microseconds> meas("motionmaster.updatemotion", {
        { "entry", std::to_string(m_owner->GetEntry()) },
        { "guid", std::to_string(m_owner->GetGUIDLow()) },
        { "unit_type", std::to_string(m_owner->GetGUIDHigh()) },
        { "map_id", std::to_string(m_owner->GetMapId()) },
        { "instance_id", std::to_string(m_owner->GetInstanceId()) }
    }, 1000);
#endif

    MANGOS_ASSERT(!empty());
    m_cleanFlag |= MMCF_UPDATE;

    if (!top()->Update(*m_owner, diff))
    {
        m_cleanFlag &= ~MMCF_UPDATE;
        MovementExpired();
    }
    else
        m_cleanFlag &= ~MMCF_UPDATE;

    if (m_expList)
    {
        for (auto mg : *m_expList)
        {
            if (!isStatic(mg))
                delete mg;
        }

        delete m_expList;
        m_expList = nullptr;

        if (empty())
            Initialize();

        if (m_cleanFlag & MMCF_RESET)
        {
            top()->Reset(*m_owner);
            m_cleanFlag &= ~MMCF_RESET;
        }
    }
}

void MotionMaster::DirectClean(bool reset, bool all)
{
    while (all ? !empty() : size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        curr->Finalize(*m_owner);

        if (!isStatic(curr))
            delete curr;
    }

    if (!all && reset)
    {
        MANGOS_ASSERT(!empty());
        top()->Reset(*m_owner);
    }
}

void MotionMaster::DelayedClean(bool reset, bool all)
{
    if (reset)
        m_cleanFlag |= MMCF_RESET;
    else
        m_cleanFlag &= ~MMCF_RESET;

    if (empty() || (!all && size() == 1))
        return;

    if (!m_expList)
        m_expList = new ExpireList();

    while (all ? !empty() : size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        curr->Finalize(*m_owner);

        if (!isStatic(curr))
            m_expList->push_back(curr);
    }
}

void MotionMaster::DirectExpire(bool reset)
{
    if (empty() || size() == 1)
        return;

    MovementGenerator* curr = top();
    bool onlyRemoveOne = false; // this is an ugly hack and is indicative of a need for a rework
    if (curr->GetMovementGeneratorType() == EFFECT_MOTION_TYPE)
        onlyRemoveOne = true;
    pop();

    // also drop stored under top() targeted motions
    if (!onlyRemoveOne)
    {
        while (!empty() && (top()->IsRemovedOnExpire()))
        {
            MovementGenerator* temp = top();
            pop();
            temp->Finalize(*m_owner);
            delete temp;
        }
    }

    // Store current top MMGen, as Finalize might push a new MMGen
    MovementGenerator* nowTop = empty() ? nullptr : top();
    // it can add another motions instead
    curr->Finalize(*m_owner);

    if (!isStatic(curr))
        delete curr;

    if (empty())
        Initialize();

    // Prevent reseting possible new pushed MMGen
    if (reset && top() == nowTop)
        top()->Reset(*m_owner);
}

void MotionMaster::DelayedExpire(bool reset)
{
    if (reset)
        m_cleanFlag |= MMCF_RESET;
    else
        m_cleanFlag &= ~MMCF_RESET;

    if (empty() || size() == 1)
        return;

    MovementGenerator* curr = top();
    pop();

    if (!m_expList)
        m_expList = new ExpireList();

    // also drop stored under top() targeted motions
    while (!empty() && (top()->IsRemovedOnExpire()))
    {
        MovementGenerator* temp = top();
        pop();
        temp ->Finalize(*m_owner);
        m_expList->push_back(temp);
    }

    curr->Finalize(*m_owner);

    if (!isStatic(curr))
        m_expList->push_back(curr);
}

void MotionMaster::MoveIdle()
{
    if (empty() || !isStatic(top()))
        push(&si_idleMovement);
}

void MotionMaster::MoveRandomAroundPoint(float x, float y, float z, float radius, float verticalZ, uint32 timer)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("%s attempt to move random.", m_owner->GetGuidStr().c_str());
    }
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s move random.", m_owner->GetGuidStr().c_str());
        if (timer)
            Mutate(new TimedWanderMovementGenerator(timer, x, y, z, radius, verticalZ));
        else
            Mutate(new WanderMovementGenerator(x, y, z, radius, verticalZ));
    }
}

void MotionMaster::MoveTargetedHome(bool runHome)
{
    if (m_owner->hasUnitState(UNIT_STAT_LOST_CONTROL))
        return;

    Clear(false);

    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s targeted home", m_owner->GetGuidStr().c_str());
        Mutate(new HomeMovementGenerator<Creature>(runHome));
    }
    else
        sLog.outError("%s attempt targeted home", m_owner->GetGuidStr().c_str());
}

void MotionMaster::MoveConfused()
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s move confused", m_owner->GetGuidStr().c_str());

    Mutate(new ConfusedMovementGenerator(*m_owner));
}

void MotionMaster::MoveChase(Unit* target, float dist, float angle, bool moveFurther, bool walk, bool combat, bool delayed)
{
    // ignore movement request if target not exist
    if (!target)
        return;

    if (GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        auto gen = (ChaseMovementGenerator*)top();
        gen->SetOffsetAndAngle(dist, angle, moveFurther);
        gen->SetNewTarget(*target);
        if (delayed)
            gen->AddDelay();
        return;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s chase to %s", m_owner->GetGuidStr().c_str(), target->GetGuidStr().c_str());

    Mutate(new ChaseMovementGenerator(*target, dist, angle, moveFurther, walk, combat));
}

void MotionMaster::DistanceYourself(float dist)
{
    if (GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        auto gen = (ChaseMovementGenerator*)top();
        gen->DistanceYourself(*m_owner, dist);
        return;
    }
}

void MotionMaster::MoveFollow(Unit* target, float dist, float angle, bool asMain/* = false*/, bool alwaysBoost/* = false*/)
{
    if (m_owner->hasUnitState(UNIT_STAT_LOST_CONTROL))
        return;

    if (asMain)
        Clear(false, true);
    else
        Clear(!empty()); // avoid resetting if we are already empty

    // ignore movement request if target not exist
    if (!target)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s follow to %s", m_owner->GetGuidStr().c_str(), target->GetGuidStr().c_str());

    Mutate(new FollowMovementGenerator(*target, dist, angle, asMain, (m_owner->IsPlayer() && !m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED)), alwaysBoost));
}

void MotionMaster::MoveInFormation(FormationSlotDataSPtr& sData, bool asMain /*= false*/)
{
    if (m_owner->hasUnitState(UNIT_STAT_LOST_CONTROL))
        return;

    if (asMain)
        Clear(false, true);
    else
        Clear(!empty()); // avoid resetting if we are already empty

    auto master = sData->GetMaster();
    // ignore movement request if target not exist
    if (!sData->GetOwner() || !master)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s is in formation with %s", m_owner->GetGuidStr().c_str(), master->GetGuidStr().c_str());
    //sLog.outString("%s is in formation with %s", m_owner->GetGuidStr().c_str(), master->GetGuidStr().c_str());

    Mutate(new FormationMovementGenerator(sData, asMain));
}

void MotionMaster::MoveStay(float x, float y, float z, float o, bool asMain)
{
    if (m_owner->hasUnitState(UNIT_STAT_LOST_CONTROL))
        return;

    if (asMain)
        Clear(false, true);
    else
        Clear(!empty()); // avoid resetting if we are already empty

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s stays at position (X: %f Y: %f Z: %f)", m_owner->GetGuidStr().c_str(), x, y, z);

    Mutate(new StayMovementGenerator(x, y, z, o));
}

void MotionMaster::MovePoint(uint32 id, Position const& position, ForcedMovement forcedMovement/* = FORCED_MOVEMENT_NONE*/, float speed/* = 0.f*/, bool generatePath/* = true*/, ObjectGuid guid/* = ObjectGuid()*/, uint32 relayId/* = 0*/)
{
    Mutate(new PointMovementGenerator(id, position.x, position.y, position.z, position.o, generatePath, forcedMovement, speed, guid, relayId));
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, ForcedMovement forcedMovement/* = FORCED_MOVEMENT_NONE*/, bool generatePath/* = true*/)
{
    return MovePoint(id, Position(x, y, z, 0.f), forcedMovement, 0.f, generatePath);
}

void MotionMaster::MovePointTOL(uint32 id, float x, float y, float z, bool takeOff, ForcedMovement forcedMovement/* = FORCED_MOVEMENT_NONE*/)
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s targeted point for %s (Id: %u X: %f Y: %f Z: %f)", m_owner->GetGuidStr().c_str(), takeOff ? "liftoff" : "landing", id, x, y, z);
    Mutate(new PointTOLMovementGenerator(id, x, y, z, takeOff, forcedMovement));
}

void MotionMaster::MovePath(std::vector<G3D::Vector3>& path, ForcedMovement forcedMovement, bool flying, bool cyclic)
{
    return MovePath(path, 0, forcedMovement, flying, cyclic);
}

void MotionMaster::MovePath(std::vector<G3D::Vector3>& path, float o, ForcedMovement forcedMovement, bool flying, bool cyclic)
{
    if (path.empty())
        return;

    const auto& dest = path.back();
    const float x = dest.x, y = dest.y, z = dest.z;

    if (o != 0.f)
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s follows a pre-calculated path to X: %f Y: %f Z: %f O: %f", m_owner->GetGuidStr().c_str(), x, y, z, o);
    else
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s follows a pre-calculated path to X: %f Y: %f Z: %f", m_owner->GetGuidStr().c_str(), x, y, z);

    Mutate(new FixedPathMovementGenerator(path, o, forcedMovement, flying, 0.0f, 0, cyclic));
}

void MotionMaster::MovePath(int32 pathId, WaypointPathOrigin wpOrigin /*= PATH_NO_PATH*/, ForcedMovement forcedMovement, bool flying, float speed, bool cyclic, ObjectGuid guid/* = ObjectGuid()*/)
{
    m_currentPathId = pathId;
    Mutate(new FixedPathMovementGenerator(*m_owner, pathId, wpOrigin, forcedMovement, flying, speed, 0, cyclic, guid));
}

void MotionMaster::MoveRetreat(float x, float y, float z, float o, uint32 delay)
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s retreats for assistance (X: %f Y: %f Z: %f)", m_owner->GetGuidStr().c_str(), x, y, z);
    Mutate(new RetreatMovementGenerator(x, y, z, o, delay));
}

void MotionMaster::MoveFleeing(Unit* source, uint32 time)
{
    if (!source)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s flee from %s", m_owner->GetGuidStr().c_str(), source->GetGuidStr().c_str());

    if (time)
        Mutate(new PanicMovementGenerator(*source, time));
    else
        Mutate(new FleeingMovementGenerator(*source));
}

void MotionMaster::MoveWaypoint(uint32 pathId /*= 0*/, uint32 source /*= 0*/, uint32 initialDelay /*= 0*/, uint32 overwriteEntry /*= 0*/, ForcedMovement forcedMovement /*= FORCED_MOVEMENT_NONE*/, ObjectGuid guid /*= ObjectGuid()*/)
{
    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        {
            sLog.outError("%s attempt to MoveWaypoint() but is already using waypoint", m_owner->GetGuidStr().c_str());
            return;
        }

        Creature* creature = (Creature*)m_owner;
        m_currentPathId = pathId;

        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s start MoveWaypoint()", m_owner->GetGuidStr().c_str());
        WaypointMovementGenerator<Creature>* newWPMMgen = new WaypointMovementGenerator<Creature>(*creature);
        newWPMMgen->SetForcedMovement(forcedMovement);
        newWPMMgen->SetGuid(guid);
        Mutate(newWPMMgen);
        newWPMMgen->InitializeWaypointPath(*creature, pathId, (WaypointPathOrigin)source, initialDelay, overwriteEntry);
    }
    else
    {
        sLog.outError("Non-creature %s attempt to MoveWaypoint()", m_owner->GetGuidStr().c_str());
    }
}

void MotionMaster::MoveLinearWP(uint32 pathId /*= 0*/, uint32 source /*= 0*/, uint32 initialDelay /*= 0*/, uint32 overwriteEntry /*= 0*/, ForcedMovement forcedMovement /*= FORCED_MOVEMENT_NONE*/, ObjectGuid guid /*= ObjectGuid()*/)
{
    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (GetCurrentMovementGeneratorType() == LINEAR_WP_MOTION_TYPE)
        {
            sLog.outError("%s attempt to MoveWaypoint() but is already using waypoint", m_owner->GetGuidStr().c_str());
            return;
        }

        Creature* creature = (Creature*)m_owner;
        m_currentPathId = pathId;

        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s start MoveWaypoint()", m_owner->GetGuidStr().c_str());
        LinearWPMovementGenerator<Creature>* newWPMMgen = new LinearWPMovementGenerator<Creature>(*creature);
        newWPMMgen->SetForcedMovement(forcedMovement);
        newWPMMgen->SetGuid(guid);
        Mutate(newWPMMgen);
        newWPMMgen->InitializeWaypointPath(*creature, pathId, (WaypointPathOrigin)source, initialDelay, overwriteEntry);
    }
    else
    {
        sLog.outError("Non-creature %s attempt to MoveLinearWP()", m_owner->GetGuidStr().c_str());
    }
}

void MotionMaster::MoveTaxi()
{
    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        sLog.outError("%s can't be in taxi flight", m_owner->GetGuidStr().c_str());
        return;
    }

    if (GetCurrentMovementGeneratorType() == TAXI_MOTION_TYPE)
    {
        static_cast<TaxiMovementGenerator*>(top())->Resume(*m_owner);
        return;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s is now in taxi flight", m_owner->GetGuidStr().c_str());

    Mutate(new TaxiMovementGenerator());
}

void MotionMaster::MoveDistract(uint32 timer)
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s distracted (timer: %u)", m_owner->GetGuidStr().c_str(), timer);
    DistractMovementGenerator* mgen = new DistractMovementGenerator(timer);
    Mutate(mgen);
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed, uint32 id/* = EVENT_CHARGE*/)
{
    if (m_owner->hasUnitState(UNIT_STAT_NO_FREE_MOVE))
        return;

    Movement::MoveSplineInit init(*m_owner);
    init.SetWalk(false);
    init.SetVelocity(speed);
    init.MoveTo(x, y, z, true);

    Mutate(new EffectMovementGenerator(init, id, false));
}

void MotionMaster::MoveCharge(Unit& target, float speed, uint32 id/* = EVENT_CHARGE*/)
{
    if (m_owner->hasUnitState(UNIT_STAT_NO_FREE_MOVE))
        return;

    Position pos;
    target.GetFirstCollisionPosition(pos, target.GetCombatReach(), target.GetAngle(m_owner));

    pos.z += 1; // blizzlike
    Movement::MoveSplineInit init(*m_owner);
    init.SetWalk(false);
    init.SetVelocity(speed);
    init.SetFacing(&target);
    init.MoveTo(pos.x, pos.y, pos.z, true);

    Mutate(new EffectMovementGenerator(init, id, false));
}

bool MotionMaster::MoveFall(ObjectGuid guid/* = ObjectGuid()*/, uint32 relayId/* = 0*/)
{
    const float x = m_owner->GetPositionX(), y = m_owner->GetPositionY(), z = m_owner->GetPositionZ();

    // use larger distance for vmap height search than in most other cases
    float tz = m_owner->GetMap()->GetHeight(x, y, z);

    if (tz <= INVALID_HEIGHT)
    {
        DEBUG_LOG("MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f).", m_owner->GetMap()->GetId(), x, y, z);
        return false;
    }

    // Abort too if the ground is very near
    if (fabs(z - tz) < g_moveFallMinFallDistance)
        return false;

    Movement::MoveSplineInit init(*m_owner);
    init.MoveTo(x, y, tz);
    init.SetFall();
    Mutate(new FallMovementGenerator(init, EVENT_FALL, false, guid, relayId));
    return true;
}

void MotionMaster::Mutate(MovementGenerator* m)
{
    if (!empty())
    {
        switch (top()->GetMovementGeneratorType())
        {
            // HomeMovement is not that important, delete it if meanwhile a new comes
            case HOME_MOTION_TYPE:
            // DistractMovement interrupted by any other movement
            case DISTRACT_MOTION_TYPE:
                MovementExpired(false);
            default:
                break;
        }

        if (!empty())
            top()->Interrupt(*m_owner);
    }

    m->Initialize(*m_owner);
    push(m);
}

void MotionMaster::InterruptPanic()
{
    if (!empty())
        if (top()->GetMovementGeneratorType() == TIMED_FLEEING_MOTION_TYPE)
            MovementExpired(false);
}

void MotionMaster::PauseWaypoints(uint32 time)
{
    if (!m_owner->IsCreature())
        return;

    m_owner->StopMoving();

    if (time == 0) // permanently
        static_cast<Creature*>(m_owner)->addUnitState(UNIT_STAT_WAYPOINT_PAUSED);
    else if (GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
    {
        auto gen = (WaypointMovementGenerator<Creature>*)top();
        gen->AddToWaypointPauseTime(time, true);
        return;
    }
    else if (GetCurrentMovementGeneratorType() == PATH_MOTION_TYPE)
    {
        auto gen = (FixedPathMovementGenerator*)top();
        gen->AddToPathPauseTime(time, true);
        return;
    }
    else if (GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
    {
        auto gen = (WanderMovementGenerator*)top();
        gen->AddToRandomPauseTime(time, true);
        return;
    }
}

void MotionMaster::UnpauseWaypoints()
{
    if (!m_owner->IsCreature())
        return;

    static_cast<Creature*>(m_owner)->clearUnitState(UNIT_STAT_WAYPOINT_PAUSED);
    if (GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
    {
        auto gen = (WaypointMovementGenerator<Creature>*)top();
        gen->AddToWaypointPauseTime(0, true);
        return;
    }
    else if (GetCurrentMovementGeneratorType() == PATH_MOTION_TYPE)
    {
        auto gen = (FixedPathMovementGenerator*)top();
        gen->AddToPathPauseTime(0, true);
        return;
    }
}

void MotionMaster::UnMarkFollowMovegens()
{
    Impl::container_type::iterator it = Impl::c.begin();
    for (; it != end(); ++it)
    {
        if ((*it)->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        {
            static_cast<FollowMovementGenerator*>((*it))->MarkMovegen();
        }
    }
    if (top()->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        MovementExpired();
}

void MotionMaster::propagateSpeedChange()
{
    Impl::container_type::iterator it = Impl::c.begin();
    for (; it != end(); ++it)
    {
        (*it)->UnitSpeedChanged();
    }
}

bool MotionMaster::SetNextWaypoint(uint32 pointId)
{
    for (Impl::container_type::reverse_iterator rItr = Impl::c.rbegin(); rItr != Impl::c.rend(); ++rItr)
    {
        if ((*rItr)->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            return (static_cast<WaypointMovementGenerator<Creature>*>(*rItr))->SetNextWaypoint(pointId);
    }
    return false;
}

uint32 MotionMaster::getLastReachedWaypoint() const
{
    for (Impl::container_type::const_reverse_iterator rItr = Impl::c.rbegin(); rItr != Impl::c.rend(); ++rItr)
    {
        if ((*rItr)->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            return (static_cast<WaypointMovementGenerator<Creature>*>(*rItr))->getLastReachedWaypoint();
    }
    return 0;
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
    if (empty())
        return IDLE_MOTION_TYPE;

    return top()->GetMovementGeneratorType();
}

void MotionMaster::GetWaypointPathInformation(std::ostringstream& oss) const
{
    for (Impl::container_type::const_reverse_iterator rItr = Impl::c.rbegin(); rItr != Impl::c.rend(); ++rItr)
    {
        if ((*rItr)->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        {
            static_cast<WaypointMovementGenerator<Creature>*>(*rItr)->GetPathInformation(oss);
            return;
        }
    }
}

bool MotionMaster::GetDestination(float& x, float& y, float& z) const
{
    if (m_owner->movespline->Finalized())
        return false;

    const G3D::Vector3& dest = m_owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}
