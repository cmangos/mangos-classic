/*
 * Copyright (C) 2010-2013 Anathema Engine project <http://valkyrie-wow.com/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2016-2017 Elysium Project <https://www.elysium-project.org>
 * Copyright (C) 2017 Light's Hope Project <http://www.lightshope.org>
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 */

#include "movement.hpp"
#include "../config.hpp"
#include "../libanticheat.hpp"
#include "../cyclic.hpp"

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Chat/Chat.h"
#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "Entities/GameObject.h"
#include "Server/WorldSession.h"
#include "Server/DBCStructure.h"
#include "World/World.h"
#include "Tools/Language.h"
#include "Movement/MoveSpline.h"
#include "Log/Log.h"

namespace
{
uint16 GetOrderResponse(uint16 opcode)
{
    switch (opcode)
    {
        case SMSG_FORCE_WALK_SPEED_CHANGE:
            return CMSG_FORCE_WALK_SPEED_CHANGE_ACK;
        case SMSG_FORCE_RUN_SPEED_CHANGE:
            return CMSG_FORCE_RUN_SPEED_CHANGE_ACK;
        case SMSG_FORCE_RUN_BACK_SPEED_CHANGE:
            return CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK;
        case SMSG_FORCE_SWIM_SPEED_CHANGE:
            return CMSG_FORCE_SWIM_SPEED_CHANGE_ACK;
        case SMSG_FORCE_SWIM_BACK_SPEED_CHANGE:
            return CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK;
        case SMSG_FORCE_TURN_RATE_CHANGE:
            return CMSG_FORCE_TURN_RATE_CHANGE_ACK;
        case SMSG_FORCE_MOVE_ROOT:
            return CMSG_FORCE_MOVE_ROOT_ACK;
        case SMSG_FORCE_MOVE_UNROOT:
            return CMSG_FORCE_MOVE_UNROOT_ACK;
        case SMSG_MOVE_FEATHER_FALL:
        case SMSG_MOVE_NORMAL_FALL:
            return CMSG_MOVE_FEATHER_FALL_ACK;
        case SMSG_MOVE_SET_HOVER:
        case SMSG_MOVE_UNSET_HOVER:
            return CMSG_MOVE_HOVER_ACK;
        case SMSG_MOVE_WATER_WALK:
        case SMSG_MOVE_LAND_WALK:
            return CMSG_MOVE_WATER_WALK_ACK;
        default:
            return 0;
    }
}

float DistanceSquared(const Position &a, const Position &b, bool includeZ)
{
    float ret = pow(a.x - b.x, 2);
    ret += pow(a.y - b.y, 2);

    if (includeZ)
        ret += pow(a.z - b.z, 2);

    return ret;
}

uint32 GetMoveType(uint32 moveFlags)
{
    if (moveFlags & MOVEFLAG_SWIMMING)
        return moveFlags & MOVEFLAG_BACKWARD ? MOVE_SWIM_BACK : MOVE_SWIM;
    else if (moveFlags & MOVEFLAG_WALK_MODE)
        return MOVE_WALK;
    else if (moveFlags & MOVEFLAG_MASK_MOVING)
        return moveFlags & MOVEFLAG_BACKWARD ? MOVE_RUN_BACK : MOVE_RUN;

    return MOVE_RUN;
}

constexpr float extrapolationEpsilon = 0.0002f;
}

namespace Movement
{
extern float computeFallElevation(float time, bool slowFall, float initialSpeed);
}

