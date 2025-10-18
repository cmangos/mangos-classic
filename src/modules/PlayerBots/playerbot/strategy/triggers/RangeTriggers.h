#pragma once
#include "playerbot/strategy/Trigger.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/generic/CombatStrategy.h"
#include "playerbot/strategy/values/PossibleAttackTargetsValue.h"
#include "playerbot/strategy/values/Formations.h"

namespace ai
{
    class EnemyTooCloseForSpellTrigger : public Trigger 
    {
    public:
        EnemyTooCloseForSpellTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for spell") {}
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                if (ai->HasStrategy("follow", BotState::BOT_STATE_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_COMBAT))
                    if(bot->getClass() != CLASS_HUNTER || sServerFacade.GetDistance2d(bot, target) > 5.0f)
                        return false;                   

                const bool canMove = !PossibleAttackTargetsValue::HasBreakableCC(target, bot) && !PossibleAttackTargetsValue::HasUnBreakableCC(target, bot);

                // Don't move if the target is targeting you and you can't add distance between you and the target
                if (target->GetTarget() == bot && canMove && target->GetSpeedInMotion() > (bot->GetSpeedInMotion() * 0.65))
                {
                    return false;
                }

                float const combatReach = bot->GetCombinedCombatReach(target, false);
                float const minDistance = ai->GetRange("spell") + combatReach;
                float const targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;

                // No need to move if the target is rooted and you can shoot
                if (!canMove && (targetDistance > minDistance))
                {
                    return false;
                }

                bool isBoss = false;
                bool isRaid = false;
                bool isVictim = target->GetVictim() && target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid();

                if (target->IsCreature())
                {
                    Creature* creature = ai->GetCreature(target->GetObjectGuid());
                    if (creature)
                    {
                        isBoss = creature->IsWorldBoss();
                    }
                }

                if (bot->GetMap()->IsRaid())
                    isRaid = true;

                //if (isBoss || isRaid)
                //    return sServerFacade.IsDistanceLessThan(targetDistance, (ai->GetRange("spell") + combatReach) / 2);

                float coeff = 0.5f;
                if (target->IsPlayer())
                {
                    if (!isVictim)
                        coeff = 0.4f;
                    else
                        coeff = 0.6f;
                }
                else
                {
                    if (!isVictim)
                        coeff = 0.4f;
                    else
                        coeff = 0.6f;
                }

                if (isRaid)
                    coeff = 0.7f;

