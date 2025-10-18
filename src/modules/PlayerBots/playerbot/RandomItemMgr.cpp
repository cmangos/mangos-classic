
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "RandomItemMgr.h"
#include "playerbot/PlayerbotAI.h"

#include "Database/DBCStore.h"
#include "Database/DatabaseEnv.h"
#include "PlayerbotAI.h"

#include "playerbot/ServerFacade.h"
#include "strategy/values/LootValues.h"

#include "Entities/ItemEnchantmentMgr.h"

#include "strategy/values/SharedValueContext.h"

char * strstri (const char* str1, const char* str2);

uint64 BotEquipKey::GetKey()
{
    return level + 100 * clazz + 10000 * spec + 1000000 * slot + 100000000 * quality;
}

RandomItemMgr::RandomItemMgr()
{
    viableSlots[EQUIPMENT_SLOT_HEAD].insert(INVTYPE_HEAD);
    viableSlots[EQUIPMENT_SLOT_NECK].insert(INVTYPE_NECK);
    viableSlots[EQUIPMENT_SLOT_SHOULDERS].insert(INVTYPE_SHOULDERS);
    viableSlots[EQUIPMENT_SLOT_BODY].insert(INVTYPE_BODY);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_CHEST);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_ROBE);
    viableSlots[EQUIPMENT_SLOT_WAIST].insert(INVTYPE_WAIST);
    viableSlots[EQUIPMENT_SLOT_LEGS].insert(INVTYPE_LEGS);
    viableSlots[EQUIPMENT_SLOT_FEET].insert(INVTYPE_FEET);
    viableSlots[EQUIPMENT_SLOT_WRISTS].insert(INVTYPE_WRISTS);
    viableSlots[EQUIPMENT_SLOT_HANDS].insert(INVTYPE_HANDS);
    viableSlots[EQUIPMENT_SLOT_FINGER1].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_FINGER2].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_TRINKET1].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_TRINKET2].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_SHIELD);
    //viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_HOLDABLE);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGED);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_THROWN);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGEDRIGHT);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RELIC);
    viableSlots[EQUIPMENT_SLOT_TABARD].insert(INVTYPE_TABARD);
    viableSlots[EQUIPMENT_SLOT_BACK].insert(INVTYPE_CLOAK);

    weightStatLink["sta"] = ITEM_MOD_STAMINA;
    weightStatLink["str"] = ITEM_MOD_STRENGTH;
    weightStatLink["agi"] = ITEM_MOD_AGILITY;
    weightStatLink["int"] = ITEM_MOD_INTELLECT;
    weightStatLink["spi"] = ITEM_MOD_SPIRIT;

    ItemStatLink[STAT_STAMINA] = "sta";
    ItemStatLink[STAT_STRENGTH] = "str";
    ItemStatLink[STAT_AGILITY] = "agi";
    ItemStatLink[STAT_INTELLECT] = "int";
    ItemStatLink[STAT_SPIRIT] = "spi";

#ifdef MANGOSBOT_TWO
    weightStatLink["splpwr"] = ITEM_MOD_SPELL_POWER;
    weightStatLink["atkpwr"] = ITEM_MOD_ATTACK_POWER;
    weightStatLink["feratkpwr"] = ITEM_MOD_FERAL_ATTACK_POWER;

    weightStatLink["exprtng"] = ITEM_MOD_EXPERTISE_RATING;
    weightStatLink["critstrkrtng"] = ITEM_MOD_CRIT_RATING;
    weightStatLink["hitrtng"] = ITEM_MOD_HIT_RATING;
    weightStatLink["hastertng"] = ITEM_MOD_HASTE_RATING;
    weightStatLink["armorpenrtng"] = ITEM_MOD_ARMOR_PENETRATION_RATING;

    weightStatLink["defrtng"] = ITEM_MOD_DEFENSE_SKILL_RATING;
    weightStatLink["dodgertng"] = ITEM_MOD_DODGE_RATING;
    weightStatLink["block"] = ITEM_MOD_BLOCK_VALUE;
    weightStatLink["blockrtng"] = ITEM_MOD_BLOCK_RATING;
    weightStatLink["parryrtng"] = ITEM_MOD_PARRY_RATING;

    weightStatLink["manargn"] = ITEM_MOD_MANA_REGENERATION;

    weightRatingLink["exprtng"] = CR_EXPERTISE;
    weightRatingLink["critstrkrtng"] = CR_CRIT_MELEE;
    weightRatingLink["hitrtng"] = CR_HIT_MELEE;
    weightRatingLink["hastertng"] = CR_HASTE_MELEE;
    weightRatingLink["armorpenrtng"] = CR_ARMOR_PENETRATION;

    weightRatingLink["defrtng"] = CR_DEFENSE_SKILL;
    weightRatingLink["dodgertng"] = CR_DODGE;
    weightRatingLink["blockrtng"] = CR_BLOCK;
    weightRatingLink["parryrtng"] = CR_PARRY;
#endif

#ifdef MANGOSBOT_ONE
    //weightStatLink["splpwr"] = ITEM_MOD_SPELL_POWER;
    //weightStatLink["atkpwr"] = ITEM_MOD_ATTACK_POWER;
    //weightStatLink["feratkpwr"] = ITEM_MOD_FERAL_ATTACK_POWER;

    weightStatLink["exprtng"] = ITEM_MOD_EXPERTISE_RATING;
    weightStatLink["critstrkrtng"] = ITEM_MOD_CRIT_RATING;
    weightStatLink["spellcritstrkrtng"] = ITEM_MOD_CRIT_SPELL_RATING;
    weightStatLink["hitrtng"] = ITEM_MOD_HIT_RATING;
    weightStatLink["spellhitrtng"] = ITEM_MOD_HIT_SPELL_RATING;
    weightStatLink["hastertng"] = ITEM_MOD_HASTE_RATING;
    weightStatLink["spellhastertng"] = ITEM_MOD_HASTE_SPELL_RATING;
    //weightStatLink["armorpenrtng"] = ITEM_MOD_ARMOR_PENETRATION_RATING;

    weightStatLink["defrtng"] = ITEM_MOD_DEFENSE_SKILL_RATING;
    weightStatLink["dodgertng"] = ITEM_MOD_DODGE_RATING;
    //weightStatLink["block"] = ITEM_MOD_BLOCK_VALUE;
    weightStatLink["blockrtng"] = ITEM_MOD_BLOCK_RATING;
    weightStatLink["parryrtng"] = ITEM_MOD_PARRY_RATING;

    //weightStatLink["manargn"] = ITEM_MOD_MANA_REGENERATION;

    weightRatingLink["exprtng"] = CR_EXPERTISE;
    weightRatingLink["critstrkrtng"] = CR_CRIT_MELEE;
    weightRatingLink["hitrtng"] = CR_HIT_MELEE;
    weightRatingLink["hastertng"] = CR_HASTE_MELEE;
    weightRatingLink["spellcritstrkrtng"] = CR_CRIT_SPELL;
    weightRatingLink["spellhastertng"] = CR_HASTE_SPELL;
    weightRatingLink["spellhitrtng"] = CR_HIT_SPELL;
    //weightRatingLink["armorpenrtng"] = CR_ARMOR_PENETRATION;

    weightRatingLink["defrtng"] = CR_DEFENSE_SKILL;
    weightRatingLink["dodgertng"] = CR_DODGE;
    weightRatingLink["blockrtng"] = CR_BLOCK;
    weightRatingLink["parryrtng"] = CR_PARRY;
#endif
}

void RandomItemMgr::Init()
{
    BuildItemInfoCache();
    BuildEquipCache();
    BuildAmmoCache();
    BuildPotionCache();
    BuildFoodCache();
    BuildTradeCache();
    LoadRandomEnchantments();
    BuildRandomItemCache();
#ifdef MANGOSBOT_TWO
    BuildGlyphCache();
#endif
    //BuildRarityCache();
}

RandomItemMgr::~RandomItemMgr()
{
    for (std::map<RandomItemType, RandomItemPredicate*>::iterator i = predicates.begin(); i != predicates.end(); ++i)
        delete i->second;

    predicates.clear();
}

bool RandomItemMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        sLog.outError( "Usage: rnditem");
        return false;
    }

    return false;
}

RandomItemList RandomItemMgr::Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList &list = randomItemCache[(level - 1) / 10][type];

    RandomItemList result;
    for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
    {
        uint32 itemId = *i;
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        if (predicate && !predicate->Apply(proto))
            continue;

        result.push_back(itemId);
    }

    return result;
}

void RandomItemMgr::BuildRandomItemCache()
{
    auto results = CharacterDatabase.PQuery("select lvl, type, item from ai_playerbot_rnditem_cache");
    if (results)
    {
        sLog.outString("Loading random item cache");
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 level = fields[0].GetUInt32();
            uint32 type = fields[1].GetUInt32();
            uint32 itemId = fields[2].GetUInt32();

            RandomItemType rit = (RandomItemType)type;
            randomItemCache[level][rit].push_back(itemId);
            count++;

        } while (results->NextRow());
        sLog.outString("Equipment cache loaded from %d records", count);
    }
    else
    {
        sLog.outString("Building random item cache from %u items", sItemStorage.GetMaxEntry());
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (strstri(proto->Name1, "qa") || strstri(proto->Name1, "test") || strstri(proto->Name1, "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;

            if (!proto->SellPrice)
                continue;

            uint32 level = proto->ItemLevel;
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemType rit = (RandomItemType)type;
                if (predicates[rit] && !predicates[rit]->Apply(proto))
                    continue;

                randomItemCache[level / 10][rit].push_back(itemId);
                CharacterDatabase.PExecute("insert into ai_playerbot_rnditem_cache (lvl, type, item) values (%u, %u, %u)",
                        level / 10, type, itemId);
            }
        }

        uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
        if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);
        for (uint32 level = 0; level < (maxLevel / 10); level++)
        {
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemList list = randomItemCache[level][(RandomItemType)type];
                sLog.outDetail("    Level %d..%d Type %d - %zu random items cached",
                        level * 10, level * 10 + 9,
                        type,
                        list.size());
                for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    uint32 itemId = *i;
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                    if (!proto)
                        continue;

                    sLog.outDetail("        [%d] %s", itemId, proto->Name1);
                }
            }
        }
    }
}

uint32 RandomItemMgr::GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList const& list = Query(level, type, predicate);
    if (list.empty())
        return 0;

    uint32 index = urand(0, list.size() - 1);
    uint32 itemId = list[index];

    return itemId;
}

bool RandomItemMgr::CanEquipItem(BotEquipKey key, ItemPrototype const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != key.quality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    std::set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
        return false;

    uint32 requiredLevel = proto->RequiredLevel;

    if (!requiredLevel)
    {
        requiredLevel = GetMinLevelFromCache(proto->ItemId);
    }
    if (!requiredLevel)
        return false;

    return true;
}

bool RandomItemMgr::CanEquipItemNew(ItemPrototype const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return false;

    bool properSlot = false;
    for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
    {
        std::set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
            properSlot = true;
    }

    return properSlot;
}

void RandomItemMgr::AddItemStats(uint32 mod, uint8 &sp, uint8 &ap, uint8 &tank)
{
    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_MANA:
    case ITEM_MOD_INTELLECT:
    case ITEM_MOD_SPIRIT:
        sp++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
        tank++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
        ap++;
        break;
    }
}

bool RandomItemMgr::CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank)
{
    switch (clazz)
    {
    case CLASS_PRIEST:
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (!sp || ap > sp || tank > sp)
            return false;
        break;
    case CLASS_PALADIN:
    case CLASS_WARRIOR:
        if ((!ap && !tank) || sp > ap || sp > tank)
            return false;
        break;
    case CLASS_HUNTER:
    case CLASS_ROGUE:
        if (!ap || sp > ap || sp > tank)
            return false;
        break;
    }

    return sp || ap || tank;
}

bool RandomItemMgr::ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if (!m_weightScales[spec].info.id)
        return false;

    std::unordered_set<uint32> resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };

    switch (playerclass)
    {
    case CLASS_WARRIOR:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name == "arms" || m_weightScales[spec].info.name == "fury")
        {
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
        }
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
        break;
    }
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_SIGIL, ITEM_SUBCLASS_ARMOR_PLATE };
        break;
    }
#endif
    case CLASS_PALADIN:
    {
        if (m_weightScales[spec].info.name != "holy" && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name != "holy")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE , ITEM_SUBCLASS_ARMOR_LIBRAM };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE, ITEM_SUBCLASS_ARMOR_LIBRAM };
        break;
    }
    case CLASS_HUNTER:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
    }
    case CLASS_ROGUE:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER };
        break;
    }
    case CLASS_PRIEST:
    {
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };
        break;
    }
    case CLASS_SHAMAN:
    {
        if (m_weightScales[spec].info.name == "enhance" && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

#ifdef MANGOSBOT_ZERO
        if (m_weightScales[spec].info.name == "enhance")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
#else
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
#endif
    }
    case CLASS_MAGE:
    case CLASS_WARLOCK:
    {
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };
        break;
    }
    case CLASS_DRUID:
    {
        if ((m_weightScales[spec].info.name == "feraltank" || m_weightScales[spec].info.name == "feraldps") && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name == "feraltank" || m_weightScales[spec].info.name == "feraldps")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_IDOL, ITEM_SUBCLASS_ARMOR_LEATHER };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_IDOL, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER };

        break;
    }
    }

    return resultArmorSubClass.find(proto->SubClass) != resultArmorSubClass.end();
}

bool RandomItemMgr::CanEquipArmor(uint8 clazz, uint8 spec, uint32 level, ItemPrototype const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN || clazz == CLASS_SHAMAN)
            && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if ((clazz == CLASS_WARRIOR || (clazz == CLASS_PALADIN && spec != 4)) && level >= 40)
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_WARRIOR || (clazz == CLASS_PALADIN && spec != 4)) && level < 40) ||
            ((clazz == CLASS_HUNTER || (clazz == CLASS_SHAMAN && spec != 21)) && level >= 40))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL && proto->InventoryType != INVTYPE_CLOAK)
        {
            if (spec == 22 && (proto->SubClass == ITEM_SUBCLASS_ARMOR_LEATHER || proto->SubClass == ITEM_SUBCLASS_ARMOR_CLOTH))
                return true;

            return false;
        }
    }

    if (((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level < 40) ||
            (((clazz == CLASS_DRUID && !(spec == 29 || spec == 31)))  || clazz == CLASS_ROGUE))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    return true;

    //uint8 sp = 0, ap = 0, tank = 0;
    //for (int j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    //{
    //    // for ItemStatValue != 0
    //    if(!proto->ItemStat[j].ItemStatValue)
    //        continue;

    //    AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    //}

    //return CheckItemStats(clazz, sp, ap, tank);
}

bool RandomItemMgr::ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto)
{
    EquipmentSlots slot_mh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_oh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_rh = EQUIPMENT_SLOT_START;
    for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
    {
        std::set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
        {
            if (i->first == EQUIPMENT_SLOT_MAINHAND)
                slot_mh = i->first;
            if (i->first == EQUIPMENT_SLOT_OFFHAND)
                slot_oh = i->first;
            if (i->first == EQUIPMENT_SLOT_RANGED)
                slot_rh = i->first;
        }
    }

    if (slot_mh == EQUIPMENT_SLOT_START && slot_oh == EQUIPMENT_SLOT_START && slot_rh == EQUIPMENT_SLOT_START)
        return false;

    if (!m_weightScales[spec].info.id)
        return false;

    std::unordered_set<uint32> mh_weapons;
    std::unordered_set<uint32> oh_weapons;
    std::unordered_set<uint32> r_weapons;

    switch (playerclass)
    {
    case CLASS_WARRIOR:
    {
        if (m_weightScales[spec].info.name == "prot")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        else if (m_weightScales[spec].info.name == "arms")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_POLEARM };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        break;
    }
    case CLASS_PALADIN:
    {
        if (m_weightScales[spec].info.name == "prot")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        else if (m_weightScales[spec].info.name == "holy")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD, ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_POLEARM };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        break;
    }
    case CLASS_HUNTER:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_FIST, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_POLEARM, ITEM_SUBCLASS_WEAPON_STAFF };
        r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        break;
    }
    case CLASS_ROGUE:
    {
        if (m_weightScales[spec].info.name == "assas")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
        }
        else if (m_weightScales[spec].info.name == "combat")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
        }

        r_weapons = { ITEM_SUBCLASS_WEAPON_THROWN, ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        break;
    }
    case CLASS_PRIEST:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE };
        oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
        r_weapons = { ITEM_SUBCLASS_WEAPON_WAND };
        break;
    }
    case CLASS_SHAMAN:
    {
        if (m_weightScales[spec].info.name == "resto")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC, ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        else if (m_weightScales[spec].info.name == "enhance")
        {
#ifdef MANGOSBOT_ZERO
            mh_weapons = { ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2 };
#else
            mh_weapons = { ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
#endif
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC, ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        break;
    }
    case CLASS_MAGE:
    case CLASS_WARLOCK:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD };
        oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
        r_weapons = { ITEM_SUBCLASS_WEAPON_WAND };
        break;
    }
    case CLASS_DRUID:
    {
        if (m_weightScales[spec].info.name == "feraltank")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else if (m_weightScales[spec].info.name == "resto")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_MACE2 };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else if (m_weightScales[spec].info.name == "feraldps")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2 };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        break;
    }
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_MACE2 };
        r_weapons = { ITEM_SUBCLASS_ARMOR_SIGIL };
        break;
    }
#endif
    }

    if (slot_mh == EQUIPMENT_SLOT_MAINHAND)
    {
        return mh_weapons.find(proto->SubClass) != mh_weapons.end();
    }
    if (slot_oh == EQUIPMENT_SLOT_OFFHAND)
    {
        return oh_weapons.find(proto->SubClass) != oh_weapons.end();
    }
    if (slot_rh == EQUIPMENT_SLOT_RANGED)
    {
        return r_weapons.find(proto->SubClass) != r_weapons.end();
    }

    return false;
}

bool RandomItemMgr::CanEquipWeapon(uint8 clazz, ItemPrototype const* proto)
{
    switch (clazz)
    {
    case CLASS_PRIEST:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
            return false;
        break;
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
    case CLASS_WARRIOR:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
    case CLASS_PALADIN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
    case CLASS_SHAMAN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_DRUID:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_HUNTER:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
            return false;
        break;
    case CLASS_ROGUE:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2)
            return false;
        break;
#endif
    }

    return true;
}

void RandomItemMgr::BuildItemInfoCache()
{
    uint32 maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    for (uint32 i = 0; i <= MAX_STAT_SCALES; ++i)
    {
        WeightScale scale;
        scale.info.id = 0;
        scale.info.name = "";
        scale.info.classId = 0;
        m_weightScales[i] = scale;
    }

    // load weightscales
    sLog.outString("Loading weightscales info");
    auto results = WorldDatabase.PQuery("select id, name, class from ai_playerbot_weightscales");

    if (results)
    {
        int totalcount = 0;
        int statcount = 0;
        int curClass = CLASS_WARRIOR;

        do
        {
            Field* fields = results->Fetch();
            uint32 id = fields[0].GetUInt32();
            std::string name = fields[1].GetString();
            uint32 clazz = fields[2].GetUInt32();

            WeightScale scale;
            scale.info.id = id;
            scale.info.name = name;
            scale.info.classId = clazz;
            m_weightScales[id] = scale;
            totalcount++;

        } while (results->NextRow());

        sLog.outString("Loaded %d weightscale class specs", totalcount);

        auto result = WorldDatabase.PQuery("select id, field, val from ai_playerbot_weightscale_data");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 id = fields[0].GetUInt32();
                std::string field = fields[1].GetString();
                uint32 weight = fields[2].GetUInt32();

                WeightScaleStat stat;
                stat.stat = field;
                stat.weight = weight;

                m_weightScales[id].stats.push_back(stat);
                statcount++;

            } while (result->NextRow());
        }

        sLog.outString("Loaded %d weightscale stat weights", statcount);
    }

    if (m_weightScales[1].stats.empty())
    {
        sLog.outError("Error loading item weight scales");
        return;
    }

    // vendor items
    sLog.outString("Loading vendor item list...");
    std::vector<uint32> vendorItems;
    std::vector<uint32> allianceItems;
    std::vector<uint32> hordeItems;
    vendorItems.clear();
    if (auto result = WorldDatabase.PQuery("%s", "SELECT item, entry FROM npc_vendor"))
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            bar.step();
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (!entry)
                continue;
            vendorItems.push_back(fields[0].GetUInt32());

#ifdef MANGOSBOT_ZERO
            uint32 vendorId = fields[1].GetUInt32();
            if (vendorId)
            {
                if (vendorId == 12782 || vendorId == 12777 || vendorId == 12785)
                    allianceItems.push_back(entry);
                if (vendorId == 14581 || vendorId == 12792 || vendorId == 12794)
                    hordeItems.push_back(entry);
            }
#endif
        } while (result->NextRow());
    }
    sLog.outString("Loaded %d vendor items...", (uint32)vendorItems.size());
    sLog.outString("Loaded %d alliance only vendor items...", (uint32)allianceItems.size());
    sLog.outString("Loaded %d horde only vendor items...", (uint32)hordeItems.size());

    // calculate drop source
    sLog.outString("Loading loot templates...");
    DropMap* dropMap = new DropMap;

    int32 sEntry;

    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); entry++)
    {
        sEntry = entry;

        LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_UNIT, entry, uint32(1)), LOOT_CORPSE);

        if (lTemplateA)
            for (LootStoreItem const& lItem : lTemplateA->Entries)
                dropMap->insert(std::make_pair(lItem.itemid, sEntry));
    }

    for (uint32 entry = 0; entry < sGOStorage.GetMaxEntry(); entry++)
    {
        sEntry = entry;

        LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, uint32(1)), LOOT_CORPSE);

        if (lTemplateA)
            for (LootStoreItem const& lItem : lTemplateA->Entries)
                dropMap->insert(std::make_pair(lItem.itemid, -sEntry));
    }

    sLog.outString("Loaded %d loot templates...", (uint32)dropMap->size());

    sLog.outString("Calculating stat weights for %d items...", sItemStorage.GetMaxEntry());
    BarGoLink bar(sItemStorage.GetMaxEntry());

    CharacterDatabase.BeginTransaction();

    // generate stat weights for classes/specs
    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        bar.step();

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        // skip non armor/weapon
        if (proto->Class != ITEM_CLASS_WEAPON &&
            proto->Class != ITEM_CLASS_ARMOR &&
            proto->Class != ITEM_CLASS_CONTAINER &&
            proto->Class != ITEM_CLASS_PROJECTILE)
            continue;

        if (!CanEquipItemNew(proto))
            continue;

        // skip test items
        if (strstr(proto->Name1, "(Test)") ||
            strstr(proto->Name1, "(TEST)") ||
            strstr(proto->Name1, "(test)") ||
            strstr(proto->Name1, "(JEFFTEST)") ||
            strstr(proto->Name1, "Test ") ||
            strstr(proto->Name1, "Test") ||
            strstr(proto->Name1, "TEST") ||
            strstr(proto->Name1, "TEST ") ||
            strstr(proto->Name1, " TEST") ||
            strstr(proto->Name1, "2200 ") ||
            strstr(proto->Name1, "Deprecated ") ||
            strstr(proto->Name1, "Unused ") ||
            strstr(proto->Name1, "Monster ") ||
            strstr(proto->Name1, "[PH]") ||
            strstr(proto->Name1, "(OLD)") ||
            strstr(proto->Name1, "zzz") ||
            strstr(proto->Name1, "ZZZ")
            )
            continue;

        // skip items with rank/rep requirements
        /*if (proto->RequiredHonorRank > 0 ||
            proto->RequiredSkillRank > 0 ||
            proto->RequiredCityRank > 0 ||
            proto->RequiredReputationRank > 0)
            continue;*/

        /*if (proto->RequiredHonorRank > 0 ||
            proto->RequiredSkillRank > 0 ||
            proto->RequiredCityRank > 0)
            continue;*/

#ifdef MANGOSBOT_TWO
        // skip heirloom items
        if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
            continue;
#endif

        // check possible equip slots
        EquipmentSlots slot = EQUIPMENT_SLOT_END;
        for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
        {
            std::set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
            if (slots.find((InventoryType)proto->InventoryType) != slots.end())
                slot = i->first;
        }

        if (slot == EQUIPMENT_SLOT_END)
            continue;

        // Init Item cache
        ItemInfoEntry* cacheInfo = new ItemInfoEntry;
        uint32 itemSpec = ITEM_SPEC_NONE;

#ifdef MANGOSBOT_TWO
        // check faction
        if (proto->Flags2 & ITEM_FLAG2_FACTION_HORDE)
            cacheInfo->team = HORDE;

        if (proto->Flags2 & ITEM_FLAG2_FACTION_ALLIANCE)
            cacheInfo->team = ALLIANCE;

        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }
#endif
#ifdef MANGOSBOT_ONE
        // check faction
        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }
#endif
#ifdef MANGOSBOT_ZERO
        // check faction
        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }

        // PvP vendors
        if (std::find(allianceItems.begin(), allianceItems.end(), proto->ItemId) != allianceItems.end())
            cacheInfo->team = ALLIANCE;
        if (std::find(hordeItems.begin(), hordeItems.end(), proto->ItemId) != hordeItems.end())
            cacheInfo->team = HORDE;
#endif

        // check min level
        if (proto->RequiredLevel)
            cacheInfo->minLevel = proto->RequiredLevel;

        // check item source

        if (proto->Flags & ITEM_FLAG_NO_DISENCHANT)
        {
            cacheInfo->source = ITEM_SOURCE_PVP;
            sLog.outDetail("Item: %d, source: PvP Reward", proto->ItemId);
        }

        // check quests
        if (cacheInfo->source == ITEM_SOURCE_NONE || cacheInfo->source == ITEM_SOURCE_PVP)
        {
            std::vector<uint32> questIds = GetQuestIdsForItem(proto->ItemId);
            if (questIds.size())
            {
                bool isAlly = false;
                bool isHorde = false;
                for (std::vector<uint32>::iterator i = questIds.begin(); i != questIds.end(); ++i)
                {
                    Quest const* quest = sObjectMgr.GetQuestTemplate(*i);
                    if (quest)
                    {
                        cacheInfo->source = ITEM_SOURCE_QUEST;
                        cacheInfo->sourceIds.push_back(*i);
                        if (!cacheInfo->minLevel)
                            cacheInfo->minLevel = quest->GetMinLevel();

                        // check quest team
                        if (!cacheInfo->team)
                        {
                            uint32 reqRace = quest->GetRequiredRaces();
                            if (reqRace)
                            {
                                if ((reqRace & RACEMASK_ALLIANCE) != 0)
                                    isAlly = true;
                                if ((reqRace & RACEMASK_HORDE) != 0)
                                    isHorde = true;
                            }
                        }

                        if (quest->GetRequiredMinRepFaction())
                        {
                            cacheInfo->repFaction = quest->GetRequiredMinRepFaction();
                            int r = 0;
                            for (; r < MAX_REPUTATION_RANK; ++r)
                            {
                                if (quest->GetRequiredMinRepValue() == ReputationMgr::PointsInRank[r])
                                    cacheInfo->repRank = uint32(r);
                            }
#ifdef MANGOSBOT_ONE
                            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(quest->GetRequiredMinRepFaction()))
#else
                            if (FactionEntry const* faction = sFactionStore.LookupEntry(quest->GetRequiredMinRepFaction()))
#endif
                            {
                                if (faction->team == ALLIANCE)
                                    cacheInfo->team = ALLIANCE;
                                if (faction->team == HORDE)
                                    cacheInfo->team = HORDE;
                            }
                        }
                    }
                }

                if (!cacheInfo->team)
                {
                    if (isAlly && isHorde)
                        cacheInfo->team = TEAM_BOTH_ALLOWED;
                    else if (isAlly)
                        cacheInfo->team = ALLIANCE;
                    else if (isHorde)
                        cacheInfo->team = HORDE;
                }

                sLog.outDetail("Item: %d, team (quest): %s", proto->ItemId, cacheInfo->team == ALLIANCE ? "Alliance" : cacheInfo->team == HORDE ? "Horde" : "Both");
                sLog.outDetail("Item: %d, source: quest %d, minlevel: %d", proto->ItemId, cacheInfo->sourceIds.front(), cacheInfo->minLevel);
            }
        }

        if (cacheInfo->minLevel)
            sLog.outDetail("Item: %d, minlevel: %d", proto->ItemId, cacheInfo->minLevel);

        // check vendors
        if (cacheInfo->source == ITEM_SOURCE_NONE || cacheInfo->source == ITEM_SOURCE_PVP)
        {
            bool isAlly = false;
            bool isHorde = false;
            for (auto& vendor : GAI_VALUE2(std::list<int32>, "item vendor list", itemId))
            {
                CreatureInfo const* cInfo = sObjectMgr.GetCreatureTemplate(vendor);
                if (!cInfo)
                    continue;

                cacheInfo->source = ITEM_SOURCE_VENDOR;
                cacheInfo->sourceIds.push_back(vendor);

                FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
                if (PlayerbotAI::friendToAlliance(factionEntry))
                    isAlly = true;
                if (PlayerbotAI::friendToHorde(factionEntry))
                    isHorde = true;

                // check faction conditions
                VendorItemData const* vItems = sObjectMgr.GetNpcVendorItemList(vendor);
                VendorItemData const* tItems = sObjectMgr.GetNpcVendorTemplateItemList(cInfo->VendorTemplateId);

                if (vItems || tItems)
                {
                    uint8 customitems = vItems ? vItems->GetItemCount() : 0;
                    uint8 numitems = customitems + (tItems ? tItems->GetItemCount() : 0);
                    for (int i = 0; i < numitems; ++i)
                    {
                        VendorItem const* crItem = i < customitems ? vItems->GetItem(i) : tItems->GetItem(i - customitems);
                        if (!crItem || !crItem->conditionId)
                            continue;

                        if (auto result = WorldDatabase.PQuery("SELECT type, value1, value2 FROM conditions WHERE condition_entry = '%u'", crItem->conditionId))
                        {
                            do
                            {
                                Field *fields = result->Fetch();
                                uint32 m_type = fields[0].GetUInt32();
                                if (m_type != CONDITION_REPUTATION_RANK_MIN)
                                    continue;

                                uint32 m_value1 = fields[1].GetUInt32();
                                uint32 m_value2 = fields[2].GetUInt32();

#ifdef MANGOSBOT_ONE
                                if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(m_value1))
#else
                                if (FactionEntry const* faction = sFactionStore.LookupEntry(m_value1))
#endif
                                {
                                    cacheInfo->repFaction = m_value1;
                                    cacheInfo->repRank = m_value2;
                                }
                            } while (result->NextRow());
                        }
                    }
                }
            }

            if (!cacheInfo->team)
            {
                if (isAlly && isHorde)
                    cacheInfo->team = TEAM_BOTH_ALLOWED;
                else if (isAlly)
                    cacheInfo->team = ALLIANCE;
                else if (isHorde)
                    cacheInfo->team = HORDE;
            }

            if (cacheInfo->source == ITEM_SOURCE_VENDOR)
                sLog.outDetail("Item: %d, source: vendor", proto->ItemId);
        }

        if (cacheInfo->team)
            sLog.outDetail("Item: %d, team (item): %s", proto->ItemId, cacheInfo->team == ALLIANCE ? "Alliance" : "Horde");

        // check drops
        std::list<int32> creatures;
        std::list<int32> gameobjects;

        auto range = dropMap->equal_range(itemId);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second > 0)
                creatures.push_back(itr->second);
            else
                gameobjects.push_back(abs(itr->second));
        }

        // check creature drop
        if (cacheInfo->source == ITEM_SOURCE_NONE)
        {
            if (creatures.size())
            {
                if (creatures.size() == 1)
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    cacheInfo->sourceIds.push_back(creatures.front());
                    sLog.outDetail("Item: %d, source: creature drop, ID: %d", proto->ItemId, creatures.front());
                }
                else
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    sLog.outDetail("Item: %d, source: creatures drop, number: %d", proto->ItemId, (uint32)creatures.size());
                }
            }
        }

        // check gameobject drop
        if (cacheInfo->source == ITEM_SOURCE_NONE || (cacheInfo->source == ITEM_SOURCE_DROP && cacheInfo->sourceIds.empty()))
        {
            if (gameobjects.size())
            {
                if (gameobjects.size() == 1)
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    cacheInfo->sourceIds.push_back(gameobjects.front());
                    sLog.outDetail("Item: %d, source: gameobject, ID: %d", proto->ItemId, gameobjects.front());
                }
                else
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    sLog.outDetail("Item: %d, source: gameobjects, number: %d", proto->ItemId, (uint32)gameobjects.size());
                }
            }
        }

        // check faction
        if (proto->RequiredReputationFaction > 0 && proto->RequiredReputationFaction != 35 && proto->RequiredReputationRank < 15)
        {
            cacheInfo->repFaction = proto->RequiredReputationFaction;
            cacheInfo->repRank = proto->RequiredReputationRank;
        }

        // check honor rank
        if (proto->RequiredHonorRank > 0)
        {
            cacheInfo->pvpRank = proto->RequiredHonorRank;
        }

        // check skill
        if (proto->RequiredSkill > 0)
        {
            cacheInfo->reqSkill = proto->RequiredSkill;
            cacheInfo->reqSkillRank = proto->RequiredSkillRank;
        }

        cacheInfo->quality = proto->Quality;
        cacheInfo->itemId = proto->ItemId;
        cacheInfo->slot = slot;
        cacheInfo->itemLevel = proto->ItemLevel;

        // calculate stat weights
        for (uint8 clazz = CLASS_WARRIOR; clazz < MAX_CLASSES; ++clazz)
        {
            // skip nonexistent classes
            if (!((1 << (clazz - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(clazz))
                continue;

            // skip wrong classes
            if ((proto->AllowableClass & (1 << (clazz - 1))) == 0)
                continue;

            for (uint32 spec = 1; spec <= MAX_STAT_SCALES; ++spec)
            {
                if (!m_weightScales[spec].info.id)
                    continue;

                if (m_weightScales[spec].info.classId != clazz)
                    continue;

                // check possible armor for spec
                if (proto->Class == ITEM_CLASS_ARMOR && (
                    slot == EQUIPMENT_SLOT_HEAD ||
                    slot == EQUIPMENT_SLOT_SHOULDERS ||
                    slot == EQUIPMENT_SLOT_CHEST ||
                    slot == EQUIPMENT_SLOT_WAIST ||
                    slot == EQUIPMENT_SLOT_LEGS ||
                    slot == EQUIPMENT_SLOT_FEET ||
                    slot == EQUIPMENT_SLOT_WRISTS ||
                    slot == EQUIPMENT_SLOT_HANDS) &&
                    !ShouldEquipArmorForSpec(clazz, spec, proto))
                    continue;

                // check possible weapon for spec
                if ((proto->Class == ITEM_CLASS_WEAPON || (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD || (proto->SubClass == ITEM_SUBCLASS_ARMOR_MISC && proto->InventoryType == INVTYPE_HOLDABLE))) &&
                    !ShouldEquipWeaponForSpec(clazz, spec, proto))
                    continue;

                //StatWeight statWeight;
                //statWeight.id = m_weightScales[spec].info.id;
                ItemSpecType tempSpec = ITEM_SPEC_NONE;
                uint32 statW = CalculateStatWeight(clazz, spec, proto, tempSpec);
                itemSpec |= tempSpec;
                if (!statW /*&& tempSpec == ITEM_SPEC_NONE*/ && cacheInfo->quality < ITEM_QUALITY_UNCOMMON && cacheInfo->minLevel <= 10)
                    statW = 1;

                // legendary stat x2
                if (proto->Quality >= ITEM_QUALITY_LEGENDARY && statW > 1)
                    statW *= 2;

                if (slot == EQUIPMENT_SLOT_BODY && statW <= 0)
                    statW = 1;

                if (slot == EQUIPMENT_SLOT_TABARD && statW <= 0)
                    statW = 1;

                // warriors only plate >= 40 lvl
                if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && cacheInfo->minLevel >= 40 && clazz == CLASS_WARRIOR)
                    statW = 0;

                // paladin tank/dps only plate >= 40 lvl
                if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && cacheInfo->minLevel >= 40 && clazz == CLASS_PALADIN && spec != 4)
                    statW = 0;

                // some trinkets have no stats
                if (cacheInfo->slot == EQUIPMENT_SLOT_TRINKET1 ||
                    cacheInfo->slot == EQUIPMENT_SLOT_TRINKET2)
                {
                    if (statW == 0 && proto->AllowableClass == uint32(clazz) && proto->Spells[0].SpellId)
                    {
                        statW = (uint32)(proto->Quality + proto->ItemLevel);
                    }
                }

                // Make wand useful
                if (!statW && cacheInfo->slot == EQUIPMENT_SLOT_RANGED && proto->SubClass == ITEM_SUBCLASS_WEAPON_WAND && (clazz == CLASS_PRIEST || clazz == CLASS_MAGE || clazz == CLASS_WARLOCK))
                    statW = 1;

                // Random properties
                if (!statW && proto->RandomProperty)
                    statW = 1;

                // set stat weight = 1 for items that can be equipped but have no proper stats
                //statWeight.weight = statW;
                // save item statWeight into ItemCache
                cacheInfo->weights[spec] = statW;
                sLog.outDetail("Item: %d, weight: %d, class: %d, spec: %s", proto->ItemId, statW, clazz, m_weightScales[spec].info.name.c_str());
            }
        }

        cacheInfo->itemSpec = (ItemSpecType)itemSpec;

        // save cache
        static SqlStatementID delCache;
        static SqlStatementID insertCache;

        SqlStatement stmt = CharacterDatabase.CreateStatement(delCache, "DELETE FROM ai_playerbot_item_info_cache WHERE id = ?");
        stmt.PExecute(proto->ItemId);

        stmt = CharacterDatabase.CreateStatement(insertCache, "INSERT INTO ai_playerbot_item_info_cache (id, quality, slot, source, sourceId, team, faction, factionRepRank, minLevel, "
            "scale_1, scale_2, scale_3, scale_4, scale_5, scale_6, scale_7, scale_8, scale_9, scale_10, scale_11, scale_12, scale_13, scale_14, scale_15, "
            "scale_16, scale_17, scale_18, scale_19, scale_20, scale_21, scale_22, scale_23, scale_24, scale_25, scale_26, scale_27, scale_28, scale_29, scale_30, scale_31, scale_32)"
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        stmt.addUInt32(cacheInfo->itemId);
        stmt.addUInt32(cacheInfo->quality);
        stmt.addUInt32(cacheInfo->slot);
        stmt.addUInt32(cacheInfo->source);
        stmt.addUInt32(cacheInfo->sourceIds.empty() ? 0 : cacheInfo->sourceIds.front());
        stmt.addUInt32(cacheInfo->team);
        stmt.addUInt32(cacheInfo->repFaction);
        stmt.addUInt32(cacheInfo->repRank);
        stmt.addUInt32(cacheInfo->minLevel);

        for (int i = 1; i <= MAX_STAT_SCALES; ++i)
        {
            if (cacheInfo->weights[i])
                stmt.addUInt32(cacheInfo->weights[i]);
            else
                stmt.addUInt32(0);
        }

        stmt.Execute();

        itemInfoCache[cacheInfo->itemId] = cacheInfo;
    }

    CharacterDatabase.CommitTransaction();
    delete dropMap;
}

uint32 RandomItemMgr::CalculateStatWeight(uint8 playerclass, uint8 spec, ItemPrototype const* proto, ItemSpecType& itSpec)
{
    uint32 specType = ITEM_SPEC_NONE;
    uint32 statWeight = 0;
    uint32 spellPower = 0;
    uint32 spellHeal = 0;
    uint32 attackPower = 0;
    bool isCasterItem = false;
    bool isAttackItem = false;
    bool isDpsItem = false;
    bool isTankItem = false;
    bool isHealingItem = false;
    bool isSpellDamageItem = false;
    bool hasInt = false;
#ifdef MANGOSBOT_TWO
    bool noCaster = (Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_DEATH_KNIGHT || (Classes)playerclass == CLASS_HUNTER || spec == 30 || spec == 32 || spec == 21 || spec == 6;
    bool hasMana = !((Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_DEATH_KNIGHT);

    if (!proto->IsWeapon() && (proto->SubClass == ITEM_SUBCLASS_ARMOR_LIBRAM || proto->SubClass == ITEM_SUBCLASS_ARMOR_IDOL || proto->SubClass == ITEM_SUBCLASS_ARMOR_TOTEM || proto->SubClass == ITEM_SUBCLASS_ARMOR_SIGIL))
        return (uint32)(proto->Quality + proto->ItemLevel);
#else
    bool noCaster = (Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_HUNTER || spec == 30 || spec == 32 || spec == 21 || spec == 6;
    bool hasMana = !((Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE);

    if ((Classes)playerclass == CLASS_HUNTER && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN)
        return (uint32)proto->ItemLevel;
#endif

    //check relicts
    if (proto->InventoryType == INVTYPE_RELIC)
    {
        if (playerclass == CLASS_PALADIN && proto->SubClass != ITEM_SUBCLASS_ARMOR_LIBRAM)
            return 0;

        if (playerclass == CLASS_DRUID && proto->SubClass != ITEM_SUBCLASS_ARMOR_IDOL)
            return 0;

        if (playerclass == CLASS_SHAMAN && proto->SubClass != ITEM_SUBCLASS_ARMOR_TOTEM)
            return 0;

        if (playerclass == CLASS_WARRIOR
            || playerclass == CLASS_HUNTER
            || playerclass == CLASS_ROGUE
            || playerclass == CLASS_PRIEST
            || playerclass == CLASS_MAGE
            || playerclass == CLASS_WARLOCK)
            return 0;

        return proto->Quality + proto->ItemLevel;
    }

    bool isWhitelist = false;
    // whitelist
    if (std::find(sPlayerbotAIConfig.randomGearWhitelist.begin(), sPlayerbotAIConfig.randomGearWhitelist.end(), proto->ItemId) != sPlayerbotAIConfig.randomGearWhitelist.end())
        isWhitelist = true;

#ifdef MANGOSBOT_ZERO
    // whitelist pvp items, as thei have wierd stats
    if (proto->RequiredHonorRank)
        isWhitelist = true;

    // whitelist the ONLY feral Off Hand in vanilla
    if ((spec == 30 || spec == 32) && proto->ItemId == 13385)
        isWhitelist = true;
#endif

    // whitelist atiesh
    if (playerclass == CLASS_MAGE && proto->ItemId == 22589)
        isWhitelist = true;
    if (playerclass == CLASS_WARLOCK && proto->ItemId == 22630)
        isWhitelist = true;
    if (playerclass == CLASS_PRIEST && proto->ItemId == 22631)
        isWhitelist = true;
    if (playerclass == CLASS_DRUID && proto->ItemId == 22632)
        isWhitelist = true;

    // check basic item stats
    int32 basicStatsWeight = 0;
    for (int j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        uint32 statType = 0;
        int32 val = 0;
        std::string weightName = "";

        //if (j >= proto->StatsCount)
        //    continue;

        statType = proto->ItemStat[j].ItemStatType;
        val = proto->ItemStat[j].ItemStatValue;

        if (val == 0)
            continue;

        for (std::map<std::string, uint32 >::iterator i = weightStatLink.begin(); i != weightStatLink.end(); ++i)
        {
            uint32 modd = i->second;
            if (modd == statType)
            {
                weightName = i->first;

#ifndef MANGOSBOT_ZERO
                // mark tank item (skip items with def stats for dps)
                if (modd == ITEM_MOD_DODGE_RATING || modd == ITEM_MOD_PARRY_RATING || modd == ITEM_MOD_BLOCK_RATING || modd == ITEM_MOD_DEFENSE_SKILL_RATING)
                    isTankItem = true;

                if (modd == ITEM_MOD_CRIT_MELEE_RATING || modd == ITEM_MOD_CRIT_RATING)
                    isDpsItem = true;

#ifdef MANGOSBOT_TWO
                if (modd == ITEM_MOD_SPELL_POWER)
                {
                    isCasterItem = true;
                    isHealingItem = true;
                    isSpellDamageItem = true;
                }
#endif
#endif

                break;
            }
        }

        if (weightName.empty())
            continue;

        uint32 singleStat = CalculateSingleStatWeight(playerclass, spec, weightName, val);
        basicStatsWeight += singleStat;

        if (val)
        {
            if (weightName == "int" && !noCaster)
                isCasterItem = true;

            if (weightName == "int")
                hasInt = true;

            if (weightName == "splpwr")
                isCasterItem = true;

            if (weightName == "str")
                isAttackItem = true;

            if (weightName == "agi")
                isAttackItem = true;

            if (weightName == "atkpwr")
                isAttackItem = true;
        }
    }

    // check defensive stats
    uint32 defenseStats = 0;
    defenseStats += CalculateSingleStatWeight(playerclass, spec, "block", proto->Block);
    defenseStats += CalculateSingleStatWeight(playerclass, spec, "armor", proto->Armor);

    // check weapon dps
    if (proto->IsWeapon())
    {
        WeaponAttackType attType = BASE_ATTACK;

        uint32 dps = 0;

        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
        {
            if (proto->Damage[i].DamageMax == 0)
                break;

            dps = (proto->Damage[i].DamageMin + proto->Damage[i].DamageMax) / (float)(proto->Delay / 1000.0f) / 2;
            if (dps)
            {
                if (proto->IsRangedWeapon())
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "rgddps", dps);
                else
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "mledps", dps);
            }
        }
    }

    // check item spells
    uint32 spellDamage = 0;
    uint32 spellHealing = 0;
    uint32 auraStatWeight = 0;
    uint32 auraApStatWeight = 0;
    uint32 auraHealStatWeight = 0;
    uint32 auraDamageStatWeight = 0;
    bool isFeral = false;
    for (const auto& spellData : proto->Spells)
    {
        // no spell
        if (!spellData.SpellId)
            continue;

        // apply only at-equip spells for weapons, on use/hit for armor
        if (!(spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP || (!proto->IsWeapon() && proto->InventoryType != INVTYPE_HOLDABLE && (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE || spellData.SpellTrigger == ITEM_SPELLTRIGGER_CHANCE_ON_HIT))))
            continue;

        // check if it is valid spell
        SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellproto)
            continue;

        bool hasAP = false;

        uint32 effectAuraStatWeight = 0;
        uint32 effectAuraApStatWeight = 0;
        uint32 effectAuraHealStatWeight = 0;
        uint32 effectAuraDamageStatWeight = 0;

        for (uint8 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((spellproto->Effect[j] == SPELL_EFFECT_APPLY_AURA) &&
                (spellproto->EffectBasePoints[j] >= 0))
            {
#ifdef MANGOSBOT_TWO
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    isSpellDamageItem = true;
                    isCasterItem = true;
                    spellDamage = spellproto->EffectBasePoints[j] + 1;

                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        isHealingItem = true;
                        effectAuraDamageStatWeight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        if (!isWhitelist && !specialDamage && isSpellDamageItem)
                            return 0;

                        effectAuraDamageStatWeight += specialDamage;
                    }
                }
#else
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    spellDamage = spellproto->EffectBasePoints[j] + 1;
                    isSpellDamageItem = true;
                    // generic spell damage
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        effectAuraDamageStatWeight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        effectAuraDamageStatWeight += specialDamage;
                    }
                }
                // spell healing
                // SPELL_AURA_MOD_HEALING_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE)
                {
                    isHealingItem = true;
                    spellHealing = spellproto->EffectBasePoints[j] + 1;
                    effectAuraHealStatWeight += CalculateSingleStatWeight(playerclass, spec, "splheal", spellproto->EffectBasePoints[j] + 1);
                }
#endif

                // spell hit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_HIT_CHANCE)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellhitrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // spell crit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_CRIT_CHANCE, SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_CRIT_CHANCE || spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellcritstrkrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // spell penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if magic type
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_SPELL)
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellpenrtng", abs(spellproto->EffectBasePoints[j] + 1));
                }

                // check attack power
                if (!hasAP && spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_ATTACK_POWER)
                {
                    hasAP = true;
                    isAttackItem = true;
                    std::string SpellName = spellproto->SpellName[0];
                    if (SpellName.find("Attack Power - Feral") != std::string::npos)
                        isFeral = true;
#ifdef MANGOSBOT_ZERO
                    if (!isWhitelist && isFeral && (playerclass != CLASS_DRUID && playerclass != CLASS_WARRIOR && playerclass != CLASS_PALADIN && proto->IsWeapon()))
                        return 0;
#else
                    if (!isWhitelist && isFeral && playerclass != CLASS_DRUID)
                        return 0;
#endif

                    effectAuraApStatWeight += CalculateSingleStatWeight(playerclass, spec, isFeral ? "feratkpwr" : "atkpwr", spellproto->EffectBasePoints[j] + 1);
                }

                // check ranged ap
                // SPELL_AURA_MOD_RANGED_ATTACK_POWER
                if (!hasAP && spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_RANGED_ATTACK_POWER)
                {
                    // filter non ranged classes
                    if (playerclass == CLASS_SHAMAN || (!proto->IsRangedWeapon() && playerclass != CLASS_HUNTER))
                        return 0;

                    hasAP = true;
                    isAttackItem = true;
                    effectAuraApStatWeight += CalculateSingleStatWeight(playerclass, spec, "atkpwr", spellproto->EffectBasePoints[j] + 1);
                }

                // check block
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SHIELD_BLOCKVALUE)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "block", spellproto->EffectBasePoints[j] + 1);
                }

                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_PARRY_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "parryrtng", spellproto->EffectBasePoints[j] + 1);
                }

                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DODGE_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "dodgertng", spellproto->EffectBasePoints[j] + 1);
                }

                // block chance
                // SPELL_AURA_MOD_BLOCK_PERCENT
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_BLOCK_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "blockrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // armor penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if physical type
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_NORMAL)
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "armorpenrtng", abs(spellproto->EffectBasePoints[j] + 1));
                }

                // hit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_HIT_CHANCE)
                {
                    isAttackItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "hitrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // crit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_CRIT_PERCENT)
                {
                    isAttackItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "critstrkrtng", spellproto->EffectBasePoints[j] + 1);
                }

                //check defense SPELL_AURA_MOD_SKILL
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SKILL)
                {
                    if (spellproto->EffectMiscValue[j] == SKILL_DEFENSE)
                    {
                        isTankItem = true;
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "defrtng", spellproto->EffectBasePoints[j] + 1);
                    }
                }

#ifndef MANGOSBOT_ZERO
                // ratings
                // enum CombatRating
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_RATING)
                {
                    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
                    {
                        if (spellproto->EffectMiscValue[0] & (1 << rating))
                        {
                            int32 val = spellproto->EffectBasePoints[j] + 1;
                            std::string weightName;

                            for (std::map<std::string, uint32 >::iterator i = weightRatingLink.begin(); i != weightRatingLink.end(); ++i)
                            {
                                uint32 modd = i->second;
                                if (modd == rating)
                                {
                                    weightName = i->first;
                                    // mark tank item (skip items with def stats for dps)
                                    if (modd == CR_DODGE || modd == CR_PARRY || modd == CR_BLOCK || modd == CR_DEFENSE_SKILL)
                                        isTankItem = true;

                                    if (modd == CR_CRIT_MELEE || modd == CR_CRIT_RANGED)
                                        isDpsItem = true;

                                    break;
                                }
                            }

                            if (weightName.empty())
                                continue;

                            effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, weightName, val);
                        }
                    }
                }
#endif

                // mana regen
                // SPELL_AURA_MOD_POWER_REGEN
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_POWER_REGEN)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "manargn", spellproto->EffectBasePoints[j] + 1);
                }
            }
        }

        // different stat weight based on trigger
        float coverage = 1;

        if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            if (spellData.SpellCooldown != 0)
            {
                int32 spellDuration = GetSpellDuration(spellproto);
                coverage = static_cast<float>(spellDuration) / spellData.SpellCooldown;
            }
            else
            {
                coverage = 0.17f; //Most often trinkets have 20 seconds buff with 2 minute cooldown which means ~17%
            }
        }
        else if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
        {
#ifdef MANGOSBOT_ZERO
            float averageItemDelay = 2.43f;
#endif
#ifdef MANGOSBOT_ONE
            float averageItemDelay = 2.09f;
#endif
#ifdef MANGOSBOT_TWO
            float averageItemDelay = 2.39f;
#endif
            coverage = (static_cast<float>(spellproto->procChance) / 100) * (static_cast<float>(GetSpellDuration(spellproto)) / averageItemDelay);

            if (coverage > 0.9f)
                coverage = 0.9f;
        }

        effectAuraStatWeight *= coverage;
        effectAuraHealStatWeight *= coverage;
        effectAuraApStatWeight *= coverage;
        effectAuraDamageStatWeight *= coverage;

        auraStatWeight += effectAuraStatWeight;
        auraHealStatWeight += effectAuraHealStatWeight;
        auraApStatWeight += effectAuraApStatWeight;
        auraDamageStatWeight += effectAuraDamageStatWeight;
    }

#ifdef MANGOSBOT_ZERO
    // skip all 1h Maces for feral druids if they have no feral AP
    if (!isWhitelist && !isFeral && playerclass == CLASS_DRUID && proto->IsWeapon() && proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE && (spec == 30 || spec == 32))
        return 0;
#endif

    statWeight += auraStatWeight;
    spellHeal += auraHealStatWeight;
    spellPower += auraDamageStatWeight;
    attackPower += auraApStatWeight;

    uint32 socketBonus = 0;
#ifndef MANGOSBOT_ZERO
    // check sockets
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (!proto->Socket[i].Color)
            continue;

        if (proto->Socket[i].Color == SOCKET_COLOR_META)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "metasocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_YELLOW)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "yellowsocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_BLUE)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "bluesocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_RED)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "redsocket", 1);
    }
#endif

#ifdef MANGOSBOT_ONE
    // filter healing items in tbc as they also have spell damage
    if (spellDamage && spellHealing && isSpellDamageItem && isHealingItem && spellHealing > spellDamage)
    {
        isSpellDamageItem = false;
        isHealingItem = true;
    }
#endif
#ifdef MANGOSBOT_TWO
    if (spellHeal || spellPower)
        specType |= ITEM_SPEC_SPELL_HEALING | ITEM_SPEC_SPELL_DAMAGE;
#else
    if (spellHeal > spellPower || isHealingItem)
        specType |= ITEM_SPEC_SPELL_HEALING;

    if (spellPower >= spellHeal)
        specType |= ITEM_SPEC_SPELL_DAMAGE;
#endif

    if (isTankItem && (noCaster || !hasMana || !spellHeal || (!isHealingItem && !isSpellDamageItem)))
        specType |= ITEM_SPEC_TANK;

    if (isAttackItem)
        specType |= ITEM_SPEC_ATTACK;

    if (!noCaster && (isCasterItem || hasInt || isSpellDamageItem))
        specType |= ITEM_SPEC_CASTER;

#ifdef MANGOSBOT_ONE
    // retribution should not use items with spell power in 61+
    if ((spec == 6 || spec == 21) && (isSpellDamageItem || spellDamage || spellHealing || spellHeal))
        return 0;

    // filter tanking items (dodge, defense, parry, block) from dps classes
    if (proto->RequiredLevel > 60 && isTankItem && !(spec == 30 || spec == 3 || spec == 5))
        return 0;

    if (proto->RequiredLevel > 60 && isDpsItem && (spec == 30 || spec == 3 || spec == 5))
        return 0;
#endif
#ifdef MANGOSBOT_TWO
    // retribution should not use items with spell power in 61+
    if ((spec == 6 || spec == 21) && (isSpellDamageItem || spellDamage || spellHealing || spellHeal))
        return 0;

    // filter tanking items (dodge, defense, parry, block) from dps classes
    if (proto->RequiredLevel > 60 && isTankItem && !(spec == 30 || spec == 3 || spec == 5 || spec == 18))
        return 0;

    if (proto->RequiredLevel > 60 && isDpsItem && (spec == 30 || spec == 3 || spec == 5 || spec == 18))
        return 0;
#endif

    // limit speed for tank weapons
    if (!isWhitelist && spec == 3 && proto->IsWeapon() && proto->Delay > 2300)
        return 0;

    if (!isWhitelist && spec == 5 && proto->IsWeapon() && proto->Delay > 2400)
        return 0;

    // check for caster item
    if (isCasterItem || hasInt || spellHeal || spellPower || isSpellDamageItem || isHealingItem)
    {
        if (!isWhitelist && (!hasMana || (noCaster && !(spec == 6 || spec == 30 || spec == 32 || spec == 21))) && (spellHeal || isHealingItem || isSpellDamageItem || spellPower))
            return 0;

        if (!isWhitelist && !hasMana && hasInt)
            return 0;

        if (!isWhitelist && !hasMana && noCaster && (spellPower > attackPower || spellHeal > attackPower))
            return 0;

#ifndef MANGOSBOT_TWO
        if (!isWhitelist && (spec != 6 && spec != 21) && !spellPower && !spellHeal && isSpellDamageItem)
            return 0;

        if (!isWhitelist && /*(spec != 6 && spec != 21) && */!spellHeal && isHealingItem && !isSpellDamageItem)
            return 0;
#endif

        if (!isWhitelist && (spec != 6 && spec != 21) && !noCaster && isSpellDamageItem && !spellPower && !(spellDamage && spellHealing && proto->IsWeapon() && proto->InventoryType == INVTYPE_WEAPONMAINHAND))
            return 0;

        bool playerCaster = false;
        for (std::vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
        {
            if (i->stat == "splpwr" || i->stat == "int" || i->stat == "manargn" || i->stat == "splheal" || i->stat == "spellcritstrkrtng" || i->stat == "spellhitrtng")
            {
                playerCaster = true;
            }
        }

        if (!isWhitelist && (spec != 6 && spec != 21 && playerclass != CLASS_HUNTER) && !playerCaster)
            return 0;
    }

    // check for caster item
    if (isAttackItem)
    {
        if (!isWhitelist && hasMana && !noCaster && !(hasInt || spellPower || spellHeal || isHealingItem || isSpellDamageItem))
            return 0;

        bool playerAttacker = false;
        for (std::vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
        {
            if (i->stat == "str" || i->stat == "agi" || i->stat == "atkpwr" || i->stat == "mledps" || i->stat == "rgddps" || i->stat == "hitrtng" || i->stat == "critstrkrtng")
            {
                playerAttacker = true;
            }
        }

        if (!isWhitelist && !playerAttacker)
            return 0;
    }


    itSpec = (ItemSpecType)specType;

    statWeight += spellPower;
    statWeight += spellHeal;
    statWeight += attackPower;
    statWeight += defenseStats;

    // if stat value consists of only socket bonuses - skip
    if (socketBonus && !(statWeight || basicStatsWeight))
        return 0;

    statWeight += socketBonus;

    // handle negative stats
    if (basicStatsWeight < 0 && ((uint32)(abs(basicStatsWeight)) >= statWeight))
        statWeight = 0;
    else
        statWeight += basicStatsWeight;

    return statWeight;
}

uint32 RandomItemMgr::CalculateRandomEnchantId(uint8 playerclass, uint8 spec, ItemPrototype const* proto)
{
    if (!proto)
        return 0;

    // Random Property case
    if (proto->RandomProperty)
    {
        uint32 randomPropId = GetItemEnchantMod(proto->RandomProperty);
        ItemRandomPropertiesEntry const* random_id = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if (!random_id)
        {
            sLog.outErrorDb("Enchantment id #%u used but it doesn't have records in 'ItemRandomProperties.dbc'", randomPropId);
            return 0;
        }

        // check stats
        if (CalculateEnchantWeight(playerclass, spec, random_id->ID))
            return random_id->ID;
    }

    return 0;
}

uint32 RandomItemMgr::CalculateBestRandomEnchantId(uint8 playerclass, uint8 spec, uint32 itemId)
{
    if (!itemId)
        return 0;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    if (!proto)
        return 0;

    std::map<uint32, std::vector<uint32> >::const_iterator tab = randomEnchantsCache.find(proto->RandomProperty);
    if (tab == randomEnchantsCache.end())
        return 0;

    uint32 bestScore = 0;
    uint32 bestId = 0;

    const std::vector<uint32> propList = tab->second;
    for (auto propId : propList)
    {
        ItemRandomPropertiesEntry const* random_id = sItemRandomPropertiesStore.LookupEntry(propId);
        if (!random_id)
            continue;

        uint32 currScore = 0;
        for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < PROP_ENCHANTMENT_SLOT_0 + 3; ++i)
        {
            uint32 enchantId = random_id->enchant_id[i - PROP_ENCHANTMENT_SLOT_0];
            currScore += CalculateEnchantWeight(playerclass, spec, enchantId);
        }

        if (currScore > bestScore)
        {
            bestScore = currScore;
            bestId = random_id->ID;;
        }
    }

    return bestId;
}

