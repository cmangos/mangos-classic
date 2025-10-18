#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class MageAiObjectContext : public AiObjectContext
    {
    public:
        MageAiObjectContext(PlayerbotAI* ai);
    };
}