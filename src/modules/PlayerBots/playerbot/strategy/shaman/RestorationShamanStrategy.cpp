
#include "playerbot/playerbot.h"
#include "ShamanMultipliers.h"
#include "RestorationShamanStrategy.h"

using namespace ai;

class RestorationShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RestorationShamanStrategyActionNodeFactory()
    {
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["mana tide totem"] = &mana_tide_totem;
        creators["searing totem"] = &searing_totem;
        creators["magma totem"] = &magma_totem;
        creators["strength of earth totem"] = &strength_of_earth_totem;
        creators["windfury"] = &windfury_totem;
        creators["healing stream totem"] = &healing_stream_totem;
        creators["riptide"] = &riptide;
        creators["riptide on party"] = &riptide_on_party;
        creators["chain heal"] = &chain_heal;
        creators["chain heal on party"] = &chain_heal_on_party;
    }

private:
    ACTION_NODE_A(earthliving_weapon, "earthliving weapon", "flametongue weapon");

    ACTION_NODE_A(mana_tide_totem, "mana tide totem", "mana potion");

    ACTION_NODE_A(searing_totem, "searing totem", "flametongue totem");

    ACTION_NODE_C(magma_totem, "magma totem", "fire nova");

    ACTION_NODE_A(strength_of_earth_totem, "strength of earth totem", "stoneskin totem");

    ACTION_NODE_A(windfury_totem, "windfury totem", "wrath of air totem");

    ACTION_NODE_A(healing_stream_totem, "healing stream totem", "mana spring totem");

    ACTION_NODE_A(riptide, "riptide", "healing wave");

    ACTION_NODE_A(riptide_on_party, "riptide on party", "healing wave on party");

    ACTION_NODE_A(chain_heal, "chain heal", "lesser healing wave");

    ACTION_NODE_A(chain_heal_on_party, "chain heal on party", "lesser healing wave on party");
};

RestorationShamanStrategy::RestorationShamanStrategy(PlayerbotAI* ai) : ShamanStrategy(ai)
{
    actionNodeFactories.Add(new RestorationShamanStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void RestorationShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana tide totem", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("riptide", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("riptide on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("chain heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        NextAction::array(0, new NextAction("healing stream totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("searing totem", ACTION_HIGH), NULL)));
}

void RestorationShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void RestorationShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void RestorationShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana tide totem", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("riptide", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("riptide on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("chain heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        NextAction::array(0, new NextAction("healing stream totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("flametongue totem", ACTION_HIGH), NULL)));
}

void RestorationShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);

        triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "earth shield on party tank",
        NextAction::array(0, new NextAction("earth shield on party tank", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "earth shield on party tank",
        NextAction::array(0, new NextAction("earth shield on party tank", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void RestorationShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void RestorationShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana tide totem", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("riptide", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("riptide on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("chain heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healing wave", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("lesser healing wave", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("lesser healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void RestorationShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPveStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPveStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanPvpStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanPvpStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitCombatTriggers(triggers);
    ShamanRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitNonCombatTriggers(triggers);
    ShamanRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitReactionTriggers(triggers);
    ShamanRaidStrategy::InitReactionTriggers(triggers);
}

void RestorationShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanStrategy::InitDeadTriggers(triggers);
    ShamanRaidStrategy::InitDeadTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanAoeStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanAoeStrategy::InitNonCombatTriggers(triggers);
    ShamanAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCcStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCcStrategy::InitNonCombatTriggers(triggers);
    ShamanCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanCureStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanCureStrategy::InitNonCombatTriggers(triggers);
    ShamanCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
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
        NextAction::array(0, new NextAction("healing stream totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("flametongue totem", ACTION_HIGH), NULL)));
}

void RestorationShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanTotemsStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanTotemsStrategy::InitNonCombatTriggers(triggers);
    ShamanTotemsRaidStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water shield",
        NextAction::array(0, new NextAction("water shield", ACTION_NORMAL), NULL)));
}

void RestorationShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "earth shield on party tank",
        NextAction::array(0, new NextAction("earth shield on party tank", ACTION_HIGH), NULL)));
}

void RestorationShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBuffStrategy::InitNonCombatTriggers(triggers);
    ShamanBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "earth shield on party tank",
        NextAction::array(0, new NextAction("earth shield on party tank", ACTION_NORMAL), NULL)));
}

void RestorationShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanBoostStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitCombatTriggers(triggers);
}

void RestorationShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    RestorationShamanBoostStrategy::InitNonCombatTriggers(triggers);
    ShamanBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif