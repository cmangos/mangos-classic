#include "playerbot/playerbot.h"
#include "SuggestWhatToDoAction.h"
#include "playerbot/AiFactory.h"
#include "Chat/ChannelMgr.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotTextMgr.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/ItemVisitors.h"

using namespace ai;

std::map<std::string, int> SuggestWhatToDoAction::instances;
std::map<std::string, int> SuggestWhatToDoAction::factions;

SuggestWhatToDoAction::SuggestWhatToDoAction(PlayerbotAI* ai, std::string name)
    : Action{ ai, name }
    , _locale{ sConfig.GetIntDefault("DBC.Locale", 0 /*LocaleConstant::LOCALE_enUS*/) }
{
    // -- In case we're using auto detect on config file^M
    if (_locale == 255)
        _locale = static_cast<int32>(LocaleConstant::LOCALE_enUS);

    suggestions.push_back(&SuggestWhatToDoAction::instance);
    suggestions.push_back(&SuggestWhatToDoAction::specificQuest);
    suggestions.push_back(&SuggestWhatToDoAction::grindMaterials);
    suggestions.push_back(&SuggestWhatToDoAction::grindReputation);
    suggestions.push_back(&SuggestWhatToDoAction::something);
    suggestions.push_back(&SuggestWhatToDoAction::somethingToxic);
    suggestions.push_back(&SuggestWhatToDoAction::toxicLinks);
    suggestions.push_back(&SuggestWhatToDoAction::thunderfury);
}

bool SuggestWhatToDoAction::isUseful()
{
    if (!sRandomPlayerbotMgr.IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId())
        return false;

    std::string qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}

bool SuggestWhatToDoAction::Execute(Event& event)
{
    int index = rand() % suggestions.size();
    (this->*suggestions[index])();

    std::string qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    ai->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(time(0) + urand(1, 60));

    return true;
}

void SuggestWhatToDoAction::instance()
{
    if (instances.empty())
    {
        instances["Ragefire Chasm"] = 15;
        instances["Deadmines"] = 18;
        instances["Wailing Caverns"] = 18;
        instances["Shadowfang Keep"] = 25;
        instances["Blackfathom Deeps"] = 20;
        instances["Stockade"] = 20;
        instances["Gnomeregan"] = 35;
        instances["Razorfen Kraul"] = 35;
        instances["Maraudon"] = 50;
        instances["Scarlet Monastery"] = 40;
        instances["Uldaman"] = 45;
        instances["Dire Maul"] = 58;
        instances["Scholomance"] = 59;
        instances["Razorfen Downs"] = 40;
        instances["Stratholme"] = 59;
        instances["Zul'Farrak"] = 45;
        instances["Blackrock Depths"] = 55;
        instances["Temple of Atal'Hakkar"] = 55;
        instances["Lower Blackrock Spire"] = 57;

        instances["Hellfire Citadel"] = 65;
        instances["Coilfang Reservoir"] = 65;
        instances["Auchindoun"] = 65;
        instances["Cavens of Time"] = 68;
        instances["Tempest Keep"] = 69;
        instances["Magister's Terrace"] = 70;

        instances["Utgarde Keep"] = 75;
        instances["The Nexus"] = 75;
        instances["Ahn'kahet: The Old Kingdom"] = 75;
        instances["Azjol-Nerub"] = 75;
        instances["Drak'Tharon Keep"] = 75;
        instances["Violet Hold"] = 80;
        instances["Gundrak"] = 77;
        instances["Halls of Stone"] = 77;
        instances["Halls of Lightning"] = 77;
        instances["Oculus"] = 77;
        instances["Utgarde Pinnacle"] = 77;
        instances["Trial of the Champion"] = 80;
        instances["Forge of Souls"] = 80;
        instances["Pit of Saron"] = 80;
        instances["Halls of Reflection"] = 80;
    }

    std::vector<std::string> allowedInstances;
    for (std::map<std::string, int>::iterator i = instances.begin(); i != instances.end(); ++i)
    {
        if ((int)bot->GetLevel() >= i->second) allowedInstances.push_back(i->first);
    }

    if (allowedInstances.empty()) return;

    BroadcastHelper::BroadcastSuggestInstance(ai, allowedInstances, bot);
}

std::vector<uint32> SuggestWhatToDoAction::GetIncompletedQuests()
{
    std::vector<uint32> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
            result.push_back(questId);
    }

    return result;
}

void SuggestWhatToDoAction::specificQuest()
{
    std::vector<uint32> quests = GetIncompletedQuests();
    if (quests.empty())
        return;

    BroadcastHelper::BroadcastSuggestQuest(ai, quests, bot);
}

void SuggestWhatToDoAction::grindMaterials()
{
    if (bot->GetLevel() <= 5)
        return;

    auto vec = ItemUsageValue::GetAllReagentItemIdsForCraftingSkillsVector();

    if (vec.size() > 0)
    {
        uint32 randomItemId = vec[urand() % vec.size()];

        const ItemPrototype* proto = ObjectMgr::GetItemPrototype(randomItemId);
        if (proto)
        {
            BroadcastHelper::BroadcastSuggestGrindMaterials(ai, ai->GetChatHelper()->formatItem(proto), bot);
        }
    }

    return;
}

