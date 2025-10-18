#ifndef _RandomItemMgr_H
#define _RandomItemMgr_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "playerbot/AiFactory.h"
#ifdef CMANGOS
#include "Entities/Player.h"
#endif
#ifdef MANGOS
#include "Object/Player.h"
#endif
#include "strategy/values/ItemUsageValue.h"

enum RandomItemType
{
    RANDOM_ITEM_GUILD_TASK,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE
};

#define MAX_STAT_SCALES 32

enum ItemSource
{
    ITEM_SOURCE_NONE,
    ITEM_SOURCE_DROP,
    ITEM_SOURCE_VENDOR,
    ITEM_SOURCE_QUEST,
    ITEM_SOURCE_CRAFT,
    ITEM_SOURCE_PVP
};

enum ItemSpecType
{
    ITEM_SPEC_NONE = 0x0,
    ITEM_SPEC_SPELL_DAMAGE = 0x01,
    ITEM_SPEC_SPELL_HEALING = 0x02,
    ITEM_SPEC_ATTACK = 0x04,
    ITEM_SPEC_TANK = 0x08,
    ITEM_SPEC_CASTER = 0x10,
};

struct WeightScaleInfo
{
    uint32 id;
    std::string name;
    uint8 classId;
};

struct WeightScaleStat
{
    std::string stat;
    uint32 weight;
};

struct ItemInfoEntry
{
    ItemInfoEntry() : minLevel(0), itemLevel(0), source(0), team(0), repRank(0), repFaction(0), reqSkill(0), reqSkillRank(0), pvpRank(0), quality(0), slot(0), itemId(0)
    {
        for (int i = 1; i <= MAX_STAT_SCALES; ++i)
        {
            weights[i] = 0;
        }
        itemSpec = ITEM_SPEC_NONE;
    }

    std::map<uint32, uint32> weights;
    uint32 minLevel;
    uint32 itemLevel;
    uint32 source;
    std::list<uint32> sourceIds;
    uint32 team;
    uint32 repRank;
    uint32 repFaction;
    uint32 reqSkill;
    uint32 reqSkillRank;
    uint32 pvpRank;
    uint32 quality;
    uint32 slot;
    uint32 itemId;
    ItemSpecType itemSpec;
};

typedef std::vector<WeightScaleStat> WeightScaleStats;
//typedef std::map<WeightScaleInfo, WeightScaleStats> WeightScaleList;

struct WeightScale
{
    WeightScaleInfo info;
    WeightScaleStats stats;
};

//typedef std::map<uint32, WeightScale> WeightScales;

class RandomItemPredicate
{
public:
    virtual ~RandomItemPredicate(){};
    virtual bool Apply(ItemPrototype const* proto) = 0;
};

typedef std::vector<uint32> RandomItemList;
typedef std::map<RandomItemType, RandomItemList> RandomItemCache;

class BotEquipKey
{
public:
    uint32 level;
    uint8 clazz;
    uint8 spec;
    uint8 slot;
    uint32 quality;
    uint64 key;

public:
    BotEquipKey() : level(0), clazz(0), spec(0), slot(0), quality(0), key(GetKey()) {}
    BotEquipKey(uint32 level, uint8 clazz, uint8 spec, uint8 slot, uint32 quality) : level(level), clazz(clazz), spec(spec), slot(slot), quality(quality), key(GetKey()) {}
    BotEquipKey(BotEquipKey const& other)  : level(other.level), clazz(other.clazz), spec(other.spec), slot(other.slot), quality(other.quality), key(GetKey()) {}

private:
    uint64 GetKey();

public:
    bool operator< (const BotEquipKey& other) const
    {
        return other.key < this->key;
    }
};

typedef std::map<BotEquipKey, RandomItemList> BotEquipCache;

class RandomItemMgr
{
    public:
        RandomItemMgr();
        virtual ~RandomItemMgr();
        static RandomItemMgr& instance()
        {
            static RandomItemMgr instance;
            return instance;
        }

