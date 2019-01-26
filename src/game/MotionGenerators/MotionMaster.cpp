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
#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"
#include "Maps/Map.h"
#include "AI/CreatureAISelector.h"
#include "Entities/Creature.h"
#include "Entities/CreatureLinkingMgr.h"
#include "Entities/Player.h"
#include "Entities/Pet.h"
#include "Server/DBCStores.h"
#include "Log.h"

#include <cassert>

inline bool isStatic(MovementGenerator* mv)
{
    return (mv == &si_idleMovement);
}

void MotionMaster::Initialize()
{
    // stop current move
    m_owner->StopMoving();

    // clear ALL movement generators (including default)
    Clear(false, true);

    // set new default movement generator
    if (m_owner->GetTypeId() == TYPEID_UNIT && !m_owner->hasUnitState(UNIT_STAT_POSSESSED))
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator((Creature*)m_owner);
        push(movement == nullptr ? &si_idleMovement : movement);
        top()->Initialize(*m_owner);
        m_currentPathId = m_defaultPathId;
        if (top()->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            (static_cast<WaypointMovementGenerator<Creature>*>(top()))->InitializeWaypointPath(*((Creature*)(m_owner)), m_currentPathId, PATH_NO_PATH, 0, 0);
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
    pop();

    // also drop stored under top() targeted motions
    while (!empty() && (top()->GetMovementGeneratorType() == CHASE_MOTION_TYPE || top()->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE))
    {
        MovementGenerator* temp = top();
        pop();
        temp->Finalize(*m_owner);
        delete temp;
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
    while (!empty() && (top()->GetMovementGeneratorType() == CHASE_MOTION_TYPE || top()->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE))
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

void MotionMaster::MoveRandomAroundPoint(float x, float y, float z, float radius, float verticalZ)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("%s attempt to move random.", m_owner->GetGuidStr().c_str());
    }
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s move random.", m_owner->GetGuidStr().c_str());
        Mutate(new RandomMovementGenerator<Creature>(x, y, z, radius, verticalZ));
    }
}

void MotionMaster::MoveTargetedHome(bool runHome)
{
    if (m_owner->hasUnitState(UNIT_STAT_LOST_CONTROL))
        return;

    Clear(false);

    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (Unit* target = m_owner->GetMaster())
        {
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s follow to %s", m_owner->GetGuidStr().c_str(), target->GetGuidStr().c_str());
            Mutate(new FollowMovementGenerator<Creature>(*target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE));
        }
        // Manual exception for linked mobs
        else if (m_owner->IsLinkingEventTrigger() && m_owner->GetMap()->GetCreatureLinkingHolder()->TryFollowMaster((Creature*)m_owner))
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s refollowed linked master", m_owner->GetGuidStr().c_str());
        else
        {
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s targeted home", m_owner->GetGuidStr().c_str());
            Mutate(new HomeMovementGenerator<Creature>(runHome));
        }
    }
    else
        sLog.outError("%s attempt targeted home", m_owner->GetGuidStr().c_str());
}

void MotionMaster::MoveConfused()
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s move confused", m_owner->GetGuidStr().c_str());

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new ConfusedMovementGenerator<Player>());
    else
        Mutate(new ConfusedMovementGenerator<Creature>());
}

void MotionMaster::MoveChase(Unit* target, float dist, float angle, bool moveFurther, bool walk, bool combat)
{
    // ignore movement request if target not exist
    if (!target)
        return;

    if (GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        auto gen = (ChaseMovementGenerator*)top();
        gen->SetOffsetAndAngle(dist, angle, moveFurther);
        gen->SetNewTarget(*target);
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

void MotionMaster::MoveFollow(Unit* target, float dist, float angle, bool asMain)
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

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new FollowMovementGenerator<Player>(*target, dist, angle));
    else
        Mutate(new FollowMovementGenerator<Creature>(*target, dist, angle));
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, bool generatePath, ForcedMovement forcedMovement)
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s targeted point (Id: %u X: %f Y: %f Z: %f)", m_owner->GetGuidStr().c_str(), id, x, y, z);

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, forcedMovement));
    else
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, forcedMovement));
}

