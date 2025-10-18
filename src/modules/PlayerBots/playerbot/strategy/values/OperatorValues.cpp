#include "playerbot/playerbot.h"
#include "playerbot/strategy/Value.h"
#include "OperatorValues.h"

namespace ai
{
    bool BoolAndValue::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (!AI_VALUE(bool, value))
                return false;
        }

        return true;
    }

    bool NotValue::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (AI_VALUE(bool, value))
                return false;
        }

        return true;
    }

    bool BoolOrValue::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (AI_VALUE(bool, value))
                return true;
        }

        return false;
    }

    bool GT32Value::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        if (values.size() < 2)
            return false;

        uint32 value1, value2;

        if (isValidNumberString(values[0]))
            value1 = stoi(values[0]);
        else
            value1 = AI_VALUE(uint32, values[0]);

        if (isValidNumberString(values[1]))
            value2 = stoi(values[1]);
        else
            value2 = AI_VALUE(uint32, values[1]);

        return value1 > value2;
    }
}
