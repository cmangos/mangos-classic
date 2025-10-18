#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class LastSaidValue : public ManualSetValue<time_t>, public Qualified
	{
	public:
        LastSaidValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, time(0) - 120, "last said"), Qualified() {}
    };

    class LastEmoteValue : public ManualSetValue<time_t>, public Qualified
	{
	public:
        LastEmoteValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, time(0) - 120, "last emote"), Qualified() {}
    };
}