uint32 RandomItemMgr::CalculateEnchantWeight(uint8 playerclass, uint8 spec, uint32 enchantId)
{
    if (!enchantId)
        return 0;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);

    if (!pEnchant)
        return 0;

    uint32 weight = 0;

    for (int s = 0; s < 3; ++s)
    {
        switch (pEnchant->type[s])
        {
        case 1: //Proc //TODO add proc values?
            break;
        case 2: //Damage
            if (!pEnchant->amount[s])
                continue;
            weight += CalculateSingleStatWeight(playerclass, spec, "mledps", pEnchant->amount[s]);
            break;
        case 3:
        {
            if (!pEnchant->spellid[s])
                continue;

            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(pEnchant->spellid[s]);

            if (!spellInfo)
                continue;

            for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
            {
                if (spellInfo->Effect[j] != SPELL_EFFECT_APPLY_AURA)
                    continue;

                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_STAT)
                {
                    uint32 stat = spellInfo->EffectMiscValue[j];
                    uint32 value = spellInfo->EffectBasePoints[j] + 1;

                    if (!value)
                        continue;

                    if (ItemStatLink.find(stat) == ItemStatLink.end())
                        continue;

                    weight += CalculateSingleStatWeight(playerclass, spec, ItemStatLink[stat], value);
                }
#ifdef MANGOSBOT_TWO
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    uint32 spellDamage = spellInfo->EffectBasePoints[j] + 1;

                    if (spellInfo->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        weight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        weight += specialDamage;
                    }
                }
#else
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    uint32 spellDamage = spellInfo->EffectBasePoints[j] + 1;
                    // generic spell damage
                    if (spellInfo->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        weight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        weight += specialDamage;
                    }
                }
                // spell healing
                // SPELL_AURA_MOD_HEALING_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE)
                {
                    weight += CalculateSingleStatWeight(playerclass, spec, "splheal", spellInfo->EffectBasePoints[j] + 1);
                }
#endif


            }
            break;
        }
        case 4: //Armor
            if (!pEnchant->amount[s])
                continue;
            weight += CalculateSingleStatWeight(playerclass, spec, "armor", pEnchant->amount[s]);
            break;
        case 5: //Stat
        {
            for (auto& statLink : weightStatLink)
            {
                if (statLink.second != pEnchant->spellid[s])
                    continue;

                weight += CalculateSingleStatWeight(playerclass, spec, statLink.first, pEnchant->amount[s]);
            }
            break;
        }
        case 6: //Totem
            break;
        case 7: //Use Spell
            break;
        case 8: //Prismatic socket
            break;
        }
    }

    return weight;
}


uint32 RandomItemMgr::CalculateRandomPropertyWeight(uint8 playerclass, uint8 spec, int32 randomPropertyId)
{
    uint32 weight = 0;
    if (randomPropertyId)
    {
        ItemRandomPropertiesEntry const* item_rand = sItemRandomPropertiesStore.LookupEntry(abs(randomPropertyId));
        if (item_rand)
        {
            for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < PROP_ENCHANTMENT_SLOT_0 + 3; ++i)
            {
                uint32 enchantId = item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_0];

                weight += CalculateEnchantWeight(playerclass, spec, enchantId);
            }
        }
    }
    return weight;
}

uint32 RandomItemMgr::CalculateGemWeight(uint8 playerclass, uint8 spec, uint32 gemId)
{
#ifdef MANGOSBOT_ZERO
    return 0;
#else
    if (!gemId)
        return 0;

    return CalculateEnchantWeight(playerclass, spec, gemId);
#endif
}

uint32 RandomItemMgr::CalculateSocketWeight(uint8 playerclass, ItemQualifier& qualifier, uint8 spec)
{
#ifdef MANGOSBOT_ZERO
    return 0;
#else

    std::vector<uint32> gems = { qualifier.GetGem1() , qualifier.GetGem2(), qualifier.GetGem3(), qualifier.GetGem4() };

    ItemPrototype const* proto = qualifier.GetProto();

    bool hasGem = false;
    for (auto gem : gems)
        if(gem)
            hasGem = true;

    if (!hasGem)
        return 0;

    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        uint8 SocketColor = proto->Socket[i].Color;

        if (!SocketColor)
            continue;

        uint8 GemColor = 0;

        if (!gems[i])
            return 0;

        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(gems[i]);

        if (!pEnchant)
            return 0;

        uint32 gemid = pEnchant->GemID;
        if (!gemid)
            return 0;

        ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
        if (!gemProto)
            return 0;

        GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);

        if (!gemProperty)
            return 0;

        GemColor = gemProperty->color;

        if (!(GemColor & SocketColor))
            return 0;
    }

    return CalculateEnchantWeight(playerclass, spec, proto->socketBonus);
#endif
}


uint32 RandomItemMgr::ItemStatWeight(Player* player, ItemQualifier& qualifier)
{
    ItemSpecType itSpec;
    uint32 weight = CalculateStatWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetProto(), itSpec);
    if(qualifier.GetEnchantId())
        weight += CalculateEnchantWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetEnchantId());
    if (qualifier.GetRandomPropertyId())
        weight += CalculateRandomPropertyWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetRandomPropertyId());
    if(qualifier.GetGem1())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem1());
    if (qualifier.GetGem2())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem2());
    if (qualifier.GetGem3())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem3());
    if (qualifier.GetGem4())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem4());

    weight += CalculateSocketWeight(player->getClass(), qualifier, GetPlayerSpecId(player));

    return weight;
}

uint32 RandomItemMgr::ItemStatWeight(Player* player, Item* item)
{
    ItemQualifier itemQualifier(item);
    return ItemStatWeight(player, itemQualifier);
}

uint32 RandomItemMgr::CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value)
{
    uint32 statWeight = 0;
    for (std::vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
    {
        if (stat == i->stat)
        {
            statWeight = i->weight * value;
            if (statWeight)
                sLog.outDetail("stat: %s, val: %d, weight: %d, total: %d, class: %d, spec: %s", stat.c_str(), value, i->weight, statWeight, playerclass, m_weightScales[spec].info.name.c_str());
            return statWeight;
        }
    }

    return statWeight;
}

bool CheckItemSpec(uint8 spec, ItemSpecType itSpec)
{
    return false;
}

uint32 RandomItemMgr::GetQuestIdForItem(uint32 itemId)
{
    bool isQuest = false;
    uint32 questId = 0;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second.get();

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewItemId[i])
                continue;

            if (quest->RewItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewChoiceItemId[i])
                continue;

            if (quest->RewChoiceItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }
        if (isQuest)
            break;
    }
    return questId;
}

std::vector<uint32> RandomItemMgr::GetQuestIdsForItem(uint32 itemId)
{
    std::vector<uint32> questIds;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second.get();

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewItemId[i])
                continue;

            if (quest->RewItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewChoiceItemId[i])
                continue;

            if (quest->RewChoiceItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }
    }
    return questIds;
}

std::string RandomItemMgr::GetPlayerSpecName(Player* player)
{
    std::string specName;
    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
    case CLASS_PRIEST:
        if (tab == 2)
            specName = "shadow";
        else if (tab == 1)
            specName = "holy";
        else
            specName = "disc";
        ;        break;
    case CLASS_SHAMAN:
        if (tab == 2)
            specName = "resto";
        else if (tab == 1)
            specName = "enhance";
        else
            specName = "elem";
        break;
    case CLASS_WARRIOR:
        if (tab == 2)
            specName = "prot";
        else if (tab == 1)
            specName = "fury";
        else
            specName = "arms";
        break;
    case CLASS_PALADIN:
        if (tab == 0)
            specName = "holy";
        else if (tab == 1)
            specName = "prot";
        else if (tab == 2)
            specName = "retrib";
        break;
    case CLASS_DRUID:
        if (tab == 0)
            specName = "balance";
        else if (tab == 1)
        {
            specName = "feraltank";
            if (player->GetLevel() > 19 && urand(0, 100) > 50)
                specName = "feraldps";
        }
        else if (tab == 2)
            specName = "resto";
        break;
    case CLASS_ROGUE:
        if (tab == 0)
            specName = "assas";
        else if (tab == 1)
            specName = "combat";
        else if (tab == 2)
            specName = "subtle";
        break;
    case CLASS_HUNTER:
        if (tab == 0)
            specName = "beast";
        else if (tab == 1)
            specName = "marks";
        else if (tab == 2)
            specName = "surv";
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        if (tab == 0)
            specName = "blooddps";
        else if (tab == 1)
        {
            specName = "frosttank";
            if (urand(0, 100) > 50)
                specName = "frostdps";
        }
        else if (tab == 2)
            specName = "unholydps";
        break;
#endif
    case CLASS_MAGE:
        if (tab == 0)
            specName = "arcane";
        else if (tab == 1)
            specName = "fire";
        else if (tab == 2)
            specName = "frost";
        break;
    case CLASS_WARLOCK:
        if (tab == 0)
            specName = "afflic";
        else if (tab == 1)
            specName = "demo";
        else if (tab == 2)
            specName = "destro";
        break;
    default:
        break;
    }
    return specName;
}

uint32 RandomItemMgr::GetPlayerSpecId(Player* player)
{
    std::string specName = GetPlayerSpecName(player);
    if (specName.empty())
        return 0;

    for (auto itr : m_weightScales)
    {
        if (itr.second.info.name == specName && itr.second.info.classId == player->getClass())
            return itr.second.info.id;
    }
    return 0;
}

uint32 RandomItemMgr::GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId)
{
    if (!player)
        return 0;

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 specId = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    std::vector<uint32> classspecs;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[specNum].info.id)
            continue;

        classspecs.push_back(m_weightScales[specNum].info.id);

        if (m_weightScales[specNum].info.name == spec)
            specId = m_weightScales[specNum].info.id;
    }
    if (!specId)
        return 0;

    if (itemId && itemInfoCache[itemId])
    {
        oldStatWeight = itemInfoCache[itemId]->weights[specId];

        if (oldStatWeight)
            sLog.outString("Old Item: %d, weight: %d", itemId, oldStatWeight);
        else
            sLog.outString("Old item has no stat weight");
    }

    for (std::map<uint32, ItemInfoEntry*>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry* info = i->second;
        if (!info)
            continue;

        // skip useless items
        if (info->weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info->minLevel > player->GetLevel())
            continue;

        // skip too low level
        if (info->minLevel < (player->GetLevel() - 10))
            continue;

        // skip wrong team
        if (info->team && info->team != player->GetTeam())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info->slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info->quality != quality)
            continue;

        // skip worse items
        if (info->weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info->weights[specId] == 1 && player->GetLevel() > 40)
            continue;

        // skip quest items
        if (info->source == ITEM_SOURCE_QUEST)
        {
            bool hasQuestDone = false;
            for (const auto& source : info->sourceIds)
            {
                if (player->GetQuestRewardStatus(source) == QUEST_STATUS_COMPLETE)
                    hasQuestDone = true;
            }
            if (!hasQuestDone)
                continue;
        }

        // skip no stats trinkets
        if (info->weights[specId] == 1 &&
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2)
            continue;

        // check if item stat score is the best among class specs
        uint32 bestSpecId = 0;
        uint32 bestSpecScore = 0;
        for (std::vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        {
            if (info->weights[*i] > bestSpecScore)
            {
                bestSpecId = *i;
                bestSpecScore = info->weights[specId];
            }
        }

        if (bestSpecId && bestSpecId != specId && player->GetLevel() > 40)
            return 0;

        if (!closestUpgrade)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }

        // pick closest upgrade
        if (info->weights[specId] < closestUpgradeWeight)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }
    }

    if (closestUpgrade)
        sLog.outString("New Item: %d, weight: %d", closestUpgrade, closestUpgradeWeight);

    return closestUpgrade;
}

std::vector<uint32> RandomItemMgr::GetUpgradeList(Player* player, uint32 specId, uint8 slot, uint32 quality, uint32 itemId, uint32 amount)
{
    std::vector<uint32> listItems;
    if (!player)
        return listItems;

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    std::vector<uint32> classspecs;

    if (itemId && itemInfoCache[itemId])
    {
        oldStatWeight = itemInfoCache[itemId]->weights[specId];

        if (oldStatWeight)
            sLog.outString("Old Item: %d, weight: %d", itemId, oldStatWeight);
        else
            sLog.outString("Old item has no stat weight");
    }

    for (std::map<uint32, ItemInfoEntry*>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry* info = i->second;
        if (!info)
            continue;

        // skip useless items
        if (info->weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info->minLevel > player->GetLevel())
            continue;

        // skip too low level
        if ((int32)info->minLevel < (int32)(player->GetLevel() - 20))
            continue;

        // skip wrong team
        if (info->team && info->team != player->GetTeam())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info->slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info->quality != quality)
            continue;

        // skip worse items
        if (info->weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info->weights[specId] == 1 && player->GetLevel() > 20)
            continue;

        // skip quest items
        if (info->source == ITEM_SOURCE_QUEST)
        {
            bool hasQuestDone = false;
            for (const auto& source : info->sourceIds)
            {
                if (player->GetQuestRewardStatus(source) == QUEST_STATUS_COMPLETE)
                    hasQuestDone = true;
            }
            if (!hasQuestDone)
                continue;
        }

        // skip no stats trinkets
        if (info->weights[specId] < 2 && (
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2))
            continue;

        // skip pvp items
        if (info->source == ITEM_SOURCE_PVP)
        {
#ifndef MANGOSBOT_ZERO
            if (!player->GetHonorPoints() && !player->GetArenaPoints())
                continue;
#else
            if (!player->GetHonorRankInfo().rank)
                continue;
#endif
        }

        //if (player->GetLevel() >= 40)
        //{
        //    // check if item stat score is the best among class specs
        //    uint32 bestSpecId = 0;
        //    uint32 bestSpecScore = 0;
        //    for (std::vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        //    {
        //        if (info->weights[*i] > bestSpecScore)
        //        {
        //            bestSpecId = *i;
        //            bestSpecScore = info->weights[specId];
        //        }
        //    }

        //    if (bestSpecId && bestSpecId != specId)
        //        continue;
        //}

        listItems.push_back(info->itemId);
        //continue;

        // pick closest upgrade
        if (info->weights[specId] > closestUpgradeWeight)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }
    }

    if (listItems.size())
        sLog.outString("New Items: %d, Old item:%d, New items max: %d", listItems.size(), oldStatWeight, closestUpgradeWeight);

    // sort by stat weight
    std::sort(listItems.begin(), listItems.end(), [specId](int a, int b) { return sRandomItemMgr.GetStatWeight(a, specId) <= sRandomItemMgr.GetStatWeight(b, specId); });

    return listItems;
}

bool RandomItemMgr::HasStatWeight(uint32 itemId)
{
    return itemInfoCache[itemId] != nullptr;
}

bool RandomItemMgr::CanBuyFromVendor(Player *player, uint32 itemId, uint32 creatureId)
{
    CreatureInfo const* cInfo = sObjectMgr.GetCreatureTemplate(creatureId);
    if (!cInfo)
        return false;

    VendorItemList vendorItems;
    VendorItemData const* vItems = sObjectMgr.GetNpcVendorItemList(creatureId);
    VendorItemData const* tItems = sObjectMgr.GetNpcVendorTemplateItemList(cInfo->VendorTemplateId);

    if (!vItems && !tItems)
    {
        return false;
    }

    uint8 customitems = vItems ? vItems->GetItemCount() : 0;
    uint8 numitems = customitems + (tItems ? tItems->GetItemCount() : 0);

    for (int i = 0; i < numitems; ++i)
    {
        VendorItem const* crItem = i < customitems ? vItems->GetItem(i) : tItems->GetItem(i - customitems);

        if (crItem && crItem->item == itemId)
        {
            ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemId);
            if (pProto)
            {
                // when no faction required but rank > 0 will be used faction id from the vendor faction template to compare the rank
                if (!pProto->RequiredReputationFaction && pProto->RequiredReputationRank > 0 &&
                    ReputationRank(pProto->RequiredReputationRank) > player->GetReputationRank(sFactionTemplateStore.LookupEntry(cInfo->Faction)->faction))
                    return false;

                if (crItem->conditionId && !sObjectMgr.IsConditionSatisfied(crItem->conditionId, player, player->GetMap(), nullptr, CONDITION_FROM_VENDOR))
                    return false;
            }
            return true;
        }
    }
    return false;
}

bool RandomItemMgr::HasSameQuestRewards(Player *player, uint32 itemId)
{
    ItemInfoEntry* info = itemInfoCache[itemId];
    if (!info)
        return false;
    if (info->source != ITEM_SOURCE_QUEST)
        return false;

    for (auto& questId : info->sourceIds)
    {
        Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
        if (!quest)
            continue;

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewItemId[i])
                continue;

            if (player->HasItemCount(quest->RewItemId[i], 1, true))
                return true;
        }

        uint32 rewChoiceItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChoiceItemCount; ++i)
        {
            if (!quest->RewChoiceItemId[i])
                continue;

            if (player->HasItemCount(quest->RewChoiceItemId[i], 1, true))
                return true;
        }
    }
    return false;
}

uint32 RandomItemMgr::GetMinLevelFromCache(uint32 itemId)
{
    ItemInfoEntry* info = itemInfoCache[itemId];
    if (!info)
        return 0;

    return info->minLevel;
}

uint32 RandomItemMgr::GetStatWeight(Player* player, uint32 itemId)
{
    if (!player || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    uint32 specId = GetPlayerSpecId(player);
    std::vector<uint32> classspecs;

    if (specId == 0)
        return 0;

    if (!m_weightScales[specId].info.id)
        return 0;

    std::map<uint32, ItemInfoEntry*>::iterator itr = itemInfoCache.find(itemId);
    if (itr != itemInfoCache.end())
    {
        statWeight = itr->second->weights[specId];
    }

    return statWeight;
}

uint32 RandomItemMgr::GetStatWeight(uint32 itemId, uint32 specId)
{
    if (!specId || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    std::vector<uint32> classspecs;

    if (!m_weightScales[specId].info.id)
        return 0;

    std::map<uint32, ItemInfoEntry*>::iterator itr = itemInfoCache.find(itemId);
    if (itr != itemInfoCache.end())
    {
        statWeight = itr->second->weights[specId];
    }

    return statWeight;
}

uint32 RandomItemMgr::GetBestRandomEnchantStatWeight(uint32 itemId, uint32 specId)
{
    if (!specId || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    if (!m_weightScales[specId].info.id)
        return 0;

    uint8 plrClass = 0;
    uint32 statWeight = 0;

    for (auto itr : m_weightScales)
    {
        if (itr.second.info.id == specId)
            plrClass = itr.second.info.classId;
    }

    if (!plrClass)
        return 0;

    std::map<uint32, ItemInfoEntry*>::iterator itr = itemInfoCache.find(itemId);
    if (itr != itemInfoCache.end())
    {
        uint32 bestEnch = CalculateBestRandomEnchantId(plrClass, specId, itemId);
        if (bestEnch)
        {
            statWeight = CalculateEnchantWeight(plrClass, specId, bestEnch);
        }
    }

    return statWeight;
}

uint32 RandomItemMgr::GetLiveStatWeight(Player* player, uint32 itemId, uint32 specId)
{
    if (!player || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    specId = specId ? specId : GetPlayerSpecId(player);
    if (specId == 0)
        return 0;

    if (!m_weightScales[specId].info.id)
        return 0;

    ItemInfoEntry* info = itemInfoCache[itemId];
    if (!info)
        return 0;

    statWeight = info->weights[specId];

    // skip higher lvl
    if (info->minLevel > player->GetLevel())
        return 0;

    // skip too low level
    //if ((int32)info->minLevel < (int32)(player->GetLevel() - 20))
    //    return 0;

    // skip wrong team
    if (info->team && (Team)info->team != player->GetTeam())
        return 0;

    // skip quest items
    if (info->source == ITEM_SOURCE_QUEST && !info->sourceIds.empty())
    {
        bool canDoQuest = false;
        for (const auto& source : info->sourceIds)
        {
            Quest const* quest = sObjectMgr.GetQuestTemplate(source);
            if (quest)
            {
                // only class quests player could do
                if (player->SatisfyQuestClass(quest, false) && player->SatisfyQuestRace(quest, false) && player->SatisfyQuestLevel(quest, false))
                    canDoQuest = true;

                // check if quest is inactive (if linked to a not running game event)
                if (!quest->IsActive())
                    canDoQuest = false;

                // can be rewarded
                if (canDoQuest)
                    break;
            }
        }
        if (!canDoQuest)
            return 0;
    }

    // skip pvp items
    /*if (info->source == ITEM_SOURCE_PVP)
    {
#ifndef MANGOSBOT_ZERO
        if (!player->GetHonorPoints() && !player->GetArenaPoints())
            return 0;
#else
        if (!player->GetHonorRankInfo().rank)
            return 0;
#endif
    }*/

    // skip missing reputation
    if (info->repFaction && uint32(player->GetReputationRank(info->repFaction)) < info->repRank)
        return 0;

#ifdef MANGOSBOT_ZERO
    // skip missing pvp ranks
    if (info->pvpRank && player->GetHonorHighestRankInfo().rank < info->pvpRank)
        return 0;
    if (info->pvpRank && info->pvpRank < 16 && player->GetHonorHighestRankInfo().rank == 18)
        return 0;

    // skip non pvp items for some specs
    if (info->pvpRank < 16 && player->GetHonorHighestRankInfo().rank == 18)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (proto && !(
            info->slot == EQUIPMENT_SLOT_WAIST ||
            info->slot == EQUIPMENT_SLOT_WRISTS ||
            info->slot == EQUIPMENT_SLOT_BACK ||
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2 ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_TOTEM ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_LIBRAM ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_IDOL)
            && !(specId == 3 || specId == 4 || specId == 5 || specId == 14 || specId == 22 || specId == 31))
            return 0;
    }
#else
    // skip missing pvp ranks
    if (info->pvpRank && player->GetHighestPvPRankIndex() < info->pvpRank)
        return 0;
#endif

    // skip missing skills
    if (info->reqSkill && player->GetSkillValue(info->reqSkill) < info->reqSkillRank)
        return 0;

    // skip no stats trinkets
    if (info->weights[specId] == 1 && (
        info->slot == EQUIPMENT_SLOT_NECK ||
        info->slot == EQUIPMENT_SLOT_TRINKET1 ||
        info->slot == EQUIPMENT_SLOT_TRINKET2 ||
        info->slot == EQUIPMENT_SLOT_FINGER1 ||
        info->slot == EQUIPMENT_SLOT_FINGER2))
        return 0;

    // check if item stat score is the best among class specs
    /*uint32 bestSpecId = 0;
    uint32 bestSpecScore = 0;
    for (uint32 spec = 1; spec < MAX_STAT_SCALES; ++spec)
    {
        if (!m_weightScales[spec].info.id)
            continue;

        if (m_weightScales[spec].info.classId != player->getClass())
            continue;

        if (info->weights[spec] > bestSpecScore && info->weights[spec] > 1)
        {
            bestSpecId = spec;
            bestSpecScore = info->weights[spec];
        }
    }*/

    // TODO test
    /*if (bestSpecId && bestSpecId != specId && player->GetLevel() >= 60)
        return 0;*/

#ifdef MANGOSBOT_ZERO
    // increase stat weights for pvp items
    if (info->pvpRank)
        return statWeight * 5;
#endif

    return statWeight;
}

void RandomItemMgr::BuildEquipCache()
{
    uint32 maxLevel = DEFAULT_MAX_LEVEL;

    auto results = CharacterDatabase.PQuery("select clazz, spec, lvl, slot, quality, item from ai_playerbot_equip_cache");
    if (results)
    {
        sLog.outString("Loading equipment cache for %d classes, %d levels, %d slots, %d quality from %d items",
                MAX_CLASSES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, sItemStorage.GetMaxEntry());
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 clazz = fields[0].GetUInt32();
            uint32 spec = fields[1].GetUInt32();
            uint32 level = fields[2].GetUInt32();
            uint32 slot = fields[3].GetUInt32();
            uint32 quality = fields[4].GetUInt32();
            uint32 itemId = fields[5].GetUInt32();

            BotEquipKey key(level, clazz, spec, slot, quality);
            equipCache[key].push_back(itemId);
            count++;

        } while (results->NextRow());
        sLog.outString("Equipment cache loaded from %d records", count);
    }
    else
    {
        uint64 total = uint64(MAX_CLASSES * 3 * maxLevel * EQUIPMENT_SLOT_END * ITEM_QUALITY_ARTIFACT);
        sLog.outString("Building equipment cache for %d classes, %d specs, %d levels, %d slots, %d quality from %d items (%zu total)",
                MAX_CLASSES, MAX_STAT_SCALES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, sItemStorage.GetMaxEntry(), total);

        BarGoLink bar(total);
        RandomItemList tabardsList;
        RandomItemList shirtsList;
        BotEquipKey tabardKey(60, 1, 1, EQUIPMENT_SLOT_TABARD, 1);
        BotEquipKey shirtKey(60, 1, 1, EQUIPMENT_SLOT_BODY, 1);

        // TO DO: Replace this with a db query
#ifdef MANGOSBOT_ZERO
        static std::unordered_set<uint32> unavailableItemIDs = { 128, 997, 1024, 1027, 1217, 1255, 23720, 2929, 7725, 15141, 13460, 5013, 6376, 6345, 6343, 6222, 21857, 22728, 22729, 17967, 1041, 1133, 1134, 2413, 2415, 5663, 5874, 5875, 8583, 8589, 8590, 8627, 8630, 8633, 16339, 20221, 13323, 13324, 14062, 23193, 21044, 2932, 41, 42, 46, 50, 54, 58, 77, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101, 102, 103, 104, 105, 113, 114, 115, 119, 122, 123, 124, 125, 126, 130, 131, 132, 133, 134, 135, 136, 137, 138, 141, 146, 149, 150, 151, 152, 155, 156, 157, 184, 527, 734, 741, 746, 751, 761, 784, 786, 788, 806, 807, 808, 813, 823, 836, 842, 855, 875, 876, 877, 883, 894, 898, 900, 901, 902, 903, 905, 906, 907, 908, 909, 913, 917, 930, 931, 941, 945, 948, 951, 956, 958, 960, 964, 965, 966, 967, 968, 973, 974, 975, 976, 980, 985, 986, 989, 992, 994, 996, 1002, 1004, 1014, 1016, 1018, 1020, 1021, 1022, 1023, 1025, 1026, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1042, 1043, 1044, 1046, 1047, 1048, 1049, 1052, 1053, 1057, 1058, 1061, 1063, 1072, 1078, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1095, 1096, 1099, 1100, 1101, 1102, 1105, 1108, 1109, 1111, 1112, 1115, 1119, 1122, 1123, 1124, 1125, 1128, 1136, 1138, 1139, 1141, 1144, 1146, 1149, 1150, 1151, 1157, 1162, 1163, 1164, 1165, 1170, 1174, 1176, 1184, 1186, 1192, 1199, 1216, 1222, 1224, 1228, 1229, 1231, 1232, 1238, 1239, 1243, 1244, 1245, 1246, 1250, 1253, 1258, 1259, 1266, 1267, 1268, 1269, 1272, 1279, 1281, 1298, 1311, 1312, 1313, 1321, 1323, 1324, 1328, 1332, 1334, 1335, 1339, 1341, 1350, 1352, 1354, 1356, 1363, 1371, 1379, 1385, 1392, 1397, 1398, 1400, 1402, 1403, 1424, 1432, 1435, 1444, 1450, 1472, 1492, 1500, 1508, 1527, 1533, 1534, 1535, 1536, 1544, 1545, 1554, 1559, 1567, 1568, 1571, 1574, 1588, 1589, 1591, 1597, 1599, 1603, 1612, 1619, 1622, 1623, 1638, 1641, 1648, 1649, 1651, 1654, 1655, 1657, 1658, 1663, 1672, 1676, 1681, 1684, 1689, 1690, 1691, 1692, 1693, 1694, 1695, 1698, 1699, 1700, 1704, 1719, 1724, 1736, 1851, 1854, 1877, 1878, 1880, 1882, 1886, 1912, 1914, 1915, 1918, 1924, 1940, 1948, 1950, 1960, 1963, 1969, 1977, 1995, 1999, 2002, 2003, 2012, 2016, 2038, 2045, 2050, 2056, 2060, 2071, 2103, 2104, 2106, 2107, 2115, 2128, 2170, 2189, 2191, 2206, 2255, 2273, 2275, 2305, 2306, 2322, 2323, 2363, 2404, 2405, 2410, 2412, 2460, 2461, 2462, 2478, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2496, 2497, 2498, 2500, 2501, 2502, 2503, 2513, 2514, 2517, 2518, 2554, 2573, 2574, 2588, 2599, 2600, 2602, 2638, 2647, 2655, 2664, 2668, 2669, 2688, 2693, 2755, 2789, 2790, 2791, 2792, 2793, 2803, 2804, 2808, 2811, 2812, 2814, 2826, 2867, 2887, 2890, 2891, 2895, 2896, 2918, 2919, 2920, 2921, 2922, 2923, 2927, 2945, 2948, 2993, 2994, 2995, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3015, 3028, 3029, 3031, 3032, 3044, 3046, 3050, 3051, 3052, 3054, 3059, 3060, 3061, 3062, 3063, 3064, 3068, 3077, 3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3096, 3097, 3098, 3099, 3100, 3101, 3102, 3109, 3113, 3114, 3115, 3116, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3132, 3133, 3134, 3136, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145, 3146, 3147, 3149, 3150, 3159, 3168, 3215, 3219, 3221, 3222, 3226, 3232, 3242, 3243, 3244, 3245, 3246, 3247, 3249, 3259, 3271, 3278, 3298, 3316, 3320, 3333, 3338, 3410, 3436, 3438, 3441, 3459, 3500, 3501, 3503, 3504, 3507, 3513, 3519, 3522, 3523, 3524, 3525, 3526, 3527, 3528, 3529, 3532, 3533, 3534, 3535, 3536, 3537, 3538, 3539, 3540, 3541, 3542, 3543, 3544, 3545, 3546, 3547, 3548, 3549, 3557, 3568, 3580, 3584, 3620, 3624, 3646, 3648, 3675, 3677, 3686, 3687, 3705, 3707, 3738, 3744, 3746, 3762, 3768, 3773, 3788, 3789, 3790, 3791, 3861, 3865, 3878, 3881, 3883, 3884, 3885, 3886, 3887, 3888, 3895, 3929, 3933, 3934, 3952, 3953, 3954, 3955, 3956, 3957, 3958, 3959, 3977, 3978, 3979, 3980, 3981, 3982, 3983, 3984, 3988, 3991, 4008, 4009, 4010, 4011, 4012, 4013, 4014, 4015, 4030, 4031, 4032, 4033, 4095, 4141, 4142, 4143, 4144, 4145, 4146, 4147, 4148, 4149, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4160, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4222, 4223, 4224, 4225, 4226, 4227, 4228, 4229, 4230, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4295, 4418, 4427, 4431, 4442, 4451, 4452, 4475, 4486, 4501, 4523, 4524, 4559, 4573, 4578, 4579, 4620, 4657, 4664, 4667, 4670, 4673, 4679, 4682, 4685, 4688, 4691, 4704, 4728, 4730, 4747, 4748, 4749, 4750, 4754, 4756, 4760, 4761, 4762, 4763, 4764, 4773, 4774, 4811, 4812, 4815, 4839, 4842, 4853, 4855, 4856, 4857, 4858, 4868, 4884, 4885, 4889, 4899, 4900, 4901, 4902, 4912, 4927, 4930, 4934, 4943, 4950, 4955, 4956, 4959, 4965, 4966, 4981, 4985, 4988, 4989, 4990, 4996, 4997, 5000, 5004, 5008, 5010, 5014, 5015, 5024, 5041, 5045, 5046, 5047, 5049, 5053, 5090, 5091, 5106, 5108, 5126, 5127, 5129, 5130, 5131, 5132, 5139, 5141, 5142, 5144, 5145, 5146, 5147, 5148, 5149, 5150, 5151, 5152, 5153, 5154, 5155, 5156, 5157, 5158, 5159, 5160, 5161, 5162, 5163, 5171, 5172, 5174, 5222, 5223, 5226, 5227, 5228, 5230, 5231, 5235, 5255, 5264, 5265, 5282, 5283, 5294, 5295, 5296, 5297, 5298, 5307, 5308, 5330, 5331, 5333, 5353, 5358, 5365, 5372, 5378, 5380, 5381, 5384, 5400, 5401, 5402, 5403, 5406, 5407, 5408, 5409, 5410, 5434, 5436, 5438, 5449, 5450, 5453, 5454, 5515, 5531, 5545, 5546, 5548, 5549, 5550, 5551, 5552, 5553, 5554, 5555, 5556, 5557, 5558, 5559, 5560, 5561, 5562, 5563, 5564, 5577, 5603, 5607, 5625, 5632, 5641, 5644, 5647, 5648, 5649, 5650, 5651, 5652, 5653, 5654, 5657, 5658, 5660, 5661, 5662, 5666, 5667, 5670, 5671, 5672, 5673, 5674, 5676, 5677, 5678, 5679, 5680, 5682, 5683, 5684, 5685, 5688, 5696, 5697, 5698, 5699, 5700, 5701, 5702, 5703, 5704, 5705, 5706, 5707, 5708, 5709, 5710, 5711, 5712, 5713, 5714, 5715, 5716, 5719, 5720, 5721, 5722, 5723, 5724, 5725, 5726, 5727, 5728, 5729, 5730, 5742, 5743, 5748, 5768, 5769, 5821, 5822, 5823, 5828, 5845, 5857, 5858, 5859, 5878, 5896, 5916, 5937, 5949, 5953, 5954, 5968, 6036, 6090, 6130, 6131, 6132, 6133, 6174, 6192, 6207, 6208, 6209, 6210, 6213, 6216, 6243, 6244, 6255, 6262, 6273, 6276, 6277, 6278, 6279, 6280, 6374, 6437, 6478, 6489, 6490, 6491, 6492, 6493, 6494, 6495, 6496, 6497, 6498, 6499, 6500, 6501, 6516, 6544, 6589, 6619, 6620, 6621, 6623, 6638, 6639, 6644, 6646, 6648, 6649, 6650, 6673, 6674, 6683, 6698, 6707, 6708, 6711, 6724, 6728, 6730, 6733, 6734, 6736, 6754, 6777, 6778, 6779, 6837, 6850, 6852, 6891, 6896, 6897, 6899, 6946, 6988, 7007, 7093, 7170, 7171, 7187, 7188, 7248, 7275, 7299, 7347, 7388, 7425, 7426, 7427, 7466, 7467, 7497, 7547, 7548, 7550, 7681, 7707, 7716, 7868, 7869, 7872, 7925, 7948, 7949, 7950, 7951, 7952, 7953, 7977, 7986, 7987, 7988, 7994, 8195, 8243, 8388, 8493, 8502, 8503, 8504, 8505, 8506, 8507, 8543, 8546, 8547, 8688, 8743, 8744, 8745, 8756, 8757, 8758, 8759, 8760, 8761, 8762, 8763, 8764, 8765, 8768, 8769, 8770, 8771, 8772, 8773, 8774, 8775, 8776, 8777, 8778, 8779, 8780, 8781, 8782, 8783, 8784, 8785, 8786, 8787, 8788, 8789, 8790, 8791, 8792, 8793, 8794, 8795, 8796, 8797, 8798, 8799, 8800, 8801, 8802, 8803, 8804, 8805, 8806, 8807, 8808, 8809, 8810, 8811, 8812, 8813, 8814, 8815, 8816, 8818, 8819, 8820, 8821, 8822, 8823, 8824, 8825, 8826, 8828, 8829, 8830, 8832, 8833, 8834, 8835, 8837, 8840, 8841, 8842, 8843, 8844, 8847, 8848, 8849, 8850, 8851, 8852, 8853, 8854, 8855, 8856, 8857, 8858, 8859, 8860, 8861, 8862, 8863, 8864, 8865, 8866, 8867, 8868, 8869, 8870, 8871, 8872, 8873, 8874, 8875, 8876, 8877, 8878, 8879, 8880, 8881, 8882, 8883, 8884, 8885, 8886, 8887, 8888, 8889, 8890, 8891, 8892, 8893, 8894, 8895, 8896, 8897, 8898, 8899, 8900, 8901, 8902, 8903, 8904, 8905, 8906, 8907, 8909, 8910, 8911, 8912, 8913, 8914, 8915, 8916, 8917, 8918, 8919, 8920, 8921, 8922, 8929, 8930, 8931, 8933, 8934, 8935, 8936, 8937, 8938, 8939, 8940, 8941, 8942, 8943, 8944, 8945, 8947, 8954, 8955, 8958, 8960, 8961, 8962, 8963, 8965, 8966, 8967, 8968, 8969, 8971, 8972, 8974, 8975, 8976, 8977, 8978, 8980, 8981, 8983, 8986, 8987, 8988, 8989, 8990, 8991, 8992, 8993, 8994, 8995, 8996, 8997, 8998, 8999, 9000, 9001, 9002, 9003, 9004, 9005, 9006, 9007, 9008, 9009, 9010, 9011, 9012, 9013, 9014, 9015, 9016, 9017, 9018, 9019, 9020, 9021, 9022, 9023, 9024, 9025, 9026, 9027, 9028, 9029, 9031, 9032, 9033, 9034, 9035, 9037, 9039, 9040, 9041, 9043, 9044, 9046, 9047, 9048, 9049, 9050, 9051, 9052, 9053, 9054, 9055, 9056, 9057, 9058, 9059, 9062, 9063, 9064, 9065, 9066, 9067, 9068, 9069, 9070, 9071, 9072, 9073, 9074, 9075, 9076, 9077, 9078, 9079, 9080, 9081, 9082, 9083, 9084, 9085, 9086, 9087, 9089, 9090, 9091, 9092, 9093, 9094, 9095, 9096, 9097, 9098, 9099, 9100, 9101, 9102, 9103, 9104, 9105, 9123, 9124, 9125, 9126, 9127, 9128, 9129, 9130, 9131, 9132, 9133, 9134, 9135, 9136, 9137, 9138, 9139, 9140, 9141, 9142, 9143, 9145, 9146, 9147, 9148, 9150, 9151, 9152, 9156, 9157, 9158, 9159, 9160, 9161, 9162, 9164, 9165, 9166, 9167, 9168, 9169, 9170, 9171, 9174, 9175, 9176, 9177, 9178, 9180, 9181, 9182, 9183, 9184, 9185, 9188, 9190, 9191, 9192, 9193, 9194, 9195, 9198, 9199, 9200, 9201, 9202, 9203, 9204, 9205, 9207, 9208, 9209, 9211, 9212, 9215, 9216, 9217, 9218, 9219, 9221, 9222, 9223, 9225, 9226, 9227, 9228, 9229, 9230, 9231, 9325, 9380, 9417, 9443, 9464, 9529, 9532, 9537, 9685, 9888, 10010, 10011, 10020, 10032, 10038, 10039, 10049, 10303, 10304, 10313, 10319, 10322, 10324, 10478, 10555, 10579, 10580, 10585, 10594, 10595, 10596, 10650, 10651, 10683, 10719, 10723, 11085, 11099, 11111, 11115, 11170, 11171, 11198, 11199, 11200, 11201, 11228, 11442, 11443, 11473, 11505, 11609, 11613, 11616, 11663, 11664, 11666, 11667, 11670, 11671, 11672, 11673, 11676, 11683, 11838, 11903, 12104, 12105, 12106, 12107, 12143, 12186, 12187, 12188, 12189, 12211, 12244, 12245, 12258, 12369, 12385, 12440, 12442, 12443, 12468, 12469, 12526, 12585, 12615, 12616, 12617, 12729, 12762, 12763, 12764, 12769, 12778, 12779, 12789, 12795, 12802, 12805, 12816, 12817, 12818, 12826, 12831, 12832, 12853, 12904, 12931, 12961, 12962, 12970, 12971, 12972, 12986, 13080, 13090, 13092, 13149, 13151, 13152, 13153, 13154, 13214, 13223, 13242, 13247, 13291, 13318, 13330, 13338, 13342, 13343, 13500, 13503, 13517, 13543, 13586, 13608, 13642, 13643, 13644, 13645, 13646, 13647, 13648, 13649, 13650, 13651, 13652, 13653, 13654, 13655, 13656, 13657, 13658, 13659, 13660, 13661, 13662, 13663, 13664, 13665, 13666, 13667, 13668, 13669, 13670, 13671, 13672, 13673, 13674, 13675, 13676, 13677, 13678, 13679, 13680, 13681, 13682, 13683, 13684, 13685, 13686, 13687, 13688, 13689, 13690, 13691, 13692, 13693, 13694, 13695, 13696, 13697, 13710, 13711, 13712, 13713, 13714, 13715, 13716, 13717, 13726, 13727, 13728, 13729, 13730, 13731, 13732, 13733, 13734, 13735, 13736, 13737, 13738, 13739, 13740, 13741, 13742, 13743, 13744, 13745, 13746, 13747, 13748, 13749, 13762, 13763, 13764, 13765, 13766, 13767, 13768, 13769, 13770, 13771, 13772, 13773, 13774, 13775, 13776, 13777, 13778, 13779, 13780, 13781, 13782, 13783, 13784, 13785, 13786, 13787, 13788, 13789, 13790, 13791, 13792, 13793, 13794, 13795, 13796, 13797, 13798, 13799, 13800, 13801, 13802, 13803, 13804, 13805, 13806, 13807, 13808, 13809, 13811, 13812, 13842, 13843, 13844, 13845, 13846, 13847, 13848, 13849, 13862, 13923, 13936, 14083, 14363, 14382, 14383, 14384, 14385, 14386, 14387, 14388, 14389, 14390, 14391, 14392, 14393, 14394, 14524, 14550, 14597, 14609, 14691, 14696, 14818, 14822, 14871, 14876, 14878, 14883, 14884, 14885, 14886, 14887, 14888, 14889, 14890, 14891, 14892, 15446, 15586, 15688, 15769, 15780, 15888, 15889, 16024, 16025, 16026, 16027, 16028, 16029, 16030, 16031, 16033, 16034, 16035, 16036, 16037, 16038, 16061, 16062, 16063, 16064, 16065, 16066, 16067, 16068, 16069, 16070, 16071, 16073, 16074, 16075, 16076, 16077, 16078, 16079, 16080, 16081, 16082, 16085, 16086, 16102, 16103, 16104, 16105, 16106, 16107, 16108, 16109, 16116, 16117, 16118, 16119, 16120, 16121, 16122, 16123, 16124, 16125, 16126, 16127, 16129, 16131, 16132, 16134, 16135, 16136, 16137, 16138, 16139, 16140, 16141, 16142, 16143, 16144, 16145, 16146, 16147, 16148, 16149, 16150, 16151, 16152, 16153, 16154, 16155, 16156, 16157, 16158, 16159, 16160, 16161, 16162, 16163, 16164, 16172, 16173, 16174, 16175, 16176, 16177, 16178, 16179, 16180, 16181, 16182, 16183, 16184, 16185, 16186, 16187, 16188, 16191, 16211, 16212, 16213, 16308, 16315, 16336, 16337, 16338, 16343, 16344, 16367, 16370, 16394, 16395, 16398, 16399, 16400, 16402, 16404, 16407, 16411, 16412, 16438, 16439, 16445, 16447, 16458, 16460, 16461, 16464, 16469, 16470, 16481, 16482, 16488, 16493, 16495, 16500, 16511, 16512, 16517, 16520, 16529, 16537, 16538, 16546, 16547, 16553, 16556, 16557, 16559, 16570, 16572, 16575, 16576, 16664, 16792, 17000, 17024, 17027, 17040, 17041, 17108, 17115, 17116, 17122, 17142, 17162, 17163, 17199, 17342, 17343, 17347, 17354, 17409, 17412, 17563, 17565, 17574, 17575, 17582, 17585, 17587, 17589, 17595, 17597, 17606, 17609, 17614, 17615, 17619, 17621, 17731, 17769, 17783, 17802, 17824, 17825, 17826, 17827, 17828, 17829, 17830, 17831, 17832, 17833, 17834, 17835, 17836, 17837, 17838, 17839, 17840, 17841, 17842, 17843, 17844, 17845, 17846, 17847, 17848, 17851, 17852, 17853, 17854, 17855, 17856, 17857, 17858, 17859, 17860, 17861, 17862, 17882, 17883, 17884, 17885, 17886, 17887, 17888, 17889, 17890, 17891, 17892, 17893, 17894, 17895, 17896, 17897, 17898, 17899, 17910, 17911, 18023, 18063, 18105, 18106, 18153, 18155, 18156, 18157, 18158, 18159, 18161, 18162, 18163, 18164, 18165, 18209, 18235, 18303, 18304, 18316, 18320, 18341, 18342, 18355, 18438, 18589, 18593, 18595, 18599, 18627, 18630, 18666, 18667, 18668, 18669, 18685, 18747, 18763, 18764, 18765, 18800, 18801, 18881, 18882, 18942, 18963, 18964, 18965, 18966, 18967, 18968, 18971, 18982, 19065, 19082, 19122, 19129, 19158, 19184, 19185, 19186, 19187, 19188, 19189, 19190, 19191, 19192, 19193, 19194, 19195, 19196, 19197, 19198, 19199, 19200, 19201, 19226, 19286, 19313, 19314, 19427, 19428, 19455, 19456, 19457, 19482, 19486, 19487, 19488, 19489, 19490, 19502, 19503, 19504, 19622, 19642, 19662, 19742, 19743, 19804, 19809, 19810, 19811, 19837, 19844, 19847, 19868, 19926, 19932, 19966, 19983, 19986, 19987, 19989, 20003, 20005, 20020, 20024, 20026, 20084, 20135, 20136, 20137, 20138, 20139, 20140, 20141, 20142, 20143, 20144, 20145, 20146, 20149, 20178, 20179, 20180, 20182, 20183, 20185, 20238, 20239, 20240, 20241, 20242, 20245, 20246, 20247, 20248, 20249, 20250, 20251, 20252, 20267, 20268, 20269, 20270, 20271, 20272, 20273, 20274, 20275, 20276, 20277, 20278, 20279, 20280, 20281, 20282, 20283, 20284, 20285, 20286, 20287, 20288, 20289, 20290, 20291, 20292, 20297, 20298, 20299, 20300, 20301, 20302, 20303, 20304, 20305, 20306, 20307, 20308, 20309, 20311, 20312, 20313, 20314, 20315, 20316, 20317, 20318, 20319, 20320, 20321, 20322, 20323, 20324, 20325, 20326, 20327, 20328, 20329, 20330, 20331, 20332, 20333, 20334, 20335, 20336, 20337, 20338, 20339, 20340, 20341, 20342, 20343, 20344, 20345, 20346, 20347, 20348, 20349, 20350, 20351, 20352, 20353, 20354, 20355, 20356, 20357, 20358, 20359, 20360, 20361, 20362, 20363, 20364, 20367, 20368, 20370, 20372, 20423, 20445, 20446, 20460, 20462, 20475, 20485, 20489, 20502, 20522, 20524, 20525, 20583, 20584, 20585, 20586, 20587, 20588, 20589, 20590, 20591, 20592, 20593, 20594, 20595, 20596, 20597, 20598, 20609, 20651, 20737, 20739, 20740, 20814, 20834, 20883, 20887, 20908, 20936, 20937, 20946, 21043, 21101, 21102, 21124, 21125, 21127, 21135, 21141, 21152, 21159, 21163, 21168, 21173, 21193, 21194, 21195, 21236, 21238, 21240, 21246, 21247, 21274, 21276, 21313, 21339, 21369, 21419, 21420, 21421, 21422, 21423, 21424, 21425, 21426, 21427, 21428, 21429, 21430, 21431, 21432, 21433, 21434, 21435, 21437, 21439, 21440, 21441, 21442, 21443, 21444, 21445, 21446, 21447, 21448, 21449, 21450, 21451, 21516, 21518, 21550, 21560, 21575, 21577, 21578, 21584, 21588, 21591, 21594, 21612, 21613, 21614, 21628, 21629, 21630, 21631, 21632, 21633, 21634, 21636, 21637, 21638, 21641, 21642, 21643, 21644, 21646, 21649, 21653, 21655, 21656, 21657, 21658, 21659, 21660, 21661, 21662, 21717, 21719, 21720, 21736, 21739, 21782, 21795, 21811, 21890, 21923, 21930, 21962, 21963, 21964, 22020, 22021, 22022, 22023, 22024, 22025, 22026, 22027, 22028, 22029, 22030, 22031, 22032, 22033, 22034, 22035, 22036, 22037, 22038, 22039, 22040, 22041, 22042, 22043, 22045, 22114, 22130, 22151, 22152, 22230, 22233, 22258, 22273, 22316, 22346, 22386, 22387, 22391, 22485, 22486, 22584, 22585, 22586, 22587, 22588, 22619, 22625, 22626, 22684, 22685, 22686, 22687, 22692, 22694, 22695, 22696, 22697, 22698, 22703, 22704, 22705, 22765, 22780, 22781, 22805, 22814, 22817, 22933, 23034, 23058, 23072, 23086, 23162, 23163, 23164, 23172, 23175, 23176, 23215, 23224, 23227, 23245, 23271, 23325, 23360, 23418, 23567, 23656, 23683, 23684, 23689, 23690, 23696, 23698, 23699, 23700, 23701, 23712, 23713, 23714, 23715, 23716, 23718, 23719, 23721, 23722, 23725, 23727, 23728, 23794, 23795, 23796, 24071, 24358, 2556, 17, 192, 2248, 2301, 2543, 2952, 3038, 3043, 3104, 3105, 3106, 3359, 3398, 3479, 3579, 3896, 4081, 4574, 4642, 5031, 5032, 5033, 5034, 5035, 5036, 5037, 5039, 5070, 5290, 6128, 6141, 6142, 6143, 6606, 7066, 7167, 7169, 7677, 7940, 9042, 9239, 9376, 9377, 10006, 10037, 10422, 11182, 11183, 11344, 11345, 12221, 12222, 12246, 12333, 12407, 12413, 12423, 12686, 12767, 12948, 13256, 13294, 13472, 13516, 13919, 14689, 14690, 14692, 14693, 14694, 14695, 14697, 14698, 14699, 14700, 14701, 14702, 14703, 14704, 14705, 14819, 15089, 16128, 16130, 16133, 16334, 16340, 18439, 18732, 18733, 19285, 19294, 19359, 19985, 20133, 20294, 20386, 20523, 20529, 20880, 20905, 21169, 21170, 21172, 21797, 21798, 21799, 21832, 22751 };
#elif MANGOSBOT_ONE
        static std::unordered_set<uint32> unavailableItemIDs = { 17, 41, 42, 46, 50, 54, 58, 77, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101, 102, 103, 104, 105, 113, 114, 115, 122, 123, 124, 125, 126, 130, 131, 132, 133, 134, 135, 136, 137, 138, 141, 146, 149, 150, 151, 152, 155, 156, 157, 184, 192, 527, 734, 741, 746, 761, 784, 786, 788, 806, 807, 808, 813, 823, 836, 842, 855, 875, 876, 877, 883, 894, 898, 900, 901, 902, 903, 905, 906, 907, 908, 909, 913, 917, 930, 931, 941, 945, 948, 951, 956, 958, 960, 964, 965, 966, 967, 968, 973, 974, 975, 976, 980, 985, 986, 989, 992, 994, 996, 997, 1004, 1014, 1016, 1018, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1041, 1042, 1043, 1044, 1046, 1047, 1048, 1049, 1052, 1053, 1057, 1058, 1061, 1063, 1072, 1078, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1095, 1096, 1099, 1100, 1101, 1102, 1105, 1108, 1109, 1111, 1112, 1115, 1119, 1122, 1123, 1124, 1125, 1128, 1133, 1134, 1136, 1138, 1139, 1141, 1144, 1146, 1149, 1150, 1151, 1157, 1162, 1163, 1164, 1165, 1166, 1167, 1170, 1174, 1176, 1184, 1186, 1189, 1192, 1193, 1199, 1216, 1222, 1224, 1228, 1229, 1231, 1232, 1238, 1239, 1243, 1244, 1245, 1246, 1250, 1253, 1255, 1258, 1259, 1266, 1267, 1268, 1269, 1272, 1279, 1281, 1298, 1311, 1312, 1313, 1321, 1323, 1324, 1328, 1332, 1334, 1339, 1341, 1350, 1352, 1354, 1356, 1363, 1371, 1379, 1385, 1386, 1392, 1397, 1398, 1400, 1402, 1403, 1424, 1432, 1435, 1444, 1450, 1472, 1492, 1500, 1508, 1527, 1533, 1534, 1535, 1536, 1544, 1545, 1554, 1559, 1567, 1568, 1571, 1574, 1588, 1589, 1591, 1597, 1599, 1603, 1612, 1619, 1622, 1623, 1638, 1641, 1648, 1649, 1651, 1654, 1655, 1657, 1658, 1663, 1672, 1676, 1681, 1684, 1689, 1690, 1691, 1692, 1693, 1694, 1695, 1698, 1699, 1700, 1704, 1719, 1724, 1736, 1851, 1854, 1877, 1882, 1886, 1912, 1914, 1915, 1918, 1924, 1940, 1948, 1950, 1960, 1963, 1969, 1977, 1995, 1999, 2002, 2003, 2012, 2016, 2038, 2045, 2050, 2056, 2060, 2071, 2103, 2104, 2106, 2107, 2115, 2128, 2170, 2189, 2191, 2206, 2255, 2273, 2275, 2305, 2306, 2322, 2323, 2363, 2377, 2404, 2405, 2410, 2412, 2413, 2415, 2441, 2442, 2443, 2444, 2460, 2461, 2462, 2478, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2496, 2497, 2498, 2513, 2514, 2517, 2518, 2554, 2556, 2573, 2574, 2588, 2599, 2600, 2602, 2638, 2647, 2655, 2664, 2668, 2669, 2688, 2755, 2789, 2790, 2791, 2792, 2793, 2803, 2804, 2811, 2814, 2826, 2867, 2887, 2890, 2891, 2895, 2896, 2918, 2919, 2923, 2927, 2945, 2948, 2993, 2994, 2995, 3001, 3002, 3006, 3007, 3015, 3028, 3029, 3031, 3032, 3046, 3050, 3051, 3052, 3054, 3059, 3060, 3061, 3062, 3063, 3064, 3077, 3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3096, 3097, 3098, 3099, 3100, 3101, 3102, 3109, 3113, 3114, 3115, 3116, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3132, 3133, 3134, 3136, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145, 3146, 3147, 3149, 3150, 3159, 3168, 3215, 3219, 3221, 3226, 3232, 3242, 3243, 3244, 3245, 3246, 3247, 3249, 3259, 3271, 3278, 3298, 3316, 3320, 3333, 3338, 3410, 3436, 3438, 3441, 3459, 3500, 3501, 3503, 3504, 3507, 3513, 3519, 3522, 3523, 3524, 3525, 3526, 3527, 3528, 3529, 3532, 3533, 3534, 3535, 3537, 3538, 3539, 3540, 3541, 3542, 3543, 3544, 3545, 3546, 3547, 3548, 3549, 3557, 3568, 3580, 3584, 3620, 3624, 3646, 3677, 3686, 3687, 3705, 3707, 3738, 3746, 3768, 3773, 3788, 3789, 3790, 3791, 3861, 3865, 3878, 3881, 3883, 3884, 3885, 3886, 3887, 3888, 3895, 3929, 3933, 3934, 3952, 3953, 3954, 3955, 3956, 3957, 3958, 3959, 3977, 3978, 3979, 3980, 3981, 3982, 3983, 3984, 3988, 3991, 4008, 4009, 4010, 4011, 4012, 4013, 4014, 4015, 4030, 4031, 4032, 4033, 4095, 4141, 4143, 4145, 4146, 4147, 4148, 4149, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4192, 4194, 4195, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4223, 4225, 4226, 4228, 4230, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4295, 4418, 4427, 4431, 4442, 4451, 4452, 4475, 4486, 4501, 4523, 4524, 4573, 4578, 4579, 4620, 4657, 4664, 4667, 4670, 4673, 4679, 4682, 4685, 4688, 4691, 4704, 4728, 4730, 4747, 4748, 4749, 4750, 4754, 4756, 4760, 4761, 4762, 4763, 4764, 4773, 4774, 4811, 4812, 4815, 4839, 4853, 4855, 4856, 4857, 4858, 4868, 4884, 4885, 4889, 4899, 4900, 4901, 4902, 4912, 4927, 4934, 4943, 4950, 4955, 4956, 4965, 4966, 4985, 4996, 4997, 5014, 5015, 5041, 5045, 5049, 5053, 5090, 5091, 5126, 5127, 5129, 5130, 5131, 5132, 5139, 5141, 5142, 5145, 5147, 5148, 5149, 5150, 5151, 5152, 5153, 5154, 5155, 5156, 5158, 5160, 5163, 5171, 5172, 5174, 5222, 5223, 5226, 5227, 5228, 5230, 5231, 5255, 5264, 5265, 5294, 5295, 5296, 5297, 5298, 5307, 5308, 5331, 5353, 5358, 5365, 5372, 5380, 5381, 5400, 5401, 5402, 5403, 5406, 5407, 5408, 5409, 5410, 5434, 5436, 5438, 5449, 5450, 5453, 5454, 5515, 5531, 5545, 5546, 5548, 5549, 5550, 5551, 5552, 5553, 5554, 5555, 5556, 5557, 5558, 5559, 5560, 5561, 5564, 5577, 5607, 5625, 5632, 5641, 5644, 5647, 5648, 5649, 5650, 5651, 5652, 5653, 5654, 5657, 5658, 5660, 5661, 5662, 5666, 5667, 5670, 5671, 5672, 5673, 5674, 5676, 5677, 5679, 5680, 5682, 5683, 5684, 5685, 5688, 5696, 5697, 5698, 5699, 5700, 5701, 5702, 5703, 5704, 5705, 5706, 5707, 5708, 5709, 5710, 5711, 5712, 5713, 5714, 5715, 5716, 5719, 5720, 5721, 5722, 5723, 5724, 5725, 5726, 5727, 5728, 5729, 5730, 5748, 5821, 5822, 5828, 5845, 5857, 5896, 5916, 5937, 5949, 5953, 5954, 5968, 6036, 6090, 6132, 6133, 6174, 6192, 6207, 6208, 6209, 6210, 6213, 6222, 6243, 6262, 6273, 6276, 6277, 6278, 6279, 6343, 6437, 6489, 6490, 6491, 6492, 6493, 6494, 6495, 6496, 6497, 6498, 6499, 6500, 6501, 6516, 6544, 6589, 6620, 6621, 6623, 6638, 6639, 6644, 6646, 6648, 6649, 6650, 6673, 6674, 6683, 6698, 6707, 6708, 6711, 6724, 6728, 6730, 6733, 6734, 6736, 6777, 6778, 6779, 6837, 6850, 6852, 6896, 6897, 6899, 6988, 7007, 7093, 7170, 7171, 7187, 7248, 7275, 7299, 7347, 7388, 7425, 7466, 7467, 7497, 7548, 7707, 7868, 7869, 7872, 7925, 7953, 7977, 7986, 7987, 7988, 7994, 8195, 8243, 8388, 8493, 8502, 8503, 8504, 8505, 8506, 8507, 8543, 8546, 8547, 8743, 8744, 8756, 8757, 8758, 8759, 8760, 8761, 8762, 8763, 8764, 8765, 8768, 8769, 8770, 8771, 8772, 8773, 8774, 8775, 8776, 8777, 8778, 8779, 8780, 8781, 8782, 8783, 8784, 8785, 8786, 8787, 8788, 8789, 8790, 8791, 8792, 8793, 8794, 8795, 8796, 8797, 8798, 8799, 8800, 8801, 8802, 8804, 8805, 8806, 8807, 8808, 8809, 8810, 8811, 8812, 8813, 8814, 8815, 8816, 8818, 8819, 8820, 8821, 8822, 8823, 8824, 8825, 8826, 8828, 8829, 8830, 8832, 8833, 8834, 8835, 8837, 8841, 8842, 8843, 8844, 8847, 8848, 8849, 8850, 8851, 8852, 8853, 8854, 8855, 8856, 8857, 8858, 8859, 8860, 8861, 8862, 8863, 8864, 8865, 8866, 8867, 8868, 8869, 8871, 8872, 8873, 8874, 8875, 8876, 8877, 8878, 8879, 8880, 8881, 8882, 8883, 8884, 8885, 8886, 8887, 8888, 8889, 8890, 8891, 8892, 8893, 8894, 8895, 8896, 8897, 8898, 8899, 8900, 8901, 8902, 8903, 8904, 8905, 8906, 8907, 8909, 8910, 8911, 8912, 8913, 8914, 8915, 8916, 8917, 8918, 8919, 8920, 8921, 8922, 8929, 8930, 8931, 8933, 8934, 8935, 8936, 8937, 8938, 8939, 8940, 8941, 8942, 8943, 8944, 8945, 8947, 8954, 8955, 8958, 8960, 8961, 8962, 8963, 8964, 8965, 8966, 8967, 8968, 8969, 8971, 8972, 8974, 8975, 8976, 8977, 8978, 8980, 8981, 8983, 8986, 8987, 8988, 8989, 8990, 8991, 8992, 8993, 8994, 8995, 8996, 8997, 8998, 8999, 9000, 9001, 9002, 9003, 9004, 9005, 9006, 9007, 9008, 9009, 9010, 9011, 9012, 9013, 9014, 9015, 9016, 9017, 9018, 9019, 9020, 9021, 9022, 9023, 9024, 9025, 9026, 9027, 9028, 9029, 9031, 9032, 9033, 9034, 9035, 9037, 9039, 9040, 9041, 9043, 9044, 9046, 9047, 9048, 9049, 9050, 9051, 9052, 9053, 9054, 9055, 9056, 9057, 9058, 9059, 9062, 9063, 9064, 9065, 9066, 9067, 9068, 9069, 9070, 9071, 9072, 9073, 9074, 9075, 9076, 9077, 9078, 9079, 9080, 9081, 9082, 9083, 9084, 9085, 9086, 9087, 9089, 9090, 9091, 9093, 9094, 9095, 9096, 9097, 9098, 9099, 9100, 9101, 9102, 9103, 9104, 9105, 9123, 9124, 9125, 9126, 9127, 9128, 9129, 9130, 9131, 9132, 9133, 9134, 9135, 9136, 9137, 9138, 9139, 9140, 9141, 9142, 9143, 9145, 9146, 9147, 9148, 9150, 9151, 9152, 9156, 9157, 9158, 9159, 9160, 9161, 9162, 9164, 9165, 9166, 9167, 9168, 9169, 9170, 9171, 9174, 9175, 9176, 9177, 9178, 9180, 9181, 9182, 9183, 9184, 9185, 9188, 9190, 9191, 9192, 9193, 9194, 9195, 9198, 9199, 9200, 9201, 9202, 9203, 9204, 9205, 9207, 9208, 9209, 9211, 9212, 9215, 9216, 9217, 9218, 9219, 9221, 9222, 9223, 9225, 9226, 9227, 9228, 9229, 9230, 9231, 9464, 9529, 9532, 9537, 10010, 10011, 10020, 10032, 10038, 10039, 10303, 10304, 10313, 10319, 10322, 10324, 10478, 10555, 10579, 10580, 10585, 10594, 10596, 10650, 10651, 10719, 10723, 11085, 11099, 11111, 11115, 11171, 11198, 11199, 11200, 11201, 11228, 11443, 11473, 11609, 11613, 11616, 11663, 11664, 11666, 11667, 11670, 11671, 11672, 11673, 11676, 11683, 11903, 12105, 12143, 12186, 12187, 12188, 12189, 12211, 12244, 12245, 12258, 12325, 12326, 12327, 12369, 12385, 12440, 12442, 12443, 12468, 12469, 12526, 12585, 12615, 12616, 12617, 12729, 12762, 12763, 12764, 12769, 12778, 12779, 12789, 12795, 12802, 12805, 12816, 12817, 12818, 12826, 12831, 12832, 12904, 12961, 12962, 12971, 12972, 13149, 13151, 13152, 13153, 13154, 13214, 13223, 13323, 13324, 13330, 13342, 13343, 13460, 13500, 13543, 13586, 13642, 13643, 13644, 13645, 13646, 13647, 13648, 13649, 13650, 13651, 13652, 13653, 13654, 13655, 13656, 13657, 13658, 13659, 13660, 13661, 13662, 13663, 13664, 13665, 13666, 13667, 13668, 13669, 13670, 13671, 13672, 13673, 13674, 13675, 13676, 13677, 13678, 13679, 13680, 13681, 13682, 13683, 13684, 13685, 13686, 13687, 13688, 13689, 13690, 13691, 13692, 13693, 13694, 13695, 13696, 13697, 13710, 13711, 13712, 13713, 13714, 13715, 13716, 13717, 13726, 13727, 13728, 13730, 13731, 13732, 13733, 13734, 13736, 13737, 13738, 13739, 13740, 13741, 13742, 13743, 13744, 13745, 13746, 13747, 13748, 13749, 13762, 13763, 13764, 13765, 13766, 13767, 13768, 13769, 13770, 13771, 13772, 13773, 13775, 13776, 13777, 13778, 13779, 13780, 13781, 13782, 13783, 13784, 13785, 13787, 13788, 13790, 13791, 13793, 13794, 13796, 13797, 13798, 13801, 13802, 13803, 13804, 13805, 13806, 13807, 13808, 13809, 13811, 13842, 13843, 13844, 13845, 13846, 13847, 13848, 13849, 14083, 14363, 14382, 14383, 14384, 14385, 14386, 14387, 14388, 14389, 14390, 14391, 14392, 14393, 14394, 14550, 14597, 14609, 14691, 14696, 14883, 14884, 14885, 14886, 14887, 14888, 14889, 14890, 14891, 14892, 15141, 15446, 15586, 15688, 15780, 15888, 15889, 16024, 16025, 16026, 16027, 16028, 16029, 16030, 16031, 16033, 16034, 16035, 16036, 16037, 16038, 16061, 16062, 16063, 16064, 16065, 16066, 16067, 16068, 16069, 16070, 16071, 16073, 16074, 16075, 16076, 16077, 16078, 16079, 16080, 16081, 16082, 16085, 16086, 16102, 16103, 16104, 16105, 16106, 16107, 16108, 16109, 16116, 16117, 16118, 16119, 16120, 16121, 16122, 16123, 16124, 16125, 16126, 16127, 16129, 16131, 16132, 16134, 16135, 16136, 16137, 16138, 16139, 16140, 16141, 16142, 16143, 16144, 16145, 16146, 16147, 16148, 16149, 16150, 16151, 16152, 16153, 16154, 16155, 16156, 16157, 16158, 16159, 16160, 16161, 16162, 16163, 16164, 16172, 16173, 16174, 16175, 16176, 16177, 16178, 16179, 16180, 16181, 16182, 16183, 16184, 16185, 16186, 16187, 16188, 16191, 16211, 16212, 16213, 16308, 16338, 16339, 16343, 16344, 16664, 16792, 17000, 17108, 17115, 17116, 17122, 17162, 17163, 17199, 17342, 17343, 17347, 17354, 17409, 17412, 17731, 17769, 17783, 17802, 17824, 17825, 17826, 17827, 17828, 17829, 17830, 17831, 17832, 17833, 17834, 17835, 17836, 17837, 17838, 17839, 17840, 17841, 17842, 17843, 17844, 17845, 17846, 17847, 17848, 17851, 17852, 17853, 17854, 17855, 17856, 17857, 17858, 17859, 17860, 17861, 17862, 17882, 17883, 17884, 17885, 17886, 17887, 17888, 17889, 17890, 17891, 17892, 17893, 17894, 17895, 17896, 17897, 17898, 17899, 17910, 17911, 17967, 18023, 18063, 18105, 18106, 18153, 18155, 18156, 18157, 18158, 18159, 18161, 18162, 18163, 18164, 18165, 18209, 18235, 18303, 18304, 18316, 18320, 18341, 18342, 18355, 18438, 18589, 18593, 18595, 18599, 18627, 18630, 18666, 18667, 18668, 18669, 18685, 18747, 18763, 18764, 18765, 18768, 18800, 18801, 18881, 18882, 18942, 18950, 18951, 18963, 18964, 18965, 18966, 18967, 18968, 18971, 18982, 19065, 19082, 19122, 19129, 19158, 19184, 19185, 19186, 19187, 19188, 19189, 19190, 19191, 19192, 19193, 19194, 19195, 19196, 19197, 19198, 19199, 19200, 19201, 19226, 19286, 19313, 19314, 19427, 19428, 19455, 19456, 19457, 19482, 19489, 19490, 19502, 19503, 19504, 19622, 19662, 19742, 19743, 19804, 19809, 19810, 19811, 19837, 19844, 19847, 19868, 19926, 19932, 19966, 19986, 19989, 20003, 20005, 20020, 20024, 20026, 20084, 20135, 20136, 20137, 20138, 20139, 20140, 20141, 20142, 20143, 20144, 20145, 20146, 20149, 20221, 20238, 20239, 20240, 20241, 20242, 20245, 20246, 20247, 20248, 20249, 20250, 20251, 20252, 20267, 20268, 20269, 20270, 20271, 20272, 20273, 20274, 20275, 20276, 20277, 20278, 20279, 20280, 20281, 20282, 20283, 20284, 20285, 20286, 20287, 20288, 20289, 20290, 20291, 20292, 20297, 20298, 20299, 20300, 20301, 20302, 20303, 20304, 20305, 20306, 20307, 20308, 20309, 20311, 20312, 20313, 20314, 20315, 20316, 20317, 20318, 20319, 20320, 20321, 20322, 20323, 20324, 20325, 20326, 20327, 20328, 20329, 20330, 20331, 20332, 20333, 20334, 20335, 20336, 20337, 20338, 20339, 20340, 20341, 20342, 20343, 20344, 20345, 20346, 20347, 20348, 20349, 20350, 20351, 20352, 20353, 20354, 20355, 20356, 20357, 20358, 20359, 20360, 20361, 20362, 20363, 20364, 20367, 20368, 20370, 20372, 20423, 20445, 20446, 20460, 20473, 20485, 20489, 20502, 20522, 20524, 20525, 20583, 20584, 20585, 20586, 20587, 20588, 20589, 20590, 20591, 20592, 20593, 20594, 20595, 20596, 20597, 20598, 20609, 20651, 20737, 20739, 20740, 20762, 20773, 20774, 20775, 20776, 20777, 20778, 20779, 20780, 20781, 20782, 20783, 20784, 20785, 20786, 20787, 20788, 20789, 20790, 20791, 20792, 20793, 20794, 20795, 20796, 20798, 20814, 20819, 20822, 20825, 20829, 20834, 20883, 20887, 20902, 20903, 20904, 20908, 20913, 20936, 20937, 20946, 20952, 20953, 20956, 20962, 20965, 20972, 20977, 20979, 20984, 21026, 21034, 21035, 21036, 21043, 21044, 21045, 21046, 21047, 21048, 21049, 21050, 21051, 21052, 21053, 21054, 21055, 21056, 21057, 21058, 21059, 21060, 21061, 21062, 21063, 21064, 21065, 21066, 21067, 21068, 21069, 21070, 21073, 21074, 21075, 21076, 21077, 21078, 21079, 21080, 21081, 21082, 21083, 21084, 21085, 21086, 21087, 21088, 21089, 21090, 21091, 21092, 21093, 21094, 21095, 21096, 21097, 21098, 21101, 21102, 21124, 21125, 21127, 21135, 21141, 21152, 21159, 21163, 21168, 21173, 21193, 21194, 21195, 21236, 21238, 21240, 21246, 21247, 21274, 21276, 21313, 21339, 21369, 21419, 21420, 21421, 21422, 21423, 21424, 21425, 21426, 21427, 21428, 21429, 21430, 21431, 21432, 21433, 21434, 21435, 21437, 21439, 21440, 21441, 21442, 21443, 21444, 21445, 21446, 21447, 21448, 21449, 21450, 21451, 21516, 21518, 21560, 21575, 21577, 21578, 21584, 21591, 21594, 21612, 21613, 21614, 21628, 21629, 21630, 21631, 21632, 21633, 21634, 21636, 21637, 21638, 21641, 21642, 21643, 21644, 21646, 21649, 21653, 21655, 21656, 21657, 21658, 21659, 21660, 21661, 21662, 21717, 21719, 21720, 21736, 21739, 21772, 21773, 21780, 21782, 21785, 21786, 21793, 21811, 21857, 21879, 21880, 21883, 21923, 21924, 21930, 21950, 21951, 21958, 21959, 21962, 21963, 21964, 22020, 22021, 22022, 22023, 22024, 22025, 22026, 22027, 22028, 22029, 22030, 22031, 22032, 22033, 22034, 22035, 22036, 22037, 22038, 22039, 22040, 22041, 22042, 22043, 22045, 22130, 22151, 22152, 22230, 22233, 22258, 22273, 22316, 22386, 22387, 22415, 22474, 22475, 22485, 22486, 22546, 22564, 22569, 22581, 22582, 22584, 22585, 22586, 22587, 22588, 22619, 22625, 22626, 22684, 22685, 22686, 22687, 22692, 22694, 22695, 22696, 22697, 22698, 22703, 22704, 22705, 22765, 22780, 22781, 22782, 22805, 22814, 22817, 22898, 22899, 22928, 22929, 22931, 22933, 22989, 23026, 23034, 23058, 23074, 23076, 23086, 23157, 23158, 23159, 23163, 23164, 23172, 23175, 23176, 23193, 23212, 23213, 23216, 23222, 23223, 23224, 23227, 23229, 23230, 23231, 23232, 23236, 23271, 23330, 23349, 23350, 23351, 23357, 23360, 23362, 23363, 23365, 23368, 23374, 23378, 23388, 23418, 23420, 23421, 23422, 23432, 23433, 23434, 23443, 23457, 23463, 23470, 23471, 23472, 23567, 23616, 23647, 23648, 23649, 23650, 23651, 23652, 23653, 23655, 23656, 23674, 23684, 23689, 23696, 23698, 23699, 23700, 23701, 23704, 23710, 23712, 23715, 23718, 23719, 23721, 23722, 23725, 23727, 23728, 23740, 23754, 23770, 23775, 23794, 23795, 23796, 23812, 23813, 23817, 23820, 23853, 23861, 23872, 23882, 23885, 23895, 23904, 23905, 23911, 23912, 23914, 23916, 23917, 23918, 23940, 23941, 23942, 23943, 23950, 23951, 23952, 23953, 23954, 23955, 23957, 23958, 23959, 23960, 23961, 23962, 23966, 23967, 23968, 23969, 23970, 23971, 23972, 23973, 23974, 23975, 23980, 23982, 23983, 23990, 23992, 23993, 24005, 24010, 24068, 24071, 24100, 24115, 24137, 24147, 24148, 24149, 24187, 24191, 24227, 24228, 24229, 24265, 24315, 24329, 24358, 24369, 24410, 24420, 24443, 24491, 24506, 24509, 24515, 24518, 24524, 24525, 24526, 24527, 24528, 24529, 24530, 24531, 24532, 24533, 24534, 24535, 24536, 24537, 24541, 24548, 24561, 24562, 24563, 24564, 24565, 24566, 24567, 24568, 24569, 24570, 24571, 24572, 25145, 25159, 25173, 25285, 25407, 25493, 25497, 25546, 25547, 25551, 25571, 25572, 25582, 25596, 25625, 25626, 25664, 25665, 25667, 25747, 25748, 25749, 25750, 25752, 25753, 25793, 25794, 25795, 25796, 25797, 25798, 25799, 25800, 25801, 25871, 25877, 25884, 25888, 26015, 26029, 26041, 26046, 26047, 26056, 26057, 26058, 26059, 26060, 26061, 26062, 26063, 26064, 26065, 26066, 26067, 26068, 26069, 26070, 26071, 26072, 26073, 26074, 26075, 26076, 26077, 26078, 26079, 26080, 26081, 26082, 26083, 26084, 26085, 26086, 26087, 26088, 26089, 26090, 26091, 26092, 26093, 26094, 26095, 26096, 26097, 26098, 26099, 26100, 26101, 26102, 26103, 26104, 26105, 26106, 26107, 26108, 26109, 26110, 26111, 26112, 26113, 26114, 26115, 26116, 26117, 26118, 26119, 26120, 26121, 26122, 26123, 26124, 26125, 26126, 26127, 26136, 26137, 26138, 26139, 26140, 26141, 26142, 26143, 26144, 26145, 26146, 26147, 26148, 26149, 26150, 26151, 26152, 26153, 26154, 26155, 26156, 26157, 26158, 26159, 26160, 26161, 26162, 26163, 26164, 26165, 26166, 26167, 26168, 26169, 26170, 26171, 26172, 26173, 26174, 26175, 26176, 26177, 26178, 26179, 26180, 26181, 26182, 26183, 26184, 26185, 26186, 26187, 26188, 26189, 26190, 26191, 26192, 26193, 26194, 26195, 26196, 26197, 26198, 26199, 26200, 26201, 26202, 26203, 26204, 26205, 26206, 26207, 26208, 26209, 26210, 26211, 26212, 26213, 26214, 26215, 26216, 26217, 26218, 26219, 26220, 26221, 26222, 26223, 26224, 26225, 26226, 26227, 26228, 26229, 26230, 26231, 26232, 26233, 26234, 26235, 26236, 26237, 26238, 26239, 26240, 26241, 26242, 26243, 26244, 26245, 26246, 26247, 26248, 26249, 26250, 26251, 26252, 26253, 26254, 26255, 26256, 26257, 26258, 26259, 26260, 26261, 26262, 26263, 26264, 26265, 26266, 26267, 26268, 26269, 26270, 26271, 26272, 26273, 26274, 26275, 26276, 26277, 26278, 26279, 26280, 26281, 26282, 26283, 26284, 26285, 26286, 26287, 26288, 26289, 26290, 26291, 26292, 26293, 26294, 26295, 26296, 26297, 26298, 26299, 26300, 26301, 26302, 26303, 26304, 26305, 26306, 26307, 26308, 26309, 26310, 26311, 26312, 26313, 26314, 26315, 26316, 26317, 26318, 26319, 26320, 26321, 26322, 26323, 26324, 26325, 26326, 26327, 26328, 26329, 26330, 26331, 26332, 26333, 26334, 26335, 26336, 26337, 26338, 26339, 26340, 26341, 26342, 26343, 26344, 26345, 26346, 26347, 26348, 26349, 26350, 26351, 26352, 26353, 26354, 26355, 26356, 26357, 26358, 26359, 26360, 26361, 26362, 26363, 26364, 26365, 26366, 26367, 26368, 26369, 26370, 26371, 26372, 26373, 26374, 26375, 26376, 26377, 26378, 26379, 26380, 26381, 26382, 26383, 26384, 26385, 26386, 26387, 26388, 26389, 26390, 26391, 26392, 26393, 26394, 26395, 26396, 26397, 26398, 26399, 26400, 26401, 26402, 26403, 26404, 26405, 26406, 26407, 26408, 26409, 26410, 26411, 26412, 26413, 26414, 26415, 26416, 26417, 26418, 26419, 26420, 26421, 26422, 26423, 26424, 26425, 26426, 26427, 26428, 26429, 26430, 26431, 26432, 26433, 26434, 26435, 26436, 26437, 26438, 26439, 26440, 26441, 26442, 26443, 26444, 26445, 26446, 26447, 26448, 26449, 26450, 26451, 26452, 26453, 26454, 26455, 26456, 26457, 26458, 26459, 26460, 26461, 26462, 26463, 26464, 26465, 26466, 26467, 26468, 26469, 26470, 26471, 26472, 26473, 26474, 26475, 26476, 26477, 26478, 26479, 26480, 26481, 26482, 26483, 26484, 26485, 26486, 26487, 26488, 26489, 26490, 26491, 26492, 26493, 26494, 26495, 26496, 26497, 26498, 26499, 26500, 26501, 26502, 26503, 26504, 26505, 26506, 26507, 26508, 26509, 26510, 26511, 26512, 26514, 26515, 26516, 26517, 26518, 26519, 26520, 26521, 26522, 26523, 26524, 26525, 26526, 26528, 26529, 26530, 26531, 26532, 26533, 26534, 26535, 26536, 26537, 26538, 26539, 26540, 26542, 26543, 26544, 26545, 26546, 26547, 26548, 26549, 26550, 26551, 26552, 26553, 26554, 26555, 26556, 26557, 26558, 26559, 26560, 26561, 26562, 26563, 26564, 26565, 26566, 26567, 26568, 26570, 26571, 26572, 26573, 26574, 26575, 26576, 26577, 26578, 26579, 26580, 26581, 26582, 26583, 26584, 26585, 26586, 26587, 26588, 26589, 26590, 26591, 26592, 26593, 26594, 26595, 26596, 26597, 26598, 26599, 26600, 26601, 26602, 26603, 26604, 26605, 26606, 26607, 26608, 26609, 26610, 26611, 26612, 26613, 26614, 26615, 26616, 26617, 26618, 26619, 26620, 26621, 26622, 26623, 26624, 26625, 26626, 26627, 26628, 26629, 26630, 26631, 26632, 26633, 26634, 26635, 26636, 26637, 26638, 26639, 26640, 26641, 26642, 26643, 26644, 26645, 26646, 26647, 26648, 26649, 26650, 26651, 26652, 26653, 26654, 26655, 26656, 26657, 26658, 26659, 26660, 26661, 26662, 26663, 26664, 26665, 26666, 26667, 26668, 26669, 26670, 26671, 26672, 26673, 26674, 26675, 26676, 26677, 26678, 26679, 26680, 26681, 26682, 26683, 26684, 26685, 26686, 26687, 26688, 26689, 26690, 26691, 26692, 26693, 26694, 26695, 26696, 26697, 26698, 26699, 26700, 26701, 26702, 26703, 26704, 26705, 26706, 26707, 26708, 26709, 26710, 26711, 26712, 26713, 26714, 26715, 26716, 26717, 26718, 26719, 26720, 26721, 26722, 26723, 26724, 26725, 26726, 26727, 26728, 26729, 26730, 26731, 26732, 26733, 26734, 26735, 26736, 26737, 26738, 26739, 26740, 26741, 26742, 26743, 26744, 26745, 26746, 26747, 26748, 26749, 26750, 26751, 26752, 26753, 26754, 26755, 26756, 26757, 26758, 26759, 26760, 26761, 26762, 26763, 26764, 26766, 26767, 26768, 26769, 26770, 26771, 26772, 26773, 26774, 26775, 26776, 26777, 26778, 26780, 26781, 26782, 26783, 26784, 26785, 26786, 26787, 26788, 26789, 26790, 26791, 26792, 26793, 26794, 26795, 26796, 26797, 26798, 26799, 26800, 26801, 26802, 26803, 26804, 26805, 26806, 26807, 26808, 26809, 26810, 26811, 26812, 26813, 26814, 26815, 26816, 26817, 26818, 26819, 26820, 26821, 26822, 26823, 26824, 26825, 26826, 26827, 26828, 26829, 26830, 26831, 26832, 26833, 26834, 26835, 26836, 26837, 26838, 26839, 26840, 26841, 26842, 26843, 26844, 26845, 26846, 26847, 26848, 26849, 26850, 26851, 26852, 26853, 26854, 26855, 26856, 26857, 26858, 26859, 26860, 26861, 26862, 26863, 26864, 26865, 26866, 26867, 26868, 26869, 26870, 26871, 26872, 26873, 26874, 26875, 26876, 26877, 26878, 26879, 26880, 26881, 26882, 26883, 26884, 26885, 26886, 26887, 26888, 26889, 26890, 26891, 26892, 26893, 26894, 26895, 26896, 26897, 26898, 26899, 26900, 26901, 26902, 26903, 26904, 26905, 26906, 26907, 26908, 26909, 26910, 26911, 26912, 26913, 26914, 26915, 26916, 26917, 26918, 26919, 26920, 26921, 26922, 26923, 26924, 26925, 26926, 26927, 26928, 26929, 26930, 26931, 26932, 26933, 26934, 26935, 26936, 26937, 26938, 26939, 26940, 26941, 26942, 26943, 26944, 26945, 26946, 26947, 26948, 26949, 26950, 26951, 26952, 26953, 26954, 26955, 26956, 26957, 26958, 26959, 26960, 26961, 26962, 26963, 26964, 26965, 26966, 26967, 26968, 26969, 26970, 26971, 26972, 26973, 26974, 26975, 26976, 26977, 26978, 26979, 26980, 26981, 26982, 26983, 26984, 26985, 26986, 26987, 26988, 26989, 26990, 26991, 26992, 26993, 26994, 26995, 26996, 26997, 26998, 26999, 27000, 27001, 27002, 27003, 27004, 27005, 27006, 27007, 27008, 27009, 27010, 27011, 27012, 27013, 27014, 27015, 27016, 27017, 27018, 27019, 27020, 27021, 27022, 27023, 27024, 27025, 27026, 27027, 27028, 27029, 27030, 27031, 27032, 27033, 27034, 27035, 27036, 27037, 27038, 27039, 27040, 27041, 27042, 27043, 27044, 27045, 27046, 27047, 27048, 27049, 27050, 27051, 27052, 27053, 27054, 27055, 27056, 27057, 27058, 27059, 27060, 27061, 27062, 27063, 27064, 27065, 27066, 27067, 27068, 27069, 27070, 27071, 27072, 27073, 27074, 27075, 27076, 27077, 27078, 27079, 27080, 27081, 27082, 27083, 27084, 27085, 27086, 27087, 27088, 27089, 27090, 27091, 27092, 27093, 27094, 27095, 27096, 27097, 27098, 27099, 27100, 27101, 27102, 27103, 27104, 27105, 27106, 27107, 27108, 27109, 27110, 27111, 27112, 27113, 27114, 27115, 27116, 27117, 27118, 27119, 27120, 27121, 27122, 27123, 27124, 27125, 27126, 27127, 27128, 27129, 27130, 27131, 27132, 27133, 27134, 27135, 27136, 27137, 27138, 27139, 27140, 27141, 27142, 27143, 27144, 27145, 27146, 27147, 27148, 27149, 27150, 27151, 27152, 27153, 27154, 27155, 27156, 27157, 27158, 27159, 27160, 27161, 27162, 27163, 27164, 27165, 27166, 27167, 27168, 27169, 27170, 27171, 27172, 27173, 27174, 27175, 27176, 27177, 27178, 27179, 27180, 27181, 27182, 27183, 27184, 27185, 27186, 27187, 27188, 27189, 27190, 27191, 27192, 27193, 27194, 27195, 27196, 27197, 27198, 27199, 27200, 27201, 27202, 27203, 27204, 27205, 27206, 27207, 27208, 27209, 27210, 27211, 27212, 27213, 27214, 27215, 27216, 27217, 27218, 27219, 27220, 27221, 27222, 27223, 27224, 27225, 27226, 27227, 27228, 27229, 27230, 27231, 27232, 27233, 27234, 27235, 27236, 27237, 27238, 27239, 27240, 27241, 27242, 27243, 27244, 27245, 27246, 27247, 27248, 27249, 27250, 27251, 27252, 27253, 27254, 27255, 27256, 27257, 27258, 27259, 27260, 27261, 27262, 27263, 27264, 27265, 27266, 27267, 27268, 27269, 27270, 27271, 27272, 27273, 27274, 27275, 27276, 27277, 27278, 27279, 27280, 27281, 27282, 27283, 27284, 27285, 27286, 27287, 27288, 27289, 27290, 27291, 27292, 27293, 27294, 27295, 27296, 27297, 27298, 27299, 27300, 27301, 27302, 27303, 27304, 27305, 27306, 27307, 27308, 27309, 27310, 27311, 27312, 27313, 27314, 27315, 27316, 27318, 27319, 27320, 27321, 27322, 27323, 27324, 27325, 27326, 27327, 27328, 27329, 27330, 27331, 27332, 27333, 27334, 27335, 27336, 27337, 27338, 27339, 27340, 27341, 27342, 27343, 27344, 27345, 27346, 27347, 27348, 27349, 27350, 27351, 27352, 27353, 27354, 27355, 27356, 27357, 27358, 27359, 27360, 27361, 27362, 27363, 27364, 27365, 27366, 27367, 27368, 27369, 27370, 27371, 27372, 27373, 27374, 27375, 27376, 27377, 27378, 27379, 27380, 27381, 27382, 27383, 27384, 27385, 27386, 27387, 27391, 27392, 27393, 27394, 27395, 27396, 27397, 27419, 27421, 27444, 27486, 27530, 27554, 27555, 27556, 27557, 27558, 27559, 27560, 27561, 27562, 27563, 27564, 27565, 27566, 27567, 27568, 27569, 27570, 27571, 27572, 27573, 27574, 27575, 27576, 27577, 27578, 27579, 27580, 27581, 27582, 27583, 27584, 27585, 27586, 27587, 27588, 27589, 27590, 27591, 27592, 27593, 27594, 27595, 27596, 27597, 27598, 27599, 27600, 27601, 27602, 27603, 27604, 27605, 27606, 27607, 27608, 27609, 27610, 27611, 27612, 27613, 27614, 27615, 27616, 27617, 27618, 27619, 27620, 27621, 27622, 27623, 27624, 27625, 27626, 27627, 27628, 27629, 27630, 27819, 27853, 27863, 27894, 27950, 27951, 27952, 27953, 27954, 27955, 27956, 27957, 27958, 27959, 27960, 27961, 27962, 27963, 27964, 27965, 27966, 27967, 27968, 27969, 27970, 27971, 27972, 27973, 27974, 27975, 27997, 27998, 27999, 28000, 28001, 28002, 28003, 28004, 28005, 28006, 28007, 28008, 28009, 28010, 28011, 28012, 28013, 28014, 28015, 28016, 28017, 28018, 28019, 28020, 28021, 28022, 28025, 28049, 28076, 28077, 28078, 28079, 28080, 28081, 28082, 28083, 28084, 28085, 28086, 28088, 28089, 28090, 28091, 28092, 28093, 28094, 28095, 28096, 28097, 28098, 28113, 28114, 28135, 28145, 28165, 28261, 28289, 28366, 28471, 28482, 28546, 28549, 28676, 28798, 28816, 29025, 29041, 29052, 29120, 29188, 29208, 29210, 29225, 29237, 29293, 29402, 29427, 29551, 29552, 29557, 29558, 29565, 29566, 29585, 29802, 29823, 29824, 29825, 29826, 29827, 29828, 29829, 29830, 29831, 29832, 29833, 29834, 29835, 29836, 29837, 29838, 29839, 29840, 29841, 29842, 29843, 29844, 29845, 29846, 29847, 29848, 29849, 29850, 29851, 29852, 29853, 29854, 29855, 29856, 29857, 29858, 29859, 29860, 29861, 29862, 29863, 29864, 29865, 29866, 29867, 29868, 29869, 29870, 29871, 29872, 29873, 29874, 29875, 29876, 29877, 29878, 29879, 29880, 29881, 29882, 29883, 29884, 29885, 29886, 29887, 29888, 29889, 29890, 29891, 29892, 29893, 29894, 29895, 29896, 29897, 29898, 29899, 29907, 30176, 30191, 30193, 30195, 30197, 30198, 30199, 30287, 30288, 30289, 30292, 30325, 30347, 30385, 30459, 30460, 30461, 30464, 30465, 30469, 30470, 30471, 30472, 30473, 30474, 30491, 30526, 30528, 30545, 30595, 30711, 30718, 30760, 30793, 30796, 30806, 30877, 30920, 30921, 30934, 30935, 30949, 30954, 30963, 30965, 31087, 31167, 31253, 31257, 31259, 31265, 31266, 31267, 31389, 31469, 31496, 31498, 31500, 31502, 31503, 31505, 31506, 31507, 31575, 31665, 31667, 31767, 31842, 31843, 31844, 31845, 31846, 31847, 31848, 31849, 31850, 31851, 31930, 31931, 31932, 31933, 31934, 31947, 31948, 31954, 32075, 32091, 32093, 32094, 32095, 32096, 32097, 32098, 32099, 32100, 32101, 32102, 32103, 32104, 32105, 32106, 32107, 32108, 32109, 32110, 32111, 32112, 32113, 32114, 32115, 32116, 32117, 32118, 32119, 32120, 32121, 32122, 32123, 32124, 32125, 32126, 32127, 32128, 32129, 32130, 32131, 32132, 32133, 32134, 32135, 32136, 32137, 32138, 32139, 32140, 32141, 32142, 32143, 32144, 32145, 32146, 32147, 32148, 32149, 32150, 32151, 32152, 32153, 32154, 32155, 32156, 32157, 32158, 32159, 32160, 32161, 32162, 32163, 32164, 32165, 32166, 32167, 32168, 32169, 32170, 32171, 32172, 32173, 32174, 32175, 32176, 32177, 32178, 32179, 32180, 32181, 32182, 32183, 32184, 32185, 32186, 32187, 32188, 32189, 32190, 32191, 32192, 32372, 32407, 32414, 32415, 32416, 32417, 32418, 32419, 32421, 32422, 32424, 32426, 32457, 32465, 32466, 32482, 32498, 32543, 32545, 32546, 32547, 32548, 32549, 32550, 32551, 32552, 32553, 32554, 32555, 32556, 32557, 32558, 32559, 32560, 32561, 32594, 32595, 32618, 32642, 32761, 32762, 32763, 32764, 32765, 32766, 32767, 32775, 32824, 32908, 32913, 32914, 32916, 32921, 32949, 32950, 32951, 32952, 32954, 32955, 32956, 32957, 32958, 32959, 32965, 32966, 32967, 32970, 32972, 32973, 32974, 32975, 32976, 32977, 32978, 32982, 32983, 32984, 32985, 32986, 32987, 32991, 32992, 32993, 32994, 32995, 32996, 33001, 33003, 33004, 33005, 33022, 33027, 33046, 33062, 33063, 33073, 33074, 33075, 33097, 33104, 33118, 33193, 33194, 33195, 33196, 33197, 33198, 33199, 33200, 33201, 33217, 33288, 33302, 33315, 33442, 33482, 33542, 33543, 33570, 33572, 33573, 33574, 33600, 33601, 33602, 33603, 33623, 33624, 33625, 33626, 33633, 33772, 33775, 33776, 33777, 33785, 33786, 33787, 33788, 33807, 33957, 33958, 33959, 33964, 33986, 33987, 33988, 33989, 33994, 33995, 33996, 33997, 33998, 34000, 34001, 34002, 34003, 34004, 34005, 34006, 34007, 34036, 34037, 34038, 34039, 34044, 34046, 34047, 34048, 34071, 34094, 34095, 34107, 34150, 34151, 34152, 34153, 34154, 34155, 34156, 34158, 34159, 34161, 34250, 34251, 34252, 34415, 34465, 34466, 34467, 34481, 34494, 34496, 34498, 34503, 34518, 34519, 34534, 34550, 34551, 34626, 34627, 34646, 34712, 34776, 34805, 34835, 35485, 35499, 35517, 35518, 35519, 35520, 35521, 35522, 35523, 35524, 35525, 35526, 35527, 35528, 35529, 35530, 35531, 35532, 35533, 35534, 35535, 35536, 35537, 35538, 35539, 35540, 35541, 35542, 35544, 35545, 35546, 35548, 35549, 35550, 35551, 35552, 35553, 35554, 35555, 35556, 35674, 35710, 35713, 35721, 35722, 35732, 35874, 36876, 36877, 37298, 37311, 37312, 37313, 37460, 37567, 37596, 37598, 37604, 37605, 37606, 37736, 37737, 37739, 37740, 37827, 37892, 37893, 37894, 37895, 37896, 37897, 37898, 37899, 37900, 37901, 37902, 37903, 37904, 37905, 37906, 37907, 37908, 37909, 38579, 39149 };
#else
        static std::unordered_set<uint32> unavailableItemIDs = { 17, 41, 42, 46, 50, 54, 58, 77, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101, 102, 103, 104, 105, 113, 114, 115, 122, 123, 124, 125, 126, 130, 131, 132, 133, 134, 135, 136, 137, 138, 141, 143, 146, 149, 150, 151, 152, 155, 156, 157, 184, 527, 734, 741, 746, 751, 761, 784, 786, 788, 806, 807, 808, 813, 823, 836, 842, 855, 875, 876, 877, 883, 894, 898, 900, 901, 902, 903, 905, 906, 907, 908, 909, 913, 917, 930, 931, 941, 945, 948, 951, 956, 958, 960, 964, 965, 966, 967, 968, 973, 974, 975, 976, 980, 985, 986, 989, 992, 994, 996, 997, 1004, 1014, 1016, 1018, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1041, 1042, 1043, 1044, 1046, 1047, 1048, 1049, 1052, 1053, 1057, 1058, 1061, 1063, 1072, 1078, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1095, 1096, 1099, 1100, 1101, 1102, 1105, 1108, 1109, 1111, 1112, 1115, 1117, 1119, 1122, 1123, 1124, 1125, 1128, 1133, 1134, 1136, 1138, 1139, 1141, 1144, 1146, 1149, 1150, 1151, 1157, 1162, 1163, 1164, 1165, 1166, 1167, 1170, 1174, 1176, 1184, 1186, 1192, 1193, 1199, 1216, 1222, 1224, 1228, 1229, 1231, 1232, 1238, 1239, 1243, 1244, 1245, 1246, 1250, 1253, 1255, 1258, 1259, 1266, 1267, 1268, 1269, 1272, 1279, 1281, 1298, 1311, 1312, 1313, 1321, 1323, 1324, 1328, 1332, 1334, 1339, 1341, 1350, 1352, 1354, 1356, 1363, 1371, 1379, 1385, 1386, 1392, 1397, 1398, 1400, 1402, 1403, 1424, 1432, 1435, 1444, 1450, 1472, 1492, 1500, 1508, 1527, 1533, 1534, 1535, 1536, 1544, 1545, 1554, 1559, 1567, 1568, 1571, 1574, 1588, 1589, 1591, 1597, 1599, 1603, 1612, 1619, 1622, 1623, 1638, 1641, 1648, 1649, 1651, 1654, 1655, 1657, 1658, 1663, 1672, 1676, 1681, 1684, 1689, 1690, 1691, 1692, 1693, 1694, 1695, 1698, 1699, 1700, 1704, 1719, 1724, 1736, 1851, 1854, 1877, 1882, 1886, 1895, 1896, 1897, 1899, 1900, 1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908, 1909, 1910, 1911, 1912, 1914, 1915, 1918, 1924, 1940, 1948, 1950, 1957, 1960, 1961, 1963, 1969, 1977, 1983, 1984, 1985, 1995, 1999, 2002, 2003, 2012, 2016, 2023, 2038, 2045, 2050, 2051, 2052, 2053, 2056, 2060, 2071, 2081, 2103, 2104, 2106, 2107, 2115, 2128, 2147, 2170, 2176, 2177, 2178, 2179, 2180, 2181, 2182, 2183, 2184, 2189, 2191, 2196, 2197, 2198, 2199, 2200, 2201, 2202, 2206, 2255, 2273, 2275, 2305, 2306, 2322, 2323, 2363, 2377, 2404, 2405, 2410, 2412, 2413, 2415, 2441, 2442, 2443, 2444, 2461, 2462, 2478, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2496, 2497, 2498, 2513, 2514, 2517, 2518, 2554, 2556, 2573, 2574, 2588, 2599, 2600, 2602, 2638, 2647, 2655, 2664, 2668, 2669, 2688, 2695, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2710, 2711, 2714, 2716, 2717, 2718, 2755, 2789, 2790, 2791, 2792, 2793, 2803, 2804, 2809, 2810, 2811, 2812, 2813, 2814, 2826, 2827, 2884, 2887, 2890, 2891, 2895, 2896, 2918, 2919, 2923, 2927, 2945, 2948, 2993, 2994, 2995, 3001, 3002, 3006, 3007, 3015, 3028, 3029, 3031, 3032, 3044, 3046, 3050, 3051, 3052, 3054, 3059, 3060, 3061, 3062, 3063, 3064, 3077, 3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3096, 3097, 3098, 3099, 3100, 3101, 3102, 3109, 3113, 3114, 3115, 3116, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3132, 3133, 3134, 3136, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145, 3146, 3147, 3149, 3150, 3159, 3168, 3215, 3219, 3221, 3226, 3232, 3242, 3243, 3244, 3245, 3246, 3247, 3249, 3259, 3271, 3278, 3298, 3316, 3320, 3326, 3333, 3338, 3346, 3350, 3351, 3361, 3362, 3364, 3366, 3367, 3368, 3410, 3432, 3433, 3436, 3438, 3441, 3459, 3494, 3500, 3501, 3503, 3504, 3507, 3513, 3519, 3522, 3523, 3524, 3525, 3526, 3527, 3528, 3529, 3532, 3533, 3534, 3535, 3537, 3538, 3539, 3540, 3541, 3542, 3543, 3544, 3545, 3546, 3547, 3548, 3549, 3557, 3568, 3580, 3584, 3620, 3624, 3646, 3677, 3686, 3687, 3694, 3695, 3696, 3697, 3698, 3699, 3705, 3707, 3738, 3746, 3756, 3757, 3768, 3773, 3774, 3776, 3788, 3789, 3790, 3791, 3861, 3865, 3878, 3881, 3883, 3884, 3885, 3886, 3887, 3888, 3895, 3929, 3933, 3934, 3952, 3953, 3954, 3955, 3956, 3957, 3958, 3959, 3977, 3978, 3979, 3980, 3981, 3982, 3983, 3984, 3988, 3991, 4008, 4009, 4010, 4011, 4012, 4013, 4014, 4015, 4030, 4031, 4032, 4033, 4095, 4141, 4143, 4145, 4146, 4147, 4148, 4149, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4192, 4194, 4195, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4223, 4225, 4226, 4228, 4230, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4295, 4418, 4427, 4431, 4442, 4451, 4452, 4475, 4486, 4501, 4523, 4524, 4573, 4578, 4579, 4620, 4657, 4664, 4667, 4670, 4673, 4679, 4682, 4685, 4688, 4691, 4704, 4728, 4730, 4747, 4748, 4749, 4750, 4754, 4756, 4760, 4761, 4762, 4763, 4764, 4773, 4774, 4811, 4812, 4815, 4839, 4853, 4855, 4856, 4857, 4858, 4868, 4884, 4885, 4889, 4899, 4900, 4901, 4902, 4912, 4927, 4934, 4943, 4950, 4955, 4956, 4965, 4966, 4985, 4991, 4993, 4994, 4996, 4997, 5014, 5015, 5041, 5045, 5049, 5053, 5090, 5091, 5126, 5127, 5129, 5130, 5131, 5132, 5139, 5141, 5142, 5145, 5147, 5148, 5149, 5150, 5151, 5152, 5153, 5154, 5155, 5156, 5158, 5160, 5163, 5171, 5172, 5174, 5222, 5223, 5226, 5227, 5228, 5230, 5231, 5255, 5258, 5259, 5260, 5261, 5262, 5264, 5265, 5276, 5277, 5278, 5280, 5281, 5282, 5283, 5284, 5285, 5286, 5287, 5288, 5289, 5291, 5292, 5293, 5294, 5295, 5296, 5297, 5298, 5300, 5301, 5303, 5304, 5305, 5307, 5308, 5331, 5353, 5358, 5365, 5372, 5380, 5381, 5400, 5401, 5402, 5403, 5406, 5407, 5408, 5409, 5410, 5434, 5436, 5438, 5449, 5450, 5453, 5454, 5491, 5495, 5502, 5515, 5531, 5532, 5545, 5546, 5548, 5549, 5550, 5551, 5552, 5553, 5554, 5555, 5556, 5557, 5558, 5559, 5560, 5561, 5564, 5577, 5597, 5598, 5599, 5600, 5607, 5625, 5632, 5641, 5644, 5647, 5648, 5649, 5650, 5651, 5652, 5653, 5654, 5657, 5658, 5660, 5661, 5662, 5666, 5667, 5670, 5671, 5672, 5673, 5674, 5676, 5677, 5679, 5680, 5682, 5683, 5684, 5685, 5688, 5696, 5697, 5698, 5699, 5700, 5701, 5702, 5703, 5704, 5705, 5706, 5707, 5708, 5709, 5710, 5711, 5712, 5713, 5714, 5715, 5716, 5719, 5720, 5721, 5722, 5723, 5724, 5725, 5726, 5727, 5728, 5729, 5730, 5745, 5746, 5747, 5748, 5821, 5822, 5828, 5856, 5857, 5870, 5896, 5916, 5937, 5949, 5953, 5954, 5968, 6036, 6088, 6090, 6132, 6133, 6174, 6192, 6207, 6208, 6209, 6210, 6213, 6222, 6224, 6225, 6227, 6228, 6229, 6230, 6231, 6232, 6233, 6234, 6235, 6236, 6237, 6254, 6262, 6273, 6276, 6277, 6278, 6279, 6322, 6334, 6343, 6434, 6437, 6489, 6490, 6491, 6492, 6493, 6494, 6495, 6496, 6497, 6498, 6499, 6500, 6501, 6516, 6544, 6589, 6618, 6620, 6621, 6623, 6638, 6639, 6644, 6646, 6648, 6649, 6650, 6673, 6674, 6680, 6683, 6698, 6707, 6708, 6711, 6724, 6728, 6734, 6736, 6777, 6778, 6779, 6837, 6850, 6852, 6886, 6896, 6897, 6899, 6946, 6988, 7007, 7093, 7170, 7171, 7187, 7248, 7275, 7299, 7347, 7388, 7425, 7466, 7467, 7497, 7548, 7612, 7706, 7707, 7826, 7868, 7869, 7872, 7953, 7977, 7986, 7987, 7988, 7994, 8148, 8243, 8388, 8493, 8502, 8503, 8504, 8505, 8506, 8507, 8543, 8546, 8547, 8743, 8744, 8756, 8757, 8758, 8759, 8760, 8761, 8762, 8763, 8764, 8765, 8768, 8769, 8770, 8771, 8772, 8773, 8774, 8775, 8776, 8777, 8778, 8779, 8780, 8781, 8782, 8783, 8784, 8785, 8786, 8787, 8788, 8789, 8790, 8791, 8792, 8793, 8794, 8795, 8796, 8797, 8798, 8799, 8800, 8801, 8802, 8804, 8805, 8806, 8807, 8808, 8809, 8810, 8811, 8812, 8813, 8814, 8815, 8816, 8818, 8819, 8820, 8821, 8822, 8823, 8824, 8825, 8826, 8828, 8829, 8830, 8832, 8833, 8834, 8835, 8837, 8841, 8842, 8843, 8844, 8847, 8848, 8849, 8850, 8851, 8852, 8853, 8854, 8855, 8856, 8857, 8858, 8859, 8860, 8861, 8862, 8863, 8864, 8865, 8866, 8867, 8868, 8869, 8871, 8872, 8873, 8874, 8875, 8876, 8877, 8878, 8879, 8880, 8881, 8882, 8883, 8884, 8885, 8886, 8887, 8888, 8889, 8890, 8891, 8892, 8893, 8894, 8895, 8896, 8897, 8898, 8899, 8900, 8901, 8902, 8903, 8904, 8905, 8906, 8907, 8909, 8910, 8911, 8912, 8913, 8914, 8915, 8916, 8917, 8918, 8919, 8920, 8921, 8922, 8929, 8930, 8931, 8933, 8934, 8935, 8936, 8937, 8938, 8939, 8940, 8941, 8942, 8943, 8944, 8945, 8947, 8954, 8955, 8958, 8960, 8961, 8962, 8963, 8964, 8965, 8966, 8967, 8968, 8969, 8971, 8972, 8974, 8975, 8976, 8977, 8978, 8980, 8981, 8983, 8986, 8987, 8988, 8989, 8990, 8991, 8992, 8993, 8994, 8995, 8996, 8997, 8998, 8999, 9000, 9001, 9002, 9003, 9004, 9005, 9006, 9007, 9008, 9009, 9010, 9011, 9012, 9013, 9014, 9015, 9016, 9017, 9018, 9019, 9020, 9021, 9022, 9023, 9024, 9025, 9026, 9027, 9028, 9029, 9031, 9032, 9033, 9034, 9035, 9037, 9039, 9040, 9041, 9043, 9044, 9046, 9047, 9048, 9049, 9050, 9051, 9052, 9053, 9054, 9055, 9056, 9057, 9058, 9059, 9062, 9063, 9064, 9065, 9066, 9067, 9068, 9069, 9070, 9071, 9072, 9073, 9074, 9075, 9076, 9077, 9078, 9079, 9080, 9081, 9082, 9083, 9084, 9085, 9086, 9087, 9089, 9090, 9091, 9093, 9094, 9095, 9096, 9097, 9098, 9099, 9100, 9101, 9102, 9103, 9104, 9105, 9123, 9124, 9125, 9126, 9127, 9128, 9129, 9130, 9131, 9132, 9133, 9134, 9135, 9136, 9137, 9138, 9139, 9140, 9141, 9142, 9143, 9145, 9146, 9147, 9148, 9150, 9151, 9152, 9156, 9157, 9158, 9159, 9160, 9161, 9162, 9164, 9165, 9166, 9167, 9168, 9169, 9170, 9171, 9174, 9175, 9176, 9177, 9178, 9180, 9181, 9182, 9183, 9184, 9185, 9188, 9190, 9191, 9192, 9193, 9194, 9195, 9198, 9199, 9200, 9201, 9202, 9203, 9204, 9205, 9207, 9208, 9209, 9211, 9212, 9215, 9216, 9217, 9218, 9219, 9221, 9222, 9223, 9225, 9226, 9227, 9228, 9229, 9230, 9231, 9464, 9529, 9532, 9537, 9659, 9702, 10303, 10304, 10313, 10319, 10322, 10324, 10478, 10555, 10568, 10579, 10591, 10594, 10596, 10611, 10612, 10613, 10614, 10615, 10616, 10617, 10618, 10619, 10650, 10651, 10685, 10756, 10825, 10878, 10898, 11019, 11021, 11025, 11041, 11042, 11085, 11087, 11099, 11111, 11115, 11171, 11198, 11199, 11200, 11201, 11228, 11264, 11314, 11317, 11321, 11322, 11323, 11342, 11343, 11365, 11369, 11383, 11424, 11443, 11473, 11505, 11506, 11542, 11585, 11586, 11587, 11588, 11589, 11591, 11609, 11613, 11616, 11663, 11664, 11666, 11667, 11670, 11671, 11672, 11673, 11676, 11683, 11762, 11763, 11838, 11903, 12063, 12105, 12142, 12143, 12182, 12183, 12186, 12187, 12188, 12189, 12211, 12244, 12245, 12258, 12285, 12290, 12294, 12297, 12298, 12304, 12322, 12325, 12326, 12327, 12328, 12329, 12331, 12332, 12338, 12348, 12369, 12385, 12403, 12421, 12440, 12442, 12443, 12452, 12453, 12454, 12456, 12461, 12468, 12469, 12482, 12502, 12523, 12526, 12585, 12591, 12593, 12615, 12616, 12617, 12629, 12729, 12742, 12743, 12744, 12745, 12746, 12747, 12748, 12749, 12750, 12751, 12754, 12755, 12762, 12763, 12778, 12786, 12787, 12788, 12789, 12801, 12805, 12816, 12817, 12818, 12826, 12831, 12832, 12850, 12851, 12852, 12853, 12854, 12855, 12856, 12857, 12858, 12859, 12860, 12861, 12862, 12863, 12864, 12865, 12867, 12868, 12869, 12870, 12882, 12883, 12889, 12890, 12892, 12893, 12901, 12902, 12904, 12931, 12932, 12933, 12934, 12937, 12941, 12943, 12944, 12949, 12950, 12951, 12959, 12961, 12962, 12971, 12972, 12980, 12981, 12991, 12993, 12995, 13050, 13061, 13069, 13078, 13104, 13147, 13149, 13150, 13151, 13152, 13153, 13154, 13160, 13165, 13214, 13219, 13220, 13221, 13222, 13223, 13290, 13291, 13292, 13293, 13312, 13316, 13318, 13319, 13323, 13324, 13330, 13336, 13337, 13338, 13339, 13341, 13342, 13343, 13406, 13407, 13500, 13504, 13543, 13586, 13604, 13605, 13606, 13607, 13608, 13609, 13610, 13611, 13622, 13623, 13625, 13627, 13628, 13629, 13630, 13631, 13632, 13642, 13643, 13644, 13645, 13646, 13647, 13648, 13649, 13650, 13651, 13652, 13653, 13654, 13655, 13656, 13657, 13658, 13659, 13660, 13661, 13662, 13663, 13664, 13665, 13666, 13667, 13668, 13669, 13670, 13671, 13672, 13673, 13674, 13675, 13676, 13677, 13678, 13679, 13680, 13681, 13682, 13683, 13684, 13685, 13686, 13687, 13688, 13689, 13690, 13691, 13692, 13693, 13694, 13695, 13696, 13697, 13698, 13705, 13706, 13707, 13708, 13709, 13710, 13711, 13712, 13713, 13714, 13715, 13716, 13717, 13718, 13719, 13720, 13721, 13722, 13723, 13726, 13727, 13728, 13730, 13731, 13732, 13733, 13734, 13736, 13737, 13738, 13739, 13740, 13741, 13742, 13743, 13744, 13745, 13746, 13747, 13748, 13749, 13750, 13751, 13753, 13762, 13763, 13764, 13765, 13766, 13767, 13768, 13769, 13770, 13771, 13772, 13773, 13775, 13776, 13777, 13778, 13779, 13780, 13781, 13782, 13783, 13784, 13785, 13787, 13788, 13790, 13791, 13793, 13794, 13796, 13797, 13798, 13801, 13802, 13803, 13804, 13805, 13806, 13807, 13808, 13809, 13811, 13814, 13842, 13843, 13844, 13845, 13846, 13847, 13848, 13849, 13854, 13855, 13859, 13861, 13862, 13894, 13922, 13923, 13924, 13925, 14082, 14083, 14084, 14085, 14092, 14105, 14118, 14363, 14382, 14383, 14384, 14385, 14386, 14387, 14388, 14389, 14390, 14391, 14392, 14393, 14394, 14475, 14524, 14527, 14532, 14533, 14534, 14535, 14550, 14575, 14597, 14609, 14618, 14642, 14643, 14691, 14696, 14706, 14707, 14818, 14820, 14822, 14823, 14824, 14836, 14837, 14845, 14870, 14871, 14873, 14874, 14875, 14876, 14877, 14878, 14879, 14880, 14881, 14882, 14883, 14884, 14885, 14886, 14887, 14888, 14889, 14890, 14891, 14892, 14893, 15446, 15460, 15586, 15688, 15780, 15888, 15889, 15910, 16024, 16025, 16026, 16027, 16028, 16029, 16030, 16031, 16033, 16034, 16035, 16036, 16037, 16038, 16061, 16062, 16063, 16064, 16065, 16066, 16067, 16068, 16069, 16070, 16071, 16073, 16074, 16075, 16076, 16077, 16078, 16079, 16080, 16081, 16082, 16085, 16086, 16102, 16103, 16104, 16105, 16106, 16107, 16108, 16109, 16116, 16117, 16118, 16119, 16120, 16121, 16122, 16123, 16124, 16125, 16126, 16127, 16129, 16131, 16132, 16134, 16135, 16136, 16137, 16138, 16139, 16140, 16141, 16142, 16143, 16144, 16145, 16146, 16147, 16148, 16149, 16150, 16151, 16152, 16153, 16154, 16155, 16156, 16157, 16158, 16159, 16160, 16161, 16162, 16163, 16164, 16172, 16173, 16174, 16175, 16176, 16177, 16178, 16179, 16180, 16181, 16182, 16183, 16184, 16185, 16186, 16187, 16188, 16191, 16211, 16212, 16213, 16308, 16321, 16322, 16323, 16324, 16325, 16338, 16339, 16343, 16344, 16582, 16664, 16792, 17000, 17040, 17041, 17108, 17115, 17116, 17122, 17123, 17162, 17163, 17199, 17282, 17283, 17342, 17343, 17347, 17354, 17382, 17383, 17409, 17412, 17462, 17463, 17482, 17731, 17769, 17783, 17802, 17824, 17825, 17826, 17827, 17828, 17829, 17830, 17831, 17832, 17833, 17834, 17835, 17836, 17837, 17838, 17839, 17840, 17841, 17842, 17843, 17844, 17845, 17846, 17847, 17848, 17851, 17852, 17853, 17854, 17855, 17856, 17857, 17858, 17859, 17860, 17861, 17862, 17882, 17883, 17884, 17885, 17886, 17887, 17888, 17889, 17890, 17891, 17892, 17893, 17894, 17895, 17896, 17897, 17898, 17899, 17910, 17911, 17942, 17967, 18023, 18062, 18063, 18105, 18106, 18122, 18123, 18153, 18155, 18156, 18157, 18158, 18159, 18161, 18162, 18163, 18164, 18165, 18166, 18167, 18209, 18235, 18293, 18303, 18304, 18316, 18320, 18341, 18342, 18355, 18419, 18438, 18589, 18593, 18595, 18596, 18599, 18627, 18630, 18644, 18666, 18667, 18668, 18669, 18685, 18747, 18763, 18764, 18765, 18768, 18800, 18801, 18881, 18882, 18942, 18963, 18964, 18965, 18966, 18967, 18968, 18971, 18982, 18983, 18985, 19014, 19015, 19053, 19054, 19055, 19065, 19082, 19122, 19158, 19184, 19185, 19186, 19187, 19188, 19189, 19190, 19191, 19192, 19193, 19194, 19195, 19196, 19197, 19198, 19199, 19200, 19201, 19214, 19226, 19286, 19313, 19314, 19404, 19427, 19428, 19455, 19456, 19457, 19482, 19485, 19486, 19487, 19488, 19489, 19490, 19502, 19503, 19504, 19622, 19623, 19662, 19742, 19743, 19762, 19763, 19804, 19809, 19810, 19811, 19837, 19844, 19847, 19868, 19916, 19917, 19924, 19926, 19932, 19966, 19980, 19981, 19983, 19986, 19987, 19988, 19989, 20003, 20005, 20020, 20024, 20026, 20084, 20135, 20136, 20137, 20138, 20139, 20140, 20141, 20142, 20143, 20144, 20145, 20146, 20149, 20221, 20238, 20239, 20240, 20241, 20242, 20245, 20246, 20247, 20248, 20249, 20250, 20251, 20252, 20267, 20268, 20269, 20270, 20271, 20272, 20273, 20274, 20275, 20276, 20277, 20278, 20279, 20280, 20281, 20282, 20283, 20284, 20285, 20286, 20287, 20288, 20289, 20290, 20291, 20292, 20297, 20298, 20299, 20300, 20301, 20302, 20303, 20304, 20305, 20306, 20307, 20308, 20309, 20311, 20312, 20313, 20314, 20315, 20316, 20317, 20318, 20319, 20320, 20321, 20322, 20323, 20324, 20325, 20326, 20327, 20328, 20329, 20330, 20331, 20332, 20333, 20334, 20335, 20336, 20337, 20338, 20339, 20340, 20341, 20342, 20343, 20344, 20345, 20346, 20347, 20348, 20349, 20350, 20351, 20352, 20353, 20354, 20355, 20356, 20357, 20358, 20359, 20360, 20361, 20362, 20363, 20364, 20367, 20368, 20370, 20372, 20412, 20417, 20423, 20445, 20446, 20460, 20468, 20473, 20485, 20489, 20502, 20522, 20524, 20525, 20583, 20584, 20585, 20586, 20587, 20588, 20589, 20590, 20591, 20592, 20593, 20594, 20595, 20596, 20597, 20598, 20609, 20651, 20718, 20719, 20737, 20738, 20739, 20740, 20762, 20773, 20774, 20775, 20776, 20777, 20778, 20779, 20780, 20781, 20782, 20783, 20784, 20785, 20786, 20787, 20788, 20789, 20790, 20791, 20792, 20793, 20794, 20795, 20796, 20798, 20814, 20819, 20822, 20825, 20829, 20834, 20883, 20887, 20902, 20903, 20904, 20908, 20913, 20936, 20937, 20946, 20952, 20953, 20962, 20965, 20972, 20977, 20979, 20984, 21026, 21034, 21035, 21036, 21043, 21044, 21045, 21046, 21047, 21048, 21049, 21050, 21051, 21052, 21053, 21054, 21055, 21056, 21057, 21058, 21059, 21060, 21061, 21062, 21063, 21064, 21065, 21066, 21067, 21068, 21069, 21070, 21073, 21074, 21075, 21076, 21077, 21078, 21079, 21080, 21081, 21082, 21083, 21084, 21085, 21086, 21087, 21088, 21089, 21090, 21091, 21092, 21093, 21094, 21095, 21096, 21097, 21098, 21101, 21102, 21121, 21122, 21123, 21124, 21125, 21127, 21129, 21135, 21141, 21152, 21159, 21163, 21168, 21173, 21192, 21193, 21194, 21195, 21236, 21238, 21240, 21246, 21247, 21274, 21276, 21286, 21313, 21339, 21369, 21419, 21420, 21421, 21422, 21423, 21424, 21425, 21426, 21427, 21428, 21429, 21430, 21431, 21432, 21433, 21434, 21435, 21437, 21439, 21440, 21441, 21442, 21443, 21444, 21445, 21446, 21447, 21448, 21449, 21450, 21451, 21465, 21516, 21518, 21549, 21550, 21551, 21553, 21554, 21555, 21560, 21564, 21572, 21573, 21575, 21577, 21578, 21580, 21584, 21591, 21594, 21612, 21613, 21614, 21628, 21629, 21630, 21631, 21632, 21633, 21634, 21636, 21637, 21638, 21641, 21642, 21643, 21644, 21646, 21649, 21653, 21655, 21656, 21657, 21658, 21659, 21660, 21661, 21662, 21717, 21719, 21720, 21736, 21739, 21772, 21773, 21782, 21785, 21786, 21794, 21795, 21796, 21811, 21857, 21879, 21880, 21883, 21923, 21924, 21930, 21950, 21951, 21958, 21959, 21962, 21963, 21964, 22020, 22021, 22022, 22023, 22024, 22025, 22026, 22027, 22028, 22029, 22030, 22031, 22032, 22033, 22034, 22035, 22036, 22037, 22038, 22039, 22040, 22041, 22042, 22043, 22045, 22130, 22151, 22152, 22180, 22199, 22210, 22211, 22213, 22215, 22230, 22233, 22258, 22273, 22316, 22341, 22346, 22386, 22387, 22391, 22415, 22474, 22475, 22485, 22486, 22546, 22564, 22569, 22581, 22582, 22584, 22585, 22586, 22587, 22588, 22596, 22619, 22625, 22626, 22684, 22685, 22686, 22687, 22692, 22694, 22695, 22696, 22697, 22698, 22703, 22704, 22705, 22709, 22724, 22738, 22765, 22780, 22781, 22782, 22805, 22814, 22817, 22898, 22899, 22928, 22929, 22931, 22933, 22989, 23026, 23034, 23052, 23058, 23074, 23076, 23086, 23157, 23158, 23159, 23163, 23164, 23172, 23174, 23175, 23176, 23193, 23212, 23213, 23216, 23222, 23223, 23224, 23225, 23227, 23229, 23230, 23231, 23232, 23236, 23240, 23241, 23271, 23328, 23330, 23335, 23349, 23350, 23351, 23356, 23357, 23360, 23362, 23363, 23365, 23368, 23369, 23374, 23378, 23382, 23388, 23416, 23418, 23420, 23421, 23422, 23428, 23432, 23433, 23434, 23443, 23450, 23457, 23463, 23470, 23471, 23472, 23481, 23567, 23582, 23583, 23616, 23647, 23648, 23649, 23650, 23651, 23652, 23653, 23655, 23656, 23673, 23674, 23684, 23689, 23696, 23698, 23699, 23700, 23701, 23704, 23708, 23710, 23712, 23715, 23718, 23719, 23721, 23722, 23725, 23727, 23728, 23740, 23741, 23743, 23754, 23794, 23795, 23796, 23812, 23813, 23853, 23856, 23861, 23872, 23882, 23885, 23889, 23895, 23904, 23905, 23906, 23907, 23908, 23911, 23912, 23914, 23916, 23917, 23918, 23940, 23941, 23942, 23943, 23950, 23951, 23952, 23953, 23954, 23955, 23957, 23958, 23959, 23960, 23961, 23962, 23966, 23967, 23968, 23969, 23970, 23971, 23972, 23973, 23974, 23975, 23980, 23982, 23983, 23990, 23992, 23993, 23996, 23998, 24005, 24010, 24011, 24012, 24013, 24014, 24015, 24016, 24017, 24018, 24019, 24034, 24038, 24068, 24071, 24100, 24115, 24137, 24147, 24148, 24149, 24187, 24191, 24227, 24228, 24229, 24244, 24265, 24315, 24319, 24320, 24321, 24322, 24324, 24325, 24326, 24327, 24328, 24329, 24331, 24332, 24333, 24358, 24369, 24409, 24410, 24418, 24420, 24443, 24491, 24495, 24506, 24509, 24512, 24515, 24518, 24524, 24525, 24526, 24527, 24528, 24529, 24530, 24531, 24532, 24533, 24534, 24535, 24536, 24537, 24541, 24548, 24561, 24562, 24563, 24564, 24565, 24566, 24567, 24568, 24569, 24570, 24571, 24572, 25145, 25159, 25173, 25285, 25407, 25493, 25497, 25520, 25546, 25547, 25551, 25571, 25572, 25582, 25587, 25588, 25596, 25625, 25626, 25646, 25663, 25664, 25665, 25667, 25688, 25698, 25747, 25748, 25749, 25750, 25752, 25753, 25758, 25793, 25794, 25795, 25796, 25797, 25798, 25799, 25800, 25801, 25816, 25818, 25839, 25859, 25860, 25871, 25877, 25879, 25888, 26003, 26015, 26029, 26041, 26046, 26047, 26056, 26057, 26058, 26059, 26060, 26061, 26062, 26063, 26064, 26065, 26066, 26067, 26068, 26069, 26070, 26071, 26072, 26073, 26074, 26075, 26076, 26077, 26078, 26079, 26080, 26081, 26082, 26083, 26084, 26085, 26086, 26087, 26088, 26089, 26090, 26091, 26092, 26093, 26094, 26095, 26096, 26097, 26098, 26099, 26100, 26101, 26102, 26103, 26104, 26105, 26106, 26107, 26108, 26109, 26110, 26111, 26112, 26113, 26114, 26115, 26116, 26117, 26118, 26119, 26120, 26121, 26122, 26123, 26124, 26125, 26126, 26127, 26136, 26137, 26138, 26139, 26140, 26141, 26142, 26143, 26144, 26145, 26146, 26147, 26148, 26149, 26150, 26151, 26152, 26153, 26154, 26155, 26156, 26157, 26158, 26159, 26160, 26161, 26162, 26163, 26164, 26165, 26166, 26167, 26168, 26169, 26170, 26171, 26172, 26173, 26174, 26175, 26176, 26177, 26178, 26179, 26180, 26181, 26182, 26183, 26184, 26185, 26186, 26187, 26188, 26189, 26190, 26191, 26192, 26193, 26194, 26195, 26196, 26197, 26198, 26199, 26200, 26201, 26202, 26203, 26204, 26205, 26206, 26207, 26208, 26209, 26210, 26211, 26212, 26213, 26214, 26215, 26216, 26217, 26218, 26219, 26220, 26221, 26222, 26223, 26224, 26225, 26226, 26227, 26228, 26229, 26230, 26231, 26232, 26233, 26234, 26235, 26236, 26237, 26238, 26239, 26240, 26241, 26242, 26243, 26244, 26245, 26246, 26247, 26248, 26249, 26250, 26251, 26252, 26253, 26254, 26255, 26256, 26257, 26258, 26259, 26260, 26261, 26262, 26263, 26264, 26265, 26266, 26267, 26268, 26269, 26270, 26271, 26272, 26273, 26274, 26275, 26276, 26277, 26278, 26279, 26280, 26281, 26282, 26283, 26284, 26285, 26286, 26287, 26288, 26289, 26290, 26291, 26292, 26293, 26294, 26295, 26296, 26297, 26298, 26299, 26300, 26301, 26302, 26303, 26304, 26305, 26306, 26307, 26308, 26309, 26310, 26311, 26312, 26313, 26314, 26315, 26316, 26317, 26318, 26319, 26320, 26321, 26322, 26323, 26324, 26325, 26326, 26327, 26328, 26329, 26330, 26331, 26332, 26333, 26334, 26335, 26336, 26337, 26338, 26339, 26340, 26341, 26342, 26343, 26344, 26345, 26346, 26347, 26348, 26349, 26350, 26351, 26352, 26353, 26354, 26355, 26356, 26357, 26358, 26359, 26360, 26361, 26362, 26363, 26364, 26365, 26366, 26367, 26368, 26369, 26370, 26371, 26372, 26373, 26374, 26375, 26376, 26377, 26378, 26379, 26380, 26381, 26382, 26383, 26384, 26385, 26386, 26387, 26388, 26389, 26390, 26391, 26392, 26393, 26394, 26395, 26396, 26397, 26398, 26399, 26400, 26401, 26402, 26403, 26404, 26405, 26406, 26407, 26408, 26409, 26410, 26411, 26412, 26413, 26414, 26415, 26416, 26417, 26418, 26419, 26420, 26421, 26422, 26423, 26424, 26425, 26426, 26427, 26428, 26429, 26430, 26431, 26432, 26433, 26434, 26435, 26436, 26437, 26438, 26439, 26440, 26441, 26442, 26443, 26444, 26445, 26446, 26447, 26448, 26449, 26450, 26451, 26452, 26453, 26454, 26455, 26456, 26457, 26458, 26459, 26460, 26461, 26462, 26463, 26464, 26465, 26466, 26467, 26468, 26469, 26470, 26471, 26472, 26473, 26474, 26475, 26476, 26477, 26478, 26479, 26480, 26481, 26482, 26483, 26484, 26485, 26486, 26487, 26488, 26489, 26490, 26491, 26492, 26493, 26494, 26495, 26496, 26497, 26498, 26499, 26500, 26501, 26502, 26503, 26504, 26505, 26506, 26507, 26508, 26509, 26510, 26511, 26512, 26514, 26515, 26516, 26517, 26518, 26519, 26520, 26521, 26522, 26523, 26524, 26525, 26526, 26528, 26529, 26530, 26531, 26532, 26533, 26534, 26535, 26536, 26537, 26538, 26539, 26540, 26542, 26543, 26544, 26545, 26546, 26547, 26548, 26549, 26550, 26551, 26552, 26553, 26554, 26555, 26556, 26557, 26558, 26559, 26560, 26561, 26562, 26563, 26564, 26565, 26566, 26567, 26568, 26570, 26571, 26572, 26573, 26574, 26575, 26576, 26577, 26578, 26579, 26580, 26581, 26582, 26583, 26584, 26585, 26586, 26587, 26588, 26589, 26590, 26591, 26592, 26593, 26594, 26595, 26596, 26597, 26598, 26599, 26600, 26601, 26602, 26603, 26604, 26605, 26606, 26607, 26608, 26609, 26610, 26611, 26612, 26613, 26614, 26615, 26616, 26617, 26618, 26619, 26620, 26621, 26622, 26623, 26624, 26625, 26626, 26627, 26628, 26629, 26630, 26631, 26632, 26633, 26634, 26635, 26636, 26637, 26638, 26639, 26640, 26641, 26642, 26643, 26644, 26645, 26646, 26647, 26648, 26649, 26650, 26651, 26652, 26653, 26654, 26655, 26656, 26657, 26658, 26659, 26660, 26661, 26662, 26663, 26664, 26665, 26666, 26667, 26668, 26669, 26670, 26671, 26672, 26673, 26674, 26675, 26676, 26677, 26678, 26679, 26680, 26681, 26682, 26683, 26684, 26685, 26686, 26687, 26688, 26689, 26690, 26691, 26692, 26693, 26694, 26695, 26696, 26697, 26698, 26699, 26700, 26701, 26702, 26703, 26704, 26705, 26706, 26707, 26708, 26709, 26710, 26711, 26712, 26713, 26714, 26715, 26716, 26717, 26718, 26719, 26720, 26721, 26722, 26723, 26724, 26725, 26726, 26727, 26728, 26729, 26730, 26731, 26732, 26733, 26734, 26735, 26736, 26737, 26738, 26739, 26740, 26741, 26742, 26743, 26744, 26745, 26746, 26747, 26748, 26749, 26750, 26751, 26752, 26753, 26754, 26755, 26756, 26757, 26758, 26759, 26760, 26761, 26762, 26763, 26764, 26766, 26767, 26768, 26769, 26770, 26771, 26772, 26773, 26774, 26775, 26776, 26777, 26778, 26780, 26781, 26782, 26783, 26784, 26785, 26786, 26787, 26788, 26789, 26790, 26791, 26792, 26793, 26794, 26795, 26796, 26797, 26798, 26799, 26800, 26801, 26802, 26803, 26804, 26805, 26806, 26807, 26808, 26809, 26810, 26811, 26812, 26813, 26814, 26815, 26816, 26817, 26818, 26819, 26820, 26821, 26822, 26823, 26824, 26825, 26826, 26827, 26828, 26829, 26830, 26831, 26832, 26833, 26834, 26835, 26836, 26837, 26838, 26839, 26840, 26841, 26842, 26843, 26844, 26845, 26846, 26847, 26848, 26849, 26850, 26851, 26852, 26853, 26854, 26855, 26856, 26857, 26858, 26859, 26860, 26861, 26862, 26863, 26864, 26865, 26866, 26867, 26868, 26869, 26870, 26871, 26872, 26873, 26874, 26875, 26876, 26877, 26878, 26879, 26880, 26881, 26882, 26883, 26884, 26885, 26886, 26887, 26888, 26889, 26890, 26891, 26892, 26893, 26894, 26895, 26896, 26897, 26898, 26899, 26900, 26901, 26902, 26903, 26904, 26905, 26906, 26907, 26908, 26909, 26910, 26911, 26912, 26913, 26914, 26915, 26916, 26917, 26918, 26919, 26920, 26921, 26922, 26923, 26924, 26925, 26926, 26927, 26928, 26929, 26930, 26931, 26932, 26933, 26934, 26935, 26936, 26937, 26938, 26939, 26940, 26941, 26942, 26943, 26944, 26945, 26946, 26947, 26948, 26949, 26950, 26951, 26952, 26953, 26954, 26955, 26956, 26957, 26958, 26959, 26960, 26961, 26962, 26963, 26964, 26965, 26966, 26967, 26968, 26969, 26970, 26971, 26972, 26973, 26974, 26975, 26976, 26977, 26978, 26979, 26980, 26981, 26982, 26983, 26984, 26985, 26986, 26987, 26988, 26989, 26990, 26991, 26992, 26993, 26994, 26995, 26996, 26997, 26998, 26999, 27000, 27001, 27002, 27003, 27004, 27005, 27006, 27007, 27008, 27009, 27010, 27011, 27012, 27013, 27014, 27015, 27016, 27017, 27018, 27019, 27020, 27021, 27022, 27023, 27024, 27025, 27026, 27027, 27028, 27029, 27030, 27031, 27032, 27033, 27034, 27035, 27036, 27037, 27038, 27039, 27040, 27041, 27042, 27043, 27044, 27045, 27046, 27047, 27048, 27049, 27050, 27051, 27052, 27053, 27054, 27055, 27056, 27057, 27058, 27059, 27060, 27061, 27062, 27063, 27064, 27065, 27066, 27067, 27068, 27069, 27070, 27071, 27072, 27073, 27074, 27075, 27076, 27077, 27078, 27079, 27080, 27081, 27082, 27083, 27084, 27085, 27086, 27087, 27088, 27089, 27090, 27091, 27092, 27093, 27094, 27095, 27096, 27097, 27098, 27099, 27100, 27101, 27102, 27103, 27104, 27105, 27106, 27107, 27108, 27109, 27110, 27111, 27112, 27113, 27114, 27115, 27116, 27117, 27118, 27119, 27120, 27121, 27122, 27123, 27124, 27125, 27126, 27127, 27128, 27129, 27130, 27131, 27132, 27133, 27134, 27135, 27136, 27137, 27138, 27139, 27140, 27141, 27142, 27143, 27144, 27145, 27146, 27147, 27148, 27149, 27150, 27151, 27152, 27153, 27154, 27155, 27156, 27157, 27158, 27159, 27160, 27161, 27162, 27163, 27164, 27165, 27166, 27167, 27168, 27169, 27170, 27171, 27172, 27173, 27174, 27175, 27176, 27177, 27178, 27179, 27180, 27181, 27182, 27183, 27184, 27185, 27186, 27187, 27188, 27189, 27190, 27191, 27192, 27193, 27194, 27195, 27196, 27197, 27198, 27199, 27200, 27201, 27202, 27203, 27204, 27205, 27206, 27207, 27208, 27209, 27210, 27211, 27212, 27213, 27214, 27215, 27216, 27217, 27218, 27219, 27220, 27221, 27222, 27223, 27224, 27225, 27226, 27227, 27228, 27229, 27230, 27231, 27232, 27233, 27234, 27235, 27236, 27237, 27238, 27239, 27240, 27241, 27242, 27243, 27244, 27245, 27246, 27247, 27248, 27249, 27250, 27251, 27252, 27253, 27254, 27255, 27256, 27257, 27258, 27259, 27260, 27261, 27262, 27263, 27264, 27265, 27266, 27267, 27268, 27269, 27270, 27271, 27272, 27273, 27274, 27275, 27276, 27277, 27278, 27279, 27280, 27281, 27282, 27283, 27284, 27285, 27286, 27287, 27288, 27289, 27290, 27291, 27292, 27293, 27294, 27295, 27296, 27297, 27298, 27299, 27300, 27301, 27302, 27303, 27304, 27305, 27306, 27307, 27308, 27309, 27310, 27311, 27312, 27313, 27314, 27315, 27316, 27318, 27319, 27320, 27321, 27322, 27323, 27324, 27325, 27326, 27327, 27328, 27329, 27330, 27331, 27332, 27333, 27334, 27335, 27336, 27337, 27338, 27339, 27340, 27341, 27342, 27343, 27344, 27345, 27346, 27347, 27348, 27349, 27350, 27351, 27352, 27353, 27354, 27355, 27356, 27357, 27358, 27359, 27360, 27361, 27362, 27363, 27364, 27365, 27366, 27367, 27368, 27369, 27370, 27371, 27372, 27373, 27374, 27375, 27376, 27377, 27378, 27379, 27380, 27381, 27382, 27383, 27384, 27385, 27386, 27387, 27391, 27392, 27393, 27394, 27395, 27396, 27397, 27405, 27406, 27407, 27419, 27421, 27444, 27486, 27496, 27530, 27554, 27555, 27556, 27557, 27558, 27559, 27560, 27561, 27562, 27563, 27564, 27565, 27566, 27567, 27568, 27569, 27570, 27571, 27572, 27573, 27574, 27575, 27576, 27577, 27578, 27579, 27580, 27581, 27582, 27583, 27584, 27585, 27586, 27587, 27588, 27589, 27590, 27591, 27592, 27593, 27594, 27595, 27596, 27597, 27598, 27599, 27600, 27601, 27602, 27603, 27604, 27605, 27606, 27607, 27608, 27609, 27610, 27611, 27612, 27613, 27614, 27615, 27616, 27617, 27618, 27619, 27620, 27621, 27622, 27623, 27624, 27625, 27626, 27627, 27628, 27629, 27630, 27701, 27819, 27836, 27849, 27850, 27851, 27852, 27853, 27862, 27863, 27894, 27923, 27950, 27951, 27952, 27953, 27954, 27955, 27956, 27957, 27958, 27959, 27960, 27961, 27962, 27963, 27964, 27965, 27966, 27967, 27968, 27969, 27970, 27971, 27972, 27973, 27974, 27975, 27997, 27998, 27999, 28000, 28001, 28002, 28003, 28004, 28005, 28006, 28007, 28008, 28009, 28010, 28011, 28012, 28013, 28014, 28015, 28016, 28017, 28018, 28019, 28020, 28021, 28022, 28023, 28025, 28037, 28049, 28067, 28076, 28077, 28078, 28079, 28080, 28081, 28082, 28083, 28084, 28085, 28086, 28087, 28088, 28089, 28090, 28091, 28092, 28093, 28094, 28095, 28096, 28097, 28098, 28113, 28114, 28115, 28125, 28133, 28135, 28145, 28165, 28195, 28196, 28197, 28198, 28199, 28200, 28201, 28208, 28261, 28289, 28354, 28365, 28366, 28456, 28471, 28482, 28487, 28488, 28489, 28546, 28549, 28648, 28650, 28676, 28678, 28736, 28737, 28738, 28739, 28798, 28816, 28905, 28906, 28914, 28916, 28965, 29025, 29041, 29052, 29107, 29114, 29120, 29188, 29208, 29210, 29225, 29237, 29293, 29310, 29402, 29403, 29404, 29405, 29406, 29407, 29408, 29409, 29410, 29413, 29414, 29415, 29416, 29417, 29418, 29419, 29420, 29421, 29422, 29423, 29424, 29427, 29430, 29431, 29432, 29433, 29435, 29436, 29437, 29438, 29439, 29440, 29441, 29442, 29444, 29446, 29455, 29462, 29479, 29484, 29537, 29538, 29541, 29542, 29543, 29544, 29551, 29552, 29557, 29558, 29565, 29566, 29585, 29619, 29620, 29621, 29622, 29623, 29626, 29627, 29628, 29629, 29630, 29631, 29632, 29633, 29634, 29635, 29636, 29637, 29638, 29639, 29640, 29641, 29642, 29643, 29644, 29645, 29646, 29647, 29648, 29649, 29650, 29651, 29652, 29653, 29654, 29655, 29656, 29657, 29658, 29659, 29660, 29661, 29662, 29663, 29665, 29666, 29667, 29668, 29670, 29671, 29676, 29678, 29679, 29680, 29681, 29683, 29685, 29686, 29687, 29688, 29690, 29692, 29694, 29695, 29696, 29697, 29705, 29706, 29707, 29708, 29709, 29710, 29711, 29712, 29715, 29716, 29748, 29802, 29809, 29816, 29819, 29820, 29821, 29823, 29824, 29825, 29826, 29827, 29828, 29829, 29830, 29831, 29832, 29833, 29834, 29835, 29836, 29837, 29838, 29839, 29840, 29841, 29842, 29843, 29844, 29845, 29846, 29847, 29848, 29849, 29850, 29851, 29852, 29853, 29854, 29855, 29856, 29857, 29858, 29859, 29860, 29861, 29862, 29863, 29864, 29865, 29866, 29867, 29868, 29869, 29870, 29871, 29872, 29873, 29874, 29875, 29876, 29877, 29878, 29879, 29880, 29881, 29882, 29883, 29884, 29885, 29886, 29887, 29888, 29889, 29890, 29891, 29892, 29893, 29894, 29895, 29896, 29897, 29898, 29899, 29900, 29907, 30078, 30128, 30147, 30176, 30178, 30179, 30180, 30181, 30182, 30191, 30193, 30195, 30197, 30198, 30199, 30204, 30208, 30209, 30261, 30287, 30288, 30289, 30292, 30310, 30325, 30347, 30367, 30376, 30385, 30387, 30388, 30389, 30390, 30391, 30392, 30393, 30403, 30405, 30406, 30407, 30408, 30409, 30410, 30411, 30412, 30414, 30423, 30424, 30439, 30440, 30441, 30445, 30452, 30455, 30456, 30469, 30470, 30471, 30472, 30473, 30474, 30482, 30484, 30485, 30491, 30502, 30528, 30545, 30557, 30576, 30577, 30578, 30580, 30595, 30624, 30625, 30636, 30647, 30648, 30660, 30661, 30662, 30669, 30670, 30671, 30697, 30698, 30699, 30702, 30711, 30714, 30715, 30718, 30760, 30790, 30793, 30795, 30796, 30801, 30802, 30806, 30848, 30877, 30920, 30921, 30934, 30935, 30949, 30954, 30963, 30965, 31081, 31082, 31083, 31087, 31167, 31205, 31206, 31207, 31208, 31253, 31257, 31259, 31265, 31266, 31267, 31273, 31274, 31296, 31301, 31302, 31309, 31311, 31325, 31327, 31348, 31352, 31353, 31389, 31466, 31467, 31468, 31469, 31496, 31497, 31498, 31499, 31500, 31502, 31503, 31505, 31506, 31507, 31551, 31600, 31601, 31603, 31604, 31605, 31608, 31609, 31611, 31612, 31654, 31665, 31667, 31669, 31743, 31767, 31801, 31802, 31803, 31805, 31806, 31842, 31843, 31844, 31845, 31846, 31847, 31848, 31849, 31850, 31851, 31930, 31931, 31932, 31933, 31934, 31947, 31948, 31954, 32065, 32066, 32075, 32091, 32093, 32094, 32095, 32096, 32097, 32098, 32099, 32100, 32101, 32102, 32103, 32104, 32105, 32106, 32107, 32108, 32109, 32110, 32111, 32112, 32113, 32114, 32115, 32116, 32117, 32118, 32119, 32120, 32121, 32122, 32123, 32124, 32125, 32126, 32127, 32128, 32129, 32130, 32131, 32132, 32133, 32134, 32135, 32136, 32137, 32138, 32139, 32140, 32141, 32142, 32143, 32144, 32145, 32146, 32147, 32148, 32149, 32150, 32151, 32152, 32153, 32154, 32155, 32156, 32157, 32158, 32159, 32160, 32161, 32162, 32163, 32164, 32165, 32166, 32167, 32168, 32169, 32170, 32171, 32172, 32173, 32174, 32175, 32176, 32177, 32178, 32179, 32180, 32181, 32182, 32183, 32184, 32185, 32186, 32187, 32188, 32189, 32190, 32191, 32192, 32246, 32272, 32322, 32360, 32371, 32372, 32384, 32407, 32414, 32415, 32416, 32417, 32418, 32419, 32421, 32422, 32424, 32425, 32426, 32448, 32457, 32460, 32463, 32465, 32466, 32477, 32482, 32484, 32498, 32499, 32504, 32507, 32530, 32543, 32545, 32546, 32547, 32548, 32549, 32550, 32551, 32552, 32553, 32554, 32555, 32556, 32557, 32558, 32559, 32560, 32561, 32562, 32565, 32594, 32595, 32603, 32604, 32605, 32607, 32610, 32611, 32612, 32613, 32614, 32615, 32618, 32632, 32633, 32642, 32644, 32699, 32729, 32730, 32731, 32740, 32743, 32761, 32762, 32763, 32764, 32765, 32766, 32767, 32775, 32824, 32826, 32827, 32841, 32856, 32874, 32875, 32876, 32877, 32878, 32879, 32884, 32885, 32886, 32887, 32889, 32890, 32891, 32892, 32893, 32894, 32908, 32914, 32916, 32921, 32922, 32923, 32924, 32925, 32926, 32927, 32928, 32929, 32930, 32931, 32932, 32933, 32934, 32935, 32936, 32937, 32938, 32939, 32940, 32949, 32950, 32951, 32952, 32953, 32954, 32955, 32956, 32957, 32958, 32959, 32965, 32966, 32967, 32970, 32973, 32974, 32975, 32976, 32977, 32978, 32982, 32983, 32984, 32985, 32986, 32987, 32991, 32992, 32993, 32994, 32995, 32996, 33001, 33002, 33003, 33005, 33022, 33027, 33046, 33049, 33073, 33074, 33075, 33080, 33089, 33094, 33097, 33100, 33104, 33116, 33118, 33121, 33125, 33161, 33162, 33168, 33169, 33170, 33171, 33172, 33177, 33178, 33180, 33181, 33193, 33194, 33195, 33196, 33197, 33198, 33199, 33200, 33201, 33210, 33212, 33213, 33217, 33220, 33227, 33275, 33276, 33288, 33294, 33295, 33301, 33302, 33315, 33316, 33318, 33319, 33320, 33338, 33350, 33442, 33456, 33482, 33521, 33525, 33526, 33542, 33543, 33544, 33564, 33565, 33570, 33572, 33573, 33574, 33594, 33595, 33596, 33597, 33598, 33600, 33601, 33602, 33603, 33604, 33608, 33609, 33610, 33623, 33624, 33625, 33626, 33633, 33772, 33773, 33775, 33776, 33777, 33785, 33786, 33787, 33788, 33789, 33790, 33793, 33795, 33798, 33802, 33807, 33957, 33958, 33959, 33963, 33964, 33975, 33979, 33980, 33981, 33982, 33983, 33984, 33986, 33987, 33988, 33989, 33990, 33991, 33992, 33994, 33995, 33996, 33997, 33998, 34004, 34005, 34006, 34007, 34025, 34030, 34034, 34036, 34037, 34038, 34039, 34044, 34045, 34046, 34047, 34048, 34058, 34071, 34079, 34080, 34093, 34094, 34095, 34096, 34097, 34098, 34107, 34108, 34126, 34138, 34139, 34144, 34145, 34146, 34147, 34148, 34149, 34150, 34151, 34152, 34153, 34154, 34155, 34156, 34158, 34159, 34161, 34187, 34217, 34219, 34250, 34251, 34252, 34260, 34263, 34264, 34265, 34266, 34267, 34268, 34269, 34270, 34271, 34272, 34273, 34274, 34275, 34276, 34277, 34278, 34279, 34280, 34281, 34282, 34283, 34284, 34285, 34286, 34287, 34288, 34289, 34290, 34291, 34292, 34293, 34294, 34295, 34296, 34297, 34298, 34299, 34300, 34301, 34302, 34303, 34304, 34305, 34306, 34307, 34308, 34309, 34310, 34311, 34312, 34313, 34314, 34315, 34316, 34317, 34318, 34320, 34321, 34322, 34323, 34324, 34325, 34326, 34327, 34328, 34415, 34449, 34450, 34451, 34452, 34453, 34454, 34455, 34456, 34457, 34458, 34459, 34460, 34461, 34462, 34463, 34465, 34466, 34467, 34481, 34496, 34503, 34505, 34506, 34507, 34508, 34509, 34510, 34511, 34512, 34513, 34514, 34515, 34516, 34517, 34518, 34519, 34520, 34521, 34522, 34523, 34524, 34525, 34526, 34531, 34532, 34534, 34536, 34550, 34551, 34552, 34553, 34586, 34588, 34589, 34590, 34591, 34596, 34626, 34637, 34638, 34639, 34640, 34642, 34643, 34644, 34645, 34646, 34647, 34660, 34661, 34662, 34663, 34668, 34681, 34682, 34687, 34693, 34694, 34696, 34712, 34716, 34717, 34718, 34723, 34724, 34725, 34726, 34727, 34728, 34729, 34730, 34731, 34732, 34733, 34734, 34735, 34737, 34738, 34739, 34740, 34741, 34742, 34743, 34744, 34745, 34746, 34770, 34771, 34776, 34784, 34803, 34805, 34816, 34817, 34818, 34819, 34820, 34821, 34873, 34874, 34875, 34876, 34877, 34878, 34879, 34880, 34881, 34882, 34883, 34884, 34885, 34886, 34899, 34965, 34966, 34967, 34969, 34970, 35117, 35118, 35120, 35124, 35220, 35235, 35236, 35312, 35396, 35397, 35398, 35399, 35400, 35417, 35418, 35419, 35420, 35421, 35422, 35423, 35424, 35425, 35426, 35427, 35428, 35429, 35430, 35431, 35432, 35433, 35434, 35435, 35436, 35437, 35438, 35439, 35440, 35441, 35442, 35443, 35444, 35445, 35446, 35447, 35448, 35449, 35450, 35451, 35452, 35453, 35454, 35455, 35456, 35457, 35458, 35459, 35460, 35461, 35462, 35463, 35480, 35482, 35485, 35499, 35510, 35515, 35517, 35518, 35519, 35520, 35521, 35522, 35523, 35524, 35525, 35526, 35527, 35528, 35529, 35530, 35531, 35532, 35533, 35534, 35535, 35536, 35537, 35538, 35539, 35540, 35541, 35542, 35544, 35545, 35546, 35548, 35549, 35550, 35551, 35552, 35553, 35554, 35555, 35556, 35558, 35561, 35567, 35621, 35626, 35667, 35674, 35684, 35689, 35710, 35712, 35713, 35714, 35715, 35719, 35721, 35722, 35724, 35727, 35732, 35735, 35740, 35741, 35742, 35743, 35744, 35745, 35757, 35775, 35776, 35777, 35778, 35779, 35781, 35787, 35789, 35804, 35805, 35840, 35853, 35854, 35855, 35874, 35876, 35939, 35942, 36454, 36477, 36491, 36505, 36519, 36533, 36561, 36575, 36603, 36617, 36631, 36645, 36659, 36673, 36687, 36701, 36715, 36745, 36749, 36750, 36755, 36761, 36762, 36763, 36773, 36778, 36790, 36791, 36792, 36795, 36799, 36830, 36831, 36837, 36838, 36839, 36840, 36841, 36842, 36843, 36844, 36845, 36866, 36867, 36869, 36896, 36897, 36898, 36899, 36900, 36911, 36914, 36915, 36955, 36959, 36960, 36963, 36964, 36965, 36966, 36967, 36968, 36970, 36987, 36990, 37001, 37059, 37089, 37090, 37100, 37102, 37103, 37119, 37120, 37123, 37126, 37130, 37131, 37132, 37133, 37146, 37154, 37157, 37158, 37161, 37174, 37175, 37176, 37210, 37225, 37244, 37245, 37249, 37266, 37273, 37278, 37279, 37281, 37284, 37285, 37286, 37295, 37296, 37298, 37301, 37308, 37309, 37310, 37311, 37312, 37313, 37315, 37316, 37317, 37318, 37321, 37323, 37324, 37326, 37327, 37328, 37329, 37335, 37336, 37337, 37338, 37341, 37342, 37343, 37345, 37346, 37348, 37385, 37386, 37420, 37433, 37444, 37448, 37450, 37451, 37453, 37454, 37455, 37457, 37466, 37468, 37469, 37470, 37472, 37473, 37474, 37477, 37485, 37510, 37511, 37531, 37532, 37533, 37534, 37536, 37544, 37549, 37550, 37551, 37561, 37563, 37578, 37579, 37589, 37596, 37598, 37600, 37605, 37608, 37609, 37610, 37611, 37649, 37697, 37698, 37699, 37706, 37738, 37739, 37740, 37827, 37832, 37837, 37838, 37839, 37878, 37914, 37916, 37917, 37919, 37924, 37926, 37946, 37951, 37952, 37955, 37967, 37972, 37973, 37974, 37975, 37976, 37977, 37978, 37979, 37994, 37997, 38009, 38015, 38016, 38017, 38018, 38019, 38020, 38021, 38022, 38038, 38052, 38058, 38059, 38060, 38061, 38062, 38063, 38064, 38065, 38066, 38067, 38069, 38074, 38075, 38076, 38077, 38078, 38079, 38099, 38113, 38119, 38120, 38121, 38122, 38123, 38124, 38125, 38136, 38139, 38140, 38158, 38159, 38164, 38166, 38167, 38168, 38179, 38180, 38182, 38183, 38184, 38185, 38192, 38193, 38199, 38200, 38201, 38203, 38204, 38205, 38209, 38210, 38211, 38215, 38216, 38235, 38236, 38243, 38244, 38245, 38246, 38247, 38248, 38249, 38254, 38255, 38256, 38265, 38270, 38271, 38272, 38282, 38296, 38297, 38298, 38304, 38307, 38315, 38316, 38317, 38331, 38377, 38378, 38383, 38385, 38387, 38388, 38389, 38390, 38391, 38392, 38394, 38395, 38442, 38443, 38444, 38445, 38446, 38447, 38448, 38449, 38450, 38451, 38468, 38469, 38470, 38471, 38472, 38474, 38475, 38476, 38478, 38479, 38480, 38481, 38482, 38484, 38485, 38486, 38487, 38488, 38489, 38490, 38491, 38492, 38493, 38494, 38495, 38496, 38497, 38499, 38500, 38501, 38502, 38503, 38507, 38508, 38509, 38511, 38524, 38525, 38526, 38527, 38539, 38554, 38565, 38567, 38568, 38569, 38570, 38571, 38572, 38580, 38593, 38594, 38595, 38596, 38597, 38598, 38599, 38602, 38603, 38604, 38606, 38608, 38609, 38625, 38630, 38635, 38640, 38641, 38643, 38652, 38654, 38683, 38685, 38690, 38691, 38692, 38693, 38694, 38702, 38704, 38720, 38721, 38722, 38723, 38724, 38725, 38738, 38740, 38742, 38916, 38952, 38957, 38958, 38970, 38983, 38994, 38996, 39007, 39008, 39009, 39012, 39014, 39024, 39032, 39037, 39039, 39042, 39061, 39067, 39069, 39071, 39101, 39103, 39106, 39107, 39111, 39122, 39123, 39126, 39137, 39145, 39147, 39148, 39149, 39153, 39155, 39163, 39214, 39263, 39287, 39288, 39289, 39290, 39300, 39302, 39303, 39304, 39312, 39325, 39352, 39359, 39364, 39370, 39381, 39382, 39384, 39410, 39440, 39447, 39456, 39460, 39511, 39526, 39527, 39584, 39585, 39586, 39587, 39600, 39644, 39658, 39659, 39660, 39661, 39662, 39663, 39677, 39687, 39692, 39699, 39705, 39707, 39708, 39709, 39710, 39711, 39715, 39741, 39742, 39743, 39744, 39745, 39746, 39749, 39750, 39751, 39752, 39753, 39754, 39813, 39819, 39828, 39832, 39879, 39884, 39885, 39892, 39903, 39904, 39913, 39921, 39922, 39923, 39924, 39925, 39926, 39928, 39929, 39930, 39931, 39971, 39972, 40183, 40218, 40219, 40220, 40221, 40222, 40223, 40224, 40225, 40226, 40227, 40228, 40229, 40230, 40231, 40276, 40307, 40308, 40309, 40310, 40311, 40312, 40313, 40314, 40355, 40389, 40413, 40434, 40435, 40436, 40440, 40441, 40442, 40443, 40444, 40452, 40464, 40479, 40480, 40481, 40484, 40485, 40487, 40501, 40534, 40535, 40537, 40538, 40540, 40542, 40553, 40582, 40595, 40596, 40597, 40598, 40599, 40605, 40606, 40607, 40608, 40609, 40644, 40646, 40647, 40648, 40649, 40650, 40651, 40654, 40655, 40656, 40657, 40658, 40659, 40660, 40661, 40662, 40663, 40664, 40665, 40677, 40725, 40727, 40729, 40754, 40759, 40760, 40761, 40762, 40763, 40764, 40765, 40766, 40770, 40774, 40776, 40777, 40796, 40800, 40832, 40839, 40843, 40894, 40945, 40948, 40967, 40968, 41022, 41090, 41091, 41093, 41111, 41118, 41125, 41147, 41175, 41176, 41177, 41178, 41180, 41244, 41247, 41256, 41259, 41261, 41263, 41342, 41343, 41358, 41360, 41364, 41365, 41370, 41371, 41374, 41403, 41404, 41405, 41406, 41407, 41408, 41409, 41410, 41411, 41412, 41413, 41414, 41415, 41416, 41417, 41418, 41419, 41420, 41421, 41422, 41423, 41425, 41583, 41585, 41605, 41606, 41613, 41691, 41695, 41741, 41748, 41749, 41756, 41757, 41758, 41759, 41764, 41804, 41811, 41823, 41889, 41977, 41978, 41979, 41980, 41981, 41982, 41983, 42139, 42140, 42147, 42161, 42165, 42166, 42167, 42168, 42169, 42174, 42179, 42180, 42181, 42182, 42186, 42189, 42190, 42191, 42192, 42193, 42194, 42195, 42196, 42197, 42198, 42199, 42200, 42201, 42202, 42205, 42206, 42207, 42212, 42213, 42214, 42215, 42216, 42217, 42218, 42219, 42220, 42221, 42222, 42223, 42224, 42226, 42231, 42236, 42238, 42239, 42240, 42241, 42247, 42254, 42259, 42264, 42269, 42274, 42279, 42284, 42289, 42294, 42295, 42296, 42297, 42316, 42321, 42326, 42331, 42343, 42344, 42345, 42351, 42379, 42380, 42381, 42382, 42383, 42388, 42389, 42426, 42427, 42444, 42445, 42446, 42447, 42448, 42449, 42477, 42478, 42484, 42489, 42494, 42501, 42506, 42507, 42509, 42511, 42512, 42517, 42518, 42523, 42524, 42529, 42530, 42535, 42536, 42543, 42544, 42547, 42548, 42556, 42557, 42558, 42563, 42568, 42569, 42574, 42575, 42576, 42577, 42582, 42587, 42590, 42593, 42594, 42595, 42596, 42601, 42606, 42611, 42612, 42613, 42618, 42619, 42625, 42626, 42627, 42628, 42629, 42630, 42631, 42632, 42633, 42634, 42635, 42636, 42637, 42638, 42639, 42654, 42655, 42656, 42657, 42658, 42659, 42660, 42661, 42662, 42663, 42664, 42665, 42666, 42667, 42668, 42669, 42670, 42671, 42672, 42673, 42674, 42675, 42676, 42677, 42678, 42680, 42681, 42682, 42683, 42684, 42685, 42686, 42687, 42688, 42689, 42690, 42691, 42692, 42693, 42694, 42695, 42696, 42697, 42698, 42699, 42703, 42704, 42705, 42706, 42707, 42708, 42709, 42710, 42711, 42712, 42713, 42714, 42715, 42716, 42717, 42718, 42719, 42720, 42721, 42722, 42755, 42756, 42757, 42759, 42764, 42773, 42775, 42776, 42851, 42856, 42873, 42875, 42885, 42886, 42919, 42920, 42921, 42923, 42924, 42925, 42929, 42932, 42933, 42934, 42935, 42936, 42937, 42938, 42939, 42940, 42941, 42953, 42975, 42976, 42977, 42978, 42979, 42980, 42981, 42982, 42983, 43002, 43003, 43006, 43008, 43014, 43038, 43091, 43092, 43093, 43097, 43098, 43110, 43111, 43112, 43113, 43114, 43150, 43152, 43170, 43175, 43196, 43199, 43205, 43216, 43219, 43220, 43221, 43222, 43223, 43224, 43226, 43227, 43254, 43267, 43292, 43293, 43294, 43295, 43296, 43301, 43302, 43303, 43304, 43336, 43337, 43362, 43383, 43384, 43460, 43468, 43471, 43474, 43475, 43476, 43477, 43479, 43487, 43503, 43514, 43525, 43532, 43540, 43557, 43558, 43559, 43560, 43561, 43562, 43563, 43578, 43579, 43580, 43581, 43596, 43598, 43602, 43603, 43604, 43605, 43606, 43607, 43614, 43617, 43618, 43619, 43620, 43621, 43623, 43625, 43648, 43649, 43651, 43692, 43700, 43727, 43728, 43729, 43730, 43731, 43732, 43733, 43734, 43735, 43736, 43737, 43738, 43739, 43740, 43741, 43742, 43743, 43744, 43745, 43746, 43747, 43748, 43749, 43750, 43751, 43752, 43753, 43754, 43755, 43756, 43757, 43758, 43759, 43760, 43761, 43762, 43763, 43764, 43765, 43766, 43767, 43768, 43769, 43770, 43771, 43772, 43773, 43774, 43775, 43776, 43777, 43778, 43779, 43780, 43781, 43782, 43783, 43784, 43785, 43786, 43787, 43788, 43789, 43790, 43791, 43792, 43793, 43794, 43795, 43796, 43797, 43798, 43799, 43800, 43801, 43802, 43803, 43804, 43805, 43806, 43807, 43808, 43809, 43810, 43811, 43812, 43813, 43814, 43815, 43816, 43817, 43818, 43819, 43820, 43822, 43848, 43878, 43886, 43887, 43895, 43901, 43902, 43922, 43936, 43938, 43963, 43964, 43967, 43985, 43999, 44056, 44090, 44119, 44124, 44125, 44126, 44162, 44169, 44172, 44191, 44227, 44232, 44233, 44236, 44237, 44238, 44252, 44298, 44299, 44300, 44333, 44389, 44390, 44391, 44392, 44414, 44416, 44418, 44428, 44432, 44439, 44451, 44454, 44555, 44556, 44557, 44578, 44580, 44598, 44600, 44603, 44604, 44620, 44646, 44656, 44699, 44705, 44706, 44715, 44720, 44726, 44727, 44728, 44730, 44733, 44754, 44755, 44760, 44761, 44804, 44805, 44807, 44811, 44821, 44832, 44833, 44845, 44846, 44847, 44848, 44851, 44856, 44857, 44866, 44867, 44869, 44870, 44871, 44872, 44873, 44874, 44875, 44876, 44877, 44878, 44879, 44880, 44881, 44882, 44883, 44884, 44915, 44924, 44926, 44945, 44948, 44952, 44964, 44972, 44988, 44989, 44991, 44992, 44993, 44994, 44996, 44997, 45006, 45007, 45008, 45009, 45010, 45024, 45026, 45028, 45029, 45030, 45031, 45032, 45033, 45034, 45035, 45036, 45041, 45048, 45053, 45055, 45081, 45084, 45120, 45123, 45124, 45126, 45172, 45173, 45174, 45175, 45229, 45230, 45231, 45276, 45277, 45279, 45280, 45290, 45350, 45499, 45528, 45569, 45575, 45629, 45630, 45692, 45705, 45726, 45727, 45765, 45856, 45860, 45863, 45884, 45899, 45900, 45901, 45908, 45918, 45924, 45925, 45926, 45942, 45985, 46020, 46054, 46055, 46089, 46090, 46092, 46093, 46101, 46103, 46104, 46105, 46107, 46112, 46213, 46214, 46215, 46216, 46217, 46218, 46219, 46220, 46221, 46222, 46223, 46224, 46225, 46226, 46227, 46228, 46230, 46231, 46232, 46233, 46234, 46235, 46236, 46237, 46238, 46239, 46240, 46241, 46242, 46243, 46244, 46245, 46246, 46247, 46248, 46249, 46250, 46251, 46252, 46253, 46254, 46255, 46256, 46257, 46258, 46259, 46260, 46261, 46262, 46263, 46264, 46265, 46266, 46267, 46268, 46269, 46270, 46271, 46272, 46273, 46274, 46275, 46276, 46277, 46278, 46279, 46280, 46281, 46282, 46283, 46284, 46285, 46286, 46287, 46288, 46289, 46290, 46291, 46292, 46293, 46294, 46295, 46296, 46297, 46298, 46299, 46300, 46301, 46302, 46303, 46304, 46305, 46306, 46307, 46309, 46319, 46326, 46327, 46328, 46329, 46330, 46331, 46332, 46333, 46334, 46335, 46336, 46358, 46371, 46393, 46394, 46395, 46709, 46733, 46736, 46737, 46738, 46778, 46837, 46839, 46840, 46841, 46842, 46844, 46845, 46852, 46854, 46886, 46892, 46894, 46957, 46981, 46982, 46983, 46984, 46987, 46998, 47005, 47013, 47014, 47031, 47032, 47034, 47102, 47103, 47505, 47507, 47542, 47543, 47544, 47842, 47844, 47846, 48527, 48601, 48786, 48945, 48947, 48949, 49016, 49018, 49020, 49022, 49024, 49070, 49072, 49148, 49152, 49154, 49156, 49158, 49160, 49198, 49206, 49223, 49224, 49225, 49291, 49292, 49293, 49311, 49312, 49313, 49314, 49334, 49340, 49341, 49342, 49344, 49345, 49346, 49349, 49353, 49357, 49358, 49363, 49372, 49373, 49374, 49377, 49637, 49638, 49645, 49653, 49654, 49664, 49666, 49680, 49681, 49684, 49686, 49687, 49689, 49691, 49692, 49708, 49709, 49713, 49714, 49716, 49717, 49719, 49720, 49721, 49722, 49724, 49725, 49726, 49727, 49728, 49729, 49730, 49731, 49732, 49733, 49734, 49735, 49736, 49737, 49738, 49761, 49762, 49763, 49764, 49767, 49768, 49773, 49774, 49775, 49777, 49814, 49864, 49865, 49868, 49873, 49886, 49913, 49917, 49918, 49922, 49923, 49924, 49925, 49926, 49931, 49933, 49935, 49984, 50039, 50043, 50076, 50091, 50092, 50093, 50129, 50132, 50133, 50164, 50204, 50216, 50217, 50221, 50224, 50225, 50248, 50249, 50251, 50252, 50256, 50257, 50301, 50329, 50330, 50331, 50332, 50419, 50422, 50431, 50433, 50434, 50435, 50442, 50752, 50753, 50757, 50758, 50814, 50815, 50817, 50840, 51028, 51029, 51322, 51323, 51324, 51949, 51986, 51988, 51997, 51998, 52007, 52008, 52009, 52010, 52011, 52012, 52015, 52016, 52034, 52037, 52042, 52058, 52062, 52189, 52202, 52272, 52274, 52275, 52276, 52344, 52345, 52358, 52359, 52361, 52562, 52563, 52565, 52567, 52686, 52706, 52707, 52713, 52729, 52835, 53048, 53055, 53056, 53096, 53476, 53491, 53492, 53493, 53494, 53495, 53496, 53497, 53498, 53499, 53500, 53501, 53502, 53503, 53504, 53505, 53506, 53507, 53508, 53509, 53785, 53835, 53889, 53890, 53891, 53924, 53933, 53934, 53935, 53936, 53937, 53938, 53963, 54068, 54452, 54455, 54467, 54468, 54555, 54592, 54612, 54810, 54822, 54848, 54857 };
#endif

        for (uint8 clazz = CLASS_WARRIOR; clazz < MAX_CLASSES; ++clazz)
        {
            // skip nonexistent classes
            if (!((1 << (clazz - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(clazz))
                continue;

            for (uint32 level = 1; level <= maxLevel; ++level)
            {
                for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    for (uint32 spec = 1; spec <= MAX_STAT_SCALES; ++spec)
                    {
                        if (!m_weightScales[spec].info.id)
                            continue;

                        if (m_weightScales[spec].info.classId != clazz)
                            continue;

                        for (uint32 quality = ITEM_QUALITY_POOR; quality <= ITEM_QUALITY_ARTIFACT; ++quality)
                        {
                            BotEquipKey key(level, clazz, spec, slot, quality);

                            RandomItemList items;
                            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
                            {
                                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                                if (!proto)
                                    continue;

                                if (proto->Quality != key.quality)
                                    continue;

                                // skip not available items
                                // TO DO: Replace this with a db query
                                if (unavailableItemIDs.find(itemId) != unavailableItemIDs.end())
                                    continue;

                                if ((slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD))
                                {
                                    std::set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
                                    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
                                        continue;

                                    if (slot == EQUIPMENT_SLOT_BODY && std::find(shirtsList.begin(), shirtsList.end(), itemId) == shirtsList.end())
                                        shirtsList.push_back(itemId);
                                    if (slot == EQUIPMENT_SLOT_TABARD && std::find(tabardsList.begin(), tabardsList.end(), itemId) == tabardsList.end())
                                        tabardsList.push_back(itemId);

                                    CharacterDatabase.PExecute("replace into ai_playerbot_equip_cache (id, clazz, spec, lvl, slot, quality, item) values (%u, %u, %u, %u, %u, %u, %u)",
                                        2000000 + itemId, 1, 1, 60, slot, 1, itemId);

                                    continue;
                                }

                                // check stat weight
                                uint32 statWeight = GetStatWeight(itemId, spec);
                                if (statWeight <= 0)
                                    continue;

                                // only accept "useless" items if bot level <= 30
                                if (statWeight == 1 && level > 30 && !proto->RandomProperty)
                                    continue;

                                uint32 minLevel = GetMinLevelFromCache(itemId);
                                // skip higher level (e.g. quest rewards)
                                if (minLevel > level)
                                    continue;

                                if (abs((int)minLevel - (int)level) > 20)
                                    continue;

                                /*if (proto->Class == ITEM_CLASS_WEAPON && abs((int)minLevel - (int)level) > 10)
                                    continue;*/

                                if (proto->Class != ITEM_CLASS_WEAPON &&
                                    proto->Class != ITEM_CLASS_ARMOR &&
                                    proto->Class != ITEM_CLASS_CONTAINER &&
                                    proto->Class != ITEM_CLASS_PROJECTILE)
                                    continue;

                                if (!CanEquipItem(key, proto))
                                    continue;

                                if (proto->Class == ITEM_CLASS_ARMOR && (
                                    slot == EQUIPMENT_SLOT_HEAD ||
                                    slot == EQUIPMENT_SLOT_SHOULDERS ||
                                    slot == EQUIPMENT_SLOT_CHEST ||
                                    slot == EQUIPMENT_SLOT_WAIST ||
                                    slot == EQUIPMENT_SLOT_LEGS ||
                                    slot == EQUIPMENT_SLOT_FEET ||
                                    slot == EQUIPMENT_SLOT_WRISTS ||
                                    slot == EQUIPMENT_SLOT_HANDS) && !CanEquipArmor(key.clazz, key.spec, key.level, proto))
                                    continue;

                                //if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(key.clazz, proto))
                                //    continue;

                                if (slot == EQUIPMENT_SLOT_OFFHAND && key.clazz == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
                                    continue;

                                items.push_back(itemId);

                                CharacterDatabase.PExecute("insert into ai_playerbot_equip_cache (clazz, spec, lvl, slot, quality, item) values (%u, %u, %u, %u, %u, %u)",
                                    clazz, spec, level, slot, quality, itemId);
                            }

                            equipCache[key] = items;
                            bar.step();
                            sLog.outDetail("Equipment cache for class: %d, level %d, slot %d, quality %d: %zu items",
                                clazz, level, slot, quality, items.size());
                        }
                    }
                }
            }
        }
        equipCache[tabardKey] = tabardsList;
        equipCache[shirtKey] = shirtsList;
        sLog.outString("Equipment cache saved to DB");
    }
}

RandomItemList RandomItemMgr::Query(uint32 level, uint8 clazz, uint8 spec, uint8 slot, uint32 quality)
{
    BotEquipKey key(level, clazz, spec, slot, quality);
    return equipCache[key];
}

void RandomItemMgr::BuildAmmoCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building ammo cache for %d levels", maxLevel);
	int counter1 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        for (uint32 subClass = ITEM_SUBCLASS_ARROW; subClass <= ITEM_SUBCLASS_BULLET; subClass++)
        {
            auto results = WorldDatabase.PQuery(
                    "select entry, RequiredLevel from item_template where class = '%u' and subclass = '%u' and RequiredLevel <= '%u' and quality = '%u' order by RequiredLevel desc",
                    ITEM_CLASS_PROJECTILE, subClass, level, ITEM_QUALITY_NORMAL);
            if (!results)
                return;

            Field* fields = results->Fetch();
            if (fields)
            {
                uint32 entry = fields[0].GetUInt32();
                ammoCache[level / 10][subClass] = entry;
				counter1++;
            }
        }

        auto results = WorldDatabase.PQuery(
            "select entry, RequiredLevel from item_template where class = '%u' and subclass = '%u' and RequiredLevel <= '%u' and quality = '%u' order by RequiredLevel desc",
            ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_THROWN, level, ITEM_QUALITY_NORMAL);
        if (!results)
            return;

        Field* fields = results->Fetch();
        if (fields)
        {
            uint32 entry = fields[0].GetUInt32();
            ammoCache[level / 10][ITEM_SUBCLASS_WEAPON_THROWN] = entry;
            counter1++;
        }
    }
	sLog.outString("Cached %d types of ammo", counter1); // TEST
}

uint32 RandomItemMgr::GetAmmo(uint32 level, uint32 subClass)
{
    return ammoCache[(level - 1) / 10][subClass];
}


void RandomItemMgr::BuildPotionCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building potion cache for %d levels", maxLevel);
	int counter2 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (int i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];

            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK) ||
                    proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        continue;

                    for (int i = 0 ; i < 3; i++)
                    {
                        if (spellInfo->Effect[i] == effect)
                        {
                            potionCache[level / 10][effect].push_back(itemId);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (int i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];
            uint32 size = potionCache[level / 10][effect].size();
			counter2++;
            sLog.outDetail("Potion cache for level=%d, effect=%d: %d items", level, effect, size);
        }
    }
	sLog.outString("Cached %d types of potions", counter2); // TEST
}

void RandomItemMgr::BuildFoodCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building food cache for %d levels", maxLevel);
	int counter3 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 categories[] = { 11, 59 };
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];

            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_FOOD && proto->SubClass != ITEM_SUBCLASS_CONSUMABLE) ||
                    (proto->Spells[0].SpellCategory != category) ||
                    proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                foodCache[level / 10][category].push_back(itemId);
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 categories[] = { 11, 59 };
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];
            uint32 size = foodCache[level / 10][category].size();
			counter3++;
            sLog.outDetail("Food cache for level=%d, category=%d: %d items", level, category, size);
        }
    }
	sLog.outString("Cached %d types of food", counter3);
}

