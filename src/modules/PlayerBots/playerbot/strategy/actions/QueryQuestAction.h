#pragma once
#include "GenericActions.h"

namespace ai
{
    class QueryQuestAction : public ChatCommandAction
    {
    public:
        QueryQuestAction(PlayerbotAI* ai) : ChatCommandAction(ai, "query quest") {}

    private:
        bool Execute(Event& event) override;
        void TellObjectives(Player* requester, uint32 questId);
        void TellObjective(Player* requester, const std::string& name, int available, int required);
    };
}