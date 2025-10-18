#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class AlwaysLootListValue : public ManualSetValue<std::set<uint32>&>
	{
	public:
        AlwaysLootListValue(PlayerbotAI* ai, std::string name = "always loot list") : ManualSetValue<std::set<uint32>&>(ai, list, name) {}

        virtual std::string Save();
        virtual bool Load(std::string value);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "always loot list"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "loot"; }
        virtual std::string GetHelpDescription() { return "This value contains a list of item ids that should always be looted."; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

    private:
        std::set<uint32> list;
    };
}
