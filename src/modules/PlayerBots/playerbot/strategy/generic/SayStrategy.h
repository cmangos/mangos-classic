#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class SayStrategy : public Strategy
    {
    public:
        SayStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "say"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
