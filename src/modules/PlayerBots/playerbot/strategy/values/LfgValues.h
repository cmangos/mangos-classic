#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/AiFactory.h"

namespace ai
{
    class LfgProposalValue : public ManualSetValue<uint32>
    {
    public:
        LfgProposalValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "lfg proposal") {}
    };

    class BotRolesValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        BotRolesValue(PlayerbotAI* ai, std::string name = "bot roles") : Uint8CalculatedValue(ai, name, 10), Qualified() {}
        virtual uint8 Calculate()
        {
            return AiFactory::GetPlayerRoles(bot);
        }
    };
}
