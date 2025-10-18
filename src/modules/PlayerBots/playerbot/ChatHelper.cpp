
#include "playerbot/playerbot.h"
#include "ChatHelper.h"
#include "playerbot/AiFactory.h"
#include "strategy/values/ItemUsageValue.h"
#include <numeric>
#include <iomanip>
#include <regex>

using namespace ai;

std::map<std::string, uint32> ChatHelper::consumableSubClasses;
std::map<std::string, uint32> ChatHelper::tradeSubClasses;
std::map<std::string, uint32> ChatHelper::itemQualities;
std::map<std::string, uint32> ChatHelper::projectileSubClasses;
std::map<std::string, std::pair<uint32, uint32>> ChatHelper::itemClasses;
std::map<std::string, uint32> ChatHelper::slots;
std::map<std::string, uint32> ChatHelper::skills;
std::map<std::string, ChatMsg> ChatHelper::chats;
std::map<uint8, std::string> ChatHelper::classes;
std::map<uint8, std::string> ChatHelper::races;
std::map<uint8, std::map<uint8, std::string> > ChatHelper::specs;
std::unordered_map<std::string, std::vector<uint32>> ChatHelper::spellIds;

template<class T>
static bool substrContainsInMap(const std::string& searchTerm, const std::map<std::string, T>& searchIn)
{
    for (typename std::map<std::string, T>::const_iterator i = searchIn.begin(); i != searchIn.end(); ++i)
    {
        std::string term = i->first;
        if (term.size() > 1 && searchTerm.find(term) != std::string::npos)
            return true;
    }

    return false;
}

