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

#ifndef MANGOS_IDLEMOVEMENTGENERATOR_H
#define MANGOS_IDLEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class IdleMovementGenerator : public MovementGenerator
{
    public:
        void Initialize(Unit&) override {}
        void Finalize(Unit&) override {}
        void Interrupt(Unit&) override {}
        void Reset(Unit&) override {}
        bool Update(Unit&, const uint32&) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return IDLE_MOTION_TYPE; }
};

extern IdleMovementGenerator si_idleMovement;

class DistractMovementGenerator : public IdleMovementGenerator
{
    public:
        explicit DistractMovementGenerator(uint32 timer) : m_timer(timer) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Reset(Unit& owner) override;
        bool Update(Unit& owner, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return DISTRACT_MOTION_TYPE; }

    private:
        ShortTimeTracker m_timer;
};

#endif
