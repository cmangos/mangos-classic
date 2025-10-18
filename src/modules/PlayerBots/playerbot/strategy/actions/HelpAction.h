#pragma once
#include "GenericActions.h"

namespace ai
{
    class HelpAction : public ChatCommandAction
    {
    public:
        HelpAction(PlayerbotAI* ai);
        virtual ~HelpAction();
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void TellChatCommands(Player* requester);
        void TellStrategies(Player* requester);
        std::string CombineSupported(std::set<std::string> commands);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "help"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This chat command will make the bot give you help on various topics.\n"
                "Each help topic will provide various chat-links which can be linked in chat.\n"
                "Links can be copied to chat with shift-click and when sent to a bot will provide feedback.\n"
                "It is possible to search for help topics by adding part of the topic name after the command.\n"
                "Example commands: 'help' and 'help rpg'.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    private:
        NamedObjectContext<Action>* chatContext;
    };

}
