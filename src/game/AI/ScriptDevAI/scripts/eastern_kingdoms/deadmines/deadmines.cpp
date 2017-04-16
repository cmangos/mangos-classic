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
SDName: Deadmines
SD%Complete: 100
SDComment: Contains GO for Iron Clad door event
SDCategory: Deadmines
EndScriptData */

#include "precompiled.h"
#include "deadmines.h"

bool GOUse_go_defias_cannon(Player* /*pPlayer*/, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_IRON_CLAD_DOOR) == DONE)
        return false;

    pInstance->SetData(TYPE_IRON_CLAD_DOOR, DONE);
    return false;
}

void AddSC_deadmines()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_defias_cannon";
    pNewScript->pGOUse = &GOUse_go_defias_cannon;
    pNewScript->RegisterSelf();
}
