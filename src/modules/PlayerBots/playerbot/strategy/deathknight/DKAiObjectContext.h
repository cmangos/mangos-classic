#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class DKAiObjectContext : public AiObjectContext
    {
    public:
        DKAiObjectContext(PlayerbotAI* ai);
    };
}