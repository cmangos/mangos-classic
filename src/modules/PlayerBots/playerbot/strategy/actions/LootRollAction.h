#pragma once
#include "QueryItemUsageAction.h"
#include "LootAction.h"
#include "GenericActions.h"

namespace ai
{
    class LootStartRollAction : public ChatCommandAction
    {
    public:
        LootStartRollAction(PlayerbotAI* ai, std::string name = "loot start roll") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "loot start roll"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This is a WorldPacket action which will stores rollable item to be rolled on later.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "active rolls" }; }
#endif 
    };

    class RollAction : public QueryItemUsageAction 
    {
    public:
        RollAction(PlayerbotAI* ai, std::string name = "roll") : QueryItemUsageAction(ai, name) {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "roll"; } //Must equal internal name
        virtual std::string GetHelpDescription()
        {
            return "This will make the bot roll a certain way on a specific item.\n"
                "Usage: roll <roll type> [itemlink].\n"
                "Types are need, greed, pass, emote and auto.\n"
                "When no itemlink is provided bots will roll on all current rollable loot.\n"
                "Examples:\n"
                "roll greed [item]\n"
                "roll auto\n"
                "See also strategies [h:strategy|roll] and [h:strategy|delayed roll].\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "item usage", "force item usage"}; }
#endif 

    protected:
        virtual ItemQualifier GetRollItem(ObjectGuid lootGuid, uint32 slot);
        virtual RollVote CalculateRollVote(ItemQualifier& itemQualifier);
        virtual bool RollOnItemInSlot(RollVote type, ObjectGuid lootGuid, uint32 slot);
    };

    class LootRollAction : public RollAction 
    {
    public:
        LootRollAction(PlayerbotAI* ai, std::string name = "loot roll") : RollAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "loot roll"; } //Must equal internal name
        virtual std::string GetHelpDescription()
        {
            return "This action will make the bot roll on an item the master just rolled on.\n"
                "The type of roll will be automatically determined based on if the bot finds the item useful.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "roll" }; }
        virtual std::vector<std::string> GetUsedValues() { return { "item usage", "force item usage" }; }
#endif 
    };

    class AutoLootRollAction : public RollAction 
    {
    public:
        AutoLootRollAction(PlayerbotAI* ai, std::string name = "auto loot roll") : RollAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isPossible();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "auto loot roll"; } //Must equal internal name
        virtual std::string GetHelpDescription()
        {
            return "This action will make the bot roll on one item that it hasn't rolled for yet.\n"
                "The type of roll will be automatically determined based on if the bot finds the item useful.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "roll" }; }
        virtual std::vector<std::string> GetUsedValues() { return { "item usage", "force item usage" }; }
#endif 
    };
}
