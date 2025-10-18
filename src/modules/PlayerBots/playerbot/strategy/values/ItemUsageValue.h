#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    class ItemQualifier
    {
    public:
        ItemQualifier() { itemId = 0; enchantId = 0; randomPropertyId = 0; gem1 = 0; gem2 = 0; gem3 = 0; gem4 = 0; proto = nullptr; };
        ItemQualifier(uint32 itemId, int32 randomPropertyId = 0, uint32 enchantId = 0, uint32 gem1 = 0, uint32 gem2 = 0, uint32 gem3 = 0, uint32 gem4 = 0) : itemId(itemId), randomPropertyId(randomPropertyId), enchantId(enchantId), gem1(gem1), gem2(gem2), gem3(gem3), gem4(gem4) { proto = nullptr; };
        ItemQualifier(Item* item) { itemId = item->GetProto()->ItemId; enchantId = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT); randomPropertyId = item->GetItemRandomPropertyId(); gem1 = GemId(item, 0); gem2 = GemId(item, 1); gem3 = GemId(item, 2); gem4 = GemId(item, 3); proto = item->GetProto(); };
        ItemQualifier(LootItem* item) { itemId = item->itemId; enchantId = 0; randomPropertyId = item->randomPropertyId; gem1 = 0; gem2 = 0; gem3 = 0; gem4 = 0; proto = nullptr; };
        ItemQualifier(AuctionEntry* auction) { itemId = auction->itemTemplate; enchantId = 0; randomPropertyId = auction->itemRandomPropertyId; gem1 = 0; gem2 = 0; gem3 = 0; gem4 = 0; proto = nullptr; };
        ItemQualifier(std::string qualifier, bool linkQualifier = true);

        uint32 GetId() { return itemId; }
        uint32 GetEnchantId() { return enchantId; }
        int32 GetRandomPropertyId() { return randomPropertyId; }
        uint32 GetGem1() { return gem1; }
        uint32 GetGem2() { return gem2; }
        uint32 GetGem3() { return gem3; }
        uint32 GetGem4() { return gem4; }

        operator bool() const { return itemId != 0; }

        bool operator==(const ItemQualifier& qualifier) const { return itemId == qualifier.itemId && enchantId == qualifier.enchantId && randomPropertyId == qualifier.randomPropertyId && gem1 == qualifier.gem1 && gem2 == qualifier.gem2 && gem3 == qualifier.gem3 && gem4 == qualifier.gem4; }

#ifdef MANGOSBOT_ZERO
        std::string GetLinkQualifier() { return std::to_string(itemId) + ":" + std::to_string(enchantId) + ":" + std::to_string(randomPropertyId) + ":0"; }
#else
        std::string GetLinkQualifier() { return std::to_string(itemId) + ":" + std::to_string(enchantId) + ":" + std::to_string(gem1) + ":" + std::to_string(gem2) + ":" + std::to_string(gem3) + ":" + std::to_string(gem4) + ":" + std::to_string(randomPropertyId) + ":0"; }
