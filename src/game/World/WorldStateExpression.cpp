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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include "WorldStateExpression.h"
#include "Entities/Player.h"
#include "GameEvents/GameEventMgr.h"
#include "Globals/Conditions.h"
#include "Globals/ObjectMgr.h"

#include <string_view>

void HexStrToByteArray(std::string_view str, uint8* out, size_t outlen, bool reverse)
{
    MANGOS_ASSERT(str.size() == (2 * outlen));

    int32 init = 0;
    int32 end = int32(str.length());
    int8 op = 1;

    if (reverse)
    {
        init = int32(str.length() - 2);
        end = -2;
        op = -1;
    }

    uint32 j = 0;
    for (int32 i = init; i != end; i += 2 * op)
    {
        char buffer[3] = { str[i], str[i + 1], '\0' };
        out[j++] = uint8(strtoul(buffer, nullptr, 16));
    }
}

ByteBuffer HexToBytes(const std::string& hex)
{
    ByteBuffer buffer(hex.length() / 2, ByteBuffer::Resize{});
    HexStrToByteArray(hex, const_cast<uint8*>(buffer.contents()), buffer.size(), false);
    return buffer;
}

static int32(* const WorldStateExpressionFunctions[WSE_FUNCTION_MAX])(Map const*, uint32, uint32) =
{
    // WSE_FUNCTION_NONE
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_RANDOM
    [](Map const* /*map*/, uint32 arg1, uint32 arg2) -> int32
    {
        return irand(std::min(arg1, arg2), std::max(arg1, arg2));
    },

    // WSE_FUNCTION_MONTH
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return map->GetCurrentTime_tm().tm_mon + 1;
    },

    // WSE_FUNCTION_DAY
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return map->GetCurrentTime_tm().tm_mday + 1;
    },

    // WSE_FUNCTION_TIME_OF_DAY
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        tm const& localTime = map->GetCurrentTime_tm();
    return localTime.tm_hour* MINUTE + localTime.tm_min;
    },

    // WSE_FUNCTION_REGION
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0x1;
        // return realm.Id.Region;
    },

    // WSE_FUNCTION_CLOCK_HOUR
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        uint32 currentHour = map->GetCurrentTime_tm().tm_hour + 1;
        return currentHour <= 12 ? (currentHour ? currentHour : 12) : currentHour - 12;
    },

    // WSE_FUNCTION_OLD_DIFFICULTY_ID
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        //if (DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(unit->GetMap()->GetDifficultyID()))
        //    return difficulty->OldEnumValue;

        return -1;
    },

    // WSE_FUNCTION_HOLIDAY_START
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_HOLIDAY_LEFT
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_HOLIDAY_ACTIVE
    [](Map const* /*map*/, uint32 arg1, uint32 /*arg2*/) -> int32
    {
        return int32(sGameEventMgr.IsActiveHoliday(HolidayIds(arg1)));
    },

    // WSE_FUNCTION_TIMER_CURRENT_TIME
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return map->GetCurrentClockTime().time_since_epoch().count();
    },

    // WSE_FUNCTION_WEEK_NUMBER
    [](Map const* map, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        auto now = map->GetCurrentClockTime().time_since_epoch().count();
        uint32 raidOrigin = 1135695600;
        //if (Cfg_RegionsEntry const* region = sCfgRegionsStore.LookupEntry(realm.Id.Region))
        //    raidOrigin = region->Raidorigin;

        return (now - raidOrigin) / WEEK;
    },

    // WSE_FUNCTION_UNK13
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK14
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_DIFFICULTY_ID
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_WAR_MODE_ACTIVE
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
        // return unit->HasPlayerFlag(PLAYER_FLAGS_WAR_MODE_ACTIVE);
    },

    // WSE_FUNCTION_UNK17
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK18
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK19
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK20
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK21
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_WORLD_STATE_EXPRESSION
    [](Map const* map, uint32 arg1, uint32 /*arg2*/) -> int32
    {
        return sObjectMgr.IsWorldStateExpressionSatisfied(arg1, map);
    },

    // WSE_FUNCTION_KEYSTONE_AFFIX
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK24
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK25
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK26
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK27
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_KEYSTONE_LEVEL
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK29
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK30
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK31
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK32
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_MERSENNE_RANDOM
    [](Map const* /*map*/, uint32 arg1, uint32 arg2) -> int32
    {
        if (arg1 == 1)
            return 1;

        // init with predetermined seed
        std::mt19937 mt(arg2 ? arg2 : 1);
        return mt() % arg1 + 1;
    },

    // WSE_FUNCTION_UNK34
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK35
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UNK36
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_UI_WIDGET_DATA
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },

    // WSE_FUNCTION_TIME_EVENT_PASSED
    [](Map const* /*map*/, uint32 /*arg1*/, uint32 /*arg2*/) -> int32
    {
        return 0;
    },
};

