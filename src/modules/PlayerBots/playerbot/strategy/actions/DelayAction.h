#pragma once

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Action.h"

namespace ai
{
    class DelayAction : public Action
    {
    public:
        DelayAction(PlayerbotAI* ai) : Action(ai, "delay")
        {}

        virtual bool Execute(Event& event)
        {
            SetDuration(sPlayerbotAIConfig.passiveDelay + sPlayerbotAIConfig.globalCoolDown);
            return true;
        }

        virtual bool isUseful()
        {
            return !ai->AllowActivity(ALL_ACTIVITY);
        }
    };
}
