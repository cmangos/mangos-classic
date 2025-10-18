#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class CurrentTargetValue : public UnitManualSetValue
	{
	public:
        CurrentTargetValue(PlayerbotAI* ai, std::string name = "current target") : UnitManualSetValue(ai, nullptr, name) {}
        virtual Unit* Get();
        virtual void Set(Unit* unit);

    private:
        ObjectGuid selection;
    };
}
