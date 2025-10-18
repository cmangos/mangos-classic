#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/AiFactory.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class EnterVehicleAction : public MovementAction
    {
    public:
        EnterVehicleAction(PlayerbotAI* ai, std::string name = "enter vehicle") : MovementAction(ai, name) {}
        virtual bool Execute(Event& event);
        //virtual bool isUseful();
    };

    class LeaveVehicleAction : public MovementAction
    {
    public:
        LeaveVehicleAction(PlayerbotAI* ai, std::string name = "leave vehicle") : MovementAction(ai, name) {}
        virtual bool Execute(Event& event);
        //virtual bool isUseful();
    };
}
