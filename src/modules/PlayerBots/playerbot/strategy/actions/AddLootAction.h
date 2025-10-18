#pragma once
#include "GenericActions.h"

namespace ai
{
    class AddLootAction : public ChatCommandAction
    {
    public:
        AddLootAction(PlayerbotAI* ai) : ChatCommandAction(ai, "add loot") {}
        virtual bool isUseful();

    private:
        bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "add loot"; }
        virtual std::string GetHelpDescription()
        {
            return "This action allows the bot to add a specific lootable object (like a corpse or chest) to its stored loot list.\n"
                   "The bot will move to and open this object later when it's safe and within range.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"available loot"}; }
#endif 
    };

    class AddAllLootAction : public ChatCommandAction
    {
    public:
        AddAllLootAction(PlayerbotAI* ai, std::string name = "add all loot") : ChatCommandAction(ai, name) {}
        virtual bool isUseful();

    protected:
        virtual bool AddLoot(Player* requester, ObjectGuid guid);
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "add all loot"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot add all nearby lootable objects (like corpses or chests) to its stored loot list.\n"
                   "The bot will move to and open these objects later, respecting group rules and bag space.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"nearest game objects no los", "nearest corpses", "bag space"}; }
#endif 
    };

    class AddGatheringLootAction : public AddAllLootAction
    {
    public:
        AddGatheringLootAction(PlayerbotAI* ai) : AddAllLootAction(ai, "add gathering loot") {}

    private:
        bool AddLoot(Player* requester, ObjectGuid guid) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "add gathering loot"; }
        virtual std::string GetHelpDescription()
        {
            return "This action directs the bot to add lootable objects that require specific gathering skills (like herbs or minerals) to its stored loot list.\n"
                   "The bot will move to and gather from these objects later if it's safe and within range.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"nearest game objects no los", "bag space"}; }
#endif 
    };
}
