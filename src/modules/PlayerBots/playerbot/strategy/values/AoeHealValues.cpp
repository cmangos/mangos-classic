
#include "playerbot/playerbot.h"
#include "AoeHealValues.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

uint8 AoeHealValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
    	return 0;

    float range = 0;
    if (qualifier == "low")
    	range = sPlayerbotAIConfig.lowHealth;
    else if (qualifier == "medium")
    	range = sPlayerbotAIConfig.mediumHealth;
    else if (qualifier == "critical")
    	range = sPlayerbotAIConfig.criticalHealth;

    uint8 count = 0;
	Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
	for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
	{
		Player *player = sObjectMgr.GetPlayer(itr->guid);
		if( !player || !sServerFacade.IsAlive(player) || sServerFacade.GetDistance2d(bot, player) > 30.0f)
			continue;

        if (bot->getClass() == CLASS_PRIEST && !bot->IsInGroup(player, true))
            continue;

	    float percent = (static_cast<float> (player->GetHealth()) / player->GetMaxHealth()) * 100;
	    if (percent <= range)
	    	count++;
	}

	return count;
}

