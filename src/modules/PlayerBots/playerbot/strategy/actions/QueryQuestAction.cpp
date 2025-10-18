
#include "playerbot/playerbot.h"
#include "QueryQuestAction.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

bool QueryQuestAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Player *bot = ai->GetBot();
    WorldPosition botPos(bot);
    std::string text = event.getParam();

    PlayerbotChatHandler ch(bot);
    uint32 questId = ch.extractQuestId(text);
    if (!questId)
    {
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = bot->GetQuestSlotQuestId(slot);

            Quest const* quest = sObjectMgr.GetQuestTemplate(logQuest);
            if (!quest)
                continue;

            if (text.find(quest->GetTitle()) != std::string::npos)
            {
                questId = quest->GetQuestId();
                break;
            }
        }
    }
    
    if (!questId)
        return false;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        if (questId != bot->GetQuestSlotQuestId(slot))
            continue;

        std::ostringstream out;
        out << "--- " << chat->formatQuest(sObjectMgr.GetQuestTemplate(questId)) << " ";
        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        {
            out << "|c0000FF00completed|r ---";
            ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
        else
        {
            out << "|c00FF0000not completed|r ---";
            ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            TellObjectives(requester, questId);
        }      

        return true;
    }

    return false;
}

void QueryQuestAction::TellObjectives(Player* requester, uint32 questId)
{
    Quest const* questTemplate = sObjectMgr.GetQuestTemplate(questId);
    QuestStatusData questStatus = bot->getQuestStatusMap()[questId];

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (!questTemplate->ObjectiveText[i].empty())
            ai->TellPlayer(requester, questTemplate->ObjectiveText[i], PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        if (questTemplate->ReqItemId[i])
        {
            int required = questTemplate->ReqItemCount[i];
            int available = questStatus.m_itemcount[i];
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(questTemplate->ReqItemId[i]);
            TellObjective(requester, chat->formatItem(proto), available, required);
        }

        if (questTemplate->ReqCreatureOrGOId[i])
        {
            int required = questTemplate->ReqCreatureOrGOCount[i];
            int available = questStatus.m_creatureOrGOcount[i];

            if (questTemplate->ReqCreatureOrGOId[i] < 0)
            {
                GameObjectInfo const* info = sObjectMgr.GetGameObjectInfo(-questTemplate->ReqCreatureOrGOId[i]);
                if (info)
                    TellObjective(requester, info->name, available, required);
            }
            else
            {
                CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(questTemplate->ReqCreatureOrGOId[i]);
                if (info)
                    TellObjective(requester, info->Name, available, required);
            }
        }
    }
}

void QueryQuestAction::TellObjective(Player* requester, const std::string& name, int available, int required)
{
    ai->TellPlayer(requester, chat->formatQuestObjective(name, available, required), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
}