
#include "playerbot/playerbot.h"
#include "ShamanMultipliers.h"
#include "RestorationShamanStrategy.h"

using namespace ai;

class ShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShamanStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["lesser healing wave"] = &lesser_healing_wave;
        creators["lesser healing wave on party"] = &lesser_healing_wave_on_party;
        creators["earth shock"] = &earth_shock;
        creators["wind shear"] = &wind_shear;
    }

private:
    ACTION_NODE_A(wind_shear, "wind shear", "earth shock");

    ACTION_NODE_A(earth_shock, "earth shock", "flame shock");

    ACTION_NODE_A(flametongue_weapon, "flametongue weapon", "frostbrand weapon");

    ACTION_NODE_A(frostbrand_weapon, "frostbrand weapon", "rockbiter weapon");

    ACTION_NODE_A(lesser_healing_wave, "lesser healing wave", "healing wave");

    ACTION_NODE_A(lesser_healing_wave_on_party, "lesser healing wave on party", "healing wave on party");
};

ShamanStrategy::ShamanStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new ShamanStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void ShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "purge",
        NextAction::array(0, new NextAction("purge", ACTION_DISPEL), NULL)));
}

void ShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("ancestral spirit", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void ShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void ShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void ShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void ShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void ShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void ShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void ShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void ShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void ShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "wind shear",
        NextAction::array(0, new NextAction("wind shear", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "wind shear on enemy healer",
        NextAction::array(0, new NextAction("wind shear on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water breathing",
        NextAction::array(0, new NextAction("water breathing", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking",
        NextAction::array(0, new NextAction("water walking", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water breathing on party",
        NextAction::array(0, new NextAction("water breathing on party", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking on party",
        NextAction::array(0, new NextAction("water walking on party", ACTION_NORMAL), NULL)));
}

void ShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemBarElementsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemBarAncestorsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemBarSpiritsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_ONE // TBC

void ShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "purge",
        NextAction::array(0, new NextAction("purge", ACTION_DISPEL), NULL)));
}

void ShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("ancestral spirit", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_LIGHT_HEAL), NULL)));
}

void ShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void ShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void ShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void ShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void ShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void ShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void ShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void ShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void ShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void ShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "wind shear",
        NextAction::array(0, new NextAction("wind shear", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "wind shear on enemy healer",
        NextAction::array(0, new NextAction("wind shear on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water breathing",
        NextAction::array(0, new NextAction("water breathing", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking",
        NextAction::array(0, new NextAction("water walking", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water breathing on party",
        NextAction::array(0, new NextAction("water breathing on party", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking on party",
        NextAction::array(0, new NextAction("water walking on party", ACTION_NORMAL), NULL)));
}

void ShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "heroism",
        NextAction::array(0, new NextAction("heroism", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "bloodlust",
        NextAction::array(0, new NextAction("bloodlust", ACTION_HIGH), NULL)));
}

void ShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemBarElementsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemBarAncestorsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemBarSpiritsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void ShamanStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "purge",
        NextAction::array(0, new NextAction("purge", ACTION_DISPEL), NULL)));

    if (sRandomPlayerbotMgr.IsRandomBot(ai->GetBot()))
    {
        triggers.push_back(new TriggerNode(
            "levelup",
            NextAction::array(0, new NextAction("set totembars on levelup", ACTION_HIGH), NULL)));
    }
}

void ShamanStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("ancestral spirit", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("healing wave", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_LIGHT_HEAL), NULL)));

    if (sRandomPlayerbotMgr.IsRandomBot(ai->GetBot()))
    {
        triggers.push_back(new TriggerNode(
            "levelup",
            NextAction::array(0, new NextAction("set totembars on levelup", ACTION_HIGH), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "totemic recall",
        NextAction::array(0, new NextAction("totemic recall", ACTION_NORMAL), NULL)));
}

void ShamanStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void ShamanStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void ShamanPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("ghost wolf", ACTION_HIGH), NULL)));
}

void ShamanPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void ShamanPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void ShamanPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void ShamanPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void ShamanPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void ShamanRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void ShamanRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void ShamanAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void ShamanAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "wind shear",
        NextAction::array(0, new NextAction("wind shear", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "wind shear on enemy healer",
        NextAction::array(0, new NextAction("wind shear on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ShamanCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void ShamanCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit poison",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit disease",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit curse",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit poison",
        NextAction::array(0, new NextAction("cleanse spirit poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit disease",
        NextAction::array(0, new NextAction("cleanse spirit disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit curse",
        NextAction::array(0, new NextAction("cleanse spirit curse on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("cure disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("cure disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit poison",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit disease",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse spirit curse",
        NextAction::array(0, new NextAction("cleanse spirit", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit poison",
        NextAction::array(0, new NextAction("cleanse spirit poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit disease",
        NextAction::array(0, new NextAction("cleanse spirit disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit curse",
        NextAction::array(0, new NextAction("cleanse spirit curse on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("cure poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("cure poison on party", ACTION_DISPEL), NULL)));
}

void ShamanCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void ShamanCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing wave on party", ACTION_MEDIUM_HEAL), NULL)));
}

void ShamanOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ShamanOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanTotemsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void ShamanBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "water breathing",
        NextAction::array(0, new NextAction("water breathing", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking",
        NextAction::array(0, new NextAction("water walking", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water breathing on party",
        NextAction::array(0, new NextAction("water breathing on party", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "water walking on party",
        NextAction::array(0, new NextAction("water walking on party", ACTION_NORMAL), NULL)));
}

void ShamanBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "heroism",
        NextAction::array(0, new NextAction("heroism", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "bloodlust",
        NextAction::array(0, new NextAction("bloodlust", ACTION_HIGH), NULL)));
}

void ShamanBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void ShamanBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ShamanTotemBarElementsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "call of the elements",
        NextAction::array(0, new NextAction("call of the elements", ACTION_HIGH), NULL)));
}

void ShamanTotemBarAncestorsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "call of the ancestors",
        NextAction::array(0, new NextAction("call of the ancestors", ACTION_HIGH), NULL)));
}

void ShamanTotemBarSpiritsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "call of the ancestors",
        NextAction::array(0, new NextAction("call of the spirits", ACTION_HIGH), NULL)));
}

#endif

void ShamanManualTotemStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_HIGH), NULL)));
}