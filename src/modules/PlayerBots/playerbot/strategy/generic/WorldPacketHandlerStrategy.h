#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class WorldPacketHandlerStrategy : public PassTroughStrategy
    {
    public:
        WorldPacketHandlerStrategy(PlayerbotAI* ai);
        std::string getName() override { return "default"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ReadyCheckStrategy : public PassTroughStrategy
    {
    public:
        ReadyCheckStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai) {}
        std::string getName() override { return "ready check"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
