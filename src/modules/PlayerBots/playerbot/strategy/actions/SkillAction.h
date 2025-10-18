#pragma once
#include "GenericActions.h"

namespace ai
{
    class SkillAction : public ChatCommandAction
    {
    public:
        SkillAction(PlayerbotAI* ai) : ChatCommandAction(ai, "skill") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "skill"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This chat command gives information about a bot\'s skills.\n"
                "Examples:\n"
                "skill : List all skills and their current level.\n"
                "skill [name] : List the skill level of a current skill.\n"
                "skill unlearn [name] : Unlearns a primary profession.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
