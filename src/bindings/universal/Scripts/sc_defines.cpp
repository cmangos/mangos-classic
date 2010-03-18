/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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

#include "sc_defines.h"

#include "../../../game/Player.h"

uint32 GetSkillLevel(Player *player,uint32 trskill)
{
    // Returns the level of some tradetrskill known by player
    // Need to add missing spells

    uint32 spell_apprentice = 0;
    uint32 spell_journeyman = 0;
    uint32 spell_expert     = 0;

    switch(trskill)
    {
        case TRADESKILL_ALCHEMY:
            spell_apprentice = 2259;
            spell_journeyman = 3101;
            spell_expert     = 3464;
            break;
        case TRADESKILL_BLACKSMITHING:
            spell_apprentice = 2018;
            spell_journeyman = 3100;
            spell_expert     = 8768;
            break;
        case TRADESKILL_COOKING:
            spell_apprentice = 2550;
            spell_journeyman = 3102;
            spell_expert     = 3413;
            break;
        case TRADESKILL_ENCHANTING:
            spell_apprentice = 7411;
            spell_journeyman = 7412;
            spell_expert     = 7413;
            break;
        case TRADESKILL_ENGINEERING:
            spell_apprentice = 4036;
            spell_journeyman = 4037;
            spell_expert     = 4038;
            break;
        case TRADESKILL_FIRSTAID:
            spell_apprentice = 3273;
            spell_journeyman = 3274;
            spell_expert     = 7924;
            break;
        case TRADESKILL_HERBALISM:
            spell_apprentice = 2372;
            spell_journeyman = 2373;
            spell_expert     = 3571;
            break;
        case TRADESKILL_LEATHERWORKING:
            spell_apprentice = 2108;
            spell_journeyman = 3104;
            spell_expert     = 20649;
            break;
        case TRADESKILL_POISONS:
            spell_apprentice = 0;
            spell_journeyman = 0;
            spell_expert     = 0;
            break;
        case TRADESKILL_TAILORING:
            spell_apprentice = 3908;
            spell_journeyman = 3909;
            spell_expert     = 3910;
            break;
        case TRADESKILL_MINING:
            spell_apprentice = 2581;
            spell_journeyman = 2582;
            spell_expert     = 3568;
            break;
        case TRADESKILL_FISHING:
            spell_apprentice = 7733;
            spell_journeyman = 7734;
            spell_expert     = 7736;
            break;
        case TRADESKILL_SKINNING:
            spell_apprentice = 8615;
            spell_journeyman = 8619;
            spell_expert     = 8620;
            break;
    }

    if (player->HasSpell(spell_expert))
        return TRADESKILL_LEVEL_EXPERT;

    if (player->HasSpell(spell_journeyman))
        return TRADESKILL_LEVEL_JOURNEYMAN;

    if (player->HasSpell(spell_apprentice))
        return TRADESKILL_LEVEL_APPRENTICE;

    return TRADESKILL_LEVEL_NONE;
}
