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

#ifndef MANGOS_TARGETEDMOVEMENTGENERATOR_H
#define MANGOS_TARGETEDMOVEMENTGENERATOR_H

#include "Movement/MoveSplineInit.h"
#include "MotionGenerators/MovementGenerator.h"
#include "MotionGenerators/FollowerReference.h"
#include <G3D/Vector3.h>
#include "Entities/ObjectGuid.h"
#include "Entities/Object.h"

class PathFinder;

class TargetedMovementGeneratorBase
{
    public:
        TargetedMovementGeneratorBase(Unit& target) { i_target.link(&target, this); }
        void stopFollowing() { }
        void SetNewTarget(Unit& target) { i_target.link(&target, this); }
    protected:
        FollowerReference i_target;
};

template<class T, typename D>
class TargetedMovementGeneratorMedium
    : public MovementGeneratorMedium< T, D >, public TargetedMovementGeneratorBase
{
    protected:
        TargetedMovementGeneratorMedium(Unit& target, float offset, float angle) :
            TargetedMovementGeneratorBase(target),
            i_recheckDistance(0),
            i_offset(offset), i_angle(angle),
            i_speedChanged(false), i_targetReached(false),
            i_path(nullptr), i_faceTarget(true)
        {
        }
        ~TargetedMovementGeneratorMedium() { delete i_path; }

    public:
        bool Update(T&, const uint32&) override;

        virtual bool IsReachable() const override;

        Unit* GetCurrentTarget() const override { return i_target.getTarget(); }
        float GetOffset() const { return i_offset; }
        float GetAngle() const { return i_angle; }

        virtual void UnitSpeedChanged() override { i_speedChanged = true; }

        virtual bool RemoveOnInvalid() const { return true; };

    protected:
        virtual bool RequiresNewPosition(T& owner, Position pos) const;
        virtual float GetDynamicTargetDistance(T& /*owner*/, bool /*forRangeCheck*/) const { return i_offset; }
        virtual bool ShouldFaceTarget() const { return i_faceTarget; }
        virtual void HandleTargetedMovement(T& owner, const uint32& time_diff) = 0;
        virtual void HandleFinalizedMovement(T& owner) = 0;
        virtual void HandleMovementFailure(T& owner) = 0;

        virtual bool _hasUnitStateNotMove(Unit& owner) = 0;
        virtual void _clearUnitStateMove(Unit& owner) = 0;
        virtual void _addUnitStateMove(Unit& owner) = 0;

        ShortTimeTracker i_recheckDistance;
        float i_offset;
        float i_angle;
        G3D::Vector3 i_lastTargetPos;
        bool i_speedChanged : 1;
        bool i_targetReached : 1;
        bool i_faceTarget : 1;

        PathFinder* i_path;
};

/*
Chases after target
Has these custom features:
Backpedaling - when player is standing inside model, mob backs away with 2 second delay, likely for smoothness
Fanning - likewise after 2 seconds, all mobs spread out at a given target - has to be coordinated with all targets which attack same victim
TODO: Implement variable chase speed for pet chase
*/
enum ChaseMovementMode
{
    CHASE_MODE_NORMAL, // chasing target
    CHASE_MODE_BACKPEDAL, // collision movement
    CHASE_MODE_DISTANCING, // running away from melee
    CHASE_MODE_FANNING, // mob collision movement
};

extern const char* ChaseModes[];

class ChaseMovementGenerator : public TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator >
{
    using TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::i_offset;
    using TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator>::i_angle;
    public:
        ChaseMovementGenerator(Unit& target, float offset, float angle, bool moveFurther = true, bool walk = false, bool combat = true)
            : TargetedMovementGeneratorMedium<Unit, ChaseMovementGenerator >(target, offset, angle),
            m_moveFurther(moveFurther), m_walk(walk), m_combat(combat), m_reachable(true),
            m_fanningEnabled(true), m_currentMode(CHASE_MODE_NORMAL), m_closenessAndFanningTimer(0), m_closenessExpired(false) {}
        ~ChaseMovementGenerator() {}

        MovementGeneratorType GetMovementGeneratorType() const override { return CHASE_MOTION_TYPE; }

        void Initialize(Unit&) override;
        void Finalize(Unit&) override;
        void Interrupt(Unit&) override;
        void Reset(Unit&) override;
        void SetOffsetAndAngle(float offset, float angle, bool moveFurther);
        void DistanceYourself(Unit& owner, float distance);
        void FanOut(Unit& owner);

        bool EnableWalking() const { return m_walk;}
        bool _lostTarget(Unit& u) const;
        bool RemoveOnInvalid() const override { return false; }
        void _reachTarget(Unit&);
        bool GetResetPosition(Unit& /*u*/, float& /*x*/, float& /*y*/, float& /*z*/, float& /*o*/) const override { return false; }
        void HandleMovementFailure(Unit& owner) override;

        ChaseMovementMode GetCurrentMode() const { return m_currentMode; }
        virtual bool IsReachable() const override;

        virtual bool IsRemovedOnExpire() const override { return true; }

        std::pair<std::string, std::string> GetPrintout() const;

        void AddDelay();

    protected:
        float GetDynamicTargetDistance(Unit& owner, bool forRangeCheck) const override;
        void HandleTargetedMovement(Unit& owner, const uint32& time_diff) override;
        void HandleFinalizedMovement(Unit& owner) override;
        bool RequiresNewPosition(Unit& owner, Position pos) const override;

        bool _hasUnitStateNotMove(Unit& u) override;
        void _clearUnitStateMove(Unit& u) override;
        void _addUnitStateMove(Unit& u) override;

    private:
        virtual bool _getLocation(Unit& owner, float& x, float& y, float& z) const;
        virtual void _setLocation(Unit& owner);

        bool IsReachablePositionToTarget(Unit& owner, float x, float y, float z, Unit& target);

        bool DispatchSplineToPosition(Unit& owner, float x, float y, float z, bool walk, bool cutPath, bool target = false, bool checkReachable = false);
        void CutPath(Unit& owner, PointsArray& path);
        void Backpedal(Unit& owner);

        bool m_moveFurther;
        bool m_walk;
        bool m_combat;
        bool m_reachable;
        bool m_fanningEnabled;

        uint32 m_closenessAndFanningTimer;
        bool m_closenessExpired;

        ChaseMovementMode m_currentMode;

        GuidVector m_spawns;
};

class FollowMovementGenerator : public TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>
{
    public:
        FollowMovementGenerator(Unit& target, float offset, float angle, bool main, bool possess, bool alwaysBoost = false)
            : TargetedMovementGeneratorMedium<Unit, FollowMovementGenerator>(target, offset, angle),
              m_main(main), m_possess(possess), m_boost(alwaysBoost), m_targetMoving(false), m_targetFaced(false)
        {
            i_faceTarget = (angle == 0.0f);
        }
        ~FollowMovementGenerator() {}

        MovementGeneratorType GetMovementGeneratorType() const override { return FOLLOW_MOTION_TYPE; }

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        virtual void Interrupt(Unit& owner) override;
        void Reset(Unit& owner) override;

        bool GetResetPosition(Unit& owner, float& x, float& y, float& z, float& o) const override;

        virtual bool EnableWalking() const;

        bool _lostTarget(Unit& owner) const;
        void _reachTarget(Unit& owner);

        void HandleMovementFailure(Unit& owner) override;

        virtual bool IsRemovedOnExpire() const override { return !m_main; }

        void MarkMovegen();

    protected:
        float GetDynamicTargetDistance(Unit& owner, bool forRangeCheck) const override;
        void HandleTargetedMovement(Unit& owner, const uint32& time_diff) override;
        void HandleFinalizedMovement(Unit& owner) override;

        bool _hasUnitStateNotMove(Unit& owner) override;
        void _clearUnitStateMove(Unit& owner) override;
        void _addUnitStateMove(Unit& owner) override;

        virtual float GetSpeed(Unit& owner) const;

        virtual bool IsBoostAllowed(Unit& owner) const;
        virtual bool IsUnstuckAllowed(Unit& owner) const;

        virtual bool Move(Unit& owner, float x, float y, float z);

    protected:
        virtual bool _getOrientation(Unit& owner, float& o) const;
        virtual bool _getLocation(Unit& owner, float& x, float& y, float& z, bool movingNow) const;
        virtual void _setOrientation(Unit& owner);
        virtual void _setLocation(Unit& owner, bool movingNow);

        static inline uint32 _getPollRateBase() { return 400; }
        static inline uint32 _getPollRateMax() { return 800; }
        virtual uint32 _getPollRateMultiplier(Unit& owner, bool targetMovingNow, bool targetMovedBefore = true) const;
        virtual uint32 _getPollRate(Unit& owner, bool movingNow, bool movingBefore = true) const;

        bool m_main;
        bool m_possess;
        bool m_boost;

        bool m_targetMoving;
        bool m_targetFaced;
};

// to be able to compute new path before the end of the current path (in milliseconds)
#define FORMATION_FOLLOWERS_CHECK_OVERRIDE 400

class FormationMovementGenerator : public FollowMovementGenerator
{
    public:
        FormationMovementGenerator(FormationSlotDataSPtr& sData, bool main);
        ~FormationMovementGenerator();

        MovementGeneratorType GetMovementGeneratorType() const override { return FORMATION_MOTION_TYPE; }

        bool Update(Unit&, const uint32&) override;
        void Interrupt(Unit& owner) override;
        bool GetResetPosition(Unit&, float& x, float& y, float& z, float& o) const override;

    protected:
        void HandleTargetedMovement(Unit& owner, const uint32& time_diff) override;
        void HandleFinalizedMovement(Unit& owner) override;

    private:
        virtual void _setLocation(Unit& owner, bool catchup) override;
        float BuildPath(Unit& owner, PointsArray& path);
        bool HandleMasterDistanceCheck(Unit& owner, const uint32& time_diff);
        FormationSlotDataSPtr m_slot;

        // used to smooth angle change
        float m_lastAngle;

        // running to master
        bool m_headingToMaster;

        // distance from where the follower will be teleported to master
        float m_tpDistance;

        // distance from where the follower will run to master.
        float m_moveToMasterDistance;

        // reset position if this is interrupted
        Position m_resetPoint;
};

#endif
