
#include "Category.h"
#include "playerbot/RandomItemMgr.h"
#include "ItemBag.h"
#include "AhBotConfig.h"
#include "PricingStrategy.h"
#include "playerbot/ServerFacade.h"

using namespace ahbot;

uint32 Category::GetStackCount(ItemPrototype const* proto)
{
    if (proto->Quality > ITEM_QUALITY_UNCOMMON)
        return 1;

    double rarity = GetPricingStrategy()->GetRarityPriceMultiplier(proto->ItemId);
    return (uint32)std::max(1.0, proto->GetMaxStackSize() / rarity);
}

PricingStrategy* Category::GetPricingStrategy()
{
    if (pricingStrategy)
        return pricingStrategy;

    std::ostringstream out; out << "AhBot.PricingStrategy." << GetName();
    std::string name = sAhBotConfig.GetStringDefault(out.str().c_str(), "default");
    return pricingStrategy = PricingStrategyFactory::Create(name, this);
}

QualityCategoryWrapper::QualityCategoryWrapper(Category* category, uint32 quality) : Category(), quality(quality), category(category)
{
    std::ostringstream out; out << category->GetName() << ".";
    switch (quality)
    {
    case ITEM_QUALITY_POOR:
        out << "gray";
        break;
    case ITEM_QUALITY_NORMAL:
        out << "white";
        break;
    case ITEM_QUALITY_UNCOMMON:
        out << "green";
        break;
    case ITEM_QUALITY_RARE:
        out << "blue";
        break;
    default:
        out << "epic";
        break;
    }

    combinedName = out.str();
}

bool QualityCategoryWrapper::Contains(ItemPrototype const* proto)
{
    return proto->Quality == quality && category->Contains(proto);
}


