#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class AcceptInvitationAction : public Action 
    {
    public:
        AcceptInvitationAction(PlayerbotAI* ai) : Action(ai, "accept invitation") {}

        virtual bool Execute(Event& event)
        {
            Group* grp = bot->GetGroupInvite();
            if (!grp)
                return false;

            Player* inviter = sObjectMgr.GetPlayer(grp->GetLeaderGuid());
            if (!inviter)
                return false;

			if (!ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE, false, inviter))
            {
                WorldPacket data(SMSG_GROUP_DECLINE, 10);
                data << bot->GetName();
                sServerFacade.SendPacket(inviter, data);
                bot->UninviteFromGroup();
                return false;
            }
            
            if (bot->isAFK())
                bot->ToggleAFK();

            WorldPacket p;
            uint32 roles_mask = 0;
            p << roles_mask;
            bot->GetSession()->HandleGroupAcceptOpcode(p);

            if (!bot->GetGroup() || !bot->GetGroup()->IsMember(inviter->GetObjectGuid()))
                return false;

            if (sRandomPlayerbotMgr.IsFreeBot(bot))
            {
                ai->SetMaster(inviter);
                ai->ChangeStrategy("+follow", BotState::BOT_STATE_NON_COMBAT);
            }

            ai->ResetStrategies();
            
            ai->ChangeStrategy("-lfg,-bg", BotState::BOT_STATE_NON_COMBAT);
            ai->Reset();

            sPlayerbotAIConfig.logEvent(ai, "AcceptInvitationAction", grp->GetLeaderName(), std::to_string(grp->GetMembersCount()));

            Player* master = inviter;

            if (master->GetPlayerbotAI()) //Copy formation from bot master.
            {
                if (sPlayerbotAIConfig.inviteChat && (sRandomPlayerbotMgr.IsFreeBot(bot) || !ai->HasActivePlayerMaster()))
                {
                    std::map<std::string, std::string> placeholders;
                    placeholders["%name"] = master->GetName();
                    std::string reply;
                    if (urand(0, 3))
                        reply = BOT_TEXT2("Send me an invite %name!", placeholders);
                    else
                        reply = BOT_TEXT2("Sure I will join you.", placeholders);

                    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());

                    if (guild && master->IsInGuild(bot))
                        guild->BroadcastToGuild(bot->GetSession(), reply, LANG_UNIVERSAL);
                    else if (sServerFacade.GetDistance2d(bot, master) < sPlayerbotAIConfig.spellDistance * 1.5)
                        bot->Say(reply, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                }

                Formation* masterFormation = MAI_VALUE(Formation*, "formation");
                FormationValue* value = (FormationValue*)context->GetValue<Formation*>("formation");
                value->Load(masterFormation->getName());
            }

            ai->TellPlayer(inviter, BOT_TEXT("hello"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            ai->DoSpecificAction("reset raids", event, true);
            ai->DoSpecificAction("update gear", event, true);

            return true;
        }

        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "accept invitation"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot accept group invitations.\n"
                   "It will automatically handle AFK status and update strategies.\n"
                   "For free bots, the inviter becomes the bot's master.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {"reset raids", "update gear"}; }
        virtual std::vector<std::string> GetUsedValues() { return {"formation"}; }
#endif 
    };

}
