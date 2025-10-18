
#include "playerbot/playerbot.h"
#include "FreeMoveValues.h"
#include "PositionValue.h"
#include "Formations.h"


using namespace ai;

GuidPosition FreeMoveCenterValue::Calculate()
{       
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
    {
        Unit* followTarget = AI_VALUE(Unit*, "follow target");

        if (!followTarget)
            return bot;

        //Use bot as center when follow target is on a different map.
        if (followTarget->GetMapId() != bot->GetMapId()) 
            return bot;

        Player* player = dynamic_cast<Player*>(followTarget);

        //Use bot as cetner when follow target is being teleported.
        if (player && player->IsBeingTeleported())
            return bot;

        //Use formation location as reference point.
        Formation* formation = AI_VALUE(Formation*, "formation");
        GuidPosition loc(followTarget->GetObjectGuid(),formation->GetLocation());

        if (Formation::IsNullLocation(loc) || loc.mapid == -1)
            return followTarget;

        //Move the location to a location around follow targets destination.
        if (player && player->GetPlayerbotAI() && PAI_VALUE(WorldPosition, "last long move"))
            loc += (PAI_VALUE(WorldPosition, "last long move") - player);

        return loc;
    }

    PositionEntry pos;

    if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) && (pos = AI_VALUE2(PositionEntry, "pos", "stay")).isSet())
        return GuidPosition(bot->GetObjectGuid(), pos.Get());

    if (ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT) && (pos = AI_VALUE2(PositionEntry, "pos", "guard")).isSet())
        return GuidPosition(bot->GetObjectGuid(), pos.Get());

    return bot;
}

float FreeMoveRangeValue::Calculate()
{
    if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT))
        return INTERACTION_DISTANCE;

    Unit* followTarget = AI_VALUE(Unit*, "follow target");

    if (!followTarget || followTarget == bot)
        return 0;

    if (bot->GetMap()->IsDungeon())
        return INTERACTION_DISTANCE; 

    if (!ai->HasActivePlayerMaster())
    {
        if (followTarget->IsInCombat())
            return sPlayerbotAIConfig.reactDistance * 0.25f;

        return sPlayerbotAIConfig.reactDistance * 0.75f;
    }

    //Increase distance as master is standing still.
    float maxDist = INTERACTION_DISTANCE;

    bool hasFollow = ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT);
    bool hasGuard = ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT);
    bool hasFree = !hasFollow && !hasGuard;

    //When far away from master stop trying to limit the bot.
    if (!hasFollow && (ai->HasStrategy("travel once", BotState::BOT_STATE_NON_COMBAT) || (WorldPosition(followTarget).fDist(bot) > (hasFree ? sPlayerbotAIConfig.sightDistance : sPlayerbotAIConfig.maxFreeMoveDistance))))
        return 0;

    if (hasFree || hasGuard)//Free and guard start with a base 20y range.
        maxDist += sPlayerbotAIConfig.proximityDistance;

    uint32 lastMasterMove = MEM_AI_VALUE(WorldPosition, "master position")->LastChangeDelay();

    if (sPlayerbotAIConfig.freeMoveDelay && lastMasterMove > sPlayerbotAIConfig.freeMoveDelay) //After 30 seconds increase the range by 1y each second.
        maxDist += (lastMasterMove - static_cast<uint32>(sPlayerbotAIConfig.freeMoveDelay));

    if (maxDist > sPlayerbotAIConfig.maxFreeMoveDistance)
    {
        if (hasFree)
        {
            maxDist = 0;
        }
        else
        {
            maxDist = sPlayerbotAIConfig.maxFreeMoveDistance;
        }
    }

    return maxDist;
}

bool CanFreeMoveToValue::Calculate()
{
    if (!GetRange())
        return true;

    GuidPosition destPos(qualifier);

    destPos.updatePosition(bot->GetInstanceId());

    GuidPosition refPos = AI_VALUE(GuidPosition, "free move center");

    return refPos.distance(destPos) < GetRange();
}
