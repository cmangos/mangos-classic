#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class AttackerWithoutAuraTargetValue : public UnitCalculatedValue, public Qualified
	{
	public:
        AttackerWithoutAuraTargetValue(PlayerbotAI* ai) : UnitCalculatedValue(ai, "attacker without aura"), Qualified() {}

    protected:
        virtual Unit* Calculate();
	};
}
