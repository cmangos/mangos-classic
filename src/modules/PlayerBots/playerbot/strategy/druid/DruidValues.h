#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/values/PartyMemberValue.h"

namespace ai
{
    class PartyTankWithoutLifebloomValue : public PartyMemberValue, public Qualified
    {
    public:
        PartyTankWithoutLifebloomValue(PlayerbotAI* ai, std::string name = "party tank without lifebloom", float range = 40.0f) :
            PartyMemberValue(ai, name), Qualified() {}

    protected:
        Unit* Calculate() override;
    };
}