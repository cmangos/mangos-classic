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

#include "MotionGenerators/PathMovementGenerator.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"
#include "Entities/Creature.h"
#include "Entities/TemporarySpawn.h"
#include "AI/BaseAI/UnitAI.h"
#include "Entities/Player.h"

#include <algorithm>

AbstractPathMovementGenerator::AbstractPathMovementGenerator(const Movement::PointsArray& path, float orientation, int32 offset/* = 0*/) :
    m_pathIndex(offset), m_orientation(orientation), m_speedChanged(false), m_firstCycle(false), m_startPoint(0)
{
    for (size_t i = 0; i < path.size(); ++i)
        m_path[i] = { path[i].x, path[i].y, path[i].z, ((i + 1) == path.size() ? orientation : 0), 0, 0 };

    // Current spline points array and spline end orientation are set in Initialize() method
    m_spline.reserve(m_path.size());
}

AbstractPathMovementGenerator::AbstractPathMovementGenerator(const WaypointPath* path, int32 offset/* = 0*/, bool cyclic/* = false*/, ObjectGuid guid /*= ObjectGuid()*/) :
    m_pathIndex(offset), m_orientation(0), m_speedChanged(false), m_cyclic(cyclic), m_firstCycle(false), m_startPoint(0), m_guid(guid)
{
    if (!path)
        return;
    // NOTE:
    // Data in WaypointManager can be removed at run time, we cant really reference it and have to make a copy just in case
    // Possibly remake storage in WaypointManager with smart pointers so we can reference it instead
    m_path = *path;
    for (auto& data : m_path)
    {
        if (data.second.delay)
        {
            m_cyclic = false;
            break;
        }
    }

    // Current spline points array, spline end orientation, and spline point index are set in Initialize() method
    m_spline.reserve(m_path.size());
}

void AbstractPathMovementGenerator::Initialize(Unit& unit)
{
    // Stop any previously dispatched splines no matter the source
    if (!unit.movespline->Finalized())
    {
        if (unit.IsClientControlled())
            unit.StopMoving(true);
        else
            unit.InterruptMoving();
    }

    if (m_path.empty())
    {
        sLog.outError("AbstractPathMovementGenerator::Initialize Path empty for unit name %s entry %u dbguid %u.", unit.GetName(), unit.GetEntry(), unit.GetDbGuid());
        return;
    }

    // Reload current spline points array and end orientation from path:
    m_spline.clear();
    m_spline.push_back(Vector3()); // will be set to current position

    if (!m_cyclic)
    {
        for (auto itr = std::next(m_path.begin(), m_pathIndex); itr != m_path.end(); ++itr)
        {
            auto& node = (*itr).second;
            m_spline.push_back({ node.x, node.y, node.z });

            if ((node.delay || std::next(itr, 1) == m_path.end()))
            {
                m_timer.Reset(node.delay);
                m_orientation = node.orientation;
                break;
            }
        }
    }
    else // start at last point and cycle through it all
    {
        uint32 count = 0;
        for (auto itr = std::next(m_path.begin(), m_pathIndex); itr != m_path.end(); ++itr, ++count)
        {
            auto& node = (*itr).second;
            m_spline.push_back({ node.x, node.y, node.z });
        }
        if (count < m_path.size())
        {
            for (auto itr = m_path.begin(); itr != m_path.end() && count < m_path.size(); ++itr, ++count)
            {
                auto& node = (*itr).second;
                m_spline.push_back({ node.x, node.y, node.z });
            }
        }
    }

    m_speedChanged = false;
}

void AbstractPathMovementGenerator::Finalize(Unit& unit)
{
    // Stop any previously dispatched splines no matter the source
    if (!unit.movespline->Finalized())
    {
        if (unit.IsClientControlled())
            unit.StopMoving(true);
        else
            unit.InterruptMoving();
    }
}

void AbstractPathMovementGenerator::Interrupt(Unit& unit)
{
    unit.InterruptMoving();
}

void AbstractPathMovementGenerator::Reset(Unit& unit)
{
    Initialize(unit);
}

