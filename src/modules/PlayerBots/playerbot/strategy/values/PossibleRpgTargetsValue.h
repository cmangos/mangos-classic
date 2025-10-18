#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class PossibleRpgTargetsValue : public NearestUnitsValue
	{
	public:
        PossibleRpgTargetsValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.rpgDistance);

    protected:
        void FindUnits(std::list<Unit*> &targets);
        bool AcceptUnit(Unit* unit);

    public:
        static std::vector<uint32> allowedNpcFlags;
	};
}
