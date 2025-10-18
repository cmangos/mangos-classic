
#include "playerbot/playerbot.h"
#include "FishValues.h"

using namespace ai;

bool CanFishValue::Calculate()
{
    if (!bot->GetSkill(SKILL_FISHING, false, false)) //Unable to fish.
        return false;

    std::list<Item*> poles = AI_VALUE2(std::list<Item*>, "inventory items", "fishing pole");

    if (poles.empty()) //No fishing pole.
        return false;

    return true;
}

bool CanOpenFishingDobberValue::Calculate()
{
    if (!bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    std::string spellName = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->SpellName[0];
    if (spellName.find("Fishing") != 0)
        return false;

    return true;
}

bool DoneFishingValue::Calculate()
{
    if (!bot->GetSkill(SKILL_FISHING, false, false)) //Unable to fish.
        return false;

    Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    //Does not have fishing pole equiped.
    if (!mhItem || mhItem->GetProto()->Class != ITEM_CLASS_WEAPON || mhItem->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        std::string spellName = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->SpellName[0];
        if (spellName.find("Fishing") == 0)
            return false;
    }

    return true;
}