namespace NamreebAnticheat
{
Movement::Movement(Player* me) :
    _anticheat(reinterpret_cast<SessionAnticheat *>(me->GetSession()->GetAnticheat())),
    _me(me), _totalDistanceTraveled(0.f), _wasMovingOther(false), _justTeleported(false),
    _serverInitTime(0), _clientInitTime(0), _jumpInitialSpeed(0.f), _inKnockBack(false),
    overSpeedDistanceTick(0.f), overSpeedDistanceTotal(0.f)
{
    memset(clientSpeeds, 0, sizeof(clientSpeeds));

    MANGOS_ASSERT(!!_anticheat);
}

void Movement::KnockBack(float speedxy, float speedz, float cos, float sin)
{
    GetLastMovementInfo().jump.startClientTime = WorldTimer::getMSTime() - GetLastMovementInfo().stime + GetLastMovementInfo().ctime;
    GetLastMovementInfo().jump.start.x = _me->GetPositionX();
    GetLastMovementInfo().jump.start.y = _me->GetPositionY();
    GetLastMovementInfo().jump.start.z = _me->GetPositionZ();
    GetLastMovementInfo().jump.cosAngle = cos;
    GetLastMovementInfo().jump.sinAngle = sin;
    GetLastMovementInfo().jump.xyspeed = speedxy;
    GetLastMovementInfo().jump.zspeed = speedz;
    GetLastMovementInfo().moveFlags = MOVEFLAG_JUMPING | (GetLastMovementInfo().moveFlags & ~MOVEFLAG_MASK_MOVING_OR_TURN);
    _jumpInitialSpeed = speedz;
    _inKnockBack = true;
}

void Movement::VerifyMovementFlags(uint32 flags, uint32 &removeFlags, bool strict) const
{
    removeFlags = 0;

    // do not apply to gms for now
    if (_me->GetSession()->GetSecurity() != SEC_PLAYER)
        return;

    // if under spline movement, do nothing
    if (!_me->movespline->Finalized())
        return;

    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_FLY_HACK))
    {
        static constexpr uint32 flyHackFlags1 = (MOVEFLAG_WALK_MODE | MOVEFLAG_SWIMMING | MOVEFLAG_HOVER | MOVEFLAG_FALLINGFAR | MOVEFLAG_FLYING | MOVEFLAG_ONTRANSPORT);
        static constexpr uint32 flyHackFlags2 = (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING);

        if ((flags & flyHackFlags1) == flyHackFlags1)
        {
            _anticheat->RecordCheatInternal(CHEAT_TYPE_FLY_HACK, "MovementFlags hack: Player %s had old flyhack moveFlags mask (flags: 0x%lx)",
                _me->GetName(), flags);
            removeFlags |= flags;
        }

        // detect flyhack (these two flags should never happen at the same time)
        if ((flags & flyHackFlags2) == flyHackFlags2)
        {
            _anticheat->RecordCheatInternal(CHEAT_TYPE_FLY_HACK, "MovementFlags hack: Player %s had MOVE_SWIMMING and MOVE_FLYING (flags: 0x%lx)",
                _me->GetName(), flags);
            removeFlags |= flyHackFlags2;
        }

        // no need to check pending orders.  players should never have this.
        if (flags & MOVEFLAG_LEVITATING)
        {
            _anticheat->RecordCheatInternal(CHEAT_TYPE_FLY_HACK, "MovementFlags hack: Player %s had MOVE_LEVITATING (only for creatures!) (flags: 0x%lx)",
                _me->GetName(), flags);
            removeFlags |= MOVEFLAG_LEVITATING;
        }

        // no need to check pending orders.  players should never have this.
        if (flags & MOVEFLAG_CAN_FLY)
        {
            _anticheat->RecordCheatInternal(CHEAT_TYPE_FLY_HACK, "MovementFlags hack: Player %s had MOVE_CAN_FLY (flags: 0x%lx)",
                _me->GetName(), flags);
            removeFlags |= MOVEFLAG_CAN_FLY;
        }
    }

    // if swimming, but not in water...
    // XXX TODO FIXME -- finish this after gathering some empirical data
    //if (!!(flags & MOVE_SWIMMING))
    //{
    //    float ground = -1234.5678f;
    //    auto const waterLevel = _me->GetTerrain()->GetWaterLevel(_me->GetPositionX(), _me->GetPositionY(), _me->GetPositionZ(), &ground);

    //    sLog.out(LOG_ANTICHEAT_DEBUG, "z: %f ground: %f water level: %f", _me->GetPositionZ(), ground, waterLevel);
    //}

    // if we are not performing a strict check, go no further
    if (!strict)
        return;

    bool pendingRoot = false, pendingWaterWalkRemoval = false, pendingSlowFallRemoval = false;
    for (auto const &order : _orders)
    {
        if (order.opcode == SMSG_FORCE_MOVE_ROOT)
            pendingRoot = true;

        if (order.opcode == SMSG_MOVE_LAND_WALK)
            pendingWaterWalkRemoval = true;

        if (order.opcode == SMSG_MOVE_NORMAL_FALL)
            pendingSlowFallRemoval = true;
    }

    // if the client is claiming to not be rooted, but the player actually is rooted, and there is
    // no pending root order (in case the order had not arrived yet), assume the order was ignored
    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_ROOT_MOVE) &&
        !(flags & MOVEFLAG_ROOT) &&
        _me->hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_ROOT) &&
        !pendingRoot)
        _anticheat->RecordCheatInternal(CHEAT_TYPE_ROOT_MOVE, "MovementFlags hack: Player %s lacks MOVEFLAG_ROOT but is stunned and/or rooted with no pending removal", _me->GetName());

    // if the client is claiming to be water walking, but the player has no aura for it, and no
    // pending cancellation order, assume it is illegitimate
    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_WATER_WALK) &&
        !!(flags & MOVEFLAG_WATERWALKING) &&
        !_me->HasAuraType(SPELL_AURA_WATER_WALK) &&
        !_me->HasAuraType(SPELL_AURA_GHOST) &&
        !pendingWaterWalkRemoval)
    {
        _anticheat->RecordCheatInternal(CHEAT_TYPE_WATER_WALK, "MovementFlags hack: Player %s had MOVEFLAG_WATERWALKING but no water walk aura with no pending removal", _me->GetName());
        removeFlags |= MOVEFLAG_WATERWALKING;
    }

    // XXX check correct movement flag on slow fall

    // if safe falling with no aura and no pending removal order, cheater
    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_SLOW_FALL) &&
        !!(flags & MOVEFLAG_SAFE_FALL) &&
        !_me->HasAuraType(SPELL_AURA_FEATHER_FALL) &&
        !pendingSlowFallRemoval)
    {
        _anticheat->RecordCheatInternal(CHEAT_TYPE_SLOW_FALL, "MovementFlags hack: Player %s had MOVEFLAG_SAFE_FALL with no slow fall aura and no pending orders", _me->GetName());
        removeFlags |= MOVEFLAG_SAFE_FALL;
    }

    // things to check: hover
}

void Movement::OrderSent(uint16 opcode, uint32 counter)
{
    _orderHistory.push_back({ opcode, WorldTimer::getMSTime() });

    _orders.emplace_back<PendingOrder>({ opcode, counter, WorldTimer::getMSTime() });

    sLog.outDebug("ORDER: %s (%u) sent to %s (counter = %u)", LookupOpcodeName(opcode), opcode, _me->GetName(), counter);
}

void Movement::OrderAck(uint16 opcode, uint32 counter)
{
    _ackHistory.push_back({ opcode, WorldTimer::getMSTime() });

    sLog.outDebug("ORDER: %s (%u) ACK %s (counter = %u)", LookupOpcodeName(opcode), opcode, _me->GetName(), counter);

    if (_orders.empty())
    {
        if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_BAD_ORDER_ACK))
            _anticheat->RecordCheatInternal(CHEAT_TYPE_BAD_ORDER_ACK, "Received ACK %s (%u) but no orders are pending",
                LookupOpcodeName(opcode), opcode);

        return;
    }

    // if this detection method is disabled, just empty the results and stop
    if (!sAnticheatConfig.IsEnabled(CHEAT_TYPE_BAD_ORDER_ACK))
    {
        _orders.clear();
        return;
    }

    // FIXME: there is currently a bug in the core whereby SMSG_FORCE_* opcodes are sent to a world session for
    // units other than the one they are controlling (i.e. a pet), prompting multiple consecutive ACKs, which
    // are not guaranteed to arrive in the same order they were sent.  once that bug is fixed, the below code
    // can be replaced with one which is both faster and more strict, where only the front of the queue is
    // checked against the data received from the client.
    for (auto i = _orders.begin(); i != _orders.end(); ++i)
    {
        auto const response = GetOrderResponse(i->opcode);

        // found? remove from the queue and return
        if (response == opcode && counter == i->counter)
        {
            _orders.erase(i);
            return;
        }
    }

    // if execution reaches here, it means that the acknowledgement we received was not in the queue to be received.
    // this should never happen for legitimate players.

    _anticheat->RecordCheatInternal(CHEAT_TYPE_BAD_ORDER_ACK, "Received ACK %s (%u) counter = %u which was not pending",
        LookupOpcodeName(opcode), opcode, counter);
}

