/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "LootMgr.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include "World.h"
#include "Util.h"
#include "SharedDefines.h"
#include "DBCStores.h"
#include "SQLStorages.h"
#include "BattleGround/BattleGroundAV.h"
#include "ItemEnchantmentMgr.h"

INSTANTIATE_SINGLETON_1(LootMgr);

static eConfigFloatValues const qualityToRate[MAX_ITEM_QUALITY] =
{
    CONFIG_FLOAT_RATE_DROP_ITEM_POOR,                       // ITEM_QUALITY_POOR
    CONFIG_FLOAT_RATE_DROP_ITEM_NORMAL,                     // ITEM_QUALITY_NORMAL
    CONFIG_FLOAT_RATE_DROP_ITEM_UNCOMMON,                   // ITEM_QUALITY_UNCOMMON
    CONFIG_FLOAT_RATE_DROP_ITEM_RARE,                       // ITEM_QUALITY_RARE
    CONFIG_FLOAT_RATE_DROP_ITEM_EPIC,                       // ITEM_QUALITY_EPIC
    CONFIG_FLOAT_RATE_DROP_ITEM_LEGENDARY,                  // ITEM_QUALITY_LEGENDARY
    CONFIG_FLOAT_RATE_DROP_ITEM_ARTIFACT,                   // ITEM_QUALITY_ARTIFACT
};

LootStore LootTemplates_Creature("creature_loot_template",     "creature entry",                 true);
LootStore LootTemplates_Disenchant("disenchant_loot_template",   "item disenchant id",             true);
LootStore LootTemplates_Fishing("fishing_loot_template",      "area id",                        true);
LootStore LootTemplates_Gameobject("gameobject_loot_template",   "gameobject lootid",              true);
LootStore LootTemplates_Item("item_loot_template",         "item entry with ITEM_FLAG_LOOTABLE", true);
LootStore LootTemplates_Mail("mail_loot_template",         "mail template id",               false);
LootStore LootTemplates_Pickpocketing("pickpocketing_loot_template", "creature pickpocket lootid",     true);
LootStore LootTemplates_Reference("reference_loot_template",    "reference id",                   false);
LootStore LootTemplates_Skinning("skinning_loot_template",     "creature skinning id",           true);

class LootTemplate::LootGroup                               // A set of loot definitions for items (refs are not allowed)
{
    public:
        void AddEntry(LootStoreItem& item);                 // Adds an entry to the group (at loading stage)
        bool HasQuestDrop() const;                          // True if group includes at least 1 quest drop entry
        bool HasQuestDropForPlayer(Player const* player) const;
        // The same for active quests of the player
        void Process(Loot& loot, Player const* lootOwner) const; // Rolls an item from the group (if any) and adds the item to the loot
        float RawTotalChance() const;                       // Overall chance for the group (without equal chanced items)
        float TotalChance() const;                          // Overall chance for the group

        void Verify(LootStore const& lootstore, uint32 id, uint32 group_id) const;
        void CheckLootRefs(LootIdSet* ref_set) const;
    private:
        LootStoreItemList ExplicitlyChanced;                // Entries with chances defined in DB
        LootStoreItemList EqualChanced;                     // Zero chances - every entry takes the same chance

        LootStoreItem const* Roll(Loot const& loot, Player const* lootOwner) const; // Rolls an item from the group, returns nullptr if all miss their chances
};

// Remove all data and free all memory
void LootStore::Clear()
{
    for (LootTemplateMap::const_iterator itr = m_LootTemplates.begin(); itr != m_LootTemplates.end(); ++itr)
        delete itr->second;
    m_LootTemplates.clear();
}

// Checks validity of the loot store
// Actual checks are done within LootTemplate::Verify() which is called for every template
void LootStore::Verify() const
{
    for (LootTemplateMap::const_iterator i = m_LootTemplates.begin(); i != m_LootTemplates.end(); ++i)
        i->second->Verify(*this, i->first);
}

// Loads a *_loot_template DB table into loot store
// All checks of the loaded template are called from here, no error reports at loot generation required
void LootStore::LoadLootTable()
{
    LootTemplateMap::const_iterator tab;
    uint32 count = 0;

    // Clearing store (for reloading case)
    Clear();

    //                                                 0      1     2                    3        4              5         6
    QueryResult* result = WorldDatabase.PQuery("SELECT entry, item, ChanceOrQuestChance, groupid, mincountOrRef, maxcount, condition_id FROM %s", GetName());

    if (result)
    {
        BarGoLink bar(result->GetRowCount());

        do
        {
            Field* fields = result->Fetch();
            bar.step();

            uint32 entry               = fields[0].GetUInt32();
            uint32 item                = fields[1].GetUInt32();
            float  chanceOrQuestChance = fields[2].GetFloat();
            uint8  group               = fields[3].GetUInt8();
            int32  mincountOrRef       = fields[4].GetInt32();
            uint32 maxcount            = fields[5].GetUInt32();
            uint16 conditionId         = fields[6].GetUInt16();

            if (maxcount > std::numeric_limits<uint8>::max())
            {
                sLog.outErrorDb("Table '%s' entry %u item %u: maxcount value (%u) to large. must be less than %u - skipped", GetName(), entry, item, maxcount, uint32(std::numeric_limits<uint8>::max()));
                continue;                                   // error already printed to log/console.
            }

            if (conditionId)
            {
                const PlayerCondition* condition = sConditionStorage.LookupEntry<PlayerCondition>(conditionId);
                if (!condition)
                {
                    sLog.outErrorDb("Table `%s` for entry %u, item %u has condition_id %u that does not exist in `conditions`, ignoring", GetName(), entry, item, uint32(conditionId));
                    continue;
                }

                if (mincountOrRef < 0 && !PlayerCondition::CanBeUsedWithoutPlayer(conditionId))
                {
                    sLog.outErrorDb("Table '%s' entry %u mincountOrRef %i < 0 and has condition %u that requires a player and is not supported, skipped", GetName(), entry, mincountOrRef, uint32(conditionId));
                    continue;
                }
            }

            LootStoreItem storeitem = LootStoreItem(item, chanceOrQuestChance, group, conditionId, mincountOrRef, maxcount);

            if (!storeitem.IsValid(*this, entry))           // Validity checks
                continue;

            // Looking for the template of the entry
            // often entries are put together
            if (m_LootTemplates.empty() || tab->first != entry)
            {
                // Searching the template (in case template Id changed)
                tab = m_LootTemplates.find(entry);
                if (tab == m_LootTemplates.end())
                {
                    std::pair< LootTemplateMap::iterator, bool > pr = m_LootTemplates.insert(LootTemplateMap::value_type(entry, new LootTemplate));
                    tab = pr.first;
                }
            }
            // else is empty - template Id and iter are the same
            // finally iter refers to already existing or just created <entry, LootTemplate>

            // Adds current row to the template
            tab->second->AddEntry(storeitem);
            ++count;
        }
        while (result->NextRow());

        delete result;

        Verify();                                           // Checks validity of the loot store

        sLog.outString(">> Loaded %u loot definitions (" SIZEFMTD " templates) from table %s", count, m_LootTemplates.size(), GetName());
        sLog.outString();
    }
    else
    {
        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 loot definitions. DB table `%s` is empty.", GetName());
    }
}

bool LootStore::HaveQuestLootFor(uint32 loot_id) const
{
    LootTemplateMap::const_iterator itr = m_LootTemplates.find(loot_id);
    if (itr == m_LootTemplates.end())
        return false;

    // scan loot for quest items
    return itr->second->HasQuestDrop(m_LootTemplates);
}

bool LootStore::HaveQuestLootForPlayer(uint32 loot_id, Player* player) const
{
    LootTemplateMap::const_iterator tab = m_LootTemplates.find(loot_id);
    if (tab != m_LootTemplates.end())
        if (tab->second->HasQuestDropForPlayer(m_LootTemplates, player))
            return true;

    return false;
}

LootTemplate const* LootStore::GetLootFor(uint32 loot_id) const
{
    LootTemplateMap::const_iterator tab = m_LootTemplates.find(loot_id);

    if (tab == m_LootTemplates.end())
        return nullptr;

    return tab->second;
}

void LootStore::LoadAndCollectLootIds(LootIdSet& ids_set)
{
    LoadLootTable();

    for (LootTemplateMap::const_iterator tab = m_LootTemplates.begin(); tab != m_LootTemplates.end(); ++tab)
        ids_set.insert(tab->first);
}

void LootStore::CheckLootRefs(LootIdSet* ref_set) const
{
    for (LootTemplateMap::const_iterator ltItr = m_LootTemplates.begin(); ltItr != m_LootTemplates.end(); ++ltItr)
        ltItr->second->CheckLootRefs(ref_set);
}

void LootStore::ReportUnusedIds(LootIdSet const& ids_set) const
{
    // all still listed ids isn't referenced
    if (!ids_set.empty())
    {
        for (LootIdSet::const_iterator itr = ids_set.begin(); itr != ids_set.end(); ++itr)
            sLog.outErrorDb("Table '%s' entry %d isn't %s and not referenced from loot, and then useless.", GetName(), *itr, GetEntryName());
        sLog.outString();
    }
}

void LootStore::ReportNotExistedId(uint32 id) const
{
    sLog.outErrorDb("Table '%s' entry %d (%s) not exist but used as loot id in DB.", GetName(), id, GetEntryName());
}

//
// --------- LootStoreItem ---------
//

// Checks if the entry (quest, non-quest, reference) takes it's chance (at loot generation)
// RATE_DROP_ITEMS is no longer used for all types of entries
bool LootStoreItem::Roll(bool rate) const
{
    if (chance >= 100.0f)
        return true;

    if (mincountOrRef < 0)                                  // reference case
        return roll_chance_f(chance * (rate ? sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_ITEM_REFERENCED) : 1.0f));

    if (needs_quest)
        return roll_chance_f(chance * (rate ? sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_ITEM_QUEST) : 1.0f));

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemid);

    float qualityModifier = pProto && rate ? sWorld.getConfig(qualityToRate[pProto->Quality]) : 1.0f;

    return roll_chance_f(chance * qualityModifier);
}

// Checks correctness of values
bool LootStoreItem::IsValid(LootStore const& store, uint32 entry) const
{
    if (group >= 1 << 7)                                    // it stored in 7 bit field
    {
        sLog.outErrorDb("Table '%s' entry %d item %d: group (%u) must be less %u - skipped", store.GetName(), entry, itemid, group, 1 << 7);
        return false;
    }

    if (mincountOrRef == 0)
    {
        sLog.outErrorDb("Table '%s' entry %d item %d: wrong mincountOrRef (%d) - skipped", store.GetName(), entry, itemid, mincountOrRef);
        return false;
    }

    if (mincountOrRef > 0)                                  // item (quest or non-quest) entry, maybe grouped
    {
        ItemPrototype const* proto = ObjectMgr::GetItemPrototype(itemid);
        if (!proto)
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: item entry not listed in `item_template` - skipped", store.GetName(), entry, itemid);
            return false;
        }

        if (chance == 0 && group == 0)                      // Zero chance is allowed for grouped entries only
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: equal-chanced grouped entry, but group not defined - skipped", store.GetName(), entry, itemid);
            return false;
        }

        if (chance != 0 && chance < 0.000001f)              // loot with low chance
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: low chance (%f) - skipped", store.GetName(), entry, itemid, chance);
            return false;
        }

        if (maxcount < mincountOrRef)                       // wrong max count
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: max count (%u) less that min count (%i) - skipped", store.GetName(), entry, itemid, uint32(maxcount), mincountOrRef);
            return false;
        }
    }
    else                                                    // mincountOrRef < 0
    {
        if (needs_quest)
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: negative chance is given for a reference, skipped", store.GetName(), entry, itemid);
            return false;
        }
        else if (chance == 0)                               // no chance for the reference
        {
            sLog.outErrorDb("Table '%s' entry %d item %d: zero chance is given for a reference, reference will never be used, skipped", store.GetName(), entry, itemid);
            return false;
        }
    }
    return true;                                            // Referenced template existence is checked at whole store level
}

