#pragma once
#include "GenericActions.h"

namespace ai
{
    class UnequipAction : public ChatCommandAction
    {
    public:
        UnequipAction(PlayerbotAI* ai) : ChatCommandAction(ai, "unequip") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void UnequipItem(Player* requester, Item* item);
        void UnequipItem(Player* requester, FindItemVisitor* visitor);
    };
}