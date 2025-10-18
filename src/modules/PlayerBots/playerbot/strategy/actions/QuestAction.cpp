
#include "playerbot/playerbot.h"
#include "QuestAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool QuestAction::Execute(Event& event)
{
    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();

    if (!guid)
    {
        if (!master)
        {
            guid = bot->GetSelectionGuid();
        }
        else
        {
            guid = master->GetSelectionGuid();
        }
    }

    if (guid)
    {
        return ProcessQuests(guid);
    }

    bool result = false;
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(unit);
    }
    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go && bot->GetDistance(go) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(go);
    }

    return result;
}

bool QuestAction::CompleteQuest(Player* player, uint32 entry)
{
    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        return false;
    }

    // Add quest items for quests that require items
    for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
        if (!id || !count)
        {
            continue;
        }

        uint32 curItemCount = player->GetItemCount(id, true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, id, true);
            player->SendNewItem(item, count - curItemCount, true, false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

        if (uint32 spell_id = pQuest->ReqSpell[i])
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->CastedCreatureOrGO(creature, ObjectGuid(), spell_id);
            }
        }
        else if (creature > 0)
        {
            if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                for (uint16 z = 0; z < creaturecount; ++z)
                {
                    player->KilledMonster(cInfo, nullptr);
                }
        }
        else if (creature < 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
            }
        }
    }

    // If the quest requires reputation to complete
    if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
        {
#ifdef MANGOSBOT_ONE
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry<FactionEntry>(repFaction))
#else
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
#endif
            {
                player->GetReputationMgr().SetReputation(factionEntry, repValue);
            }
        }
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0)
    {
        player->ModifyMoney(-ReqOrRewMoney);
    }

    player->CompleteQuest(entry);
    return true;
}

bool QuestAction::ProcessQuests(ObjectGuid questGiver)
{
    GameObject *gameObject = ai->GetGameObject(questGiver);
    if (gameObject && gameObject->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        return ProcessQuests(gameObject);
    }

    Creature* creature = ai->GetCreature(questGiver);
    if (creature)
    {
        return ProcessQuests(creature);
    }

    return false;
}

bool QuestAction::ProcessQuests(WorldObject* questGiver)
{
    ObjectGuid guid = questGiver->GetObjectGuid();

    if (sServerFacade.GetDistance2d(bot, questGiver) > INTERACTION_DISTANCE && !sPlayerbotAIConfig.syncQuestWithPlayer)
    {
        Player* master = ai->GetMaster();
        if (!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(master, BOT_TEXT("quest_error_talk"));

        return false;
    }

    if (!sServerFacade.IsInFront(bot, questGiver, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
    {
        sServerFacade.SetFacingTo(bot, questGiver);
    }

    bot->SetSelectionGuid(guid);
    bot->PrepareQuestMenu(guid);
    QuestMenu& questMenu = bot->GetPlayerMenu()->GetQuestMenu();

    bool hasAccept = false;
    for (uint32 i = 0; i < questMenu.MenuItemCount(); ++i)
    {
        QuestMenuItem const& menuItem = questMenu.GetItem(i);
        uint32 questID = menuItem.m_qId;
        Quest const* quest = sObjectMgr.GetQuestTemplate(questID);
        if (!quest)
        {
            continue;
        }

        hasAccept |= ProcessQuest(GetMaster(), quest, questGiver);
    }

    return hasAccept;
}

bool QuestAction::AcceptQuest(Player* requester, Quest const* quest, uint64 questGiver)
{
    bool success = false;
    const uint32 questId = quest->GetQuestId();

    std::string outputMessage;
    std::map<std::string, std::string> args;
    args["%quest"] = chat->formatQuest(quest);
    
    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
    {
        outputMessage = BOT_TEXT2("quest_error_completed", args);
    }
    else if (! bot->CanTakeQuest(quest, false))
    {
        if (! bot->SatisfyQuestStatus(quest, false))
        {
            outputMessage = BOT_TEXT2("quest_error_have_quest", args);
        }
        else if (!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        {
            outputMessage = BOT_TEXT2("quest_error_cant_take", args);
        }
    }
    else if (! bot->SatisfyQuestLog(false))
    {
        outputMessage = BOT_TEXT2("quest_error_log_full", args);
    }
    else if (! bot->CanAddQuest(quest, false))
    {
        outputMessage = BOT_TEXT2("quest_error_bag_full", args);
    }
    else
    {
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        uint32 unk1 = 0;
        p << questGiver << questId << unk1;
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_NONE && sPlayerbotAIConfig.syncQuestWithPlayer)
        {
            Object* pObject = bot->GetObjectByTypeMask((ObjectGuid)questGiver, TYPEMASK_CREATURE_GAMEOBJECT_PLAYER_OR_ITEM);
            bot->AddQuest(quest, pObject);
        }

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE && bot->GetQuestStatus(questId) != QUEST_STATUS_AVAILABLE)
        {
            BroadcastHelper::BroadcastQuestAccepted(ai, bot, quest);

            sPlayerbotAIConfig.logEvent(ai, "AcceptQuestAction", quest->GetTitle(), std::to_string(quest->GetQuestId()));
            outputMessage = BOT_TEXT2("quest_accepted", args);
            success = true;
        }
    }

    if (success || !ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayer(requester, outputMessage, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return success;
}

/*
* For creature or gameobject
*/
bool QuestUpdateAddKillAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 entry, questId, available, required;
    ObjectGuid guid;
    p >> questId >> entry >> available >> required >> guid;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(questId);

    if (qInfo && (entry & 0x80000000))
    {
        entry &= 0x7FFFFFFF;
        GameObjectInfo const* info = sObjectMgr.GetGameObjectInfo(entry);
        if (info)
        {
            ai->TellPlayer(requester, chat->formatQuestObjective(info->name, available, required), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            BroadcastHelper::BroadcastQuestUpdateAddKill(ai, bot, qInfo, available, required, info->name);
        }
    }
    else if (qInfo)
    {
        CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(entry);
        if (info)
        {
            ai->TellPlayer(requester, chat->formatQuestObjective(info->Name, available, required), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            BroadcastHelper::BroadcastQuestUpdateAddKill(ai, bot, qInfo, available, required, info->Name);
        }
    }
    else
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%quest_id"] = questId;
        placeholders["%available"] = available;
        placeholders["%required"] = required;

        ai->TellPlayer(
            requester,
            BOT_TEXT2("%available/%required for questId: %quest_id", placeholders),
            PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL,
            false
        );
    }

    sPlayerbotAIConfig.logEvent(ai, "QuestUpdateAddKillAction", std::to_string(questId), std::to_string((float)available / (float)required));
    return false;
}

bool QuestUpdateAddItemAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 itemId, count;
    p >> itemId >> count;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    ItemPrototype const* itemPrototype = sObjectMgr.GetItemPrototype(itemId);

    if (itemPrototype)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%item_link"] = ai->GetChatHelper()->formatItem(itemPrototype);
        uint32 availableItemsCount = ai->GetInventoryItemsCountWithId(itemId);
        placeholders["%quest_obj_available"] = std::to_string(availableItemsCount);

        for (const auto& pair : ai->GetCurrentQuestsRequiringItemId(itemId))
        {
            placeholders["%quest_link"] = chat->formatQuest(pair.first);
            uint32 requiredItemsCount = pair.second;
            placeholders["%quest_obj_required"] = std::to_string(requiredItemsCount);
            ai->TellPlayer(
                requester,
                BOT_TEXT2("%quest_link - %item_link %quest_obj_available/%quest_obj_required", placeholders),
                PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL,
                false
            );

            BroadcastHelper::BroadcastQuestUpdateAddItem(ai, bot, pair.first, availableItemsCount, requiredItemsCount, itemPrototype);
        }
    }
    else {
        std::map<std::string, std::string> placeholders;
        placeholders["%item_id"] = itemId;
        placeholders["%count"] = count;

        ai->TellPlayer(
            requester,
            BOT_TEXT2("Got %count of itemId: %item_id for quest", placeholders),
            PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL,
            false
        );
    }

    sPlayerbotAIConfig.logEvent(ai, "QuestUpdateAddItemAction", std::to_string(itemId), "count: " + std::to_string(count));
    return false;
}

bool QuestUpdateFailedAction::Execute(Event& event)
{
    //opcode SMSG_QUESTUPDATE_FAILED is never sent...(yet?)
    return false;
}

bool QuestUpdateFailedTimerAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId;
    p >> questId;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(questId);

    if (qInfo)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(qInfo);

        ai->TellPlayer(requester, BOT_TEXT2("Failed timer for %quest_link, abandoning", placeholders), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        BroadcastHelper::BroadcastQuestUpdateFailedTimer(ai, bot, qInfo);
    }
    else
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_id"] = std::to_string(questId);

        ai->TellPlayer(requester, BOT_TEXT2("Failed timer for %quest_id", placeholders), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }

    //drop quest
    bot->GetPlayerbotAI()->DropQuest(questId);

    sPlayerbotAIConfig.logEvent(ai, "QuestUpdateFailedTimerAction", std::to_string(questId), "FailedTimer");
    return false;
}

bool QuestUpdateCompleteAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId;
    p >> questId;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(questId);

    if (qInfo)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(qInfo);

        ai->TellPlayer(requester, BOT_TEXT2("Completed %quest_link", placeholders), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        BroadcastHelper::BroadcastQuestUpdateComplete(ai, bot, qInfo);
    }
    else {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_id"] = std::to_string(questId);

        ai->TellPlayer(requester, BOT_TEXT2("Completed %quest_id", placeholders), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }

    sPlayerbotAIConfig.logEvent(ai, "QuestUpdateCompleteAction", std::to_string(questId), "Complete");
    return false;
}
