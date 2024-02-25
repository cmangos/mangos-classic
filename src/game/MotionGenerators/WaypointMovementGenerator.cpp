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

#include "WaypointMovementGenerator.h"
#include "Globals/ObjectMgr.h"
#include "Entities/Player.h"
#include "Entities/Creature.h"
#include "Entities/TemporarySpawn.h"
#include "AI/BaseAI/CreatureAI.h"
#include "WaypointManager.h"
#include "DBScripts/ScriptMgr.h"
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"
#include "Maps/GridDefines.h"
#include "Entities/Transports.h"

#include <cassert>

//-----------------------------------------------//
void WaypointMovementGenerator<Creature>::LoadPath(Creature& creature, int32 pathId, WaypointPathOrigin wpOrigin, uint32 overwriteEntry)
{
    DETAIL_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "LoadPath: loading waypoint path for %s", creature.GetGuidStr().c_str());

    if (!overwriteEntry)
        overwriteEntry = creature.GetEntry();

    //sLog.outErrorScriptLib("LoadPath: Assign creature guid(%u) and entry(%u) path %i", creature.GetDbGuid(), overwriteEntry, pathId);

    if (wpOrigin == PATH_NO_PATH && pathId == 0)
        i_path = sWaypointMgr.GetDefaultPath(overwriteEntry, creature.GetDbGuid(), &m_PathOrigin);
    else
    {
        m_PathOrigin = wpOrigin == PATH_NO_PATH ? PATH_FROM_ENTRY : wpOrigin;
        i_path = sWaypointMgr.GetPathFromOrigin(overwriteEntry, creature.GetDbGuid(), pathId, m_PathOrigin);
    }
    m_pathId = pathId;

    // No movement found for entry nor guid
    if (!i_path)
    {
        if (m_PathOrigin == PATH_FROM_EXTERNAL)
            sLog.outErrorScriptLib("WaypointMovementGenerator::LoadPath: %s doesn't have waypoint path %i", creature.GetGuidStr().c_str(), pathId);
        else
            sLog.outErrorDb("WaypointMovementGenerator::LoadPath: %s doesn't have waypoint path %i", creature.GetGuidStr().c_str(), pathId);
        return;
    }

    if (i_path->empty())
        return;
    // Initialize the i_currentNode to point to the first node
    i_currentNode = i_path->begin()->first;
    m_lastReachedWaypoint = 0;
    m_currentWaypointNode = i_path->begin();

    if (i_path->size() < 2)
    {
        auto& node = i_path->begin()->second;
        if (node.delay == 0)
        {
            // db dev seem to use one wp for some specific script without delay
            sLog.outErrorDb("WaypointMovementGenerator::LoadPath()> creature %s have only one wp and without any delay. Delay forced to 5 sec!",
                creature.GetGuidStr().c_str());

            // little hack to change db data
            auto modNode = const_cast<WaypointNode&>(node);
            modNode.delay = 5000;
        }
    }
}

void WaypointMovementGenerator<Creature>::Initialize(Creature& creature)
{
    creature.addUnitState(UNIT_STAT_ROAMING);
    creature.clearUnitState(UNIT_STAT_WAYPOINT_PAUSED);
}

void WaypointMovementGenerator<Creature>::InitializeWaypointPath(Creature& u, int32 pathId, WaypointPathOrigin wpSource, uint32 initialDelay, uint32 overwriteEntry/* = 0*/)
{
    LoadPath(u, pathId, wpSource, overwriteEntry);
    i_nextMoveTime.Reset(initialDelay);
    if (!i_path)
    {
        sLog.outErrorDb("void WaypointMovementGenerator<Creature>::InitializeWaypointPath> unable to intialize path for %s", u.GetGuidStr().c_str());
        return;
    }

    i_nextMoveTime.Reset(initialDelay);

    uint32 startPoint = -1;                                 // TODO add an argument to set the start point
    if (startPoint >= 0)
    {
        auto selPoint = i_path->find(startPoint);
        if (selPoint != i_path->end())
        {
            i_currentNode = startPoint;
            m_currentWaypointNode = selPoint;
        }
        else
        {
            m_currentWaypointNode = i_path->begin();
            i_currentNode = m_currentWaypointNode->first;
        }
    }

    // fixup last reached node
    if (m_currentWaypointNode != i_path->begin())
    {
        auto lastNode = m_currentWaypointNode;
        --lastNode;
        m_lastReachedWaypoint = lastNode->first;
    }
    else
        m_lastReachedWaypoint = 0;

    // Start moving if possible
    SendNextWayPointPath(u);
    m_resetPoint = u.GetPosition(u.GetTransport());
}

