#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class NearestFriendlyPlayersValue : public NearestUnitsValue
	{
	public:
        NearestFriendlyPlayersValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance) :
            NearestUnitsValue(ai, "nearest friendly players", range) {}

    protected:
        void FindUnits(std::list<Unit*> &targets);
        virtual bool AcceptUnit(Unit* unit);
	};
}
