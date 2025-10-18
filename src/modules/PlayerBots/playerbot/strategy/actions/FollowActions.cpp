
#include "playerbot/playerbot.h"
#include "FollowActions.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/LootObjectStack.h"

using namespace ai;

bool FollowAction::Execute(Event& event)
{
    bool moved = false;
    Unit* followTarget = AI_VALUE(Unit*, "follow target");
    Formation* formation = AI_VALUE(Formation*, "formation");

    if (ai->IsSafe(followTarget))
    {
        if (formation)
        {
            WorldLocation loc = formation->GetLocation();
            if (!Formation::IsNullLocation(loc) && loc.mapid != -1)
            {
                moved = Follow(followTarget, formation->GetOffset(), formation->GetAngle());
            }
        }
    }

    return moved;
}

bool FollowAction::isUseful()
{
    if (!ai->CanMove())
        return false;

    float distance = 0;
    Unit* followTarget = AI_VALUE(Unit*, "follow target");
    Formation* formation = AI_VALUE(Formation*, "formation");

    if (followTarget && followTarget->IsPlayer())
    {
        if (AI_VALUE(GuidPosition, "rpg target") && AI_VALUE2(bool, "can free move to", AI_VALUE(GuidPosition, "rpg target").to_string()))
        {
            return false;
        }
    }

    if (followTarget)
    {
        if (followTarget->IsTaxiFlying() || !CanDeadFollow(followTarget) || followTarget->GetGUIDLow() == bot->GetGUIDLow())
        {
            return false;
        }
    }

    if (followTarget)
    {
        distance = sServerFacade.GetDistance2d(bot, followTarget);
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || bot->GetMapId() != loc.mapid)
        {
            return false;
        }

        distance = sServerFacade.GetDistance2d(bot, loc.coord_x, loc.coord_y);
    }

    if (sServerFacade.IsDistanceGreaterThan(distance, sPlayerbotAIConfig.sightDistance))
    {
        return true;
    }

    // maybe jump is faster?
    if (ai->HasStrategy("follow jump", BotState::BOT_STATE_NON_COMBAT) && ai->AllowActivity())
    {
        return true;
    }

    if (followTarget && sServerFacade.GetChaseTarget(bot) && sServerFacade.GetChaseTarget(bot)->GetObjectGuid() == followTarget->GetObjectGuid() && formation->GetAngle() == sServerFacade.GetChaseAngle(bot) && formation->GetOffset() == sServerFacade.GetChaseOffset(bot))
    {
        return false;
    }

    return true;
}

bool FollowAction::CanDeadFollow(Unit* target)
{
    //Move to corpse when dead and player is alive or not a ghost.
    if (!sServerFacade.IsAlive(bot) && (sServerFacade.IsAlive(target) || !target->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)))
    {
        return false;
    }
    
    return true;
}

bool FleeToMasterAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Unit* fTarget = AI_VALUE(Unit*, "master target");
    bool canFollow = Follow(fTarget);
    if (!canFollow)
    {
        //SetDuration(5000);
        return false;
    }

    WorldPosition targetPos(fTarget);
    WorldPosition bosPos(bot);
    float distance = bosPos.fDist(targetPos);


    uint32 scale = 5;
    if (bot->GetGroup() && bot->GetGroup()->GetMembersCount() > 5)
        scale = bot->GetGroup()->GetMembersCount();
        

    if (distance > sPlayerbotAIConfig.reactDistance && bot->IsInCombat())
    {
        if (!urand(0, scale))
            ai->TellPlayerNoFacing(requester, "I'm heading to your location but I'm in combat", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            //ai->TellPlayer(BOT_TEXT("wait_travel_combat"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
    else if (distance < sPlayerbotAIConfig.reactDistance * 3)
    {
        if (!urand(0, scale))
            ai->TellPlayerNoFacing(requester, BOT_TEXT("wait_travel_close"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
    else if (distance < 1000)
    {
        if (!urand(0, scale*4))
            ai->TellPlayerNoFacing(requester, BOT_TEXT("wait_travel_medium"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
    else
        if (!urand(0, scale*6))
            ai->TellPlayerNoFacing(requester, BOT_TEXT("wait_travel_medium"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
           
    SetDuration(3000U);
    return true;
}

bool FleeToMasterAction::isUseful()
{
    if (!ai->CanMove())
        return false;

    if (!ai->GetGroupMaster())
        return false;

    if (ai->GetGroupMaster() == bot)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");

    if (target && ai->GetGroupMaster()->HasTarget(target->GetObjectGuid()))
        return false;

    if (!ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
        return false;

    Unit* fTarget = AI_VALUE(Unit*, "master target");
    
    if (!CanDeadFollow(fTarget))
        return false;

    if (fTarget && fTarget->IsPlayer())
    {
        if (AI_VALUE(GuidPosition, "rpg target") && AI_VALUE2(bool, "can free move to", AI_VALUE(GuidPosition, "rpg target").to_string()))
            return false;
    }

    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsLootPossible(bot))
        return false;

    return true;
}

