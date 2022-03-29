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

#ifndef MANGOS_PATHMOVEMENTGENERATOR_H
#define MANGOS_PATHMOVEMENTGENERATOR_H

#include "MotionGenerators/MovementGenerator.h"
#include "MotionGenerators/WaypointManager.h"
#include "Movement/MoveSplineInit.h"
#include "Entities/ObjectGuid.h"

#include <vector>

class AbstractPathMovementGenerator : public MovementGenerator
{
    public:
        explicit AbstractPathMovementGenerator(const Movement::PointsArray& path, float orientation = 0, int32 offset = 0);
        explicit AbstractPathMovementGenerator(const WaypointPath* path, int32 offset = 0, bool cyclic = false, ObjectGuid guid = ObjectGuid());

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;
        void Reset(Unit& owner) override;
        bool Update(Unit& owner, const uint32&/* diff*/) override;

        void UnitSpeedChanged() override { m_speedChanged = true; }

    protected:
        virtual void MovementInform(Unit& unit);

    protected:
        WaypointPath m_path;
        int32 m_pathIndex;
        Movement::PointsArray m_spline;
        float m_orientation;
        ShortTimeTracker m_timer;
        bool m_cyclic;
        bool m_firstCycle;
        uint32 m_startPoint;
        ObjectGuid m_guid;

    private:
        bool m_speedChanged;
};

class FixedPathMovementGenerator : public AbstractPathMovementGenerator
{
    public:
        FixedPathMovementGenerator(const Movement::PointsArray &path, float orientation, uint32 forcedMovement, bool flying = false, float speed = 0, int32 offset = 0) :
            AbstractPathMovementGenerator(path, orientation, offset), m_flying(flying), m_speed(speed), m_forcedMovement(forcedMovement) {}
        FixedPathMovementGenerator(const Movement::PointsArray& path, uint32 forcedMovement, bool flying = false, float speed = 0, int32 offset = 0) :
            FixedPathMovementGenerator(path, 0, forcedMovement, flying, speed, offset) {}
        FixedPathMovementGenerator(Unit& unit, int32 pathId, WaypointPathOrigin wpOrigin, ForcedMovement forcedMovement, bool flying = false, float speed = 0, int32 offset = 0, bool cyclic = false, ObjectGuid guid = ObjectGuid());
        FixedPathMovementGenerator(Creature& creature);

        void Initialize(Unit& unit) override;
        void Finalize(Unit& unit) override;
        void Interrupt(Unit& unit) override;
        void Reset(Unit& unit) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return PATH_MOTION_TYPE; }

        void AddToPathPauseTime(int32 waitTimeDiff, bool force);

    protected:
        virtual bool Move(Unit& unit) const;

    protected:
        bool m_flying;
        float m_speed;
        uint32 m_forcedMovement;
};

class TaxiMovementGenerator : public AbstractPathMovementGenerator
{
    public:
        explicit TaxiMovementGenerator() :
            AbstractPathMovementGenerator(Movement::PointsArray()) {}

        void Initialize(Unit& unit) override;
        void Finalize(Unit& unit) override;
        void Interrupt(Unit& unit) override;
        bool Update(Unit& unit, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return TAXI_MOTION_TYPE; }

        bool Resume(Unit& unit);

    protected:
        virtual bool Move(Unit& unit);
};

#endif