bool AbstractPathMovementGenerator::Update(Unit& unit, const uint32& diff)
{
    if (!m_spline.empty())
    {
        // Foolproofing if current spline was tampered with or ran out of splines in the path:
        if (unit.movespline->FinalDestination() != m_spline.back())
        {
            if (unit.movespline->Finalized()) // stopped movement evidence
                Initialize(unit);
            else
                return false;
        }

        if (m_cyclic)
        {
            for (; m_pathIndex != unit.movespline->currentPathIdx();)
            {
                if (!m_firstCycle || m_pathIndex != 0)
                    MovementInform(unit);
                m_pathIndex = (m_pathIndex + 1) % (m_path.size() + 1);
                if (unit.movespline->Finalized())
                    return true;
            }
            if (m_pathIndex == m_startPoint)
                m_firstCycle = false;
        }
        else
        {
            for (; m_pathIndex < unit.movespline->currentPathIdx(); ++m_pathIndex)
                MovementInform(unit);
        }

        if (!unit.movespline->Finalized())
        {
            if (m_speedChanged)
                Initialize(unit);

            return true;
        }
    }

    m_timer.Update(diff);

    // Wait for timer:
    if (!m_timer.Passed())
        return true;

    // Load next spline from the path if still needed
    if (size_t(m_pathIndex) < m_path.size())
    {
        Initialize(unit);
        return true;
    }
    return false;
}

void AbstractPathMovementGenerator::MovementInform(Unit& unit)
{
    const uint32 index = (*std::next(m_path.begin(), ((m_pathIndex + (m_firstCycle ? 0 : 1)) % m_path.size()))).first;

    WaypointNode const& node = m_path[index];

    if (node.script_id)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Creature movement start script %u at point %u for %s.", node.script_id, index, unit.GetGuidStr().c_str());
        unit.GetMap()->ScriptsStart(sCreatureMovementScripts, node.script_id, &unit, &unit);
    }

    // unit.SummonCreature(1, node.x, node.y, node.z, node.orientation, TEMPSPAWN_TIMED_DESPAWN, 5000);

    MovementGeneratorType const type = GetMovementGeneratorType();

    if (UnitAI* ai = unit.AI())
        ai->MovementInform(type, index);

    if (unit.GetTypeId() == TYPEID_UNIT && static_cast<Creature&>(unit).IsTemporarySummon())
    {
        if (unit.GetSpawnerGuid().IsCreatureOrPet())
        {
            if (Creature* pSummoner = unit.GetMap()->GetAnyTypeCreature(unit.GetSpawnerGuid()))
            {
                if (UnitAI* ai = pSummoner->AI())
                    ai->SummonedMovementInform(static_cast<Creature*>(&unit), type, index);
            }
        }
    }
}

FixedPathMovementGenerator::FixedPathMovementGenerator(Unit& creature, int32 pathId, WaypointPathOrigin wpOrigin, ForcedMovement forcedMovement, bool flying, float speed, int32 offset, bool cyclic, ObjectGuid guid) :
    AbstractPathMovementGenerator((pathId || wpOrigin != PATH_NO_PATH ? sWaypointMgr.GetPathFromOrigin(creature.GetEntry(), creature.GetDbGuid(), pathId, (wpOrigin == PATH_NO_PATH && pathId ? PATH_FROM_ENTRY : wpOrigin))
        : sWaypointMgr.GetDefaultPath(creature.GetEntry(), creature.GetDbGuid())), offset, cyclic, guid), m_flying(flying), m_speed(speed), m_forcedMovement(forcedMovement)
{
}

FixedPathMovementGenerator::FixedPathMovementGenerator(Creature& creature) :
    AbstractPathMovementGenerator((creature.GetMotionMaster()->GetPathId() ? sWaypointMgr.GetPathFromOrigin(creature.GetEntry(), creature.GetDbGuid(), creature.GetMotionMaster()->GetPathId(), PATH_FROM_ENTRY)
        : sWaypointMgr.GetDefaultPath(creature.GetEntry(), creature.GetDbGuid())), 0, true), m_flying(true), m_speed(0.f), m_forcedMovement(FORCED_MOVEMENT_NONE)
{
}

void FixedPathMovementGenerator::Initialize(Unit& unit)
{
    unit.addUnitState(UNIT_STAT_ROAMING);

    AbstractPathMovementGenerator::Initialize(unit);

    if (m_spline.size() && Move(unit))
    {
        m_firstCycle = true;
        m_startPoint = m_pathIndex;
        unit.addUnitState(UNIT_STAT_ROAMING_MOVE);
    }
}

void FixedPathMovementGenerator::Finalize(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    AbstractPathMovementGenerator::Finalize(unit);
}

void FixedPathMovementGenerator::Interrupt(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    AbstractPathMovementGenerator::Interrupt(unit);
}

void FixedPathMovementGenerator::Reset(Unit& unit)
{
    unit.addUnitState(UNIT_STAT_ROAMING);
    AbstractPathMovementGenerator::Reset(unit);
}

