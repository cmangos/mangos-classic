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

#ifndef MANGOS_LOOTMGR_H
#define MANGOS_LOOTMGR_H

#include "ByteBuffer.h"
#include "ObjectGuid.h"
#include "SharedDefines.h"

#include <vector>
#include "Bag.h"

#define LOOT_ROLL_TIMEOUT  (1*MINUTE*IN_MILLISECONDS)

class Player;
class Group;
class LootStore;
class WorldObject;
class LootTemplate;
class Loot;
struct LootItem;
struct ItemPrototype;


#define MAX_NR_LOOT_ITEMS 16
// note: the client cannot show more than 16 items total

enum PermissionTypes
{
    ALL_PERMISSION    = 0,
    GROUP_PERMISSION  = 1,
    MASTER_PERMISSION = 2,
    OWNER_PERMISSION  = 3,                                  // for single player only loots
    NONE_PERMISSION   = 4
};

enum LootSlotType
{
    LOOT_SLOT_NORMAL  = 0,                                  // can be looted
    LOOT_SLOT_VIEW    = 1,                                  // can be only view (ignore any loot attempts)
    LOOT_SLOT_MASTER  = 2,                                  // can be looted only master (error message)
    LOOT_SLOT_REQS    = 3,                                  // can't be looted (error message about missing reqs)
    MAX_LOOT_SLOT_TYPE                                      // custom, use for mark skipped from show items
};

enum RollVote
{
    ROLL_PASS = 0,
    ROLL_NEED = 1,
    ROLL_GREED = 2,
    ROLL_DISENCHANT = 3,
    ROLL_NOT_EMITED_YET = 4,                             // send to client
    ROLL_NOT_VALID = 5                                   // not send to client
};

// set what votes allowed
enum RollVoteMask
{
    ROLL_VOTE_MASK_PASS = 0x01,
    ROLL_VOTE_MASK_NEED = 0x02,
    ROLL_VOTE_MASK_GREED = 0x04,
    ROLL_VOTE_MASK_DISENCHANT = 0x08,

    ROLL_VOTE_MASK_ALL = 0x0F,
};

enum LootItemType
{
    LOOTITEM_TYPE_NORMAL        = 1,
    LOOTITEM_TYPE_QUEST         = 2,
    LOOTITEM_TYPE_CONDITIONNAL  = 3
};

// loot type sent to clients
enum ClientLootType
{
    CLIENT_LOOT_CORPSE          = 1,
    CLIENT_LOOT_PICKPOCKETING   = 2,
    CLIENT_LOOT_FISHING         = 3,
    CLIENT_LOOT_DISENCHANTING   = 4
};

enum LootStatus
{
    LOOT_STATUS_NOT_FULLY_LOOTED = 0x01,
    LOOT_STATUS_CONTAIN_FFA      = 0x02,
    LOOT_STATUS_CONTAIN_GOLD     = 0x04
};

struct PlayerRollVote
{
    PlayerRollVote() : vote(ROLL_NOT_VALID), number(0) {}
    RollVote vote;
    uint8    number;
};

class GroupLootRoll
{
public:
    typedef std::unordered_map<ObjectGuid, PlayerRollVote> RollVoteMap;

    GroupLootRoll() : m_rollVoteMap(ROLL_VOTE_MASK_ALL), m_isStarted(false), m_lootItem(nullptr), m_loot(nullptr), m_itemSlot(0), m_voteMask(), m_endTime(0)
    {}
    ~GroupLootRoll();

    bool TryToStart(Loot& loot, uint32 itemSlot);
    bool PlayerVote(Player* playerGuid, RollVote vote);
    bool UpdateRoll();

private:
    void SendStartRoll();
    void SendAllPassed();
    void SendRoll(ObjectGuid const& targetGuid, uint32 rollNumber, uint32 rollType);
    void SendLootRollWon(ObjectGuid const& targetGuid, uint32 rollNumber, RollVote rollType);
    void Finish(RollVoteMap::const_iterator& winnerItr);
    bool AllPlayerVoted(RollVoteMap::const_iterator& winnerItr);
    RollVoteMap           m_rollVoteMap;
    bool                  m_isStarted;
    LootItem*             m_lootItem;
    Loot*                 m_loot;
    uint32                m_itemSlot;
    RollVoteMask          m_voteMask;
    time_t                m_endTime;
};
typedef std::unordered_map<uint32, GroupLootRoll> GroupLootRollMap;

struct LootStoreItem
{
    uint32  itemid;                                         // id of the item
    float   chance;                                         // always positive, chance to drop for both quest and non-quest items, chance to be used for refs
    int32   mincountOrRef;                                  // mincount for drop items (positive) or minus referenced TemplateleId (negative)
    uint8   group       : 7;
    bool    needs_quest : 1;                                // quest drop (negative ChanceOrQuestChance in DB)
    uint8   maxcount    : 8;                                // max drop count for the item (mincountOrRef positive) or Ref multiplicator (mincountOrRef negative)
    uint16  conditionId : 16;                               // additional loot condition Id

    // Constructor, converting ChanceOrQuestChance -> (chance, needs_quest)
    // displayid is filled in IsValid() which must be called after
    LootStoreItem(uint32 _itemid, float _chanceOrQuestChance, int8 _group, uint16 _conditionId, int32 _mincountOrRef, uint8 _maxcount)
        : itemid(_itemid), chance(fabs(_chanceOrQuestChance)), mincountOrRef(_mincountOrRef),
          group(_group), needs_quest(_chanceOrQuestChance < 0), maxcount(_maxcount), conditionId(_conditionId)
    {}

    bool Roll(bool rate) const;                             // Checks if the entry takes it's chance (at loot generation)
    bool IsValid(LootStore const& store, uint32 entry) const;
    // Checks correctness of values
};

struct LootItem
{
    uint32       itemId;
    uint32       randomSuffix;
    int32        randomPropertyId;
    uint32       displayID;
    LootItemType lootItemType;
    GuidSet      lootedBy;                                          // player's guid who looted this item
    uint32       lootSlot;                                          // the slot number will be send to client
    uint16       conditionId       : 16;                            // allow compiler pack structure
    uint8        count             : 8;
    bool         isBlocked         : 1;
    bool         freeForAll        : 1;                             // free for all
    bool         isUnderThreshold  : 1;
    bool         currentLooterPass : 1;
    bool         isNotVisibleForML : 1;                             // true when in master loot the leader do not have the condition to see the item
    bool         checkRollNeed     : 1;                             // true if for this item we need to check if roll is needed

    // storing item prototype for fast access
    ItemPrototype const* itemProto;

    // Constructor, copies most fields from LootStoreItem, generates random count and random suffixes/properties
    // Should be called for non-reference LootStoreItem entries only (mincountOrRef > 0)
    explicit LootItem(LootStoreItem const& li, uint32 _lootSlot, uint32 threshold);

    LootItem(uint32 _itemid, uint32 _count, uint32 _randomSuffix, int32 _randomPropertyId, uint32 _lootSlot);

    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const* player, WorldObject const* lootTarget) const;
    LootSlotType GetSlotTypeForSharedLoot(Player const* player, Loot const* loot) const;
    bool IsLootedFor(ObjectGuid const& playerGuid) const { return lootedBy.find(playerGuid) != lootedBy.end(); }
};

typedef std::vector<LootItem*> LootItemList;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef std::unordered_map<uint32, LootTemplate*> LootTemplateMap;
typedef std::set<uint32> LootIdSet;

class LootStore
{
    public:
        explicit LootStore(char const* name, char const* entryName, bool ratesAllowed)
            : m_name(name), m_entryName(entryName), m_ratesAllowed(ratesAllowed) {}
        virtual ~LootStore() { Clear(); }

        void Verify() const;

        void LoadAndCollectLootIds(LootIdSet& ids_set);
        void CheckLootRefs(LootIdSet* ref_set = nullptr) const; // check existence reference and remove it from ref_set
        void ReportUnusedIds(LootIdSet const& ids_set) const;
        void ReportNotExistedId(uint32 id) const;

