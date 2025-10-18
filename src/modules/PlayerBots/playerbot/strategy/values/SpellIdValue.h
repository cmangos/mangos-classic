#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class SpellIdValue : public CalculatedValue<uint32>, public Qualified
	{
	public:
        SpellIdValue(PlayerbotAI* ai);
        virtual uint32 Calculate();
    };

    class VehicleSpellIdValue : public CalculatedValue<uint32>, public Qualified
    {
    public:
        VehicleSpellIdValue(PlayerbotAI* ai);
        virtual uint32 Calculate();
    };
}
