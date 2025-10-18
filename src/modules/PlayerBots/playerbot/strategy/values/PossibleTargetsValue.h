#pragma once
#include "playerbot/strategy/Value.h"
#include "NearestUnitsValue.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    // List of hostile and neutral targets in a range around the bot
    class PossibleTargetsValue : public NearestUnitsValue, public Qualified
	{
	public:
        PossibleTargetsValue(PlayerbotAI* ai, std::string name = "possible targets", float range = sPlayerbotAIConfig.sightDistance, bool ignoreLos = false) :
          NearestUnitsValue(ai, name, range, ignoreLos), Qualified() {}

        std::list<ObjectGuid> Calculate() override;
        static bool IsValid(Unit* target, Player* player, bool ignoreLos = false);

    protected:
        virtual void FindUnits(std::list<Unit*> &targets) override;
        virtual bool AcceptUnit(Unit* unit) override;

        static void FindPossibleTargets(Player* player, std::list<Unit*>& targets, float range);
        static bool IsFriendly(Unit* target, Player* player);
        static bool IsAttackable(Unit* target, Player* player);
	};

    class AllTargetsValue : public PossibleTargetsValue
	{
	public:
        AllTargetsValue(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance) :
        PossibleTargetsValue(ai, "all targets", range, true) {}
	};
}
