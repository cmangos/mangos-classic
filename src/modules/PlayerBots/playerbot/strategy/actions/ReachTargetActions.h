#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/values/MoveStyleValue.h"
#include "GenericSpellActions.h"
#include "playerbot/PlayerbotFactory.h"

namespace ai
{
    class ReachTargetAction : public MovementAction, public Qualified
    {
    public:
        ReachTargetAction(PlayerbotAI* ai, std::string name, float range = 0.0f) : MovementAction(ai, name), Qualified(), range(range), spellName("") {}

        virtual void Qualify(const std::string& qualifier) override
        {
            Qualified::Qualify(qualifier);

            // Get the distance from the qualified spell given
            if (!qualifier.empty())
            {
                spellName = Qualified::getMultiQualifierStr(qualifier, 0, "::");

                float maxSpellRange;
                if (ai->GetSpellRange(spellName, &maxSpellRange))
                {
                    range = maxSpellRange;
                }
            }
        }

        virtual bool Execute(Event& event) override
		{
            Unit* target = GetTarget();
            if (target)
            {
                UpdateMovementState();

                // Ignore movement if too far
                const float distanceToTarget = bot->GetDistance(target, false, DIST_CALC_COMBAT_REACH);
                float chaseDist = range;
                const bool inLos = bot->IsWithinLOSInMap(target, true);
                const bool isFriend = sServerFacade.IsFriendlyTo(bot, target);

                if (range > 0.0f)
                {
                    chaseDist = inLos ? range : (isFriend ? std::min(distanceToTarget * 0.9f, range) : range);
                    chaseDist = (chaseDist - sPlayerbotAIConfig.contactDistance);
                }

                if (MoveStyleValue::WaitForEnemy(ai) && target->m_movementInfo.HasMovementFlag(movementFlagsMask) &&
                        sServerFacade.IsInFront(target, bot, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT) &&
                        sServerFacade.IsDistanceGreaterThan(distanceToTarget, sPlayerbotAIConfig.tooCloseDistance))
                {
                    return true;
                }                 

                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    std::ostringstream out;
                    out << "Moving to reach " << ChatHelper::formatWorldobject(target);
                    ai->TellPlayerNoFacing(GetMaster(), out);
                }
                
                if (inLos && isFriend && (range <= ai->GetRange("follow")))
                {
                    return MoveNear(target, chaseDist);
                }
                else
                {
                    return ChaseTo(target, chaseDist, bot->GetAngle(target));
                }
            }

            return false;
        }

        virtual bool isUseful() override
		{
            // Do not move if stay strategy is set
            if (!ai->HasStrategy("stay", ai->GetState()))
            {
                Unit* target = GetTarget();
                if (target)
                {
                    // Do not move while casting
                    if (!bot->IsNonMeleeSpellCasted(true, false, true))
                    {
                        // Check if the spell for which the reach action is used for can be casted
                        if (!spellName.empty() && !ai->CanCastSpell(spellName, target, true, nullptr, true, true, true))
                        {
                            return false;
                        }

                        // Force move if not in los
                        if (bot->IsWithinLOSInMap(target, true))
                        {
                            // Check if the bot is already on the range required
                            return bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > range;
                        }

                        return true;
                    }
                }
            }

            return false;
        }

        virtual std::string GetTargetName() override { return "current target"; }
        std::string GetSpellName() const { return spellName; }

        virtual Unit* GetTarget() override
        {
            // Get the target from the qualifiers
            if (!qualifier.empty())
            {
                std::string targetQualifier;
                const std::string targetName = Qualified::getMultiQualifierStr(qualifier, 1, "::");
                if (targetName != "current target")
                {
                    targetQualifier = Qualified::getMultiQualifierStr(qualifier, 2, "::");
                }

                return targetQualifier.empty() ? AI_VALUE(Unit*, targetName) : AI_VALUE2(Unit*, targetName, targetQualifier);
            }
            else
            {
                return AI_VALUE(Unit*, GetTargetName());
            }
        }

    protected:
        float range;
        std::string spellName;
    };

    class CastReachTargetSpellAction : public CastSpellAction
    {
    public:
        CastReachTargetSpellAction(PlayerbotAI* ai, std::string spell, float distance) : CastSpellAction(ai, spell), distance(distance) {}

		virtual bool isUseful() override
		{
            // Do not move if stay strategy is set
            if (ai->HasStrategy("stay", ai->GetState()))
                return false;

			return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), (distance + sPlayerbotAIConfig.contactDistance));
		}

    protected:
        float distance;
    };

    class ReachMeleeAction : public ReachTargetAction
	{
    public:
        ReachMeleeAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach melee") {}
    };

    class ReachSpellAction : public ReachTargetAction
	{
    public:
        ReachSpellAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach spell", ai->GetRange("spell")) {}
    };

    class ReachPullAction : public ReachTargetAction
    {
    public:
        ReachPullAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach pull")
        {
            PullStrategy* strategy = PullStrategy::Get(ai);
            if (strategy)
            {
                range = strategy->GetRange();
            }
        }

        void Qualify(const std::string& qualifier) override
        {
            ReachTargetAction::Qualify(qualifier);

            // Reduce the range slightly for a more accurate pull (moving targets can get out of reach)
            const float threshold = 5.0f;
            range = range > threshold ? range - threshold : range;
        }

        std::string GetTargetName() override { return "pull target"; }
    };

    class ReachPartyMemberToHealAction : public ReachTargetAction
    {
    public:
        ReachPartyMemberToHealAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach party member to heal", ai->GetRange("heal")) {}
        virtual std::string GetTargetName() override { return "party member to heal"; }
    };
}
