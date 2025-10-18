#pragma once

#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class DruidAiObjectContext : public AiObjectContext
    {
    public:
        DruidAiObjectContext(PlayerbotAI* ai);
    };
}