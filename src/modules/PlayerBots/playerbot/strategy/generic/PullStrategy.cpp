
#include "playerbot/playerbot.h"
#include "playerbot/strategy/PassiveMultiplier.h"
#include "PullStrategy.h"

using namespace ai;

class PullStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    PullStrategyActionNodeFactory()
    {
        creators["pull start"] = &pull_start;
    }

private:
    static ActionNode* pull_start(PlayerbotAI* ai)
    {
        return new ActionNode("pull start",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NextAction::array(0, new NextAction("pull action", ACTION_NORMAL), NULL));
    }
};

PullStrategy::PullStrategy(PlayerbotAI* ai, std::string pullAction, std::string prePullAction)
: Strategy(ai)
, pullActionName(pullAction)
, preActionName(prePullAction)
, pendingToStart(false)
, pullStartTime(0)
, petReactState(REACT_DEFENSIVE)
{
    actionNodeFactories.Add(new PullStrategyActionNodeFactory());

    if (!ai->GetBot())
        return;
}

std::string PullStrategy::GetPullActionName() const
{
    std::string modPullActionName = pullActionName;

    // Select the faerie fire based on druid strategy
    if (ai->GetBot()->getClass() == CLASS_DRUID)
    {
        if (modPullActionName == "faerie fire")
        {
            if (ai->HasSpell("faerie fire (feral)") && (ai->HasStrategy("tank feral", BotState::BOT_STATE_COMBAT) || ai->HasStrategy("dps feral", BotState::BOT_STATE_COMBAT)))
            {
                modPullActionName = "faerie fire (feral)";
            }
        }
    }

    return modPullActionName;
}

std::string PullStrategy::GetSpellName() const
{
    std::string spellName = GetPullActionName();
    if (spellName == "shoot")
    {
        const Item* equippedWeapon = ai->GetBot()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (equippedWeapon)
        {
            const ItemPrototype* itemPrototype = equippedWeapon->GetProto();
            if (itemPrototype)
            {
                switch (itemPrototype->SubClass)
                {
#ifdef MANGOSBOT_ZERO
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    {
                        spellName += " gun";
                        break;
                    }

                    case ITEM_SUBCLASS_WEAPON_BOW:
                    {
                        spellName += " bow";
                        break;
                    }

                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        spellName += " crossbow";
                        break;
                    }
#endif
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    {
                        spellName = "throw";
                        break;
                    }

                    default: break;
                }
            }
        }
    }

    return spellName;
}

float PullStrategy::GetRange() const
{
    float range;

    // Try to get the pull action range
    if (ai->GetSpellRange(GetSpellName(), &range))
    {
        range -= CONTACT_DISTANCE;
    }
    else
    {
        // Set the default range if the range was not found
        range = (pullActionName == "shoot") ? ai->GetRange("shoot") : ai->GetRange("spell");
    }

    return range;
}

std::string PullStrategy::GetPreActionName() const
{
    std::string modPullActionName = preActionName;

    // Select the faerie fire based on druid strategy
    if (ai->GetBot()->getClass() == CLASS_DRUID)
    {
        if (modPullActionName == "dire bear form")
        {
            if (GetPullActionName() == "faerie fire")
            {
                modPullActionName.clear();
            }
        }
    }

    return modPullActionName;
}

void PullStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "pull start",
        NextAction::array(0, new NextAction("pull start", ACTION_MOVE), new NextAction("pull action", ACTION_MOVE), NULL)));

    triggers.push_back(new TriggerNode(
        "pull end",
        NextAction::array(0, new NextAction("pull end", ACTION_MOVE), NULL)));
}

void PullStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}

void PullStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new PullMultiplier(ai));
}

void PullStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    InitCombatMultipliers(multipliers);
}

PullStrategy* PullStrategy::Get(PlayerbotAI* ai)
{
    return ai ? ai->GetStrategy<PullStrategy>("pull", BotState::BOT_STATE_COMBAT) : nullptr;
}

Unit* PullStrategy::GetTarget() const
{
    AiObjectContext* context = ai->GetAiObjectContext();
    return AI_VALUE(Unit*, "pull target");
}

void PullStrategy::SetTarget(Unit* target)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    SET_AI_VALUE(Unit*, "pull target", target);
}

bool PullStrategy::CanDoPullAction(Unit* target)
{
    // Check if the bot can perform the pull action

    // check if has ranged weapon
    if (ai->GetBot()->getClass() != CLASS_DRUID && ai->GetBot()->getClass() != CLASS_PALADIN && !ai->GetBot()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        return false;

    bool canPull = false;
    const std::string& pullAction = GetPullActionName();
    if (!pullAction.empty())
    {
        // Temporarily set the pull target to be used by the can do specific action method
        AiObjectContext* context = ai->GetAiObjectContext();
        Unit* previousTarget = GetTarget();
        SetTarget(target);

        canPull = ai->CanDoSpecificAction("pull action", true, false);

        // Restore the previous pull target
        SetTarget(previousTarget);
    }

    return canPull;
}


void PullStrategy::OnPullStarted()
{
    pendingToStart = false;
}

void PullStrategy::OnPullEnded()
{
    pullStartTime = 0;
    SetTarget(nullptr);
}

void PullStrategy::RequestPull(Unit* target, bool resetTime)
{
    SetTarget(target);
    pendingToStart = true;
    if(resetTime)
    {
        pullStartTime = time(0);
    }
}

float PullMultiplier::GetValue(Action* action)
{
    const PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy && strategy->HasTarget())
    {
        if ((action->getName() == "pull my target") ||
            (action->getName() == "pull rti target") ||
            (action->getName() == "reach pull") ||
            (action->getName() == "pull start") ||
            (action->getName() == "pull action") ||
            (action->getName() == "return to pull position") ||
            (action->getName() == "pull end"))
        {
            return 1.0f;
        }

        if (action->getRelevance() >= 100)
        {
            return 0.01f;
        }

        return 0.0f;
    }

    return 1.0f;
}

void PossibleAdsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "possible ads",
        NextAction::array(0, new NextAction("flee with pet", ACTION_EMERGENCY), NULL)));
}

void PullBackStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "return to pull position",
        NextAction::array(0, new NextAction("return to pull position", ACTION_MOVE + 5.0f), NULL)));
}

void PullBackStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}