#include "playerbot/playerbot.h"
#include "AutoCompleteQuestAction.h"

using namespace ai;

bool AutoCompleteQuestAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    // Static list of quest IDs to auto-complete
    static const std::vector<uint32> autoCompleteQuests = {
        0,     //Dummy quest : please remove when a classic quest is found that needs to be skipped.
#ifdef MANGOSBOT_TWO  
        25229, // a-few-good-gnomes: Bot uses motivatron but it does nothing in game.
        12641, // Death Comes From On High: Requires complex vehicle control.
#endif
    };

    bool completedQuest = false;

    // Check the bot's quest log for any active quests from the list
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        if (std::find(autoCompleteQuests.begin(), autoCompleteQuests.end(), questId) == autoCompleteQuests.end())
            continue;

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        if (!pQuest)
            continue;

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
            continue;

        // Add quest items for quests that require items
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 id = pQuest->ReqItemId[x];
            uint32 count = pQuest->ReqItemCount[x];
            if (!id || !count)
                continue;

            uint32 curItemCount = bot->GetItemCount(id, true);

            ItemPosCountVec dest;
            uint8 msg = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = bot->StoreNewItem(dest, id, true);
                bot->SendNewItem(item, count - curItemCount, true, false);
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
                    bot->CastedCreatureOrGO(creature, ObjectGuid((creature > 0 ? HIGHGUID_UNIT : HIGHGUID_GAMEOBJECT), uint32(std::abs(creature)), 1u), spell_id);
            }
            else if (creature > 0)
            {
                if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                    for (uint16 z = 0; z < creaturecount; ++z)
                        bot->KilledMonster(cInfo, nullptr);
            }
            else if (creature < 0)
            {
                for (uint16 z = 0; z < creaturecount; ++z)
                    bot->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
            }
        }
#ifdef MANGOSBOT_TWO
        // player kills
        if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_PLAYER_KILL))
            if (uint32 reqPlayers = pQuest->GetPlayersSlain())
                bot->KilledPlayerCreditForQuest(reqPlayers, pQuest);
#endif

        // If the quest requires reputation to complete
        if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
        {
            uint32 repValue = pQuest->GetRepObjectiveValue();
            uint32 curRep = bot->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue)
#ifndef MANGOSBOT_ONE
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
#else
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry<FactionEntry>(repFaction))
#endif
                    bot->GetReputationMgr().SetReputation(factionEntry, repValue);
        }

        // If the quest requires money
        int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
        if (ReqOrRewMoney < 0)
            bot->ModifyMoney(-ReqOrRewMoney);

        bot->CompleteQuest(questId);
        completedQuest = true;
    }

    if (completedQuest)
    {
        return true;
    }

    return false;
}

bool AutoCompleteQuestAction::isUseful()
{
    if (ai->HasActivePlayerMaster())
        return false;

    return true;
}
