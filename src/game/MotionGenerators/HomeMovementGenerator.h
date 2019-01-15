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

#ifndef MANGOS_HOMEMOVEMENTGENERATOR_H
#define MANGOS_HOMEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class Creature;

template < class T >
class HomeMovementGenerator;

template <>
class HomeMovementGenerator<Creature>
    : public MovementGeneratorMedium< Creature, HomeMovementGenerator<Creature> >
{
    public:

        HomeMovementGenerator(bool _runHome = true) : arrived(false), runHome(_runHome), wasActive(false)
        {
        }

        ~HomeMovementGenerator() {}

        void Initialize(Creature&);
        void Finalize(Creature&);
        void Interrupt(Creature&) {}
        void Reset(Creature&);
        bool Update(Creature&, const uint32&);
        MovementGeneratorType GetMovementGeneratorType() const override { return HOME_MOTION_TYPE; }

    private:
        void _setTargetLocation(Creature&);
        bool arrived;
        bool runHome;
        bool wasActive;
};
#endif
