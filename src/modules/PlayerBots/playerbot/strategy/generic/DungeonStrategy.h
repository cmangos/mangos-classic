#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class DungeonStrategy : public Strategy
    {
    public:
        DungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "dungeon"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "dungeon"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This strategy will enable and disable various dungeon and raid specific strategies as the bot enters and leaves.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return {"onyxia's lair", "molten core" }; }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}