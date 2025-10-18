#pragma once
#include "GenericActions.h"

namespace ai
{
    enum QuestListFilter {
        QUEST_LIST_FILTER_SUMMARY = 0,
        QUEST_LIST_FILTER_COMPLETED = 1,
        QUEST_LIST_FILTER_INCOMPLETED = 2,
        QUEST_LIST_FILTER_ALL = QUEST_LIST_FILTER_COMPLETED | QUEST_LIST_FILTER_INCOMPLETED
    };

    enum QuestTravelDetail {
        QUEST_TRAVEL_DETAIL_NONE = 0,
        QUEST_TRAVEL_DETAIL_SUMMARY = 1,
        QUEST_TRAVEL_DETAIL_FULL = 2
    };

    class ListQuestsAction : public ChatCommandAction
    {
    public:
        ListQuestsAction(PlayerbotAI* ai) : ChatCommandAction(ai, "quests") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void ListQuests(Player* requester, QuestListFilter filter, QuestTravelDetail travelDetail = QUEST_TRAVEL_DETAIL_NONE, std::set<uint32> onlyQuestIds = {});
        int ListQuests(Player* requester, bool completed, bool silent, QuestTravelDetail travelDetail, std::set<uint32> onlyQuestIds = {});
    };
}