ChatHelper::ChatHelper(PlayerbotAI* ai) : PlayerbotAIAware(ai)
{
    itemQualities["poor"] = ITEM_QUALITY_POOR;
    itemQualities["gray"] = ITEM_QUALITY_POOR;
    itemQualities["normal"] = ITEM_QUALITY_NORMAL;
    itemQualities["white"] = ITEM_QUALITY_NORMAL;
    itemQualities["uncommon"] = ITEM_QUALITY_UNCOMMON;
    itemQualities["green"] = ITEM_QUALITY_UNCOMMON;
    itemQualities["rare"] = ITEM_QUALITY_RARE;
    itemQualities["blue"] = ITEM_QUALITY_RARE;
    itemQualities["epic"] = ITEM_QUALITY_EPIC;
    itemQualities["violet"] = ITEM_QUALITY_EPIC;
    itemQualities["legendary"] = ITEM_QUALITY_LEGENDARY;
    itemQualities["yellow"] = ITEM_QUALITY_LEGENDARY;

    consumableSubClasses["potion"] = ITEM_SUBCLASS_POTION;
    consumableSubClasses["elixir"] = ITEM_SUBCLASS_ELIXIR;
    consumableSubClasses["flask"] = ITEM_SUBCLASS_FLASK;
    consumableSubClasses["scroll"] = ITEM_SUBCLASS_SCROLL;
    consumableSubClasses["food"] = ITEM_SUBCLASS_FOOD;
    consumableSubClasses["bandage"] = ITEM_SUBCLASS_BANDAGE;
    consumableSubClasses["enchant"] = ITEM_SUBCLASS_CONSUMABLE_OTHER;

    projectileSubClasses["arrows"] = ITEM_SUBCLASS_ARROW;
    projectileSubClasses["bullets"] = ITEM_SUBCLASS_BULLET;

    //tradeSubClasses["cloth"] = ITEM_SUBCLASS_CLOTH;
    //tradeSubClasses["leather"] = ITEM_SUBCLASS_LEATHER;
    //tradeSubClasses["metal"] = ITEM_SUBCLASS_METAL_STONE;
    //tradeSubClasses["stone"] = ITEM_SUBCLASS_METAL_STONE;
    //tradeSubClasses["ore"] = ITEM_SUBCLASS_METAL_STONE;
    //tradeSubClasses["meat"] = ITEM_SUBCLASS_MEAT;
    //tradeSubClasses["herb"] = ITEM_SUBCLASS_HERB;
    //tradeSubClasses["elemental"] = ITEM_SUBCLASS_ELEMENTAL;
    //tradeSubClasses["disenchants"] = ITEM_SUBCLASS_ENCHANTING;
    //tradeSubClasses["enchanting"] = ITEM_SUBCLASS_ENCHANTING;
    //tradeSubClasses["gems"] = ITEM_SUBCLASS_JEWELCRAFTING;
    //tradeSubClasses["jewels"] = ITEM_SUBCLASS_JEWELCRAFTING;
    //tradeSubClasses["jewelcrafting"] = ITEM_SUBCLASS_JEWELCRAFTING;

    itemClasses["1h axe"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_AXE };
    itemClasses["2h axe"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_AXE2 };
    itemClasses["bow"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_BOW };
    itemClasses["gun"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_GUN };
    itemClasses["1h mace"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_MACE };
    itemClasses["2h mace"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_MACE2 };
    itemClasses["polearm"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_POLEARM };
    itemClasses["1h sword"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_SWORD };
    itemClasses["2h sword"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_SWORD2 };
    itemClasses["staff"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_STAFF };
    itemClasses["first"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_FIST };
    itemClasses["dagger"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_DAGGER };
    itemClasses["thrown"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_THROWN };
    itemClasses["crossbow"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_CROSSBOW };
    itemClasses["wand"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_WAND };
    itemClasses["fishing pole"] = { ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_FISHING_POLE };

    itemClasses["bag"] = { ITEM_CLASS_CONTAINER, ITEM_SUBCLASS_CONTAINER };

    itemClasses["cloth"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_CLOTH };
    itemClasses["leather"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_LEATHER };
    itemClasses["mail"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_MAIL };
    itemClasses["plate"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_PLATE };
    itemClasses["shield"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_SHIELD };
    itemClasses["libram"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_LIBRAM };
    itemClasses["idol"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_IDOL };
    itemClasses["totem"] = { ITEM_CLASS_ARMOR, ITEM_SUBCLASS_ARMOR_TOTEM };

    slots["head"] = EQUIPMENT_SLOT_HEAD;
    slots["neck"] = EQUIPMENT_SLOT_NECK;
    slots["shoulder"] = EQUIPMENT_SLOT_SHOULDERS;
    slots["shirt"] = EQUIPMENT_SLOT_BODY;
    slots["chest"] = EQUIPMENT_SLOT_CHEST;
    slots["waist"] = EQUIPMENT_SLOT_WAIST;
    slots["legs"] = EQUIPMENT_SLOT_LEGS;
    slots["feet"] = EQUIPMENT_SLOT_FEET;
    slots["wrist"] = EQUIPMENT_SLOT_WRISTS;
    slots["hands"] = EQUIPMENT_SLOT_HANDS;
    slots["finger 1"] = EQUIPMENT_SLOT_FINGER1;
    slots["finger 2"] = EQUIPMENT_SLOT_FINGER2;
    slots["trinket 1"] = EQUIPMENT_SLOT_TRINKET1;
    slots["trinket 2"] = EQUIPMENT_SLOT_TRINKET2;
    slots["back"] = EQUIPMENT_SLOT_BACK;
    slots["main hand"] = EQUIPMENT_SLOT_MAINHAND;
    slots["off hand"] = EQUIPMENT_SLOT_OFFHAND;
    slots["ranged"] = EQUIPMENT_SLOT_RANGED;
    slots["tabard"] = EQUIPMENT_SLOT_TABARD;

    skills["first aid"] = SKILL_FIRST_AID;
    skills["fishing"] = SKILL_FISHING;
    skills["cooking"] = SKILL_COOKING;
    skills["alchemy"] = SKILL_ALCHEMY;
    skills["enchanting"] = SKILL_ENCHANTING;
    skills["engineering"] = SKILL_ENGINEERING;
    skills["leatherworking"] = SKILL_LEATHERWORKING;
    skills["blacksmithing"] = SKILL_BLACKSMITHING;
    skills["tailoring"] = SKILL_TAILORING;
    skills["herbalism"] = SKILL_HERBALISM;
    skills["mining"] = SKILL_MINING;
    skills["skinning"] = SKILL_SKINNING;

    chats["party"] = CHAT_MSG_PARTY;
    chats["p"] = CHAT_MSG_PARTY;
    chats["guild"] = CHAT_MSG_GUILD;
    chats["g"] = CHAT_MSG_GUILD;
    chats["raid"] = CHAT_MSG_RAID;
    chats["r"] = CHAT_MSG_RAID;
    chats["whisper"] = CHAT_MSG_WHISPER;
    chats["w"] = CHAT_MSG_WHISPER;

    classes[CLASS_DRUID] = "druid";
    specs[CLASS_DRUID][0] = "balance";
    specs[CLASS_DRUID][1] = "feral combat";
    specs[CLASS_DRUID][2] = "restoration";

    classes[CLASS_HUNTER] = "hunter";
    specs[CLASS_HUNTER][0] = "beast mastery";
    specs[CLASS_HUNTER][1] = "marksmanship";
    specs[CLASS_HUNTER][2] = "survival";

    classes[CLASS_MAGE] = "mage";
    specs[CLASS_MAGE][0] = "arcane";
    specs[CLASS_MAGE][1] = "fire";
    specs[CLASS_MAGE][2] = "frost";

    classes[CLASS_PALADIN] = "paladin";
    specs[CLASS_PALADIN][0] = "holy";
    specs[CLASS_PALADIN][1] = "protection";
    specs[CLASS_PALADIN][2] = "retribution";

    classes[CLASS_PRIEST] = "priest";
    specs[CLASS_PRIEST][0] = "discipline";
    specs[CLASS_PRIEST][1] = "holy";
    specs[CLASS_PRIEST][2] = "shadow";

    classes[CLASS_ROGUE] = "rogue";
    specs[CLASS_ROGUE][0] = "assasination";
    specs[CLASS_ROGUE][1] = "combat";
    specs[CLASS_ROGUE][2] = "subtlety";

    classes[CLASS_SHAMAN] = "shaman";
    specs[CLASS_SHAMAN][0] = "elemental";
    specs[CLASS_SHAMAN][1] = "enhancement";
    specs[CLASS_SHAMAN][2] = "restoration";

    classes[CLASS_WARLOCK] = "warlock";
    specs[CLASS_WARLOCK][0] = "affliction";
    specs[CLASS_WARLOCK][1] = "demonology";
    specs[CLASS_WARLOCK][2] = "destruction";

    classes[CLASS_WARRIOR] = "warrior";
    specs[CLASS_WARRIOR][0] = "arms";
    specs[CLASS_WARRIOR][1] = "fury";
    specs[CLASS_WARRIOR][2] = "protection";

#ifdef MANGOSBOT_TWO
    classes[CLASS_DEATH_KNIGHT] = "dk";
    specs[CLASS_DEATH_KNIGHT][0] = "blood";
    specs[CLASS_DEATH_KNIGHT][1] = "frost";
    specs[CLASS_DEATH_KNIGHT][2] = "unholy";
#endif

    races[RACE_DWARF] = "Dwarf";
    races[RACE_GNOME] = "Gnome";
    races[RACE_HUMAN] = "Human";
    races[RACE_NIGHTELF] = "Night Elf";
    races[RACE_ORC] = "Orc";
    races[RACE_TAUREN] = "Tauren";
    races[RACE_TROLL] = "Troll";
    races[RACE_UNDEAD] = "Undead";
#ifndef MANGOSBOT_ZERO
    races[RACE_BLOODELF] = "Blood Elf";
    races[RACE_DRAENEI] = "Draenei";
#endif
}