uint32 RandomItemMgr::GetRandomPotion(uint32 level, uint32 effect)
{
    std::vector<uint32> potions = potionCache[(level - 1) / 10][effect];
    if (potions.empty()) return 0;
    return potions[urand(0, potions.size() - 1)];
}

uint32 RandomItemMgr::GetFood(uint32 level, uint32 category)
{
    std::vector<uint32> items;
    if (category == 11)
    {
        if (level < 5)
            items = { 787, 117, 4540, 2680 };
        else if (level < 15)
            items = { 2287, 4592, 4541, 21072 };
        else if (level < 25)
            items = { 3770, 16170, 4542, 20074 };
        else if (level < 35)
            items = { 4594, 3771, 1707, 4457 };
        else if (level < 45)
            items = { 4599, 4601, 21552, 17222 /*21030, 16168 */ };
#ifdef MANGOSBOT_ZERO
        else
#else
        else if (level < 55)
#endif
            items = { 8950, 8952, 8957, 21023 /*21033, 21031 */ };
#ifdef MANGOSBOT_ONE
        else if (level < 65)
            items = { 29292, 27859, 30458, 27662 };
        else
            items = { 29450, 29451, 29452 };
#endif
#ifdef MANGOSBOT_TWO
    else if (level < 65)
        items = { 29292, 27859, 30458, 27662 };
    else if (level < 75)
        items = { 29450, 29451, 29452 };
    else
        items = { 35947 };
#endif
    }

    if (category == 59)
    {
        if (level < 5)
            items = { 159, 117 };
        else if (level < 15)
            items = { 1179, 21072 };
        else if (level < 25)
            items = { 1205 };
        else if (level < 35)
            items = { 1708 };
        else if (level < 45)
            items = { 1645 };
#ifdef MANGOSBOT_ZERO
        else
#else
        else if (level < 55)
#endif
            items = { 8766 };
#ifdef MANGOSBOT_ONE
        else if (level < 65)
            items = { 28399 };
        else
            items = { 27860 };
#endif
#ifdef MANGOSBOT_TWO
    else if (level < 65)
        items = { 28399 };
    else if (level < 75)
        items = { 27860 };
    else
        items = { 33445 };
#endif
    }

    if (items.empty()) return 0;
    return items[urand(0, items.size() - 1)];
}

