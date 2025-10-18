#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "RtiTargetValue.h"

namespace ai
{
    class TankTargetValue : public RtiTargetValue
	{
	public:
        TankTargetValue(PlayerbotAI* ai, std::string type = "rti", std::string name = "tank target") : RtiTargetValue(ai, type, name) {}

    public:
        Unit* Calculate();
    };
}
