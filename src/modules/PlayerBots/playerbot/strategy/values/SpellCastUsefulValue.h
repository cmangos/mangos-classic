#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class SpellCastUsefulValue : public BoolCalculatedValue, public Qualified
	{
	public:
        SpellCastUsefulValue(PlayerbotAI* ai, std::string name = "spell cast useful") : BoolCalculatedValue(ai, name), Qualified() {}
        virtual bool Calculate();
    };

    class SpellReadyValue : public BoolCalculatedValue, public Qualified
    {
    public:
        SpellReadyValue(PlayerbotAI* ai, std::string name = "spell ready") : BoolCalculatedValue(ai, name), Qualified() {}
        virtual bool Calculate();
    };
}