bool FixedPathMovementGenerator::Move(Unit& unit) const
{
    Movement::MoveSplineInit init(unit);
    init.MovebyPath(m_spline);
    if (m_forcedMovement == FORCED_MOVEMENT_WALK)
        init.SetWalk(true);
    else if (m_forcedMovement == FORCED_MOVEMENT_RUN)
        init.SetWalk(false);
    else
        init.SetWalk(!unit.hasUnitState(UNIT_STAT_RUNNING));
    if (m_flying)
        init.SetFly();
    if (m_orientation != 0.f)
        init.SetFacing(m_orientation);
    if (m_speed != 0.f)
        init.SetVelocity(m_speed);
    if (m_cyclic)
        init.SetCyclic();
    init.SetFirstPointId(m_pathIndex);
    return bool(init.Launch());
}

void FixedPathMovementGenerator::AddToPathPauseTime(int32 waitTimeDiff, bool force)
{
    if (force)
    {
        m_timer.Reset(waitTimeDiff);
        m_spline.clear();
    }
    else if (!m_timer.Passed())
    {
        // creature is stopped already
        // Prevent <= 0, the code in Update requires to catch the change from moving to not moving
        int32 newWaitTime = m_timer.GetExpiry() + waitTimeDiff;
        m_timer.Reset(newWaitTime > 0 ? newWaitTime : 1);
    }
    // TODO: Add AI delay future support
}

void TaxiMovementGenerator::Initialize(Unit& unit)
{
    AbstractPathMovementGenerator::Initialize(unit);

    // Client-controlled unit should have control removed
    if (const Player* controllingClientPlayer = unit.GetClientControlling())
        controllingClientPlayer->UpdateClientControl(&unit, false);

    unit.addUnitState(UNIT_STAT_TAXI_FLIGHT);
    unit.SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_CLIENT_CONTROL_LOST | UNIT_FLAG_TAXI_FLIGHT));
}

void TaxiMovementGenerator::Finalize(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_TAXI_FLIGHT);
    unit.RemoveFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_CLIENT_CONTROL_LOST | UNIT_FLAG_TAXI_FLIGHT));

    if (unit.GetTypeId() == TYPEID_PLAYER)
        unit.RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK);

    // Client-controlled unit should have control restored
    if (const Player* controllingClientPlayer = unit.GetClientControlling())
        controllingClientPlayer->UpdateClientControl(&unit, true);

    AbstractPathMovementGenerator::Finalize(unit);
}

void TaxiMovementGenerator::Interrupt(Unit& unit)
{
    unit.clearUnitState(UNIT_STAT_TAXI_FLIGHT);
    AbstractPathMovementGenerator::Interrupt(unit);
}

bool TaxiMovementGenerator::Update(Unit& unit, const uint32& diff)
{
    if (unit.GetTypeId() != TYPEID_PLAYER)
        return (AbstractPathMovementGenerator::Update(unit, diff) || Resume(unit));

    Player& player = static_cast<Player&>(unit);

    Movement::MoveSpline* spline = player.movespline;
    const bool movement = !spline->Finalized();

    if (!player.OnTaxiFlightUpdate(size_t(spline->currentPathIdx()), movement))
    {
        // Some other code messed with our spline before it was completed. Have it your way, no taxi for you!
        player.OnTaxiFlightEject();
        return false;
    }

    return (movement || Resume(player));
}

#define TAXI_FLIGHT_SPEED        32.0f

bool TaxiMovementGenerator::Move(Unit& unit)
{
    Movement::MoveSplineInit init(unit);
    Movement::PointsArray& spline = init.Path();
    std::copy((m_spline.begin() + m_pathIndex), m_spline.end(), back_inserter(spline));
    init.SetFirstPointId(m_pathIndex);
    init.SetFly();
    init.SetVelocity(TAXI_FLIGHT_SPEED);
    return bool(init.Launch());
}

bool TaxiMovementGenerator::Resume(Unit& unit)
{
    if (unit.GetTypeId() == TYPEID_PLAYER)
    {
        Player& player = static_cast<Player&>(unit);

        if (player.IsMounted())
            player.OnTaxiFlightSplineEnd();

        if (!player.OnTaxiFlightSplineUpdate())
            return false;

        // Load and execute the spline (transitional populating the parent movegen: to be reworked in the future)

        auto nodes = player.GetTaxiPathSpline();
        m_path.clear();
        m_spline.clear();
        m_spline.reserve(nodes.size());

        uint32 i = 0;
        for (auto itr = nodes.begin(); itr != nodes.end(); ++itr, ++i)
        {
            m_path.emplace(i, WaypointNode((*itr)->x, (*itr)->y, (*itr)->z , 0, 0, 0));
            m_spline.push_back({ (*itr)->x, (*itr)->y, (*itr)->z });
        }

        m_pathIndex = player.GetTaxiPathSplineOffset();
    }

    return Move(unit);
}