void WaypointMovementGenerator<Creature>::Finalize(Creature& creature)
{
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

void WaypointMovementGenerator<Creature>::Interrupt(Creature& creature)
{
    // be sure we are not already interrupted before saving current pos
    if (creature.hasUnitState(UNIT_STAT_ROAMING_MOVE))
    {
        // save the current position in case of reset
        m_resetPoint = creature.GetPosition(creature.GetTransport());
    }

    creature.InterruptMoving();
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

void WaypointMovementGenerator<Creature>::Reset(Creature& creature)
{
    creature.addUnitState(UNIT_STAT_ROAMING);
    if (!i_path || Stopped(creature))
        return;
    SendNextWayPointPath(creature);
}

void WaypointMovementGenerator<Creature>::SwitchToNextNode(Creature& creature, WaypointPath::const_iterator& nodeItr)
{
    // switch to next node
    ++nodeItr;
    if (nodeItr == i_path->end())
    {
        nodeItr = i_path->begin();
    }
}

void WaypointMovementGenerator<Creature>::OnArrived(Creature& creature)
{
    // already arrived?
    if (m_lastReachedWaypoint == i_currentNode)
        return;

    m_lastReachedWaypoint = m_currentWaypointNode->first;

    WaypointNode const& node = m_currentWaypointNode->second;

    if (node.script_id)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Creature movement start script %u at point %u for %s.", node.script_id, i_currentNode, creature.GetGuidStr().c_str());
        creature.GetMap()->ScriptsStart(SCRIPT_TYPE_CREATURE_MOVEMENT, node.script_id, &creature, m_guid ? creature.GetMap()->GetWorldObject(m_guid) : &creature);
    }

    // Inform script
    if (creature.AI())
    {
        bool inform = false;
        if (m_PathOrigin == PATH_FROM_EXTERNAL)
            inform = true;
        else if (m_PathOrigin == PATH_FROM_WAYPOINT_PATH) // temporary transitional code for transitioning from script_texts to waypoint_path
        {
            if (m_pathId == creature.AI()->GetCurrentWaypointPath())
                inform = true;
        }

        uint32 type = WAYPOINT_MOTION_TYPE;
        if (inform)
            type = EXTERNAL_WAYPOINT_MOVE;
        InformAI(creature, type, i_currentNode);
    }

    // save position and orientation in case of GetResetPosition() call
    m_resetPoint = creature.GetPosition(creature.GetTransport());

    // AI can modify node delay so we have to compute it
    int32 newWaitTime = node.delay + m_scriptTime;

    // switch to next node
    SwitchToNextNode(creature, m_currentWaypointNode);

    // Wait delay ms
    if (newWaitTime > 0)
    {
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        Stop(newWaitTime);

        // if script modified the node delay we already sent path to client to move the creature so we have to stop it manually
        if (!creature.movespline->Finalized() && node.delay == 0 && m_scriptTime != 0)
        {
            Movement::MoveSplineInit init(creature);
            init.Stop();
        }
    }
    else if (newWaitTime == 0 && node.delay != 0)
    {
        // we have to manually restart movement as script changed the node delay to 0
        SendNextWayPointPath(creature);
    }

    // Fire event only if it is already moving to the next node (for normal start it is already done in SendNewPath)
    if (!m_nodeIndexes.empty() && !Stopped(creature))
    {
        bool inform = false;
        if (m_PathOrigin == PATH_FROM_EXTERNAL)
            inform = true;
        else if (m_PathOrigin == PATH_FROM_WAYPOINT_PATH) // temporary transitional code for transitioning from script_texts to waypoint_path
        {
            if (m_pathId == creature.AI()->GetCurrentWaypointPath())
                inform = true;
        }

        // Inform AI that we start to move
        if (creature.AI() && inform)
            InformAI(creature, uint32(EXTERNAL_WAYPOINT_MOVE_START), m_currentWaypointNode->first);
    }

    i_currentNode = m_currentWaypointNode->first;
    m_scriptTime = 0;
}

// return added travel time - TODO: implement caching of path
uint32 WaypointMovementGenerator<Creature>::BuildIntPath(PointsArray& path, Creature& creature, Vector3 const& endPos)
{
    Vector3 startPos = path.back();
    uint32 travelTime = 0;
    auto speedType = MovementInfo::GetSpeedType(creature.m_movementInfo.GetMovementFlags());
    float creatureSpeed = creature.GetSpeed(speedType);

    PathFinder pathfinder(&creature, true);
    pathfinder.calculate(startPos, endPos, true);
    auto genPath = pathfinder.getPath();

    Vector3 firstPoint = startPos;
    bool first = true;
    for (auto& point : genPath)
    {
        if (first) // first point is already in path
        {
            first = false;
            continue;
        }
        const Vector3 offset = point - firstPoint;
        const float distance = offset.magnitude();
        travelTime += distance / creatureSpeed * 1000;
        path.push_back(point);
    }
    const Vector3 offset = endPos - firstPoint;
    const float distance = offset.magnitude();
    travelTime = distance / creatureSpeed * 1000;

    return travelTime;
}


bool WaypointMovementGenerator<Creature>::GetNodeAfter(WaypointPath::const_iterator& nodeItr, bool)
{
    ++nodeItr;
    if (nodeItr == i_path->end())
        nodeItr = i_path->begin();

    return true;
}

// minimum time that will take the unit to travel the path (much higher value have been seen in sniff)
static const uint32 MinimumPathTime = 6000;
// client need to receive new path before the end of previous path (much higher value have been seen in sniff)
static const uint32 PreSendTime     = 1500;

// build and send path to next node
void WaypointMovementGenerator<Creature>::SendNextWayPointPath(Creature& creature)
{
    // make sure to reset spline index as its new path
    m_nodeIndexes.clear();

    // prevent movement while casting spells with cast time or channel time
    if (!creature.IsAlive() || creature.IsImmobilizedState() || creature.IsNonMeleeSpellCasted(false, false, true, true))
    {
        if (!creature.movespline->Finalized())
        {
            if (creature.IsClientControlled())
                creature.StopMoving(true);
            else
                creature.InterruptMoving();
        }
        return;
    }

    creature.addUnitState(UNIT_STAT_ROAMING_MOVE);
    m_speedChanged = false;

    WaypointNode const* nextNode = &m_currentWaypointNode->second;

    bool inform = false;
    if (m_PathOrigin == PATH_FROM_EXTERNAL)
        inform = true;
    else if (m_PathOrigin == PATH_FROM_WAYPOINT_PATH && creature.AI()) // temporary transitional code for transitioning from script_texts to waypoint_path
    {
        if (m_pathId == creature.AI()->GetCurrentWaypointPath())
            inform = true;
    }

    // Inform AI that we start to move or reached last node
    if (creature.AI() && inform)
    {
        uint32 type = uint32(EXTERNAL_WAYPOINT_MOVE_START);
        if (i_path->begin()->first == i_currentNode && i_path->rbegin()->first == m_lastReachedWaypoint)
            type = uint32(EXTERNAL_WAYPOINT_FINISHED_LAST);

        InformAI(creature, type, i_currentNode);

        if (creature.IsDead() || !creature.IsInWorld()) // Might have happened with above calls
            return;
    }

    // will contain generated path
    PointsArray genPath;
    genPath.reserve(20);    // little optimization

    Vector3 startPos(creature.GetPositionX(), creature.GetPositionY(), creature.GetPositionZ());
    if (!creature.movespline->Finalized())
    {
        if (GenericTransport* transport = creature.GetTransport())
            transport->CalculatePassengerPosition(startPos.x, startPos.y, startPos.z, nullptr);
        startPos = creature.movespline->ComputePosition();
    }

    genPath.push_back(startPos);

    // compute path to next node and put it in the path
    uint32 travelTime = BuildIntPath(genPath, creature, Vector3(nextNode->x, nextNode->y, nextNode->z));

    bool looped = false;
    auto currPointItr = m_currentWaypointNode;
    // add more node until travel time is big enough
    while (!nextNode->delay && travelTime < MinimumPathTime)
    {
        // we'll add path to node after this one too to make animation more smoother
        auto nodeAfterItr = currPointItr;

        if (!GetNodeAfter(nodeAfterItr, looped))
        {
            if (looped) //that should never happen. (Well can but with abnormally short path)
                break;

            // we reach the end of the path better is to retry in the opposite direction
            looped = true;
            if (!GetNodeAfter(nodeAfterItr, true))
                break; // did all the path and not reached MinimumPathTime?
        }

        auto const nodeAfter = nodeAfterItr->second;

        // extend path only if next node is different than current node
        m_nodeIndexes.push_back(genPath.size() - 1);
        Vector3 nodeAfterCoord(nodeAfter.x, nodeAfter.y, nodeAfter.z);
        travelTime += BuildIntPath(genPath, creature, nodeAfterCoord);
        currPointItr = nodeAfterItr;
        nextNode = &nodeAfterItr->second;
    }

    Movement::MoveSplineInit init(creature);
    init.MovebyPath(genPath);
    if (nextNode->orientation != 100 && nextNode->delay != 0)
        init.SetFacing(nextNode->orientation);
    if (m_forcedMovement == FORCED_MOVEMENT_WALK)
        init.SetWalk(true);
    else if (m_forcedMovement == FORCED_MOVEMENT_RUN)
        init.SetWalk(false);
    else
        init.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING));

    // send path to client
    m_pathDuration = init.Launch();

    // remove presend time if next node have no delay
    if (nextNode->delay == 0)
        m_pathDuration -= PreSendTime;
    if (i_path->size() == 1)
        m_lastReachedWaypoint = 0;
}