int32 EvalSingleValue(ByteBuffer& buffer, Map const* map)
{
    WorldStateExpressionValueType valueType = buffer.read<WorldStateExpressionValueType>();
    int32 value = 0;

    switch (valueType)
    {
        case WorldStateExpressionValueType::CONSTANT:
        {
            value = buffer.read<int32>();
            break;
        }
        case WorldStateExpressionValueType::WORLDSTATE:
        {
            uint32 worldStateId = buffer.read<uint32>();
            value = map->GetVariableManager().GetVariable(worldStateId);
            break;
        }
        case WorldStateExpressionValueType::FUNCTION:
        {
            uint32 functionType = buffer.read<uint32>();
            int32 arg1 = EvalSingleValue(buffer, map);
            int32 arg2 = EvalSingleValue(buffer, map);

            if (functionType >= WSE_FUNCTION_MAX)
                return 0;

            value = WorldStateExpressionFunctions[functionType](map, arg1, arg2);
            break;
        }
        default:
            break;
    }

    return value;
}

int32 EvalValue(ByteBuffer& buffer, Map const* map)
{
    int32 leftValue = EvalSingleValue(buffer, map);

    WorldStateExpressionOperatorType operatorType = buffer.read<WorldStateExpressionOperatorType>();
    if (operatorType == WorldStateExpressionOperatorType::NONE)
        return leftValue;

    int32 rightValue = EvalSingleValue(buffer, map);

    switch (operatorType)
    {
        case WorldStateExpressionOperatorType::SUM:             return leftValue + rightValue;
        case WorldStateExpressionOperatorType::SUBTRACTION:    return leftValue - rightValue;
        case WorldStateExpressionOperatorType::MULTIPLICATION:  return leftValue * rightValue;
        case WorldStateExpressionOperatorType::DIVISION:        return !rightValue ? 0 : leftValue / rightValue;
        case WorldStateExpressionOperatorType::REMAINDER:       return !rightValue ? 0 : leftValue % rightValue;
        default:
            break;
    }

    return leftValue;
}

bool EvalRelOp(ByteBuffer& buffer, Map const* map)
{
    int32 leftValue = EvalValue(buffer, map);

    ConditionOperation compareLogic = buffer.read<ConditionOperation>();
    if (compareLogic == ConditionOperation::NONE)
        return leftValue != 0;

    int32 rightValue = EvalValue(buffer, map);

    return ConditionEntry::CheckOp(compareLogic, leftValue, rightValue);
}

std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> WorldStateExpressionMgr::Load()
{
    auto expressions = std::make_shared<std::map<int32, WorldStateExpressionEntry>>();

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT * FROM worldstate_expression"));
    if (!result)
    {
        sLog.outString(">> Loaded 0 worldstate_expression definitions - table is empty.");
        sLog.outString();
        m_expressions = expressions;
        return expressions;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 Count = 0;

    do
    {
        bar.step();
        Field* fields = result->Fetch();

        WorldStateExpressionEntry conditionEntry;
        conditionEntry.Id = fields[0].GetUInt32();
        conditionEntry.Expression = fields[1].GetCppString();

        expressions->emplace(conditionEntry.Id, conditionEntry);
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u UnitCondition definitions", Count);
    sLog.outString();

    m_expressions = expressions;

    return expressions;
}

bool WorldStateExpressionMgr::Meets(Map const* map, int32 Id)
{
    auto expressions = map->GetMapDataContainer().GetWorldStateExpressions();
    auto itr = expressions->find(Id);
    if (expressions->end() == itr)
        return false;

    WorldStateExpressionEntry const& expression = itr->second;

    ByteBuffer buffer = HexToBytes(expression.Expression);
    if (buffer.empty())
        return false;

    bool enabled = buffer.read<bool>();
    if (!enabled)
        return false;

    bool finalResult = EvalRelOp(buffer, map);
    ConditionLogic resultLogic = buffer.read<ConditionLogic>();

    while (resultLogic != ConditionLogic::NONE)
    {
        bool secondResult = EvalRelOp(buffer, map);

        switch (resultLogic)
        {
            case ConditionLogic::AND: finalResult = finalResult && secondResult; break;
            case ConditionLogic::OR:  finalResult = finalResult || secondResult; break;
            case ConditionLogic::XOR: finalResult = finalResult != secondResult; break;
            default:
                break;
        }

        if (buffer.rpos() < buffer.size())
            break;

        resultLogic = buffer.read<ConditionLogic>();
    }

    return finalResult;
}

bool WorldStateExpressionMgr::Exists(int32 Id)
{
    return m_expressions->find(Id) != m_expressions->end();
}