//
// --------- LootItem ---------
//

// Constructor, copies most fields from LootStoreItem and generates random count
LootItem::LootItem(LootStoreItem const& li, uint32 _lootSlot, uint32 threshold)
{
    if (li.needs_quest)
        lootItemType = LOOTITEM_TYPE_QUEST;
    else if (li.conditionId)
        lootItemType = LOOTITEM_TYPE_CONDITIONNAL;
    else
        lootItemType = LOOTITEM_TYPE_NORMAL;

    itemProto         = ObjectMgr::GetItemPrototype(li.itemid);
    if (itemProto)
    {
        freeForAll       = !!(itemProto->Flags & ITEM_FLAG_PARTY_LOOT);
        displayID        = itemProto->DisplayInfoID;
        isUnderThreshold = itemProto->Quality < threshold;
    }
    else
    {
        sLog.outError("LootItem::LootItem()> item ID(%u) have no prototype!", li.itemid); // maybe i must make an assert here
        freeForAll = false;
        displayID = 0;
        isUnderThreshold = false;
    }


    itemId            = li.itemid;
    conditionId       = li.conditionId;
    lootSlot          = _lootSlot;
    count             = urand(li.mincountOrRef, li.maxcount);     // constructor called for mincountOrRef > 0 only
    randomSuffix      = 0; // TODO:: do we need to implement it? GenerateEnchSuffixFactor(itemId);
    randomPropertyId  = Item::GenerateItemRandomPropertyId(itemId);
    isBlocked         = false;
    currentLooterPass = false;
    isNotVisibleForML = false;
    checkRollNeed     = false;
}

LootItem::LootItem(uint32 _itemId, uint32 _count, uint32 _randomSuffix, int32 _randomPropertyId, uint32 _lootSlot)
{
    itemProto = ObjectMgr::GetItemPrototype(_itemId);
    if (itemProto)
    {
        freeForAll = !!(itemProto->Flags & ITEM_FLAG_PARTY_LOOT);
        displayID = itemProto->DisplayInfoID;
    }
    else
    {
        sLog.outError("LootItem::LootItem()> item ID(%u) have no prototype!", _itemId); // maybe i must make an assert here
        freeForAll = false;
        displayID = 0;
    }

    itemId            = _itemId;
    lootSlot          = _lootSlot;
    conditionId       = 0;
    lootItemType      = LOOTITEM_TYPE_NORMAL;
    count             = _count;
    randomSuffix      = _randomSuffix;
    randomPropertyId  = _randomPropertyId;
    isBlocked         = false;
    isUnderThreshold  = false;
    currentLooterPass = false;
    isNotVisibleForML = false;
    checkRollNeed     = false;
}


// Basic checks for player/item compatibility - if false no chance to see the item in the loot
bool LootItem::AllowedForPlayer(Player const* player, WorldObject const* lootTarget) const
{
    if (!itemProto)
        return false;

    switch (lootItemType)
    {
        case LOOTITEM_TYPE_NORMAL:
            break;

        case LOOTITEM_TYPE_CONDITIONNAL:
            // DB conditions check
            if (!sObjectMgr.IsPlayerMeetToCondition(conditionId, player, player->GetMap(), lootTarget, CONDITION_FROM_LOOT))
                return false;
            break;

        case LOOTITEM_TYPE_QUEST:
            // Checking quests for quest-only drop (check only quests requirements in this case)
            if (!player->HasQuestForItem(itemId))
                return false;
            break;

        default:
            break;
    }

    // Not quest only drop (check quest starting items for already accepted non-repeatable quests)
    if (itemProto->StartQuest && player->GetQuestStatus(itemProto->StartQuest) != QUEST_STATUS_NONE && !player->HasQuestForItem(itemId))
        return false;
	
    return true;
}

LootSlotType LootItem::GetSlotTypeForSharedLoot(Player const* player, Loot const* loot) const
{
    // ignore looted, FFA (each player get own copy) and not allowed items
    if (IsLootedFor(player->GetObjectGuid()) || !AllowedForPlayer(player, loot->GetLootTarget()))
        return MAX_LOOT_SLOT_TYPE;

    if (freeForAll)
        return LOOT_SLOT_NORMAL;                                         // player have not yet looted a free for all item

    if (!lootedBy.empty())
        return MAX_LOOT_SLOT_TYPE;                                       // a not free for all item should not be looted more than once

    if (lootItemType == LOOTITEM_TYPE_QUEST)
    {
        if (loot->m_lootMethod == NOT_GROUP_TYPE_LOOT || loot->m_lootMethod == FREE_FOR_ALL)
            return LOOT_SLOT_NORMAL;
        else
        {
            // Check if its turn of that player to loot a not party loot. The loot may be released or the item may be passed by currentLooter
            if (loot->m_isReleased || currentLooterPass || loot->m_currentLooterGuid == player->GetObjectGuid())
                return LOOT_SLOT_NORMAL;
            return MAX_LOOT_SLOT_TYPE;
        }
    }

    switch (loot->m_lootMethod)
    {
        case FREE_FOR_ALL:
            return LOOT_SLOT_NORMAL;
        case GROUP_LOOT:
        case NEED_BEFORE_GREED:
        {
            if (!isBlocked)
            {
                if (loot->m_isReleased || player->GetObjectGuid() == loot->m_currentLooterGuid)
                    return LOOT_SLOT_NORMAL;
                else
                    return MAX_LOOT_SLOT_TYPE;
            }
            return LOOT_SLOT_VIEW;
        }
        case MASTER_LOOT:
        {
            if (isUnderThreshold)
            {
                if (loot->m_isReleased || player->GetObjectGuid() == loot->m_currentLooterGuid)
                    return LOOT_SLOT_NORMAL;
                return MAX_LOOT_SLOT_TYPE;
            }
            else
            {
                if (player->GetObjectGuid() == loot->m_masterOwnerGuid)
                    return LOOT_SLOT_MASTER;
                else
                {
                    if (!isBlocked && isNotVisibleForML)
                        return LOOT_SLOT_NORMAL;
                }
                return LOOT_SLOT_VIEW;
            }
        }
        case ROUND_ROBIN:
        {
            if (loot->m_isReleased || player->GetObjectGuid() == loot->m_currentLooterGuid)
                return LOOT_SLOT_NORMAL;
            return MAX_LOOT_SLOT_TYPE;
        }
        case NOT_GROUP_TYPE_LOOT:
            return LOOT_SLOT_NORMAL;
        default:
            return MAX_LOOT_SLOT_TYPE;
    }
}

//
// ------- Loot Roll -------
//

// Send the roll for the whole group
void GroupLootRoll::SendStartRoll()
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8 + 4 + 4 + 4 + 4 + 4 + 1));
    data << m_loot->GetLootGuid();                          // creature guid what we're looting
    data << uint32(m_itemSlot);                             // item slot in loot
    data << uint32(m_lootItem->itemId);                     // the itemEntryId for the item that shall be rolled for
    data << uint32(m_lootItem->randomSuffix);               // randomSuffix
    data << uint32(m_lootItem->randomPropertyId);           // item random property ID
    data << uint32(LOOT_ROLL_TIMEOUT);                      // the countdown time to choose "need" or "greed"

    size_t voteMaskPos = data.wpos();
    data << uint8(0);

    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        if (itr->second.vote == ROLL_NOT_VALID)
            continue;

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr || !plr->GetSession())
            continue;
        // dependent from player
        RollVoteMask mask = m_voteMask;
        // In NEED_BEFORE_GREED need disabled for non-usable item for player
        if (m_loot->m_lootMethod == NEED_BEFORE_GREED && plr->CanUseItem(m_lootItem->itemProto) != EQUIP_ERR_OK)
            mask = RollVoteMask(mask & ~ROLL_VOTE_MASK_NEED);
        data.put<uint8>(voteMaskPos, uint8(mask));

        plr->GetSession()->SendPacket(&data);
    }
}

// Send all passed message
void GroupLootRoll::SendAllPassed()
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8 + 4 + 4 + 4 + 4));
    data << m_loot->GetLootGuid();                          // creature guid what we're looting
    data << uint32(m_itemSlot);                             // item slot in loot
    data << uint32(m_lootItem->itemId);                     // the itemEntryId for the item that shall be rolled for
    data << uint32(m_lootItem->randomSuffix);               // randomSuffix
    data << uint32(m_lootItem->randomPropertyId);           // item random property ID

    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        if (itr->second.vote == ROLL_NOT_VALID)
            continue;

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr || !plr->GetSession())
            continue;

        plr->GetSession()->SendPacket(&data);
    }
}

// Send roll 'value' of the whole group and the winner to the whole group
void GroupLootRoll::SendLootRollWon(ObjectGuid const& targetGuid, uint32 rollNumber, RollVote rollType)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8 + 4 + 4 + 4 + 4 + 8 + 1 + 1));
    data << m_loot->GetLootGuid();                          // creature guid what we're looting
    data << uint32(m_itemSlot);                             // item slot in loot
    data << uint32(m_lootItem->itemId);                     // the itemEntryId for the item that shall be rolled for
    data << uint32(m_lootItem->randomSuffix);               // randomSuffix
    data << uint32(m_lootItem->randomPropertyId);           // item random property ID
    data << targetGuid;                                     // guid of the player who won.
    data << uint8(rollNumber);                              // rollnumber related to SMSG_LOOT_ROLL
    data << uint8(rollType);                                // Rolltype related to SMSG_LOOT_ROLL

    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        switch (itr->second.vote)
        {
            case ROLL_PASS:
                break;
            case ROLL_NOT_EMITED_YET:
            case ROLL_NOT_VALID:
                SendRoll(itr->first, 128, 128);
                break;
            default:
                SendRoll(itr->first, itr->second.number, itr->second.vote);
                break;
        }
    }

    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        if (itr->second.vote == ROLL_NOT_VALID)
            continue;

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr || !plr->GetSession())
            continue;
        plr->GetSession()->SendPacket(&data);
    }
}

// Send roll of targetGuid to the whole group (included targuetGuid)
void GroupLootRoll::SendRoll(ObjectGuid const& targetGuid, uint32 rollNumber, uint32 rollType)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8 + 4 + 8 + 4 + 4 + 4 + 1 + 1 + 1));
    data << m_loot->GetLootGuid();                          // creature guid what we're looting
    data << uint32(m_itemSlot);                             // item slot in loot
    data << targetGuid;
    data << uint32(m_lootItem->itemId);                     // the itemEntryId for the item that shall be rolled for
    data << uint32(m_lootItem->randomSuffix);               // randomSuffix
    data << uint32(m_lootItem->randomPropertyId);           // item random property ID
    data << uint8(rollNumber);                              // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << uint8(rollType);                                // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll
    data << uint8(0);                                       // auto pass on loot

    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        if (itr->second.vote == ROLL_NOT_VALID)
            continue;

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr || !plr->GetSession())
            continue;

        plr->GetSession()->SendPacket(&data);
    }
}

