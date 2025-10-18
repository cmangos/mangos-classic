
#include "playerbot/playerbot.h"
#include "GuildAcceptAction.h"
#include "playerbot/ServerFacade.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

bool GuildAcceptAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p(event.getPacket());
    p.rpos(0);
    Player* inviter = nullptr;
    std::string Invitedname;
    p >> Invitedname;

    if (normalizePlayerName(Invitedname))
        inviter = ObjectAccessor::FindPlayerByName(Invitedname.c_str());

    if (!inviter)
        return false;

    std::map<std::string, std::string> placeholders;
    placeholders["%name"] = inviter->GetName();

    bool accept = true;
    uint32 guildId = inviter->GetGuildId();
    if (!guildId)
    {
        ai->TellError(requester, "You are not in a guild!");

        if(sServerFacade.GetDistance2d(bot, inviter) < sPlayerbotAIConfig.spellDistance * 1.5 && inviter->GetPlayerbotAI())
            bot->Say(BOT_TEXT2("You are not in a guild %name!", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        accept = false;
    }
    else if (bot->GetGuildId())
    {
        ai->TellError(requester, "Sorry, I am in a guild already");

        if (sServerFacade.GetDistance2d(bot, inviter) < sPlayerbotAIConfig.spellDistance * 1.5 && inviter->GetPlayerbotAI())
            bot->Say(BOT_TEXT2("Sorry, I am in a guild already %name.", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        accept = false;
    }
    else if (!ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_GUILD, false, inviter, true))
    {
        ai->TellError(requester, "Sorry, I don't want to join your guild :(");

        if (sServerFacade.GetDistance2d(bot, inviter) < sPlayerbotAIConfig.spellDistance * 1.5 && inviter->GetPlayerbotAI())
            bot->Say(BOT_TEXT2("Sorry, I don't want to join your guild %name :(.", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        accept = false;
    }

    Guild* guild = sGuildMgr.GetGuildById(guildId);

    if(guild && guild->GetMemberSize() > 1000)
    {
        ai->TellError(requester, "This guild has over 1000 members. To stop it from reaching the 1064 member limit I refuse to join it.");

        if (sServerFacade.GetDistance2d(bot, inviter) < sPlayerbotAIConfig.spellDistance * 1.5 && inviter->GetPlayerbotAI())
            bot->Say(BOT_TEXT2("%name, your guild has over 1000 members. To stop it from reaching the 1064 member limit I refuse to join it.", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        accept = false;
    }

    if (accept && sPlayerbotAIConfig.inviteChat && sServerFacade.GetDistance2d(bot, inviter) < sPlayerbotAIConfig.spellDistance * 1.5 && inviter->GetPlayerbotAI() && (sRandomPlayerbotMgr.IsFreeBot(bot) || !ai->HasActivePlayerMaster()))
    {
        if (urand(0, 3))
            bot->Say(BOT_TEXT2("Sounds good %name sign me up!", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        else
            bot->Say(BOT_TEXT2("I would love to join!", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    }

    WorldPacket packet;
    if (accept)
    {
        bot->GetSession()->HandleGuildAcceptOpcode(packet);

        TalentSpec::SetPublicNote(bot);

        sPlayerbotAIConfig.logEvent(ai, "GuildAcceptAction", guild->GetName(), std::to_string(guild->GetMemberSize()));
    }
    else
    {
        bot->GetSession()->HandleGuildDeclineOpcode(packet);
    }
    return true;
}