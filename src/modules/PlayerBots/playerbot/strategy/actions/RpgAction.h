#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"

namespace ai
{
    class RpgAction : public MovementAction {
    public:
        RpgAction(PlayerbotAI* ai, std::string name = "rpg") : MovementAction(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();

    protected:
        virtual bool SetNextRpgAction();

        typedef void (RpgAction::* RpgElement) (ObjectGuid guid);
        virtual bool AddIgnore(ObjectGuid guid);
        virtual bool RemIgnore(ObjectGuid guid);
        virtual bool HasIgnore(ObjectGuid guid);   
    };

    class CRpgAction : public RpgAction {
    public:
        CRpgAction(PlayerbotAI* ai) : RpgAction(ai, "crpg") {}

        virtual bool isUseful() { RESET_AI_VALUE(GuidPosition,"rpg target"); return true; };
    };
}
