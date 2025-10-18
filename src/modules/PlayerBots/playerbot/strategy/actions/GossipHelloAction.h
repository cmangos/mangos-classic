#pragma once
#include "GenericActions.h"

namespace ai
{
    class GossipHelloAction : public ChatCommandAction
    {
    public:
        GossipHelloAction(PlayerbotAI* ai) : ChatCommandAction(ai, "gossip hello") {}
        virtual bool Execute(Event& event) override;

    private:
        void TellGossipMenus(Player* requester);
        bool ProcessGossip(Player* requester, ObjectGuid creatureGuid, int menuToSelect);
        void TellGossipText(Player* requester, uint32 textId);
    };
}
