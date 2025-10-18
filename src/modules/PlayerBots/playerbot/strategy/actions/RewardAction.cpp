
#include "playerbot/playerbot.h"
#include "RewardAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

bool RewardAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string link = event.getParam();

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
    {
        return false;
    }

    uint32 itemId = *itemIds.begin();
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* npc = ai->GetUnit(*i);
        if (npc && Reward(requester, itemId, npc))
        {
            return true;
        }
    }

    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go && Reward(requester, itemId, go))
        {
            return true;
        }
    }

    if (requester && Reward(requester, itemId, requester))
    {
       return true;
    }

    if(!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayer(requester, BOT_TEXT("quest_error_talk"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return false;
}

bool RewardAction::Reward(Player* requester, uint32 itemId, Object* questGiver)
{
    QuestMenu& questMenu = bot->GetPlayerMenu()->GetQuestMenu();
    for (uint32 iI = 0; iI < questMenu.MenuItemCount(); ++iI)
    {
        QuestMenuItem const& qItem = questMenu.GetItem(iI);

        uint32 questID = qItem.m_qId;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questID);
        QuestStatus status = bot->GetQuestStatus(questID);

        // if quest is complete, turn it in
        if (status == QUEST_STATUS_COMPLETE &&
            !bot->GetQuestRewardStatus(questID) &&
            pQuest->GetRewChoiceItemsCount() > 1 &&
            bot->CanRewardQuest(pQuest, false))
        {
            for (uint8 rewardIdx=0; rewardIdx < pQuest->GetRewChoiceItemsCount(); ++rewardIdx)
            {
                ItemPrototype const * const pRewardItem = sObjectMgr.GetItemPrototype(pQuest->RewChoiceItemId[rewardIdx]);
                if (itemId == pRewardItem->ItemId)
                {
                    bot->RewardQuest(pQuest, rewardIdx, questGiver, false);

                    std::map<std::string, std::string> args;
                    args["%item"] = chat->formatItem(pRewardItem);
                    ai->TellPlayer(requester, BOT_TEXT2("quest_choose_reward", args));

                    return true;
                }
            }
        }
    }

    return false;
}
