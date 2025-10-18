#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/LootObjectStack.h"
#include "playerbot/ServerFacade.h"

namespace ai
{

    class AvailableLootValue : public ManualSetValue<LootObjectStack*>
	{
	public:
        AvailableLootValue(PlayerbotAI* ai, std::string name = "available loot") : ManualSetValue<LootObjectStack*>(ai, NULL, name)
        {
            value = new LootObjectStack(bot);
        }

        virtual ~AvailableLootValue()
        {
            if (value)
                delete value;
        }
    };

    class LootTargetValue : public ManualSetValue<LootObject>
    {
    public:
        LootTargetValue(PlayerbotAI* ai, std::string name = "loot target") : ManualSetValue<LootObject>(ai, LootObject(), name) {}
    };

    class CanLootValue : public BoolCalculatedValue
    {
    public:
        CanLootValue(PlayerbotAI* ai, std::string name = "can loot") : BoolCalculatedValue(ai, name) {}

        virtual bool Calculate()
        {
            LootObject loot = AI_VALUE(LootObject, "loot target");

            return !loot.IsEmpty() &&
                    loot.GetWorldObject(bot) &&    
                    loot.IsLootPossible(bot) &&
                    sServerFacade.IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), INTERACTION_DISTANCE);
        }
    };
}
