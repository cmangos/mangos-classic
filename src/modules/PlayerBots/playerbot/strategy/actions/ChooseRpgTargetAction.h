#pragma once

#include "playerbot/strategy/Action.h"
#include "RpgAction.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class ChooseRpgTargetAction : public Action {
    public:
        ChooseRpgTargetAction(PlayerbotAI* ai, std::string name = "choose rpg target") : Action(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();

        std::unordered_map<ObjectGuid, float> GetTargets(Player* requester, bool debug = false);
    private:        
        float getMaxRelevance(GuidPosition guidP);
        bool HasSameTarget(ObjectGuid guid, uint32 max, std::list<ObjectGuid>& nearGuids);

        std::unordered_map <ObjectGuid, std::string> rgpActionReason;
    };

    class ClearRpgTargetAction : public ChooseRpgTargetAction {
    public:
        ClearRpgTargetAction(PlayerbotAI* ai) : ChooseRpgTargetAction(ai, "clear rpg target") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

}
