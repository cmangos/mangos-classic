
#include "playerbot/playerbot.h"
#include "TankTargetValue.h"
#include "PossibleAttackTargetsValue.h"

using namespace ai;

class FindTargetForTankStrategy : public FindNonCcTargetStrategy
{
public:
    FindTargetForTankStrategy(PlayerbotAI* ai) : FindNonCcTargetStrategy(ai)
    {
        minThreat = 0;
    }

public:
    virtual void CheckAttacker(Unit* creature, ThreatManager* threatManager)
    {
        Player* player = ai->GetBot();
        if (IsCcTarget(creature)) return;

        if (!PossibleAttackTargetsValue::IsValid(creature, player))
        {
            std::list<ObjectGuid> attackers = PAI_VALUE(std::list<ObjectGuid>, "possible attack targets");
            if (std::find(attackers.begin(), attackers.end(), creature->GetObjectGuid()) == attackers.end())
                return;
        }

        float threat = threatManager->getThreat(player);
        if (!result || (minThreat - threat) > 0.1f)
        {
            minThreat = threat;
            result = creature;
        }
    }

protected:
    float minThreat;
};


Unit* TankTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindTargetForTankStrategy strategy(ai);
    return FindTarget(&strategy);
}
