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

#ifndef MANGOS_POINTMOVEMENTGENERATOR_H
#define MANGOS_POINTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class PointMovementGenerator : public MovementGenerator
{
    public:
        PointMovementGenerator(uint32 id, float x, float y, float z, float o, bool generatePath, uint32 forcedMovement, float speed = 0.f) :
            m_x(x), m_y(y), m_z(z), m_o(o), m_speed(speed), m_generatePath(generatePath), m_forcedMovement(forcedMovement), m_id(id), m_speedChanged(false) {}
        PointMovementGenerator(uint32 id, float x, float y, float z, bool generatePath, uint32 forcedMovement, float speed = 0.f) :
            PointMovementGenerator(id, x, y, z, 0, generatePath, forcedMovement, speed) {}

        void Initialize(Unit& unit) override;
        void Finalize(Unit& unit) override;
        void Interrupt(Unit& unit) override;
        void Reset(Unit& unit) override;
        bool Update(Unit& unit, const uint32&/* diff*/) override;

        void UnitSpeedChanged() override { m_speedChanged = true; }

        MovementGeneratorType GetMovementGeneratorType() const override { return POINT_MOTION_TYPE; }

    protected:
        virtual void Move(Unit& unit);
        virtual void MovementInform(Unit& unit);

    protected:
        float m_x, m_y, m_z, m_o, m_speed;
        bool m_generatePath;
        uint32 m_forcedMovement;

    private:
        uint32 m_id;
        bool m_speedChanged;
};

class RetreatMovementGenerator : public PointMovementGenerator
{
    public:
        RetreatMovementGenerator(float x, float y, float z, float o, uint32 delay) :
            PointMovementGenerator(0, x, y, z, o, true, 0), m_delayTimer(delay), m_arrived(false) {}

        void Initialize(Unit& unit) override;
        void Finalize(Unit& unit) override;
        void Interrupt(Unit& unit) override;
        void Reset(Unit& unit) override;
        bool Update(Unit& unit, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return RETREAT_MOTION_TYPE; }

    protected:
        void MovementInform(Unit&) override {}

    private:
        ShortTimeTracker m_delayTimer;
        bool m_arrived;
};

class StayMovementGenerator : public PointMovementGenerator
{
    public:
        StayMovementGenerator(float x, float y, float z, float o = 0) :
            PointMovementGenerator(0, x, y, z, o, true, FORCED_MOVEMENT_RUN), m_arrived(false) {}

        void Initialize(Unit& unit) override;
        void Finalize(Unit& unit) override;
        void Interrupt(Unit& unit) override;
        bool Update(Unit& unit, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return STAY_MOTION_TYPE; }

    protected:
        void MovementInform(Unit&) override {}

    private:
        bool m_arrived;
};

class PointTOLMovementGenerator : public PointMovementGenerator
{
    public:
        PointTOLMovementGenerator(uint32 id, float x, float y, float z, bool takeOff, uint32 forcedMovement, float speed = 0) :
            PointMovementGenerator(id, x, y, z, false, forcedMovement, speed), m_takeOff(takeOff) {}

    protected:
        void Move(Unit& unit) override;

    private:
        bool m_takeOff;
};

#endif
