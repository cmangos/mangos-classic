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

#ifndef MANGOS_WAYPOINTMOVEMENTGENERATOR_H
#define MANGOS_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "WaypointManager.h"
#include "Server/DBCStructure.h"
#include "Entities/Object.h"

#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  (3 * MINUTE * IN_MILLISECONDS)// 3 Minutes

// forward declaration (declared in MovementSplineInit.h)
namespace Movement
{
    typedef std::vector<G3D::Vector3> PointsArray;
}

template<class T, class P>
class PathMovementBase
{
    public:
        PathMovementBase() : i_currentNode(0) {}
        virtual ~PathMovementBase() {};

        // template pattern, not defined .. override required
        void LoadPath(T&);
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        P i_path;
        uint32 i_currentNode;
};

/** WaypointMovementGenerator loads a series of way points
 * from the DB and apply it to the creature's movement generator.
 * Hence, the creature will move according to its predefined way points.
 */

template<class T>
class WaypointMovementGenerator;

template<>
class WaypointMovementGenerator<Creature>
    : public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
      public PathMovementBase<Creature, WaypointPath const*>
{
    public:
        WaypointMovementGenerator(Creature&) :
            i_nextMoveTime(0), m_scriptTime(0), m_lastReachedWaypoint(0), m_pathId(0),
            m_pathDuration(0), m_PathOrigin(), m_speedChanged(false), m_forcedMovement(FORCED_MOVEMENT_NONE)
        {}
        ~WaypointMovementGenerator() { i_path = nullptr; }
        void Initialize(Creature& creature);
        void Interrupt(Creature&);
        void Finalize(Creature&);
        void Reset(Creature& creature);
        bool Update(Creature& creature, const uint32& diff);
        void InitializeWaypointPath(Creature& u, int32 pathId, WaypointPathOrigin wpSource, uint32 initialDelay, uint32 overwriteEntry = 0);

        MovementGeneratorType GetMovementGeneratorType() const override { return WAYPOINT_MOTION_TYPE; }

        bool GetResetPosition(Creature&, float& /*x*/, float& /*y*/, float& /*z*/, float& /*o*/) const;
        uint32 getLastReachedWaypoint() const { return m_lastReachedWaypoint; }
        void GetPathInformation(uint32& pathId, WaypointPathOrigin& wpOrigin) const { pathId = m_pathId; wpOrigin = m_PathOrigin; }
        void GetPathInformation(std::ostringstream& oss) const;

        void AddToWaypointPauseTime(int32 waitTimeDiff, bool force = false);
        bool SetNextWaypoint(uint32 pointId);
        void SetForcedMovement(ForcedMovement forcedMovement) { m_forcedMovement = forcedMovement; }
        void SetGuid(ObjectGuid guid) { m_guid = guid; }

        void UnitSpeedChanged() override { m_speedChanged = true; }

    protected:
        virtual void SwitchToNextNode(Creature& creature, WaypointPath::const_iterator& nodeItr);
        //virtual bool GetNodeAfter(WaypointPath::const_iterator& nodeItr);
        virtual bool GetNodeAfter(WaypointPath::const_iterator& nodeItr, bool looped = false);

    private:
        void LoadPath(Creature& creature, int32 pathId, WaypointPathOrigin wpOrigin, uint32 overwriteEntry);
        uint32 BuildIntPath(Movement::PointsArray& path, Creature& creature, G3D::Vector3 const& endPos);

        void Stop(int32 time) { i_nextMoveTime.Reset(time); }
        bool Stopped(Creature& u);
        bool CanMove(int32 diff, Creature& u);

        void OnArrived(Creature&);
        void SendNextWayPointPath(Creature&);
        void InformAI(Creature& creature, uint32 type, uint32 data);

        WaypointPath::const_iterator m_currentWaypointNode;
        ShortTimeTracker i_nextMoveTime;
        int32 m_scriptTime;                                 // filled with delay change when script is instantly executed and want to change node delay
        uint32 m_lastReachedWaypoint;
        Position m_resetPoint;

        uint32 m_pathId;
        int32 m_pathDuration;
        std::list<int32> m_nodeIndexes;
        WaypointPathOrigin m_PathOrigin;

        bool m_speedChanged;
        ForcedMovement m_forcedMovement;

        ObjectGuid m_guid;
};

/** LinearWPMovementGenerator loads a series of way points
 * from the DB and apply it to the creature's movement generator.
 * Creature will move in sequence from point A -> B -> C -> D -> C -> B -> A -> B .....
 * Or if you prefer it will go back and forth all along provided nodes.
 */
template<class T>
class LinearWPMovementGenerator;

template<>
class LinearWPMovementGenerator<Creature> : public WaypointMovementGenerator<Creature>
{
    public:
    LinearWPMovementGenerator(Creature& creature) : WaypointMovementGenerator(creature), m_driveWayBack(false)
    {}

    // return WAYPOINT_MOTION_TYPE on purpose so it act like normal waypoint for large majority of the core
    //MovementGeneratorType GetMovementGeneratorType() const override { return LINEAR_WP_MOTION_TYPE; }

    private:
    void SwitchToNextNode(Creature& creature, WaypointPath::const_iterator& nodeItr) override;
    //bool GetNodeAfter(WaypointPath::const_iterator& nodeItr) override;
    bool GetNodeAfter(WaypointPath::const_iterator& nodeItr, bool looped = false) override;

    bool m_driveWayBack;
};

#endif
