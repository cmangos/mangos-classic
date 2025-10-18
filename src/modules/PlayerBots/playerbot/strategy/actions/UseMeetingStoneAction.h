#pragma once
#include "MovementActions.h"

namespace ai
{
    class SummonAction : public MovementAction
    {
    public:
        SummonAction(PlayerbotAI* ai, std::string name = "summon") : MovementAction(ai, name) {}
        bool isPossible() override { return true; }
        bool isUseful() override { return true; }
        bool isUsefulWhenStunned() override { return true; }
        virtual bool Execute(Event& event);

    protected:
        bool Teleport(Player* requester, Player *summoner, Player *player);
        bool SummonUsingGos(Player* requester, Player *summoner, Player *player);
        bool SummonUsingNpcs(Player* requester, Player *summoner, Player *player);
    };

    class UseMeetingStoneAction : public SummonAction
    {
    public:
        UseMeetingStoneAction(PlayerbotAI* ai) : SummonAction(ai, "use meeting stone") {}
        virtual bool Execute(Event& event);
    };

    class AcceptSummonAction : public Action
    {
    public:
        AcceptSummonAction(PlayerbotAI* ai) : Action(ai, "accept summon") {}
        virtual bool Execute(Event& event);
    };
}