void Movement::CheckExpiredOrders(uint32 latency)
{
    auto const now = WorldTimer::getMSTime();
    const uint32 expirationDelay = 2.5 * latency;

    for (auto i = _orders.begin(); i != _orders.end(); ++i)
    {
        auto const expected = i->time + expirationDelay;

        if (expected < now)
        {
            //m_anticheat->RecordCheatInternal(CHEAT_TYPE_BAD_ORDER_ACK, "Expired order received no ACK for %s (%u) counter %u sent %u expected %u latency %u now %u",
            //    LookupOpcodeName(i->opcode), i->opcode, i->counter, i->time, expected, latency, now);
            i = _orders.erase(i);

            if (i == _orders.end())
                break;
        }
    }
}

// Movement processing anticheat main routine
bool Movement::HandleAnticheatTests(MovementInfo& movementInfo, WorldSession* session, const WorldPacket& packet)
{
    MANGOS_ASSERT(!!_me);
    MANGOS_ASSERT(session);

    if (!sAnticheatConfig.EnableAnticheat())
        return true;

    // TODO: Currently anticheat is disabled with Mind Controlled players!
    if (!_me->IsSelfMover())
    {
        _wasMovingOther = true;
        return true;
    }
    // if we were moving someone else, but are no longer, skip one more tick
    else if (_wasMovingOther)
    {
        _wasMovingOther = false;
        return true;
    }

    auto const opcode = packet.GetOpcode();

    // Dont accept movement packets while movement is controlled by server (fear, charge, etc..)
    if (!_me->movespline->Finalized())
        return false;

    // two dimensional distance traveled
    auto const dist2d =
        (movementInfo.pos.x - GetLastMovementInfo().pos.x) * (movementInfo.pos.x - GetLastMovementInfo().pos.x) +
        (movementInfo.pos.y - GetLastMovementInfo().pos.y) * (movementInfo.pos.y - GetLastMovementInfo().pos.y);

    _totalDistanceTraveled += sqrtf(dist2d);

    if (_inKnockBack && opcode != MSG_MOVE_FALL_LAND)
        movementInfo.jump = GetLastMovementInfo().jump;

    if (opcode == CMSG_MOVE_FEATHER_FALL_ACK)
    {
        GetLastMovementInfo().jump.startClientTime = movementInfo.jump.startClientTime = movementInfo.ctime;
        //GetLastMovementInfo().jump.start = movementInfo.jump.start = movementInfo.pos;
        _jumpInitialSpeed = std::max(_jumpInitialSpeed, 7.f);
    }

    /*if ((movementInfo.moveFlags & MOVE_ROOT) && (movementInfo.moveFlags & MOVE_MASK_MOVING_OR_TURN))
        APPEND_CHEAT(CHEAT_TYPE_ROOT_MOVE);*/

    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_MULTIJUMP) &&
        opcode == MSG_MOVE_JUMP &&
        GetLastMovementInfo().moveFlags & MOVEFLAG_JUMPING)
    {
        _anticheat->RecordCheatInternal(CHEAT_TYPE_MULTIJUMP, "Jump start %u", movementInfo.jump.startClientTime);

        std::string str;
        DumpMovement(str);

        sLog.outBasic("Multijump debug output:\n%s", str.c_str());
    }

    if (opcode == MSG_MOVE_STOP_SWIM && (movementInfo.moveFlags & MOVEFLAG_SWIMMING))
        _anticheat->RecordCheatInternal(CHEAT_TYPE_FLY_HACK);

    if (!movementInfo.ctime && sAnticheatConfig.IsEnabled(CHEAT_TYPE_NULL_CLIENT_TIME))
        _anticheat->RecordCheatInternal(CHEAT_TYPE_NULL_CLIENT_TIME);

    // This can happen if the player is knocked back while jumping
    if (opcode == MSG_MOVE_JUMP && sAnticheatConfig.IsEnabled(CHEAT_TYPE_OVERSPEED_JUMP) && !IsInKnockBack())
    {
        // if the player is jumping, use the movement speed based on flags from the last movement.
        // otherwise, use this one.  note that if the player was swimming, we should also use the
        // old movement flags.  this is because the client will use the swimming speed for this
        // case, rather than the ground speed.
        auto const expected = !!(GetLastMovementInfo().moveFlags & (MOVEFLAG_JUMPING | MOVEFLAG_SWIMMING)) ?
            GetXYFlagBasedClientSpeed(GetLastMovementInfo().moveFlags) :
            GetXYFlagBasedClientSpeed(movementInfo.moveFlags);

        // XXX: these should all be temporary until the above logic is perfected
        auto const oldServerSpeed = _me->GetXYFlagBasedSpeed(GetLastMovementInfo().moveFlags);
        auto const oldClientSpeed = GetXYFlagBasedClientSpeed(GetLastMovementInfo().moveFlags);
        auto const serverSpeed = _me->GetXYFlagBasedSpeed(movementInfo.moveFlags);
        auto const clientSpeed = GetXYFlagBasedClientSpeed(movementInfo.moveFlags);

        if (movementInfo.jump.xyspeed > expected)
        {
            _anticheat->RecordCheatInternal(CHEAT_TYPE_OVERSPEED_JUMP,
                "Lateral jump speed is too high (%f > %f) flags 0x%lx -> 0x%lx old server: %f old client: %f new server: %f new client: %f opcode: %s",
                movementInfo.jump.xyspeed, expected, GetLastMovementInfo().moveFlags, movementInfo.moveFlags,
                oldServerSpeed, oldClientSpeed, serverSpeed, clientSpeed, LookupOpcodeName(opcode));

            std::string str;

            _anticheat->GetMovementDebugString(str);

            sLog.outBasic("%s", str.c_str());
        }
    }

    // Not allowed to change horizontal speed while jumping (unless it was zero)
    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_JUMP_SPEED_CHANGE) &&
        !IsInKnockBack() &&
        opcode != CMSG_MOVE_KNOCK_BACK_ACK && opcode != MSG_MOVE_STOP &&
        (movementInfo.moveFlags & (MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)) &&
        (GetLastMovementInfo().moveFlags & (MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)) &&
        movementInfo.jump.xyspeed > GetLastMovementInfo().jump.xyspeed + 0.0001f        &&
        GetLastMovementInfo().jump.xyspeed > 0.0001f &&
        !(!!((movementInfo.moveFlags | GetLastMovementInfo().moveFlags) & MOVEFLAG_WATERWALKING)
            && GetLastMovementInfo().jump.xyspeed < 0.001f))   // true when entering water with waterwalking
    {
        _anticheat->RecordCheatInternal(CHEAT_TYPE_JUMP_SPEED_CHANGE,
            "Lateral jump speed changed from %f to %f flags 0x%lx -> 0x%lx opcode %s jump zspeed: %f cos: %f sin: %f fall time: %d",
            GetLastMovementInfo().jump.xyspeed, movementInfo.jump.xyspeed, GetLastMovementInfo().moveFlags,
            movementInfo.moveFlags, LookupOpcodeName(opcode), movementInfo.jump.zspeed,
            movementInfo.jump.cosAngle, movementInfo.jump.sinAngle, movementInfo.fallTime);

        std::string str;
        DumpMovement(str);
        sLog.outBasic("Lateral check recent movements:\n%s", str.c_str());
    }

    // use the first processed movement as a client/server clock sync
    if (!_serverInitTime || !_clientInitTime)
    {
        _serverInitTime = movementInfo.stime;
        _clientInitTime = movementInfo.ctime;
    }
    else
    {
        if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_NUM_DESYNC) && !_anticheat->IsKickTimerActive() && !_anticheat->IsBanTimerActive())
        {
            constexpr float msPerMinuteTolerance = 500.f;
            auto constexpr toleranceMultiplier = msPerMinuteTolerance / (MINUTE * IN_MILLISECONDS);

            // some computers clocks move at slightly different speeds.  experimentally, the maximum
            // tolerance observed is about 140ms per minute.  however there is little gain for a speedhack
            // which accelerates the clock by merely 500ms per minute as this would resemble an increase
            // of only 0.8% movement speed.  therefore we will apply a tolerance of 500ms per minute since
            // the clock has synced.

            auto const serverDiff = WorldTimer::getMSTimeDiff(movementInfo.stime, _serverInitTime);
            auto const clientDiff = WorldTimer::getMSTimeDiff(movementInfo.ctime, _clientInitTime);
            auto const tolerance = std::max(5000.f, serverDiff * toleranceMultiplier);

            // if more time passed for us than it did for them, this is not useful as a cheat, but can happen due to
            // fluctuations in network latency
            if (clientDiff > (serverDiff + tolerance))
            {
                _anticheat->RecordCheatInternal(CHEAT_TYPE_NUM_DESYNC,
                    "Client desync exceeded tolerance.  Desync: %d server diff: %d client diff: %d tolerance: %f",
                    clientDiff - serverDiff, serverDiff, clientDiff, tolerance);
            }
        }

        // Client going back in time ... ?!
        if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_TIME_BACK) && movementInfo.ctime < GetLastMovementInfo().ctime)
        {
            // temporary measure to remove false positives
            if (GetLastMovementInfo().ctime - movementInfo.ctime < 1000)
            {
                _anticheat->RecordCheatInternal(CHEAT_TYPE_TIME_BACK, "Clock moved in reverse from %u to %u",
                    GetLastMovementInfo().ctime, movementInfo.ctime);
            }
            else
            {
                std::string log;
                _anticheat->GetMovementDebugString(log);

                sLog.outBasic("TimeBack debug output.\n%s", log.c_str());
            }
        }
    }

    // Warsong Battleground - specific checks
    if (_me->GetMapId() == 489 && sAnticheatConfig.IsEnabled(CHEAT_TYPE_FORBIDDEN))
    {
        // Too high - not allowed (but possible with some engineering items malfunction)
        if (!(movementInfo.moveFlags & (MOVEFLAG_FALLINGFAR | MOVEFLAG_JUMPING)) && movementInfo.pos.z > 380.0f)
            _anticheat->RecordCheatInternal(CHEAT_TYPE_FORBIDDEN, "Too high in Warsong Gulch (z = %f)", movementInfo.pos.z);

        if (BattleGround* bg = _me->GetBattleGround())
        {
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
            {
                // Battleground not started. Players should be in their starting areas.
                if (_me->GetTeam() == ALLIANCE && movementInfo.pos.x < 1490.0f)
                    _anticheat->RecordCheatInternal(CHEAT_TYPE_FORBIDDEN, "Outside alliance starting area (%f, %f, %f)",
                        movementInfo.pos.x, movementInfo.pos.y, movementInfo.pos.z);
                if (_me->GetTeam() == HORDE && movementInfo.pos.x > 957.0f)
                    _anticheat->RecordCheatInternal(CHEAT_TYPE_FORBIDDEN, "Outside horde starting area (%f, %f, %f)",
                        movementInfo.pos.x, movementInfo.pos.y, movementInfo.pos.z);
            }
        }
    }

    // Minimal checks on transports
    if (movementInfo.moveFlags & MOVEFLAG_ONTRANSPORT &&
        sAnticheatConfig.IsEnabled(CHEAT_TYPE_TELE_TO_TRANSPORT) &&
        GetLastMovementInfo().ctime &&
        !(GetLastMovementInfo().moveFlags & MOVEFLAG_ONTRANSPORT))
    {
        auto constexpr maxDist2d = 100.f * 100.f;

        if (dist2d > maxDist2d)
            _anticheat->RecordCheatInternal(CHEAT_TYPE_TELE_TO_TRANSPORT, "Movement distance %f", sqrtf(dist2d));
    }

    if (opcode == CMSG_MOVE_FALL_RESET && CheckFallReset(movementInfo))
        _anticheat->RecordCheatInternal(CHEAT_TYPE_BAD_FALL_RESET, "Bad fall reset");

    // Distance computation related
    if (!_me->IsTaxiFlying() &&
        !(movementInfo.moveFlags & MOVEFLAG_ONTRANSPORT) &&
        sAnticheatConfig.EnableAntiSpeedHack() &&
        !_justTeleported)
    {
        float allowedDXY = 0.0f;
        float allowedDZ = 0.0f;
        float realDistance2D_sq = 0.0f;

        auto const dt = movementInfo.ctime - GetLastMovementInfo().ctime;

        // Check vs extrapolation
        if (sAnticheatConfig.EnableExtrapolation())
        {
            Position extrap;

            // predict destination given the last movement position, direction, and flags, and compare to value reported by the client
            if (ExtrapolateMovement(GetLastMovementInfo(), dt, extrap))
            {
                auto const includeZ = !!((movementInfo.moveFlags | GetLastMovementInfo().moveFlags) & MOVEFLAG_JUMPING);

                // how far our extrapolation predicted they will go (squared)
                auto const extrapDist2 = DistanceSquared(GetLastMovementInfo().pos, extrap, includeZ);

                // how far they actually went (squared)
                auto const theirDist2 = DistanceSquared(GetLastMovementInfo().pos, movementInfo.pos, includeZ);

                // if they went further than we expected, record it
                if (theirDist2 > extrapDist2)
                {
                    constexpr float minErr = 1.f;

                    auto const extrapDist = sqrt(extrapDist2);
                    auto const theirDist = sqrt(theirDist2);

                    auto const delta = theirDist - extrapDist;

                    if (delta > extrapolationEpsilon)
                    {
                        overSpeedDistanceTick += delta;
                        overSpeedDistanceTotal += delta;
                    }

                    if (delta >= minErr)
                        if (auto const anticheat = dynamic_cast<AnticheatLib *>(GetAnticheatLib()))
                            anticheat->OfferExtrapolationData(
                                GetLastMovementInfo(),
                                clientSpeeds[GetMoveType(GetLastMovementInfo().moveFlags)],
                                clientSpeeds[GetMoveType(movementInfo.moveFlags)],
                                movementInfo, extrap, delta);
                }
            }
        }
        else if (GetMaxAllowedDist(GetLastMovementInfo(), dt, allowedDXY, allowedDZ))
        {
            // Allow some margin
            allowedDXY += 0.5f;
            allowedDZ += 0.5f;

            realDistance2D_sq = DistanceSquared(movementInfo.pos, GetLastMovementInfo().pos, false);

            if (realDistance2D_sq > allowedDXY * allowedDXY * 1.1f)
            {
                auto const dist = sqrt(realDistance2D_sq) - allowedDXY;

                if (dist > extrapolationEpsilon)
                {
                    overSpeedDistanceTick += dist;
                    overSpeedDistanceTotal += dist;
                }
            }

            if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_OVERSPEED_Z) && fabs(movementInfo.pos.z - GetLastMovementInfo().pos.z) > allowedDZ)
                _anticheat->RecordCheatInternal(CHEAT_TYPE_OVERSPEED_Z, "z changed from %f to %f, difference of %f, max allowed: %f",
                    GetLastMovementInfo().pos.z, movementInfo.pos.z, fabs(movementInfo.pos.z - GetLastMovementInfo().pos.z), allowedDZ);
        }

        // Client should send heartbeats every 500ms at the most (it can send them sooner
        // if movement flags change, i.e. entering or exiting a transport)
        if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_SKIPPED_HEARTBEATS) &&
            dt > 1000 &&
            GetLastMovementInfo().ctime &&
            GetLastMovementInfo().moveFlags & MOVEFLAG_MASK_MOVING)
            _anticheat->RecordCheatInternal(CHEAT_TYPE_SKIPPED_HEARTBEATS, "Time since last movement: %d", dt);
    }

    //GetLastMovementInfo() = movementInfo;
    //GetLastMovementInfo().UpdateTime(WorldTimer::getMSTime());
    // This is required for proper movement interpolation
    if (opcode == MSG_MOVE_JUMP)
        _jumpInitialSpeed = 7.95797334f;
    else if (opcode == MSG_MOVE_FALL_LAND)
    {
        _jumpInitialSpeed = -9.645f;
        _inKnockBack = false;
    }

    if (_justTeleported)
    {
        _moveHistory.clear();
        _justTeleported = false;
    }

    _moveHistory.push_back({ static_cast<uint16>(packet.GetOpcode()), movementInfo });

    return true;
}

