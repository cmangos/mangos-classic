#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class TravelAction : public MovementAction {
    public:
        TravelAction(PlayerbotAI* ai) : MovementAction(ai, "travel") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class MoveToDarkPortalAction : public MovementAction {
    public:
        MoveToDarkPortalAction(PlayerbotAI* ai) : MovementAction(ai, "move to dark portal") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class DarkPortalAzerothAction : public MovementAction {
    public:
        DarkPortalAzerothAction(PlayerbotAI* ai) : MovementAction(ai, "dark portal azeroth") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class MoveFromDarkPortalAction : public MovementAction {
    public:
        MoveFromDarkPortalAction(PlayerbotAI* ai) : MovementAction(ai, "move from dark portal") {}

        virtual bool Execute(Event& event);
    };

}
