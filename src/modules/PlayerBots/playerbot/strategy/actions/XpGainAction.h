#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class XpGainAction : public Action
    {
    public:
        XpGainAction(PlayerbotAI* ai) : Action(ai, "xp gain") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    private:
        void GiveXP(int32 xp, Unit* victim);
    };
}