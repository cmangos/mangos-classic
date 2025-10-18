#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class RtiValue : public ManualSetValue<std::string>
	{
	public:
        RtiValue(PlayerbotAI* ai);
        virtual std::string Save() { return value; }
        virtual bool Load(std::string text) { value = text; return true; }
    };

    class RtiCcValue : public ManualSetValue<std::string>
    {
    public:
        RtiCcValue(PlayerbotAI* ai);

        virtual std::string Save() { return value; }
        virtual bool Load(std::string text) { value = text; return true; }
    };
}
