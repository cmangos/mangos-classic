#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class NearestNpcsValue : public NearestUnitsValue
	{
	public:
        NearestNpcsValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance, bool ignoreLos = false) :
          NearestUnitsValue(ai, "nearest npcs", range, ignoreLos) {}

    protected:
        void FindUnits(std::list<Unit*> &targets);
        bool AcceptUnit(Unit* unit);
	};

    class NearestVehiclesValue : public NearestUnitsValue
    {
    public:
        NearestVehiclesValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance) :
            NearestUnitsValue(ai, "nearest vehicles", range) {}

    protected:
        void FindUnits(std::list<Unit*>& targets);
        bool AcceptUnit(Unit* unit);
    };
}
