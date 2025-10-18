#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class ChatCommandHandlerStrategy : public PassTroughStrategy
    {
    public:
        ChatCommandHandlerStrategy(PlayerbotAI* ai);
        std::string getName() override { return "chat"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "chat"; } //Must equal internal name
        virtual std::string GetHelpDescription() 
        {
            return "This strategy will make bots respond to various chat commands.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { }; }
#endif

    private:
        void InitReactionTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
