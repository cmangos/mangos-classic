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

#include "Globals/CombatCondition.h"
#include "Globals/UnitCondition.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Globals/Conditions.h"
#include "World/WorldStateExpression.h"

CombatConditionMgr::CombatConditionMgr(UnitConditionMgr& unitConditionMgr, WorldStateExpressionMgr& worldStateExpressionMgr)
    : m_unitConditionMgr(unitConditionMgr), m_worldStateExpressionMgr(worldStateExpressionMgr)
{
}

std::shared_ptr<std::map<int32, CombatConditionEntry>> CombatConditionMgr::Load()
{
    auto combatConditions = std::make_shared<std::map<int32, CombatConditionEntry>>();

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT * FROM combat_condition"));
    if (!result)
    {
        sLog.outString(">> Loaded 0 combat_condition definitions - table is empty.");
        sLog.outString();
        m_combatConditions = combatConditions;
        return combatConditions;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 Count = 0;

    do
    {
        bar.step();
        Field* fields = result->Fetch();

        CombatConditionEntry conditionEntry;
        conditionEntry.Id = fields[0].GetUInt32();
        conditionEntry.WorldStateConditionId	= fields[1].GetUInt32();
        if (conditionEntry.WorldStateConditionId && !m_worldStateExpressionMgr.Exists(conditionEntry.WorldStateConditionId))
        {
            sLog.outErrorDb("Table combat_condition entry %d has invalid worldstate expression %u. Setting to 0.", conditionEntry.Id, conditionEntry.WorldStateConditionId);
            conditionEntry.WorldStateConditionId = 0;
        }
        conditionEntry.SelfConditionId			= fields[2].GetUInt32();
        if (conditionEntry.SelfConditionId && !m_unitConditionMgr.Exists(conditionEntry.SelfConditionId))
        {
            sLog.outErrorDb("Table combat_condition entry %d has invalid SelfConditionId %u. Setting to 0.", conditionEntry.Id, conditionEntry.SelfConditionId);
            conditionEntry.SelfConditionId = 0;
        }
        conditionEntry.TargetConditionId		= fields[3].GetUInt32();
        if (conditionEntry.TargetConditionId && !m_unitConditionMgr.Exists(conditionEntry.TargetConditionId))
        {
            sLog.outErrorDb("Table combat_condition entry %d has invalid TargetConditionId %u. Setting to 0.", conditionEntry.Id, conditionEntry.TargetConditionId);
            conditionEntry.TargetConditionId = 0;
        }
        conditionEntry.FriendConditionLogic		= fields[4].GetUInt32();
        conditionEntry.EnemyConditionLogic		= fields[5].GetUInt32();
        for (uint32 i = 0; i < 2; ++i)
        {
            conditionEntry.FriendConditionId[i] = fields[6 + i].GetUInt32();
            if (conditionEntry.FriendConditionId[i] && !m_unitConditionMgr.Exists(conditionEntry.FriendConditionId[i]))
            {
                sLog.outErrorDb("Table combat_condition entry %d has invalid conditionEntry.FriendConditionId %u %u. Setting to 0.", conditionEntry.Id, i, conditionEntry.FriendConditionId[i]);
                conditionEntry.FriendConditionId[i] = 0;
            }
        }
        for (uint32 i = 0; i < 2; ++i)
        {
            conditionEntry.FriendConditionOp[i] = fields[8 + i].GetUInt32();
            if (conditionEntry.FriendConditionOp[i] >= uint32(ConditionOperation::MAX))
            {
                sLog.outErrorDb("Table combat_condition entry %d has invalid conditionEntry.FriendConditionOp %u %u. Setting to 0.", conditionEntry.Id, i, conditionEntry.FriendConditionOp[i]);
                conditionEntry.FriendConditionOp[i] = 0;
            }
        }
        for (uint32 i = 0; i < 2; ++i)
            conditionEntry.FriendConditionCount[i] = fields[10 + i].GetUInt32();
        for (uint32 i = 0; i < 2; ++i)
        {
            conditionEntry.EnemyConditionId[i] = fields[12 + i].GetUInt32();
            if (conditionEntry.EnemyConditionId[i] && !m_unitConditionMgr.Exists(conditionEntry.EnemyConditionId[i]))
            {
                sLog.outErrorDb("Table combat_condition entry %d has invalid conditionEntry.EnemyConditionId %u %u. Setting to 0.", conditionEntry.Id, i, conditionEntry.EnemyConditionId[i]);
                conditionEntry.EnemyConditionId[i] = 0;
            }
        }
        for (uint32 i = 0; i < 2; ++i)
        {
            conditionEntry.EnemyConditionOp[i] = fields[14 + i].GetUInt32();
            if (conditionEntry.EnemyConditionOp[i] >= uint32(ConditionOperation::MAX))
            {
                sLog.outErrorDb("Table combat_condition entry %d has invalid conditionEntry.EnemyConditionOp %u %u. Setting to 0.", conditionEntry.Id, i, conditionEntry.EnemyConditionOp[i]);
                conditionEntry.EnemyConditionOp[i] = 0;
            }
        }
        for (uint32 i = 0; i < 2; ++i)
            conditionEntry.EnemyConditionCount[i] = fields[16 + i].GetUInt32();

        combatConditions->emplace(conditionEntry.Id, conditionEntry);
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u combat_condition definitions", Count);
    sLog.outString();

    m_combatConditions = combatConditions;

    return combatConditions;
}

bool CombatConditionMgr::Meets(Unit const* self, int32 Id, float range)
{
    auto combatConditions = self->GetMap()->GetMapDataContainer().GetCombatConditions();
    auto itr = combatConditions->find(Id);
    if (combatConditions->end() == itr)
        return false;

    CombatConditionEntry& entry = itr->second;
    if (entry.WorldStateConditionId)
    {
        if (!m_worldStateExpressionMgr.Meets(self->GetMap(), entry.WorldStateConditionId))
            return false;
    }

    if (entry.SelfConditionId)
        if (!m_unitConditionMgr.Meets(self, self, entry.SelfConditionId))
            return false;

    if (entry.TargetConditionId)
        if (!self->GetTarget() || !m_unitConditionMgr.Meets(self->GetTarget(), self, entry.SelfConditionId))
            return false;

    if (entry.FriendConditionId[0])
    {
        uint32 eligibleUnits[2];
        memset(eligibleUnits, 0, sizeof(eligibleUnits));

        UnitList friendlies;
        MaNGOS::AnyFriendlyUnitInObjectRangeCheck u_check(self, range);
        MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyUnitInObjectRangeCheck> searcher(friendlies, u_check);
        Cell::VisitGridObjects(self, searcher, range);

        for (Unit* friendly : friendlies)
            for (uint32 i = 0; i < 2; ++i)
                if (entry.FriendConditionId[i] && m_unitConditionMgr.Meets(friendly, self, entry.FriendConditionId[i]))
                    ++eligibleUnits[i];

        switch (ConditionLogic(entry.FriendConditionLogic))
        {
            case ConditionLogic::NONE:
                if (!ConditionEntry::CheckOp(ConditionOperation(entry.FriendConditionOp[0]), eligibleUnits[0], entry.FriendConditionCount[0]))
                    return false;
                break;
            case ConditionLogic::AND:
            case ConditionLogic::OR:
            case ConditionLogic::XOR:
                bool results[2];
                for (uint32 i = 0; i < 2; ++i)
                    results[i] = ConditionEntry::CheckOp(ConditionOperation(entry.FriendConditionOp[i]), eligibleUnits[i], entry.FriendConditionCount[i]);

                switch (ConditionLogic(entry.FriendConditionLogic))
                {
                    case ConditionLogic::AND: if (!results[0] || !results[1]) return false; break;
                    case ConditionLogic::OR:  if (!results[0] && !results[1]) return false; break;
                    case ConditionLogic::XOR: if (results[0] == results[1])  return false; break;
                }
                break;
        }
    }

    if (entry.EnemyConditionId[0])
    {
        uint32 eligibleUnits[2];
        memset(eligibleUnits, 0, sizeof(eligibleUnits));
        if (!self->CanHaveThreatList())
        {
            for (auto const& ref : self->getHostileRefManager())
                if (Unit* victim = ref.getSource()->getOwner())
                    for (uint32 i = 0; i < 2; ++i)
                        if (entry.EnemyConditionId[i] && m_unitConditionMgr.Meets(victim, self, entry.EnemyConditionId[i]))
                            ++eligibleUnits[i];
        }
        else
        {
            for (auto ref : self->getThreatManager().getThreatList())
                if (Unit* victim = ref->getTarget())
                    for (uint32 i = 0; i < 2; ++i)
                        if (entry.EnemyConditionId[i] && m_unitConditionMgr.Meets(victim, self, entry.EnemyConditionId[i]))
                            ++eligibleUnits[i];
        }

        switch (ConditionLogic(entry.EnemyConditionLogic))
        {
            case ConditionLogic::NONE:
                if (!ConditionEntry::CheckOp(ConditionOperation(entry.EnemyConditionOp[0]), eligibleUnits[0], entry.EnemyConditionCount[0]))
                    return false;
                break;
            case ConditionLogic::AND:
            case ConditionLogic::OR:
            case ConditionLogic::XOR:
                bool results[2];
                for (uint32 i = 0; i < 2; ++i)
                    results[i] = ConditionEntry::CheckOp(ConditionOperation(entry.EnemyConditionOp[i]), eligibleUnits[i], entry.EnemyConditionCount[i]);

                switch (ConditionLogic(entry.EnemyConditionLogic))
                {
                    case ConditionLogic::AND: if (!results[0] || !results[1]) return false; break;
                    case ConditionLogic::OR:  if (!results[0] && !results[1]) return false; break;
                    case ConditionLogic::XOR: if (results[0]  == results[1])  return false; break;
                }
                break;
        }
    }

    return true;
}
