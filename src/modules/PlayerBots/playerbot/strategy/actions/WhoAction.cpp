
#include "playerbot/playerbot.h"
#include "WhoAction.h"
#include "playerbot/AiFactory.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/RandomPlayerbotMgr.h"

using namespace ai;

#ifndef WIN32
inline int strcmpi(const char* s1, const char* s2)
{
    for (; *s1 && *s2 && (toupper(*s1) == toupper(*s2)); ++s1, ++s2);
    return *s1 - *s2;
}
#endif

bool WhoAction::Execute(Event& event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    if (!owner->IsInWorld())
        return false;

    if (owner->IsStunnedByLogout())
        return false;

    if (!sObjectMgr.GetPlayer(owner->GetObjectGuid()))
        return false;

    std::ostringstream out;
    std::string text = event.getParam();
    if (!text.empty())
    {
        out << QuerySkill(text);

        if (sRandomPlayerbotMgr.IsFreeBot(bot))
            out << QueryTrade(text);
    }
    else
    {
        out << QuerySpec(text);
    }

    if (!out.str().empty())
    {
        if (AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(sServerFacade.GetAreaId(bot)))
        {
            out << ", (|cffb04040" << areaEntry->area_name[0] << "|r)";
        }
    }

    if (ai->GetMaster())
    {
        if (!out.str().empty()) out << ", ";
        out << "playing with " << ai->GetMaster()->GetName();
    }

    std::string tell = out.str();
    if (tell.empty())
        return false;

    // ignore random bot chat filter
	bot->Whisper(tell, LANG_UNIVERSAL, owner->GetObjectGuid());
    return true;
}


std::string WhoAction::QueryTrade(std::string text)
{
    std::ostringstream out;

    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    std::list<Item*> items = ai->InventoryParseItems(text, mask);
    for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        Item* sell = *i;
        int32 sellPrice = ItemUsageValue::GetBotSellPrice(sell->GetProto(), bot) * sell->GetCount();
        if (!sellPrice)
            continue;

        out << "Selling " << chat->formatItem(sell, sell->GetCount()) << " for " << chat->formatMoney(sellPrice);
        return out.str();
    }

    return "";
}

std::string WhoAction::QuerySkill(std::string text)
{
    std::ostringstream out;
    uint32 skill = chat->parseSkillName(text);
    if (!skill || !ai->HasSkill((SkillType)skill))
        return "";

    std::string skillName = chat->getSkillName(skill);
    uint32 spellId = AI_VALUE2(uint32, "spell id", skillName);
    uint16 value = bot->GetSkillValue(skill);
#ifdef MANGOS
    uint16 maxSkill = bot->GetMaxSkillValue(skill);
#endif
#ifdef CMANGOS
    uint16 maxSkill = bot->GetSkillMax(skill);
#endif
    ObjectGuid guid = bot->GetObjectGuid();
    std::string data = "0";
    out << "|cFFFFFF00|Htrade:" << spellId << ":" << value << ":" << maxSkill << ":"
            << std::hex << std::uppercase << guid.GetRawValue()
            << std::nouppercase << std::dec << ":" << data
            << "|h[" << skillName << "]|h|r"
            << " |h|cff00ff00" << value << "|h|cffffffff/"
            << "|h|cff00ff00" << maxSkill << "|h|cffffffff ";

    return out.str();
}

std::string WhoAction::QuerySpec(std::string text)
{
    std::ostringstream out;

    int spec = AiFactory::GetPlayerSpecTab(bot);
    out << "|h|cffffffff" << chat->formatClass(bot, spec);
    out << " (|h|cff00ff00" << (uint32)bot->GetLevel() << "|h|cffffffff lvl), ";
    out << "|h|cff00ff00" << ai->GetEquipGearScore(bot, false, false) << "|h|cffffffff GS (";

    ItemCountByQuality visitor;
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);

    bool needSlash = false;
    if (visitor.count[ITEM_QUALITY_LEGENDARY])
    {
        out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_LEGENDARY] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_EPIC])
    {
        out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_EPIC] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_RARE])
    {
        if (needSlash) out << "/";
        out << "|h|cff8080ff" << visitor.count[ITEM_QUALITY_RARE] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_UNCOMMON])
    {
        if (needSlash) out << "/";
        out << "|h|cff00ff00" << visitor.count[ITEM_QUALITY_UNCOMMON] << "|h|cffffffff";
        needSlash = true;
    }

    out << ")";

    return out.str();
}
