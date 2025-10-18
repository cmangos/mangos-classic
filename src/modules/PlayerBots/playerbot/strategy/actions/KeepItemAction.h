#pragma once
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "GenericActions.h"

namespace ai
{
    class KeepItemAction : public ChatCommandAction
    {
    public:
        KeepItemAction(PlayerbotAI* ai, std::string name = "keep") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

        std::unordered_map<ForceItemUsage, std::string> keepName = 
        {
            {ForceItemUsage::FORCE_USAGE_NONE, "do not keep"},
            {ForceItemUsage::FORCE_USAGE_KEEP, "keep"},
            {ForceItemUsage::FORCE_USAGE_EQUIP, "equip"},
            {ForceItemUsage::FORCE_USAGE_GREED, "buy or greed"},
            {ForceItemUsage::FORCE_USAGE_NEED, "buy or need"}
        };

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "keep"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will force bots to keep an item in their inventory.\n"
                "The command has an optional parameter to specify what the bot should do with the items\n"
                "Usage: keep <type> [itemlink or item qualifier]\n"
                "Example: keep equip equip\n"
                "Example: keep ? all\n"
                "Example: keep need [itemlink]\n"
                "types:\n"
                "?: list item and their forced usage.\n"
                "none: the item will not be kept by the bot.\n"
                "keep: the item will not be sold or destroyed.\n"
                "equip: the item will be equipped over other items.\n"
                "greed: the bot will try to get more of this item and roll greed.\n"
                "need: the bot will try to get more of this item and roll need.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "force item usage" }; }
#endif 
    };
}
