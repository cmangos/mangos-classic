
#include "playerbot/playerbot.h"
#include "ProtectionWarriorStrategy.h"

using namespace ai;

class ProtectionWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ProtectionWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["sunder armor"] = &sunder_armor;
        creators["commanding shout"] = &commanding_shout;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
        creators["heroic throw taunt"] = &heroic_throw_taunt;
        creators["taunt"] = &taunt;
        creators["berserker rage fear"] = &berserker_rage_fear;
    }

private:
    ACTION_NODE_A(charge, "charge", "reach melee");

    ACTION_NODE_A(sunder_armor, "sunder armor", "melee");

    ACTION_NODE_A(commanding_shout, "commanding shout", "battle shout");

    ACTION_NODE_A(devastate, "devastate", "sunder armor");

    ACTION_NODE_A(last_stand, "last stand", "intimidating shout");

    ACTION_NODE_A(heroic_throw_on_snare_target, "heroic throw on snare target", "taunt on snare target");

    ACTION_NODE_A(heroic_throw_taunt, "heroic throw", "taunt");

    ACTION_NODE_A(taunt, "taunt", "battle shout taunt");

    ACTION_NODE_A(berserker_rage_fear, "berserker rage", "death wish");
};

ProtectionWarriorStrategy::ProtectionWarriorStrategy(PlayerbotAI* ai) : WarriorStrategy(ai)
{
    actionNodeFactories.Add(new ProtectionWarriorStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

NextAction** ProtectionWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("shield wall", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 8), new NextAction("charge", ACTION_MOVE + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("berserker stance", ACTION_MOVE + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("intercept", ACTION_MOVE + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("heroic throw taunt", ACTION_MOVE + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "demoralizing shout",
        NextAction::array(0, new NextAction("demoralizing shout", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "shield block",
        NextAction::array(0, new NextAction("shield block", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "sunder armor",
        NextAction::array(0, new NextAction("devastate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "revenge",
        NextAction::array(0, new NextAction("revenge", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "light rage available",
        NextAction::array(0, new NextAction("shield slam", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "bloodthirst",
        NextAction::array(0, new NextAction("bloodthirst", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "mortal strike",
        NextAction::array(0, new NextAction("mortal strike", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "disarm",
        NextAction::array(0, new NextAction("disarm", ACTION_NORMAL), NULL)));
}

void ProtectionWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void ProtectionWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging shout", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("battle shout taunt", ACTION_HIGH), NULL)));
}

void ProtectionWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "defensive stance",
        NextAction::array(0, new NextAction("defensive stance", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "feared",
        NextAction::array(0, new NextAction("berserker rage fear", ACTION_INTERRUPT), NULL)));
}

void ProtectionWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("stoneshield potion", ACTION_HIGH), NULL)));
}

void ProtectionWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "concussion blow",
        NextAction::array(0, new NextAction("concussion blow", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ProtectionWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

NextAction** ProtectionWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("shield wall", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("intervene", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 8), new NextAction("charge", ACTION_MOVE + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("berserker stance", ACTION_MOVE + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("intercept", ACTION_MOVE + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("heroic throw taunt", ACTION_MOVE + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "spell reflection",
        NextAction::array(0, new NextAction("spell reflection", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "demoralizing shout",
        NextAction::array(0, new NextAction("demoralizing shout", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "shield block",
        NextAction::array(0, new NextAction("shield block", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "sunder armor",
        NextAction::array(0, new NextAction("devastate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "light rage available",
        NextAction::array(0, new NextAction("shield slam", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "revenge",
        NextAction::array(0, new NextAction("revenge", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "disarm",
        NextAction::array(0, new NextAction("disarm", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "devastate",
        NextAction::array(0, new NextAction("devastate", ACTION_NORMAL), NULL)));
}

void ProtectionWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));
}

void ProtectionWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging shout", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("battle shout taunt", ACTION_HIGH), NULL)));
}

void ProtectionWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "defensive stance",
        NextAction::array(0, new NextAction("defensive stance", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "commanding shout",
        NextAction::array(0, new NextAction("commanding shout", ACTION_HIGH), NULL)));
}

void ProtectionWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "concussion blow",
        NextAction::array(0, new NextAction("concussion blow", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT), NULL)));
}

void ProtectionWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

NextAction** ProtectionWarriorStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_IDLE), NULL);
}

void ProtectionWarriorStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("shield wall", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "has blessing of salvation",
        NextAction::array(0, new NextAction("remove blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "has greater blessing of salvation",
        NextAction::array(0, new NextAction("remove greater blessing of salvation", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("intervene", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 8), new NextAction("charge", ACTION_MOVE + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("berserker stance", ACTION_MOVE + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("intercept", ACTION_MOVE + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("heroic throw taunt", ACTION_MOVE + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "spell reflection",
        NextAction::array(0, new NextAction("spell reflection", ACTION_MOVE + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "demoralizing shout",
        NextAction::array(0, new NextAction("demoralizing shout", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "sword and board",
        NextAction::array(0, new NextAction("shield slam", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "shield block",
        NextAction::array(0, new NextAction("shield block", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "light rage available",
        NextAction::array(0, new NextAction("shield slam", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "sunder armor",
        NextAction::array(0, new NextAction("devastate", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "heroic strike",
        NextAction::array(0, new NextAction("heroic strike", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "revenge",
        NextAction::array(0, new NextAction("revenge", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "disarm",
        NextAction::array(0, new NextAction("disarm", ACTION_NORMAL), NULL)));
}

void ProtectionWarriorStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPveStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPveStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorPvpStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorPvpStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitCombatTriggers(triggers);
    WarriorRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitNonCombatTriggers(triggers);
    WarriorRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitReactionTriggers(triggers);
    WarriorRaidStrategy::InitReactionTriggers(triggers);
}

void ProtectionWarriorRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorStrategy::InitDeadTriggers(triggers);
    WarriorRaidStrategy::InitDeadTriggers(triggers);
}

void ProtectionWarriorAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("challenging shout", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("battle shout taunt", ACTION_HIGH), NULL)));
}

void ProtectionWarriorAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorAoeStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorAoeStrategy::InitNonCombatTriggers(triggers);
    WarriorAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "defensive stance",
        NextAction::array(0, new NextAction("defensive stance", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "commanding shout",
        NextAction::array(0, new NextAction("commanding shout", ACTION_HIGH), NULL)));
}

void ProtectionWarriorBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBuffStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBuffStrategy::InitNonCombatTriggers(triggers);
    WarriorBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorBoostStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorBoostStrategy::InitNonCombatTriggers(triggers);
    WarriorBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "concussion blow",
        NextAction::array(0, new NextAction("concussion blow", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shockwave on snare target",
        NextAction::array(0, new NextAction("shockwave on snare target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shockwave",
        NextAction::array(0, new NextAction("shockwave", ACTION_INTERRUPT), NULL)));
}

void ProtectionWarriorCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarriorCcStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPveStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPveStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitCombatTriggers(triggers);
}

void ProtectionWarriorCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ProtectionWarriorCcStrategy::InitNonCombatTriggers(triggers);
    WarriorCcRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif