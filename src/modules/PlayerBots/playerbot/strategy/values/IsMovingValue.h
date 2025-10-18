#pragma once
#include "playerbot/strategy/Value.h"
#include "MotionGenerators/TargetedMovementGenerator.h"

namespace ai
{
    class IsMovingValue : public BoolCalculatedValue, public Qualified
	{
	public:
        IsMovingValue(PlayerbotAI* ai, std::string name = "is moving") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate()
        {
            Unit* target = AI_VALUE(Unit*, qualifier);

            if (!target)
                return false;

            return !target->IsStopped();
        }
    };

    class IsSwimmingValue : public BoolCalculatedValue, public Qualified
	{
	public:
        IsSwimmingValue(PlayerbotAI* ai, std::string name = "is swimming") : BoolCalculatedValue(ai, name), Qualified() {}

        virtual bool Calculate()
        {
            Unit* target = AI_VALUE(Unit*, qualifier);

            if (!target)
                return false;

            return sServerFacade.IsUnderwater(target) || target->IsInSwimmableWater();
        }
    };
}
