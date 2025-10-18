
#include "playerbot/playerbot.h"
#include "CurrentCcTargetValue.h"

using namespace ai;

class FindCurrentCcTargetStrategy : public FindTargetStrategy
{
public:
    FindCurrentCcTargetStrategy(PlayerbotAI* ai, std::string spell) : FindTargetStrategy(ai)
    {
        this->spell = spell;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager)
    {
        if (ai->HasAura(spell, attacker))
            result = attacker;
    }

private:
    std::string spell;
};


Unit* CurrentCcTargetValue::Calculate()
{
    FindCurrentCcTargetStrategy strategy(ai, qualifier);
    return FindTarget(&strategy);
}