bool Movement::HandleSpeedChangeAck(MovementInfo& movementInfo, WorldSession* session, const WorldPacket& packet, float newSpeed)
{
    static char const* move_type_name[MAX_MOVE_TYPE] = {"Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate"};
    UnitMoveType moveType;
    switch (packet.GetOpcode())
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:
            moveType = MOVE_WALK;
            break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:
            moveType = MOVE_RUN;
            break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:
            moveType = MOVE_RUN_BACK;
            break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:
            moveType = MOVE_SWIM;
            break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:
            moveType = MOVE_SWIM_BACK;
            break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:
            moveType = MOVE_TURN_RATE;
            break;
        default:
            return false;
    }

    // Compare to server side speed.
    if (!_me->GetTransport() && fabs(_me->GetSpeed(moveType) - newSpeed) > 0.01f)
    {
        DETAIL_LOG("%sSpeedChange player %s incorrect. %f -> %f (instead of %f)",
                       move_type_name[moveType], _me->GetName(), clientSpeeds[moveType], newSpeed, _me->GetSpeed(moveType));
        _me->SetSpeedRate(moveType, _me->GetSpeedRate(moveType), true);
    }

    // Compute anticheat generic checks - with old speed.
    HandleAnticheatTests(movementInfo, session, packet);
    clientSpeeds[moveType] = newSpeed;
    return true;
}

