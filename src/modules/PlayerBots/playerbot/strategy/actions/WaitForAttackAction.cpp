
#include "playerbot/playerbot.h"
#include "WaitForAttackAction.h"
#include "playerbot/strategy/generic/CombatStrategy.h"

using namespace ai;

bool WaitForAttackKeepSafeDistanceAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");

    if (target && !target->IsStopped() && target->GetTarget() && target->GetTarget()->IsStopped())
        target = target->GetTarget();


    if (target && target->IsAlive())
    {
        const float safeDistance = std::max(float(target->GetAttackDistance(bot) + ATTACK_DISTANCE), WaitForAttackStrategy::GetSafeDistance());
        const float safeDistanceThreshold = WaitForAttackStrategy::GetSafeDistanceThreshold();

        // Find the best point around the target.
        const WorldPosition bestPoint = GetBestPoint(target, (safeDistance - safeDistanceThreshold), safeDistance);
        if (bestPoint)
        {
            // Move to the best point
            return MoveTo(bestPoint.getMapId(), bestPoint.getX(), bestPoint.getY(), bestPoint.getZ(), false, false, false, true);
        }
    }

    return false;
}

const ai::WorldPosition WaitForAttackKeepSafeDistanceAction::GetBestPoint(Unit* target, float minDistance, float maxDistance) const
{
    const Map* map = target->GetMap();
    const WorldPosition botPosition(bot);
    const WorldPosition targetPosition(target);
    const int8 startDir = urand(0, 1) * 2 - 1;
    const float radiansIncrement = (5.0f / 180.0f) * (M_PI_F);
    const float startAngle = targetPosition.getAngleTo(botPosition) + urand(0.f,radiansIncrement) * startDir;
    const float distance = frand(minDistance, maxDistance);
    const std::list<ObjectGuid> enemies = AI_VALUE(std::list<ObjectGuid>, "possible targets no los");

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
    {
        for (uint32 dist = 0; dist < distance; dist++)
        {
            WorldPosition point = targetPosition + WorldPosition(0, dist * cos(startAngle), dist * sin(startAngle), 1.0f);
            Creature* wpCreature = bot->SummonCreature(1, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 1000.0f + dist * 100.0f);
        }
    }

    for (float tryAngle = 0.0f; tryAngle < M_PI_F; tryAngle += radiansIncrement)
    {
        for (int8 tryDir = -1; tryAngle && tryDir < 1; tryDir += 2)
        {
            float pointAngle = startAngle;
            pointAngle += tryAngle * startDir * tryDir;

            WorldPosition point = targetPosition + WorldPosition(0, distance * cos(pointAngle), distance * sin(pointAngle), 1.0f);

            point.setZ(point.getHeight());

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
            {
                Creature* wpCreature = bot->SummonCreature(1, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + tryAngle * 1000.0f);
            }

            // Check if the target is visible from the point
            if (!target->IsWithinLOS(point.getX(), point.getY(), point.getZ() + bot->GetCollisionHeight()))
                continue;

            // Check if the point is not surrounded by other enemies
            if (IsEnemyClose(point, enemies))
                continue;

            // Check if the bot can move to this point.
            if (!botPosition.canPathTo(point,bot))
                continue;

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
            {
                Creature* wpCreature = bot->SummonCreature(15631, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + tryAngle * 1000.0f);
            }

            return point;
        }
    }



    return botPosition;
}

bool WaitForAttackKeepSafeDistanceAction::IsEnemyClose(const WorldPosition& point, const std::list<ObjectGuid>& enemies) const
{
    for (const ObjectGuid& enemyGUID : enemies)
    {
        Unit* enemy = ai->GetUnit(enemyGUID);
        if (enemy)
        {
            // If the enemy is visible in the same map
            if (enemy->IsWithinLOSInMap(bot))
            {
                // If the enemy is not neutral
                if (enemy->CanAttackOnSight(bot))
                {
                    const float enemyAttackRange = enemy->GetAttackDistance(bot) + ATTACK_DISTANCE;
                    const float distanceToPoint = WorldPosition(enemy).sqDistance(point);
                    if (distanceToPoint <= (enemyAttackRange * enemyAttackRange))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
