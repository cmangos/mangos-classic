#pragma once
#include "playerbot/strategy/Value.h"
#include "PartyMemberValue.h"

namespace ai
{
    class PartyMemberToDispel : public PartyMemberValue, public Qualified
	{
	public:
        PartyMemberToDispel(PlayerbotAI* ai, std::string name = "party member to dispel") :
          PartyMemberValue(ai, name), Qualified() {}

    protected:
        virtual Unit* Calculate();
	};
}
