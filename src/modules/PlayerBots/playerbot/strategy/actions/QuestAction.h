#pragma once

#include "playerbot/strategy/Action.h"
#include "Quests/QuestDef.h"

namespace ai
{
    class QuestAction : public Action 
    {
    public:
        QuestAction(PlayerbotAI* ai, std::string name = "quest") : Action(ai, name) {}

    public:
        virtual bool Execute(Event& event);

    protected:
        virtual bool ProcessQuest(Player* requester, Quest const* quest, WorldObject* questGiver) = 0;
        bool CompleteQuest(Player* player, uint32 entry);
        bool AcceptQuest(Player* requester, Quest const* quest, uint64 questGiver);
        bool ProcessQuests(ObjectGuid questGiver);
        bool ProcessQuests(WorldObject* questGiver);
    };
    
    class QuestUpdateAddKillAction : public Action
    {
    public:
        QuestUpdateAddKillAction(PlayerbotAI* ai) : Action(ai, "quest update add kill") {}

    public:
        virtual bool Execute(Event& event);
    };

    class QuestUpdateAddItemAction : public Action
    {
    public:
        QuestUpdateAddItemAction(PlayerbotAI* ai) : Action(ai, "quest update add item") {}

    public:
        virtual bool Execute(Event& event);
    };

    class QuestUpdateFailedAction : public Action
    {
    public:
        QuestUpdateFailedAction(PlayerbotAI* ai) : Action(ai, "quest update failed") {}

    public:
        virtual bool Execute(Event& event);
    };

    class QuestUpdateFailedTimerAction : public Action
    {
    public:
        QuestUpdateFailedTimerAction(PlayerbotAI* ai) : Action(ai, "quest update failed timer") {}

    public:
        virtual bool Execute(Event& event);
    };

    class QuestUpdateCompleteAction : public Action
    {
    public:
        QuestUpdateCompleteAction(PlayerbotAI* ai) : Action(ai, "quest update complete") {}

    public:
        virtual bool Execute(Event& event);
    };
}
