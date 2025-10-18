#pragma once
#include "GenericSpellActions.h"
#include "GenericActions.h"

namespace ai
{
    class PullRequestAction : public ChatCommandAction
    {
    public:
        PullRequestAction(PlayerbotAI* ai, std::string name) : ChatCommandAction(ai, name) {}

    protected:
        virtual bool Execute(Event& event) override;
        virtual Unit* GetTarget(Event& event) = 0;
    };

    class PullMyTargetAction : public PullRequestAction
    {
    public:
        PullMyTargetAction(PlayerbotAI* ai) : PullRequestAction(ai, "pull my target") {}
    
    private:
        Unit* GetTarget(Event& event);
    };

    class PullRTITargetAction : public PullRequestAction
    {
    public:
        PullRTITargetAction(PlayerbotAI* ai) : PullRequestAction(ai, "pull rti target") {}

    private:
        Unit* GetTarget(Event& event);
    };

    class PullStartAction : public Action
    {
    public:
        PullStartAction(PlayerbotAI* ai, std::string name = "pull start") : Action(ai, name) {}
        bool Execute(Event& event) override;
    };

    class PullAction : public CastSpellAction
    {
    public:
        PullAction(PlayerbotAI* ai, std::string name = "pull action");
        bool Execute(Event& event) override;
        bool isPossible() override;
    private:
        void InitPullAction();
        std::string GetTargetName() override { return "pull target"; }
        std::string GetReachActionName() override { return "reach pull"; }
    };

    class PullEndAction : public Action
    {
    public:
        PullEndAction(PlayerbotAI* ai, std::string name = "pull end") : Action(ai, name) {}
        bool Execute(Event& event) override;
    };
}
