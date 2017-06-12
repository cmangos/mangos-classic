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

#include "AggressorAI.h"
#include "Errors.h"
#include "Entities/Creature.h"
#include "World/World.h"
#include "Server/DBCStores.h"
#include "Maps/Map.h"
#include "Log.h"

int AggressorAI::Permissible(const Creature* creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if (!(creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_AGGRO) && !creature->IsNeutralToAll())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

AggressorAI::AggressorAI(Creature* c) : CreatureAI(c), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void AggressorAI::UpdateAI(const uint32 /*diff*/)
{
    // update i_victimGuid
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    DoMeleeAttackIfReady();
}

bool AggressorAI::IsVisible(Unit* pl) const
{
    return m_creature->IsWithinDist(pl, sWorld.getConfig(CONFIG_FLOAT_SIGHT_MONSTER))
           && pl->isVisibleForOrDetect(m_creature, m_creature, true);
}
