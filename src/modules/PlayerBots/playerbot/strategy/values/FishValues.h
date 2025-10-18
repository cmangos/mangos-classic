#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/values/LootValues.h"
#include "playerbot/WorldSquare.h"

namespace ai
{    
    /*                uint32 zone, subzone;
    gameObject->GetZoneAndAreaId(zone, subzone);
    // if subzone loot exist use it
    if (!FillLoot(subzone, LootTemplates_Fishing, player, true, (subzone != zone)) && subzone != zone)
        // else use zone loot (if zone diff. from subzone, must exist in like case)
        FillLoot(zone, LootTemplates_Fishing, player, true);
        */

    //LootTemplate const* tab = LootTemplates_Fishing.GetLootFor(loot_id);

    /*
    class FishLocation
    public:

    private:
        WorldPosition waterPos, landPos;
    };

    class FishZone : public MapWpSquare
    {
    public:
    private:
        uint32 zone;
        uint32 reqSkill;
        std::vector<uint32> loot;
        //LootTemplate const* tab;
    };

    class FishLocationMapValue : public CalculatedValue<FishZone>
	{
	public:
        FishLocationMapValue(PlayerbotAI* ai) : GuidPositionCalculatedValue(ai, "fish location map", 5) {};
        virtual GuidPosition Calculate() override;
    };   
    */

    class CanFishValue : public BoolCalculatedValue
    {
    public:
        CanFishValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fish", 60) {};
        virtual bool Calculate() override;
    };

    class CanOpenFishingDobberValue : public BoolCalculatedValue
    {
    public:
        CanOpenFishingDobberValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can open fishing dobber") {};
        virtual bool Calculate() override;
    };

    class DoneFishingValue : public BoolCalculatedValue
    {
    public:
        DoneFishingValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "done fishing") {};
        virtual bool Calculate() override;
    };
};

