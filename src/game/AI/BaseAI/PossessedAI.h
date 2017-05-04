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

#ifndef MANGOS_POSSESSEDAI_H
#define MANGOS_POSSESSEDAI_H

#include "CreatureAI.h"
#include "Entities/ObjectGuid.h"
#include "Timer.h"

class Creature;
class Spell;

class PossessedAI : public CreatureAI
{
public:

    explicit PossessedAI(Creature* c) : CreatureAI(c) {}
    explicit PossessedAI(Unit* unit) : CreatureAI(unit) {}

    static int Permissible(const Creature*) { return PERMIT_BASE_NO; }

    //void GetAIInformation(ChatHandler& reader) override;

    void UpdateAI(const uint32 diff) override
    {
        DoMeleeAttackIfReady();
    }
};
#endif
