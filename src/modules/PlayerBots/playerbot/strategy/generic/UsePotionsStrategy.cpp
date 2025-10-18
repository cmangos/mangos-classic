
#include "playerbot/playerbot.h"
#include "UsePotionsStrategy.h"

using namespace ai;

class UsePotionsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UsePotionsStrategyActionNodeFactory()
    {
        creators["healthstone"] = &healthstone;
        creators["healing potion"] = &healing_potion;
        creators["dark rune"] = &dark_rune;
    }

private:
    ACTION_NODE_A(healthstone, "healthstone", "healing potion");

    ACTION_NODE_A(healing_potion, "healing potion", "use bandage");

    ACTION_NODE_A(dark_rune, "dark rune", "mana potion");
};

UsePotionsStrategy::UsePotionsStrategy(PlayerbotAI* ai) : Strategy(ai)
{
    actionNodeFactories.Add(new UsePotionsStrategyActionNodeFactory());
}

void UsePotionsStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healthstone", ACTION_CRITICAL_HEAL + 1), NULL)));

#ifdef MANGOSBOT_ZERO
    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("whipper root tuber", ACTION_CRITICAL_HEAL), NULL)));
#endif

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("use bandage", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dark rune", ACTION_HIGH), NULL)));
}

void UsePotionsStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}
