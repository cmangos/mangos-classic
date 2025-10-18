#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class PriestAiObjectContext : public AiObjectContext
    {
    public:
        PriestAiObjectContext(PlayerbotAI* ai);
    };
}