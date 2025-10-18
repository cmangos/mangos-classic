
#include "playerbot/playerbot.h"
#include "BuffAction.h"

#include "playerbot/strategy/values/ItemCountValue.h"

using namespace ai;

class FindBuffVisitor : public IterateItemsVisitor {
public:
    FindBuffVisitor(Player* bot) : IterateItemsVisitor(), bot(bot)
    {
    }

    virtual bool Visit(Item* item)
    {
        if (bot->CanUseItem(item->GetProto()) != EQUIP_ERR_OK)
            return true;

        const ItemPrototype* proto = item->GetProto();

        if (proto->Class != ITEM_CLASS_CONSUMABLE)
            return true;

        if (proto->SubClass != ITEM_SUBCLASS_ELIXIR && 
            proto->SubClass != ITEM_SUBCLASS_FLASK &&
            proto->SubClass != ITEM_SUBCLASS_SCROLL && 
            proto->SubClass != ITEM_SUBCLASS_FOOD &&
            proto->SubClass != ITEM_SUBCLASS_CONSUMABLE_OTHER &&
            proto->SubClass != ITEM_SUBCLASS_ITEM_ENHANCEMENT)
            return true;

        for (int i=0; i<MAX_ITEM_PROTO_SPELLS; i++)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            if (bot->HasAura(spellId))
                return true;

            Item* itemForSpell = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<Item*>("item for spell", spellId);
            if (itemForSpell && itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                return true;
        
            if (items.find(proto->SubClass) == items.end())
                items[proto->SubClass] = std::list<Item*>();

            items[proto->SubClass].push_back(item);
            break;
        }

        return true;
    }

public:
    std::map<uint32, std::list<Item*> > items;

private:
    Player* bot;
};

void BuffAction::TellHeader(uint32 subClass, Player* requester)
{
    switch (subClass)
    {
    case ITEM_SUBCLASS_ELIXIR:
        ai->TellPlayer(requester, "--- Elixir ---");
        return;
    case ITEM_SUBCLASS_FLASK:
        ai->TellPlayer(requester, "--- Flask ---");
        return;
    case ITEM_SUBCLASS_SCROLL:
        ai->TellPlayer(requester, "--- Scroll ---");
        return;
    case ITEM_SUBCLASS_FOOD:
        ai->TellPlayer(requester, "--- Food ---");
        return;
    case ITEM_SUBCLASS_ITEM_ENHANCEMENT:
        ai->TellPlayer(requester, "--- Enchant ---");
        return;
    }
}

bool BuffAction::Execute(Event& event)
{
    std::string text = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    FindBuffVisitor visitor(bot);
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    uint32 oldSubClass = -1;
    for (std::map<uint32, std::list<Item*> >::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        std::list<Item*> items = i->second;

        uint32 subClass = i->first;
        if (oldSubClass != subClass)
        {
            if (!items.empty())
            {
                TellHeader(subClass, requester);
            }

            oldSubClass = subClass;
        }
        for (std::list<Item*>::iterator j = items.begin(); j != items.end(); ++j)
        {
            Item* item = *j;
            std::ostringstream out;
            out << chat->formatItem(item, item->GetCount());
            ai->TellPlayer(requester, out);
        }
    }
    
    return true;
}
