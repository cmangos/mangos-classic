#pragma once
#include "playerbot/strategy/Value.h"
#include "PartyMemberValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class FriendlyUnitWithoutAuraValue : public PartyMemberValue, public Qualified
    {
    public:
        FriendlyUnitWithoutAuraValue(PlayerbotAI* ai, std::string name = "friendly unit without aura", float range = sPlayerbotAIConfig.sightDistance) :
            PartyMemberValue(ai, name), Qualified() {}

    protected:
        virtual Unit* Calculate();
    };

    class PartyMemberWithoutAuraValue : public PartyMemberValue, public Qualified
	{
	public:
        PartyMemberWithoutAuraValue(PlayerbotAI* ai, std::string name = "party member without aura", float range = sPlayerbotAIConfig.sightDistance) :
          PartyMemberValue(ai, name), Qualified() {}

    protected:
        virtual Unit* Calculate();
	};

    class PartyMemberWithoutMyAuraValue : public PartyMemberValue, public Qualified
    {
    public:
        PartyMemberWithoutMyAuraValue(PlayerbotAI* ai, std::string name = "party member without my aura", float range = 30.0f) :
            PartyMemberValue(ai, name), Qualified() {}

    protected:
        virtual Unit* Calculate();
    };

    class PartyTankWithoutAuraValue : public PartyMemberValue, public Qualified
    {
    public:
        PartyTankWithoutAuraValue(PlayerbotAI* ai, std::string name = "party tank without aura", float range = 30.0f) :
            PartyMemberValue(ai, name), Qualified() {}

    protected:
        virtual Unit* Calculate();
    };
}
