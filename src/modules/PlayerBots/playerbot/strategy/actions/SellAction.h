#pragma once
#include "GenericActions.h"

namespace ai
{
    class SellAction : public ChatCommandAction
    {
    public:
        SellAction(PlayerbotAI* ai, std::string name = "sell") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;

        bool Sell(Player* requester, FindItemVisitor* visitor);
        bool Sell(Player* requester, Item* item);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "sell"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make bots sell items to a nearby vendor.\n"
                "Usage: sell qualifier or itemlinks\n"
                "Example: sell vendor (sell based on item use)\n"
                "Example: sell gray\n"
                "Example: sell [itemlink] [itemlink]\n"
                "Example: sell equip \n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "item usage" }; }
#endif 
    };
}