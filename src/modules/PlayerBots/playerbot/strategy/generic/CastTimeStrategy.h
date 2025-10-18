#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class CastTimeMultiplier : public Multiplier
    {
    public:
        CastTimeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "cast time") {}

    public:
        virtual float GetValue(Action* action);
    };

    class CastTimeStrategy : public Strategy
    {
    public:
        CastTimeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "cast time"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "cast time"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy will make bots less likely to cast long casttime spells when the target is at critical health.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { }; }
#endif
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers);
    };
}