        bool HaveLootFor(uint32 loot_id) const { return m_LootTemplates.find(loot_id) != m_LootTemplates.end(); }
        bool HaveQuestLootFor(uint32 loot_id) const;
        bool HaveQuestLootForPlayer(uint32 loot_id, Player* player) const;

        LootTemplate const* GetLootFor(uint32 loot_id) const;

        char const* GetName() const { return m_name; }
        char const* GetEntryName() const { return m_entryName; }
        bool IsRatesAllowed() const { return m_ratesAllowed; }
    protected:
        void LoadLootTable();
        void Clear();
    private:
        LootTemplateMap m_LootTemplates;
        char const* m_name;
        char const* m_entryName;
        bool m_ratesAllowed;
};

class LootTemplate
{
        class  LootGroup;                                   // A set of loot definitions for items (refs are not allowed inside)
        typedef std::vector<LootGroup> LootGroups;

    public:
        // Adds an entry to the group (at loading stage)
        void AddEntry(LootStoreItem& item);
        // Rolls for every item in the template and adds the rolled items the the loot
        void Process(Loot& loot, Player const* lootOwner, LootStore const& store, bool rate, uint8 GroupId = 0) const;

        // True if template includes at least 1 quest drop entry
        bool HasQuestDrop(LootTemplateMap const& store, uint8 GroupId = 0) const;
        // True if template includes at least 1 quest drop for an active quest of the player
        bool HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 GroupId = 0) const;

        // Checks integrity of the template
        void Verify(LootStore const& store, uint32 Id) const;
        void CheckLootRefs(LootIdSet* ref_set) const;
    private:
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimized) processing, grouped entries go there
};

//=====================================================

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li);

class Loot
{
public:
    friend struct LootItem;
    friend class GroupLootRoll;

    Loot(Player* player, Creature* creature, LootType type);
    Loot(Player* player, GameObject* gameObject, LootType type);
    Loot(Player* player, Corpse* corpse, LootType type);
    Loot(Player* player, Item* item, LootType type);
    Loot(Player* player, uint32 id, LootType type);
    Loot(Unit* unit, Item* item);

    ~Loot();

    // Inserts the item into the loot (called by LootTemplate processors)
    void AddItem(LootStoreItem const& item);
    void AddItem(uint32 _itemid, uint32 _count, uint32 _randomSuffix, int32 _randomPropertyId);             // used in item.cpp to explicitly load a saved item
    bool AutoStore(Player* player, bool broadcast = false, uint32 bag = NULL_BAG, uint32 slot = NULL_SLOT);
    bool CanLoot(Player const* player);
    void ShowContentTo(Player* plr);
    void Update();
    bool IsChanged() { return m_isChanged; }
    void Release(Player* player);
    void GetLootItemsListFor(Player* player, LootItemList& lootList);
    void SetGoldAmount(uint32 _gold);
    void SendGold(Player* player);
    bool IsItemAlreadyIn(uint32 itemId) const;
    uint32 GetGoldAmount() const { return m_gold; }
    LootType GetLootType() const { return m_lootType; }
    LootItem* GetLootItemInSlot(uint32 itemSlot);
    GroupLootRoll* GetRollForSlot(uint32 itemSlot);
    InventoryResult SendItem(Player* target, uint32 itemSlot);
    InventoryResult SendItem(Player* target, LootItem* lootItem);
    WorldObject const* GetLootTarget() const { return m_lootTarget; }
    ObjectGuid const& GetLootGuid() const { return m_guidTarget; }
    ObjectGuid const& GetMasterLootGuid() const { return m_masterOwnerGuid; }

private:
    Loot(): m_lootTarget(nullptr), m_itemTarget(nullptr), m_gold(0), m_maxSlot(0), m_lootType(), m_clientLootType(), m_lootMethod(), m_threshold(), m_maxEnchantSkill(0), m_isReleased(false), m_haveItemOverThreshold(false), m_isChecked(false), m_isChest(false), m_isChanged(false)
    {}
    void Clear();
    bool IsLootedFor(Player const* player) const;
    bool IsLootedForAll() const;
    void SendReleaseFor(ObjectGuid const& guid);
    void SendReleaseFor(Player* plr);
    void SendReleaseForAll();
    void SendAllowedLooter();
    void NotifyMoneyRemoved();
    void NotifyItemRemoved(uint32 lootIndex);
    void NotifyItemRemoved(Player* player, uint32 lootIndex);
    void GroupCheck();
    void CheckIfRollIsNeeded(Player const* plr);
    void SetGroupLootRight(Player* player);
    void GenerateMoneyLoot(uint32 minAmount, uint32 maxAmount);
    bool FillLoot(uint32 loot_id, LootStore const& store, Player* loot_owner, bool personal, bool noEmptyError = false);
    void ForceLootAnimationCLientUpdate();
    void SetPlayerIsLooting(Player* player);
    void SetPlayerIsNotLooting(Player* player);
    void GetLootContentFor(Player* player, ByteBuffer& buffer);
    uint32 GetLootStatusFor(Player const* player) const;

