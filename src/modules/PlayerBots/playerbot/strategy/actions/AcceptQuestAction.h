#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class AcceptAllQuestsAction : public QuestAction 
    {
    public:
        AcceptAllQuestsAction(PlayerbotAI* ai, std::string name = "accept all quests") : QuestAction(ai, name) {}
        virtual bool isUsefulWhenStunned() override { return true; }

    protected:
        virtual bool ProcessQuest(Player* requester, Quest const* quest, WorldObject* questGiver) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "accept all quests"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action wil accept all quests from the npc selected by the master\n"
                "It will use all nearby quest givers if none is selected.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs" ,  "nearest game objects no los" }; }
#endif 
    };

    class AcceptQuestAction : public AcceptAllQuestsAction 
    {
    public:
        AcceptQuestAction(PlayerbotAI* ai) : AcceptAllQuestsAction(ai, "accept quest") {}
        virtual bool Execute(Event& event);
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "accept quests"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action wil accept a specified quest (link or *) from any nearby quest giver.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { }; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs" ,  "nearest game objects no los" }; }
#endif 
    };

    class AcceptQuestShareAction : public Action 
    {
    public:
        AcceptQuestShareAction(PlayerbotAI* ai) : Action(ai, "accept quest share") {}
        virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "accept quest share"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will make a bot click accept when shared a quest.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { }; }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class ConfirmQuestAction : public Action {
    public:
        ConfirmQuestAction(PlayerbotAI* ai) : Action(ai, "confirm quest") {}
        virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "confirm quest"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will make a bot click accept asked to start a quest.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { }; }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class QuestDetailsAction : public Action {
    public:
        QuestDetailsAction(PlayerbotAI* ai) : Action(ai, "quest details") {}
        virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "quest details"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will make a bot click accept in the quest details page.";
        }
        virtual std::vector<std::string> GetUsedActions() { return { }; }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };
}
