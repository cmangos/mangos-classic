#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class RangeValue : public ManualSetValue<float>, public Qualified
	{
	public:
        RangeValue(PlayerbotAI* ai);
        virtual std::string Save();
        virtual bool Load(std::string value);
    };
}
