
#include "playerbot.h"
#include "FleeManager.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "Groups/Group.h"
#include "strategy/values/MoveStyleValue.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

void FleeManager::calculateDistanceToCreatures(FleePoint *point)
{
    point->minDistance = -1.0f;
    point->sumDistance = 0.0f;
    std::list<ObjectGuid> units = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("possible targets no los");
	for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); ++i)
    {
		Unit* unit = bot->GetPlayerbotAI()->GetUnit(*i);
		if (!unit)
		    continue;

        // do not count non-LOS mobs
        if (!unit->IsWithinLOS(point->x, point->y, point->z + unit->GetCollisionHeight(), true))
            continue;

		float d = sServerFacade.GetDistance2d(unit, point->x, point->y);
        point->sumDistance += d;
        if (point->minDistance < 0 || point->minDistance > d) point->minDistance = d;
	}
}

bool intersectsOri(float angle, std::list<float>& angles, float angleIncrement)
{
    for (std::list<float>::iterator i = angles.begin(); i != angles.end(); ++i)
    {
        float ori = *i;
        if (abs(angle - ori) < angleIncrement) return true;
    }

    return false;
}

void FleeManager::calculatePossibleDestinations(std::list<FleePoint*> &points)
{
    Unit *target = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<Unit*>("current target");

    float botPosX = startPosition.getX();
    float botPosY = startPosition.getY();
    float botPosZ = startPosition.getZ();
    
    FleePoint start(bot->GetPlayerbotAI(), botPosX, botPosY, botPosZ);
    calculateDistanceToCreatures(&start);

    std::list<float> enemyOri;
    std::list<ObjectGuid> units = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("possible targets no los");
    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = bot->GetPlayerbotAI()->GetUnit(*i);
        if (!unit)
            continue;

        float ori = bot->GetAngle(unit);
        enemyOri.push_back(ori);
    }

    float distIncrement = std::max(sPlayerbotAIConfig.followDistance, (maxAllowedDistance - sPlayerbotAIConfig.tooCloseDistance) / 10.0f);
    for (float dist = maxAllowedDistance; dist >= sPlayerbotAIConfig.tooCloseDistance; dist -= distIncrement)
    {
        float angleIncrement = std::max(M_PI / 20, M_PI / 4 / (1.0 + dist - sPlayerbotAIConfig.tooCloseDistance));
        for (float add = 0.0f; add < M_PI / 4 + angleIncrement; add += angleIncrement)
        {
            for (float angle = add; angle < add + 2 * M_PI_F + angleIncrement; angle += M_PI_F / 4)
            {
                if (intersectsOri(angle, enemyOri, angleIncrement)) continue;

                float x = botPosX + cos(angle) * maxAllowedDistance, y = botPosY + sin(angle) * maxAllowedDistance, z = botPosZ + CONTACT_DISTANCE;
                if (MoveStyleValue::CheckForEdges(bot->GetPlayerbotAI()) && isTooCloseToEdge(x, y, z, angle)) continue;

                if (forceMaxDistance && sServerFacade.IsDistanceLessThan(sServerFacade.GetDistance2d(bot, x, y), maxAllowedDistance - sPlayerbotAIConfig.tooCloseDistance))
                    continue;

                bot->UpdateAllowedPositionZ(x, y, z);

                const TerrainInfo* terrain = startPosition.getTerrain();
                if (terrain && terrain->IsInWater(x, y, z))
                    continue;

                if (/*!bot->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true) || */(target && !target->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true)))
                    continue;

                FleePoint *point = new FleePoint(bot->GetPlayerbotAI(), x, y, z);
                calculateDistanceToCreatures(point);

                if (sServerFacade.IsDistanceGreaterOrEqualThan(point->minDistance - start.minDistance, sPlayerbotAIConfig.followDistance))
                    points.push_back(point);
                else
                    delete point;
            }
        }
	}
}

bool FleeManager::isTooCloseToEdge(float x, float y, float z, float angle)
{
    Map* map = bot->GetMap();
    const TerrainInfo* terrain = map->GetTerrain();
    for (double a = angle; a <= angle + 2*M_PI; a += M_PI / 4)
    {
        float dist = sPlayerbotAIConfig.followDistance;
        float tx = x + cos(a) * dist;
        float ty = y + sin(a) * dist;
        float tz = z;
        bot->UpdateAllowedPositionZ(tx, ty, tz);

        if (terrain && terrain->IsInWater(tx, ty, tz))
            return true;

        if (!bot->IsWithinLOS(tx, ty, tz))
            return true;
    }

    return false;
}

void FleeManager::cleanup(std::list<FleePoint*> &points)
{
	for (std::list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
		delete point;
	}
	points.clear();
}

bool FleeManager::isBetterThan(FleePoint* point, FleePoint* other)
{
    return point->sumDistance - other->sumDistance > 0;
}

FleePoint* FleeManager::selectOptimalDestination(std::list<FleePoint*> &points)
{
	FleePoint* best = NULL;
	for (std::list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
        if (!best || isBetterThan(point, best))
            best = point;
	}

	return best;
}

bool FleeManager::CalculateDestination(float* rx, float* ry, float* rz)
{
    std::list<FleePoint*> points;
	calculatePossibleDestinations(points);

    FleePoint* point = selectOptimalDestination(points);
    if (!point)
    {
        cleanup(points);
        return false;
    }

	*rx = point->x;
	*ry = point->y;
	*rz = point->z;

    cleanup(points);
	return true;
}

bool FleeManager::isUseful()
{
    // It the bot is a victim of an aoe attack it should move no matter the target attack distance
    bool const inAoe = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<bool>("has area debuff", "self target")->Get();
    if (!inAoe)
    {
        std::list<ObjectGuid> units = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("possible targets no los");
        for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); ++i)
        {
            Unit* unit = bot->GetPlayerbotAI()->GetUnit(*i);
            if (unit)
            {
                float const distanceSquared = startPosition.sqDistance(WorldPosition(unit));
                float attackDistanceSquared = unit->GetAttackDistance(bot);
                attackDistanceSquared *= attackDistanceSquared;
                if (distanceSquared < attackDistanceSquared)
                {
                    return true;
                }

                //float d = sServerFacade.GetDistance2d(unit, bot);
                //if (sServerFacade.IsDistanceLessThan(d, sPlayerbotAIConfig.aggroDistance)) return true;
            }
        }

        return false;
    }

    return true;
}
