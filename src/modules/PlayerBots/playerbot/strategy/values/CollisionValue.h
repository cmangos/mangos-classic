#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class CollisionValue : public BoolCalculatedValue, public Qualified
	{
	public:
        CollisionValue(PlayerbotAI* ai, std::string name = "collision") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate();
    };
}
