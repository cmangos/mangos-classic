#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class WarriorAiObjectContext : public AiObjectContext
    {
    public:
        WarriorAiObjectContext(PlayerbotAI* ai);
    };
}