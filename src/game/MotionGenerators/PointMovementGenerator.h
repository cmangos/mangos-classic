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

template<class T>
class PointMovementGenerator
    : public MovementGeneratorMedium< T, PointMovementGenerator<T> >
{
    public:
        PointMovementGenerator(uint32 id, float x, float y, float z, bool generatePath, uint32 forcedMovement, float speed = 0.f) :
            i_x(x), i_y(y), i_z(z), m_id(id), m_generatePath(generatePath), m_speedChanged(false), m_forcedMovement(forcedMovement), m_speed(speed) {}

        virtual void Initialize(T&);
        virtual void Finalize(T&);
        void Interrupt(T&);
        void Reset(T& unit);
        bool Update(T&, const uint32& diff);

        void MovementInform(T&);

        virtual void UnitSpeedChanged() override { m_speedChanged = true; }

        MovementGeneratorType GetMovementGeneratorType() const override { return POINT_MOTION_TYPE; }

    protected:
        float i_x, i_y, i_z;

    private:
        uint32 m_id;
        bool m_generatePath;
        bool m_speedChanged;
        uint32 m_forcedMovement;
        float m_speed;
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        AssistanceMovementGenerator(float x, float y, float z) :
            PointMovementGenerator<Creature>(0, x, y, z, true, 0) {}

        void Initialize(Creature& unit) override;
        void Finalize(Creature& unit) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return ASSISTANCE_MOTION_TYPE; }
};

class FlyOrLandMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        FlyOrLandMovementGenerator(uint32 id, float x, float y, float z, bool liftOff) :
            PointMovementGenerator<Creature>(id, x, y, z, false, 0),
            m_liftOff(liftOff) {}

        void Initialize(Creature& unit) override;

    private:
        bool m_liftOff;
};

#endif