GroupLootRoll::~GroupLootRoll()
{
    if (m_isStarted)
        SendAllPassed();
}

// Try to start the group roll for the specified item (it may fail for quest item or any condition
// If this method return false the roll have to be removed from the container to avoid any problem
bool GroupLootRoll::TryToStart(Loot& loot, uint32 itemSlot)
{
    if (!m_isStarted)
    {
        if (itemSlot >= loot.m_lootItems.size())
            return false;

        // initialize the data needed for the roll
        m_lootItem = loot.GetLootItemInSlot(itemSlot);

        if (m_lootItem->lootItemType == LOOTITEM_TYPE_QUEST)
            return false;

        m_loot = &loot;
        m_itemSlot = itemSlot;
        m_lootItem->isBlocked = true;                          // block the item while rolling

        uint32 playerCount = 0;
        for (GuidSet::const_iterator itr = m_loot->m_ownerSet.begin(); itr != m_loot->m_ownerSet.end(); ++itr)
        {
            Player* plr = sObjectMgr.GetPlayer(*itr);
            if (!plr || !m_lootItem->AllowedForPlayer(plr, loot.GetLootTarget()))   // check if player meet the condition to be able to roll this item
            {
                m_rollVoteMap[*itr].vote = ROLL_NOT_VALID;
                continue;
            }
            m_rollVoteMap[*itr].vote = ROLL_NOT_EMITED_YET; // initialize player vote map
            ++playerCount;
        }

        // initialize item prototype
        m_voteMask = ROLL_VOTE_MASK_ALL;

        if (playerCount > 1)                                  // check if more than one player can loot this item
        {
            // start the roll
            SendStartRoll();
            m_endTime = time(nullptr) + (LOOT_ROLL_TIMEOUT / 1000);
            m_isStarted = true;
            return true;
        }
        // no need to start roll if one or less player can loot this item so place it under threshold
        m_lootItem->isUnderThreshold = true;
        m_lootItem->isBlocked = false;
    }
    return false;
}

// Add vote from playerGuid
bool GroupLootRoll::PlayerVote(Player* player, RollVote vote)
{
    ObjectGuid const& playerGuid = player->GetObjectGuid();
    RollVoteMap::iterator voterItr = m_rollVoteMap.find(playerGuid);
    if (voterItr == m_rollVoteMap.end())
        return false;

    voterItr->second.vote = vote;

    if (vote != ROLL_PASS && vote != ROLL_NOT_VALID)
        voterItr->second.number = urand(1, 100);

    switch (vote)
    {
        case ROLL_PASS:                                     // Player choose pass
        {
            SendRoll(playerGuid, 128, 128);
            break;
        }
        case ROLL_NEED:                                     // player choose Need
        {
            SendRoll(playerGuid, 0, 0);
            break;
        }
        case ROLL_GREED:                                    // player choose Greed
        {
            SendRoll(playerGuid, 128, 2);
            break;
        }
        default:                                            // Roll removed case
            return false;
    }
    return true;
}

// check if we can found a winner for this roll or if timer is expired
bool GroupLootRoll::UpdateRoll()
{
    RollVoteMap::const_iterator winnerItr = m_rollVoteMap.end();

    if (AllPlayerVoted(winnerItr) || m_endTime <= time(nullptr))
    {
        Finish(winnerItr);
        return true;
    }
    return false;
}

/**
* \brief: Check if all player have voted and return true in that case. Also return current winner.
* \param: RollVoteMap::const_iterator& winnerItr > will be different than m_rollCoteMap.end() if winner exist. (Someone voted greed or need)
* \returns: bool > true if all players voted
**/
bool GroupLootRoll::AllPlayerVoted(RollVoteMap::const_iterator& winnerItr)
{
    uint32 notVoted = 0;
    bool isSomeoneNeed = false;

    winnerItr = m_rollVoteMap.end();
    for (RollVoteMap::const_iterator itr = m_rollVoteMap.begin(); itr != m_rollVoteMap.end(); ++itr)
    {
        switch (itr->second.vote)
        {
            case ROLL_NEED:
                if (!isSomeoneNeed || winnerItr == m_rollVoteMap.end() || itr->second.number > winnerItr->second.number)
                {
                    isSomeoneNeed = true;                                               // first passage will force to set winner because need is prioritized
                    winnerItr = itr;
                }
                break;
            case ROLL_GREED:
            case ROLL_DISENCHANT:
                if (!isSomeoneNeed)                                                      // if at least one need is detected then winner can't be a greed
                {
                    if (winnerItr == m_rollVoteMap.end() || itr->second.number > winnerItr->second.number)
                        winnerItr = itr;
                }
                break;
            // Explicitly passing excludes a player from winning loot, so no action required.
            case ROLL_PASS:
                break;
            case ROLL_NOT_EMITED_YET:
                ++notVoted;
                break;
            default:
                break;
        }
    }

    return notVoted == 0;
}

// terminate the roll
void GroupLootRoll::Finish(RollVoteMap::const_iterator& winnerItr)
{
    m_lootItem->isBlocked = false;
    if (winnerItr == m_rollVoteMap.end())
    {
        SendAllPassed();
        m_loot->m_isReleased = true;
    }
    else
    {
        SendLootRollWon(winnerItr->first, winnerItr->second.number, winnerItr->second.vote);

        Player* plr = sObjectMgr.GetPlayer(winnerItr->first);
        if (plr && plr->GetSession())
        {
            m_loot->SendItem(plr, m_itemSlot);
        }
        else
        {
            // hum the winner is not available
            m_loot->m_isReleased = true;
        }
    }
    m_isStarted = false;
}

GroupLootRoll* Loot::GetRollForSlot(uint32 itemSlot)
{
    GroupLootRollMap::iterator rollItr = m_roll.find(itemSlot);
    if (rollItr == m_roll.end())
        return nullptr;
    return &rollItr->second;
}

//
// --------- Loot ---------
//

// Inserts the item into the loot (called by LootTemplate processors)
void Loot::AddItem(LootStoreItem const& item)
{
    if (m_lootItems.size() < MAX_NR_LOOT_ITEMS)                              // Normal drop
    {
        LootItem* lootItem = new LootItem(item, m_maxSlot++, uint32(m_threshold));

        if (!lootItem->isUnderThreshold)                                    // set flag for later to know that we have an over threshold item
            m_haveItemOverThreshold = true;

        m_lootItems.push_back(lootItem);
    }
}

// Insert item into the loot explicit way. (used for container item and Item::LoadFromDB)
void Loot::AddItem(uint32 itemid, uint32 count, uint32 randomSuffix, int32 randomPropertyId)
{
    if (m_lootItems.size() < MAX_NR_LOOT_ITEMS)                              // Normal drop
    {
        LootItem* lootItem = new LootItem(itemid, count, randomSuffix, randomPropertyId, m_maxSlot++);

        m_lootItems.push_back(lootItem);
    }
}

// Calls processor of corresponding LootTemplate (which handles everything including references)
bool Loot::FillLoot(uint32 loot_id, LootStore const& store, Player* lootOwner, bool personal, bool noEmptyError)
{
    // Must be provided
    if (!lootOwner)
        return false;

    LootTemplate const* tab = store.GetLootFor(loot_id);

    if (!tab)
    {
        if (!noEmptyError)
            sLog.outErrorDb("Table '%s' loot id #%u used but it doesn't have records.", store.GetName(), loot_id);
        return false;
    }

    m_lootItems.reserve(MAX_NR_LOOT_ITEMS);

    tab->Process(*this, lootOwner, store, store.IsRatesAllowed()); // Processing is done there, callback via Loot::AddItem()

    // Must now check if current looter have right to loot all item or he will lockout that item until he look and release the loot
    if (!m_currentLooterGuid.IsEmpty() && m_ownerSet.size() > 1 && m_lootMethod != FREE_FOR_ALL) // only for group that are not free for all
    {
        Player* currentLooter = ObjectAccessor::FindPlayer(m_currentLooterGuid);
        for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
        {
            LootItem* lootItem = *lootItemItr;

            // Normal loot have no condition to check
            if (lootItem->lootItemType == LOOTITEM_TYPE_NORMAL)
                continue;

            if (!currentLooter || !lootItem->AllowedForPlayer(currentLooter, m_lootTarget))
                lootItem->currentLooterPass = true;         // Some item may not be allowed for current looter, must set this flag to avoid item not distributed to other player
        }
    }

    return true;
}

// Get loot status for a specified player
uint32 Loot::GetLootStatusFor(Player const* player) const
{
    uint32 status = 0;

    if (m_gold != 0)
        status |= LOOT_STATUS_CONTAIN_GOLD;

    for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;

        LootSlotType slotType = lootItem->GetSlotTypeForSharedLoot(player, this);
        if (slotType == MAX_LOOT_SLOT_TYPE)
            continue;

        status |= LOOT_STATUS_NOT_FULLY_LOOTED;

        if (lootItem->freeForAll)
            status |= LOOT_STATUS_CONTAIN_FFA;
    }
    return status;
}

// Is there is any loot available for provided player
bool Loot::IsLootedFor(Player const* player) const
{
    return (GetLootStatusFor(player) == 0);
}

bool Loot::IsLootedForAll() const
{
    for (GuidSet::const_iterator itr = m_ownerSet.begin(); itr != m_ownerSet.end(); ++itr)
    {
        Player* player = ObjectAccessor::FindPlayer(*itr);
        if (!player)
            continue;

        if (!IsLootedFor(player))
            return false;
    }
    return true;
}

bool Loot::CanLoot(Player const* player)
{
    ObjectGuid const& playerGuid = player->GetObjectGuid();

    // not in Guid list of possible owner mean cheat or big problem
    GuidSet::const_iterator itr = m_ownerSet.find(playerGuid);
    if (itr == m_ownerSet.end())
        return false;

    uint32 lootStatus = GetLootStatusFor(player);

    // is already looted?
    if (!lootStatus)
        return false;

    // all player that have right too loot have right to loot dropped money
    if (lootStatus & LOOT_STATUS_CONTAIN_GOLD || lootStatus & LOOT_STATUS_CONTAIN_FFA)
        return true;

    if (m_lootMethod == NOT_GROUP_TYPE_LOOT || m_lootMethod == FREE_FOR_ALL)
        return true;

    if (m_haveItemOverThreshold)
    {
        // master loot have always loot right when the loot contain over threshold item
        if (m_lootMethod == MASTER_LOOT && player->GetObjectGuid() == m_masterOwnerGuid)
            return true;

        // player can all loot on 'group loot' or 'need before greed' loot type
        if (m_lootMethod != MASTER_LOOT && m_lootMethod != ROUND_ROBIN)
            return true;
    }

    // if the player is the current looter (his turn to loot under threshold item) or the current looter released the loot then the player can loot
    if (m_isReleased || player->GetObjectGuid() == m_currentLooterGuid)
        return true;

    return false;
}

void Loot::NotifyItemRemoved(uint32 lootIndex)
{
    // notify all players that are looting this that the item was removed
    GuidSet::iterator i_next;
    for (GuidSet::iterator i = m_playersLooting.begin(); i != m_playersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        Player* plr = ObjectAccessor::FindPlayer(*i);

        if (plr && plr->GetSession())
            NotifyItemRemoved(plr, lootIndex);
        else
            m_playersLooting.erase(i);
    }
}

