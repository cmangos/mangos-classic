
#include "playerbot/playerbot.h"
#include "ListQuestsActions.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

bool ListQuestsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (event.getParam() == "completed" || event.getParam() == "co")
    {
        ListQuests(requester, QUEST_LIST_FILTER_COMPLETED);
    }
    else if (event.getParam() == "incompleted" || event.getParam() == "in")
    {
        ListQuests(requester, QUEST_LIST_FILTER_INCOMPLETED);
    }
    else if (event.getParam() == "all")
    {
        ListQuests(requester, QUEST_LIST_FILTER_ALL);
    }
    else if (event.getParam().find("travel") == 0)
    {
        std::set<uint32> questIds;
        if (event.getParam().size() > 8)
            questIds = ChatHelper::ExtractAllQuestIds(event.getParam());

        ListQuests(requester, QUEST_LIST_FILTER_ALL, QUEST_TRAVEL_DETAIL_SUMMARY, questIds);
    }
    else
    {
        ListQuests(requester, QUEST_LIST_FILTER_SUMMARY);
    }
    return true;
}

void ListQuestsAction::ListQuests(Player* requester, QuestListFilter filter, QuestTravelDetail travelDetail, std::set<uint32> onlyQuestIds)
{
    bool showIncompleted = filter & QUEST_LIST_FILTER_INCOMPLETED;
    bool showCompleted = filter & QUEST_LIST_FILTER_COMPLETED;

    if (showIncompleted)
        ai->TellPlayer(requester, "--- Incompleted quests ---");
    int incompleteCount = ListQuests(requester, false, !showIncompleted, travelDetail, onlyQuestIds);

    if (showCompleted)
        ai->TellPlayer(requester, "--- Completed quests ---");
    int completeCount = ListQuests(requester, true, !showCompleted, travelDetail, onlyQuestIds);

    ai->TellPlayer(requester, "--- Summary ---");
    std::ostringstream out;
    out << "Total: " << (completeCount + incompleteCount) << " / 25 (incompleted: " << incompleteCount << ", completed: " << completeCount << ")";
    ai->TellPlayer(requester, out);
}

int ListQuestsAction::ListQuests(Player* requester, bool completed, bool silent, QuestTravelDetail travelDetail, std::set<uint32> onlyQuestIds)
{
    TravelTarget* target;
    WorldPosition botPos(bot);
    PlayerTravelInfo info(bot);
    uint32 PurposeFlag = (uint32)TravelDestinationPurpose::QuestGiver | (uint32)TravelDestinationPurpose::QuestTaker | (uint32)TravelDestinationPurpose::QuestObjective1 | (uint32)TravelDestinationPurpose::QuestObjective2 |
        (uint32)TravelDestinationPurpose::QuestObjective3 | (uint32)TravelDestinationPurpose::QuestObjective4;

    std::string red = "|cFFFF0000";
    std::string yellow = "|cFFFFFF00";
    std::string green = "|cFF00FF00";

    std::unordered_map<TravelDestinationPurpose, std::string> typeName = 
    { {TravelDestinationPurpose::QuestGiver, "G"}, {TravelDestinationPurpose::QuestTaker,"T"}, {TravelDestinationPurpose::QuestObjective1,"1"}, {TravelDestinationPurpose::QuestObjective2,"2"}, {TravelDestinationPurpose::QuestObjective3,"3"}, {TravelDestinationPurpose::QuestObjective4,"4"} };
    
    if (travelDetail != QUEST_TRAVEL_DETAIL_NONE)
        target = AI_VALUE(TravelTarget*,"travel target");
   
    std::list<uint32> questIds;

    for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "group members"))
    {
        Player* player = sObjectMgr.GetPlayer(guid);

        if (!player)
            continue;

        if (travelDetail == QUEST_TRAVEL_DETAIL_NONE && player != bot)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (!player->GetPlayerbotAI())
            continue;

        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = player->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            if (onlyQuestIds.size() && onlyQuestIds.find(questId) == onlyQuestIds.end())
                continue;

            bool isCompletedQuest = player->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE;
            if (completed != isCompletedQuest)
                continue;

            if (travelDetail != QUEST_TRAVEL_DETAIL_NONE)
            {
                focusQuestTravelList focusList = AI_VALUE(focusQuestTravelList, "focus travel target");
                if (!focusList.empty() && focusList.find(questId) == focusList.end())
                    continue;
            }

            if (std::find(questIds.begin(), questIds.end(), questId) == questIds.end())
                questIds.push_back(questId);
        }

        if (silent)
            return questIds.size();
    }

    for (auto& questId : questIds)
    {
        std::ostringstream out;

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        out << chat->formatQuest(pQuest);

        if (travelDetail == QUEST_TRAVEL_DETAIL_NONE)
        {
            ai->TellPlayer(requester, out);
            continue;
        }

        if (target->GetDestination() && (typeid(*target->GetDestination()) == typeid(QuestRelationTravelDestination) || typeid(*target->GetDestination()) == typeid(QuestObjectiveTravelDestination)))
        {
            QuestTravelDestination* QuestDestination = (QuestTravelDestination*)target->GetDestination();

            if (QuestDestination->GetQuestId() == questId)
                out << "[Active]" << target->GetPosition()->distance(botPos);
        }

        DestinationList destinations = sTravelMgr.GetDestinations(info, PurposeFlag, { (int32)questId }, false, 1000000.0f);

        std::sort(destinations.begin(), destinations.end(), [](TravelDestination* i, TravelDestination* j) {return (uint32)static_cast<EntryTravelDestination*>(i)->GetPurpose() < (uint32)static_cast<EntryTravelDestination*>(j)->GetPurpose(); });

        uint32 minDistance = 999999;
        uint32 inpossiblePoints = 0, possiblePoints = 0, activePoints = 0;
        out << " dests: [";

        for (auto& destination : destinations)
        {
            EntryTravelDestination* entryDestination = static_cast<EntryTravelDestination*>(destination);
            uint32 points = destination->GetPoints().size();
            MANGOS_ASSERT(points >= 0 && points < 10000);
            minDistance = std::min(minDistance, (uint32)destination->DistanceTo(bot));

            if (destination->IsActive(bot, info))
            {
                out << green;
                activePoints += points;
            }
            else if (destination->IsPossible(info))
            {
                out << yellow;
                possiblePoints += points;
            }
            else
            {
                inpossiblePoints += points;
                out << red;
            }
            out << typeName[entryDestination->GetPurpose()] << "|r";
        }

        if (destinations.empty())
        {
            out << " none]";
        }
        else
        {
            out << "] point: [";
            out << green << activePoints << "|r " << yellow << possiblePoints << "|r " << red << inpossiblePoints << "|r";
            out << "] dist: (" << minDistance << "y)";
        }

        ai->TellPlayer(requester, out);
    }    

    return questIds.size();
}
