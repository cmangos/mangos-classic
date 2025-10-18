#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class MoveToTravelTargetAction : public MovementAction {
    public:
        MoveToTravelTargetAction(PlayerbotAI* ai) : MovementAction(ai, "move to travel target") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

}
