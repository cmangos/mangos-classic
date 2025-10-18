#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class LastPotionUsedTimeValue : public ManualSetValue<time_t>
	{
	public:
        LastPotionUsedTimeValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, 0) {}
    };
}
