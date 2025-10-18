#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class KiteStrategy : public Strategy
    {
    public:
        KiteStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "kite"; }
    
    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
