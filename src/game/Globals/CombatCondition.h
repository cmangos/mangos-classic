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

#ifndef MANGOS_COMBAT_CONDITION_H
#define MANGOS_COMBAT_CONDITION_H

#include "Common.h"
#include <map>
#include <memory>

class Unit;
class UnitConditionMgr;
class WorldStateExpressionMgr;

struct CombatConditionEntry
{
    int32 Id;
    uint32 WorldStateConditionId;   // WorldstateExpressionId
    int32 SelfConditionId;          // UnitCondition
    int32 TargetConditionId;        // UnitCondition
    uint8 FriendConditionLogic;     // CombatConditionLogic
    uint8 EnemyConditionLogic;      // CombatConditionLogic
    int32 FriendConditionId[2];     // UnitCondition
    uint8 FriendConditionOp[2];     // ConditionOperation
    uint32 FriendConditionCount[2]; // count of how many friends fulfill condition compared with op
    int32 EnemyConditionId[2];      // UnitCondition
    uint8 EnemyConditionOp[2];      // ConditionOperation
    uint32 EnemyConditionCount[2];  // count of how many enemies fulfill condition compared with op
};

class CombatConditionMgr
{
    public:
        CombatConditionMgr(UnitConditionMgr& unitConditionMgr, WorldStateExpressionMgr& worldStateExpressionMgr);

        std::shared_ptr<std::map<int32, CombatConditionEntry>> Load();

        // radius needed for friend condition
        bool Meets(Unit const* self, int32 Id, float range);

        std::shared_ptr<std::map<int32, CombatConditionEntry>> Get() { return m_combatConditions; }
    private:
        std::shared_ptr<std::map<int32, CombatConditionEntry>> m_combatConditions;
        UnitConditionMgr& m_unitConditionMgr;
        WorldStateExpressionMgr& m_worldStateExpressionMgr;
};

#endif