void Loot::NotifyItemRemoved(Player* player, uint32 lootIndex)
{
    // notify a player that are looting this that the item was removed
    WorldPacket data(SMSG_LOOT_REMOVED, 1);
    data << uint8(lootIndex);
    player->GetSession()->SendPacket(&data);
}

void Loot::NotifyMoneyRemoved()
{
    // notify all players that are looting this that the money was removed
    GuidSet::iterator i_next;
    for (GuidSet::iterator i = m_playersLooting.begin(); i != m_playersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        Player* plr = ObjectAccessor::FindPlayer(*i);
        if (plr && plr->GetSession())
        {
            WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
            plr->GetSession()->SendPacket(&data);
        }
        else
            m_playersLooting.erase(i);
    }
}

void Loot::GenerateMoneyLoot(uint32 minAmount, uint32 maxAmount)
{
    if (maxAmount > 0)
    {
        if (maxAmount <= minAmount)
            m_gold = uint32(maxAmount * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
        else if ((maxAmount - minAmount) < 32700)
            m_gold = uint32(urand(minAmount, maxAmount) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
        else
            m_gold = uint32(urand(minAmount >> 8, maxAmount >> 8) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY)) << 8;
    }
}

void Loot::SendReleaseFor(ObjectGuid const& guid)
{
    Player* plr = sObjectAccessor.FindPlayer(guid);
    if (plr && plr->GetSession())
        SendReleaseFor(plr);
}

// no check for null pointer so it must be valid
void Loot::SendReleaseFor(Player* plr)
{
    WorldPacket data(SMSG_LOOT_RELEASE_RESPONSE, (8 + 1));
    data << m_guidTarget;
    data << uint8(1);
    plr->GetSession()->SendPacket(&data);
    SetPlayerIsNotLooting(plr);
}

void Loot::SendReleaseForAll()
{
    GuidSet::iterator itr = m_playersLooting.begin();
    while (itr != m_playersLooting.end())
        SendReleaseFor(*itr++);
}

void Loot::SetPlayerIsLooting(Player* player)
{
    m_playersLooting.insert(player->GetObjectGuid());      // add 'this' player as one of the players that are looting 'loot'
    player->SetLootGuid(m_guidTarget);                     // used to keep track of what loot is opened for that player
    if (m_lootType == LOOT_CORPSE || m_isChest)
    {
        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
        if (m_guidTarget.IsGameObject())
            static_cast<GameObject*>(m_lootTarget)->SetInUse(true);
    }
}

void Loot::SetPlayerIsNotLooting(Player* player)
{
    m_playersLooting.erase(player->GetObjectGuid());
    player->SetLootGuid(ObjectGuid());
    if (m_lootType == LOOT_CORPSE || m_isChest)
    {
        player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
        if (m_guidTarget.IsGameObject())
            static_cast<GameObject*>(m_lootTarget)->SetInUse(false);
    }
}

void Loot::Release(Player* player)
{
    bool updateClients = false;
    // the owner of the loot released it
    if (!m_isReleased && player->GetObjectGuid() == m_currentLooterGuid)
    {
        m_isReleased = true;
        updateClients = true;
    }

    switch (m_guidTarget.GetHigh())
    {
        case HIGHGUID_GAMEOBJECT:
        {
            GameObject* go = (GameObject*) m_lootTarget;
            SetPlayerIsNotLooting(player);

            // GO is mineral vein? so it is not removed after its looted
            switch (go->GetGoType())
            {
                case GAMEOBJECT_TYPE_DOOR:
                    // locked doors are opened with spelleffect openlock, prevent remove its as looted
                    go->UseDoorOrButton();
                    break;
                case GAMEOBJECT_TYPE_CHEST:
                {
                    if (!IsLootedFor(player))
                        break;

                    uint32 go_min = go->GetGOInfo()->chest.minSuccessOpens;
                    uint32 go_max = go->GetGOInfo()->chest.maxSuccessOpens;
                    bool refill = false;

                    // only vein pass this check
                    if (go_min != 0 && go_max > go_min)
                    {
                        float amount_rate = sWorld.getConfig(CONFIG_FLOAT_RATE_MINING_AMOUNT);
                        float min_amount = go_min * amount_rate;
                        float max_amount = go_max * amount_rate;

                        go->AddUse();
                        float uses = float(go->GetUseCount());
                        if (uses < max_amount)
                        {
                            if (uses >= min_amount)
                            {
                                float chance_rate = sWorld.getConfig(CONFIG_FLOAT_RATE_MINING_NEXT);

                                int32 ReqValue = 175;
                                LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->chest.lockId);
                                if (lockInfo)
                                    ReqValue = lockInfo->Skill[0];
                                float skill = float(player->GetSkillValue(SKILL_MINING)) / (ReqValue + 25);
                                double chance = pow(0.8 * chance_rate, 4 * (1 / double(max_amount)) * double(uses));
                                if (roll_chance_f(float(100.0f * chance + skill)))
                                    refill = true;
                            }
                            else
                                refill = true;  // 100% chance untill min uses
                        }
                    }

                    if (refill)
                    {
                        // this vein have can be now refilled, as this is a vein no other player are looting it so no need to send them release
                        Clear();                // clear the content and reset some values
                        FillLoot(go->GetGOInfo()->GetLootId(), LootTemplates_Gameobject, player, false); // refill the loot with new items
                        go->SetLootState(GO_READY);
                    }
                    else
                        go->SetLootState(GO_JUST_DEACTIVATED);

                    break;
                }
                case GAMEOBJECT_TYPE_FISHINGHOLE:
                    // The fishing hole used once more
                    go->AddUse();                           // if the max usage is reached, will be despawned at next tick
                    if (go->GetUseCount() >= urand(go->GetGOInfo()->fishinghole.minSuccessOpens, go->GetGOInfo()->fishinghole.maxSuccessOpens))
                    {
                        go->SetLootState(GO_JUST_DEACTIVATED);
                    }
                    else
                        go->SetLootState(GO_READY);
                    break;
                default:
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    break;
            }
            break;
        }
        case HIGHGUID_CORPSE:                               // ONLY remove insignia at BG
        {
            Corpse* corpse = (Corpse*) m_lootTarget;
            if (!corpse || !corpse->IsWithinDistInMap(player, INTERACTION_DISTANCE))
                return;

            if (IsLootedFor(player))
            {
                Clear();
                corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
            }
            break;
        }
        case HIGHGUID_ITEM:
        {
            switch (m_lootType)
            {
                // temporary loot in stacking items, clear loot state, no auto loot move
                case LOOT_PROSPECTING:
                {
                    uint32 count = m_itemTarget->GetCount();

                    // >=5 checked in spell code, but will work for cheating cases also with removing from another stacks.
                    if (count > 5)
                        count = 5;

                    // reset loot for allow repeat looting if stack > 5
                    Clear();
                    m_itemTarget->SetLootState(ITEM_LOOT_REMOVED);

                    player->DestroyItemCount(m_itemTarget, count, true);
                    break;
                }
                // temporary loot, auto loot move
                case LOOT_DISENCHANTING:
                {
                    if (!IsLootedFor(player))
                        AutoStore(player); // can be lost if no space
                    Clear();
                    m_itemTarget->SetLootState(ITEM_LOOT_REMOVED);
                    player->DestroyItem(m_itemTarget->GetBagSlot(), m_itemTarget->GetSlot(), true);
                    break;
                }
                // normal persistence loot
                default:
                {
                    // must be destroyed only if no loot
                    if (IsLootedFor(player))
                    {
                        m_itemTarget->SetLootState(ITEM_LOOT_REMOVED);
                        player->DestroyItem(m_itemTarget->GetBagSlot(), m_itemTarget->GetSlot(), true);
                    }
                    break;
                }
            }
            return;                                         // item can be looted only single player
        }
        case HIGHGUID_UNIT:
        {
            switch (m_lootType)
            {
                case LOOT_PICKPOCKETING:
                {
                    if (IsLootedFor(player))
                    {
                        Creature* creature = (Creature*)m_lootTarget;
                        creature->SetLootStatus(CREATURE_LOOT_STATUS_PICKPOCKETED);
                    }
                    break;
                }
                case LOOT_SKINNING:
                {
                    SetPlayerIsNotLooting(player);
                    Creature* creature = (Creature*)m_lootTarget;
                    if (IsLootedFor(player))
                    {
                        creature->SetLootStatus(CREATURE_LOOT_STATUS_SKINNED);
                    }
                    else
                    {
                        // player released his skin so we can make it available for the member group or for himself
                        Group* grp = player->GetGroup();
                        m_ownerSet.clear();
                        if (grp)
                        {
                            // we need to fill m_ownerSet with player who have access to the loot
                            Group::MemberSlotList const& memberList = grp->GetMemberSlots();
                            Group::MemberSlotList::const_iterator memberItr;
                            for (memberItr = memberList.begin(); memberItr != memberList.end(); ++memberItr)
                                m_ownerSet.insert(memberItr->guid);
                        }
                        else
                            m_ownerSet.insert(player->GetObjectGuid());
                        m_lootMethod = FREE_FOR_ALL;
                        creature->SetLootStatus(CREATURE_LOOT_STATUS_SKIN_AVAILABLE);
                        ForceLootAnimationCLientUpdate();
                    }
                    break;
                }
                case LOOT_CORPSE:
                {
                    Creature* creature = (Creature*)m_lootTarget;
                    SetPlayerIsNotLooting(player);
                    if (IsLootedForAll())
                    {
                        SendReleaseForAll();
                        creature->SetLootStatus(CREATURE_LOOT_STATUS_LOOTED);
                    }
                    else if (updateClients)
                    {
                        // player have released the corpse and some loot still available, we need to resend loot flags for each players to provide them remaininig loot access
                        ForceLootAnimationCLientUpdate();
                    }

                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

// Popup windows with loot content
void Loot::ShowContentTo(Player* plr)
{
    if (m_isChest)
    {
        if (static_cast<GameObject*>(m_lootTarget)->IsInUse())
        {
            SendReleaseFor(plr);
            return;
        }

        if (m_ownerSet.find(plr->GetObjectGuid()) == m_ownerSet.end())
        {
            // TODO:: Player who had no right before opened the chest. Sure for wild chest this is right but, in case of
            // a player released its corpse during a boss fight and that boss drop a chest that player must not have right
            // to loot the chest in any way
            SetGroupLootRight(plr);
        }
    }

    if (m_lootMethod != NOT_GROUP_TYPE_LOOT)
    {
        if (!m_isChecked)
            GroupCheck();

        switch (m_lootMethod)
        {
            case MASTER_LOOT:
            case NEED_BEFORE_GREED:
            case GROUP_LOOT:
            {
                CheckIfRollIsNeeded(plr);               // check if there is the need to start a roll
                break;
            }
            default:
                break;
        }
    }


    WorldPacket data(SMSG_LOOT_RESPONSE);
    data << m_guidTarget;
    data << uint8(m_clientLootType);

    GetLootContentFor(plr, data);                           // fill the data with items contained in the loot (may be empty)
    SetPlayerIsLooting(plr);

    plr->SendDirectMessage(&data);
}

void Loot::GroupCheck()
{
    m_isChecked = true;
    switch (m_lootMethod)
    {
        case MASTER_LOOT:
        {
            Player* masterLooter = nullptr;
            uint8 playerCount = 0;
            WorldPacket data(SMSG_LOOT_MASTER_LIST);
            data << uint8(0);
            for (GuidSet::const_iterator itr = m_ownerSet.begin(); itr != m_ownerSet.end(); ++itr)
            {
                Player* looter = sObjectAccessor.FindPlayer(*itr);
                if (!looter)
                    continue;

                if (*itr == m_masterOwnerGuid)
                    masterLooter = looter;

                data << *itr;
                ++playerCount;
            }
            data.put<uint8>(0, playerCount);

            for (GuidSet::const_iterator itr = m_ownerSet.begin(); itr != m_ownerSet.end(); ++itr)
            {
                Player* looter = sObjectAccessor.FindPlayer(*itr);
                if (!looter)
                    continue;
                looter->GetSession()->SendPacket(&data);
            }

            for (uint8 itemSlot = 0; itemSlot < m_lootItems.size(); ++itemSlot)
            {
                LootItem* lootItem = GetLootItemInSlot(itemSlot);

                // roll for over-threshold item if it's one-player loot
                if (!lootItem->freeForAll && lootItem->itemProto->Quality < uint32(m_threshold))
                    lootItem->isUnderThreshold = true;

                if (!lootItem->isUnderThreshold)
                {
                    // we have to check is the item is visible for the master or no one could be able to get it
                    if (masterLooter)
                    {
                        if (!lootItem->AllowedForPlayer(masterLooter, m_lootTarget))
                        {
                            lootItem->isNotVisibleForML = true;
                            lootItem->checkRollNeed = true;
                        }
                    }
                    else
                    {
                        // master loot is not connected, thus we will just set all conditional and quest item visible for players that fulfill conditions
                        if (lootItem->lootItemType != LOOTITEM_TYPE_NORMAL)
                        {
                            lootItem->isNotVisibleForML = true;
                            lootItem->checkRollNeed = true;
                        }
                    }
                }
            }
            break;
        }
        case NEED_BEFORE_GREED:
        case GROUP_LOOT:
        {
            for (uint8 itemSlot = 0; itemSlot < m_lootItems.size(); ++itemSlot)
            {
                LootItem* lootItem = GetLootItemInSlot(itemSlot);

                // roll for over-threshold item if it's one-player loot
                if (lootItem->itemProto->Quality >= uint32(m_threshold) && !lootItem->freeForAll)
                    lootItem->checkRollNeed = true;
                else
                    lootItem->isUnderThreshold = true;
            }
            break;
        }
        default:
            break;
    }
}

// check if there is need to launch a roll
void Loot::CheckIfRollIsNeeded(Player const* plr)
{
    if (!plr)
        return;

    for (uint8 itemSlot = 0; itemSlot < m_lootItems.size(); ++itemSlot)
    {
        LootItem* lootItem = GetLootItemInSlot(itemSlot);

        if (!lootItem->checkRollNeed)
            continue;

        if (((m_lootMethod == MASTER_LOOT && lootItem->isNotVisibleForML) || m_lootMethod != MASTER_LOOT) && lootItem->AllowedForPlayer(plr, m_lootTarget))
        {
            if (!m_roll[itemSlot].TryToStart(*this, itemSlot))      // Create and try to start a roll
                m_roll.erase(m_roll.find(itemSlot));                // Cannot start roll so we have to delete it (find will not fail as the item was just created)

            lootItem->checkRollNeed = false;                       // No more check is needed for this item
            return;
        }
    }
}

// Set the player who have right for this loot
void Loot::SetGroupLootRight(Player* player)
{
    m_ownerSet.clear();
    Group* grp = player->GetGroup();
    if (grp && (!m_isChest || (m_isChest && static_cast<GameObject*>(m_lootTarget)->GetGOInfo()->chest.groupLootRules)))
    {
        m_lootMethod = grp->GetLootMethod();
        m_threshold = grp->GetLootThreshold();

        // we need to fill m_ownerSet with player who have access to the loot
        Group::MemberSlotList const& memberList = grp->GetMemberSlots();
        ObjectGuid currentLooterGuid = grp->GetCurrentLooterGuid();
        GuidList ownerList;                 // used to keep order of the player (important to get correctly next looter)

        // current looter must be in the group
        Group::MemberSlotList::const_iterator currentLooterItr;
        for (currentLooterItr = memberList.begin(); currentLooterItr != memberList.end(); ++currentLooterItr)
            if (currentLooterItr->guid == currentLooterGuid)
                break;

        // if not then assign first player in group
        if (currentLooterItr == memberList.end())
        {
            currentLooterItr = memberList.begin();
            currentLooterGuid = currentLooterItr->guid;
            grp->SetNextLooterGuid(currentLooterGuid);
        }

        // now that we get a valid current looter iterator we can start to check the loot owner
        Group::MemberSlotList::const_iterator itr = currentLooterItr;
        do
        {
            ++itr;                              // we start by next current looter position in list that way we have directly next looter in result ownerSet (if any exist)
            if (itr == memberList.end())
                itr = memberList.begin();       // reached the end of the list is possible so simply restart from the first element in it

            Player* looter = ObjectAccessor::FindPlayer(itr->guid);

            if (!looter)
                continue;

            if (looter->IsWithinDist(m_lootTarget, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            {
                m_ownerSet.insert(itr->guid);   // save this guid to main owner set
                ownerList.push_back(itr->guid); // save this guid to local ordered GuidList (we need to keep it ordered only here)

                // get enchant skill of authorized looter
                uint32 enchantSkill = looter->GetSkillValue(SKILL_ENCHANTING);
                if (m_maxEnchantSkill < enchantSkill)
                    m_maxEnchantSkill = enchantSkill;
            }

        } while (itr != currentLooterItr);

        if (m_lootMethod == MASTER_LOOT)
        {
            m_masterOwnerGuid = grp->GetMasterLooterGuid();
            // check if master is in looter list
            if (m_ownerSet.find(m_masterOwnerGuid) == m_ownerSet.end())
            {
                m_ownerSet.insert(m_masterOwnerGuid);
                ownerList.push_back(m_masterOwnerGuid);
            }
        }

        // if more than one player have right to loot than we have to handle group method, round robin, roll, etc..
        if (m_ownerSet.size() > 1 && m_lootMethod != FREE_FOR_ALL)
        {
            // is current looter have right for this loot?
            if (m_ownerSet.find(currentLooterGuid) == m_ownerSet.end())
            {
                // as owner list is filled from NEXT current looter position we can assign first element in list as looter and next element in list as next looter
                m_currentLooterGuid = ownerList.front();                        // set first player who have right to loot as current looter
                grp->SetNextLooterGuid(*(++ownerList.begin()));                 // set second player who have right as next looter
            }
            else
            {
                // as owner set is filled from NEXT current looter position we can assign first element in list as next looter
                m_currentLooterGuid = currentLooterGuid;
                grp->SetNextLooterGuid(ownerList.front());                      // set first player who have right as next looter
            }

            SendAllowedLooter();
            m_isChecked = false;
            return;
        }
        m_currentLooterGuid = player->GetObjectGuid();
        SendAllowedLooter();
    }

    m_ownerSet.insert(player->GetObjectGuid());
    m_lootMethod = NOT_GROUP_TYPE_LOOT;
}

Loot::Loot(Player* player, Creature* creature, LootType type) :
    m_lootTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(type),
    m_clientLootType(CLIENT_LOOT_CORPSE), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false),
    m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    // the player whose group may loot the corpse
    if (!player)
    {
        sLog.outError("LootMgr::CreateLoot> Error cannot get looter info to create loot!");
        return;
    }

    if (!creature)
    {
        sLog.outError("Loot::CreateLoot> cannot create loot, no creature passed!");
        return;
    }

    m_lootTarget = creature;
    m_guidTarget = creature->GetObjectGuid();
    CreatureInfo const* creatureInfo = creature->GetCreatureInfo();

    switch (type)
    {
        case LOOT_CORPSE:
        {
            // setting loot right
            SetGroupLootRight(player);
            m_clientLootType = CLIENT_LOOT_CORPSE;

            if ((creatureInfo->LootId && FillLoot(creatureInfo->LootId, LootTemplates_Creature, player, false)) || creatureInfo->MaxLootGold > 0)
            {
                GenerateMoneyLoot(creatureInfo->MinLootGold, creatureInfo->MaxLootGold);
                // loot may be anyway empty
                if (!IsLootedForAll())      // TODO:: implement empty windows? sWorld.getConfig(CONFIG_BOOL_CORPSE_EMPTY_LOOT_SHOW))
                {
                    creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                    break;
                }
            }

            sLog.outDebug("Loot::CreateLoot> cannot create corpse loot, FillLoot failed with loot id(%u)!", creatureInfo->LootId);
            creature->SetLootStatus(CREATURE_LOOT_STATUS_LOOTED);
            break;
        }
        case LOOT_PICKPOCKETING:
        {
            m_clientLootType = CLIENT_LOOT_PICKPOCKETING;

            if (!creature->isAlive() || player->getClass() != CLASS_ROGUE)
                return;

            if (!creatureInfo->LootId || !FillLoot(creatureInfo->PickpocketLootId, LootTemplates_Pickpocketing, player, false))
            {
                sLog.outError("Loot::CreateLoot> cannot create pickpocket loot, FillLoot failed with loot id(%u)!", creatureInfo->LootId);
                return;
            }

            // Generate extra money for pick pocket loot
            const uint32 a = urand(0, creature->getLevel() / 2);
            const uint32 b = urand(0, player->getLevel() / 2);
            m_gold = uint32(10 * (a + b) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));

            // setting loot right
            m_ownerSet.insert(player->GetObjectGuid());
            m_lootMethod = NOT_GROUP_TYPE_LOOT;
            break;
        }
        case LOOT_SKINNING:
        {
            m_clientLootType = CLIENT_LOOT_PICKPOCKETING;
            if (!creatureInfo->SkinningLootId || !FillLoot(creatureInfo->SkinningLootId, LootTemplates_Skinning, player, false))
            {
                sLog.outError("Loot::CreateLoot> cannot create skinning loot, FillLoot failed with loot id(%u)!", creatureInfo->SkinningLootId);
                return;
            }

            // setting loot right
            m_ownerSet.insert(player->GetObjectGuid());
            m_lootMethod = NOT_GROUP_TYPE_LOOT;
            break;
        }
        default:
            sLog.outError("Loot::CreateLoot> Cannot create loot for %s with invalid LootType(%u)", creature->GetGuidStr().c_str(), uint32(type));
            break;
    }

    return;
}

Loot::Loot(Player* player, GameObject* gameObject, LootType type) :
    m_lootTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(type),
    m_clientLootType(CLIENT_LOOT_CORPSE), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false),
    m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    // the player whose group may loot the corpse
    if (!player)
    {
        sLog.outError("LootMgr::CreateLoot> Error cannot get looter info to create loot!");
        return;
    }

    if (!gameObject)
    {
        sLog.outError("Loot::CreateLoot> cannot create game object loot, no game object passed!");
        return;
    }

    m_lootTarget = gameObject;
    m_guidTarget = gameObject->GetObjectGuid();

    // not check distance for GO in case owned GO (fishing bobber case, for example)
    // And permit out of range GO with no owner in case fishing hole
    if ((type != LOOT_FISHINGHOLE &&
        ((type != LOOT_FISHING && type != LOOT_FISHING_FAIL) || gameObject->GetOwnerGuid() != player->GetObjectGuid()) &&
        !gameObject->IsWithinDistInMap(player, INTERACTION_DISTANCE)))
    {
        sLog.outError("Loot::CreateLoot> cannot create game object loot, basic check failed!");
        return;
    }

    // generate loot only if ready for open and spawned in world
    if (gameObject->getLootState() == GO_READY && gameObject->isSpawned())
    {
        if ((gameObject->GetEntry() == BG_AV_OBJECTID_MINE_N || gameObject->GetEntry() == BG_AV_OBJECTID_MINE_S))
        {
            if (BattleGround* bg = player->GetBattleGround())
                if (bg->GetTypeID() == BATTLEGROUND_AV)
                    if (!(((BattleGroundAV*)bg)->PlayerCanDoMineQuest(gameObject->GetEntry(), player->GetTeam())))
                    {
                        return;
                    }
        }

        switch (type)
        {
            case LOOT_FISHING_FAIL:
            {
                // Entry 0 in fishing loot template used for store junk fish loot at fishing fail it junk allowed by config option
                // this is overwrite fishinghole loot for example
                FillLoot(0, LootTemplates_Fishing, player, true);

                // setting loot right
                m_ownerSet.insert(player->GetObjectGuid());
                m_lootMethod = NOT_GROUP_TYPE_LOOT;
                m_clientLootType = CLIENT_LOOT_FISHING;
                break;
            }
            case LOOT_FISHING:
            {
                uint32 zone, subzone;
                gameObject->GetZoneAndAreaId(zone, subzone);
                // if subzone loot exist use it
                if (!FillLoot(subzone, LootTemplates_Fishing, player, true, (subzone != zone)) && subzone != zone)
                    // else use zone loot (if zone diff. from subzone, must exist in like case)
                    FillLoot(zone, LootTemplates_Fishing, player, true);

                // setting loot right
                m_ownerSet.insert(player->GetObjectGuid());
                m_lootMethod = NOT_GROUP_TYPE_LOOT;
                m_clientLootType = CLIENT_LOOT_FISHING;
                break;
            }
            default:
            {
                if (uint32 lootid = gameObject->GetGOInfo()->GetLootId())
                {
                    if (gameObject->GetGOInfo()->type == GAMEOBJECT_TYPE_CHEST)
                        m_isChest = true;
                    else
                        SetGroupLootRight(player);

                    FillLoot(lootid, LootTemplates_Gameobject, player, false);
                    GenerateMoneyLoot(gameObject->GetGOInfo()->MinMoneyLoot, gameObject->GetGOInfo()->MaxMoneyLoot);

                    if (m_lootType == LOOT_FISHINGHOLE)
                        m_clientLootType = CLIENT_LOOT_FISHING;
                    else
                        m_clientLootType = CLIENT_LOOT_PICKPOCKETING;
                }
                break;
            }
        }
        gameObject->SetLootState(GO_ACTIVATED);
    }
    return;
}

Loot::Loot(Player* player, Corpse* corpse, LootType type) :
    m_lootTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(type),
    m_clientLootType(CLIENT_LOOT_CORPSE), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false),
    m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    // the player whose group may loot the corpse
    if (!player)
    {
        sLog.outError("LootMgr::CreateLoot> Error cannot get looter info to create loot!");
        return;
    }

    if (!corpse)
    {
        sLog.outError("Loot::CreateLoot> cannot create corpse loot, no corpse passed!");
        return;
    }

    m_lootTarget = corpse;
    m_guidTarget = corpse->GetObjectGuid();

    if (type != LOOT_INSIGNIA || corpse->GetType() == CORPSE_BONES)
        return;

    if (!corpse->lootForBody)
    {
        corpse->lootForBody = true;
        uint32 pLevel;
        if (Player* plr = sObjectAccessor.FindPlayer(corpse->GetOwnerGuid()))
            pLevel = plr->getLevel();
        else
            pLevel = player->getLevel(); // TODO:: not correct, need to save real player level in the corpse data in case of logout

        if (player->GetBattleGround()->GetTypeID() == BATTLEGROUND_AV)
            FillLoot(0, LootTemplates_Creature, player, false);
        // It may need a better formula
        // Now it works like this: lvl10: ~6copper, lvl70: ~9silver
        m_gold = (uint32)(urand(50, 150) * 0.016f * pow(((float)pLevel) / 5.76f, 2.5f) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
    }
    m_ownerSet.insert(player->GetObjectGuid());
    m_lootMethod = NOT_GROUP_TYPE_LOOT;
    m_clientLootType = CLIENT_LOOT_CORPSE;
    return;
}

Loot::Loot(Player* player, Item* item, LootType type) :
    m_lootTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(type),
    m_clientLootType(CLIENT_LOOT_CORPSE), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false),
    m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    // the player whose group may loot the corpse
    if (!player)
    {
        sLog.outError("LootMgr::CreateLoot> Error cannot get looter info to create loot!");
        return;
    }

    if (!item)
    {
        sLog.outError("Loot::CreateLoot> cannot create item loot, no item passed!");
        return;
    }

    m_itemTarget = item;
    m_guidTarget = item->GetObjectGuid();

    switch (type)
    {
        case LOOT_DISENCHANTING:
            FillLoot(item->GetProto()->DisenchantID, LootTemplates_Disenchant, player, true);
            item->SetLootState(ITEM_LOOT_TEMPORARY);
            break;
        default:
            FillLoot(item->GetEntry(), LootTemplates_Item, player, true, item->GetProto()->MaxMoneyLoot == 0);
            GenerateMoneyLoot(item->GetProto()->MinMoneyLoot, item->GetProto()->MaxMoneyLoot);
            item->SetLootState(ITEM_LOOT_CHANGED);
            break;
    }
    m_ownerSet.insert(player->GetObjectGuid());
    m_lootMethod = NOT_GROUP_TYPE_LOOT;
    m_clientLootType = CLIENT_LOOT_PICKPOCKETING;
    return;
}

