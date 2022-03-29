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

#include "Chat/Chat.h"
#include "Server/WorldSession.h"
#include "Entities/Creature.h"

bool ChatHandler::HandleNpcListSpells(char* args)
{
    Creature* creature = getSelectedCreature();
    if (!creature)
        return false;

    CreatureSpellList const& spellList = creature->GetSpellList();
    PSendSysMessage("Name %s Used %s", spellList.Name.c_str(), spellList.Disabled ? "false" : "true");
    for (auto& spellData : spellList.Spells)
    {
        CreatureSpellListSpell const& spell = spellData.second;
        PSendSysMessage("SpellID %u InitialMin %u InitialMax %u RepeatMin %u RepeatMax %u", spell.SpellId, spell.InitialMin, spell.InitialMax, spell.RepeatMin, spell.RepeatMax);
        PSendSysMessage("Availability %u Probability %u Flags %u TargetId %u TargetComment %s", spell.Availability, spell.Probability, spell.Flags, spell.Target->Id, spell.Target->Comment.c_str());
    }

    return true;
}