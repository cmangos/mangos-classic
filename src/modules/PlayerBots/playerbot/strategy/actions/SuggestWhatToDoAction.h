#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class SuggestWhatToDoAction : public Action
    {
    public:
        SuggestWhatToDoAction(PlayerbotAI* ai, std::string name = "suggest what to do");
        virtual bool Execute(Event& event) override;
        virtual bool isUseful();
        virtual bool isUsefulWhenStunned() override { return true; }

    protected:
        typedef void (SuggestWhatToDoAction::*Suggestion) ();
        std::vector<Suggestion> suggestions;
        void instance();
        void specificQuest();
        void grindMaterials();
        void grindReputation();
        void something();
        void somethingToxic();
        void toxicLinks();
        void thunderfury();

        std::vector<uint32> GetIncompletedQuests();

    private:
        static std::map<std::string, int> instances;
        static std::map<std::string, int> factions;
        int32 _locale;
    };

    class SuggestTradeAction : public SuggestWhatToDoAction
    {
    public:
        SuggestTradeAction(PlayerbotAI* ai) : SuggestWhatToDoAction(ai) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
