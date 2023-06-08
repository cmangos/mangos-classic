/*
 * Copyright (C) 2010-2013 Anathema Engine project <http://valkyrie-wow.com/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2016-2017 Elysium Project <https://www.elysium-project.org>
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 */

#ifndef __MOVEMENT_HPP_
#define __MOVEMENT_HPP_

#include "Common.h"
#include "Entities/Unit.h"
#include "../libanticheat.hpp"
#include "../cyclic.hpp"

#include <vector>
#include <map>
#include <deque>

class ChatHandler;
class Player;
class WorldSession;
class WorldPacket;
struct AreaTableEntry;

namespace NamreebAnticheat
{
struct PendingOrder
{
    uint16 opcode;
    uint32 counter;
    uint32 time;
};

class Movement
{
    private:
        SessionAnticheat * const _anticheat;
        Player * const _me;

        float _totalDistanceTraveled;

        // true when the previous tick of anticheat tests was aborted due to the session controlling a unit other than its own player (mind control)
        bool _wasMovingOther;

        // true when the client has acknowledged a teleport after the last movement packet was received
        bool _justTeleported;

        // movement orders sent to the client, acknowledgement still pending
        std::deque<PendingOrder> _orders;

        // keep track of the client clock in relation to our own.  some speed hacks function by accelerating the client clock.
        // if more time passes for the client than for us then it is probably a hack as this should not happen under normal circumstances.
        uint32 _serverInitTime;
        uint32 _clientInitTime;

        float _jumpInitialSpeed;
        bool _inKnockBack;

        struct Order
        {
            uint16 opcode;
            uint32 time;
        };
        nam::cyclic<Order, 15> _orderHistory;

        struct Ack
        {
            uint16 opcode;
            uint32 time;
        };
        nam::cyclic<Ack, 15> _ackHistory;

        struct TimeSkip
        {
            ObjectGuid mover;
            uint32 ms;
            uint32 time;
        };
        nam::cyclic<TimeSkip, 15> _timeSkipHistory;

        struct WorldChange
        {
            bool enter;
            uint32 time;
        };
        nam::cyclic<WorldChange, 15> _worldChangeHistory;

        struct MovementHistory
        {
            uint16 opcode;
            MovementInfo movementInfo;
        };
        nam::cyclic<MovementHistory, 45> _moveHistory;

        MovementInfo& GetLastMovementInfo() const;

        // this is somewhat redundant with Unit::GetXYFlagBasedSpeed() but uses our understanding of what the client should have for speed
        float GetXYFlagBasedClientSpeed(uint32 moveFlags) const;

        bool IsTeleportAllowed(MovementInfo const& movementInfo, float& distance);

        bool CheckFallReset(MovementInfo const& movementInfo) const;

    public:
        float clientSpeeds[MAX_MOVE_TYPE];

        // in this tick of the anticheat, how far have they traveled past what we expected
        float overSpeedDistanceTick;

        // over all since entering the world, how far have they traveled past what we expected
        float overSpeedDistanceTotal;

        explicit Movement(Player* me);

        float TotalDistanceTraveled() const { return _totalDistanceTraveled; }

        bool IsInKnockBack() const { return _inKnockBack; }
        void KnockBack(float speedxy, float speedz, float cos, float sin);

        bool HandleAnticheatTests(MovementInfo& movementInfo, WorldSession* session, const WorldPacket& packet);
        bool HandleSpeedChangeAck(MovementInfo& movementInfo, WorldSession* session, const WorldPacket& packet, float newSpeed);
        void HandleEnterWorld();
        void HandleLeaveWorld();
        void TimeSkipped(const ObjectGuid &mover, uint32 ms);
        void HandleTeleport(const Position &pos);

        // verify movement flags.  when strict is true, it will include considering pending orders.  this is optional because warden also
        // uses this function, but the results will be delayed
        void VerifyMovementFlags(uint32 flags, uint32 &removeFlags, bool strict) const;

        void OrderSent(uint16 opcode, uint32 counter);
        void OrderAck(uint16 opcode, uint32 counter);
        void CheckExpiredOrders(uint32 latency);

        bool ExtrapolateMovement(MovementInfo const& mi, uint32 diffMs, Position &pos) const;
        bool GetMaxAllowedDist(MovementInfo const& mi, uint32 diffMs, float &dxy, float &dz) const;
        void OnExplore(AreaTableEntry const* p);
        void OnTransport(Player* plMover, ObjectGuid transportGuid);

        bool CheckTeleport(uint16 opcode, MovementInfo& movementInfo);

        // commands
        void SendOrderInfo(ChatHandler *handler) const;

        size_t PendingOrderCount() const { return _orders.size(); }

        // debug logging
        size_t DumpOrders(std::string &out) const;
        size_t DumpAcks(std::string &out) const;
        size_t DumpTimeSkips(std::string &out) const;
        size_t DumpWorldChanges(std::string &out) const;
        size_t DumpMovement(std::string &out) const;
};
}

#endif /*!__MOVEMENT_HPP*/
