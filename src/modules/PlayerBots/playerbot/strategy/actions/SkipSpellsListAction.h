#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"

namespace ai
{
    class SkipSpellsListAction : public ChatCommandAction
    {
    public:
        SkipSpellsListAction(PlayerbotAI* ai) : ChatCommandAction(ai, "skip spells list") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "skip spells list"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This chat command gives control over the list of spells bots aren't allowed to cast.\n"
                "Examples:\n"
                "ss ? : List the ignored spells.\n"
                "ss [spell name] : Never cast this spell.\n"
                "ss -[spell name] : Remove this spell from the ignored spell list.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "skip spells list" }; }
#endif 

    private:
        std::vector<std::string> ParseSpells(const std::string& text);
    };
}
