#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class ActiveSpellValue : public CalculatedValue<uint32>
	{
	public:
        ActiveSpellValue(PlayerbotAI* ai, std::string name = "active spell") : CalculatedValue<uint32>(ai, name) {}

        virtual uint32 Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "active spell"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "spell"; }
        virtual std::string GetHelpDescription() { return "This value contains the spell id of the spell that the bot is currently casting."; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