Loot::Loot(Unit* unit, Item* item) :
    m_lootTarget(nullptr), m_itemTarget(item), m_gold(0), m_maxSlot(0),
    m_lootType(LOOT_SKINNING), m_clientLootType(CLIENT_LOOT_PICKPOCKETING), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false),
    m_haveItemOverThreshold(false), m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    m_ownerSet.insert(unit->GetObjectGuid());
    m_guidTarget = item->GetObjectGuid();
}

Loot::Loot(Player* player, uint32 id, LootType type) :
    m_lootTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(type),
    m_clientLootType(CLIENT_LOOT_CORPSE), m_lootMethod(NOT_GROUP_TYPE_LOOT), m_threshold(ITEM_QUALITY_UNCOMMON), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false),
    m_isChecked(false), m_isChest(false), m_isChanged(false)
{
    switch (type)
    {
        case LOOT_MAIL:
            FillLoot(id, LootTemplates_Mail, player, true, true);
            m_clientLootType = CLIENT_LOOT_PICKPOCKETING;
            break;
        case LOOT_SKINNING:
            FillLoot(id, LootTemplates_Skinning, player, true, true);
            m_clientLootType = CLIENT_LOOT_PICKPOCKETING;
            break;
        default:
            sLog.outError("Loot::Loot> invalid loot type passed to loot constructor.");
            break;
    }
}

