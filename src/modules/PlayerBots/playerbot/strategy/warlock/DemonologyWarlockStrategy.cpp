
#include "playerbot/playerbot.h"
#include "WarlockTriggers.h"
#include "WarlockMultipliers.h"
#include "DemonologyWarlockStrategy.h"

using namespace ai;

class DemonologyWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DemonologyWarlockStrategyActionNodeFactory()
    {
        creators["demonic sacrifice raid"] = &demonic_sacrifice_raid;
    }

private:
    ACTION_NODE_P(demonic_sacrifice_raid, "demonic sacrifice", "summon succubus");
};

DemonologyWarlockStrategy::DemonologyWarlockStrategy(PlayerbotAI* ai) : WarlockStrategy(ai)
{
    actionNodeFactories.Add(new DemonologyWarlockStrategyActionNodeFactory());
}

#ifdef MANGOSBOT_ZERO // Vanilla

ai::NextAction** DemonologyWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void DemonologyWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "soul link",
        NextAction::array(0, new NextAction("soul link", ACTION_NORMAL), NULL)));
}

void DemonologyWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demonic sacrifice",
        NextAction::array(0, new NextAction("demonic sacrifice raid", ACTION_HIGH), NULL)));
}

void DemonologyWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /* NO PET BECAUSE OF DEMONIC SACRIFICE
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
    */
}

void DemonologyWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_ONE // TBC

ai::NextAction** DemonologyWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void DemonologyWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "soul link",
        NextAction::array(0, new NextAction("soul link", ACTION_NORMAL), NULL)));
}

void DemonologyWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demonic sacrifice",
        NextAction::array(0, new NextAction("demonic sacrifice raid", ACTION_HIGH), NULL)));
}

void DemonologyWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", ACTION_NORMAL + 2), NULL)));
}

void DemonologyWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", ACTION_NORMAL + 2), NULL)));
}

void DemonologyWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /* NO PET BECAUSE OF DEMONIC SACRIFICE
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
    */
}

void DemonologyWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif
#ifdef MANGOSBOT_TWO // WOTLK

ai::NextAction** DemonologyWarlockStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shadow bolt", ACTION_IDLE), NULL);
}

void DemonologyWarlockStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPveStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPveStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockPvpStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockPvpStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitCombatTriggers(triggers);
    WarlockRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitNonCombatTriggers(triggers);
    WarlockRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitReactionTriggers(triggers);
    WarlockRaidStrategy::InitReactionTriggers(triggers);
}

void DemonologyWarlockRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockStrategy::InitDeadTriggers(triggers);
    WarlockRaidStrategy::InitDeadTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockAoeStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockAoeStrategy::InitNonCombatTriggers(triggers);
    WarlockAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "soul link",
        NextAction::array(0, new NextAction("soul link", ACTION_NORMAL), NULL)));
}

void DemonologyWarlockBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBuffStrategy::InitNonCombatTriggers(triggers);
    WarlockBuffRaidStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demonic sacrifice",
        NextAction::array(0, new NextAction("demonic sacrifice raid", ACTION_HIGH), NULL)));
}

void DemonologyWarlockBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockBoostStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockBoostStrategy::InitNonCombatTriggers(triggers);
    WarlockBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCcStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPveStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCcStrategy::InitNonCombatTriggers(triggers);
    WarlockCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockPetStrategy::InitNonCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPveStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", ACTION_NORMAL + 2), NULL)));
}

void DemonologyWarlockPetPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetPvpStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", ACTION_NORMAL + 2), NULL)));
}

void DemonologyWarlockPetRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockPetStrategy::InitCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockPetRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /* NO PET BECAUSE OF DEMONIC SACRIFICE
    DemonologyWarlockPetStrategy::InitNonCombatTriggers(triggers);
    WarlockPetRaidStrategy::InitNonCombatTriggers(triggers);
    */
}

void DemonologyWarlockCursesStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    WarlockCursesStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPveStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesPvpStrategy::InitCombatTriggers(triggers);
}

void DemonologyWarlockCursesRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DemonologyWarlockCursesStrategy::InitCombatTriggers(triggers);
    WarlockCursesRaidStrategy::InitCombatTriggers(triggers);
}

#endif