void WaypointMovementGenerator<Creature>::InformAI(Creature& creature, uint32 type, uint32 data)
{
    creature.AI()->MovementInform(type, data);
    if (creature.IsTemporarySummon())
    {
        if (creature.GetSpawnerGuid().IsCreatureOrPet())
            if (Creature* pSummoner = creature.GetMap()->GetAnyTypeCreature(creature.GetSpawnerGuid()))
                if (pSummoner->AI())
                    pSummoner->AI()->SummonedMovementInform(&creature, type, data);
    }
}

bool WaypointMovementGenerator<Creature>::Update(Creature& creature, const uint32& diff)
{
    // Waypoint movement can be switched on/off
    // This is quite handy for escort quests and other stuff
    if (creature.hasUnitState(UNIT_STAT_NOT_MOVE))
    {
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
    {
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    if (Stopped(creature) || m_speedChanged)
    {
        // If a script just have set the waypoint to be paused or stopped we have to check
        // if the client did get a path for this creature. If it is the case, we have to
        // explicitly send stop so the client knows that we want that creature to be stopped
        if (!creature.movespline->Finalized())
        {
            Movement::MoveSplineInit init(creature);
            init.Stop();
        }

        if (CanMove(diff, creature))
            SendNextWayPointPath(creature);
    }
    else
    {
        if (creature.IsStopped())
        {
            if (creature.IsNonMeleeSpellCasted(false, false, true, true))
                return true;
            Stop(1000);
        }
        else
        {
            if (creature.movespline->Finalized())
            {
                // we arrived to a node either by movespline finalized or node reached while creature continue to move
                OnArrived(creature);                    // fire script events
                if (!Stopped(creature))                 // check if not stopped in OnArrived
                    SendNextWayPointPath(creature);     // restart movement
            }
            else
            {
                if (m_pathDuration <= 0)
                {
                    // time to send new packet
                    if (m_currentWaypointNode->second.delay == 0)
                        SendNextWayPointPath(creature);
                }
                else
                    m_pathDuration -= diff;

                if (!m_nodeIndexes.empty() && creature.movespline->currentPathIdx() >= m_nodeIndexes.front())
                {
                    // node reached while moving
                    m_nodeIndexes.pop_front();
                    OnArrived(creature);                // fire script events
                }
            }
        }
    }

    return true;
}

bool WaypointMovementGenerator<Creature>::Stopped(Creature& u)
{
    return !i_nextMoveTime.Passed() || u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED);
}

bool WaypointMovementGenerator<Creature>::CanMove(int32 diff, Creature& u)
{
    i_nextMoveTime.Update(diff);
    if (i_nextMoveTime.Passed() && u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED))
        i_nextMoveTime.Reset(1);

    return i_nextMoveTime.Passed() && !u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED);
}

