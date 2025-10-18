#pragma once

namespace ai
{
    class UseFoodStrategy : public Strategy
    {
    public:
        UseFoodStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "food"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
