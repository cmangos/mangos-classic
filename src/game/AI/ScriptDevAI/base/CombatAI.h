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

#ifndef COMBAT_AI_H
#define COMBAT_AI_H

#include "Entities/Creature.h"
#include "AI/ScriptDevAI/include/sc_creature.h"

/*
    Small wrapper class that reduces the amount of code needed to use CombatActions and adds proper spell action reset functionality
*/
class CombatAI : public ScriptedAI
{
    public:
        CombatAI(Creature* creature, uint32 combatActions);

        void Reset() override;

        void HandleDelayedInstantAnimation(SpellEntry const* spellInfo) override;
        void HandleTargetRestoration();
        bool IsTargetingRestricted();
        void StopTargeting(bool state) { m_stopTargeting = state; }

        void AddOnKillText(int32 text);
        template<typename... Targs>
        void AddOnKillText(int32 value, Targs... fargs)
        {
            AddOnKillText(value);
            AddOnKillText(fargs...);
        }
        void KilledUnit(Unit* /*victim*/) override;
    private:
        ObjectGuid m_storedTarget;

        std::vector<int32> m_onDeathTexts;
        bool m_onKillCooldown;

        bool m_stopTargeting;
};

#endif
