#pragma once
#include "playerbot/strategy/Value.h"
#include "PartyMemberValue.h"

namespace ai
{
    class PartyMemberToResurrect : public PartyMemberValue
	{
	public:
        PartyMemberToResurrect(PlayerbotAI* ai, std::string name = "party member to resurrect") :
          PartyMemberValue(ai,name) {}
    
    protected:
        virtual Unit* Calculate();
	};
}
