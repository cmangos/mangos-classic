#pragma once
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

#ifndef MANGOS_GUARDIANAI_H
#define MANGOS_GUARDIANAI_H

#include "PetAI.h"
#include "../EventAI/CreatureEventAI.h"
#include "Entities/ObjectGuid.h"
#include "Util/Timer.h"

class Creature;
class Spell;

class GuardianAI : public CreatureEventAI
{
    public:

        explicit GuardianAI(Creature* creature);
        static int Permissible(const Creature* creature);

        //bool IsControllable() const override { return true; }
        virtual void JustRespawned() override;
        virtual void EnterEvadeMode() override;
        virtual void UpdateAI(const uint32 diff) override;
        virtual void CombatStop() override;

        void JustDied(Unit* killer) override;

        void OnUnsummon() override;
    protected:
        std::string GetAIName() override { return "GuardianAI"; }
};
#endif
