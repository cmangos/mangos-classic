
#include "playerbot/playerbot.h"
#include "PaladinMultipliers.h"
#include "ProtectionPaladinStrategy.h"

using namespace ai;

class ProtectionPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ProtectionPaladinStrategyActionNodeFactory()
    {
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["judgement"] = &judgement;
    }

private:
    ACTION_NODE_A(seal_of_vengeance, "seal of vengeance", "seal of righteousness");

    ACTION_NODE_A(hand_of_reckoning, "hand of reckoning", "righteous defense");

    ACTION_NODE_A(judgement, "judgement", "exorcism");
};

ProtectionPaladinStrategy::ProtectionPaladinStrategy(PlayerbotAI* ai) : PaladinStrategy(ai)
{
    actionNodeFactories.Add(new ProtectionPaladinStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ProtectionPaladinStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("hand of reckoning", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "holy shield",
        NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("seal of wisdom", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "exorcism",
        NextAction::array(0, new NextAction("exorcism", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of vengeance", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement",
        NextAction::array(0, new NextAction("judgement", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void ProtectionPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "consecration",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH), NULL)));
}

void ProtectionPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array(0, new NextAction("righteous fury", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("stoneshield potion", ACTION_HIGH), NULL)));
}

void ProtectionPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("retribution aura", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** ProtectionPaladinStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);  

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("hand of reckoning", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "holy shield",
        NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("seal of wisdom", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "exorcism",
        NextAction::array(0, new NextAction("exorcism", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of vengeance", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement",
        NextAction::array(0, new NextAction("judgement", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void ProtectionPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "consecration",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "avenger's shield",
        NextAction::array(0, new NextAction("avenger's shield", ACTION_HIGH), NULL)));
}

void ProtectionPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array(0, new NextAction("righteous fury", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("retribution aura", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ProtectionPaladinStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("hand of reckoning", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "holy shield",
        NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("judgement of wisdom", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("seal of wisdom", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "exorcism",
        NextAction::array(0, new NextAction("exorcism", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of vengeance", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement of light",
        NextAction::array(0, new NextAction("judgement of light", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "set glyph::Hammer of Justice",
        NextAction::array(0, new NextAction("set glyph::Hammer of Justice", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "set glyph::the Wise",
        NextAction::array(0, new NextAction("set glyph::the Wise", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), NULL)));
}

void ProtectionPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("hammer of the righteous", ACTION_HIGH + 3),
                             new NextAction("avenger's shield", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "melee light aoe",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "consecration",
        NextAction::array(0, new NextAction("consecration", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "avenger's shield",
        NextAction::array(0, new NextAction("avenger's shield", ACTION_HIGH), NULL)));
}

void ProtectionPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array(0, new NextAction("righteous fury", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void ProtectionPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void ProtectionPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("retribution aura", ACTION_NORMAL + 1), NULL)));
}

void ProtectionPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