void Movement::HandleEnterWorld()
{
    _worldChangeHistory.push_back({ true, WorldTimer::getMSTime() });

    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        clientSpeeds[i] = _me->GetSpeed(static_cast<UnitMoveType>(i));
}

void Movement::HandleLeaveWorld()
{
    _worldChangeHistory.push_back({ false, WorldTimer::getMSTime() });

    _serverInitTime = _clientInitTime = 0;
}

void Movement::TimeSkipped(const ObjectGuid &mover, uint32 ms)
{
    _timeSkipHistory.push_back({ mover, ms, WorldTimer::getMSTime() });

    _clientInitTime += ms;

    sLog.outDebug("Time skipped: Player %s (GUID 0x%x) mover 0x%lx ms %u",
        _me ? _me->GetName() : "<none>", _me ? _me->GetGUIDLow() : 0, mover.GetRawValue(), ms);
}

MovementInfo& Movement::GetLastMovementInfo() const
{
    return _me->m_movementInfo;
}

float Movement::GetXYFlagBasedClientSpeed(uint32 moveFlags) const
{
    // swimming?
    if (!!(moveFlags & MOVEFLAG_SWIMMING))
    {
        if (!!(moveFlags & MOVEFLAG_BACKWARD))
            return clientSpeeds[MOVE_SWIM_BACK];

        return clientSpeeds[MOVE_SWIM];
    }

    // not moving laterally? zero!
    if (!(moveFlags & MOVEFLAG_MASK_XY))
        return 0.f;

    // walking?
    if (!!(moveFlags & MOVEFLAG_WALK_MODE))
    {
        // Seems to always be same speed forward and backward when walking
        return clientSpeeds[MOVE_WALK];
    }

    // Presumably only running left when IsMoving is true
    if (!!(moveFlags & MOVEFLAG_BACKWARD))
        return clientSpeeds[MOVE_RUN_BACK];

    return clientSpeeds[MOVE_RUN];
}

bool Movement::ExtrapolateMovement(MovementInfo const& mi, uint32 diffMs, Position &pos) const
{
    // TODO: These cases are not handled in movement extrapolation
    // - Transports
    if (mi.moveFlags & (MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN | MOVEFLAG_ONTRANSPORT))
        return false;
    // - Not correctly handled yet (issues regarding feather fall auras)
    if (mi.moveFlags & MOVEFLAG_FALLINGFAR)
        return false;
    // - Server side movement (should be easy to extrapolate actually)
    if (!_me->movespline->Finalized())
        return false;
    // Dernier paquet pas a jour (connexion, TP autre map ...)
    if (mi.ctime == 0)
        return false;

    // if we are not moving, position shouldn't change
    if (!(mi.moveFlags & MOVEFLAG_MASK_MOVING))
    {
        pos = mi.pos;
        return true;
    }

    pos = mi.pos;

    float o = pos.o;

    // Not allowed to move
    /*if (mi.moveFlags & MOVE_ROOT)
        return true;*/

    auto const speed = clientSpeeds[GetMoveType(mi.moveFlags)];

    if (mi.moveFlags & MOVEFLAG_BACKWARD)
        o += M_PI_F;
    else if (mi.moveFlags & MOVEFLAG_STRAFE_LEFT)
    {
        if (mi.moveFlags & MOVEFLAG_FORWARD)
            o += M_PI_F / 4;
        else
            o += M_PI_F / 2;
    }
    else if (mi.moveFlags & MOVEFLAG_STRAFE_RIGHT)
    {
        if (mi.moveFlags & MOVEFLAG_FORWARD)
            o -= M_PI_F / 4;
        else
            o -= M_PI_F / 2;
    }

    if (mi.moveFlags & MOVEFLAG_JUMPING)
    {
        float diffT = WorldTimer::getMSTimeDiff(mi.jump.startClientTime, diffMs + mi.ctime) / 1000.f;
        pos.x = mi.jump.start.x;
        pos.y = mi.jump.start.y;
        pos.z = mi.jump.start.z;
        // Fatal error. Avoid crashing here ...
        if (!pos.x || !pos.y || !pos.z || diffT > 10000.f)
            return false;
        pos.x += mi.jump.cosAngle * mi.jump.xyspeed * diffT;
        pos.y += mi.jump.sinAngle * mi.jump.xyspeed * diffT;
        pos.z -= ::Movement::computeFallElevation(diffT, mi.moveFlags & MOVEFLAG_SAFE_FALL, -_jumpInitialSpeed);
    }
    else if (mi.moveFlags & (MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT))
    {
        if (mi.moveFlags & MOVEFLAG_MASK_MOVING)
        {
            // Every 2 sec
            float T = 0.75f * (clientSpeeds[MOVE_TURN_RATE]) * (diffMs / 1000.f);
            float R = 1.295f * speed / M_PI * cos(mi.s_pitch);
            pos.z += diffMs * speed / 1000.0f * sin(mi.s_pitch);
            // Find the center of the circle we are moving on
            if (mi.moveFlags & MOVEFLAG_TURN_LEFT)
            {
                pos.x += R * cos(o + M_PI / 2);
                pos.y += R * sin(o + M_PI / 2);
                pos.o += T;
                T = T - M_PI / 2.0f;
            }
            else
            {
                pos.x += R * cos(o - M_PI / 2);
                pos.y += R * sin(o - M_PI / 2);
                pos.o -= T;
                T = -T + M_PI / 2.0f;
            }
            pos.x += R * cos(o + T);
            pos.y += R * sin(o + T);
        }
        else
        {
            float diffO = clientSpeeds[MOVE_TURN_RATE] * diffMs / 1000.f;
            if (mi.moveFlags & MOVEFLAG_TURN_LEFT)
                pos.o += diffO;
            else
                pos.o -= diffO;
            return true;
        }
    }
    else if (mi.moveFlags & MOVEFLAG_MASK_MOVING)
    {
        auto const dist = speed * diffMs / 1000.f;

        pos.x += dist * cos(o) * cos(mi.s_pitch);
        pos.y += dist * sin(o) * cos(mi.s_pitch);
        pos.z += dist * sin(mi.s_pitch);
    }
    else // If we reach here, we did not move
        return true;

    if (!MaNGOS::IsValidMapCoord(pos.x, pos.y, pos.z, pos.o))
        return false;

    if (!(mi.moveFlags & (MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR | MOVEFLAG_SWIMMING | MOVEFLAG_WATERWALKING)))
        pos.z = _me->GetMap()->GetHeight(pos.x, pos.y, pos.z);

    return _me->GetMap()->IsInLineOfSight(mi.pos.x, mi.pos.y, mi.pos.z + 0.5f, pos.x, pos.y, pos.z + 0.5f, false);
}

bool Movement::GetMaxAllowedDist(MovementInfo const& mi, uint32 diffMs, float &dxy, float &dz) const
{
    dxy = dz = 0.001f; // Epsilon

    if (mi.moveFlags & MOVEFLAG_ONTRANSPORT)
        return false;

    if (!_me->movespline->Finalized())
        return false;

    // Dernier paquet pas a jour (connexion, TP autre map ...)
    if (!mi.ctime)
        return false;

    if (!(mi.moveFlags & MOVEFLAG_MASK_MOVING))
    {
        dxy = dz = 0.f;
        return true;
    }

    // No mvt allowed
    /*if (mi.moveFlags & MOVE_ROOT || !(mi.moveFlags & MOVE_MASK_MOVING))
        return true;*/

    auto const speed = clientSpeeds[GetMoveType(mi.moveFlags)];

    if (mi.moveFlags & (MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR))
    {
        dxy = mi.jump.xyspeed / 1000 * diffMs;
        static constexpr float terminalVelocity = 60.148003f;
        static constexpr float terminalSlowFallVelocity = 7.f;
        dz = (mi.moveFlags & MOVEFLAG_SAFE_FALL) ? terminalSlowFallVelocity : terminalVelocity;
        dz = dz / 1000 * diffMs;
        return true;
    }

    // TODO: Maximum dyx/dz (max climb angle) if not swimming.
    dxy = speed / 1000 * diffMs;
    dz = speed / 1000 * diffMs;

    return true;
}

void Movement::OnExplore(AreaTableEntry const* p)
{
    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_EXPLORE))
        _anticheat->RecordCheatInternal(CHEAT_TYPE_EXPLORE);

    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_EXPLORE_HIGH_LEVEL) && static_cast<int32>(_me->GetLevel() + 10) < p->area_level)
        _anticheat->RecordCheatInternal(CHEAT_TYPE_EXPLORE_HIGH_LEVEL);
}

void Movement::HandleTeleport(const Position &)
{
    _justTeleported = true;
}

void Movement::OnTransport(Player* plMover, ObjectGuid transportGuid)
{
    // The anticheat is disabled on transports, so we need to be sure that the player is indeed on a transport.
    GameObject* transportGobj = plMover->GetMap()->GetGameObject(transportGuid);

    auto const maxDist2d = plMover->GetMapId() == 369 ? 3000.f : 70.f; // Transports usually dont go far away (exception is deeprun tram)

    if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_FAKE_TRANSPORT) &&
        (!transportGobj || !transportGobj->IsTransport() || !transportGobj->IsWithinDist(plMover, maxDist2d, false)))
        _anticheat->RecordCheatInternal(CHEAT_TYPE_FAKE_TRANSPORT);
}

bool Movement::IsTeleportAllowed(MovementInfo const& movementInfo, float& distance)
{
    // check valid source coordinates
    if (_me->GetPositionX() == 0.0f || _me->GetPositionY() == 0.0f || _me->GetPositionZ() == 0.0f)
        return true;

    // check valid destination coordinates
    if (movementInfo.GetPos().x == 0.0f || movementInfo.GetPos().y == 0.0f || movementInfo.GetPos().z == 0.0f)
        return true;

    // if knockbacked
    if (_me->IsLaunched())
        return true;

    // ignore valid teleport state
    if (_me->IsBeingTeleported())
        return true;

    // some exclude zones - lifts and other, but..
    uint32 destZoneId = 0;
    uint32 destAreaId = 0;

    _me->GetTerrain()->GetZoneAndAreaId(destZoneId, destAreaId, movementInfo.pos.x, movementInfo.pos.y, movementInfo.pos.z);

    // checks far teleports
    if (destZoneId == _me->GetZoneId() && destAreaId == _me->GetAreaId())
    {
        // Thousand Needles - Great Lift
        if (_me->GetZoneId() == 2257 || (_me->GetZoneId() == 400 && _me->GetAreaId() == 485))
            return true;

        // Undercity Lift
        if (_me->GetZoneId() == 1497 && _me->GetAreaId() == 1497)
            return true;
    }

    float deltaX = _me->GetPositionX() - movementInfo.pos.x;
    float deltaY = _me->GetPositionY() - movementInfo.pos.y;
    float deltaZ = _me->GetPositionZ() - movementInfo.pos.z;
    distance = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

    if (distance < 40.0f)
        return true;

    return false;
}

bool Movement::CheckFallReset(MovementInfo const& movementInfo) const
{
    if (!sAnticheatConfig.getConfig(CONFIG_BOOL_AC_MOVEMENT_CHEAT_BAD_FALL_RESET_ENABLED))
        return false;

    if (GetLastMovementInfo().ctime)
    {
        if (!GetLastMovementInfo().HasMovementFlag(MovementFlags(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)))
            return true;
    }

    return movementInfo.fallTime != 0 || movementInfo.jump.zspeed != 0.0f;
}

