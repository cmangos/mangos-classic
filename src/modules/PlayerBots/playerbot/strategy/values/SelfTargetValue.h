#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class SelfTargetValue : public UnitCalculatedValue
	{
	public:
        SelfTargetValue(PlayerbotAI* ai, std::string name = "self target") : UnitCalculatedValue(ai, name) {}

        virtual Unit* Calculate() { return ai->GetBot(); }
    };
}
