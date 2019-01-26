/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#include "Entities/Creature.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

void CombatAI::ExecuteActions()
{
    if (!CanExecuteCombatAction())
        return;

    for (uint32 i = 0; i < GetCombatActionCount(); ++i)
    {
        if (GetActionReadyStatus(i))
            ExecuteAction(i);
    }
}

void CombatAI::UpdateAI(const uint32 diff)
{
    UpdateTimers(diff, m_creature->isInCombat());

    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    ExecuteActions();

    DoMeleeAttackIfReady();
}