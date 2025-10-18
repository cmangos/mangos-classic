#pragma once
#include "GenericActions.h"

namespace ai
{
    class RewardAction : public ChatCommandAction
    {
    public:
        RewardAction(PlayerbotAI* ai) : ChatCommandAction(ai, "reward") {}
        virtual bool Execute(Event& event) override;

    private:
        bool Reward(Player* requester, uint32 itemId, Object* pNpc);
    };
}