void Loot::SendAllowedLooter()
{
    if (m_lootMethod == FREE_FOR_ALL || m_lootMethod == NOT_GROUP_TYPE_LOOT)
        return;

    WorldPacket data(SMSG_LOOT_LIST);
    data << GetLootGuid();

    if (m_lootMethod == MASTER_LOOT)
        data << m_masterOwnerGuid.WriteAsPacked();
    else
        data << uint8(0);

    data << m_currentLooterGuid.WriteAsPacked();

    for (GuidSet::const_iterator itr = m_ownerSet.begin(); itr != m_ownerSet.end(); ++itr)
        if (Player* plr = ObjectAccessor::FindPlayer(*itr))
            plr->GetSession()->SendPacket(&data);
}

InventoryResult Loot::SendItem(Player* target, uint32 itemSlot)
{
    LootItem* lootItem = GetLootItemInSlot(itemSlot);
    return SendItem(target, lootItem);
}

InventoryResult Loot::SendItem(Player* target, LootItem* lootItem)
{
    if (!lootItem)
        return EQUIP_ERR_ITEM_NOT_FOUND;

    bool playerGotItem = false;
    InventoryResult msg;
    if (target && target->GetSession())
    {
        ItemPosCountVec dest;
        msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, lootItem->itemId, lootItem->count);
        if (msg == EQUIP_ERR_OK)
        {
            Item* newItem = target->StoreNewItem(dest, lootItem->itemId, true, lootItem->randomPropertyId);

            if (lootItem->freeForAll)
            {
                NotifyItemRemoved(target, lootItem->lootSlot);
                sLog.outDebug("This item is free for all!!");
            }
            else
                NotifyItemRemoved(lootItem->lootSlot);

            target->SendNewItem(newItem, uint32(lootItem->count), false, false, true);

            lootItem->lootedBy.insert(target->GetObjectGuid());     // mark looted by this target

            playerGotItem = true;
            m_isChanged = true;
        }
        else
            target->SendEquipError(msg, nullptr, nullptr, lootItem->itemId);
    }

    if (!playerGotItem)
    {
        // an error occurred player didn't received his loot
        lootItem->isBlocked = false;                                  // make the item available (was blocked since roll started)
        m_currentLooterGuid = target->GetObjectGuid();                // change looter guid to let only him right to loot
        m_isReleased = false;                                         // be sure the loot was not already released by another player
        SendAllowedLooter();                                          // update the looter right for client
    }
    else
    {
        if (IsLootedForAll())
        {
            SendReleaseForAll();
            if (m_isChest)
            {
                GameObject* go = (GameObject*)m_lootTarget;
                uint32 go_min = go->GetGOInfo()->chest.minSuccessOpens;
                uint32 go_max = go->GetGOInfo()->chest.maxSuccessOpens;
                // only vein pass this check
                if (go_min != 0 && go_max > go_min)
                {
                    // nothing to do refill is handled in Loot::Release()
                }
                else
                    go->SetLootState(GO_JUST_DEACTIVATED);
            }
        }
        else
            if (IsLootedFor(target))
                SendReleaseFor(target);
        ForceLootAnimationCLientUpdate();
    }
    return msg;
}

bool Loot::AutoStore(Player* player, bool broadcast /*= false*/, uint32 bag /*= NULL_BAG*/, uint32 slot /*= NULL_SLOT*/)
{
    bool result = true;
    for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;
        if (!lootItem->AllowedForPlayer(player, m_lootTarget))
            continue; // player have no right to see/loot this item

        if (!lootItem->lootedBy.empty())
            continue; // already looted

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(bag, slot, dest, lootItem->itemId, lootItem->count);
        if (msg != EQUIP_ERR_OK && slot != NULL_SLOT)
            msg = player->CanStoreNewItem(bag, NULL_SLOT, dest, lootItem->itemId, lootItem->count);
        if (msg != EQUIP_ERR_OK && bag != NULL_BAG)
            msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, lootItem->itemId, lootItem->count);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, nullptr, nullptr, lootItem->itemId);
            result = false;
            continue;
        }

        lootItem->lootedBy.insert(player->GetObjectGuid());
        Item* pItem = player->StoreNewItem(dest, lootItem->itemId, true, lootItem->randomPropertyId);
        player->SendNewItem(pItem, lootItem->count, false, false, broadcast);
        m_isChanged = true;
    }

    return result;
}

