
#include "playerbot/playerbot.h"
#include "DruidValues.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

class TankWithoutLifebloomPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
	explicit TankWithoutLifebloomPredicate(PlayerbotAI* ai) :
		FindPlayerPredicate(), PlayerbotAIAware(ai)  {}

	bool Check(Unit* unit) override
	{
		if (unit && unit->IsPlayer() && sServerFacade.IsAlive(unit) && ai->IsTank((Player*)unit))
		{
			return !ai->HasAura("lifebloom", unit, true, true, -1, false, 2000, 8);
		}

		return false;
	}

private:
	std::vector<std::string> auras;
};

Unit* PartyTankWithoutLifebloomValue::Calculate()
{
	TankWithoutLifebloomPredicate predicate(ai);
	return FindPartyMember(predicate);
}