
#include "playerbot/playerbot.h"
#include "WarlockMultipliers.h"
#include "WarlockStrategy.h"

using namespace ai;

class WarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarlockStrategyActionNodeFactory()
    {
        creators["banish"] = &banish;
        creators["fel armor"] = &fel_armor;
        creators["demon armor"] = &demon_armor;
    }

private:
    ACTION_NODE_A(banish, "banish", "fear");

    ACTION_NODE_A(fel_armor, "fel armor", "demon armor");

    ACTION_NODE_A(demon_armor, "demon armor", "demon skin");
};

WarlockStrategy::WarlockStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new WarlockStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void WarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("sacrifice", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "life tap",
        NextAction::array(0, new NextAction("life tap", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow trance",
        NextAction::array(0, new NextAction("shadow bolt", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "drain soul",
        NextAction::array(0, new NextAction("drain soul", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "corruption",
        NextAction::array(0, new NextAction("corruption", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void WarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("initialize pet", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no healthstone",
        NextAction::array(0, new NextAction("create healthstone", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "no firestone",
        NextAction::array(0, new NextAction("create firestone", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no spellstone",
        NextAction::array(0, new NextAction("create spellstone", ACTION_NORMAL), NULL)));
    */
}

void WarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void WarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void WarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));
}

void WarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void WarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void WarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));
}

void WarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void WarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void WarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH + 9), NULL)));
}

void WarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void WarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void WarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "corruption on attacker",
        NextAction::array(0, new NextAction("corruption on attacker", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("rain of fire", ACTION_HIGH), NULL)));
}

void WarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow ward",
        NextAction::array(0, new NextAction("shadow ward", ACTION_INTERRUPT), NULL)));
}

void WarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demon armor",
        NextAction::array(0, new NextAction("fel armor", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "soulstone",
        NextAction::array(0, new NextAction("soulstone", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void WarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "amplify curse",
        NextAction::array(0, new NextAction("amplify curse", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "inferno",
        NextAction::array(0, new NextAction("inferno", ACTION_HIGH), NULL)));
}

void WarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "spell lock on enemy healer",
        NextAction::array(0, new NextAction("spell lock on enemy healer", ACTION_INTERRUPT + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "spell lock",
        NextAction::array(0, new NextAction("spell lock", ACTION_INTERRUPT + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil interrupt",
        NextAction::array(0, new NextAction("death coil", ACTION_INTERRUPT + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on enemy healer",
        NextAction::array(0, new NextAction("death coil on enemy healer", ACTION_INTERRUPT + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on snare target",
        NextAction::array(0, new NextAction("death coil on snare target", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "banish",
        NextAction::array(0, new NextAction("banish on cc", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "fear",
        NextAction::array(0, new NextAction("fear on cc", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("howl of terror", ACTION_INTERRUPT), NULL)));
}

void WarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear pvp",
        NextAction::array(0, new NextAction("fear", ACTION_INTERRUPT + 1), NULL)));
}

void WarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon succubus", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felhunter", ACTION_NORMAL), NULL)));
}

void WarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon voidwalker", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "no curse on attacker",
            NextAction::array(0, new NextAction("curse of agony on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no curse",
        NextAction::array(0, new NextAction("curse of agony", ACTION_NORMAL + 2), NULL)));
}

void WarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_ONE // TBC

void WarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("sacrifice", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "life tap",
        NextAction::array(0, new NextAction("life tap", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow trance",
        NextAction::array(0, new NextAction("shadow bolt", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "drain soul",
        NextAction::array(0, new NextAction("drain soul", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "corruption",
        NextAction::array(0, new NextAction("corruption", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void WarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no healthstone",
        NextAction::array(0, new NextAction("create healthstone", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "no firestone",
        NextAction::array(0, new NextAction("create firestone", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no spellstone",
        NextAction::array(0, new NextAction("create spellstone", ACTION_NORMAL), NULL)));
    */
}

void WarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void WarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void WarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));
}

void WarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void WarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void WarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));
}

void WarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void WarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void WarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("soulshatter", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH + 9), NULL)));
}

void WarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void WarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void WarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "seed of corruption on attacker",
        NextAction::array(0, new NextAction("seed of corruption on attacker", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "corruption on attacker",
        NextAction::array(0, new NextAction("corruption on attacker", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("rain of fire", ACTION_HIGH), NULL)));
}

void WarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow ward",
        NextAction::array(0, new NextAction("shadow ward", ACTION_INTERRUPT), NULL)));
}

void WarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demon armor",
        NextAction::array(0, new NextAction("fel armor", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "soulstone",
        NextAction::array(0, new NextAction("soulstone", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void WarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "amplify curse",
        NextAction::array(0, new NextAction("amplify curse", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "inferno",
        NextAction::array(0, new NextAction("inferno", ACTION_HIGH), NULL)));
}

void WarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "spell lock on enemy healer",
        NextAction::array(0, new NextAction("spell lock on enemy healer", ACTION_INTERRUPT + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "spell lock",
        NextAction::array(0, new NextAction("spell lock", ACTION_INTERRUPT + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil interrupt",
        NextAction::array(0, new NextAction("death coil", ACTION_INTERRUPT + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on enemy healer",
        NextAction::array(0, new NextAction("death coil on enemy healer", ACTION_INTERRUPT + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on snare target",
        NextAction::array(0, new NextAction("death coil on snare target", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "banish",
        NextAction::array(0, new NextAction("banish on cc", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "fear",
        NextAction::array(0, new NextAction("fear on cc", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("howl of terror", ACTION_INTERRUPT), NULL)));
}

void WarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear pvp",
        NextAction::array(0, new NextAction("fear", ACTION_INTERRUPT + 1), NULL)));
}

void WarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("initialize pet", ACTION_NORMAL), NULL)));
}

void WarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon succubus", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felhunter", ACTION_NORMAL), NULL)));
}

void WarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon voidwalker", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "no curse on attacker",
            NextAction::array(0, new NextAction("curse of agony on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no curse",
        NextAction::array(0, new NextAction("curse of agony", ACTION_NORMAL + 2), NULL)));
}

void WarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void WarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("sacrifice", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "life tap",
        NextAction::array(0, new NextAction("life tap", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow trance",
        NextAction::array(0, new NextAction("shadow bolt", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "drain soul",
        NextAction::array(0, new NextAction("drain soul", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "corruption",
        NextAction::array(0, new NextAction("corruption", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void WarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no healthstone",
        NextAction::array(0, new NextAction("create healthstone", ACTION_NORMAL), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "no firestone",
        NextAction::array(0, new NextAction("create firestone", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no spellstone",
        NextAction::array(0, new NextAction("create spellstone", ACTION_NORMAL), NULL)));
    */
}

void WarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void WarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void WarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("shadowburn", ACTION_HIGH), NULL)));
}

void WarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void WarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void WarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("drain life", ACTION_LIGHT_HEAL), NULL)));
}

void WarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void WarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void WarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("soulshatter", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH + 9), NULL)));
}

void WarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void WarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void WarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "seed of corruption on attacker",
        NextAction::array(0, new NextAction("seed of corruption on attacker", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "corruption on attacker",
        NextAction::array(0, new NextAction("corruption on attacker", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged medium aoe",
        NextAction::array(0, new NextAction("rain of fire", ACTION_HIGH), NULL)));
}

void WarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void WarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow ward",
        NextAction::array(0, new NextAction("shadow ward", ACTION_INTERRUPT), NULL)));
}

void WarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demon armor",
        NextAction::array(0, new NextAction("fel armor", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "soulstone",
        NextAction::array(0, new NextAction("soulstone", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void WarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "amplify curse",
        NextAction::array(0, new NextAction("amplify curse", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "inferno",
        NextAction::array(0, new NextAction("inferno", ACTION_HIGH), NULL)));
}

void WarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "spell lock on enemy healer",
        NextAction::array(0, new NextAction("spell lock on enemy healer", ACTION_INTERRUPT + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "spell lock",
        NextAction::array(0, new NextAction("spell lock", ACTION_INTERRUPT + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil interrupt",
        NextAction::array(0, new NextAction("death coil", ACTION_INTERRUPT + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on enemy healer",
        NextAction::array(0, new NextAction("death coil on enemy healer", ACTION_INTERRUPT + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "death coil on snare target",
        NextAction::array(0, new NextAction("death coil on snare target", ACTION_INTERRUPT + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "banish",
        NextAction::array(0, new NextAction("banish on cc", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "fear",
        NextAction::array(0, new NextAction("fear on cc", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("howl of terror", ACTION_INTERRUPT), NULL)));
}

void WarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fear pvp",
        NextAction::array(0, new NextAction("fear", ACTION_INTERRUPT + 1), NULL)));
}

void WarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void WarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void WarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void WarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon succubus", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felhunter", ACTION_NORMAL), NULL)));
}

void WarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon voidwalker", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", ACTION_NORMAL), NULL)));
}

void WarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            "no curse on attacker",
            NextAction::array(0, new NextAction("curse of agony on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "no curse",
        NextAction::array(0, new NextAction("curse of agony", ACTION_NORMAL + 2), NULL)));
}

void WarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

void WarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{

}

#endif

void WarlockManualPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("initialize pet", ACTION_NORMAL + 1), NULL)));
}

void WarlockManualCurseStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    if (ai->HasStrategy("aoe", BotState::BOT_STATE_COMBAT))
    {
        triggers.push_back(new TriggerNode(
            triggerName + " on attacker",
            NextAction::array(0, new NextAction(actionName + " on attacker", ACTION_HIGH + 1), NULL)));
    }

    triggers.push_back(new TriggerNode(
        triggerName,
        NextAction::array(0, new NextAction(actionName, ACTION_HIGH), NULL)));
}
