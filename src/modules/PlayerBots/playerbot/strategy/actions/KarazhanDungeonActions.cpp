
#include "playerbot/playerbot.h"
#include "KarazhanDungeonActions.h"
#include "playerbot/strategy/Action.h"

using namespace ai;

bool RemoveNetherPortalBuffsFromNetherspiteAction::Execute(Event& event)
{
	Unit* target = AI_VALUE(Unit*, "current target");
	if (target && ai->HasAura(30466, target))
		target->RemoveAurasDueToSpell(30466);

	if (target && ai->HasAura(30467, target))
		target->RemoveAurasDueToSpell(30467);

	if (target && ai->HasAura(30468, target))
		target->RemoveAurasDueToSpell(30468);

	return true;
}