
#include "playerbot/playerbot.h"
#include "RtiValue.h"

using namespace ai;

RtiValue::RtiValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "skull", "rti")
{
}

RtiCcValue::RtiCcValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "moon", "rti cc")
{
}
