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

#ifndef MANGOS_WRAPPERMOVEMENTGENERATOR_H
#define MANGOS_WRAPPERMOVEMENTGENERATOR_H

#include "MotionGenerators/MovementGenerator.h"
#include "Movement/MoveSplineInit.h"

class AbstractWrapperMovementGenerator : virtual public MovementGenerator
{
    public:
        AbstractWrapperMovementGenerator(Movement::MoveSplineInit& spline, uint32 id = 0, bool delayed = false, bool resident = true, ObjectGuid guid = ObjectGuid(), uint32 relayId = 0) :
            i_spline(spline), i_id(id), i_delayed(delayed), i_resident(resident), i_dispatched(false), i_informed(false), i_useTimer(false), i_timerSet(false), m_guid(guid), m_relayId(relayId) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;
        void Reset(Unit& owner) override;
        bool Update(Unit& owner, const uint32&/* diff*/) override;

        void UnitSpeedChanged() override = 0;

        MovementGeneratorType GetMovementGeneratorType() const override = 0;

        void Inform(Unit& owner);

    protected:
        Movement::MoveSplineInit i_spline;
        uint32 i_id;
        bool i_delayed, i_resident, i_dispatched, i_informed, i_useTimer, i_timerSet;
        ShortTimeTracker m_delayTimer;
        ObjectGuid m_guid;
        uint32 m_relayId;
};

class EffectMovementGenerator : public AbstractWrapperMovementGenerator
{
    public:
        EffectMovementGenerator(Movement::MoveSplineInit& spline, uint32 id = 0, bool delayed = false, ObjectGuid guid = ObjectGuid(), uint32 relayId = 0) :
            AbstractWrapperMovementGenerator(spline, id, delayed, false, guid, relayId) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;
        bool Update(Unit& owner, const uint32& diff) override;

        void UnitSpeedChanged() override {}

        MovementGeneratorType GetMovementGeneratorType() const override { return EFFECT_MOTION_TYPE; }
};

class FallMovementGenerator : public EffectMovementGenerator
{
    public:
        using EffectMovementGenerator::EffectMovementGenerator;

        MovementGeneratorType GetMovementGeneratorType() const override { return FALL_MOTION_TYPE; }
};

#endif
