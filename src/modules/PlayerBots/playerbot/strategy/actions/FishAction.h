#pragma once
#include "CastCustomSpellAction.h"
#include "UseItemAction.h"


namespace ai
{
    class MoveToFishAction : public MovementAction, public Qualified
    {
    public:
        MoveToFishAction(PlayerbotAI* ai) : MovementAction(ai, "move to fish"), Qualified() {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "move to fish"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot move to a fishing location.\n"
                   "It identifies suitable spots for fishing and navigates there.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };

    class FishAction : public CastCustomSpellAction
    {
    public:
        FishAction(PlayerbotAI* ai) : CastCustomSpellAction(ai, "fish") {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "fish"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot perform fishing.\n"
                   "It casts the fishing spell to catch fish at the current location.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };

    class UseFishingBobberAction : public UseAction
    {
    public:
        UseFishingBobberAction(PlayerbotAI* ai) : UseAction(ai, "use fishing bobber") {}

        bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "use fishing bobber"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot use a fishing bobber.\n"
                   "It interacts with the bobber to complete the fishing process.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
