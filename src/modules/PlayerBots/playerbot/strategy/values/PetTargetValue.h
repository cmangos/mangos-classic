#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class PetTargetValue : public UnitCalculatedValue
	{
	public:
        PetTargetValue(PlayerbotAI* ai, std::string name = "pet target") : UnitCalculatedValue(ai, name) {}

        virtual Unit* Calculate() { return (Unit*)(ai->GetBot()->GetPet()); }
    };
}
