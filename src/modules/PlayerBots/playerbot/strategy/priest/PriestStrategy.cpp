#include "playerbot/playerbot.h"
#include "PriestStrategy.h"

using namespace ai;

class PriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    PriestStrategyActionNodeFactory()
    {
        creators["abolish disease"] = &abolish_disease;
        creators["abolish disease on party"] = &abolish_disease_on_party;
        creators["greater heal"] = &greater_heal;
        creators["greater heal on party"] = &greater_heal_on_party;
        creators["heal"] = &heal;
        creators["heal on party"] = &heal_on_party;
        creators["flash heal"] = &flash_heal;
        creators["flash heal on party"] = &flash_heal_on_party;
        creators["renew"] = &renew;
        creators["renew on party"] = &renew_on_party;
        creators["resurrection"] = &resurrection;
    }

private:
    ACTION_NODE_A(abolish_disease, "abolish disease", "cure disease");

    ACTION_NODE_A(abolish_disease_on_party, "abolish disease on party", "cure disease on party");

    static ActionNode* greater_heal(PlayerbotAI* ai)
    {
        return new ActionNode("greater heal",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("heal"), NULL),
            /*C*/ NULL);
    }

    static ActionNode* greater_heal_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("greater heal on party",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("heal on party"), NULL),
            /*C*/ NULL);
    }

    static ActionNode* heal(PlayerbotAI* ai)
    {
        return new ActionNode("heal",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("lesser heal"), NULL),
            /*C*/ NULL);
    }

    static ActionNode* heal_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("heal on party",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("lesser heal on party"), NULL),
            /*C*/ NULL);
    }

    static ActionNode* flash_heal(PlayerbotAI* ai)
    {
        return new ActionNode("flash heal",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("greater heal"), NULL),
            /*C*/ NULL);
    }

    static ActionNode* flash_heal_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("flash heal on party",
            /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
            /*A*/ NextAction::array(0, new NextAction("greater heal on party"), NULL),
            /*C*/ NULL);
    }

    ACTION_NODE_P(renew, "renew", "remove shadowform");

    ACTION_NODE_P(renew_on_party, "renew on party", "remove shadowform");

    ACTION_NODE_P(resurrection, "resurrection", "remove shadowform");
};

PriestStrategy::PriestStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new PriestStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void PriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("desperate prayer", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("flash heal", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("greater heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH), NULL)));
}

void PriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("resurrection", ACTION_EMERGENCY), NULL)));
}

void PriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("fade", ACTION_DISPEL), NULL)));
}

void PriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shackle undead",
        NextAction::array(0, new NextAction("shackle undead", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("psychic scream", ACTION_INTERRUPT), NULL)));
}

void PriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("elune's grace", ACTION_HIGH + 3), NULL)));
}

void PriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "prayer of fortitude on party",
        NextAction::array(0, new NextAction("prayer of fortitude on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of spirit on party",
        NextAction::array(0, new NextAction("prayer of spirit on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of shadow protection on party",
        NextAction::array(0, new NextAction("prayer of shadow protection on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude on party",
        NextAction::array(0, new NextAction("power word: fortitude on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit on party",
        NextAction::array(0, new NextAction("divine spirit on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection on party",
        NextAction::array(0, new NextAction("shadow protection on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude",
        NextAction::array(0, new NextAction("power word: fortitude", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit",
        NextAction::array(0, new NextAction("divine spirit", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection",
        NextAction::array(0, new NextAction("shadow protection", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "inner fire",
        NextAction::array(0, new NextAction("inner fire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "touch of weakness",
        NextAction::array(0, new NextAction("touch of weakness", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowguard",
        NextAction::array(0, new NextAction("shadowguard", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "swimming",
        NextAction::array(0, new NextAction("levitate", ACTION_NORMAL), NULL)));
    */
}

void PriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "power infusion",
        NextAction::array(0, new NextAction("power infusion", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "inner focus",
        NextAction::array(0, new NextAction("inner focus", ACTION_HIGH), NULL)));
}

void PriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));
}

void PriestOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PriestOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitCombatTriggers(triggers);

    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "shadow word: pain on attacker",
            NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));

        triggers.push_back(new TriggerNode(
            "melee medium aoe",
            NextAction::array(0, new NextAction("holy nova", ACTION_HIGH), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "holy fire",
        NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("starshards", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "smite",
        NextAction::array(0, new NextAction("smite", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void PriestOffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void PriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("desperate prayer", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("flash heal", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("greater heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH), NULL)));
}

void PriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("resurrection", ACTION_EMERGENCY), NULL)));
}

void PriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("fade", ACTION_DISPEL), NULL)));
}

void PriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "chastise",
        NextAction::array(0, new NextAction("chastise", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shackle undead",
        NextAction::array(0, new NextAction("shackle undead", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("psychic scream", ACTION_INTERRUPT), NULL)));
}

void PriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("elune's grace", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("symbol of hope", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), NULL)));
}

void PriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "prayer of fortitude on party",
        NextAction::array(0, new NextAction("prayer of fortitude on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of spirit on party",
        NextAction::array(0, new NextAction("prayer of spirit on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of shadow protection on party",
        NextAction::array(0, new NextAction("prayer of shadow protection on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude on party",
        NextAction::array(0, new NextAction("power word: fortitude on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit on party",
        NextAction::array(0, new NextAction("divine spirit on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection on party",
        NextAction::array(0, new NextAction("shadow protection on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude",
        NextAction::array(0, new NextAction("power word: fortitude", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit",
        NextAction::array(0, new NextAction("divine spirit", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection",
        NextAction::array(0, new NextAction("shadow protection", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "inner fire",
        NextAction::array(0, new NextAction("inner fire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "touch of weakness",
        NextAction::array(0, new NextAction("touch of weakness", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowguard",
        NextAction::array(0, new NextAction("shadowguard", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "swimming",
        NextAction::array(0, new NextAction("levitate", ACTION_NORMAL), NULL)));
    */
}

void PriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "inner focus",
        NextAction::array(0, new NextAction("inner focus", ACTION_HIGH), NULL)));
}

void PriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));
}

void PriestOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PriestOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitCombatTriggers(triggers);

    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "shadow word: pain on attacker",
            NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));

        triggers.push_back(new TriggerNode(
            "melee medium aoe",
            NextAction::array(0, new NextAction("holy nova", ACTION_HIGH), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "holy fire",
        NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("starshards", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "smite",
        NextAction::array(0, new NextAction("smite", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void PriestOffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void PriestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("desperate prayer", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("flash heal", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("greater heal", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH), NULL)));
}

void PriestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("resurrection", ACTION_EMERGENCY), NULL)));
}

void PriestStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PriestStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PriestPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PriestPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PriestPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PriestPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PriestPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PriestPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PriestRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("fade", ACTION_DISPEL), NULL)));
}

void PriestRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PriestRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PriestAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PriestAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PriestAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PriestAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array(0, new NextAction("dispel magic", ACTION_DISPEL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array(0, new NextAction("dispel magic on party", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array(0, new NextAction("abolish disease", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array(0, new NextAction("abolish disease on party", ACTION_DISPEL), NULL)));
}

void PriestCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PriestCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PriestCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shackle undead",
        NextAction::array(0, new NextAction("shackle undead", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy five yards",
        NextAction::array(0, new NextAction("psychic scream", ACTION_INTERRUPT), NULL)));
}

void PriestCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PriestCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PriestCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PriestCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "target of attacker",
        NextAction::array(0, new NextAction("elune's grace", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), NULL)));
}

void PriestBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "prayer of fortitude on party",
        NextAction::array(0, new NextAction("prayer of fortitude on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of spirit on party",
        NextAction::array(0, new NextAction("prayer of spirit on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "prayer of shadow protection on party",
        NextAction::array(0, new NextAction("prayer of shadow protection on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude on party",
        NextAction::array(0, new NextAction("power word: fortitude on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit on party",
        NextAction::array(0, new NextAction("divine spirit on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection on party",
        NextAction::array(0, new NextAction("shadow protection on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "power word: fortitude",
        NextAction::array(0, new NextAction("power word: fortitude", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "divine spirit",
        NextAction::array(0, new NextAction("divine spirit", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow protection",
        NextAction::array(0, new NextAction("shadow protection", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "inner fire",
        NextAction::array(0, new NextAction("inner fire", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fear ward",
        NextAction::array(0, new NextAction("fear ward", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "touch of weakness",
        NextAction::array(0, new NextAction("touch of weakness", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowguard",
        NextAction::array(0, new NextAction("shadowguard", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "swimming",
        NextAction::array(0, new NextAction("levitate", ACTION_NORMAL), NULL)));
    */
}

void PriestBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PriestBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "inner focus",
        NextAction::array(0, new NextAction("inner focus", ACTION_HIGH + 1), NULL)));
}

void PriestBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PriestBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PriestBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 1),
                             new NextAction("flash heal on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL), NULL)));
}

void PriestOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("greater heal on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PriestOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitCombatTriggers(triggers);

    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "shadow word: pain on attacker",
            NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_HIGH + 1), NULL)));

        triggers.push_back(new TriggerNode(
            "melee medium aoe",
            NextAction::array(0, new NextAction("holy nova", ACTION_HIGH), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "holy fire",
        NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("starshards", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "smite",
        NextAction::array(0, new NextAction("smite", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void PriestOffdpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffdpsStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPvpStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPvpStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsPveStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsPveStrategy::InitNonCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitCombatTriggers(triggers);
    OffdpsRaidStrategy::InitCombatTriggers(triggers);
}

void PriestOffdpsRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PriestOffdpsStrategy::InitNonCombatTriggers(triggers);
    OffdpsRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif