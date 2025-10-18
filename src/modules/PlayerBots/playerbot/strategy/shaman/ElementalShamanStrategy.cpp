
#include "playerbot/playerbot.h"
#include "ShamanMultipliers.h"
#include "ElementalShamanStrategy.h"

using namespace ai;

class ElementalShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ElementalShamanStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["totem of wrath"] = &totem_of_wrath;
        creators["searing totem"] = &searing_totem;
        creators["strength of earth totem"] = &strength_of_earth_totem;
        creators["windfury totem"] = &windfury_totem;
        creators["mana spring totem"] = &mana_spring_totem;
    }

private:
    ACTION_NODE_A(flametongue_weapon, "flametongue weapon", "windfury weapon");

    ACTION_NODE_A(totem_of_wrath, "totem of wrath", "searing totem");

    ACTION_NODE_A(searing_totem, "searing totem", "flametongue totem");

    ACTION_NODE_A(strength_of_earth_totem, "strength of earth totem", "stoneskin totem");

    ACTION_NODE_A(windfury_totem, "windfury totem", "wrath of air totem");
   
    ACTION_NODE_A(mana_spring_totem, "mana spring totem", "healing stream totem");
};

ElementalShamanStrategy::ElementalShamanStrategy(PlayerbotAI* ai) 
: ShamanStrategy(ai)
{
    actionNodeFactories.Add(new ElementalShamanStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ElementalShamanStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("lightning bolt", ACTION_IDLE), NULL);
}

void ElementalShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array(0, new NextAction("earth shock", ACTION_NORMAL), NULL)));
}

void ElementalShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "chain lightning",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("fire nova", ACTION_HIGH), NULL)));
}

void ElementalShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged light aoe",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH), NULL)));
}

void ElementalShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array(0, new NextAction("frost shock", ACTION_HIGH), NULL)));
}

void ElementalShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("windfury totem", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("wrath of air totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "water totem",
        NextAction::array(0, new NextAction("mana spring totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem aoe",
        NextAction::array(0, new NextAction("magma totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("totem of wrath", ACTION_HIGH), NULL)));
}

void ElementalShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "lightning shield",
        NextAction::array(0, new NextAction("lightning shield", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void ElementalShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** ElementalShamanStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("lightning bolt", ACTION_IDLE), NULL);
}

void ElementalShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array(0, new NextAction("earth shock", ACTION_NORMAL), NULL)));
}

void ElementalShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "chain lightning",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("fire nova", ACTION_HIGH), NULL)));
}

void ElementalShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged light aoe",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH), NULL)));
}

void ElementalShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array(0, new NextAction("frost shock", ACTION_HIGH), NULL)));
}

void ElementalShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("windfury totem", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("wrath of air totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "water totem",
        NextAction::array(0, new NextAction("mana spring totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem aoe",
        NextAction::array(0, new NextAction("searing totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("totem of wrath", ACTION_HIGH), NULL)));
}

void ElementalShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void ElementalShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ElementalShamanStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("lightning bolt", ACTION_IDLE), NULL);
}

void ElementalShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array(0, new NextAction("earth shock", ACTION_NORMAL), NULL)));
}

void ElementalShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "chain lightning",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("fire nova", ACTION_HIGH), NULL)));
}

void ElementalShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void ElementalShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void ElementalShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ranged light aoe",
        NextAction::array(0, new NextAction("chain lightning", ACTION_HIGH), NULL)));
}

void ElementalShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array(0, new NextAction("frost shock", ACTION_HIGH), NULL)));
}

void ElementalShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("windfury totem", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("wrath of air totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "water totem",
        NextAction::array(0, new NextAction("mana spring totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem aoe",
        NextAction::array(0, new NextAction("magma totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("totem of wrath", ACTION_HIGH), NULL)));
}

void ElementalShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_HIGH), NULL)));
}

void ElementalShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("flametongue weapon", ACTION_NORMAL), NULL)));
}

void ElementalShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ElementalShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ElementalShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif