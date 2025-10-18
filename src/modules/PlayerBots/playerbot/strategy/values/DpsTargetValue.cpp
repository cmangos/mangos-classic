
#include "playerbot/playerbot.h"
#include "DpsTargetValue.h"

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

        if (IsCcTarget(attacker)) return;

        Group* group = ai->GetBot()->GetGroup();
        if (group)
        {
            uint64 guid = group->GetTargetIcon(4);
            if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
                return;
        }
        if (!result || result->GetHealth() > attacker->GetHealth())
            result = attacker;
    }

protected:
    float minHealth;
};

Unit* DpsTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindLeastHpTargetStrategy strategy(ai);
    return TargetValue::FindTarget(&strategy);
}

class FindMaxHpTargetStrategy : public FindTargetStrategy
{
public:
    FindMaxHpTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai)
    {
        maxHealth = 0;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager)
    {
        Group* group = ai->GetBot()->GetGroup();
        if (group)
        {
            uint64 guid = group->GetTargetIcon(4);
            if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
                return;
        }
        if (!result || result->GetHealth() < attacker->GetHealth())
            result = attacker;
    }

protected:
    float maxHealth;
};

Unit* DpsAoeTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindMaxHpTargetStrategy strategy(ai);
    return TargetValue::FindTarget(&strategy);
}
