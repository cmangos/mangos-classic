#include "playerbot/playerbot.h"
#include "PaladinStrategy.h"

using namespace ai;

class PaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    PaladinStrategyActionNodeFactory()
    {
        creators["cleanse poison"] = &cleanse_poison;
        creators["cleanse disease"] = &cleanse_disease;
        creators["cleanse poison on party"] = &cleanse_poison_on_party;
        creators["cleanse disease on party"] = &cleanse_disease_on_party;
        creators["judgement of wisdom"] = &judgement_of_wisdom;
        creators["divine shield"] = &divine_shield;
        creators["flash of light"] = &flash_of_light;
        creators["flash of light on party"] = &flash_of_light_on_party;
        creators["holy wrath"] = &holy_wrath;
        creators["lay on hands"] = &lay_on_hands;
        creators["lay on hands on party"] = &lay_on_hands_on_party;
        creators["greater blessing of might"] = &greater_blessing_of_might;
        creators["greater blessing of kings"] = &greater_blessing_of_kings;
        creators["greater blessing of wisdom"] = &greater_blessing_of_wisdom;
        creators["greater blessing of sanctuary"] = &greater_blessing_of_sanctuary;
        creators["greater blessing of might on party"] = &greater_blessing_of_might_on_party;
        creators["greater blessing of kings on party"] = &greater_blessing_of_kings_on_party;
        creators["greater blessing of wisdom on party"] = &greater_blessing_of_wisdom_on_party;
    }

private:
    ACTION_NODE_A(greater_blessing_of_might, "greater blessing of might", "blessing of might");

    ACTION_NODE_A(greater_blessing_of_kings, "greater blessing of kings", "blessing of kings");

    ACTION_NODE_A(greater_blessing_of_wisdom, "greater blessing of wisdom", "blessing of wisdom");

    ACTION_NODE_A(greater_blessing_of_sanctuary, "greater blessing of sanctuary", "blessing of sanctuary");

    ACTION_NODE_A(greater_blessing_of_might_on_party, "greater blessing of might on party", "blessing of might on party");

    ACTION_NODE_A(greater_blessing_of_kings_on_party, "greater blessing of kings on party", "blessing of kings on party");

    ACTION_NODE_A(greater_blessing_of_wisdom_on_party, "greater blessing of wisdom on party", "blessing of wisdom on party");

    static ActionNode* lay_on_hands(PlayerbotAI* ai)
    {
        return new ActionNode("lay on hands",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("divine shield"), 
                                       new NextAction("flash of light"), NULL),
            /*C*/ NULL);
    }

    ACTION_NODE_A(lay_on_hands_on_party, "lay on hands on party", "holy light on party");

    ACTION_NODE_A(cleanse_poison, "cleanse poison", "purify poison");

    ACTION_NODE_A(cleanse_disease, "cleanse disease", "purify disease");

    ACTION_NODE_A(cleanse_poison_on_party, "cleanse poison on party", "purify poison on party");

    ACTION_NODE_A(cleanse_disease_on_party, "cleanse disease on party", "purify disease on party");

    ACTION_NODE_A(judgement_of_wisdom, "judgement of wisdom", "judgement of light");

    ACTION_NODE_A(divine_shield, "divine shield", "divine protection");

    ACTION_NODE_A(flash_of_light, "flash of light", "holy light");

    ACTION_NODE_A(flash_of_light_on_party, "flash of light on party", "holy light on party");

    ACTION_NODE_A(holy_wrath, "holy wrath", "consecration");
};

PaladinStrategy::PaladinStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new PaladinStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void PaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("lay on hands", ACTION_EMERGENCY + 2), NULL)));
}

void PaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("redemption", ACTION_EMERGENCY), NULL)));
}

void PaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice interrupt",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        NextAction::array(0, new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        NextAction::array(0, new NextAction("hammer of justice on snare target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "turn undead",
        NextAction::array(0, new NextAction("turn undead", ACTION_INTERRUPT), NULL)));
}

void PaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void PaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    
}

void PaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("paladin aura", ACTION_NORMAL), NULL)));
}

void PaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pvp greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pvp blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pvp greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pvp blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pve greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pve blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pve greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pve blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    
}

void PaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("raid greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("raid blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("raid greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("raid blessing", ACTION_NORMAL), NULL)));
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void PaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("lay on hands", ACTION_EMERGENCY + 2), NULL)));
}

void PaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("redemption", ACTION_EMERGENCY), NULL)));
}

void PaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice interrupt",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        NextAction::array(0, new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        NextAction::array(0, new NextAction("hammer of justice on snare target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "turn undead",
        NextAction::array(0, new NextAction("turn undead", ACTION_INTERRUPT), NULL)));
}

void PaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 1), NULL)));
}

void PaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("paladin aura", ACTION_NORMAL), NULL)));
}

void PaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pvp greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pvp blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pvp greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pvp blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pve greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pve blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pve greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pve blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("raid greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("raid blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("raid greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("raid blessing", ACTION_NORMAL), NULL)));
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void PaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("lay on hands", ACTION_EMERGENCY + 2), NULL)));
}

void PaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array(0, new NextAction("redemption", ACTION_EMERGENCY), NULL)));
}

void PaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void PaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void PaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void PaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void PaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void PaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void PaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void PaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void PaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void PaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void PaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rooted",
        NextAction::array(0, new NextAction("blessing of freedom", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member rooted",
        NextAction::array(0, new NextAction("blessing of freedom on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL), NULL)));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL), NULL)));
}

void PaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void PaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "hammer of justice interrupt",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        NextAction::array(0, new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        NextAction::array(0, new NextAction("hammer of justice on snare target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "turn undead",
        NextAction::array(0, new NextAction("turn undead", ACTION_INTERRUPT), NULL)));
}

void PaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void PaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 1), NULL)));
}

void PaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply stone", ACTION_NORMAL), NULL)));
}

void PaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    OffhealStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));
}

void PaladinOffhealPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPvpStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPvpStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealPveStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealPveStrategy::InitNonCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitCombatTriggers(triggers);
    OffhealRaidStrategy::InitCombatTriggers(triggers);
}

void PaladinOffhealRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinOffhealStrategy::InitNonCombatTriggers(triggers);
    OffhealRaidStrategy::InitNonCombatTriggers(triggers);
}

void PaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("paladin aura", ACTION_NORMAL), NULL)));
}

void PaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pvp greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pvp blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pvp greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pvp blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("pve greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("pve blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("pve greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("pve blessing", ACTION_NORMAL), NULL)));
}

void PaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void PaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater blessing on party",
        NextAction::array(0, new NextAction("raid greater blessing on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("raid blessing on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater blessing",
        NextAction::array(0, new NextAction("raid greater blessing", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("raid blessing", ACTION_NORMAL), NULL)));
}

#endif

void PaladinManualAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_NORMAL), NULL)));
}

void PaladinManualBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "greater " + triggerName + " on party",
        NextAction::array(0, new NextAction("greater " + actionName + " on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        triggerName + " on party",
        NextAction::array(0, new NextAction(actionName + " on party", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "greater " + triggerName,
        NextAction::array(0, new NextAction("greater " + actionName, ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_NORMAL), NULL)));
}