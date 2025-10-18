#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
   
    class CurrentCcTargetValue : public TargetValue, public Qualified
	{
	public:
        CurrentCcTargetValue(PlayerbotAI* ai, std::string name = "current cc target") : TargetValue(ai, name), Qualified() {}

    public:
        Unit* Calculate();
    };
}
