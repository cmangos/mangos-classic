#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class ThreatMultiplier : public Multiplier
    {
    public:
        ThreatMultiplier(PlayerbotAI* ai) : Multiplier(ai, "threat") {}

    public:
        float GetValue(Action* action) override;
    };

    class ThreatStrategy : public Strategy
    {
    public:
        ThreatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "threat"; }

    private:
        void InitCombatMultipliers(std::list<Multiplier*> &multipliers) override;
    };
}