void SuggestWhatToDoAction::grindReputation()
{
    if (factions.empty())
    {
        factions["Argent Dawn"] = 60;
        factions["Bloodsail Buccaneers"] = 40;
        factions["Brood of Nozdormu"] = 60;
        factions["Cenarion Circle"] = 55;
        factions["Darkmoon Faire"] = 20;
        factions["Hydraxian Waterlords"] = 60;
        factions["Ravenholdt"] = 20;
        factions["Thorium Brotherhood"] = 40;
        factions["Timbermaw Hold"] = 50;
        factions["Wintersaber Trainers"] = 50;
        factions["Booty Bay"] = 30;
        factions["Everlook"] = 40;
        factions["Gadgetzan"] = 50;
        factions["Ratchet"] = 20;

#ifndef MANGOSBOT_ZERO
        factions["Ashtongue Deathsworn"] = 70;
        factions["Cenarion Expedition"] = 62;
        factions["The Consortium"] = 65;
        factions["Honor Hold"] = 66;
        factions["Keepers of Time"] = 68;
        factions["Netherwing"] = 65;
        factions["Ogri'la"] = 65;
        factions["The Scale of the Sands"] = 65;
        factions["Sporeggar"] = 65;
        factions["Tranquillien"] = 10;
        factions["The Violet Eye"] = 70;
#endif

#ifdef MANGOSBOT_TWO
        factions["Argent Crusade"] = 75;
        factions["Ashen Verdict"] = 75;
        factions["The Kalu'ak"] = 72;
        factions["Kirin Tor"] = 75;
        factions["Knights of the Ebon Blade"] = 77;
        factions["The Sons of Hodir"] = 78;
        factions["The Wyrmrest Accord"] = 77;
#endif
    }

    std::vector<std::string> levels;
    levels.push_back("honored");
    levels.push_back("revered");
    levels.push_back("exalted");

    std::vector<std::string> allowedFactions;
    for (std::map<std::string, int>::iterator i = factions.begin(); i != factions.end(); ++i) {
        if ((int)bot->GetLevel() >= i->second) allowedFactions.push_back(i->first);
    }

    if (allowedFactions.empty()) return;

    BroadcastHelper::BroadcastSuggestGrindReputation(ai, levels, allowedFactions, bot);
}

void SuggestWhatToDoAction::something()
{
    BroadcastHelper::BroadcastSuggestSomething(ai, bot);
}

void SuggestWhatToDoAction::somethingToxic()
{
    BroadcastHelper::BroadcastSuggestSomethingToxic(ai, bot);
}

void SuggestWhatToDoAction::toxicLinks()
{
    BroadcastHelper::BroadcastSuggestToxicLinks(ai, bot);
}

void SuggestWhatToDoAction::thunderfury()
{
    BroadcastHelper::BroadcastSuggestThunderfury(ai, bot);
}

class FindTradeItemsVisitor : public IterateItemsVisitor
{
public:
    FindTradeItemsVisitor(uint32 quality) : quality(quality), IterateItemsVisitor() {}

    virtual bool Visit(Item* item)
    {
        ItemPrototype const* proto = item->GetProto();
        if (proto->Quality != quality)
            return true;

        if (proto->Class == ITEM_CLASS_TRADE_GOODS && proto->Bonding == NO_BIND)
        {
            if(proto->Quality == ITEM_QUALITY_NORMAL && item->GetCount() > 1 && item->GetCount() == item->GetMaxStackCount())
                stacks.push_back(proto->ItemId);

            items.push_back(proto->ItemId);
            count[proto->ItemId] += item->GetCount();
        }

        return true;
    }

    std::map<uint32, int > count;
    std::vector<uint32> stacks;
    std::vector<uint32> items;

private:
    uint32 quality;
};

bool SuggestTradeAction::isUseful()
{
    if (!sRandomPlayerbotMgr.IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId())
        return false;

    return true;
}

bool SuggestTradeAction::Execute(Event& event)
{
    uint32 quality = urand(0, 100);
    if (quality > 95)
        quality = ITEM_QUALITY_LEGENDARY;
    else if (quality > 90)
        quality = ITEM_QUALITY_EPIC;
    else if (quality > 75)
        quality = ITEM_QUALITY_RARE;
    else if (quality > 50)
        quality = ITEM_QUALITY_UNCOMMON;
    else
        quality = ITEM_QUALITY_NORMAL;

    uint32 item = 0, count = 0;
    while (quality-- > ITEM_QUALITY_POOR)
    {
        FindTradeItemsVisitor visitor(quality);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (!visitor.stacks.empty())
        {
            int index = urand(0, visitor.stacks.size() - 1);
            item = visitor.stacks[index];
        }

        if (!item)
        {
            if (!visitor.items.empty())
            {
                int index = urand(0, visitor.items.size() - 1);
                item = visitor.items[index];
            }
        }

        if (item)
        {
            count = visitor.count[item];
            break;
        }
    }

    if (!item || !count)
        return false;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(item);
    if (!proto)
        return false;

    uint32 price = ItemUsageValue::GetBotSellPrice(proto, bot) * count;
    if (!price)
        return false;

    BroadcastHelper::BroadcastSuggestSell(ai, proto, count, price, bot);

    return true;
}