std::string ChatHelper::formatMoney(uint32 copper)
{
    std::ostringstream out;
	if (!copper)
	{
		out << "0";
		return out.str();
	}

    uint32 gold = uint32(copper / 10000);
    copper -= (gold * 10000);
    uint32 silver = uint32(copper / 100);
    copper -= (silver * 100);

    bool space = false;
    if (gold > 0)
    {
        out << gold <<  "g";
        space = true;
    }

    if (silver > 0 && gold < 50)
    {
        if (space) out << " ";
        out << silver <<  "s";
        space = true;
    }

	if (copper > 0 && gold < 10)
	{
        if (space) out << " ";
		out << copper <<  "c";
	}

    return out.str();
}

uint32 ChatHelper::parseMoney(const std::string& text)
{
    // if user specified money in ##g##s##c format
    std::string acum = "";
    uint32 copper = 0;
    for (uint8 i = 0; i < text.length(); i++)
    {
        if (text[i] == 'g')
        {
            copper += (atol(acum.c_str()) * 100 * 100);
            acum = "";
        }
        else if (text[i] == 'c')
        {
            copper += atol(acum.c_str());
            acum = "";
        }
        else if (text[i] == 's')
        {
            copper += (atol(acum.c_str()) * 100);
            acum = "";
        }
        else if (text[i] == ' ')
            break;
        else if (text[i] >= 48 && text[i] <= 57)
            acum += text[i];
        else
        {
            copper = 0;
            break;
        }
    }
    return copper;
}

std::set<uint32> ChatHelper::ExtractAllQuestIds(const std::string& text)
{
    std::set<uint32> ids;

    std::regex rgx("Hquest:[0-9]+");
    auto begin = std::sregex_iterator(text.begin(), text.end(), rgx);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; ++i)
    {
        std::smatch match = *i;
        ids.insert(std::stoi(match.str().erase(0, 7)));
    }

    return ids;
}

std::set<uint32> ChatHelper::ExtractAllItemIds(const std::string& text)
{
    std::set<uint32> ids;

    std::regex rgx("Hitem:[0-9]+");
    auto begin = std::sregex_iterator(text.begin(), text.end(), rgx);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; ++i)
    {
        std::smatch match = *i;
        ids.insert(std::stoi(match.str().erase(0, 6)));
    }

    return ids;
}

std::set<uint32> ChatHelper::ExtractAllSkillIds(const std::string& text)
{
    std::set<uint32> ids;

    std::regex rgx("Hskill:[0-9]+");
    auto begin = std::sregex_iterator(text.begin(), text.end(), rgx);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; ++i)
    {
        std::smatch match = *i;
        ids.insert(std::stoi(match.str().erase(0, 7)));
    }

    return ids;
}

std::set<uint32> ChatHelper::ExtractAllFactionIds(const std::string& text)
{
    std::set<uint32> ids;

    std::regex rgx("Hfaction:[0-9]+");
    auto begin = std::sregex_iterator(text.begin(), text.end(), rgx);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; ++i)
    {
        std::smatch match = *i;
        ids.insert(std::stoi(match.str().erase(0, 9)));
    }

    return ids;
}

ItemIds ChatHelper::parseItems(const std::string& text, bool validate)
{
    std::vector<uint32> itemIDsUnordered = parseItemsUnordered(text, validate);
    return ItemIds(itemIDsUnordered.begin(), itemIDsUnordered.end());
}

