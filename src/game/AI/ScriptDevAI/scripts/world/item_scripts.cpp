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
SDName: Item_Scripts
SD%Complete: 100
SDComment: quest 4743
SDCategory: Items
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
EndContentData */


#include "Spells/Spell.h"

/*#####
# item_orb_of_draconic_energy
#####*/

enum
{
    SPELL_DOMINION_SOUL     = 16053,
    NPC_EMBERSTRIFE         = 10321
};

bool ItemUse_item_orb_of_draconic_energy(Player* pPlayer, Item* pItem, const SpellCastTargets& /*pTargets*/)
{
    Creature* pEmberstrife = GetClosestCreatureWithEntry(pPlayer, NPC_EMBERSTRIFE, 20.0f);
    if (!pEmberstrife)
        return false;

    // If Emberstrife already mind controled or above 10% HP: force spell cast failure
    if (pEmberstrife->HasAura(SPELL_DOMINION_SOUL) || pEmberstrife->GetHealthPercent() > 10.0f)
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DOMINION_SOUL))
            Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_TARGET_AURASTATE);

        return true;
    }

    return false;
}

void AddSC_item_scripts()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "item_orb_of_draconic_energy";
    pNewScript->pItemUse = &ItemUse_item_orb_of_draconic_energy;
    pNewScript->RegisterSelf();
}
