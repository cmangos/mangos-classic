#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class MarkRtiStrategy : public Strategy
    {
    public:
        MarkRtiStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "mark rti"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