std::vector<uint32> ChatHelper::parseItemsUnordered(const std::string& text, bool validate)
{
    std::string textCpy = text;

    // Remove gameobject links
    if (textCpy.find("Hfound:") != -1)
    {
        std::vector<std::string> goLinks = findSubstringsBetween(textCpy, "|c", "|r", true);
        for (const std::string& goLink : goLinks)
        {
            std::vector<std::string> goIDs = findSubstringsBetween(goLink, "Hfound:", ":");
            if (!goIDs.empty())
            {
                replaceSubstring(textCpy, goLink, "");
            }
        }
    }

    // Replace all item links with item ids
    if (textCpy.find("Hitem:") != -1)
    {
        std::vector<std::string> itemLinks = findSubstringsBetween(textCpy, "|c", "|r", true);
        for (const std::string& itemLink : itemLinks)
        {
            std::vector<std::string> itemIDs = findSubstringsBetween(itemLink, "Hitem:", ":");
            if (!itemIDs.empty())
            {
                std::ostringstream itemIDWithSpaces;
                itemIDWithSpaces << " " << itemIDs[0] << " ";
                replaceSubstring(textCpy, itemLink, itemIDWithSpaces.str());
            }
        }
    }

    std::vector<uint32> itemIds;
    for (const std::string& itemStr : splitString(textCpy, " "))
    {
        if (itemStr.empty())
        {
            continue;
        }

        if (isNumeric(itemStr))
        {
            const uint32 itemID = std::stoi(itemStr);
            if (!validate || sObjectMgr.GetItemPrototype(itemID) != nullptr)
            {
                itemIds.push_back(std::stoi(itemStr));
            }
        }
    }

    return itemIds;
}

std::set<std::string> ChatHelper::parseItemQualifiers(const std::string& text)
{
    std::set<std::string> qualifiers;

    uint8 pos = 0;
    while (true)
    {
        int i = text.find("Hitem:", pos);
        if (i == -1)
            break;
        pos = i + 6;
        int endPos = text.find('|', pos);
        if (endPos == -1)
            break;

        std::string qualifierString = text.substr(pos, endPos - pos);

        qualifiers.insert(qualifierString);
    }

    return qualifiers;
}

std::string ChatHelper::formatQuest(Quest const* quest)
{
    std::ostringstream out;
    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
    std::string title = quest->GetTitle();
    sObjectMgr.GetQuestLocaleStrings(quest->GetQuestId(), loc_idx, &title);
    out << "|cFFFFFF00|Hquest:" << quest->GetQuestId() << ':' << quest->GetQuestLevel() << "|h[" << title << "]|h|r";
    return out.str();
}

