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

#ifndef MANGOS_PETAI_H
#define MANGOS_PETAI_H

#include "UnitAI.h"
#include "Entities/ObjectGuid.h"
#include "Timer.h"

class Creature;
class Spell;

class PetAI : public UnitAI
{
    public:

        explicit PetAI(Creature* creature);
        explicit PetAI(Unit* unit);

        void MoveInLineOfSight(Unit* who) override;
        void AttackStart(Unit* who) override;
        void EnterEvadeMode() override;
        void AttackedBy(Unit* attacker) override;

        void UpdateAI(const uint32 diff) override;
        static int Permissible(const Creature* creature);

    protected:
        std::string GetAIName() override { return "PetAI"; }
        Creature* m_creature; // TODO: Make PetAI only available for creatures

    private:
        void UpdateAllies();

        bool inCombat;

        GuidSet m_AllySet;
        uint32 m_updateAlliesTimer;

        float m_followAngle;
        float m_followDist;
};
#endif
