#pragma once

#include "AiObject.h"

class PlayerbotAI;

namespace ai
{
    class Action;
}

namespace ai
{
    class Multiplier : public AiNamedObject
    {
    public:
        Multiplier(PlayerbotAI* ai, std::string name) : AiNamedObject(ai, name) {}
        virtual ~Multiplier() {}

    public:
        virtual float GetValue(Action* action) { return 1.0f; }
    };

}