                return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, minDistance * coeff);
            }
            return false;
        }
    };

    class EnemyTooCloseForShootTrigger : public Trigger 
    {
    public:
        EnemyTooCloseForShootTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for shoot") {}
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                // Don't move if the target is targeting you and you can't add distance between you and the target
                if (target->GetTarget() == bot && !target->IsRooted() && target->GetSpeedInMotion() > (bot->GetSpeedInMotion() * 0.65))
                {
                    return false;
                }

                float const combatReach = bot->GetCombinedCombatReach(target, false);
                float const minShootDistance = ai->GetRange("shoot") + combatReach;
                float const targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;

                // No need to move if the target is rooted and you can shoot
                if (target->IsRooted() && (targetDistance > minShootDistance))
                {
                    return false;
                }

                bool isBoss = false;
                bool isRaid = false;
                bool isVictim = target->GetVictim() && target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid();

                if (target->IsCreature())
                {
                    Creature* creature = ai->GetCreature(target->GetObjectGuid());
                    if (creature)
                    {
                        isBoss = creature->IsWorldBoss();
                    }
                }

                if (bot->GetMap()->IsRaid())
                    isRaid = true;

                //if (isBoss || isRaid)
                //    return sServerFacade.IsDistanceLessThan(targetDistance, (ai->GetRange("spell") + combatReach));

                float coeff = 0.5f;
                if (target->IsPlayer())
                {
                    if (!isVictim)
                        coeff = 0.7f;
                    else
                        coeff = 1.0f;
                }
                else
                {
                    if (!isVictim)
                        coeff = 0.4f;
                    else
                        coeff = 0.6f;
                }

                if (isRaid)
                    coeff = 1.0f;

                return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, minShootDistance * coeff);
            }

            return false;
        }
    };

    class EnemyTooCloseForMeleeTrigger : public Trigger 
    {
    public:
        EnemyTooCloseForMeleeTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for melee", 3) {}
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target && target->IsPlayer())
                return false;

            return target && AI_VALUE2(bool, "inside target", "current target");
        }
    };

    class EnemyInRangeTrigger : public Trigger 
    {
    public:
        EnemyInRangeTrigger(PlayerbotAI* ai, std::string name, float distance, bool enemyMustBePlayer = false, bool enemyTargetsBot = false)
        : Trigger(ai, name)
        , distance(distance)
        , enemyMustBePlayer(enemyMustBePlayer)
        , enemyTargetsBot(enemyTargetsBot) {}
        
        virtual bool IsActive() override
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                if (enemyMustBePlayer && !target->IsPlayer())
                {
                    return false;
                }

                if (enemyTargetsBot && target->GetTarget() != bot)
                {
                    return false;
                }

                return sServerFacade.IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), distance);
            }

            return false;
        }

    protected:
        float distance;
        bool enemyMustBePlayer;
        bool enemyTargetsBot;
    };

    class EnemyIsCloseTrigger : public EnemyInRangeTrigger
    {
    public:
        EnemyIsCloseTrigger(PlayerbotAI* ai) : EnemyInRangeTrigger(ai, "enemy is close", sPlayerbotAIConfig.tooCloseDistance) {}
    };

    class OutOfRangeTrigger : public Trigger 
    {
    public:
        OutOfRangeTrigger(PlayerbotAI* ai, std::string name, float distance) : Trigger(ai, name)
        {
            this->distance = distance;
        }

        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            return target &&
                sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", GetTargetName()), distance);
        }

        virtual std::string GetTargetName() { return "current target"; }

    protected:
        float distance;
    };

    class EnemyOutOfMeleeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfMeleeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of melee range", sPlayerbotAIConfig.meleeDistance) {}
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return !bot->CanReachWithMeleeAttack(target) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class EnemyOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of spell range", ai->GetRange("spell")) {}
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class PartyMemberToHealOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        PartyMemberToHealOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "party member to heal out of spell range", ai->GetRange("heal")) {}
        virtual std::string GetTargetName() { return "party member to heal"; }
        
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class FarFromMasterTrigger : public Trigger 
    {
    public:
        FarFromMasterTrigger(PlayerbotAI* ai, std::string name = "far from master", float distance = 12.0f, int checkInterval = 50) : Trigger(ai, name, checkInterval), distance(distance) {}

        virtual bool IsActive()
        {
            Unit* master = AI_VALUE(Unit*, "master target");
            if (master && sServerFacade.IsFriendlyTo(bot, master))
            {
                if (master->GetTransport() && master->GetTransport() == bot->GetTransport())
                    return false;

                return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), distance);
            }

            return false;
        }

    private:
        float distance;
    };

    class OutOfReactRangeTrigger : public FarFromMasterTrigger
    {
    public:
        OutOfReactRangeTrigger(PlayerbotAI* ai, std::string name = "out of react range", float distance = sPlayerbotAIConfig.reactDistance, int checkInterval = 2) : FarFromMasterTrigger(ai, name, distance, checkInterval) {}
    };

    class NotNearMasterTrigger : public OutOfReactRangeTrigger
    {
    public:
        NotNearMasterTrigger(PlayerbotAI* ai, std::string name = "not near master", int checkInterval = 2) : OutOfReactRangeTrigger(ai, name, 5.0f, checkInterval) {}

        virtual bool IsActive()
        {
            return FarFromMasterTrigger::IsActive() && !sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig.reactDistance);
        }
    };

    class UpdateFollowTrigger : public NotNearMasterTrigger
    {
    public:
        UpdateFollowTrigger(PlayerbotAI* ai) : NotNearMasterTrigger(ai, "update follow", 3) {}

        virtual bool IsActive()
        {
            //We are not near target. Move closer
            if (NotNearMasterTrigger::IsActive())
                return true;

            Unit* followTarget = AI_VALUE(Unit*, "follow target");

            if (!followTarget || !ai->IsSafe(followTarget))
                return false;

            //We need to land or liftoff.
            if (followTarget->IsFlying() != bot->IsFlying() || followTarget->IsTaxiFlying())
                return true;

            Formation* formation = AI_VALUE(Formation*, "formation");

            //Already using proper formation.
            if (sServerFacade.GetChaseTarget(bot) && sServerFacade.GetChaseTarget(bot)->GetObjectGuid() == followTarget->GetObjectGuid() && formation->GetAngle() == sServerFacade.GetChaseAngle(bot) && formation->GetOffset() == sServerFacade.GetChaseOffset(bot))
                return false;

            if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT))
                return true;

            Unit* target = AI_VALUE(Unit*, "current target");

            if (!target)
                return true;

            if (target->GetTarget() == bot) //Try pulling target to follow position
                return true;

            if (!ai->IsRanged(bot)) //Melee bots stay in melee.
                return false;

            WorldPosition formationPosition = AI_VALUE(WorldPosition, "formation position");

            if (formationPosition.sqDistance2d(target) > ai->GetRange("spell")) //Do not move to follow if we can't attack from that position.
                return false;

            return true;
        }
    };

    class StopFollowTrigger : public Trigger
    {
    public:
        StopFollowTrigger(PlayerbotAI* ai) : Trigger(ai, "stop follow", 1) {}

        virtual bool IsActive()
        {
            if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                return false;

            if (sServerFacade.GetChaseTarget(bot) && !sServerFacade.GetChaseTarget(bot)->IsPlayer() && sServerFacade.GetChaseTarget(bot)->IsMoving())
                return false;

            Unit* followTarget = AI_VALUE(Unit*, "follow target");

            if (!followTarget)
                return true;

            if (bot->GetTransport() != followTarget->GetTransport())
                return true;

            //We need to land or liftoff.
            if (followTarget->IsTaxiFlying())
                return true;

            return false;
        }
    };

    class WaitForAttackSafeDistanceTrigger : public Trigger
    {
    public:
        WaitForAttackSafeDistanceTrigger(PlayerbotAI* ai, std::string name = "wait for attack safe distance") : Trigger(ai, name) {}

        virtual bool IsActive()
        {
            if (WaitForAttackStrategy::ShouldWait(ai))
            {
                // Do not move if stay strategy is set
                if (!ai->HasStrategy("stay", ai->GetState()))
                {
                    // Do not move if currently being targeted
                    const bool isBeingTargeted = !bot->getAttackers().empty();
                    if (!isBeingTargeted)
                    {
                        Unit* target = AI_VALUE(Unit*, "current target");
                        if (target)
                        {
                            const float safeDistance = WaitForAttackStrategy::GetSafeDistance();
                            const float safeDistanceThreshold = WaitForAttackStrategy::GetSafeDistanceThreshold();
                            const float distanceToTarget = sServerFacade.GetDistance2d(bot, target);
                            return (distanceToTarget > (safeDistance + safeDistanceThreshold)) ||
                                   (distanceToTarget < (safeDistance - safeDistanceThreshold));
                        }
                    }
                }
            }

            return false;
        }
    };
}