uint32 RandomItemMgr::GetRandomFood(uint32 level, uint32 category)
{
    std::vector<uint32> food = foodCache[(level - 1) / 10][category];
    if (food.empty()) return 0;
    return food[urand(0, food.size() - 1)];
}

void RandomItemMgr::BuildTradeCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building trade cache for %d levels", maxLevel);
	int counter4 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_TRADE_GOODS || proto->Bonding != NO_BIND)
                continue;

            if (proto->ItemLevel < level)
                continue;

            if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                continue;

            if (proto->RequiredSkill)
                continue;

            tradeCache[level / 10].push_back(itemId);
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 size = tradeCache[level / 10].size();
        sLog.outDetail("Trade cache for level=%d: %d items", level, size);
		counter4++;
    }
	sLog.outString("Cached %d trade categories", counter4); // TEST
}

uint32 RandomItemMgr::GetRandomTrade(uint32 level)
{
    std::vector<uint32> trade = tradeCache[(level - 1) / 10];
    if (trade.empty()) return 0;
    return trade[urand(0, trade.size() - 1)];
}

std::vector<uint32> RandomItemMgr::GetGemsList()
{
    std::vector<uint32>_gems;

#ifndef MANGOSBOT_ZERO
    if (_gems.empty())
    {
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_GEM)
                continue;

            if (proto->SubClass == ITEM_SUBCLASS_GEM_SIMPLE)
                continue;

            _gems.push_back(proto->ItemId);
        }
    }
