#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class UsePotionsStrategy : public Strategy
    {
    public:
        UsePotionsStrategy(PlayerbotAI* ai);
        std::string getName() override { return "potions"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
