
#include "playerbot/playerbot.h"
#include "CombatStrategy.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

void CombatStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "invalid target",
        NextAction::array(0, new NextAction("select new target", 89.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "mounted",
        NextAction::array(0, new NextAction("check mount state", 88.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "combat stuck",
        NextAction::array(0, new NextAction("unstuck", 0.7f), NULL)));

    triggers.push_back(new TriggerNode(
        "combat long stuck",
        NextAction::array(0, new NextAction("unstuck", 0.9f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("use trinket", 50.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", 80.0f), NULL)));
}

float AvoidAoeStrategyMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string name = action->getName();
    if (name == "follow" || name == "co" || name == "nc" || name == "react" || name == "select new target" || name == "flee")
        return 1.0f;

    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!pSpellInfo) return 1.0f;

    if (spellId && pSpellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        return 1.0f;
    else if (spellId && pSpellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        return 1.0f;

    uint32 CastingTime = !IsChanneledSpell(pSpellInfo) ? GetSpellCastTime(pSpellInfo, bot) : GetSpellDuration(pSpellInfo);

    if (AI_VALUE2(bool, "has area debuff", "self target") && spellId && CastingTime > 0)
    {
        return 0.0f;
    }

    return 1.0f;
}

void AvoidAoeStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "has area debuff",
        NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 5), NULL)));
}

void AvoidAoeStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}

void AvoidAoeStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new AvoidAoeStrategyMultiplier(ai));
}

void AvoidAoeStrategy::InitReactionMultipliers(std::list<Multiplier*>& multipliers)
{
    InitCombatMultipliers(multipliers);
}

void WaitForAttackStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wait for attack safe distance",
        NextAction::array(0, new NextAction("wait for attack keep safe distance", 60.0f), NULL)));
}

void WaitForAttackStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new WaitForAttackMultiplier(ai));
}

bool WaitForAttackStrategy::ShouldWait(PlayerbotAI* ai)
{
    // Only check if the bot has the strategy enabled
    if (ai->HasStrategy("wait for attack", BotState::BOT_STATE_COMBAT))
    {
        // Only check if bot is in a group with a real player
        Player* player = ai->GetBot();
        if (player->GetGroup() && ai->HasRealPlayerMaster())
        {
            // Don't wait if the current target is an enemy player
            bool enemyPlayer = false;
            Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            if (target)
            {
                Player* player = dynamic_cast<Player*>(target);
                if (player)
                {
                    enemyPlayer = !sServerFacade.IsFriendlyTo(target, player);
                }
            }

            if (!enemyPlayer)
            {
                // Check if bot is currently in combat
                const time_t combatStartTime = PAI_VALUE(time_t, "combat start time");
                if (combatStartTime > 0)
                {
                    // Check the amount of time elapsed from the combat start
                    const time_t elapsedTime = time(0) - combatStartTime;
                    return elapsedTime < GetWaitTime(ai);
                }
            }
        }
    }

    return false;
}

uint8 WaitForAttackStrategy::GetWaitTime(PlayerbotAI* ai)
{
    Player* player = ai->GetBot();
    return PAI_VALUE(uint8, "wait for attack time");
}

float WaitForAttackMultiplier::GetValue(Action* action)
{
    // Allow some movement and targeting actions
    const std::string& actionName = action->getName();
    if ((actionName != "wait for attack keep safe distance") && 
        (actionName != "dps assist") && 
        (actionName != "set facing") &&
        (actionName != "pull my target") &&
        (actionName != "pull rti target") &&
        (actionName != "pull start") &&
        (actionName != "pull action") &&
        (actionName != "pull end"))
    {
        return WaitForAttackStrategy::ShouldWait(ai) ? 0.0f : 1.0f;
    }

    return 1.0f;
}

void HealInterruptStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "heal target full health",
        NextAction::array(0, new NextAction("interrupt current spell", ACTION_EMERGENCY), NULL)));
}

void HealInterruptStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}
