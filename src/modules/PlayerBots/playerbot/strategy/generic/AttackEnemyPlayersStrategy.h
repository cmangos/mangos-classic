#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class AttackEnemyPlayersStrategy : public Strategy
    {
    public:
        AttackEnemyPlayersStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "pvp"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "pvp"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy detect nearby enemy players and makes the bot attack them.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return {}; }
#endif
    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
