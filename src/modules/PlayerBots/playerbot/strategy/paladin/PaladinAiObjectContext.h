#pragma once
#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class PaladinAiObjectContext : public AiObjectContext
    {
    public:
        PaladinAiObjectContext(PlayerbotAI* ai);
    };
}