
#include "playerbot/playerbot.h"
#include "RangeValues.h"

using namespace ai;

RangeValue::RangeValue(PlayerbotAI* ai)
    : ManualSetValue<float>(ai, 0, "range"), Qualified()
{
}

std::string RangeValue::Save()
{
    std::ostringstream out; out << value; return out.str();
}

bool RangeValue::Load(std::string text)
{
    value = atof(text.c_str());
    return true;
}
