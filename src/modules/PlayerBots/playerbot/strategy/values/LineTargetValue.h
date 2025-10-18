#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class LineTargetValue : public UnitCalculatedValue
	{
	public:
        LineTargetValue(PlayerbotAI* ai, std::string name = "line target") : UnitCalculatedValue(ai, name) {}

    public:
        virtual Unit* Calculate();
	};
}