#endif
        std::string GetQualifier() { return std::to_string(itemId) + ((enchantId || gem1 || gem2 || gem3 || gem4 || randomPropertyId) ? ":" + std::to_string(enchantId) + ":" + std::to_string(gem1) + ":" + std::to_string(gem2) + ":" + std::to_string(gem3) + ":" + std::to_string(gem4) + ":" + std::to_string(randomPropertyId) : ""); }

        ItemPrototype const* GetProto() { if (!proto) proto = sItemStorage.LookupEntry<ItemPrototype>(itemId); return proto; };
        static uint32 GemId(Item* item, uint8 gemSlot = 0);
    private:
        uint32 itemId;
        uint32 enchantId;
        int32 randomPropertyId;
        uint32 gem1;
        uint32 gem2;
        uint32 gem3;
        uint32 gem4;
        ItemPrototype const* proto;
    };

    enum class ItemUsage : uint8
    {
        ITEM_USAGE_NONE = 0,
        ITEM_USAGE_EQUIP = 1,
        ITEM_USAGE_BAD_EQUIP = 2,
        ITEM_USAGE_BROKEN_EQUIP = 3,
        ITEM_USAGE_QUEST = 4,
        ITEM_USAGE_SKILL = 5,
        ITEM_USAGE_USE = 6,
        ITEM_USAGE_GUILD_TASK = 7,
        ITEM_USAGE_DISENCHANT = 8,
        ITEM_USAGE_AH = 9,
        ITEM_USAGE_BROKEN_AH = 10,
        ITEM_USAGE_KEEP = 11,
        ITEM_USAGE_VENDOR = 12,
        ITEM_USAGE_AMMO = 13,
        ITEM_USAGE_FORCE_NEED = 14,
        ITEM_USAGE_FORCE_GREED = 15
    };

    enum class ForceItemUsage : uint8
    {
        FORCE_USAGE_NONE = 0,  //Normal usage.
        FORCE_USAGE_KEEP = 1,  //Do not sell item.
        FORCE_USAGE_EQUIP = 2, //Equip item if no other forced equipped.
        FORCE_USAGE_GREED = 3,  //Get more and greed for rolls.
        FORCE_USAGE_NEED = 4    //Get more and need for rolls.
    };

    class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
    {
    public:
        ItemUsageValue(PlayerbotAI* ai, std::string name = "item usage") : CalculatedValue<ItemUsage>(ai, name), Qualified() {}
        virtual ItemUsage Calculate();

        static ItemUsage QueryItemUsageForEquip(ItemQualifier& itemQualifier, Player* bot);
        static uint32 GetSmallestBagSize(Player* bot);
        static std::string ReasonForNeed(ItemUsage usage, ItemQualifier qualifier = ItemQualifier(), uint32 count = 1, Player* bot = nullptr);
        static uint32 GetAhDepositCost(ItemPrototype const* proto, uint32 count = 1);
        static Item* CurrentItem(ItemPrototype const* proto, Player* bot);
        static Item* CurrentItemInSlot(ItemPrototype const* proto, Player* bot);
        static uint32 ItemCreatedFrom(uint32 wantItemId);
        static bool IsNeededForQuest(Player* player, uint32 itemId, bool ignoreInventory = false);
    private:        
        bool IsItemNeededForSkill(ItemPrototype const* proto);
        bool IsItemUsefulForSkill(ItemPrototype const* proto);
        bool IsItemNeededForUsefullCraft(ItemPrototype const* proto, bool checkAllReagents);
        float BetterStacks(ItemPrototype const* proto, std::string usageType = "");

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "item usage"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "item"; }
        virtual std::string GetHelpDescription()
        {
            return "This value gives the reason why a bot finds an item useful.\n"
                "Based on this value bots will equip/unequip/need/greed/loot/destroy/sell/ah/craft items.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { "bag space", "force item usage", "inventory items", "item count" }; }
#endif
    private:
        //crafting/gathering/secondary profession material (reagent) ids
        //<professtion skill id, set of reagent item ids>
        static std::unordered_map<uint32, std::unordered_set<uint32>> m_reagentItemIdsForCraftingSkills;
        //flat for quick checks
        static std::unordered_set<uint32> m_allReagentItemIdsForCraftingSkills;
        static std::vector<uint32> m_allReagentItemIdsForCraftingSkillsVector;

        static std::unordered_map<uint32, std::vector<std::pair<uint32, uint32>>> m_craftingReagentItemIdsForCraftableItem;

        static std::unordered_set<uint32> m_allItemIdsSoldByAnyVendors;
        static std::unordered_set<uint32> m_itemIdsSoldByAnyVendorsWithLimitedMaxCount;

        static double GetRarityPriceMultiplier(ItemPrototype const* proto);
        static double GetLevelPriceMultiplier(ItemPrototype const* proto);

        static bool MustEquipForQuest(ItemPrototype const* proto, Player* bot);
    public:
        static float CurrentStacks(PlayerbotAI* ai, ItemPrototype const* proto);
        static std::vector<uint32> SpellsUsingItem(uint32 itemId, Player* bot);

        static bool IsHpFoodOrDrink(ItemPrototype const* proto);
        static bool IsManaFoodOrDrink(ItemPrototype const* proto);
        static bool IsHealingPotion(ItemPrototype const* proto);
        static bool IsManaPotion(ItemPrototype const* proto);
        static bool IsBandage(ItemPrototype const* proto);

        static uint32 GetRecipeSpell(ItemPrototype const* proto);

        static void PopulateProfessionReagentIds();
        static void PopulateReagentItemIdsForCraftableItemIds();
        static void PopulateSoldByVendorItemIds();

        static std::vector<uint32> GetAllReagentItemIdsForCraftingSkillsVector();
        static std::vector<std::pair<uint32, uint32>> GetAllReagentItemIdsForCraftingItem(ItemPrototype const* proto);

        static bool IsItemSoldByAnyVendor(ItemPrototype const* proto);
        static bool IsItemSoldByAnyVendorButHasLimitedMaxCount(ItemPrototype const* proto);

        static bool IsItemUsedBySkill(ItemPrototype const* proto, SkillType skillType);
        static bool IsItemUsedToCraftAnything(ItemPrototype const* proto);

        /*
        * Median buyout price for current listings of the item on AH
        */
        static uint32 GetAHMedianBuyoutPricePerItem(ItemPrototype const* proto);
        static uint32 GetAHListingLowestBuyoutPricePerItem(ItemPrototype const* proto);
        static bool AreCurrentAHListingsTooCheap(ItemPrototype const* proto);
        static bool IsMoreProfitableToSellToAHThanToVendor(ItemPrototype const* proto, Player* bot);
        static bool IsWorthBuyingFromVendorToResellAtAH(ItemPrototype const* proto, bool isLimitedSupply);
        static bool IsWorthBuyingFromAhToResellAtAH(ItemPrototype const* proto, uint32 totalCost, uint32 itemCount);

        static uint32 GetItemBaseValue(ItemPrototype const* proto, uint8 maxReagentLevel = 5);

        /*
        * bots buy at this price
        */
        static uint32 GetBotBuyPrice(ItemPrototype const* proto, Player* bot);
        /*
        * bots sell at this price
        */
        static uint32 GetBotSellPrice(ItemPrototype const* proto, Player* bot);
        static uint32 GetBotAHSellMinPrice(ItemPrototype const* proto);
        static uint32 GetBotAHSellMaxPrice(ItemPrototype const* proto);
        static uint32 GetCraftingFee(ItemPrototype const* proto);

        static uint32 DesiredPricePerItem(Player* bot, const ItemPrototype* proto, uint32 count, uint32 priceModifier);
    };

    class ForceItemUsageValue : public ManualSetValue<ForceItemUsage>, public Qualified
    {
    public:
        ForceItemUsageValue(PlayerbotAI* ai, std::string name = "force item usage") : ManualSetValue<ForceItemUsage>(ai, ForceItemUsage::FORCE_USAGE_NONE, name), Qualified() {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "force item usage"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "item"; }
        virtual std::string GetHelpDescription()
        {
            return "This value overrides some reasons why a bot finds an item useful\n"
                "Based on this value bots will no longer sell/ah/destroy/unequip items.";
        }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif

        virtual std::string Save() override { return (uint8)value ? std::to_string((uint8)value) : "?"; }
        virtual bool Load(std::string force) override { if (!force.empty()) value = ForceItemUsage(stoi(force)); return !force.empty(); }
    };
}
