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

#ifndef MANGOS_PLAYER_AI_H
#define MANGOS_PLAYER_AI_H

#include "AI/BaseAI/UnitAI.h"
#include "AI/ScriptDevAI/base/TimerAI.h"
#include "Entities/CreatureSpellList.h"
#include <functional>
#include <vector>

class Player;

// for future use - in case PlayerAI needs a different purpose from control during charms split them
class PlayerAI : public UnitAI
{
    public:
        PlayerAI(Player* player);

        void JustGotCharmed(Unit* charmer) override;
        void EnterEvadeMode() override;
        void AttackClosestEnemy() override;

        CreatureSpellList const& GetSpellList() const override { return m_spellList; }
    protected:
        void AddPlayerSpellAction(uint32 spellId, std::function<Unit*()> selector = nullptr);
        Player* m_player;

        CreatureSpellList m_spellList;
    private:
        struct SpellData
        {
            uint32 spellId;
            std::function<Unit*()> targetFinder;
            SpellData(uint32 spellId, std::function<Unit*()> targetFinder) : spellId(spellId), targetFinder(targetFinder) {}
        };

        void ExecuteSpells();
        std::vector<SpellData> m_playerSpellActions;
        bool m_spellsDisabled;
        bool m_executeTargetChange;
};

UnitAI* GetClassAI(Classes playerClass, Player* player);

#endif