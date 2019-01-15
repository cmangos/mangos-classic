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

#include "HomeMovementGenerator.h"
#include "Maps/Map.h"
#include "Entities/Creature.h"
#include "AI/BaseAI/CreatureAI.h"
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"

void HomeMovementGenerator<Creature>::Initialize(Creature& owner)
{
    wasActive = owner.isActiveObject();
    if (!wasActive)
        owner.SetActiveObjectState(true);
    owner.SetEvade(EVADE_HOME);

    _setTargetLocation(owner);
}

void HomeMovementGenerator<Creature>::Reset(Creature&)
{
}

void HomeMovementGenerator<Creature>::_setTargetLocation(Creature& owner)
{
    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    Movement::MoveSplineInit init(owner);
    float x, y, z, o;
    // at apply we can select more nice return points base at current movegen
    if (owner.GetMotionMaster()->empty() || !owner.GetMotionMaster()->top()->GetResetPosition(owner, x, y, z, o))
        owner.GetCombatStartPosition(x, y, z, o);

    if (x == 0 && x == y && y == z)
        owner.GetRespawnCoord(x, y, z, &o);

    init.SetFacing(o);
    init.MoveTo(x, y, z, true);
    init.SetWalk(!runHome);
    init.Launch();

    arrived = false;
    owner.clearUnitState(static_cast<uint32>(UNIT_STAT_ALL_DYN_STATES));
}

bool HomeMovementGenerator<Creature>::Update(Creature& owner, const uint32& /*time_diff*/)
{
    arrived = owner.movespline->Finalized();
    return !arrived;
}

void HomeMovementGenerator<Creature>::Finalize(Creature& owner)
{
    owner.SetEvade(EVADE_NONE);
    if (arrived)
    {
        if (owner.GetTemporaryFactionFlags() & TEMPFACTION_RESTORE_REACH_HOME)
            owner.ClearTemporaryFaction();

        owner.SetWalk(!owner.hasUnitState(UNIT_STAT_RUNNING_STATE) && !owner.IsLevitating(), false);
        owner.LoadCreatureAddon(true);
        owner.AI()->JustReachedHome();

        if (owner.IsTemporarySummon())
        {
            if (owner.GetSpawnerGuid().IsCreatureOrPet())
                if (Creature* pSummoner = owner.GetMap()->GetAnyTypeCreature(owner.GetSpawnerGuid()))
                    if (pSummoner->AI())
                        pSummoner->AI()->SummonedJustReachedHome(&owner);
        }

        if (!wasActive)
            owner.SetActiveObjectState(false);
    }
}
