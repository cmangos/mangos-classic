#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class MyThreatValue : public LogCalculatedValue<float>, public Qualified
    {
    public:
        MyThreatValue(PlayerbotAI* ai, std::string name = "my threat") : LogCalculatedValue(ai, name), Qualified() { minChangeInterval = 1; lastTarget = ObjectGuid(); }
        virtual bool EqualToLast(float value) { return value == lastValue; }
        virtual float Calculate();

    public:
        ObjectGuid lastTarget;
    };

    class TankThreatValue : public FloatCalculatedValue, public Qualified
    {
    public:
        TankThreatValue(PlayerbotAI* ai, std::string name = "tank threat") : FloatCalculatedValue(ai, name), Qualified() {}
        virtual float Calculate();
    };

    class ThreatValue : public Uint8CalculatedValue, public Qualified
	{
	public:
        ThreatValue(PlayerbotAI* ai, std::string name = "threat") : Uint8CalculatedValue(ai, name), Qualified() {}
    	virtual uint8 Calculate();
        static float GetThreat(Player* player, Unit* target);
        static float GetTankThreat(PlayerbotAI* ai, Unit* target);

    protected:
    	uint8 Calculate(Unit* target);
    };
}
