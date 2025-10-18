#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class IsBehindValue : public BoolCalculatedValue, public Qualified
	{
	public:
        IsBehindValue(PlayerbotAI* ai) : BoolCalculatedValue(ai), Qualified() {}

        virtual bool Calculate() 
        {
            Unit* target = AI_VALUE(Unit*, qualifier);
            if (!target)
                return false;

            float targetOrientation = target->GetOrientation();
            float orientation = bot->GetOrientation();
            return bot->CanReachWithMeleeAttack(target) && abs(targetOrientation - orientation) < M_PI / 2;
        }
    };
}
