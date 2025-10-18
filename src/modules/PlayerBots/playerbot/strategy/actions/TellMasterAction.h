#pragma once
#include "MovementActions.h"

namespace ai
{
    class TellMasterAction : public Action 
    {
    public:
        TellMasterAction(PlayerbotAI* ai, std::string text) : Action(ai, "tell master"), text(text) {}

        virtual bool Execute(Event& event)
        {
            ai->TellPlayer(GetMaster(), text);
            return true;
        }

        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        std::string text;
    };

    class OutOfReactRangeAction : public MovementAction 
    {
    public:
        OutOfReactRangeAction(PlayerbotAI* ai) : MovementAction(ai, "tell out of react range") {}

        virtual bool Execute(Event& event)
        {
            Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
            ai->TellPlayer(requester, BOT_TEXT("wait_travel_close"));
            return true;
        }

        virtual bool isUseful() 
        {
            bool canFollow = Follow(AI_VALUE(Unit*, "master target"));
            if (!canFollow)
                return false;

            return true;
        }

        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
