#pragma once
#include "NonCombatStrategy.h"

namespace ai
{
    class GuildStrategy : public NonCombatStrategy
    {
    public:
        GuildStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        std::string getName() override { return "guild"; }
        int GetType() override { return STRATEGY_TYPE_GENERIC; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
