#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/strategy/Value.h"

namespace ai
{
    class MCRunesValue : public StringCalculatedValue, public Qualified
    {
    public:
        MCRunesValue(PlayerbotAI* ai, std::string name = "mc runes") : StringCalculatedValue(ai, name, 1), Qualified() {}

        virtual std::string Calculate() { return "176951,176952,176953,176954,176955,176956,176957"; };

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "mc runes"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "entry"; }
        virtual std::string GetHelpDescription()
        {
            return "This value contains the entries of the Molten Core runes.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };
}