bool WaypointMovementGenerator<Creature>::GetResetPosition(Creature&, float& x, float& y, float& z, float& o) const
{
    x = m_resetPoint.x;
    y = m_resetPoint.y;
    z = m_resetPoint.z;
    o = m_resetPoint.o;

    return true;
    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    WaypointPath::const_iterator lastPoint = i_path->find(m_lastReachedWaypoint);
    // Special case: Before the first waypoint is reached, m_lastReachedWaypoint is set to 0 (which may not be contained in i_path)
    if (!m_lastReachedWaypoint && lastPoint == i_path->end())
        return false;

    MANGOS_ASSERT(lastPoint != i_path->end());

    WaypointNode const* curWP = &(lastPoint->second);

    x = curWP->x;
    y = curWP->y;
    z = curWP->z;

    if (curWP->orientation != 100)
        o = curWP->orientation;
    else                                                    // Calculate the resulting angle based on positions between previous and current waypoint
    {
        WaypointNode const* prevWP;
        if (lastPoint != i_path->begin())                   // Not the first waypoint
        {
            --lastPoint;
            prevWP = &(lastPoint->second);
        }
        else                                                // Take the last waypoint (crbegin()) as previous
            prevWP = &(i_path->rbegin()->second);

        float dx = x - prevWP->x;
        float dy = y - prevWP->y;
        o = atan2(dy, dx);                                  // returns value between -Pi..Pi

        o = (o >= 0) ? o : 2 * M_PI_F + o;
    }

    return true;
}

