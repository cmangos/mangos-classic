#pragma once
#include "GenericActions.h"

namespace ai
{
    class QuestRewardAction : public ChatCommandAction
    {
    public:
        QuestRewardAction(PlayerbotAI* ai) : ChatCommandAction(ai, "quest reward") {}
        virtual bool Execute(Event& event) override;
    };
}