bool Movement::CheckTeleport(uint16 opcode, MovementInfo& movementInfo)
{
    if (_me->GetMover()->GetTypeId() != TYPEID_PLAYER)
        return true;

    auto const mover = reinterpret_cast<const Player *>(_me->GetMover());

    // teleport hack check
    if (!mover->GetTransport() && !mover->HasMovementFlag(MOVEFLAG_ONTRANSPORT) && !mover->IsTaxiFlying())
    {
        float distance;

        if (!IsTeleportAllowed(movementInfo, distance))
        {
            uint32 destZoneId = 0;
            uint32 destAreaId = 0;

            mover->GetTerrain()->GetZoneAndAreaId(destZoneId, destAreaId,
                movementInfo.pos.x,
                movementInfo.pos.y,
                movementInfo.pos.z);

            // get zone and area info
            MapEntry const* mapEntry = sMapStore.LookupEntry(mover->GetMapId());
            auto const srcZoneEntry = GetAreaEntryByAreaID(mover->GetZoneId());
            auto const srcAreaEntry = GetAreaEntryByAreaID(mover->GetAreaId());
            auto const destZoneEntry = GetAreaEntryByAreaID(destZoneId);
            auto const destAreaEntry = GetAreaEntryByAreaID(destAreaId);

            const char *mapName = mapEntry ? reinterpret_cast<const char *>(mapEntry->name) : "<unknown>";
            const char *srcZoneName = srcZoneEntry ? reinterpret_cast<const char *>(srcZoneEntry->area_name) : "<unknown>";
            const char *srcAreaName = srcAreaEntry ? reinterpret_cast<const char *>(srcAreaEntry->area_name) : "<unknown>";
            const char *destZoneName = destZoneEntry ? reinterpret_cast<const char *>(destZoneEntry->area_name) : "<unknown>";
            const char *destAreaName = destAreaEntry ? reinterpret_cast<const char *>(destAreaEntry->area_name) : "<unknown>";
            
            auto const cheatType = distance > 150.f ? CHEAT_TYPE_TELEPORT_FAR : CHEAT_TYPE_TELEPORT;

            if (sAnticheatConfig.IsEnabled(cheatType))
                _anticheat->RecordCheatInternal(
                    cheatType,
                    "Teleport hack: player %s, %s, %.2f yd\n"
                    "    map %u \"%s\"\n"
                    "    source: zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f\n"
                    "    dest:   zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f",
                    mover->GetName(), LookupOpcodeName(opcode), distance,
                    mover->GetMapId(), mapName,
                    mover->GetZoneId(), srcZoneName, mover->GetAreaId(), srcAreaName,
                    mover->GetPositionX(), mover->GetPositionY(), mover->GetPositionZ(),
                    destZoneId, destZoneName, destAreaId, destAreaName,
                    movementInfo.pos.x, movementInfo.pos.y, movementInfo.pos.z);

            // on gm island?
            if (sAnticheatConfig.IsEnabled(CHEAT_TYPE_FORBIDDEN) && _me->GetSession()->GetSecurity() == SEC_PLAYER && destZoneId == 876 && destAreaId == 876)
                _anticheat->RecordCheatInternal(CHEAT_TYPE_FORBIDDEN, "Player is on GM Island");

            // save prevoius point
            Player::SavePositionInDB(mover->GetObjectGuid(), mover->GetMapId(),
                mover->m_movementInfo.pos.x,
                mover->m_movementInfo.pos.y,
                mover->m_movementInfo.pos.z,
                mover->m_movementInfo.pos.o, mover->GetZoneId());

            return false;
        }
    }

    // in launched/falling/spline movement - maybe false positives
    if (!mover->IsLaunched() && (!mover->movespline || mover->movespline->Finalized()))
    {
        auto const moveFlags = movementInfo.GetMovementFlags();
        uint32 removeFlags;

        VerifyMovementFlags(moveFlags, removeFlags, true);

        if (!!removeFlags)
        {
            // XXX temporary debug logging
            sLog.outBasic("ANTICHEAT: Player %s sent bad movement flags (0x%x) with opcode %s (%u)",
                mover->GetName(), moveFlags, LookupOpcodeName(opcode), opcode);
            movementInfo.RemoveMovementFlag(MovementFlags(removeFlags));
        }
    }

    return true;
}

void Movement::SendOrderInfo(ChatHandler *handler) const
{
    handler->PSendSysMessage("Order ACKs pending: %lu Current time: %u", uint64(_orders.size()), WorldTimer::getMSTime());

    if (!_orders.empty())
    {
        static constexpr size_t maxCount = 10;

        size_t count = 0;
        for (auto const &order : _orders)
        {
            handler->PSendSysMessage("Next: %s Age: %ums", LookupOpcodeName(order.opcode), WorldTimer::getMSTime() - order.time);

            if (++count >= maxCount)
                break;
        }
    }
    
    std::string str;

    _anticheat->GetMovementDebugString(str);

    handler->PSendSysMessage("%s", str.c_str());
}

size_t Movement::DumpOrders(std::string &out) const
{
    std::stringstream str;

    size_t result = 0;

    auto const now = WorldTimer::getMSTime();

    for (auto const &order : _orderHistory)
    {
        str << LookupOpcodeName(order.opcode) << " at " << order.time
            << " ("<< WorldTimer::getMSTimeDiff(order.time, now) << "ms ago)\n";
        ++result;
    }

    out = str.str();

    return result;
}

size_t Movement::DumpAcks(std::string &out) const
{
    std::stringstream str;

    size_t result = 0;

    auto const now = WorldTimer::getMSTime();

    for (auto const &ack : _ackHistory)
    {
        str << LookupOpcodeName(ack.opcode) << " at " << ack.time << " ("
            << WorldTimer::getMSTimeDiff(ack.time, now) << "ms ago)\n";
        ++result;
    }

    out = str.str();

    return result;
}

size_t Movement::DumpTimeSkips(std::string &out) const
{
    std::stringstream str;

    size_t result = 0;

    auto const now = WorldTimer::getMSTime();

    for (auto const &skip : _timeSkipHistory)
    {
        str << skip.mover.GetCounter() << ": Skipped " << skip.ms << " ms at time "
            << skip.time << " (" << WorldTimer::getMSTimeDiff(skip.time, now) << "ms ago)\n";
        ++result;
    }

    out = str.str();

    return result;
}

size_t Movement::DumpWorldChanges(std::string &out) const
{
    std::stringstream str;

    size_t result = 0;

    for (auto const &change : _worldChangeHistory)
    {
        str << (change.enter ? "Enter" : "Leave") << " at time " << change.time << "\n";
        ++result;
    }

    out = str.str();

    return result;
}

size_t Movement::DumpMovement(std::string &out) const
{
    std::stringstream str;

    size_t result = 0;

    auto const now = WorldTimer::getMSTime();

    for (auto const &move : _moveHistory)
    {
        str << move.movementInfo.ctime << ": age " << (now - move.movementInfo.stime) << " flags 0x" << std::hex << move.movementInfo.moveFlags << std::dec
            << " at (" << move.movementInfo.pos.x << ", " << move.movementInfo.pos.y << ", " << move.movementInfo.pos.z
            << ")";

        if (move.movementInfo.HasMovementFlag(MOVEFLAG_JUMPING))
            str << " jump xy speed " << move.movementInfo.jump.xyspeed << " start ctime "
                << move.movementInfo.jump.startClientTime;

        str << " " << LookupOpcodeName(move.opcode) << "\n";

        ++result;
    }

    out = str.str();

    return result;
}
}
