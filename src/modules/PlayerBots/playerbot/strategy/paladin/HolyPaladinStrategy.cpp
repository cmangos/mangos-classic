
#include "playerbot/playerbot.h"
#include "PaladinMultipliers.h"
#include "HolyPaladinStrategy.h"

using namespace ai;

#ifdef MANGOSBOT_ZERO // Vanilla

void HolyPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy shock on party", ACTION_MEDIUM_HEAL + 5), 
                             new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy shock", ACTION_MEDIUM_HEAL + 2), 
                             new NextAction("holy light", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("divine shield", ACTION_EMERGENCY + 1),
                             new NextAction("holy light", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of light", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("judgement", ACTION_NORMAL), NULL)));
}

void HolyPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));
}

void HolyPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HolyPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "divine favor",
        NextAction::array(0, new NextAction("divine favor", ACTION_HIGH), NULL)));
}

void HolyPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("concentration aura", ACTION_NORMAL + 1), NULL)));
}

void HolyPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_ONE // TBC

void HolyPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 6), 
                             new NextAction("holy shock on party", ACTION_MEDIUM_HEAL + 5), 
                             new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 3), 
                             new NextAction("holy shock", ACTION_MEDIUM_HEAL + 2), 
                             new NextAction("holy light", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("divine shield", ACTION_EMERGENCY + 1),
            new NextAction("holy light", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of light", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("judgement", ACTION_NORMAL), NULL)));
}

void HolyPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));
}

void HolyPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HolyPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "divine illumination",
        NextAction::array(0, new NextAction("divine illumination", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "divine favor",
        NextAction::array(0, new NextAction("divine favor", ACTION_HIGH), NULL)));
}

void HolyPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("concentration aura", ACTION_NORMAL + 1), NULL)));
}

void HolyPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

#ifdef MANGOSBOT_TWO // WOTLK

void HolyPaladinStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("lay on hands on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "hand of sacrifice",
        NextAction::array(0, new NextAction("hand of sacrifice", ACTION_MEDIUM_HEAL + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 6), 
                             new NextAction("holy shock on party", ACTION_MEDIUM_HEAL + 5), 
                             new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 3), 
                             new NextAction("holy shock", ACTION_MEDIUM_HEAL + 2), 
                             new NextAction("holy light", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("divine shield", ACTION_EMERGENCY + 1),
            new NextAction("holy light", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of light", ACTION_NORMAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("judgement of light", ACTION_NORMAL + 1), NULL)));
}

void HolyPaladinStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "set glyph::Divinity",
        NextAction::array(0, new NextAction("set glyph::Divinity", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "set glyph::Blessing of Wisdom",
        NextAction::array(0, new NextAction("set glyph::Blessing of Wisdom", ACTION_NORMAL + 1), NULL)));
}

void HolyPaladinStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPveStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPveStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPveStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPveStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinPvpStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinPvpStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinPvpStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitCombatTriggers(triggers);
    PaladinRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitNonCombatTriggers(triggers);
    PaladinRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitReactionTriggers(triggers);
    PaladinRaidStrategy::InitReactionTriggers(triggers);
}

void HolyPaladinRaidStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinStrategy::InitDeadTriggers(triggers);
    PaladinRaidStrategy::InitDeadTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAoeStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAoeRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAoeStrategy::InitNonCombatTriggers(triggers);
    PaladinAoeRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBuffStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply oil", ACTION_NORMAL), NULL)));
}

void HolyPaladinBuffPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBuffRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBuffStrategy::InitNonCombatTriggers(triggers);
    PaladinBuffRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "divine illumination",
        NextAction::array(0, new NextAction("divine illumination", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "divine favor",
        NextAction::array(0, new NextAction("divine favor", ACTION_HIGH), NULL)));
}

void HolyPaladinBoostStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBoostStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBoostRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBoostStrategy::InitNonCombatTriggers(triggers);
    PaladinBoostRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCcStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCcStrategy::InitNonCombatTriggers(triggers);
    PaladinCcRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinCureStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCurePvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCurePvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinCureRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinCureStrategy::InitNonCombatTriggers(triggers);
    PaladinCureRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinAuraStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "no paladin aura",
        NextAction::array(0, new NextAction("concentration aura", ACTION_NORMAL + 1), NULL)));
}

void HolyPaladinAuraPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinAuraRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinAuraStrategy::InitNonCombatTriggers(triggers);
    PaladinAuraRaidStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    PaladinBlessingStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPveStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPveStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingPvpStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingPvpStrategy::InitNonCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitCombatTriggers(triggers);
}

void HolyPaladinBlessingRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    HolyPaladinBlessingStrategy::InitNonCombatTriggers(triggers);
    PaladinBlessingRaidStrategy::InitNonCombatTriggers(triggers);
}

#endif

