
#include "playerbot/playerbot.h"
#include "LeastHpTargetValue.h"
#include "TargetValue.h"

using namespace ai;

class FindLeastHpTargetStrategy : public FindNonCcTargetStrategy
{
public:
    FindLeastHpTargetStrategy(PlayerbotAI* ai) : FindNonCcTargetStrategy(ai)
    {
        minHealth = 0;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager)
    {
        // do not use this logic for pvp
        if (attacker->IsPlayer())
            return;

        Player* bot = ai->GetBot();
        if (IsCcTarget(attacker)) return;

        if (!result || result->GetHealth() > attacker->GetHealth())
            result = attacker;
    }

protected:
    float minHealth;
};


Unit* LeastHpTargetValue::Calculate()
{
    FindLeastHpTargetStrategy strategy(ai);
    return FindTarget(&strategy);
}
