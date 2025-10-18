#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class SkipSpellsListValue : public ManualSetValue<std::set<uint32>&>
	{
	public:
        SkipSpellsListValue(PlayerbotAI* ai, std::string name = "skip spells list") : ManualSetValue<std::set<uint32>&>(ai, list, name) {}

        virtual std::string Save();
        virtual bool Load(std::string value);

    private:
        std::set<uint32> list;
    };
}
