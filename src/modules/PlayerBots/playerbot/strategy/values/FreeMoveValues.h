#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{    
    class FreeMoveCenterValue : public GuidPositionCalculatedValue
	{
	public:
        FreeMoveCenterValue(PlayerbotAI* ai) : GuidPositionCalculatedValue(ai, "free move center", 5) {};
        virtual GuidPosition Calculate() override;
    };   

    class FreeMoveRangeValue : public FloatCalculatedValue
    {
    public:
        FreeMoveRangeValue(PlayerbotAI* ai) : FloatCalculatedValue(ai, "free move range", 2) {};
        virtual float Calculate() override;
    };

    class CanFreeMoveToValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanFreeMoveToValue(PlayerbotAI* ai, std::string name = "can free move to", int checkInterval = 2) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {};
        virtual bool Calculate() override;
    protected:
        virtual float GetRange() { return AI_VALUE(float, "free move range"); }
    };

    class CanFreeTargetValue : public CanFreeMoveToValue
    {
    public:
        CanFreeTargetValue(PlayerbotAI* ai) : CanFreeMoveToValue(ai, "can free target") {};
        virtual float GetRange() { return ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) ? std::min(ai->GetRange("spell"), AI_VALUE(float, "free move range")) : AI_VALUE(float, "free move range"); }
    };

    class CanFreeAttackValue : public CanFreeMoveToValue
    {
    public:
        CanFreeAttackValue(PlayerbotAI* ai) : CanFreeMoveToValue(ai, "can free attack", 1) {};
        virtual float GetRange() { return ai->GetRange("attack"); }
    };

};

