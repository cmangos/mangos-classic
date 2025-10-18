
#include "playerbot/playerbot.h"
#include "PassiveStrategy.h"
#include "playerbot/strategy/PassiveMultiplier.h"

using namespace ai;

void PassiveStrategy::InitNonCombatMultipliers(std::list<Multiplier*> &multipliers)
{
    multipliers.push_back(new PassiveMultiplier(ai));
}

void PassiveStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    InitNonCombatMultipliers(multipliers);
}