    // What is looted
    WorldObject*     m_lootTarget;
    Item*            m_itemTarget;
    ObjectGuid       m_guidTarget;

    LootItemList     m_lootItems;                     // store of the items contained in loot
    uint32           m_gold;                          // amount of money contained in loot
    uint32           m_maxSlot;                       // used to increment slot index and get total items count
    LootType         m_lootType;                      // internal loot type
    ClientLootType   m_clientLootType;                // client loot type
    LootMethod       m_lootMethod;                    // used to know what kind of check must be done at loot time
    ItemQualities    m_threshold;                     // group threshold for items
    ObjectGuid       m_masterOwnerGuid;               // master loot player or round robin owner
    ObjectGuid       m_currentLooterGuid;             // current player for under threshold items (Round Robin)
    GuidSet          m_ownerSet;                      // set of all player who have right to the loot
    uint32           m_maxEnchantSkill;               // used to know group right to use disenchant option
    bool             m_isReleased;                    // used to release loot for round robin item
    bool             m_haveItemOverThreshold;         // if at least one item in the loot is over threshold
    bool             m_isChecked;                     // true if at least one player received the loot content
    bool             m_isChest;                       // chest type object have special loot right
    bool             m_isChanged;                     // true if at least one item is looted
    GroupLootRollMap m_roll;                          // used if an item is under rolling
    GuidSet          m_playersLooting;                // player who opened loot windows
};

extern LootStore LootTemplates_Creature;
extern LootStore LootTemplates_Fishing;
extern LootStore LootTemplates_Gameobject;
extern LootStore LootTemplates_Item;
extern LootStore LootTemplates_Mail;
extern LootStore LootTemplates_Pickpocketing;
extern LootStore LootTemplates_Skinning;
extern LootStore LootTemplates_Disenchant;

void LoadLootTemplates_Creature();
void LoadLootTemplates_Fishing();
void LoadLootTemplates_Gameobject();
void LoadLootTemplates_Item();
void LoadLootTemplates_Mail();
void LoadLootTemplates_Pickpocketing();
void LoadLootTemplates_Skinning();
void LoadLootTemplates_Disenchant();

void LoadLootTemplates_Reference();

inline void LoadLootTables()
{
    LoadLootTemplates_Creature();
    LoadLootTemplates_Fishing();
    LoadLootTemplates_Gameobject();
    LoadLootTemplates_Item();
    LoadLootTemplates_Mail();
    LoadLootTemplates_Pickpocketing();
    LoadLootTemplates_Skinning();
    LoadLootTemplates_Disenchant();

    LoadLootTemplates_Reference();
}

class LootMgr
{
public:
    bool IsAllowedToLoot(Player* player, Creature* creature);
    void PlayerVote(Player* player, ObjectGuid const& lootTargetGuid, uint32 itemSlot, RollVote vote);
    Loot* GetLoot(Player* player, ObjectGuid const& targetGuid = ObjectGuid());
};

#define sLootMgr MaNGOS::Singleton<LootMgr>::Instance()
#endif
