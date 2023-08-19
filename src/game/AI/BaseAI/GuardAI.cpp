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

#include "GuardAI.h"
#include "Entities/Creature.h"
#include "Entities/Player.h"
#include "World/World.h"

int GuardAI::Permissible(const Creature* creature)
{
    if (creature->IsGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature* creature) : CreatureAI(creature)
{
}

void GuardAI::MoveInLineOfSight(Unit* who)
{
    // Ignore Z for flying creatures
    if (!m_creature->CanFly() && who->IsFlying() && m_creature->GetDistanceZ(who) > (IsRangedUnit() ? CREATURE_Z_ATTACK_RANGE_RANGED : CREATURE_Z_ATTACK_RANGE_MELEE))
        return;

    if (m_creature->GetVictim())
        return;

    if (who->IsInCombat() && m_creature->CanAssist(who))
    {
        Unit* victim = who->getAttackerForHelper();

        if (!victim)
            return;

        if (m_creature->CanInitiateAttack() && m_creature->CanAttackOnSight(victim) && victim->isInAccessablePlaceFor(m_creature))
        {
            if (who->GetTypeId() == TYPEID_PLAYER && victim->GetTypeId() != TYPEID_PLAYER)
            {
                if (m_creature->IsWithinDistInMap(who, 5.0) && m_creature->IsWithinDistInMap(victim, 10.0) && m_creature->IsWithinLOSInMap(victim, true))
                {
                    AttackStart(victim);
                }
            }
            else if ((who->GetTypeId() == TYPEID_PLAYER && victim->GetTypeId() == TYPEID_PLAYER) || (victim->GetObjectGuid().IsCreature() && ((Creature*)victim)->IsPet() && ((Creature*)victim)->GetOwnerGuid().IsPlayer()))
            {
                if (m_creature->IsWithinDistInMap(who, 30.0) && m_creature->IsWithinLOSInMap(who, true))
                {
                    AttackStart(victim);
                }
            }
            else if (who->GetObjectGuid().IsCreature())
            {
                if (((Creature*)who)->IsGuard() || ((Creature*)who)->IsCivilian())
                {
                    if (m_creature->IsWithinDistInMap(who, 20.0) && m_creature->IsWithinLOSInMap(who, true))
                    {
                        AttackStart(victim);
                    }
                }

            }
        }
    }
    else
    {
        if (m_creature->CanInitiateAttack() && m_creature->CanAttackOnSight(who) && who->isInAccessablePlaceFor(m_creature))
        {
            float attackRadius = m_creature->GetAttackDistance(who);
            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->IsWithinLOSInMap(who, true))
            {
                AttackStart(who);
            }
        }
    }
}
