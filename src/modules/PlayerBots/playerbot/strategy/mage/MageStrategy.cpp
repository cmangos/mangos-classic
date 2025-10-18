
#include "playerbot/playerbot.h"
#include "MageMultipliers.h"
#include "MageStrategy.h"

using namespace ai;

class MageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    MageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["frost nova"] = &frost_nova;
        creators["evocation"] = &evocation;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
        creators["invisibility"] = &invisibility;
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }

private:
    ACTION_NODE_A(frostbolt, "frostbolt", "shoot");

    ACTION_NODE_A(frost_nova, "frost nova", "cone of cold");

    ACTION_NODE_A(evocation, "evocation", "mana potion");

    ACTION_NODE_A(remove_curse, "remove curse", "remove lesser curse");

    ACTION_NODE_A(remove_curse_on_party, "remove curse on party", "remove lesser curse on party");

    ACTION_NODE_A(frostfire_bolt, "frostfire bolt", "fireball");

    static ActionNode* dragons_breath(PlayerbotAI* ai)
    {
        return new ActionNode("dragon's breath",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("blast wave"), NULL),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), NULL));
    }

    static ActionNode* blast_wave(PlayerbotAI* ai)
    {
        return new ActionNode("blast wave",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("frost nova"), NULL),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), NULL));
    }

    ACTION_NODE_A(invisibility, "invisibility", "lesser invisibility");

    ACTION_NODE_A(molten_armor, "molten armor", "mage armor");

    ACTION_NODE_A(mage_armor, "mage armor", "ice armor");

    ACTION_NODE_A(ice_armor, "ice armor", "frost armor");
};

MageStrategy::MageStrategy(PlayerbotAI* ai) : ClassStrategy(ai)
{
    actionNodeFactories.Add(new MageStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

void MageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("ice block", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blink",
        NextAction::array(0, new NextAction("blink", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana gem", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("evocation", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind aura",
        NextAction::array(0, new NextAction("pyroblast", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind aura",
        NextAction::array(0, new NextAction("frostbolt", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no drink",
        NextAction::array(0, new NextAction("conjure water", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no food",
        NextAction::array(0, new NextAction("conjure food", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana gem",
        NextAction::array(0, new NextAction("conjure mana gem", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void MageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void MagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH), NULL)));
}

void MagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void MagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void MagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void MagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void MagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void MagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void MagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void MageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "multiple attackers",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH + 9), NULL)));
}

void MageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void MageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void MageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void MageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void MageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fire ward",
        NextAction::array(0, new NextAction("fire ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "frost ward",
        NextAction::array(0, new NextAction("frost ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("mana shield", ACTION_HIGH), NULL)));
}

void MageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane intellect on party",
        NextAction::array(0, new NextAction("arcane intellect on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane brilliance on party",
        NextAction::array(0, new NextAction("arcane brilliance on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane intellect",
        NextAction::array(0, new NextAction("arcane intellect", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "give water",
        NextAction::array(0, new NextAction("give water", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "give food",
        NextAction::array(0, new NextAction("give food", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void MageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_HIGH), NULL)));
}

void MageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_NORMAL), NULL)));
}

void MageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array(0, new NextAction("mage armor", ACTION_HIGH), NULL)));
}

void MageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array(0, new NextAction("mage armor", ACTION_NORMAL), NULL)));
}

void MageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void MageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void MageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("frost nova", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array(0, new NextAction("counterspell on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("counterspell", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array(0, new NextAction("polymorph", ACTION_INTERRUPT), NULL)));
}

void MageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void MageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void MageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL), NULL)));
}

void MageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_NORMAL), NULL)));
}

void MageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void MageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void MageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

void MageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("ice block", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blink",
        NextAction::array(0, new NextAction("blink", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "spellsteal",
        NextAction::array(0, new NextAction("spellsteal", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana gem", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("evocation", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no drink",
        NextAction::array(0, new NextAction("conjure water", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no food",
        NextAction::array(0, new NextAction("conjure food", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana gem",
        NextAction::array(0, new NextAction("conjure mana gem", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void MageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void MagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH), NULL)));
}

void MagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void MagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void MagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void MagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void MagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void MagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void MagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void MageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "multiple attackers",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH), NULL)));
}

void MageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void MageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void MageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void MageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void MageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "fire ward",
        NextAction::array(0, new NextAction("fire ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "frost ward",
        NextAction::array(0, new NextAction("frost ward", ACTION_HIGH), NULL)));
}

void MageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane intellect on party",
        NextAction::array(0, new NextAction("arcane intellect on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane brilliance on party",
        NextAction::array(0, new NextAction("arcane brilliance on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane intellect",
        NextAction::array(0, new NextAction("arcane intellect", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "give water",
        NextAction::array(0, new NextAction("give water", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "give food",
        NextAction::array(0, new NextAction("give food", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void MageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_HIGH), NULL)));
}

void MageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_NORMAL), NULL)));
}

void MageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "molten armor",
        NextAction::array(0, new NextAction("molten armor", ACTION_HIGH), NULL)));
}

void MageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "molten armor",
        NextAction::array(0, new NextAction("molten armor", ACTION_NORMAL), NULL)));
}

void MageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void MageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void MageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("frost nova", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array(0, new NextAction("counterspell on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("counterspell", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array(0, new NextAction("polymorph", ACTION_INTERRUPT), NULL)));
}

void MageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void MageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void MageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL), NULL)));
}

void MageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_NORMAL), NULL)));
}

void MageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void MageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void MageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

void MageStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("ice block", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blink",
        NextAction::array(0, new NextAction("blink", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "spellsteal",
        NextAction::array(0, new NextAction("spellsteal", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana gem", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("evocation", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana",
        NextAction::array(0, new NextAction("shoot", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no drink",
        NextAction::array(0, new NextAction("conjure water", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no food",
        NextAction::array(0, new NextAction("conjure food", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "no mana gem",
        NextAction::array(0, new NextAction("conjure mana gem", ACTION_NORMAL), NULL)));
}

void MageStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitReactionTriggers(triggers);
}

void MageStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassStrategy::InitDeadTriggers(triggers);
}

void MagePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH), NULL)));
}

void MagePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitNonCombatTriggers(triggers);
}

void MagePvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitReactionTriggers(triggers);
}

void MagePvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPvpStrategy::InitDeadTriggers(triggers);
}

void MagePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitCombatTriggers(triggers);
}

void MagePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitNonCombatTriggers(triggers);
}

void MagePveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitReactionTriggers(triggers);
}

void MagePveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassPveStrategy::InitDeadTriggers(triggers);
}

void MageRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "high threat",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "multiple attackers",
        NextAction::array(0, new NextAction("invisibility", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("shoot", ACTION_HIGH), NULL)));
}

void MageRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitReactionTriggers(triggers);
}

void MageRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    ClassRaidStrategy::InitDeadTriggers(triggers);
}

void MageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitCombatTriggers(triggers);
}

void MageAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitCombatTriggers(triggers);
}

void MageAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitCombatTriggers(triggers);
}

void MageAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoePveStrategy::InitNonCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitCombatTriggers(triggers);
}

void MageAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "mirror image",
        NextAction::array(0, new NextAction("mirror image", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire ward",
        NextAction::array(0, new NextAction("fire ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "frost ward",
        NextAction::array(0, new NextAction("frost ward", ACTION_HIGH), NULL)));
}

void MageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane intellect on party",
        NextAction::array(0, new NextAction("arcane intellect on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane brilliance on party",
        NextAction::array(0, new NextAction("arcane brilliance on party", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "give water",
        NextAction::array(0, new NextAction("give water", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "give food",
        NextAction::array(0, new NextAction("give food", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "arcane intellect",
        NextAction::array(0, new NextAction("arcane intellect", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void MageBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_HIGH), NULL)));
}

void MageBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "ice armor",
        NextAction::array(0, new NextAction("ice armor", ACTION_NORMAL), NULL)));
}

void MageBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "molten armor",
        NextAction::array(0, new NextAction("molten armor", ACTION_HIGH), NULL)));
}

void MageBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "molten armor",
        NextAction::array(0, new NextAction("molten armor", ACTION_NORMAL), NULL)));
}

void MageBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitCombatTriggers(triggers);
}

void MageBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitCombatTriggers(triggers);
}

void MageBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitCombatTriggers(triggers);
}

void MageBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitCombatTriggers(triggers);
}

void MageBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostPveStrategy::InitNonCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitCombatTriggers(triggers);
}

void MageBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    BoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("frost nova", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array(0, new NextAction("counterspell on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("counterspell", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array(0, new NextAction("polymorph", ACTION_INTERRUPT), NULL)));
}

void MageCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("throw grenade", ACTION_INTERRUPT), NULL)));
}

void MageCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitCombatTriggers(triggers);
}

void MageCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcPveStrategy::InitNonCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitCombatTriggers(triggers);
}

void MageCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CcRaidStrategy::InitNonCombatTriggers(triggers);
}

void MageCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL), NULL)));
}

void MageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_NORMAL), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_NORMAL), NULL)));
}

void MageCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitCombatTriggers(triggers);
}

void MageCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePvpStrategy::InitNonCombatTriggers(triggers);
}

void MageCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitCombatTriggers(triggers);
}

void MageCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CurePveStrategy::InitNonCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitCombatTriggers(triggers);
}

void MageCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    CureRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif