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

#include "BaseAI/NullCreatureAI.h"
#include "BaseAI/ReactorAI.h"
#include "BaseAI/AggressorAI.h"
#include "BaseAI/GuardAI.h"
#include "BaseAI/PetAI.h"
#include "BaseAI/TotemAI.h"
#include "BaseAI/GuardianAI.h"
#include "BaseAI/PossessedAI.h"
#include "BaseAI/CreatureAIImpl.h"
#include "EventAI/CreatureEventAI.h"
#include "MotionGenerators/RandomMovementGenerator.h"
#include "MotionGenerators/MovementGeneratorImpl.h"
#include "MotionGenerators/WaypointMovementGenerator.h"
#include "CreatureAIRegistry.h"

namespace AIRegistry
{
    void Initialize()
    {
        (new CreatureAIFactory<NullCreatureAI>("NullAI"))->RegisterSelf();
        (new CreatureAIFactory<AggressorAI>("AggressorAI"))->RegisterSelf();
        (new CreatureAIFactory<ReactorAI>("ReactorAI"))->RegisterSelf();
        (new CreatureAIFactory<GuardAI>("GuardAI"))->RegisterSelf();
        (new CreatureAIFactory<PetAI>("PetAI"))->RegisterSelf();
        (new CreatureAIFactory<TotemAI>("TotemAI"))->RegisterSelf();
        (new CreatureAIFactory<CreatureEventAI>("EventAI"))->RegisterSelf();
        (new CreatureAIFactory<GuardianAI>("GuardianAI"))->RegisterSelf();
        (new CreatureAIFactory<PossessedAI>("PossessedAI"))->RegisterSelf();

        (new MovementGeneratorFactory<RandomMovementGenerator<Creature> >(RANDOM_MOTION_TYPE))->RegisterSelf();
        (new MovementGeneratorFactory<WaypointMovementGenerator<Creature> >(WAYPOINT_MOTION_TYPE))->RegisterSelf();
    }
}
