#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class AcceptDuelAction : public Action
    {
    public:
        AcceptDuelAction(PlayerbotAI* ai) : Action(ai, "accept duel")
        {}

        virtual bool Execute(Event& event)
        {
            WorldPacket p(event.getPacket());

            ObjectGuid flagGuid;
            p >> flagGuid;
            ObjectGuid playerGuid;
            p >> playerGuid;

            // do not auto duel with low hp or below certain level
            if (bot->GetLevel() < sPlayerbotAIConfig.botAcceptDuelMinimumLevel
                || ((!ai->HasRealPlayerMaster() || (ai->GetMaster() && ai->GetMaster()->GetObjectGuid() != playerGuid)) && AI_VALUE2(uint8, "health", "self target") < 90))
            {
                WorldPacket packet(CMSG_DUEL_CANCELLED, 8);
                packet << flagGuid;
                bot->GetSession()->HandleDuelCancelledOpcode(packet);
            }

            WorldPacket packet(CMSG_DUEL_ACCEPTED, 8);
            packet << flagGuid;
            bot->GetSession()->HandleDuelAcceptedOpcode(packet);

            ai->ResetStrategies();
            return true;
        }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "accept duel"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action accepts or declines duel invitations based on conditions.\n"
                "The bot will decline if below minimum level or low health (unless from master).\n"
                "After accepting, combat strategies are reset for the duel.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };

}
