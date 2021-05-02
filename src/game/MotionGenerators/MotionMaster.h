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

#ifndef MANGOS_MOTIONMASTER_H
#define MANGOS_MOTIONMASTER_H

#include "Common.h"
#include "Globals/SharedDefines.h"
#include "MotionGenerators/WaypointManager.h"

#include <stack>
#include <vector>

class MovementGenerator;
class Unit;
struct Position;

namespace G3D
{
    class Vector2;
    class Vector3;
    class Vector4;
}

// Creature Entry ID used for waypoints show, visible only for GMs
#define VISUAL_WAYPOINT 1

#define PET_FOLLOW_DIST  1.0f
#define PET_FOLLOW_ANGLE (M_PI_F / 2.0f)

// define minimum falling distance required to launch MoveFall generator
static float MOVE_FALL_MIN_FALL_DISTANCE = 0.5f;

// values 0 ... MAX_DB_MOTION_TYPE-1 used in DB
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE                = 0,                    // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE              = 1,                    // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE            = 2,                    // WaypointMovementGenerator.h
    PATH_MOTION_TYPE                = 3,                    // PathMovementGenerator.h

    MAX_DB_MOTION_TYPE              = 4,                    // *** this and below motion types can't be set in DB.

    DISTRACT_MOTION_TYPE            = 4,                    // IdleMovementGenerator.h
    STAY_MOTION_TYPE                = 5,                    // PointMovementGenerator.h
    FOLLOW_MOTION_TYPE              = 6,                    // TargetedMovementGenerator.h
    CHASE_MOTION_TYPE               = 7,                    // TargetedMovementGenerator.h
    RETREAT_MOTION_TYPE             = 8,                    // PointMovementGenerator.h
    TIMED_FLEEING_MOTION_TYPE       = 9,                    // RandomMovementGenerator.h
    POINT_MOTION_TYPE               = 10,                   // PointMovementGenerator.h
    HOME_MOTION_TYPE                = 11,                   // HomeMovementGenerator.h
    FLEEING_MOTION_TYPE             = 12,                   // RandomMovementGenerator.h
    CONFUSED_MOTION_TYPE            = 13,                   // RandomMovementGenerator.h
    EFFECT_MOTION_TYPE              = 14,                   // WrapperMovementGenerator.h
    TAXI_MOTION_TYPE                = 15,                   // WaypointMovementGenerator.h
    TIMED_RANDOM_MOTION_TYPE        = 16,                   // RandomMovementGenerator.h

    EXTERNAL_WAYPOINT_MOVE          = 17,                   // Only used in UnitAI::MovementInform when a waypoint is reached. The pathId >= 0 is added as additonal value
    EXTERNAL_WAYPOINT_MOVE_START    = 18,                   // Only used in UnitAI::MovementInform when a waypoint is started. The pathId >= 0 is added as additional value
    EXTERNAL_WAYPOINT_FINISHED_LAST = 19,                   // Only used in UnitAI::MovementInform when the waittime of the last wp is finished The pathId >= 0 is added as additional value
};

enum MMCleanFlag
{
    MMCF_NONE   = 0,
    MMCF_UPDATE = 1,                                        // Clear or Expire called from update
    MMCF_RESET  = 2                                         // Flag if need top()->Reset()
};

enum ForcedMovement
{
    FORCED_MOVEMENT_NONE,
    FORCED_MOVEMENT_WALK,
    FORCED_MOVEMENT_RUN,
    FORCED_MOVEMENT_FLIGHT,
};

class MotionMaster : private std::stack<MovementGenerator*>
{
    private:
        typedef std::stack<MovementGenerator*> Impl;
        typedef std::vector<MovementGenerator*> ExpireList;


    public:
        explicit MotionMaster(Unit* unit) : m_owner(unit), m_expList(nullptr), m_cleanFlag(MMCF_NONE), m_defaultPathId(0) {}
        ~MotionMaster();

        void Initialize();

        MovementGenerator const* GetCurrent() const { return top(); }

        using Impl::top;
        using Impl::empty;


        typedef Impl::container_type::const_iterator const_iterator;
        const_iterator begin() const { return Impl::c.begin(); }
        const_iterator end() const { return Impl::c.end(); }

        void UpdateMotion(uint32 diff);
        void Clear(bool reset = true, bool all = false)
        {
            if (m_cleanFlag & MMCF_UPDATE)
                DelayedClean(reset, all);
            else
                DirectClean(reset, all);
        }
        void MovementExpired(bool reset = true)
        {
            if (m_cleanFlag & MMCF_UPDATE)
                DelayedExpire(reset);
            else
                DirectExpire(reset);
        }

        void MoveIdle();
        void MoveRandomAroundPoint(float x, float y, float z, float radius, float verticalZ = 0.0f, uint32 timer = 0);
        void MoveTargetedHome(bool runHome = true);
        void MoveFollow(Unit* target, float dist, float angle, bool asMain = false, bool alwaysBoost = false);
        void MoveStay(float x, float y, float z, float o = 0, bool asMain = false);
        void MoveChase(Unit* target, float dist = 0.0f, float angle = 0.0f, bool moveFurther = false, bool walk = false, bool combat = true);
        void DistanceYourself(float dist);
        void MoveConfused();
        void MoveFleeing(Unit* enemy, uint32 time = 0);
        void MovePoint(uint32 id, Position const& position, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE, float speed = 0.f, bool generatePath = true);
        void MovePoint(uint32 id, float x, float y, float z, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE, bool generatePath = true);
        void MovePointTOL(uint32 id, float x, float y, float z, bool takeOff, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE);
        void MovePath(std::vector<G3D::Vector3>& path, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE, bool flying = false);
        void MovePath(std::vector<G3D::Vector3>& path, float o, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE, bool flying = false);
        void MovePath(int32 pathId, WaypointPathOrigin wpOrigin = PATH_NO_PATH, ForcedMovement forcedMovement = FORCED_MOVEMENT_NONE, bool flying = false, float speed = 0.f, bool cyclic = false);
        void MoveRetreat(float x, float y, float z, float o, uint32 delay);
        void MoveWaypoint(uint32 pathId = 0, uint32 source = 0, uint32 initialDelay = 0, uint32 overwriteEntry = 0);
        void MoveTaxi();
        void MoveDistract(uint32 timer);
        void MoveCharge(float x, float y, float z, float speed, uint32 id = EVENT_CHARGE);
        void MoveCharge(Unit& target, float speed, uint32 id = EVENT_CHARGE);
        bool MoveFall();

        MovementGeneratorType GetCurrentMovementGeneratorType() const;

        void propagateSpeedChange();
        bool SetNextWaypoint(uint32 pointId);

        uint32 getLastReachedWaypoint() const;
        void GetWaypointPathInformation(std::ostringstream& oss) const;
        bool GetDestination(float& x, float& y, float& z) const;

        void SetDefaultPathId(uint32 pathId) { m_defaultPathId = pathId; }
        uint32 GetPathId() const { return m_currentPathId; }

        void InterruptPanic();

        void PauseWaypoints(uint32 time = MINUTE * IN_MILLISECONDS);
        void UnpauseWaypoints();

        void UnMarkFollowMovegens();

    private:
        void Mutate(MovementGenerator* m);                  // use Move* functions instead

        void DirectClean(bool reset, bool all);
        void DelayedClean(bool reset, bool all);

        void DirectExpire(bool reset);
        void DelayedExpire(bool reset);

        Unit*       m_owner;
        ExpireList* m_expList;
        uint8       m_cleanFlag;

        uint32      m_defaultPathId;
        uint32      m_currentPathId;
};
#endif
