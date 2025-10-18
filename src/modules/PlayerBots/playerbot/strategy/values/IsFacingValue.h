#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class IsFacingValue : public BoolCalculatedValue, public Qualified
	{
	public:
        IsFacingValue(PlayerbotAI* ai, std::string name = "is facing") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate()
        {
            Unit* target = AI_VALUE(Unit*, qualifier);
            if (!target)
                return false;

            return sServerFacade.IsInFront(bot, target, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT);
        }
    };
}
