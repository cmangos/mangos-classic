#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class RogueAiObjectContext : public AiObjectContext
    {
    public:
        RogueAiObjectContext(PlayerbotAI* ai);
    };
}