#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "PossibleTargetsValue.h"

namespace ai
{
    // List of enemy player targets that are near the bot (or bot group)
    class EnemyPlayersValue : public ObjectGuidListCalculatedValue, public Qualified
    {
    public:
        EnemyPlayersValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "enemy players"), Qualified() {}
        std::list<ObjectGuid> Calculate();

        static bool IsValid(Unit* target, Player* player);

    private:
        void ApplyFilter(std::list<ObjectGuid>& targets, bool getOne);
    };

    class HasEnemyPlayersValue : public BoolCalculatedValue, public Qualified
    {
    public:
        HasEnemyPlayersValue(PlayerbotAI* ai, std::string name = "has enemy players") : BoolCalculatedValue(ai, name, 3), Qualified() {}
        virtual bool Calculate();
    };

    class EnemyPlayerValue : public UnitCalculatedValue
    {
    public:
        EnemyPlayerValue(PlayerbotAI* ai, std::string name = "enemy player") : UnitCalculatedValue(ai, name) {}
        virtual Unit* Calculate();

        static float GetMaxAttackDistance(Player* bot);
    };
}
