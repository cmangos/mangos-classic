#pragma once

namespace ai
{
    // TO DO: Delete this when no more dependencies
    class ReactionStrategy : public Strategy
    {
    public:
        ReactionStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual int GetType() override { return STRATEGY_TYPE_REACTION; }

    protected:
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