	public:
        void Init();
        static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
        RandomItemList Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate);
        RandomItemList Query(uint32 level, uint8 clazz, uint8 specId, uint8 slot, uint32 quality);
        uint32 GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId);
        std::vector<uint32> GetUpgradeList(Player* player, uint32 specId, uint8 slot, uint32 quality, uint32 itemId, uint32 amount = 1);
        bool HasStatWeight(uint32 itemId);
        bool CanBuyFromVendor(Player* player, uint32 itemId, uint32 creatureId);
        bool HasSameQuestRewards(Player* player, uint32 itemId);
        uint32 GetMinLevelFromCache(uint32 itemId);
        uint32 GetStatWeight(Player* player, uint32 itemId);
        uint32 GetLiveStatWeight(Player* player, uint32 itemId, uint32 specId = 0);
        uint32 GetStatWeight(uint32 itemId, uint32 specId);
        uint32 GetBestRandomEnchantStatWeight(uint32 itemId, uint32 specId);
        uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = NULL);
        uint32 GetAmmo(uint32 level, uint32 subClass);
        uint32 GetRandomPotion(uint32 level, uint32 effect);
        uint32 GetRandomFood(uint32 level, uint32 category);
        uint32 GetFood(uint32 level, uint32 category);
        uint32 GetRandomTrade(uint32 level);
        std::vector<uint32> GetGemsList();

        uint32 CalculateRandomEnchantId(uint8 playerclass, uint8 spec, ItemPrototype const* proto);
        uint32 CalculateBestRandomEnchantId(uint8 playerclass, uint8 spec, uint32 itemId);
        uint32 CalculateEnchantWeight(uint8 playerclass, uint8 spec, uint32 enchantId);
        uint32 CalculateRandomPropertyWeight(uint8 playerclass, uint8 spec, int32 randomPropertyId);
        uint32 CalculateGemWeight(uint8 playerclass, uint8 spec, uint32 gemId);
        uint32 CalculateSocketWeight(uint8 playerclass, ItemQualifier& qualifier, uint8 spec);

        uint32 CalculateStatWeight(uint8 playerclass, uint8 spec, ItemPrototype const* proto, ItemSpecType &itSpec);
        uint32 ItemStatWeight(Player* player, ItemQualifier& qualifier);
        uint32 ItemStatWeight(Player* player, Item* item);

        uint32 CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value);
        bool CanEquipArmor(uint8 clazz, uint8 spec, uint32 level, ItemPrototype const* proto);
        bool ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto);
        bool CanEquipWeapon(uint8 clazz, ItemPrototype const* proto);
        bool ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto);
        bool CheckItemSpec(uint8 spec, ItemSpecType itSpec);
        float GetItemRarity(uint32 itemId);
        uint32 GetQuestIdForItem(uint32 itemId);
        std::vector<uint32> GetQuestIdsForItem(uint32 itemId);
        std::string GetPlayerSpecName(Player* player);
        uint32 GetPlayerSpecId(Player* player);
        std::vector<uint32> GetGlyphs(uint8 playerClass) { return glyphCache.at(playerClass); }
    private:
        void BuildRandomItemCache();
        void BuildEquipCache();
        void BuildItemInfoCache();
        void BuildAmmoCache();
        void BuildFoodCache();
        void BuildPotionCache();
        void BuildTradeCache();
        void BuildRarityCache();
        void BuildGlyphCache();
        void LoadRandomEnchantments();
        bool CanEquipItem(BotEquipKey key, ItemPrototype const* proto);
        bool CanEquipItemNew(ItemPrototype const* proto);
        void AddItemStats(uint32 mod, uint8 &sp, uint8 &ap, uint8 &tank);
        bool CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank);

    private:
        std::map<uint32, RandomItemCache> randomItemCache;
        std::map<RandomItemType, RandomItemPredicate*> predicates;
        BotEquipCache equipCache;
        std::map<EquipmentSlots, std::set<InventoryType> > viableSlots;
        std::map<uint32, std::map<uint32, uint32> > ammoCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > potionCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > foodCache;
        std::map<uint32, std::vector<uint32> > tradeCache;
        std::map<uint32, float> rarityCache;
        std::map<uint32, WeightScale> m_weightScales;
        std::map<std::string, uint32 > weightStatLink;
        std::map<uint32, std::string > ItemStatLink;
        std::map<std::string, uint32 > weightRatingLink;
        std::map<uint32, ItemInfoEntry*> itemInfoCache;
        std::map<uint32, std::vector<uint32> > randomEnchantsCache;
        std::map<uint8, std::vector<uint32>> glyphCache;
};

#define sRandomItemMgr RandomItemMgr::instance()

#endif
