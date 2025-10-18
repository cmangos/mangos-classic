#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class ActionPossibleValue : public BoolCalculatedValue, public Qualified
	{
	public:
        ActionPossibleValue(PlayerbotAI* ai, std::string name = "action possible") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate();
    };

    class ActionUsefulValue : public BoolCalculatedValue, public Qualified
    {
    public:
        ActionUsefulValue(PlayerbotAI* ai, std::string name = "action useful") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate();
    };

    class TriggerActiveValue : public BoolCalculatedValue, public Qualified
    {
    public:
        TriggerActiveValue(PlayerbotAI* ai, std::string name = "trigger active") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate();
    };

    class HasStrategyValue : public BoolCalculatedValue, public Qualified
    {
    public:
        HasStrategyValue(PlayerbotAI* ai, std::string name = "has strategy") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate();
    };
}
