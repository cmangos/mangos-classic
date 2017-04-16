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
SDName: Molten_Core
SD%Complete:
SDComment:
SDCategory: Molten Core
EndScriptData */

/* ContentData
EndContentData */

#include "precompiled.h"
#include "molten_core.h"

/*######
## go_molten_core_rune
######*/

bool GOUse_go_molten_core_rune(Player* pPlayer, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return true;

    for (uint8 i = 0; i < MAX_MOLTEN_RUNES; ++i)
    {
        if (pGo->GetEntry() == m_aMoltenCoreRunes[i].m_uiRuneEntry)
        {
            // check if boss is already dead - if not return true
            if (pInstance->GetData(m_aMoltenCoreRunes[i].m_uiType) != DONE)
                return true;

            pInstance->SetData(m_aMoltenCoreRunes[i].m_uiType, SPECIAL);
            return false;
        }
    }

    return true;
}

void AddSC_molten_core()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_molten_core_rune";
    pNewScript->pGOUse = &GOUse_go_molten_core_rune;
    pNewScript->RegisterSelf();
}
