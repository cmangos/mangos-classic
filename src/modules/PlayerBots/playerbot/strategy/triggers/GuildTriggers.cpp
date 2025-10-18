
#include "playerbot/playerbot.h"
#include "GuildTriggers.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

bool LeaveLargeGuildTrigger::IsActive()
{
	if (!bot->GetGuildId())
		return false;

	if (ai->HasActivePlayerMaster())
		return false;

	if (ai->IsAlt())
		return false;

	Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());

	if (guild->GetMemberSize() >= 1000) //Try to prevent guild overflow.
		return true;

	if (!sPlayerbotAIConfig.randomBotGuildNearby)
		return false;

	if (ai->IsInRealGuild())
		return false;

	Player* leader = sObjectMgr.GetPlayer(guild->GetLeaderGuid());

	//Only leave the guild if we know the leader is not a real player.
	if (!leader || !leader->GetPlayerbotAI() || leader->GetPlayerbotAI()->IsRealPlayer())
		return false;

	uint32 members = guild->GetMemberSize();
	uint32 maxMembers = ai->GetMaxPreferedGuildSize();

	return members > maxMembers;
}

