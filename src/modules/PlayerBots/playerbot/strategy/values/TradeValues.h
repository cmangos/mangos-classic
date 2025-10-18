#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class ItemsUsefulToGiveValue : public CalculatedValue< std::list<Item*>>, public Qualified
	{
	public:
        ItemsUsefulToGiveValue(PlayerbotAI* ai, std::string name = "useful to give") : CalculatedValue(ai, name), Qualified() {}
        std::list<Item*> Calculate();

    private:
        bool IsTradingItem(uint32 entry);
    };

    class ItemsUsefulToEnchantValue : public CalculatedValue< std::list<Item*>>, public Qualified
    {
    public:
        ItemsUsefulToEnchantValue(PlayerbotAI* ai, std::string name = "useful to enchant") : CalculatedValue(ai, name), Qualified() {}
        std::list<Item*> Calculate();
    };
}