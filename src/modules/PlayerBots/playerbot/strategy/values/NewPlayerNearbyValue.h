#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class NewPlayerNearbyValue : public CalculatedValue<ObjectGuid>
	{
	public:
        NewPlayerNearbyValue(PlayerbotAI* ai) : CalculatedValue<ObjectGuid>(ai, "new player nearby") {}
        virtual ObjectGuid Calculate();
    };

    class AlreadySeenPlayersValue : public ManualSetValue<std::set<ObjectGuid>& >
	{
	public:
        AlreadySeenPlayersValue(PlayerbotAI* ai) : ManualSetValue<std::set<ObjectGuid>& >(ai, data, "already seen players") {}

    private:
        std::set<ObjectGuid> data;
    };
}
