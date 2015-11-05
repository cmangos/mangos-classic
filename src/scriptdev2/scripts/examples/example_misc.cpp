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
SDName: Example_Misc
SD%Complete: 100
SDComment: Item, Areatrigger and other small code examples
SDCategory: Script Examples
EndScriptData */

#include "precompiled.h"

enum
{
    SAY_HI  = -1999925
};

bool AreaTrigger_at_example(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    DoScriptText(SAY_HI, pPlayer);
    return true;
}

extern void LoadDatabase();
bool ItemUse_example_item(Player* /*pPlayer*/, Item* /*pItem*/, SpellCastTargets const& /*scTargets*/)
{
    LoadDatabase();
    return true;
}

bool GOUse_example_go_teleporter(Player* pPlayer, GameObject* /*pGo*/)
{
    pPlayer->TeleportTo(0, 1807.07f, 336.105f, 70.3975f, 0.0f);
    return false;
}

void AddSC_example_misc()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "at_example";
    pNewScript->pAreaTrigger = &AreaTrigger_at_example;
    pNewScript->RegisterSelf(false);

    pNewScript = new Script;
    pNewScript->Name = "example_item";
    pNewScript->pItemUse = &ItemUse_example_item;
    pNewScript->RegisterSelf(false);

    pNewScript = new Script;
    pNewScript->Name = "example_go_teleporter";
    pNewScript->pGOUse = &GOUse_example_go_teleporter;
    pNewScript->RegisterSelf(false);
}
