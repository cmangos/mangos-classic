
#include "playerbot/playerbot.h"
#include "RogueTriggers.h"
#include "RogueActions.h"

using namespace ai;

bool RiposteCastTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (!target)
		return false;

	bool isMelee = true;
	if (target->IsPlayer())
	{
		isMelee = !ai->IsRanged((Player*)target);
	}

	return SpellCanBeCastedTrigger::IsActive() && isMelee;
}