#endif
    return _gems;
}

void RandomItemMgr::BuildRarityCache()
{
    auto results = CharacterDatabase.PQuery("select item, rarity from ai_playerbot_rarity_cache");
    if (results)
    {
        sLog.outBasic("Loading item rarity cache");
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            float rarity = fields[1].GetFloat();

            rarityCache[itemId] = rarity;
            count++;

        } while (results->NextRow());
        sLog.outString("Item rarity cache loaded from %d records", count);
    }
    else
    {
        sLog.outBasic("Building item rarity cache from %u items", sItemStorage.GetMaxEntry());
        BarGoLink bar(sItemStorage.GetMaxEntry());
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            bar.step();
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (proto->Quality == ITEM_QUALITY_POOR)
                continue;

            if (strstri(proto->Name1, "qa") || strstri(proto->Name1, "test") || strstri(proto->Name1, "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;
            auto results = WorldDatabase.PQuery(
                    "select max(q.chance) from ( "
                    // "-- Creature "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from creature_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from creature_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'creature' type "
                    "from creature_loot_template lt "
                    "join creature_template ct on ct.LootId = lt.entry "
                    "join creature c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Gameobject "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from gameobject_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from gameobject_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'gameobject' type "
                    "from gameobject_loot_template lt "
                    "join gameobject_template ct on ct.data1 = lt.entry "
                    "join gameobject c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Disenchant "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from disenchant_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from disenchant_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'disenchant' type "
                    "from disenchant_loot_template lt "
                    "join item_template ct on ct.DisenchantID = lt.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Fishing "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from fishing_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from fishing_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'fishing' type "
                    "from fishing_loot_template lt "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Skinning "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance  "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from skinning_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    * ifnull((select 1/count(*) from skinning_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0), 1) "
                    "    end "
                    ") chance, 'skinning' type "
                    "from skinning_loot_template lt "
                    "join creature_template ct on ct.SkinningLootId = lt.entry "
                    "join creature c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    ") q; ",
                             itemId,itemId,itemId,itemId,itemId);

            if (results)
            {
                Field* fields = results->Fetch();
                float rarity = fields[0].GetFloat();
                if (rarity > 0.01)
                {
                    rarityCache[itemId] = rarity;

                    CharacterDatabase.PExecute("insert into ai_playerbot_rarity_cache (item, rarity) values (%u, %f)",
                            itemId, rarity);
                }
            }
        }
        sLog.outString("Item rarity cache built from %u items", sItemStorage.GetMaxEntry());
    }
}

#ifdef MANGOSBOT_TWO
void RandomItemMgr::BuildGlyphCache()
{
    sLog.outString("Building glyphCache", sItemStorage.GetMaxEntry());
    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        if (proto->Class != ITEM_CLASS_GLYPH)
            continue;

        glyphCache[proto->AllowableClass].push_back(proto->ItemId);
    }
}
#endif

void RandomItemMgr::LoadRandomEnchantments()
{
    randomEnchantsCache.clear();

    uint32 count = 0;
    auto queryResult = WorldDatabase.Query("SELECT entry, ench, chance FROM item_enchantment_template");

    if (queryResult)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 ench = fields[1].GetUInt32();
            float chance = fields[2].GetFloat();

            if (chance > 0.000001f && chance <= 100.0f)
                randomEnchantsCache[entry].push_back(ench);

            ++count;
        } while (queryResult->NextRow());

        sLog.outString(">> Loaded %u Item Enchantment definitions", count);
    }
    else
        sLog.outErrorDb(">> Loaded 0 Item Enchantment definitions. DB table `item_enchantment_template` is empty.");

    sLog.outString();
}

float RandomItemMgr::GetItemRarity(uint32 itemId)
{
    return rarityCache[itemId];
}