void Loot::Update()
{
    m_isChanged = false;
    GroupLootRollMap::iterator itr = m_roll.begin();
    while (itr != m_roll.end())
    {
        if (itr->second.UpdateRoll())
            m_roll.erase(itr++);
        else
            ++itr;
    }
}

void Loot::ForceLootAnimationCLientUpdate()
{
    if (m_guidTarget.IsCreature() && m_lootTarget)
        m_lootTarget->ForceValuesUpdateAtIndex(UNIT_DYNAMIC_FLAGS);
}

// will return the pointer of item in loot slot provided without any right check
LootItem* Loot::GetLootItemInSlot(uint32 itemSlot)
{
    for (LootItemList::iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;
        if (lootItem->lootSlot == itemSlot)
            return lootItem;
    }
    return nullptr;
}

// Will return available loot item for specific player. Use only for own loot like loot in item and mail
void Loot::GetLootItemsListFor(Player* player, LootItemList& lootList)
{
    for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;
        if (!lootItem->AllowedForPlayer(player, m_lootTarget))
            continue; // player have no right to see/loot this item

        if (!lootItem->lootedBy.empty())
            continue; // already looted

        lootList.push_back(lootItem);
    }
}

Loot::~Loot()
{
    SendReleaseForAll();
    for (LootItemList::iterator itr = m_lootItems.begin(); itr != m_lootItems.end(); ++itr)
        delete *itr;
}

void Loot::Clear()
{
    for (LootItemList::iterator itr = m_lootItems.begin(); itr != m_lootItems.end(); ++itr)
        delete *itr;
    m_lootItems.clear();
    m_playersLooting.clear();
    m_gold = 0;
    m_ownerSet.clear();
    m_masterOwnerGuid.Clear();
    m_currentLooterGuid.Clear();
    m_roll.clear();
    m_maxEnchantSkill = 0;
    m_isReleased = false;
    m_haveItemOverThreshold = false;
    m_isChecked = false;
    m_maxSlot = 0;
}

// only used from explicitly loaded loot
void Loot::SetGoldAmount(uint32 _gold)
{
    if (m_lootType == LOOT_SKINNING)
        m_gold = _gold;
}

void Loot::SendGold(Player* player)
{
    NotifyMoneyRemoved();

    if (m_lootMethod != NOT_GROUP_TYPE_LOOT)           // item can be looted only single player
    {
        uint32 money_per_player = uint32(m_gold / (m_ownerSet.size()));

        for (GuidSet::const_iterator itr = m_ownerSet.begin(); itr != m_ownerSet.end(); ++itr)
        {
            Player* plr = sObjectMgr.GetPlayer(*itr);
            if (!plr || !plr->GetSession())
                continue;

            plr->ModifyMoney(money_per_player);

            WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4);
            data << uint32(money_per_player);

            plr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        player->ModifyMoney(m_gold);

        if (m_guidTarget.IsItem())
        {
            if (Item* item = player->GetItemByGuid(m_guidTarget))
                item->SetLootState(ITEM_LOOT_CHANGED);
        }
    }
    m_gold = 0;

    if (IsLootedFor(player))
        Release(player);
    ForceLootAnimationCLientUpdate();
}

bool Loot::IsItemAlreadyIn(uint32 itemId) const
{
    for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;
        if (lootItem->itemId == itemId)
            return true;
    }
    return false;
}

// fill in the bytebuffer with loot content for specified player
void Loot::GetLootContentFor(Player* player, ByteBuffer& buffer)
{
    uint8 itemsShown = 0;

    // gold
    buffer << uint32(m_gold);

    size_t count_pos = buffer.wpos();                            // pos of item count byte
    buffer << uint8(0);                                          // item count placeholder

    for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
    {
        LootItem* lootItem = *lootItemItr;
        LootSlotType slot_type = lootItem->GetSlotTypeForSharedLoot(player, this);
        if (slot_type >= MAX_LOOT_SLOT_TYPE)
        {
            sLog.outDebug("Item not visible for %s> itemid(%u) in slot (%u)!", player->GetGuidStr().c_str(), lootItem->itemId, uint32(lootItem->lootSlot));
            continue;
        }

        buffer << uint8(lootItem->lootSlot);
        buffer << *lootItem;
        buffer << uint8(slot_type);                              // 0 - get 1 - look only 2 - master selection
        ++itemsShown;

        sLog.outDebug("Sending loot to %s> itemid(%u) in slot (%u)!", player->GetGuidStr().c_str(), lootItem->itemId, uint32(lootItem->lootSlot));
    }

    // update number of items shown
    buffer.put<uint8>(count_pos, itemsShown);
}

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li)
{
    b << uint32(li.itemId);
    b << uint32(li.count);                                  // nr of items of this type
    b << uint32(ObjectMgr::GetItemPrototype(li.itemId)->DisplayInfoID);
    b << uint32(0);
    b << uint32(li.randomPropertyId);
    return b;
}

//
// --------- LootTemplate::LootGroup ---------
//

// Adds an entry to the group (at loading stage)
void LootTemplate::LootGroup::AddEntry(LootStoreItem& item)
{
    if (item.chance != 0)
        ExplicitlyChanced.push_back(item);
    else
        EqualChanced.push_back(item);
}

// Rolls an item from the group, returns nullptr if all miss their chances
LootStoreItem const* LootTemplate::LootGroup::Roll(Loot const& loot, Player const* lootOwner) const
{
    if (!ExplicitlyChanced.empty())                         // First explicitly chanced entries are checked
    {
        std::vector <LootStoreItem const*> lootStoreItemVector; // we'll use new vector to make easy the randomization

        // fill the new vector with correct pointer to our item list
        for (LootStoreItemList::const_iterator itr = ExplicitlyChanced.begin(); itr != ExplicitlyChanced.end(); ++itr)
            lootStoreItemVector.push_back(&(*itr));

        // randomize the new vector
        random_shuffle(lootStoreItemVector.begin(), lootStoreItemVector.end());

        float chance = rand_chance_f();

        // as the new vector is randomized we can start from first element and stop at first one that meet the condition
        for (std::vector <LootStoreItem const*>::const_iterator itr = lootStoreItemVector.begin(); itr != lootStoreItemVector.end(); ++itr)
        {
            LootStoreItem const* lsi = *itr;

            if (lsi->conditionId && !sObjectMgr.IsPlayerMeetToCondition(lsi->conditionId, lootOwner, lootOwner->GetMap(), loot.GetLootTarget(), CONDITION_FROM_REFERING_LOOT))
            {
                sLog.outDebug("In explicit chance -> This item cannot be added! (%u)", lsi->itemid);
                continue;
            }

            if (lsi->chance >= 100.0f)
                return lsi;

            chance -= lsi->chance;
            if (chance < 0)
                return lsi;
        }
    }

    if (!EqualChanced.empty())                              // If nothing selected yet - an item is taken from equal-chanced part
    {
        std::vector <LootStoreItem const*> lootStoreItemVector; // we'll use new vector to make easy the randomization

        // fill the new vector with correct pointer to our item list
        for (LootStoreItemList::const_iterator itr = EqualChanced.begin(); itr != EqualChanced.end(); ++itr)
            lootStoreItemVector.push_back(&(*itr));

        // randomize the new vector
        random_shuffle(lootStoreItemVector.begin(), lootStoreItemVector.end());

        // as the new vector is randomized we can start from first element and stop at first one that meet the condition
        for (std::vector <LootStoreItem const*>::const_iterator itr = lootStoreItemVector.begin(); itr != lootStoreItemVector.end(); ++itr)
        {
            LootStoreItem const* lsi = *itr;

            //check if we already have that item in the loot list
            if (loot.IsItemAlreadyIn(lsi->itemid))
            {
                // the item is already looted, let's give a 50%  chance to pick another one
                uint32 chance = urand(0,1);

                if (chance)
                    continue;                               // pass this item
            }

            if (lsi->conditionId && !sObjectMgr.IsPlayerMeetToCondition(lsi->conditionId, lootOwner, lootOwner->GetMap(), loot.GetLootTarget(), CONDITION_FROM_REFERING_LOOT))
            {
                sLog.outDebug("In equal chance -> This item cannot be added! (%u)", lsi->itemid);
                continue;
            }
            return lsi;
        }
    }

    return nullptr;                                            // Empty drop from the group
}