void MotionMaster::MoveSeekAssistance(float x, float y, float z)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("%s attempt to seek assistance", m_owner->GetGuidStr().c_str());
    }
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s seek assistance (X: %f Y: %f Z: %f)",
                         m_owner->GetGuidStr().c_str(), x, y, z);
        Mutate(new AssistanceMovementGenerator(x, y, z));
    }
}

void MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("%s attempt to call distract after assistance", m_owner->GetGuidStr().c_str());
    }
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s is distracted after assistance call (Time: %u)",
                         m_owner->GetGuidStr().c_str(), time);
        Mutate(new AssistanceDistractMovementGenerator(time));
    }
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s flee from %s", m_owner->GetGuidStr().c_str(), enemy->GetGuidStr().c_str());

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetObjectGuid()));
    else
    {
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetObjectGuid(), time));
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetObjectGuid()));
    }
}

void MotionMaster::MoveWaypoint(uint32 pathId /*=0*/, uint32 source /*=0==PATH_NO_PATH*/, uint32 initialDelay /*=0*/, uint32 overwriteEntry /*=0*/)
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
        Mutate(newWPMMgen);
        newWPMMgen->InitializeWaypointPath(*creature, pathId, (WaypointPathOrigin)source, initialDelay, overwriteEntry);
    }
    else
    {
        sLog.outError("Non-creature %s attempt to MoveWaypoint()", m_owner->GetGuidStr().c_str());
    }
}

void MotionMaster::MoveTaxiFlight()
{
    if (m_owner->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    {
        if (m_owner->GetTypeId() == TYPEID_PLAYER)
        {
            auto flightMGen = static_cast<FlightPathMovementGenerator const*>(m_owner->GetMotionMaster()->GetCurrent());
            flightMGen->Resume(*static_cast<Player*>(m_owner));
            return;
        }

        do
        {
            // remove this generator from stack
            m_owner->GetMotionMaster()->MovementExpired(false);
        }
        while (m_owner->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE);

        sLog.outError("%s can't be in taxi flight", m_owner->GetGuidStr().c_str());
        return;
    }

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s is now in taxi flight", m_owner->GetGuidStr().c_str());
        Mutate(new FlightPathMovementGenerator());
    }
    else
        sLog.outError("%s can't be in taxi flight", m_owner->GetGuidStr().c_str());
}

void MotionMaster::MoveDistract(uint32 timer)
{
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s distracted (timer: %u)", m_owner->GetGuidStr().c_str(), timer);
    DistractMovementGenerator* mgen = new DistractMovementGenerator(timer);
    Mutate(mgen);
}

void MotionMaster::MoveFlyOrLand(uint32 id, float x, float y, float z, bool liftOff)
{
    if (m_owner->GetTypeId() != TYPEID_UNIT)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "%s targeted point for %s (Id: %u X: %f Y: %f Z: %f)", m_owner->GetGuidStr().c_str(), liftOff ? "liftoff" : "landing", id, x, y, z);
    Mutate(new FlyOrLandMovementGenerator(id, x, y, z, liftOff));
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

void MotionMaster::InterruptFlee()
{
    if (!empty())
        if (top()->GetMovementGeneratorType() == TIMED_FLEEING_MOTION_TYPE)
            MovementExpired(false);
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

void MotionMaster::MoveFall()
{
    // use larger distance for vmap height search than in most other cases
    float tz = m_owner->GetMap()->GetHeight(m_owner->GetPositionX(), m_owner->GetPositionY(), m_owner->GetPositionZ());
    if (tz <= INVALID_HEIGHT)
    {
        DEBUG_LOG("MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f).",
                  m_owner->GetMap()->GetId(), m_owner->GetPositionX(), m_owner->GetPositionY(), m_owner->GetPositionZ());
        return;
    }

    // Abort too if the ground is very near
    if (fabs(m_owner->GetPositionZ() - tz) < 0.1f)
        return;

    Movement::MoveSplineInit init(*m_owner);
    init.MoveTo(m_owner->GetPositionX(), m_owner->GetPositionY(), tz);
    init.SetFall();
    init.Launch();
}
