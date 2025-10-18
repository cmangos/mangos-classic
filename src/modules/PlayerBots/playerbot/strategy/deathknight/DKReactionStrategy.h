#pragma once
#include "playerbot/strategy/generic/ReactionStrategy.h"

namespace ai
{
    class DKReactionStrategy : public ReactionStrategy
    {
    public:
        DKReactionStrategy(PlayerbotAI* ai) : ReactionStrategy(ai) {}
        std::string getName() override { return "react"; }

    private:
        void InitReactionTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
