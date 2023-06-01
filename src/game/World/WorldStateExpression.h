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

#ifndef WORLD_STATE_EXPRESSION_H
#define WORLD_STATE_EXPRESSION_H

#include "Common.h"
#include <map>
#include <string>
#include <memory>

class Map;

enum class WorldStateExpressionValueType : uint8
{
    CONSTANT    = 1,
    WORLDSTATE  = 2,
    FUNCTION    = 3
};

enum class WorldStateExpressionOperatorType : uint8
{
    NONE            = 0,
    SUM             = 1,
    SUBTRACTION     = 2,
    MULTIPLICATION  = 3,
    DIVISION        = 4,
    REMAINDER       = 5,
};

enum WorldStateExpressionFunctions
{
    WSE_FUNCTION_NONE = 0,
    WSE_FUNCTION_RANDOM,
    WSE_FUNCTION_MONTH,
    WSE_FUNCTION_DAY,
    WSE_FUNCTION_TIME_OF_DAY,
    WSE_FUNCTION_REGION,
    WSE_FUNCTION_CLOCK_HOUR,
    WSE_FUNCTION_OLD_DIFFICULTY_ID,
    WSE_FUNCTION_HOLIDAY_START,
    WSE_FUNCTION_HOLIDAY_LEFT,
    WSE_FUNCTION_HOLIDAY_ACTIVE,
    WSE_FUNCTION_TIMER_CURRENT_TIME,
    WSE_FUNCTION_WEEK_NUMBER,
    WSE_FUNCTION_UNK13,
    WSE_FUNCTION_UNK14,
    WSE_FUNCTION_DIFFICULTY_ID,
    WSE_FUNCTION_WAR_MODE_ACTIVE,
    WSE_FUNCTION_UNK17,
    WSE_FUNCTION_UNK18,
    WSE_FUNCTION_UNK19,
    WSE_FUNCTION_UNK20,
    WSE_FUNCTION_UNK21,
    WSE_FUNCTION_WORLD_STATE_EXPRESSION,
    WSE_FUNCTION_KEYSTONE_AFFIX,
    WSE_FUNCTION_UNK24,
    WSE_FUNCTION_UNK25,
    WSE_FUNCTION_UNK26,
    WSE_FUNCTION_UNK27,
    WSE_FUNCTION_KEYSTONE_LEVEL,
    WSE_FUNCTION_UNK29,
    WSE_FUNCTION_UNK30,
    WSE_FUNCTION_UNK31,
    WSE_FUNCTION_UNK32,
    WSE_FUNCTION_MERSENNE_RANDOM,
    WSE_FUNCTION_UNK34,
    WSE_FUNCTION_UNK35,
    WSE_FUNCTION_UNK36,
    WSE_FUNCTION_UI_WIDGET_DATA,
    WSE_FUNCTION_TIME_EVENT_PASSED,

    WSE_FUNCTION_MAX,
};

struct WorldStateExpressionEntry
{
    uint32 Id;
    std::string Expression;
};

class WorldStateExpressionMgr
{
    public:
        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> Load();

        bool Meets(Map const* map, int32 Id);

        bool Exists(int32 Id);

        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> Get() { return m_expressions; }
    private:
        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> m_expressions;
};

#endif

