#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    class ApplyGlyphTrigger : public RandomTrigger
    {
    public:
        ApplyGlyphTrigger(PlayerbotAI* ai, std::string triggerName = "apply glyphs") : RandomTrigger(ai, triggerName, 60) {}
        virtual bool IsActive()
        {
            if (bot->GetLevel() < 15)
                return false;

            return RandomTrigger::IsActive();
        }
    };   
}