std::string ChatHelper::formatGameobject(const GameObject* go)
{
    std::ostringstream out;
    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
    std::string name = go->GetGOInfo()->name;
    if (loc_idx >= 0)
    {
        GameObjectLocale const* gl = sObjectMgr.GetGameObjectLocale(go->GetEntry());
        if (gl)
        {
            if ((int32)gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                name = gl->Name[loc_idx];
        }
    }
    out << "|cFFFFFF00|Hfound:" << go->GetObjectGuid().GetRawValue() << ":" << go->GetEntry() << ":" <<  "|h[" << name << "]|h|r";
    return out.str();
}

std::string ChatHelper::formatWorldobject(const WorldObject* wo)
{
    std::ostringstream out;
    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
    std::string name = (wo->IsGameObject() ? ((GameObject*)wo)->GetGOInfo()->name : wo->GetName());
    if (name.empty()) name = "unknown:" + std::to_string(wo->GetEntry());
    if (loc_idx >= 0 && wo->IsGameObject())
    {
        GameObjectLocale const* gl = sObjectMgr.GetGameObjectLocale(wo->GetEntry());
        if (gl)
        {
            if ((int32)gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                name = gl->Name[loc_idx];
        }
    }
    out << "|cFFFFFF00|Hfound:" << wo->GetObjectGuid().GetRawValue() << ":" << wo->GetEntry() << ":" << "|h[" << name << "]|h|r";
    return out.str();
}

std::string ChatHelper::formatWorldEntry(int32 entry)
{
    CreatureInfo const* cInfo = NULL;
    GameObjectInfo const* gInfo = NULL;

    if (entry > 0)
        cInfo = ObjectMgr::GetCreatureTemplate(entry);
    else
        gInfo = ObjectMgr::GetGameObjectInfo(entry * -1);

    std::ostringstream out;
    out << "|cFFFFFF00|Hentry:" << abs(entry) << ":" << "|h[";

    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
    std::string name;
    if (entry < 0 && gInfo)
        name = gInfo->name;
    else if (entry > 0 && cInfo)
        name = cInfo->Name;
    
    if(name.empty())
        name = "unknown:" + std::to_string(entry);

    if (loc_idx >= 0 && entry < 0)
    {
        GameObjectLocale const* gl = sObjectMgr.GetGameObjectLocale(entry);
        if (gl)
        {
            if ((int32)gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                name = gl->Name[loc_idx];
        }
    }
    if (loc_idx >= 0 && entry > 0)
    {
        char const* tname = "";
        sObjectMgr.GetCreatureLocaleStrings(entry, loc_idx, &tname);
        if (*tname)
        {
            name = *tname;
        }
    }
    
    out << name;
    
    out << "]|h|r";
    return out.str();
}

std::string ChatHelper::formatSpell(SpellEntry const *sInfo)
{
    std::ostringstream out;
    out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r";
    return out.str();
}

std::string ChatHelper::formatItem(ItemQualifier& itemQualifier, int count, int total)
{
    char color[32];
    ItemPrototype const* proto = itemQualifier.GetProto();
    sprintf(color, "%x", ItemQualityColors[proto->Quality]);

    std::ostringstream out;
    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
    std::string name = proto->Name1;
    if (loc_idx >= 0)
    {
        std::string tname;
        sObjectMgr.GetItemLocaleStrings(itemQualifier.GetId(), loc_idx, &tname);
        if (!tname.empty())
            name = tname;
    }

    if (itemQualifier.GetRandomPropertyId())
    {
        ItemRandomPropertiesEntry const* item_rand = sItemRandomPropertiesStore.LookupEntry(abs(itemQualifier.GetRandomPropertyId()));

        if (item_rand)
        {
            if (loc_idx < 0)
                loc_idx = 0;
            std::string suffix = item_rand->nameSuffix[loc_idx];
            name += " " + suffix;
        }
    }

    out << "|c" << color << "|Hitem:" << itemQualifier.GetLinkQualifier() << "|h[" << name
        << "]|h|r";

    if (count > 1)
        out << "x" << count;

    if (total > 0)
        out << " (" << total << ")";

    return out.str();
}

std::string ChatHelper::formatItem(ItemPrototype const* proto, int count, int total)
{
    ItemQualifier ItemQualifier(proto->ItemId);
    return formatItem(ItemQualifier, count, total);
}

std::string ChatHelper::formatItem(Item* item, int count, int total)
{
    ItemQualifier itemQualifier(item);
    return formatItem(itemQualifier, count, total);
}

std::string ChatHelper::formatQItem(uint32 itemId)
{
    char color[32];
    sprintf(color, "%x", ItemQualityColors[0]);

    std::ostringstream out;
    out << "|c" << color << "|Hitem:" << itemId
        << ":0:0:0:0:0:0:0" << "|h[item"
        << "]|h|r";

    return out.str();
}

std::string ChatHelper::formatSkill(uint32 skillId, Player* player)
{
    std::string name = "unknown skill";

    SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(skillId);
    if (skillInfo)
    {
        int loc_idx = sPlayerbotTextMgr.GetLocalePriority();

        if (loc_idx == -1)
            loc_idx = 0;
        name = skillInfo->name[loc_idx];
    }
    std::ostringstream out;
    out << "|cffffffff|Hskill:" << skillId
        << "|h[" << name << "]|h|r";

    if (player && player->HasSkill(skillId))
    {
        uint32 curValue = player->GetSkillValuePure(skillId);
        uint32 maxValue = player->GetSkillMaxPure(skillId);
        uint32 permValue = player->GetSkillBonusPermanent(skillId);
        uint32 tempValue = player->GetSkillBonusTemporary(skillId);

        out << " (";

        out << curValue << "/" << maxValue;

        if (permValue)
            out << " +perm " << permValue;

        if (tempValue)
            out << " +temp " << permValue;
    }

    return out.str();
}

std::string ChatHelper::formatReaction(ReputationRank rank, Player* player)
{
    uint32 ReputationRankStrIndex[MAX_REPUTATION_RANK];
    ReputationRankStrIndex[REP_HATED] = LANG_REP_HATED;
    ReputationRankStrIndex[REP_HOSTILE] = LANG_REP_HOSTILE;
    ReputationRankStrIndex[REP_UNFRIENDLY] = LANG_REP_UNFRIENDLY;
    ReputationRankStrIndex[REP_NEUTRAL] = LANG_REP_NEUTRAL;
    ReputationRankStrIndex[REP_FRIENDLY] = LANG_REP_FRIENDLY;
    ReputationRankStrIndex[REP_HONORED] = LANG_REP_HONORED;
    ReputationRankStrIndex[REP_REVERED] = LANG_REP_REVERED;
    ReputationRankStrIndex[REP_EXALTED] = LANG_REP_EXALTED;

    int loc_idx = sPlayerbotTextMgr.GetLocalePriority();

    std::string rankName = sObjectMgr.GetMangosString(ReputationRankStrIndex[rank], loc_idx);

    std::ostringstream out;

    switch (rank)
    {
    case REP_HATED:
        out << "|cff8b0000";
        break;
    case REP_HOSTILE:
        out << "|cffff0000";
        break;
    case REP_NEUTRAL:
    case REP_UNFRIENDLY:
        out << "|cffffff00";
        break;
    case REP_FRIENDLY:
    case REP_HONORED:
    case REP_REVERED:
    case REP_EXALTED:
        out << "|cff00ff00";
        break;
    }

    out << rankName << "|r";

    return out.str();
}

std::string ChatHelper::formatFaction(uint32 factionId, Player* player)
{
    std::string name = "unknown faction";

#ifndef MANGOSBOT_ONE
    const FactionEntry* factionEntry = sFactionStore.LookupEntry(factionId);
#else
    const FactionEntry* factionEntry = sFactionStore.LookupEntry<FactionEntry>(factionId);
#endif

    if (factionEntry)
    {
        int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
        if (loc_idx == -1)
            loc_idx = 0;
        name = factionEntry->name[loc_idx];
    }
    std::ostringstream out;
    out << "|cffffffff|Hfaction:" << factionId
        << "|h[" << name << "]|h|r";

    if (player)
    {
        FactionState const* repState = player->GetReputationMgr().GetState(factionEntry);

        if (repState && repState->Flags & FACTION_FLAG_VISIBLE)
        {                        
            ReputationRank rank = player->GetReputationMgr().GetRank(factionEntry);
            out << " " << formatReaction(rank, player) << " (" << player->GetReputationMgr().GetReputation(factionEntry) << ")";

            if (repState->Flags & FACTION_FLAG_AT_WAR)
                out << " at war";
        }
    }

    return out.str();
}

ChatMsg ChatHelper::parseChat(const std::string& text)
{
    if (chats.find(text) != chats.end())
        return chats[text];

    return CHAT_MSG_SYSTEM;
}

std::string ChatHelper::formatChat(ChatMsg chat)
{
    switch (chat)
    {
    case CHAT_MSG_GUILD:
        return "guild";
    case CHAT_MSG_PARTY:
        return "party";
    case CHAT_MSG_WHISPER:
        return "whisper";
    case CHAT_MSG_RAID:
        return "raid";
    default:
        return "unknown";
    }

    return "unknown";
}


uint32 ChatHelper::parseSpell(std::string& text)
{
    PlayerbotChatHandler handler(ai->GetBot());
    return handler.extractSpellId(text);
}

std::list<ObjectGuid> ChatHelper::parseGameobjects(const std::string& text)
{
    std::list<ObjectGuid> gos;
    //    Link format
    //    |cFFFFFF00|Hfound:" << guid << ':'  << entry << ':'  <<  "|h[" << gInfo->name << "]|h|r";
    //    |cFFFFFF00|Hfound:9582:1731|h[Copper Vein]|h|r

    uint8 pos = 0;
    while (true)
    {
        // extract GO guid
        int i = text.find("Hfound:", pos);     // base H = 11
        if (i == -1)     // break if error
            break;

        pos = i + 7;     //start of window in text 11 + 7 = 18
        int endPos = text.find(':', pos);     // end of window in text 22
        if (endPos == -1)     //break if error
            break;
        std::istringstream stream(text.substr(pos, endPos - pos));
        uint64 guid; stream >> guid;

        // extract GO entry
        pos = endPos + 1;
        endPos = text.find(':', pos);     // end of window in text
        if (endPos == -1)     //break if error
            break;

        std::string entryC = text.substr(pos, endPos - pos);     // get std::string within window i.e entry
        uint32 entry = std::atol(entryC.c_str());     // convert ascii to float

        ObjectGuid lootCurrent = ObjectGuid(guid);

        if (guid)
            gos.push_back(lootCurrent);
    }

    return gos;
}

std::list<int32> ChatHelper::parseWorldEntries(const std::string& text)
{
    std::list<int32> entries;
    //    Link format
    //    |cFFFFFF00|Hentry:" << entry << ':'  <<  "|h[" << gInfo->name << "]|h|r";
    //    |cFFFFFF00|Hfound:9582:1731|h[Copper Vein]|h|r

    uint8 pos = 0;
    while (true)
    {
        // extract GO guid
        int i = text.find("Hentry:", pos);     // base H = 11
        if (i == -1)     // break if error
            break;

        pos = i + 7;     //start of window in text 11 + 7 = 18
        int endPos = text.find(':', pos);     // end of window in text 22
        if (endPos == -1)     //break if error
            break;
        std::string entryC = text.substr(pos, endPos - pos);     // get std::string within window i.e entry        
        uint32 entry = std::atol(entryC.c_str());     // convert ascii to float

        if (entry)
            entries.push_back(entry);
    }

    return entries;
}

std::string ChatHelper::formatQuestObjective(const std::string& name, int available, int required)
{
    std::ostringstream out;
    out << "|cFFFFFFFF" << name << (available >= required ? "|c0000FF00: " : "|c00FF0000: ")
        << available << "/" << required << "|r";

    return out.str();
}

std::string ChatHelper::formatValue(const std::string& type, const std::string& topicCode, const std::string& topicName, const std::string& color)
{
    std::ostringstream out;
    out << "|c"<< color << "|Hvalue:" << type << ":" << topicCode << "|h[" << topicName << "]|h|r";
    return out.str();
}

std::string ChatHelper::parseValue(const std::string& type, const std::string& text) {
    std::string retString;

    std::string pattern = "Hvalue:" + type + ":";

    int pos = text.find(pattern, 0);
    if (pos == -1)
        return retString;

    pos += pattern.size();

    int endPos = text.find('|', pos);
    if (endPos == -1)
        return retString;

    retString = text.substr(pos, endPos - pos);
    return retString;
}

uint32 ChatHelper::parseItemQuality(const std::string& text)
{
    if (itemQualities.find(text) == itemQualities.end())
        return MAX_ITEM_QUALITY;

    return itemQualities[text];
}

bool ChatHelper::parseItemClass(const std::string& text, uint32* itemClass, uint32* itemSubClass)
{
    if (text == "questitem")
    {
        *itemClass = ITEM_CLASS_QUEST;
        *itemSubClass = ITEM_SUBCLASS_QUEST;
        return true;
    }

    if (consumableSubClasses.find(text) != consumableSubClasses.end())
    {
        *itemClass = ITEM_CLASS_CONSUMABLE;
        *itemSubClass = consumableSubClasses[text];
        return true;
    }

    if (tradeSubClasses.find(text) != tradeSubClasses.end())
    {
        *itemClass = ITEM_CLASS_TRADE_GOODS;
        *itemSubClass = tradeSubClasses[text];
        return true;
    }

    if (projectileSubClasses.find(text) != projectileSubClasses.end())
    {
        *itemClass = ITEM_CLASS_PROJECTILE;
        *itemSubClass = projectileSubClasses[text];
        return true;
    }

    if (itemClasses.find(text) != itemClasses.end())
    {
        *itemClass = itemClasses[text].first;
        *itemSubClass = itemClasses[text].second;
        return true;
    }

    return false;
}

uint32 ChatHelper::parseSlot(const std::string& text)
{
    if (slots.find(text) != slots.end())
        return slots[text];

    return EQUIPMENT_SLOT_END;
}

bool ChatHelper::parseable(const std::string& text)
{
    return text.find("|H") != std::string::npos ||
        text.find("[h:") != std::string::npos ||
            text == "questitem" ||
            text == "ammo" ||
            substrContainsInMap<uint32>(text, consumableSubClasses) ||
            substrContainsInMap<uint32>(text, tradeSubClasses) ||
            substrContainsInMap<uint32>(text, itemQualities) ||
            (substrContainsInMap<uint32>(text, slots) && text.find("rtsc ") == std::string::npos) ||
            (substrContainsInMap<ChatMsg>(text, chats) && text.find(" on party") == std::string::npos) ||
            substrContainsInMap<uint32>(text, skills) ||
            parseMoney(text) > 0;
}

std::string ChatHelper::specName(const Player* player)
{
    return specs[player->getClass()][AiFactory::GetPlayerSpecTab(player)];
}

std::string ChatHelper::formatClass(const Player* player, int spec)
{
    uint8 cls = player->getClass();

    std::ostringstream out;
    out << specs[cls][spec] << " (";

    std::map<uint32, int32> tabs = AiFactory::GetPlayerSpecTabs(player);
    int c0 = (int)tabs[0];
    int c1 = (int)tabs[1];
    int c2 = (int)tabs[2];

    out << (c0 ? "|h|cff00ff00" : "") << c0 << "|h|cffffffff/";
    out << (c1 ? "|h|cff00ff00" : "") << c1 << "|h|cffffffff/";
    out << (c2 ? "|h|cff00ff00" : "") << c2 << "|h|cffffffff";

    out << ")|r " << classes[cls];
    return out.str();
}

std::string ChatHelper::formatClass(uint8 cls)
{
    return classes[cls];
}

std::string ChatHelper::formatRace(uint8 race)
{
    return races[race];
}

std::string ChatHelper::formatFactionName(uint32 factionId)
{
    std::string name = "unknown faction";

#ifndef MANGOSBOT_ONE
    const FactionEntry* factionEntry = sFactionStore.LookupEntry(factionId);
#else
    const FactionEntry* factionEntry = sFactionStore.LookupEntry<FactionEntry>(factionId);
#endif

    if (factionEntry)
    {
        int loc_idx = sPlayerbotTextMgr.GetLocalePriority();
        if (loc_idx == -1)
            loc_idx = 0;
        name = factionEntry->name[loc_idx];
    }

    return name;
}

uint32 ChatHelper::parseSkillName(const std::string& text)
{
    if (skills.find(text) != skills.end())
        return skills[text];

    return SKILL_NONE;
}

std::string ChatHelper::getSkillName(uint32 skill)
{
    for (std::map<std::string, uint32>::iterator i = skills.begin(); i != skills.end(); ++i)
    {
        if (i->second == skill)
            return i->first;
    }

    return "";
}

std::string ChatHelper::formatAngle(float angle)
{
    std::vector<std::string> headings = { "north", "north west", "west", "south west", "south", "south east", "east", "north east" };

    float headingAngle = angle / M_PI_F * 180;

    if (headingAngle < 0)
        headingAngle += 360;

    return headings[int32(round(headingAngle / 45)) % 8];
}

std::string ChatHelper::formatWorldPosition(const WorldPosition& pos, const WorldPosition refPos)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    if (!refPos || refPos.getMapId() != pos.getMapId())
    {
        out << pos.getX() << "," << pos.getY() << "," << pos.getZ();

        if (pos.getMap(pos.getFirstInstanceId()))
            out << " in " << pos.getMap(pos.getFirstInstanceId())->GetMapName();
        else
            out << " map:" << pos.getMapId();
    }
    else
    {
        float distance = refPos.distance(pos);
        float angle = refPos.getAngleTo(pos);

        out << distance << "y to the " << formatAngle(angle);
    }

    return out.str();
}


std::string ChatHelper::formatGuidPosition(const GuidPosition& guidP, const GuidPosition& ref)
{
    std::ostringstream out;
    if (guidP.GetWorldObject(guidP.getFirstInstanceId()))
        out << formatWorldobject(guidP.GetWorldObject(guidP.getFirstInstanceId()));
    else if (guidP.GetCreatureTemplate())
        out << formatWorldEntry(guidP.GetEntry());
    else if (guidP.GetGameObjectInfo())
        out << formatWorldEntry(guidP.GetEntry()*-1);
    else
        out << "|cFFFFFF00|Hfound:" << guidP.GetRawValue() << ":" << guidP.GetEntry() << ":" << "|h[unkown " << guidP.GetTypeName() << " " << guidP.GetRawValue() << "]|h|r";

    out.seekp(-5, out.cur);

    if (WorldPosition(guidP))
        out << " " << formatWorldPosition(guidP, ref);

    if(ref && ref.IsPlayer() && (guidP.IsCreature() || guidP.IsPlayer()))
        out << " " << formatReaction(guidP.GetReactionTo(ref, ref.getFirstInstanceId()));

    out << "]|h|r";

    return out.str();
}

std::string ChatHelper::formatBoolean(bool flag)
{
    return flag ? "|cff00ff00ON|r" : "|cffffff00OFF|r";
}

void ChatHelper::eraseAllSubStr(std::string& mainStr, const std::string& toErase)
{
    size_t pos = std::string::npos;
    // Search for the substring in std::string in a loop untill nothing is found
    while ((pos = mainStr.find(toErase)) != std::string::npos)
    {
        // If found then erase it from std::string
        mainStr.erase(pos, toErase.length());
    }
}

inline std::string toInitCap(const std::string& str) {
    std::string result = str;
    bool isFirstChar = true;
    for (char& c : result) {
        if (isFirstChar && std::isalpha(c)) {
            c = std::toupper(c);
            isFirstChar = false;
        }
        else if (std::isspace(c)) {
            isFirstChar = true;
        }
        else {
            c = std::tolower(c);
        }
    }
    return result;
}

void ChatHelper::PopulateSpellNameList()
{
    for (uint32 i = 0; i < GetSpellStore()->GetMaxEntry(); ++i)
    {
        SpellEntry const* tempSpell = GetSpellStore()->LookupEntry<SpellEntry>(i);

        if (!tempSpell)
            continue;

        std::string lowerSpellName = tempSpell->SpellName[0];
        strToLower(lowerSpellName);

        spellIds[tempSpell->SpellName[0]].push_back(tempSpell->Id);
        spellIds[lowerSpellName].push_back(tempSpell->Id);
    }
}

std::vector<uint32> ChatHelper::SpellIds(const std::string& name)
{
    auto spells = spellIds.find(name);

    if (spells == spellIds.end())
        return {};

    return spells->second;
}

std::vector<std::string> ChatHelper::splitString(const std::string& text, const std::string& delimiter)
{
    std::vector<std::string> tokens;

    if (text.empty())
    {
        return tokens;
    }

    if (delimiter.empty()) 
    {
        tokens.push_back(text);
        return tokens;
    }

    std::size_t pos = 0;
    std::size_t found;

    while ((found = text.find(delimiter, pos)) != std::string::npos)
    {
        tokens.push_back(text.substr(pos, found - pos));
        pos = found + delimiter.length();
    }

    tokens.push_back(text.substr(pos)); // Last token

    return tokens;
}

std::vector<std::string> ChatHelper::findSubstringsBetween(const std::string& input, const std::string& start, const std::string& end, bool includeDelimiters)
{
    std::vector<std::string> substrings;
    size_t startPos = 0;
    size_t endPos;

    while ((startPos = input.find(start, startPos)) != std::string::npos) 
    {
        startPos += start.length();
        endPos = input.find(end, startPos);
        if (endPos == std::string::npos) 
        {
            break;
        }

        if (includeDelimiters) 
        {
            substrings.push_back(input.substr(startPos - start.length(), (endPos - (startPos - start.length())) + end.length()));
        }
        else 
        {
            substrings.push_back(input.substr(startPos, endPos - startPos));
        }

        startPos = endPos + end.length();
    }

    return substrings;
}

void ChatHelper::replaceSubstring(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = str.find(oldStr);
    while (pos != std::string::npos) 
    {
        str.replace(pos, oldStr.length(), newStr);
        pos = str.find(oldStr, pos + newStr.length());
    }
}
