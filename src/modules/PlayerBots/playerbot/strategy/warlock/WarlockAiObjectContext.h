#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class WarlockAiObjectContext : public AiObjectContext
    {
    public:
        WarlockAiObjectContext(PlayerbotAI* ai);
    };
}