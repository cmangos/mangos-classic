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

GuardAI::GuardAI(Creature* c) : CreatureAI(c), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void GuardAI::MoveInLineOfSight(Unit* u)
{
    // Ignore Z for flying creatures
    if (!m_creature->CanFly() && m_creature->GetDistanceZ(u) > CREATURE_Z_ATTACK_RANGE)
        return;

    if (!m_creature->getVictim() && m_creature->CanAttackOnSight(u) &&
            (u->IsHostileToPlayers() || m_creature->IsHostileTo(u) /*|| u->getVictim() && m_creature->IsFriendlyTo(u->getVictim())*/) &&
            u->isInAccessablePlaceFor(m_creature))
    {
        float attackRadius = m_creature->GetAttackDistance(u);
        if (m_creature->IsWithinDistInMap(u, attackRadius))
        {
            // Need add code to let guard support player
            AttackStart(u);
            u->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
        }
    }
}

void GuardAI::UpdateAI(const uint32 /*diff*/)
{
    // update i_victimGuid if i_creature.getVictim() !=0 and changed
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    i_victimGuid = m_creature->getVictim()->GetObjectGuid();

    DoMeleeAttackIfReady();
}

bool GuardAI::IsVisible(Unit* pl) const
{
    return m_creature->IsWithinDist(pl, sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER))
           && pl->isVisibleForOrDetect(m_creature, m_creature, true);
}