void WaypointMovementGenerator<Creature>::GetPathInformation(std::ostringstream& oss) const
{
    oss << "WaypointMovement: Last Reached WP: " << m_lastReachedWaypoint << " ";
    oss << "(Loaded path " << m_pathId << " from " << WaypointManager::GetOriginString(m_PathOrigin) << ")\n";
}

void WaypointMovementGenerator<Creature>::AddToWaypointPauseTime(int32 waitTimeDiff, bool force)
{
    if (force)
        Stop(waitTimeDiff);
    else if (!i_nextMoveTime.Passed())
    {
        // creature is stopped already
        // Prevent <= 0, the code in Update requires to catch the change from moving to not moving
        int32 newWaitTime = i_nextMoveTime.GetExpiry() + waitTimeDiff;
        i_nextMoveTime.Reset(newWaitTime > 0 ? newWaitTime : 1);
    }
    else
    {
        // creature is not stopped yet (script with 0 delay may be calling this)
        m_scriptTime = waitTimeDiff;
    }
}

bool WaypointMovementGenerator<Creature>::SetNextWaypoint(uint32 pointId)
{
    if (!i_path || i_path->empty())
        return false;

    // point id should not be bigger than the size of the path -1
    pointId = uint32(pointId % i_path->size());

    WaypointPath::const_iterator currPoint = i_path->find(pointId);
    if (currPoint == i_path->end())
        return false;

    // Allow Moving with next tick
    // Handle allow movement this way to not interact with PAUSED state.
    // If this function is called while PAUSED, it will move properly when unpaused.
    i_nextMoveTime.Reset(1);

    // Set the point
    i_currentNode = pointId;
    m_currentWaypointNode = currPoint;

    // set last reached point accordingly to avoid going back to point 0 if the
    // movegen is interrupted before reaching next point
    if (pointId > 0)
        m_lastReachedWaypoint = pointId - 1;
    else
        m_lastReachedWaypoint = i_path->rbegin()->first;
    return true;
}

void LinearWPMovementGenerator<Creature>::SwitchToNextNode(Creature& creature, WaypointPath::const_iterator& nodeItr)
{
    // switch to next node
    if (!m_driveWayBack)
    {
        if (std::next(nodeItr) == i_path->end())
        {
            --nodeItr;

            m_driveWayBack = true;
            // call end of path hook
            if (creature.GetFormationSlot())
                creature.GetFormationSlot()->GetFormationData()->OnWPEndNode();
        }
        else
            ++nodeItr;
    }
    else
    {
        if (nodeItr == i_path->begin())
        {
            m_driveWayBack = false;
            ++nodeItr;
            // call start of path hook
            if (creature.GetFormationSlot())
                creature.GetFormationSlot()->GetFormationData()->OnWPStartNode();
        }
        else
            --nodeItr;
    }
}

bool LinearWPMovementGenerator<Creature>::GetNodeAfter(WaypointPath::const_iterator& nodeItr, bool looped /*= false*/)
{
    bool movingToLastNode = looped ? m_driveWayBack : !m_driveWayBack;

    if (movingToLastNode)
    {
        if (std::next(nodeItr) == i_path->end())
            return false;
        else
            ++nodeItr;
    }
    else
    {
        if (nodeItr == i_path->begin())
            return false;
        else
            --nodeItr;
    }
    return true;
}
