#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class NearestNonBotPlayersValue : public NearestUnitsValue
	{
	public:
        NearestNonBotPlayersValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.reactDistance) :
          NearestUnitsValue(ai, "nearest non bot players", range, true) {}

    protected:
        void FindUnits(std::list<Unit*> &targets);
        bool AcceptUnit(Unit* unit);
	};
}
