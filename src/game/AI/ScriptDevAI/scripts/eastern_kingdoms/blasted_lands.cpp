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

/* ScriptData
SDName: Blasted_Lands
SD%Complete: 0
SDComment: Placeholder
SDCategory: Blasted Lands
EndScriptData */

/* ContentData
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"

bool TrapTargetSearchRuneOfTheDefiler(Unit* unit)
{
    if (unit->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = static_cast<Player*>(unit);
        if (player->HasItemCount(10757, 1)) // Ward of the Defiler
            return true;
    }

    return false;
}

std::function<bool(Unit*)> functionRuneOfTheDefiler = &TrapTargetSearchRuneOfTheDefiler;

void AddSC_blasted_lands()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "go_rune_of_the_defiler";
    pNewScript->pTrapSearching = &functionRuneOfTheDefiler;
    pNewScript->RegisterSelf();
}