// True if group includes at least 1 quest drop entry
bool LootTemplate::LootGroup::HasQuestDrop() const
{
    for (LootStoreItemList::const_iterator i = ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (i->needs_quest)
            return true;
    for (LootStoreItemList::const_iterator i = EqualChanced.begin(); i != EqualChanced.end(); ++i)
        if (i->needs_quest)
            return true;
    return false;
}

// True if group includes at least 1 quest drop entry for active quests of the player
bool LootTemplate::LootGroup::HasQuestDropForPlayer(Player const* player) const
{
    for (LootStoreItemList::const_iterator i = ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (player->HasQuestForItem(i->itemid))
            return true;
    for (LootStoreItemList::const_iterator i = EqualChanced.begin(); i != EqualChanced.end(); ++i)
        if (player->HasQuestForItem(i->itemid))
            return true;
    return false;
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
void LootTemplate::LootGroup::Process(Loot& loot, Player const* lootOwner) const
{
    LootStoreItem const* item = Roll(loot, lootOwner);
    if (item != nullptr)
        loot.AddItem(*item);
}

// Overall chance for the group without equal chanced items
float LootTemplate::LootGroup::RawTotalChance() const
{
    float result = 0;

    for (LootStoreItemList::const_iterator i = ExplicitlyChanced.begin(); i != ExplicitlyChanced.end(); ++i)
        if (!i->needs_quest)
            result += i->chance;

    return result;
}

// Overall chance for the group
float LootTemplate::LootGroup::TotalChance() const
{
    float result = RawTotalChance();

    if (!EqualChanced.empty() && result < 100.0f)
        return 100.0f;

    return result;
}

void LootTemplate::LootGroup::Verify(LootStore const& lootstore, uint32 id, uint32 group_id) const
{
    float chance = RawTotalChance();
    if (chance > 101.0f)                                    // TODO: replace with 100% when DBs will be ready
    {
        sLog.outErrorDb("Table '%s' entry %u group %d has total chance > 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }

    if (chance >= 100.0f && !EqualChanced.empty())
    {
        sLog.outErrorDb("Table '%s' entry %u group %d has items with chance=0%% but group total chance >= 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }
}

void LootTemplate::LootGroup::CheckLootRefs(LootIdSet* ref_set) const
{
    for (LootStoreItemList::const_iterator ieItr = ExplicitlyChanced.begin(); ieItr != ExplicitlyChanced.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }

    for (LootStoreItemList::const_iterator ieItr = EqualChanced.begin(); ieItr != EqualChanced.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }
}

//
// --------- LootTemplate ---------
//

// Adds an entry to the group (at loading stage)
void LootTemplate::AddEntry(LootStoreItem& item)
{
    if (item.group > 0 && item.mincountOrRef > 0)           // Group
    {
        if (item.group >= Groups.size())
            Groups.resize(item.group);                      // Adds new group the the loot template if needed
        Groups[item.group - 1].AddEntry(item);              // Adds new entry to the group
    }
    else                                                    // Non-grouped entries and references are stored together
        Entries.push_back(item);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::Process(Loot& loot, Player const* lootOwner, LootStore const& store, bool rate, uint8 groupId) const
{
    if (groupId)                                            // Group reference uses own processing of the group
    {
        if (groupId > Groups.size())
            return;                                         // Error message already printed at loading stage

        Groups[groupId - 1].Process(loot, lootOwner);
        return;
    }

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin() ; i != Entries.end() ; ++i)
    {
        if (!i->Roll(rate))
            continue;                                       // Bad luck for the entry

        if (i->mincountOrRef < 0)                           // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-i->mincountOrRef);

            if (!Referenced)
                continue;                                   // Error message already printed at loading stage

            // Check condition
            if (i->conditionId && !sObjectMgr.IsPlayerMeetToCondition(i->conditionId, nullptr, nullptr, loot.GetLootTarget(), CONDITION_FROM_REFERING_LOOT))
                continue;

            for (uint32 loop = 0; loop < i->maxcount; ++loop) // Ref multiplicator
                Referenced->Process(loot, lootOwner, store, rate, i->group);
        }
        else                                                // Plain entries (not a reference, not grouped)
            loot.AddItem(*i);                               // Chance is already checked, just add
    }

    // Now processing groups
    for (LootGroups::const_iterator i = Groups.begin() ; i != Groups.end() ; ++i)
        i->Process(loot, lootOwner);
}

// True if template includes at least 1 quest drop entry
bool LootTemplate::HasQuestDrop(LootTemplateMap const& store, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message [should be] already printed at loading stage
        return Groups[groupId - 1].HasQuestDrop();
    }

    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->mincountOrRef < 0)                           // References
        {
            LootTemplateMap::const_iterator Referenced = store.find(-i->mincountOrRef);
            if (Referenced == store.end())
                continue;                                   // Error message [should be] already printed at loading stage
            if (Referenced->second->HasQuestDrop(store, i->group))
                return true;
        }
        else if (i->needs_quest)
            return true;                                    // quest drop found
    }

    // Now processing groups
    for (LootGroups::const_iterator i = Groups.begin() ; i != Groups.end() ; ++i)
        if (i->HasQuestDrop())
            return true;

    return false;
}

// True if template includes at least 1 quest drop for an active quest of the player
bool LootTemplate::HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message already printed at loading stage
        return Groups[groupId - 1].HasQuestDropForPlayer(player);
    }

    // Checking non-grouped entries
    for (LootStoreItemList::const_iterator i = Entries.begin() ; i != Entries.end() ; ++i)
    {
        if (i->mincountOrRef < 0)                           // References processing
        {
            LootTemplateMap::const_iterator Referenced = store.find(-i->mincountOrRef);
            if (Referenced == store.end())
                continue;                                   // Error message already printed at loading stage
            if (Referenced->second->HasQuestDropForPlayer(store, player, i->group))
                return true;
        }
        else if (player->HasQuestForItem(i->itemid))
            return true;                                    // active quest drop found
    }

    // Now checking groups
    for (LootGroups::const_iterator i = Groups.begin(); i != Groups.end(); ++i)
        if (i->HasQuestDropForPlayer(player))
            return true;

    return false;
}

// Checks integrity of the template
void LootTemplate::Verify(LootStore const& lootstore, uint32 id) const
{
    // Checking group chances
    for (uint32 i = 0; i < Groups.size(); ++i)
        Groups[i].Verify(lootstore, id, i + 1);

    // TODO: References validity checks
}

void LootTemplate::CheckLootRefs(LootIdSet* ref_set) const
{
    for (LootStoreItemList::const_iterator ieItr = Entries.begin(); ieItr != Entries.end(); ++ieItr)
    {
        if (ieItr->mincountOrRef < 0)
        {
            if (!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if (ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }

    for (LootGroups::const_iterator grItr = Groups.begin(); grItr != Groups.end(); ++grItr)
        grItr->CheckLootRefs(ref_set);
}

void LoadLootTemplates_Creature()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Creature.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sCreatureStorage.GetMaxEntry(); ++i)
    {
        if (CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i))
        {
            if (uint32 lootid = cInfo->LootId)
            {
                if (ids_set.find(lootid) == ids_set.end())
                    LootTemplates_Creature.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for (LootIdSet::const_iterator itr = ids_setUsed.begin(); itr != ids_setUsed.end(); ++itr)
        ids_set.erase(*itr);

    // for alterac valley we've defined Player-loot inside creature_loot_template id=0
    // this hack is used, so that we won't need to create an extra table player_loot_template for just one case
    ids_set.erase(0);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Creature.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Disenchant()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Disenchant.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sItemStorage.GetMaxEntry(); ++i)
    {
        if (ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(i))
        {
            if (uint32 lootid = proto->DisenchantID)
            {
                if (ids_set.find(lootid) == ids_set.end())
                    LootTemplates_Disenchant.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for (LootIdSet::const_iterator itr = ids_setUsed.begin(); itr != ids_setUsed.end(); ++itr)
        ids_set.erase(*itr);
    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Disenchant.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Fishing()
{
    LootIdSet ids_set;
    LootTemplates_Fishing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sAreaStore.GetNumRows(); ++i)
    {
        if (AreaTableEntry const* areaEntry = sAreaStore.LookupEntry(i))
            if (ids_set.find(areaEntry->ID) != ids_set.end())
                ids_set.erase(areaEntry->ID);
    }

    // by default (look config options) fishing at fail provide junk loot, entry 0 use for store this loot
    ids_set.erase(0);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Fishing.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Gameobject()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Gameobject.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (SQLStorageBase::SQLSIterator<GameObjectInfo> itr = sGOStorage.getDataBegin<GameObjectInfo>(); itr < sGOStorage.getDataEnd<GameObjectInfo>(); ++itr)
    {
        if (uint32 lootid = itr->GetLootId())
        {
            if (ids_set.find(lootid) == ids_set.end())
                LootTemplates_Gameobject.ReportNotExistedId(lootid);
            else
                ids_setUsed.insert(lootid);
        }
    }
    for (LootIdSet::const_iterator itr = ids_setUsed.begin(); itr != ids_setUsed.end(); ++itr)
        ids_set.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Gameobject.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Item()
{
    LootIdSet ids_set;
    LootTemplates_Item.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sItemStorage.GetMaxEntry(); ++i)
    {
        if (ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(i))
        {
            if (!(proto->Flags & ITEM_FLAG_LOOTABLE))
                continue;

            if (ids_set.find(proto->ItemId) != ids_set.end() || proto->MaxMoneyLoot > 0)
                ids_set.erase(proto->ItemId);
            // wdb have wrong data cases, so skip by default
            else if (!sLog.HasLogFilter(LOG_FILTER_DB_STRICTED_CHECK))
                LootTemplates_Item.ReportNotExistedId(proto->ItemId);
        }
    }

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Item.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Pickpocketing()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Pickpocketing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sCreatureStorage.GetMaxEntry(); ++i)
    {
        if (CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i))
        {
            if (uint32 lootid = cInfo->PickpocketLootId)
            {
                if (ids_set.find(lootid) == ids_set.end())
                    LootTemplates_Pickpocketing.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for (LootIdSet::const_iterator itr = ids_setUsed.begin(); itr != ids_setUsed.end(); ++itr)
        ids_set.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Pickpocketing.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Mail()
{
    LootIdSet ids_set;
    LootTemplates_Mail.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sMailTemplateStore.GetNumRows(); ++i)
        if (sMailTemplateStore.LookupEntry(i))
            if (ids_set.find(i) != ids_set.end())
                ids_set.erase(i);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Mail.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Skinning()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Skinning.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for (uint32 i = 1; i < sCreatureStorage.GetMaxEntry(); ++i)
    {
        if (CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i))
        {
            if (uint32 lootid = cInfo->SkinningLootId)
            {
                if (ids_set.find(lootid) == ids_set.end())
                    LootTemplates_Skinning.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for (LootIdSet::const_iterator itr = ids_setUsed.begin(); itr != ids_setUsed.end(); ++itr)
        ids_set.erase(*itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Skinning.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Reference()
{
    LootIdSet ids_set;
    LootTemplates_Reference.LoadAndCollectLootIds(ids_set);

    // check references and remove used
    LootTemplates_Creature.CheckLootRefs(&ids_set);
    LootTemplates_Fishing.CheckLootRefs(&ids_set);
    LootTemplates_Gameobject.CheckLootRefs(&ids_set);
    LootTemplates_Item.CheckLootRefs(&ids_set);
    LootTemplates_Pickpocketing.CheckLootRefs(&ids_set);
    LootTemplates_Skinning.CheckLootRefs(&ids_set);
    LootTemplates_Disenchant.CheckLootRefs(&ids_set);
    LootTemplates_Mail.CheckLootRefs(&ids_set);
    LootTemplates_Reference.CheckLootRefs(&ids_set);

    // output error for any still listed ids (not referenced from any loot table)
    LootTemplates_Reference.ReportUnusedIds(ids_set);
}

// Vote for an ongoing roll
void LootMgr::PlayerVote(Player* player, ObjectGuid const& lootTargetGuid, uint32 itemSlot, RollVote vote)
{
    Loot* loot = GetLoot(player, lootTargetGuid);

    if (!loot)
    {
        sLog.outError("LootMgr::PlayerVote> Error cannot get loot object info!");
        return;
    }

    GroupLootRoll* roll = loot->GetRollForSlot(itemSlot);
    if (!roll)
    {
        sLog.outError("LootMgr::PlayerVote> Invalid itemSlot!");
        return;
    }

    roll->PlayerVote(player, vote);
}

// Get loot by object guid
// If target guid is not provided, try to find it by recipient or current player target
Loot* LootMgr::GetLoot(Player* player, ObjectGuid const& targetGuid)
{
    Loot* loot = nullptr;
    ObjectGuid lguid;
    if (targetGuid.IsEmpty())
    {
        lguid = player->GetLootGuid();

        if (lguid.IsEmpty())
        {
            lguid = player->GetTargetGuid();
            if (lguid.IsEmpty())
                return nullptr;
        }
    }
    else
        lguid = targetGuid;

    switch (lguid.GetHigh())
    {
        case HIGHGUID_GAMEOBJECT:
        {
            GameObject* gob = player->GetMap()->GetGameObject(lguid);

            // not check distance for GO in case owned GO (fishing bobber case, for example)
            if (gob)
                loot = gob->loot;

            break;
        }
        case HIGHGUID_CORPSE:                               // remove insignia ONLY in BG
        {
            Corpse* bones = player->GetMap()->GetCorpse(lguid);

            if (bones)
                loot = bones->loot;

            break;
        }
        case HIGHGUID_ITEM:
        {
            Item* item = player->GetItemByGuid(lguid);
            if (item && item->HasGeneratedLoot())
                loot = item->loot;
            break;
        }
        case HIGHGUID_UNIT:
        {
            Creature* creature = player->GetMap()->GetCreature(lguid);

            if (creature)
                loot = creature->loot;

            break;
        }
        default:
            return nullptr;                                         // unlootable type
    }

    return loot;
}

bool LootMgr::IsAllowedToLoot(Player* player, Creature* creature)
{
    // never tapped by any (mob solo kill)
    if (!creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED))
        return false;

    bool canLoot = false;
    if (Loot* loot = creature->loot)
        canLoot = loot->CanLoot(player);

    return canLoot;
}
