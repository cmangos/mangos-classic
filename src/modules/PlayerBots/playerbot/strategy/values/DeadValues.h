#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    enum DeadValueConstants
    {
        DEATH_COUNT_BEFORE_EVAC = 15,
        DEATH_COUNT_BEFORE_TRYING_ANOTHER_GRAVEYARD = 10,
        DEATH_COUNT_BEFORE_REVIVING_AT_SPIRIT_HEALER = 8
    };

    class GraveyardValue : public GuidPositionCalculatedValue, public Qualified
    {
    public:
        GraveyardValue(PlayerbotAI* ai, std::string name = "graveyard", int checkInterval = 10) : GuidPositionCalculatedValue(ai, name, checkInterval), Qualified() {}

    public:
        GuidPosition Calculate();
        WorldSafeLocsEntry const* GetAnotherAppropriateClosestGraveyard() const;
    };

    class BestGraveyardValue : public GuidPositionCalculatedValue
    {
    public:
        BestGraveyardValue(PlayerbotAI* ai, std::string name = "best graveyard", int checkInterval = 10) : GuidPositionCalculatedValue(ai, name, checkInterval) {}

    public:
        GuidPosition Calculate();
    };

    class ShouldSpiritHealerValue : public BoolCalculatedValue
    {
    public:
        ShouldSpiritHealerValue(PlayerbotAI* ai, std::string name = "should spirit healer") : BoolCalculatedValue(ai, name) {}

    public:
        bool Calculate();
    };
}
