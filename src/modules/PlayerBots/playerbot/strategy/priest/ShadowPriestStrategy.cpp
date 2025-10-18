#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "ShadowPriestStrategy.h"

using namespace ai;

class ShadowPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShadowPriestStrategyActionNodeFactory()
    {
        creators["dispersion"] = &dispersion;
    }

private:
    ACTION_NODE_A(dispersion, "dispersion", "mana potion");
};

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* ai) : PriestStrategy(ai)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ShadowPriestStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("mind flay", ACTION_IDLE), NULL);
}

void ShadowPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "mind blast",
        NextAction::array(0, new NextAction("mind blast", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric embrace",
        NextAction::array(0, new NextAction("vampiric embrace", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "devouring plague",
        NextAction::array(0, new NextAction("devouring plague", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "hex of weakness",
        NextAction::array(0, new NextAction("hex of weakness", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void ShadowPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));
}

void ShadowPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadowform",
        NextAction::array(0, new NextAction("shadowform", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "feedback",
        NextAction::array(0, new NextAction("feedback", ACTION_HIGH), NULL)));
}

void ShadowPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "silence",
        NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "silence on enemy healer",
        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShadowPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** ShadowPriestStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("mind flay", ACTION_IDLE), NULL);
}

void ShadowPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadow word: death", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "mind blast",
        NextAction::array(0, new NextAction("mind blast", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric embrace",
        NextAction::array(0, new NextAction("vampiric embrace", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "devouring plague",
        NextAction::array(0, new NextAction("devouring plague", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "hex of weakness",
        NextAction::array(0, new NextAction("hex of weakness", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric touch",
        NextAction::array(0, new NextAction("vampiric touch", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void ShadowPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric touch on attacker",
        NextAction::array(0, new NextAction("vampiric touch on attacker", ACTION_HIGH), NULL)));
}

void ShadowPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadowform",
        NextAction::array(0, new NextAction("shadowform", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "feedback",
        NextAction::array(0, new NextAction("feedback", ACTION_HIGH), NULL)));
}

void ShadowPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "silence",
        NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "silence on enemy healer",
        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShadowPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ShadowPriestStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("mind flay", ACTION_IDLE), NULL);
}

void ShadowPriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadow word: death", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "mind blast",
        NextAction::array(0, new NextAction("mind blast", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric embrace",
        NextAction::array(0, new NextAction("vampiric embrace", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "devouring plague",
        NextAction::array(0, new NextAction("devouring plague", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "hex of weakness",
        NextAction::array(0, new NextAction("hex of weakness", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric touch",
        NextAction::array(0, new NextAction("vampiric touch", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void ShadowPriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PriestStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPveStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPveStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestPvpStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestPvpStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitCombatTriggers(triggers);
    PriestRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitNonCombatTriggers(triggers);
    PriestRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitReactionTriggers(triggers);
    PriestRaidStrategy::InitReactionTriggers(triggers);
}

void ShadowPriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestStrategy::InitDeadTriggers(triggers);
    PriestRaidStrategy::InitDeadTriggers(triggers);
}

void ShadowPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric touch on attacker",
        NextAction::array(0, new NextAction("vampiric touch on attacker", ACTION_HIGH), NULL)));
}

void ShadowPriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestAoeStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestAoeStrategy::InitNonCombatTriggers(triggers);
    PriestAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadowform",
        NextAction::array(0, new NextAction("shadowform", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "feedback",
        NextAction::array(0, new NextAction("feedback", ACTION_HIGH), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dispersion", ACTION_EMERGENCY + 5), NULL)));
    */
}

void ShadowPriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBuffStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBuffStrategy::InitNonCombatTriggers(triggers);
    PriestBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestBoostStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestBoostStrategy::InitNonCombatTriggers(triggers);
    PriestBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "silence",
        NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "silence on enemy healer",
        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShadowPriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCcStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcPvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitCombatTriggers(triggers);
    PriestCcRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCcStrategy::InitNonCombatTriggers(triggers);
    PriestCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestCureStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePveStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCurePvpStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitCombatTriggers(triggers);
    PriestCureRaidStrategy::InitCombatTriggers(triggers);
}

void ShadowPriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShadowPriestCureStrategy::InitNonCombatTriggers(triggers);
    PriestCureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif