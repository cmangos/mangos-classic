#pragma once
#include "GenericActions.h"

namespace ai
{
    class EquipAction : public ChatCommandAction
    {
    public:
        EquipAction(PlayerbotAI* ai, std::string name = "equip") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        void EquipItems(Player* requester, ItemIds ids);
        void EquipItem(Player* requester, Item* item);
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "equip"; }
        virtual std::string GetHelpDescription()
        {
            return "This command makes the bot equip a specific item.\n"
                   "Usage: e [itemlink or itemname]\n"
                   "Example: e [Iron Sword] (equips the specified item if available)";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

    private:
        void EquipItem(Player* requester, FindItemVisitor* visitor);
        void ListItems(Player* requester);
        uint8 GetSmallestBagSlot();
    };

    class EquipUpgradesAction : public EquipAction
    {
    public:
        EquipUpgradesAction(PlayerbotAI* ai, std::string name = "equip upgrades") : EquipAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "equip upgrades"; }
        virtual std::string GetHelpDescription()
        {
            return "This command makes the bot equip any available upgrades.\n"
                   "Usage: do equip upgrades\n"
                   "It automatically scans inventory for better gear and equips it.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
