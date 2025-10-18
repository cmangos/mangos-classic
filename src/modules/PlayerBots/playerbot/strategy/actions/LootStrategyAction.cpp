
#include "playerbot/playerbot.h"
#include "LootStrategyAction.h"
#include "playerbot/strategy/values/SubStrategyValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "LootAction.h"
#include "playerbot/PlayerbotAIAware.h"

using namespace ai;

bool LootStrategyAction::Execute(Event& event)
{
    std::string strategy = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    LootObjectStack* lootItems = AI_VALUE(LootObjectStack*, "available loot");
    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    std::set<uint32>& skipLootItems = AI_VALUE(std::set<uint32>&, "skip loot list");
    std::set<uint32>& skipGoLootList = AI_VALUE(std::set<uint32>&, "skip go loot list");

    if (strategy == "?")
    {
        {
            std::ostringstream out;
            out << "Loot strategy: ";
            out << AI_VALUE(std::string, "loot strategy");
            ai->TellPlayer(requester, out);
        }

        TellLootList(requester, "always loot list");
        TellLootList(requester, "skip loot list");
        TellGoList(requester, "skip go loot list");
    }
    else if (strategy == "clear")
    {
        alwaysLootItems.clear();
        skipLootItems.clear();
        ai->TellPlayer(requester, "My loot list is now empty");
        return true;
    }    
    else
    {
        std::set<std::string> itemQualifiers = chat->parseItemQualifiers(strategy);
        std::list<ObjectGuid> gos = chat->parseGameobjects(strategy);

        if (itemQualifiers.size() == 0 && gos.size() == 0)
        {
            SET_AI_VALUE(std::string, "loot strategy", strategy);

            std::string lootStrategy = AI_VALUE(std::string, "loot strategy");

            std::ostringstream out;
            out << "Loot strategy set to " << lootStrategy;
            ai->TellPlayer(requester, out);
            return true;
        }

        bool ignore = strategy.size() > 1 && strategy.substr(0, 1) == "!";
        bool remove = strategy.size() > 1 && strategy.substr(0, 1) == "-";
        bool query = strategy.size() > 1 && strategy.substr(0, 1) == "?";
        bool add = !ignore && !remove && !query;
        bool changes = false;
        for (auto& qualifier : itemQualifiers)
        {
            ItemQualifier itemQualifier(qualifier);
            auto itemid = itemQualifier.GetId();
            if (query)
            {
                if (itemQualifier.GetProto())
                {
                    std::ostringstream out;
                    out << (StoreLootAction::IsLootAllowed(itemQualifier, ai) ? "|cFF000000Will loot " : "|c00FF0000Won't loot ") << ChatHelper::formatItem(itemQualifier);
                    ai->TellPlayer(requester, out.str());
                }
            }
            
            if (remove || add)
            {
                std::set<uint32>::iterator j = skipLootItems.find(itemid);
                if (j != skipLootItems.end()) skipLootItems.erase(j);
                changes = true;
            }
            
            if (remove || ignore)
            {
                std::set<uint32>::iterator j = alwaysLootItems.find(itemid);
                if (j != alwaysLootItems.end()) alwaysLootItems.erase(j);
                changes = true;
            }
            
            if (ignore)
            {
                skipLootItems.insert(itemid);
                changes = true;
            }

            if (add)
            {
                alwaysLootItems.insert(itemid);
                changes = true;
            }
        }
            
        for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); ++i)
        {
            GameObject *go = ai->GetGameObject(*i);
            if (!go) continue;
            uint32 goId = go->GetGOInfo()->id;

            if (remove || add)
            {
                std::set<uint32>::iterator j = skipGoLootList.find(goId);
                if (j != skipGoLootList.end()) skipGoLootList.erase(j);
                changes = true;
            }

            if (ignore)
            {
                skipGoLootList.insert(goId);
                changes = true;
            }
        }

        if (changes)
        {
            TellLootList(requester, "always loot list");
            TellLootList(requester, "skip loot list");
            TellGoList(requester, "skip go loot list");
            AI_VALUE(LootObjectStack*, "available loot")->Clear();
        }
    }

    return true;
}

void LootStrategyAction::TellLootList(Player* requester, const std::string& name)
{
    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, name);
    std::ostringstream out;
    out << "My " << name << ":";

    for (std::set<uint32>::iterator i = alwaysLootItems.begin(); i != alwaysLootItems.end(); i++)
    {
        ItemPrototype const *proto = sItemStorage.LookupEntry<ItemPrototype>(*i);
        if (!proto)
        {
            continue;
        }

        out << " " << chat->formatItem(proto);
    }
    
    ai->TellPlayer(requester, out);
}

void LootStrategyAction::TellGoList(Player* requester, const std::string& name)
{
    std::set<uint32>& skipGoItems = AI_VALUE(std::set<uint32>&, name);
    std::ostringstream out;
    out << "My " << name << ":";

    for (std::set<uint32>::iterator i = skipGoItems.begin(); i != skipGoItems.end(); i++)
    {
        uint32 id = *i;
        GameObjectInfo const *proto = sGOStorage.LookupEntry<GameObjectInfo>(id);
        if (!proto)
            continue;

        out << " |cFFFFFF00|Hfound:" << 0 << ":" << id << ":" <<  "|h[" << proto->name << "]|h|r";
    }
    ai->TellPlayer(requester, out);
}
