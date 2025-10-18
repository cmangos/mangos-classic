#pragma once
#include "GenericActions.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

namespace ai
{
    //UsageBoughtList[usage][itemId]=count
    using BoughtList = std::unordered_map<uint32, uint32>;
    using UsageBoughtList = std::unordered_map<ItemUsage, BoughtList>;

    class BuyAction : public ChatCommandAction
    {
    public:
        BuyAction(PlayerbotAI* ai, std::string name = "buy") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;

    private:
        bool BuyItem(Player* requester, VendorItemData const* tItems, ObjectGuid vendorguid, const ItemPrototype* proto, UsageBoughtList& bought, ItemUsage usage = ItemUsage::ITEM_USAGE_NONE);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "buy"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make bots buy items from a nearby vendor.\n"
                "Usage: buy [itemlink]\n"
                "Example: buy usefull (buy based on item use)\n"
                "Example: buy [itemlink]\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "equip upgrades" }; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "item count", "item usage", "free money for"}; }
#endif 
    };

    class BuyBackAction : public BuyAction
    {
    public:
        BuyBackAction(PlayerbotAI* ai, std::string name = "buyback") : BuyAction(ai, name) {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "buyback"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make bots retrieve items after being sold.\n"
                "Usage: buyback [itemlink]\n"
                "Example: buyback all (try to get all items back)\n"
                "Example: buyback [itemlink] (get a specific item back)\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "" }; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "item count", "bag space"}; }
#endif 
    };
}
