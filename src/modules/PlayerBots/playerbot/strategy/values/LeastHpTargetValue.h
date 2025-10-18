#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class LeastHpTargetValue : public TargetValue
	{
	public:
        LeastHpTargetValue(PlayerbotAI* ai, std::string name = "least hp target") : TargetValue(ai, name) {}

    public:
        Unit* Calculate();
    };
}
