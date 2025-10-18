#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class QuestStrategy : public PassTroughStrategy
    {
    public:
        QuestStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override {}
    };

    class DefaultQuestStrategy : public QuestStrategy
    {
    public:
        DefaultQuestStrategy(PlayerbotAI* ai) : QuestStrategy(ai) {}
        std::string getName() override { return "quest"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class AcceptAllQuestsStrategy : public QuestStrategy
    {
    public:
        AcceptAllQuestsStrategy(PlayerbotAI* ai) : QuestStrategy(ai) {}
        std::string getName() override { return "accept all quests"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
