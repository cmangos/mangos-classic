
#include "playerbot/playerbot.h"
#include "HunterStrategy.h"
#include "HunterAiObjectContext.h"

using namespace ai;

class HunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HunterStrategyActionNodeFactory()
    {
        creators["aimed shot"] = &aimed_shot;
        creators["rapid fire"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["aspect of the dragonhawk"] = &aspect_of_the_dragonhawk;
        creators["wing clip"] = &wing_clip;
        creators["intimidation"] = &intimidation;
        creators["scatter shot"] = &scatter_shot;
        creators["feign death threat"] = &feign_death_threat;
    }

private:
    ACTION_NODE_A(aimed_shot, "aimed shot", "arcane shot");

    ACTION_NODE_A(rapid_fire, "rapid fire", "readiness");

    ACTION_NODE_A(aspect_of_the_pack, "aspect of the pack", "aspect of the cheetah");

    ACTION_NODE_A(aspect_of_the_dragonhawk, "aspect of the dragonhawk", "aspect of the hawk");

    ACTION_NODE_A(wing_clip, "wing clip", "raptor strike");

    ACTION_NODE_A(intimidation, "intimidation", "wing clip");

    ACTION_NODE_A(scatter_shot, "scatter shot", "intimidation");

    ACTION_NODE_C(feign_death_threat, "feign death", "remove feign death");
};

HunterStrategy::HunterStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new HunterStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** HunterStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("auto shot", ACTION_IDLE), NULL);
}

void HunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "switch to ranged",
        NextAction::array(0, new NextAction("switch to ranged", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "switch to melee",
        NextAction::array(0, new NextAction("switch to melee", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "counterattack",
        NextAction::array(0, new NextAction("counterattack", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hunter's mark",
        NextAction::array(0, new NextAction("hunter's mark", ACTION_NORMAL + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "aimed shot",
        NextAction::array(0, new NextAction("aimed shot", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("raptor strike", ACTION_NORMAL + 1), NULL)));
}

void HunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "no ammo",
        NextAction::array(0, new NextAction("say::no ammo", ACTION_NORMAL), NULL)));
}

void HunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel enrage",
        NextAction::array(0, new NextAction("tranquilizing shot", ACTION_INTERRUPT), NULL)));
}

void HunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void HunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void HunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void HunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void HunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("immolation trap on target", ACTION_INTERRUPT), NULL)));
}

void HunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void HunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void HunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("feign death threat", ACTION_EMERGENCY), NULL)));
}

void HunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void HunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void HunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "explosive trap",
        NextAction::array(0, new NextAction("explosive trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("volley", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "multi-shot",
        NextAction::array(0, new NextAction("multi-shot", ACTION_HIGH), NULL)));
}

void HunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("deterrence", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "feign death",
        NextAction::array(0, new NextAction("remove feign death", ACTION_HIGH), NULL)));
}

void HunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "feign death",
        NextAction::array(0, new NextAction("remove feign death", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rapid fire",
        NextAction::array(0, new NextAction("rapid fire", ACTION_HIGH + 5), NULL)));
}

void HunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "freezing trap",
        NextAction::array(0, new NextAction("freezing trap on cc", ACTION_INTERRUPT + 1), NULL)));

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "frost trap",
        NextAction::array(0, new NextAction("frost trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "scare beast",
        NextAction::array(0, new NextAction("scare beast on cc", ACTION_HIGH + 2), NULL)));
}

void HunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("freezing trap in place", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("scatter shot on closest attacker targeting me", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy fifteen yards",
        NextAction::array(0, new NextAction("concussive shot", ACTION_HIGH + 1), NULL)));
}

void HunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("scatter shot on closest attacker targeting me", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy fifteen yards",
        NextAction::array(0, new NextAction("concussive shot", ACTION_HIGH + 1), NULL)));
}

void HunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "serpent sting on attacker",
            NextAction::array(0, new NextAction("serpent sting on attacker", ACTION_HIGH + 2), NULL)));

        triggers.push_back(new TriggerNode(
            "viper sting on attacker",
            NextAction::array(0, new NextAction("viper sting on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("viper sting", ACTION_NORMAL + 5), NULL)));
}

void HunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the hawk",
        NextAction::array(0, new NextAction("aspect of the hawk", ACTION_HIGH + 5), NULL)));
}

void HunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the hawk",
        NextAction::array(0, new NextAction("aspect of the hawk", ACTION_NORMAL), NULL)));
}

void HunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** HunterStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("auto shot", ACTION_IDLE), NULL);
}

void HunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "switch to ranged",
        NextAction::array(0, new NextAction("switch to ranged", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "switch to melee",
        NextAction::array(0, new NextAction("switch to melee", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "counterattack",
        NextAction::array(0, new NextAction("counterattack", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hunter's mark",
        NextAction::array(0, new NextAction("hunter's mark", ACTION_NORMAL + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "kill command",
        NextAction::array(0, new NextAction("kill command", ACTION_NORMAL + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "steady shot",
        NextAction::array(0, new NextAction("steady shot", ACTION_NORMAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("raptor strike", ACTION_NORMAL + 1), NULL)));
}

void HunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "no ammo",
        NextAction::array(0, new NextAction("say::no ammo", ACTION_NORMAL), NULL)));
}

void HunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel enrage",
        NextAction::array(0, new NextAction("tranquilizing shot", ACTION_INTERRUPT), NULL)));
}

void HunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void HunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void HunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void HunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void HunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("immolation trap on target", ACTION_INTERRUPT), NULL)));
}

void HunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void HunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void HunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("feign death threat", ACTION_EMERGENCY), NULL)));
}

void HunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void HunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void HunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "explosive trap",
        NextAction::array(0, new NextAction("explosive trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged high aoe",
        NextAction::array(0, new NextAction("volley", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "multi-shot",
        NextAction::array(0, new NextAction("multi-shot", ACTION_HIGH), NULL)));
}

void HunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("deterrence", ACTION_EMERGENCY + 1), NULL)));
}

void HunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "feign death",
        NextAction::array(0, new NextAction("remove feign death", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rapid fire",
        NextAction::array(0, new NextAction("rapid fire", ACTION_HIGH + 5), NULL)));
}

void HunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("scatter shot on closest attacker targeting me", ACTION_INTERRUPT + 2), NULL)));

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "freezing trap",
        NextAction::array(0, new NextAction("freezing trap on cc", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "frost trap",
        NextAction::array(0, new NextAction("frost trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "scare beast",
        NextAction::array(0, new NextAction("scare beast on cc", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy fifteen yards",
        NextAction::array(0, new NextAction("concussive shot", ACTION_HIGH + 1), NULL)));
}

void HunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("freezing trap in place", ACTION_INTERRUPT + 3), NULL)));
}

void HunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void HunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "serpent sting on attacker",
            NextAction::array(0, new NextAction("serpent sting on attacker", ACTION_HIGH + 2), NULL)));

        triggers.push_back(new TriggerNode(
            "viper sting on attacker",
            NextAction::array(0, new NextAction("viper sting on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("viper sting", ACTION_NORMAL + 5), NULL)));
}

void HunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the hawk",
        NextAction::array(0, new NextAction("aspect of the hawk", ACTION_HIGH + 5), NULL)));
}

void HunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the hawk",
        NextAction::array(0, new NextAction("aspect of the hawk", ACTION_NORMAL), NULL)));
}

void HunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // TO DO: This needs to be done better, currently it spams unnecessarily
    /*
    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("aspect of the monkey", ACTION_EMERGENCY), NULL)));
    */

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** HunterStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("auto shot", ACTION_IDLE), NULL);
}

void HunterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "switch to ranged",
        NextAction::array(0, new NextAction("switch to ranged", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "switch to melee",
        NextAction::array(0, new NextAction("switch to melee", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "counterattack",
        NextAction::array(0, new NextAction("counterattack", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hunter's mark",
        NextAction::array(0, new NextAction("hunter's mark", ACTION_NORMAL + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "aimed shot",
        NextAction::array(0, new NextAction("aimed shot", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("raptor strike", ACTION_NORMAL + 1), NULL)));
}

void HunterStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "stealthed nearby",
        NextAction::array(0, new NextAction("flare", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "no ammo",
        NextAction::array(0, new NextAction("say::no ammo", ACTION_NORMAL), NULL)));
}

void HunterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel enrage",
        NextAction::array(0, new NextAction("tranquilizing shot", ACTION_INTERRUPT), NULL)));
}

void HunterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void HunterPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void HunterPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void HunterPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void HunterPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("immolation trap on target", ACTION_INTERRUPT), NULL)));
}

void HunterPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void HunterPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void HunterRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("feign death threat", ACTION_EMERGENCY), NULL)));
}

void HunterRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void HunterRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void HunterAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "explosive trap",
        NextAction::array(0, new NextAction("explosive trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("volley", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "multi-shot",
        NextAction::array(0, new NextAction("multi-shot", ACTION_HIGH), NULL)));
}

void HunterAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void HunterAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void HunterAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void HunterAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("deterrence", ACTION_HIGH + 7), NULL)));
}

void HunterBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "feign death",
        NextAction::array(0, new NextAction("remove feign death", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HunterBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void HunterBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rapid fire",
        NextAction::array(0, new NextAction("rapid fire", ACTION_HIGH + 5), NULL)));
}

void HunterBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void HunterBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void HunterBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("scatter shot on closest attacker targeting me", ACTION_INTERRUPT + 2), NULL)));

    // Traps must be on ACTION_INTERRUPT or higher due to its movements and chained actions
    triggers.push_back(new TriggerNode(
        "freezing trap",
        NextAction::array(0, new NextAction("freezing trap on cc", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "frost trap",
        NextAction::array(0, new NextAction("frost trap on target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "scare beast",
        NextAction::array(0, new NextAction("scare beast on cc", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy fifteen yards",
        NextAction::array(0, new NextAction("concussive shot", ACTION_HIGH + 1), NULL)));
}

void HunterCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("freezing trap in place", ACTION_INTERRUPT + 3), NULL)));
}

void HunterCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void HunterCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void HunterCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HunterStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "serpent sting on attacker",
            NextAction::array(0, new NextAction("serpent sting on attacker", ACTION_HIGH + 2), NULL)));

        triggers.push_back(new TriggerNode(
            "viper sting on attacker",
            NextAction::array(0, new NextAction("viper sting on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("viper sting", ACTION_NORMAL + 5), NULL)));
}

void HunterStingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction("serpent sting", ACTION_NORMAL + 4), NULL)));
}

void HunterStingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the dragonhawk",
        NextAction::array(0, new NextAction("aspect of the dragonhawk", ACTION_HIGH + 5), NULL)));
}

void HunterAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the dragonhawk",
        NextAction::array(0, new NextAction("aspect of the dragonhawk", ACTION_NORMAL), NULL)));
}

void HunterAspectPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // TO DO: This needs to be done better, currently it spams unnecessarily
    /*
    triggers.push_back(new TriggerNode(
        "target of attacker close",
        NextAction::array(0, new NextAction("aspect of the monkey", ACTION_EMERGENCY), NULL)));
    */

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("remove aspect of the cheetah", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("aspect of the cheetah", ACTION_HIGH + 6), NULL)));
}

void HunterAspectPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void HunterAspectRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif

void HunterManualStingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            triggerName + " on attacker",
            NextAction::array(0, new NextAction(actionName + " on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array(0, new NextAction(actionName, ACTION_HIGH), NULL)));
}

void HunterManualAspectStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_HIGH), NULL)));
}

void HunterManualAspectStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}

void HunterPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array(0, new NextAction("mend pet", ACTION_HIGH), NULL)));
}

void HunterPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("call pet", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "pet not happy",
        NextAction::array(0, new NextAction("feed pet", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array(0, new NextAction("mend pet", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hunters pet dead",
        NextAction::array(0, new NextAction("revive pet", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("initialize pet", ACTION_NORMAL + 1), NULL)));
}
