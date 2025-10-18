#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class RacialsStrategy : public Strategy
    {
    public:
        RacialsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "racials"; }
    
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}