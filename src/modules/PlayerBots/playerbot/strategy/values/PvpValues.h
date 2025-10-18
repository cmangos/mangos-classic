#pragma once

#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/AiObjectContext.h"

namespace ai
{
    class BgTypeValue : public ManualSetValue<uint32>
    {
    public:
        BgTypeValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "bg type") {}
    };

    class ArenaTypeValue : public ManualSetValue<uint32>
    {
    public:
        ArenaTypeValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "arena type") {}
    };

    class BgRoleValue : public ManualSetValue<uint32>
    {
    public:
        BgRoleValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "bg role") {}
    };

    class BgMastersValue : public SingleCalculatedValue<std::list<CreatureDataPair const*>>, public Qualified
    {
    public:
        BgMastersValue(PlayerbotAI* ai) : SingleCalculatedValue<std::list<CreatureDataPair const*>>(ai, "bg masters"), Qualified() {}
        virtual std::list<CreatureDataPair const*> Calculate();
    };

    class BgMasterValue : public CDPairCalculatedValue, public Qualified
    {
    public:
        BgMasterValue(PlayerbotAI* ai) : CDPairCalculatedValue(ai, "bg master", 60), Qualified() {}
        virtual CreatureDataPair const* Calculate();
        virtual CreatureDataPair const* NearestBm(bool allowDead = true);
    };

    class RpgBgTypeValue : public CalculatedValue<BattleGroundTypeId>
    {
    public:
        RpgBgTypeValue(PlayerbotAI* ai) : CalculatedValue(ai, "rpg bg type") {}

        virtual BattleGroundTypeId Calculate();
    };

    class FlagCarrierValue : public UnitCalculatedValue
    {
    public:
        FlagCarrierValue(PlayerbotAI* ai, bool sameTeam = false, bool ignoreRange = false) :
            UnitCalculatedValue(ai), sameTeam(sameTeam), ignoreRange(ignoreRange) {}

        virtual Unit* Calculate();

    private:
        bool sameTeam;
        bool ignoreRange;
    };
}
