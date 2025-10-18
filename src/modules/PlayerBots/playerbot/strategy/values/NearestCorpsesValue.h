#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class NearestCorpsesValue : public NearestUnitsValue
	{
	public:
        NearestCorpsesValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance) :
          NearestUnitsValue(ai, "nearest corpses", range) {}

    protected:
        void FindUnits(std::list<Unit*> &targets);
        bool AcceptUnit(Unit* unit);

	};
}
