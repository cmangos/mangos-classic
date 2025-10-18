#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class WorldBuffAction : public Action {
    public:
        WorldBuffAction(PlayerbotAI* ai) : Action(ai, "world buff") {}
        virtual bool Execute(Event& event);
        virtual bool isUsefulWhenStunned() override { return true; }
        static std::vector<uint32> NeedWorldBuffs(Unit* unit);
        //static bool AddAura(Unit* unit, uint32 spellId);
    private:
    };

}
