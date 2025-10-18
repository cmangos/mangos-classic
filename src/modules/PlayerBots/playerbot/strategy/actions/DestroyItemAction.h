#pragma once
#include "GenericActions.h"

namespace ai
{
    class DestroyItemAction : public ChatCommandAction
    {
    public:
        DestroyItemAction(PlayerbotAI* ai, std::string name = "destroy") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "destroy"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make the bot destroy a certain item.\n"
                "Usage: destroy [itemlink]\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "force item usage" }; }
#endif

    protected:
        void DestroyItem(FindItemVisitor* visitor, Player* requester);
    };

    class SmartDestroyItemAction : public DestroyItemAction 
    {
    public:
        SmartDestroyItemAction(PlayerbotAI* ai) : DestroyItemAction(ai, "smart destroy item") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() { return !ai->HasActivePlayerMaster(); }
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "smart destroy item"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make the bot destroy a certain item.\n"
                "Usage: destroy [itemlink]\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {"destroy"}; }
        virtual std::vector<std::string> GetUsedValues() { return { "bag space", "force item usage" }; }
#endif
    };

    class DestroyAllGrayItemsAction : public DestroyItemAction
    {
    public:
        DestroyAllGrayItemsAction(PlayerbotAI* ai) : DestroyItemAction(ai, "destroy all gray") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "destroy all gray"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make the bot destroy all gray items.\n"
                "Usage: destroy all gray\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif
    };
}
