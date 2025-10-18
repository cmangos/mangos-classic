#pragma once
#include "GenericActions.h"

namespace ai
{
    class BankAction : public ChatCommandAction
    {
    public:
        BankAction(PlayerbotAI* ai) : ChatCommandAction(ai, "bank") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "bank"; }
        virtual std::string GetHelpDescription()
        {
            return "This command allows the bot to interact with the bank:\n"
                "Usage: bank [command] [itemlink/itemid]\n"
                "Example: bank ? (lists items in bank)\n"
                "Example: bank -[itemlink] (withdraws item)\n"
                "Example: bank [itemlink] (deposits specified item)";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

    private:
        bool ExecuteCommand(Player* requester, const std::string& text, Unit* bank);
        void ListItems(Player* requester);
        bool Withdraw(Player* requester, const uint32 itemid);
        bool Deposit(Player* requester, Item* pItem);
        Item* FindItemInBank(uint32 ItemId);
    };
}
