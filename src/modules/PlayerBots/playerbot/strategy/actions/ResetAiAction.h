#pragma once
#include "GenericActions.h"

namespace ai
{
    class ResetAiAction : public ChatCommandAction
    {
    public:
        ResetAiAction(PlayerbotAI* ai, bool fullReset = true, std::string name = "reset ai") : ChatCommandAction(ai, name), fullReset(fullReset) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "reset ai"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "Reset the bot to it's initial state.\n"
                "Saved settings and values will be cleared.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "reset strats" , "reset values" }; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    protected:
        virtual void ResetStrategies();
        virtual void ResetValues();
        bool fullReset;
    };

    class SaveAiAction : public ChatCommandAction, public Qualified
    {
    public:
       SaveAiAction(PlayerbotAI* ai, std::string name = "save ai") : ChatCommandAction(ai, name), Qualified() {}
       virtual bool Execute(Event& event) override;
       bool isUseful() override { return true; };
       virtual bool isUsefulWhenStunned() override { return true; }
    };

    class LoadAiAction : public ChatCommandAction, public Qualified
    {
    public:
       LoadAiAction(PlayerbotAI* ai, std::string name = "load ai") : ChatCommandAction(ai, name), Qualified() {}
       virtual bool Execute(Event& event) override;
       bool isUseful() override { return true; };
       virtual bool isUsefulWhenStunned() override { return true; }
    };

    class ListAiAction : public ChatCommandAction, public Qualified
    {
    public:
       ListAiAction(PlayerbotAI* ai, std::string name = "list ai") : ChatCommandAction(ai, name), Qualified() {}
       virtual bool Execute(Event& event) override;
       bool isUseful() override { return true; };
       virtual bool isUsefulWhenStunned() override { return true; }
    };

    class ResetStratsAction : public ResetAiAction
    {
    public:
        ResetStratsAction(PlayerbotAI* ai, std::string name = "reset strats", bool fullReset = true) : ResetAiAction(ai, fullReset, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "reset strats"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "Reset the strategies of the bot to the standard values.\n"
                "Saved strategies will be cleared.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {"reset ai"}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };

    class ResetValuesAction : public ResetAiAction
    {
    public:
        ResetValuesAction(PlayerbotAI* ai, std::string name = "reset values", bool fullReset = true) : ResetAiAction(ai, fullReset, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "reset values"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "Reset the settings of the bot to the default values.\n"
                "Saved values will be cleared.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "reset ai" }; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };

    class ResetAction : public Action
    {
    public:
        ResetAction(PlayerbotAI* ai, std::string name = "reset") : Action(ai, name) {}
        virtual bool Execute(Event& event) override { ai->Reset(true); return true; };
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "reset"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "Reset internal buffers to clear current behavior.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { "reset ai" }; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
