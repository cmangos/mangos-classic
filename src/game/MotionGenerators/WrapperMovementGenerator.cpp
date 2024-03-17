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

#include "MotionGenerators/WrapperMovementGenerator.h"
#include "Entities/Creature.h"
#include "AI/BaseAI/UnitAI.h"
#include "Entities/TemporarySpawn.h"
#include "Movement/MoveSpline.h"
#include "Movement/MoveSplineInit.h"

void AbstractWrapperMovementGenerator::Initialize(Unit& owner)
{
    // Stop any previously dispatched splines no matter the source
    if (!owner.movespline->Finalized())
    {
        if (owner.IsClientControlled())
            owner.StopMoving(true);
        else
            owner.InterruptMoving();
    }

    if (!i_delayed && !i_dispatched)
    {
        i_spline.Launch();
        i_dispatched = true;
    }

    if (!owner.IsPlayerControlled())
    {
        i_useTimer = true;
        if (owner.movespline->isFacingTarget())
        {
            if (Unit* target = ObjectAccessor::GetUnit(owner, ObjectGuid(owner.movespline->GetFacing().target)))
            {
                if (owner.CanAttackInCombat(target, false, false))
                {
                    owner.Attack(target, owner.AI()->IsMeleeEnabled());
                    owner.SetTarget(target);
                }
            }
        }
    }
}

void AbstractWrapperMovementGenerator::Finalize(Unit& owner)
{
    // Stop any previously dispatched splines no matter the source
    if (!owner.movespline->Finalized())
    {
        if (owner.IsClientControlled())
            owner.StopMoving(true);
        else
            owner.InterruptMoving();
    }
    // Inform AI about spline completed
    else if (!i_informed)
        Inform(owner);
}

void AbstractWrapperMovementGenerator::Interrupt(Unit& owner)
{
    i_dispatched = (!i_resident || owner.movespline->Finalized());
    owner.InterruptMoving();
}

void AbstractWrapperMovementGenerator::Reset(Unit& owner)
{
    Initialize(owner);
}

bool AbstractWrapperMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    if (i_delayed && !i_dispatched)
    {
        i_spline.Launch();
        i_dispatched = true;
    }

    bool finalized = owner.movespline->Finalized();
    if (finalized && i_useTimer && !i_informed)
        Inform(owner);

    if (finalized && i_useTimer)
    {
        if (i_timerSet)
        {
            m_delayTimer.Update(diff);
            return !m_delayTimer.Passed();
        }
        else
        {
            m_delayTimer.Reset(1000);
            i_timerSet = true;
            return true;
        }
    }

    return !finalized;
}

void AbstractWrapperMovementGenerator::Inform(Unit& owner)
{
    i_informed = true;

    const MovementGeneratorType type = GetMovementGeneratorType();

    if (UnitAI* ai = owner.AI())
        ai->MovementInform(type, i_id);

    if (owner.GetTypeId() == TYPEID_UNIT && static_cast<Creature&>(owner).IsTemporarySummon())
    {
        ObjectGuid const& spawnerGuid = owner.GetSpawnerGuid();

        if (spawnerGuid.IsCreatureOrPet())
        {
            if (Creature* spawner = owner.GetMap()->GetAnyTypeCreature(spawnerGuid))
            {
                if (UnitAI* ai = spawner->AI())
                    ai->SummonedMovementInform(&static_cast<Creature&>(owner), type, i_id);
            }
        }
    }

    if (m_relayId)
        owner.GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, m_relayId, &owner, m_guid ? owner.GetMap()->GetWorldObject(m_guid) : nullptr);
}

void EffectMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_ROAMING | UNIT_STAT_PROPELLED);

    if (!i_delayed && !i_dispatched)
        owner.addUnitState(UNIT_STAT_ROAMING_MOVE);

    AbstractWrapperMovementGenerator::Initialize(owner);
}

void EffectMovementGenerator::Finalize(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE | UNIT_STAT_PROPELLED);
    AbstractWrapperMovementGenerator::Finalize(owner);
}

void EffectMovementGenerator::Interrupt(Unit& owner)
{
    owner.clearUnitState(UNIT_STAT_ROAMING_MOVE);
    AbstractWrapperMovementGenerator::Interrupt(owner);
}

bool EffectMovementGenerator::Update(Unit &owner, const uint32 &diff)
{
    if (i_delayed && !i_dispatched)
        owner.addUnitState(UNIT_STAT_ROAMING_MOVE);

    return AbstractWrapperMovementGenerator::Update(owner, diff);
}
