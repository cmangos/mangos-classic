
#include "playerbot/playerbot.h"
#include "PvpTriggers.h"
#include "playerbot/ServerFacade.h"
#include "BattleGround/BattleGroundWS.h"
#include "playerbot/strategy/values/PositionValue.h"
#ifndef MANGOSBOT_ZERO
#include "BattleGround/BattleGroundEY.h"
#endif

using namespace ai;

bool EnemyPlayerNear::IsActive()
{
    // Check if we have any enemy players to attack
    if(AI_VALUE(bool, "has enemy player targets"))
    {
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget)
        {
            // Check if we have a better enemy player to attack
            Player* currentPlayerTarget = dynamic_cast<Player*>(currentTarget);
            if(currentPlayerTarget)
            {
                return currentPlayerTarget != AI_VALUE(Unit*, "enemy player target");
            }
        }

        return true;
    }

    return false;
}

bool PlayerHasNoFlag::IsActive()
{
#ifdef MANGOS
    if (ai->GetBot()->InBattleGround())
    {
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();
            if (!(bg->GetFlagState(bg->GetOtherTeam(bot->GetTeam())) == BG_WS_FLAG_STATE_ON_PLAYER))
                return true;
            if (bot->GetObjectGuid() == bg->GetAllianceFlagCarrierGuid() || bot->GetObjectGuid() == bg->GetHordeFlagCarrierGuid())
            {
                return false;
            }
            return true;
        }
        return false;
    }
#endif
    return false;
}

bool PlayerIsInBattleground::IsActive()
{
    return ai->GetBot()->InBattleGround();
}

bool BgWaitingTrigger::IsActive()
{
    if (bot->InBattleGround())
    {
        if (bot->GetBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_WAIT_JOIN)
            return true;
    }
    return false;
}

bool BgActiveTrigger::IsActive()
{
    if (bot->InBattleGround())
    {
        if (bot->GetBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_IN_PROGRESS)
            return true;
    }
    return false;
}

bool BgInviteActiveTrigger::IsActive()
{
    if (bot->InBattleGround() || !bot->InBattleGroundQueue())
    {
        return false;
    }

    for (int i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattleGroundQueueTypeId queueTypeId = bot->GetBattleGroundQueueTypeId(i);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;
#ifdef MANGOSBOT_TWOx
        BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
        GroupQueueInfo ginfo;
        if (bgQueue.GetPlayerGroupInfoData(bot->GetObjectGuid(), &ginfo))
        {
            if (ginfo.isInvitedToBgInstanceGuid && ginfo.removeInviteTime)
            {
                sLog.outDetail("Bot #%d <%s> (%u %s) : Invited to BG but not in BG", bot->GetGUIDLow(), bot->GetName(), bot->GetLevel(), bot->GetTeam() == ALLIANCE ? "A" : "H");
                return true;
            }
        }
#endif
#ifndef MANGOSBOT_ZERO
        if (bot->IsInvitedForBattleGroundQueueType(queueTypeId))
            return true;
#endif
    }
    return false;
}

bool BgEndedTrigger::IsActive()
{
    if (bot->InBattleGround())
    {
        if (bot->GetBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_WAIT_LEAVE)
            return true;
    }
    return false;
}

bool PlayerIsInBattlegroundWithoutFlag::IsActive()
{
#ifdef MANGOS
    if (ai->GetBot()->InBattleGround())
    {
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();
            if (!(bg->GetFlagState(bg->GetOtherTeam(bot->GetTeam())) == BG_WS_FLAG_STATE_ON_PLAYER))
                return true;
            if (bot->GetGUIDLow() == bg->GetAllianceFlagCarrierGuid() || bot->GetGUIDLow() == bg->GetHordeFlagCarrierGuid())
            {
                return false;
            }
        }
        return true;
    }
#endif
    return false;
}

bool PlayerHasFlag::IsActive()
{
    ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
    if (bot->InBattleGround())
    {
        if (bot->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            if (pos.isSet() && sServerFacade.GetDistance2d(bot, pos.x, pos.y) < 10.0f)
                return false;

            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

            if (!bg)
                return false;

            if (bot->GetObjectGuid() == bg->GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE) || bot->GetObjectGuid() == bg->GetFlagCarrierGuid(TEAM_INDEX_HORDE))
            {
                return true;
            }
        }
#ifndef MANGOSBOT_ZERO
        if (bot->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_EY)
        {
            BattleGroundEY* bg = (BattleGroundEY*)ai->GetBot()->GetBattleGround();
            return bot->GetObjectGuid() == bg->GetFlagCarrierGuid();
        }
#endif
        return false;
    }
    return false;
}

bool TeamHasFlag::IsActive()
{
#ifdef MANGOS
    if (ai->GetBot()->InBattleGround())
    {
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

            if (bot->GetObjectGuid() == bg->GetAllianceFlagCarrierGuid() || bot->GetObjectGuid() == bg->GetHordeFlagCarrierGuid())
            {
                return false;
            }

            if (bg->GetFlagState(bg->GetOtherTeam(bot->GetTeam())) == BG_WS_FLAG_STATE_ON_PLAYER)
                return true;
        }
        return false;
    }
#endif
    return false;
}

bool EnemyTeamHasFlag::IsActive()
{
    if (ai->GetBot()->InBattleGround())
    {
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

            if (!bg)
                return false;

            if (bot->GetTeam() == HORDE)
            {
                if (!bg->GetFlagCarrierGuid(TEAM_INDEX_HORDE).IsEmpty())
                    return true;
            }
            else
            {
                if (!bg->GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE).IsEmpty())
                    return true;
            }
        }
        return false;
    }
    return false;
}

bool EnemyFlagCarrierNear::IsActive()
{
    Unit* carrier = AI_VALUE(Unit*, "enemy flag carrier");
    return carrier && sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(bot, carrier), VISIBILITY_DISTANCE_SMALL);
}

bool TeamFlagCarrierNear::IsActive()
{
    Unit* carrier = AI_VALUE(Unit*, "team flag carrier");
    return carrier && sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(bot, carrier), VISIBILITY_DISTANCE_SMALL);
}

bool PlayerWantsInBattlegroundTrigger::IsActive()
{
    if (bot->InBattleGround())
        return false;

    if (bot->GetBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_WAIT_JOIN)
        return false;

    if (bot->GetBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_IN_PROGRESS)
        return false;

    if (!bot->CanJoinToBattleground())
        return false;

    return true;
};

bool VehicleNearTrigger::IsActive()
{
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest vehicles");
    return npcs.size();
}

bool InVehicleTrigger::IsActive()
{
    return ai->IsInVehicle(false,false,false,false,false, getQualifier());
}
