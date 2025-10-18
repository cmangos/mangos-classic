#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class DuelTargetValue : public TargetValue
	{
	public:
        DuelTargetValue(PlayerbotAI* ai, std::string name = "duel target") : TargetValue(ai, name) {}

    public:
        Unit* Calculate();
    };
}
