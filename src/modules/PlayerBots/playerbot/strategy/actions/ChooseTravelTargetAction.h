#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "GenericActions.h"
#include "playerbot/TravelMgr.h"

namespace ai
{
    const std::vector<uint32> travelPartitions = { 100, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 10000, 50000, 100000, 500000 };

    class ChooseTravelTargetAction : public Action {
    public:
        ChooseTravelTargetAction(PlayerbotAI* ai, std::string name = "choose travel target") : Action(ai, name) {}

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    protected:
        void setNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);
        void ReportTravelTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);

        bool SetBestTarget(Player* requester, TravelTarget* target, PartitionedTravelList& travelPartitions, bool onlyActive = true);
    public:
        static DestinationList FindDestination(PlayerTravelInfo info, std::string name, bool zones = true, bool npcs = true, bool quests = true, bool mobs = true, bool bosses = true);
    private:
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "choose travel target"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action is used to select a target for the bots to travel to.\n"
                "Bots will travel to specific places for specific reasons.\n"
                "For example bots will travel to a vendor if they need to sell stuff\n"
                "The current destination or those of group members are also options.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "travel target", "group or", "should sell","can sell","can ah sell","should repair","can repair","following party","near leader","should get money","can fight equal","can fight elite","can fight boss","can free move to","rpg target","attack target",}; }
#endif 
    };

    class ChooseGroupTravelTargetAction : public ChooseTravelTargetAction {
    public:
        ChooseGroupTravelTargetAction(PlayerbotAI* ai, std::string name = "choose group travel target") : ChooseTravelTargetAction(ai, name) {}

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };

    class RefreshTravelTargetAction : public ChooseTravelTargetAction {
    public:
        RefreshTravelTargetAction(PlayerbotAI* ai, std::string name = "refresh travel target") : ChooseTravelTargetAction(ai, name) {}

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };

    class ResetTargetAction : public ChooseTravelTargetAction {
    public:
        ResetTargetAction(PlayerbotAI* ai, std::string name = "reset travel target") : ChooseTravelTargetAction(ai, name) {}

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };    

    class RequestTravelTargetAction : public Action, public Qualified {
    public:
        RequestTravelTargetAction(PlayerbotAI* ai, std::string name = "request travel target") : Action(ai, name), Qualified() {}
    private:
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isAllowed() const;
    };

    class RequestNamedTravelTargetAction : public RequestTravelTargetAction {
    public:
        RequestNamedTravelTargetAction(PlayerbotAI* ai, std::string name = "request named travel target") : RequestTravelTargetAction(ai, name) {}
    private:
        virtual bool Execute(Event& event) override;
        virtual bool isAllowed() const override;
    };

    class RequestQuestTravelTargetAction : public RequestTravelTargetAction {
    public:
        RequestQuestTravelTargetAction(PlayerbotAI* ai, std::string name = "request quest travel target") : RequestTravelTargetAction(ai, name) {}
    private:
        virtual bool Execute(Event& event) override;
        virtual bool isAllowed() const override;
    };

    class FocusTravelTargetAction : public ChatCommandAction {
    public:
        FocusTravelTargetAction(PlayerbotAI* ai, std::string name = "focus travel target") : ChatCommandAction(ai, name) {}

        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "focus travel target"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command specifies quests the bot should focus on.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
