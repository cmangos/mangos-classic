/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
#include "Entities/Player.h"
#include "Movement/MoveSpline.h"
#include "Maps/MapManager.h"
#include "Entities/Transports.h"
#include "BattleGround/BattleGround.h"
#include "WaypointMovementGenerator.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Globals/ObjectMgr.h"
#include "World/World.h"
#include "Anticheat/Anticheat.hpp"

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAckOpcode();
}

void WorldSession::HandleMoveWorldportAckOpcode()
{
    // ignore unexpected far teleports
    if (!GetPlayer()->IsBeingTeleportedFar())
        return;

    // get start teleport coordinates (will used later in fail case)
    WorldLocation old_loc;
    GetPlayer()->GetPosition(old_loc);

    // get the teleport destination
    WorldLocation& loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check (only cheating case possible)
    if (!MapManager::IsValidMapCoord(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation))
    {
        sLog.outError("WorldSession::HandleMoveWorldportAckOpcode: %s was teleported far to a not valid location "
                      "(map:%u, x:%f, y:%f, z:%f) We port him to his homebind instead..",
                      GetPlayer()->GetGuidStr().c_str(), loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z);
        // stop teleportation else we would try this again and again in LogoutPlayer...
        GetPlayer()->SetSemaphoreTeleportFar(false);
        // and teleport the player to a valid place
        GetPlayer()->TeleportToHomebind();
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.mapid);
    
    auto returnHomeFunc = [this, player = GetPlayer(), old_loc, loc]()
    {
        Map* map = nullptr;
        // must have map in teleport
        if (!map)
            map = sMapMgr.CreateMap(loc.mapid, player);
        if (!map)
            map = sMapMgr.CreateMap(old_loc.mapid, player);

        if (!map)
        {
            KickPlayer();
            return;
        }

        player->SetMap(map);

        player->SetSemaphoreTeleportFar(false);

        // Teleport to previous place, if cannot be ported back TP to homebind place
        if (!player->TeleportTo(old_loc))
        {
            DETAIL_LOG("WorldSession::HandleMoveWorldportAckOpcode: %s cannot be ported to his previous place, teleporting him to his homebind place...",
                player->GetGuidStr().c_str());
            player->TeleportToHomebind();
        }
    };

    Map* map = nullptr;

    // prevent crash at attempt landing to not existed battleground instance
    if (mEntry->IsBattleGround())
    {
        if (GetPlayer()->GetBattleGroundId())
            map = sMapMgr.FindMap(loc.mapid, GetPlayer()->GetBattleGroundId());

        if (!map)
        {
            DETAIL_LOG("WorldSession::HandleMoveWorldportAckOpcode: %s was teleported far to nonexisten battleground instance "
                       " (map:%u, x:%f, y:%f, z:%f) Trying to port him to his previous place..",
                       GetPlayer()->GetGuidStr().c_str(), loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z);

            returnHomeFunc();
            return;
        }
    }

    uint32 miscRequirement = 0;
    AreaTrigger const* at = sObjectMgr.GetMapEntranceTrigger(loc.mapid);
    if (at)
    {
        if (AREA_LOCKSTATUS_OK != GetPlayer()->GetAreaTriggerLockStatus(at, miscRequirement))
        {
            returnHomeFunc();
            return;
        }
    }

    InstanceTemplate const* mInstance = ObjectMgr::GetInstanceTemplate(loc.mapid);

    // reset instance validity, except if going to an instance inside an instance
    if (!GetPlayer()->m_InstanceValid && !mInstance)
        GetPlayer()->m_InstanceValid = true;

    GetPlayer()->SetSemaphoreTeleportFar(false);

    // relocate the player to the teleport destination
    if (!map)
        map = sMapMgr.CreateMap(loc.mapid, GetPlayer());

    // if dead player is entering an instance of same id but corpse is not found, likely means entering different instance id
    if (GetPlayer()->IsDelayedResurrect() && !map->GetCorpse(GetPlayer()->GetObjectGuid()) && at)
    {
        // respawn at entrance
        loc.coord_x = at->target_X;
        loc.coord_y = at->target_Y;
        loc.coord_z = at->target_Z;
    }

    GetPlayer()->SetMap(map);
    bool found = true;
    if (GetPlayer()->m_teleportTransport)
    {
        found = false;
        GetPlayer()->m_movementInfo.t_pos = Position(loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation); // when teleporting onto transport, position is local coords
        if (GenericTransport* transport = map->GetTransport(GetPlayer()->m_teleportTransport))
            if (transport->GetMapId() == loc.mapid)
                found = true;
    }
    else
        GetPlayer()->Relocate(loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation);
    auto lambda = [this, loc, old_loc, mEntry, mInstance](Map* map)
    {
        if (GenericTransport* transport = map->GetTransport(GetPlayer()->m_teleportTransport))
        {
            if (transport->GetMapId() == loc.mapid)
            {
                transport->AddPassenger(GetPlayer(), false);
                transport->UpdatePassengerPosition(GetPlayer());
            }
        }
        GetPlayer()->m_teleportTransport = ObjectGuid();

        GetPlayer()->SendInitialPacketsBeforeAddToMap();
        // the CanEnter checks are done in TeleporTo but conditions may change
        // while the player is in transit, for example the map may get full
        if (!map->Add(GetPlayer()))
        {
            // if player wasn't added to map, reset his map pointer!
            GetPlayer()->ResetMap();

            DETAIL_LOG("WorldSession::HandleMoveWorldportAckOpcode: %s was teleported far but couldn't be added to map "
                " (map:%u, x:%f, y:%f, z:%f) Trying to port him to his previous place..",
                GetPlayer()->GetGuidStr().c_str(), loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z);

            // Teleport to previous place, if cannot be ported back TP to homebind place
            if (!GetPlayer()->TeleportTo(old_loc))
            {
                DETAIL_LOG("WorldSession::HandleMoveWorldportAckOpcode: %s cannot be ported to his previous place, teleporting him to his homebind place...",
                    GetPlayer()->GetGuidStr().c_str());
                GetPlayer()->TeleportToHomebind();
            }
            return;
        }

        // battleground state prepare (in case join to BG), at relogin/tele player not invited
        // only add to bg group and object, if the player was invited (else he entered through command)
        if (_player->InBattleGround())
        {
            // cleanup seting if outdated
            if (!mEntry->IsBattleGround())
            {
                // We're not in BG
                _player->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);
                // reset destination bg team
                _player->SetBGTeam(TEAM_NONE);
            }
            // join to bg case
            else if (BattleGround* bg = _player->GetBattleGround())
            {
                if (_player->IsInvitedForBattleGroundInstance(_player->GetBattleGroundId()))
                    bg->AddPlayer(_player);
            }
        }

        m_anticheat->Teleport({ loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation });

        GetPlayer()->SendInitialPacketsAfterAddToMap(false);

        // flight fast teleport case
        if (_player->InBattleGround())
            _player->TaxiFlightInterrupt();
        else
            _player->TaxiFlightResume();

        if (mEntry->IsRaid() && mInstance)
        {
            if (time_t timeReset = sMapPersistentStateMgr.GetScheduler().GetResetTimeFor(mEntry->MapID))
            {
                uint32 timeleft = uint32(timeReset - time(nullptr));
                GetPlayer()->SendInstanceResetWarning(mEntry->MapID, timeleft);
            }
        }

        // mount allow check
        if (!map->IsMountAllowed())
            _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        _player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_WORLD);

        // honorless target
        if (GetPlayer()->pvpInfo.inPvPEnforcedArea)
            GetPlayer()->CastSpell(GetPlayer(), 2479, TRIGGERED_OLD_TRIGGERED);

        _player->ResummonPetTemporaryUnSummonedIfAny();

        // lets process all delayed operations on successful teleport
        GetPlayer()->ProcessDelayedOperations();

        // notify group after successful teleport
        if (_player->GetGroup())
            _player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
    };
    if (found)
        lambda(map);
    else
        map->GetMessager().AddMessage(lambda);
}

void WorldSession::HandleMoveTeleportAckOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("MSG_MOVE_TELEPORT_ACK");

    ObjectGuid guid;

    recv_data >> guid;

    uint32 counter, time;
    recv_data >> counter >> time;
    DEBUG_LOG("Guid: %s", guid.GetString().c_str());
    DEBUG_LOG("Counter %u, time %u", counter, time / IN_MILLISECONDS);

    Unit* mover = _player->GetMover();
    Player* plMover = mover->GetTypeId() == TYPEID_PLAYER ? (Player*)mover : nullptr;

    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    if (guid != plMover->GetObjectGuid())
        return;

    m_anticheat->OrderAck(recv_data.GetOpcode(), counter);

    plMover->SetSemaphoreTeleportNear(false);

    uint32 old_zone = plMover->GetZoneId();

    WorldLocation const& dest = plMover->GetTeleportDest();

#ifdef ENABLE_PLAYERBOTS
    // send MSG_MOVE_TELEPORT to observers around old position
    SendTeleportToObservers(dest.coord_x, dest.coord_y, dest.coord_z, dest.orientation);

    // interrupt moving for bot if any
    if (plMover->GetPlayerbotAI() && !plMover->GetMotionMaster()->empty())
    {
        if (MovementGenerator* movgen = plMover->GetMotionMaster()->top())
            movgen->Interrupt(*plMover);
    }
#endif

    plMover->SetDelayedZoneUpdate(false, 0);

    plMover->SetPosition(dest.coord_x, dest.coord_y, dest.coord_z, dest.orientation, true);

#ifdef ENABLE_PLAYERBOTS
    plMover->m_movementInfo.ChangePosition(dest.coord_x, dest.coord_y, dest.coord_z, dest.orientation);
#endif

    plMover->SetFallInformation(0, dest.coord_z);

    GenericTransport* currentTransport = nullptr;
    if (plMover->m_teleportTransport)
        currentTransport = plMover->GetMap()->GetTransport(plMover->m_teleportTransport);
    if (currentTransport)
        currentTransport->AddPassenger(plMover);
    plMover->m_teleportTransport = ObjectGuid();

    uint32 newzone, newarea;
    plMover->GetZoneAndAreaId(newzone, newarea);

#ifdef ENABLE_PLAYERBOTS
    // new zone
    if (old_zone != newzone)
        plMover->UpdateZone(newzone, newarea);

    // honorless target
    if (plMover->pvpInfo.inPvPEnforcedArea)
        plMover->CastSpell(plMover, 2479, TRIGGERED_OLD_TRIGGERED);

    // reset moving for bot if any
    if (plMover->GetPlayerbotAI() && !plMover->GetMotionMaster()->empty())
    {
        if (MovementGenerator* movgen = plMover->GetMotionMaster()->top())
            movgen->Reset(*plMover);
    }

    // send MSG_MOVE_TELEPORT to observers around new position
    SendTeleportToObservers(dest.coord_x, dest.coord_y, dest.coord_z, dest.orientation);
#else
    plMover->UpdateZone(newzone, newarea);

    // new zone
    if (old_zone != newzone)
    {
        // honorless target
        if (plMover->pvpInfo.inPvPEnforcedArea)
            plMover->CastSpell(plMover, 2479, TRIGGERED_OLD_TRIGGERED);
    }
#endif

    m_anticheat->Teleport({ dest.coord_x, dest.coord_y, dest.coord_z, dest.orientation });

    // resummon pet
    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();

    // lets process all delayed operations on successful teleport
    GetPlayer()->ProcessDelayedOperations();
}

void WorldSession::HandleMovementOpcodes(WorldPacket& recv_data)
{
    Opcodes opcode = recv_data.GetOpcode();
    if (!sLog.HasLogFilter(LOG_FILTER_PLAYER_MOVES))
    {
        DEBUG_LOG("WORLD: Received opcode %s (%u, 0x%X)", LookupOpcodeName(opcode), opcode, opcode);
        recv_data.hexlike();
    }

    Unit* mover = _player->GetMover();
    Player* plMover = mover->GetTypeId() == TYPEID_PLAYER ? (Player*)mover : nullptr;

    /* extract packet */
    MovementInfo movementInfo;
    recv_data >> movementInfo;
    /*----------------*/

    if (!ProcessMovementInfo(movementInfo, mover, plMover, recv_data))
        return;

    // CMSG opcode has no handler in client, should not be sent to others.
    // It is sent by client when you jump and hit something on the way up,
    // thus stopping upward movement and causing you to descend sooner.
    if (opcode == CMSG_MOVE_FALL_RESET)
        return;

    WorldPacket data(opcode, recv_data.size());
    data << mover->GetPackGUID();             // write guid
    movementInfo.Write(data);                               // write data
    mover->SendMessageToSetExcept(data, _player);
}

void WorldSession::HandleForceSpeedChangeAckOpcodes(WorldPacket& recv_data)
{
    uint16 opcode = recv_data.GetOpcode();
    DEBUG_LOG("WORLD: Received %s (%u, 0x%X) opcode", recv_data.GetOpcodeName(), opcode, opcode);

    /* extract packet */
    ObjectGuid guid;
    uint32 counter;
    MovementInfo movementInfo;
    float  newspeed;

    recv_data >> guid;
    recv_data >> counter;
    recv_data >> movementInfo;
    recv_data >> newspeed;

    // we must inform the anticheat even if it is not for the mover, as it will affect pets
    m_anticheat->OrderAck(opcode, counter);
    /*----------------*/

    // Process anticheat checks, remember client-side speed ...
    if (_player->IsSelfMover() && !m_anticheat->SpeedChangeAck(movementInfo, recv_data, newspeed))
        return;

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitMoveType move_type;
    UnitMoveType force_move_type;

    static char const* move_type_name[MAX_MOVE_TYPE] = {  "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate" };

    switch (opcode)
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:          move_type = MOVE_WALK;          force_move_type = MOVE_WALK;        break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:           move_type = MOVE_RUN;           force_move_type = MOVE_RUN;         break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:      move_type = MOVE_RUN_BACK;      force_move_type = MOVE_RUN_BACK;    break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:          move_type = MOVE_SWIM;          force_move_type = MOVE_SWIM;        break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:     move_type = MOVE_SWIM_BACK;     force_move_type = MOVE_SWIM_BACK;   break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:           move_type = MOVE_TURN_RATE;     force_move_type = MOVE_TURN_RATE;   break;
        default:
            sLog.outError("WorldSession::HandleForceSpeedChangeAck: Unknown move type opcode: %u", opcode);
            return;
    }

    Unit* mover = _player->GetMover();

    if (!ProcessMovementInfo(movementInfo, mover, _player, recv_data))
        return;

    const SpeedOpcodePair& speedOpcodes = SetSpeed2Opc_table[move_type];
    WorldPacket data(speedOpcodes[2], 18);
    data << guid.WriteAsPacked();
    data << movementInfo;
    data << newspeed;
    mover->SendMessageToSetExcept(data, _player);

    // skip all forced speed changes except last and unexpected
    // in run/mounted case used one ACK and it must be skipped.m_forced_speed_changes[MOVE_RUN} store both.
    if (_player->m_forced_speed_changes[force_move_type] > 0)
    {
        --_player->m_forced_speed_changes[force_move_type];
        if (_player->m_forced_speed_changes[force_move_type] > 0)
            return;
    }

    if (!_player->GetTransport() && fabs(mover->GetSpeed(move_type) - newspeed) > 0.01f)
    {
        if (mover->GetSpeed(move_type) > newspeed)        // must be greater - just correct
        {
            sLog.outError("%sSpeedChange player %s is NOT correct (must be %f instead %f), force set to correct value",
                          move_type_name[move_type], _player->GetName(), mover->GetSpeed(move_type), newspeed);
            mover->SetSpeedRate(move_type, _player->GetSpeedRate(move_type), true);
        }
        else                                                // must be lesser - cheating
        {
            BASIC_LOG("Player %s from account id %u kicked for incorrect speed (must be %f instead %f)",
                      _player->GetName(), _player->GetSession()->GetAccountId(), mover->GetSpeed(move_type), newspeed);
            _player->GetSession()->KickPlayer();
        }
    }
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_SET_ACTIVE_MOVER");

    ObjectGuid guid;
    recv_data >> guid;

    m_anticheat->Movement(_player->GetMover()->m_movementInfo, recv_data);

    if (_player->GetMover()->GetObjectGuid() != guid)
    {
        sLog.outError("HandleSetActiveMoverOpcode: incorrect mover guid: mover is %s and should be %s",
                      _player->GetMover()->GetGuidStr().c_str(), guid.GetString().c_str());
    }
}

void WorldSession::HandleMoveNotActiveMoverOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_MOVE_NOT_ACTIVE_MOVER");
    recv_data.hexlike();

    ObjectGuid old_mover_guid;
    MovementInfo mi;

    recv_data >> old_mover_guid;
    recv_data >> mi;

    if (_player->IsClientControlled() && _player->GetMover() && _player->GetMover()->GetObjectGuid() == old_mover_guid)
    {
        sLog.outError("HandleMoveNotActiveMover: incorrect mover guid: mover is %s and should be %s instead of %s",
                      _player->GetMover()->GetGuidStr().c_str(),
                      _player->GetGuidStr().c_str(),
                      old_mover_guid.GetString().c_str());
        recv_data.rpos(recv_data.wpos());                   // prevent warnings spam
        return;
    }

    if (!_player->IsTaxiFlying())
        _player->m_movementInfo = mi;
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvdata*/)
{
    // DEBUG_LOG("WORLD: Received opcode CMSG_MOUNTSPECIAL_ANIM");

    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << GetPlayer()->GetObjectGuid();

    GetPlayer()->SendMessageToSet(data, false);
}

void WorldSession::HandleMoveKnockBackAck(WorldPacket& recv_data)
{
    DEBUG_LOG("CMSG_MOVE_KNOCK_BACK_ACK");

    Unit* mover = _player->GetMover();

    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (mover->IsPlayer() && static_cast<Player*>(mover)->IsBeingTeleported())
    {
        recv_data.rpos(recv_data.wpos());                   // prevent warnings spam
        return;
    }

    ObjectGuid guid;
    uint32 counter;
    MovementInfo movementInfo;

    recv_data >> guid;
    recv_data >> counter;
    recv_data >> movementInfo;

    m_anticheat->OrderAck(recv_data.GetOpcode(), counter);

    if (!ProcessMovementInfo(movementInfo, mover, _player, recv_data))
        return;

    if (mover->IsPlayer() && static_cast<Player*>(mover)->IsFreeFlying())
        mover->SetCanFly(true);

    WorldPacket data(MSG_MOVE_KNOCK_BACK, recv_data.size() + 15);
    data << mover->GetPackGUID();
    data << movementInfo;
    data << movementInfo.jump.cosAngle;
    data << movementInfo.jump.sinAngle;
    data << movementInfo.jump.xyspeed;
    data << movementInfo.jump.zspeed;
    mover->SendMessageToSetExcept(data, _player);
}

void WorldSession::SendKnockBack(Unit* who, float angle, float horizontalSpeed, float verticalSpeed)
{
    GetPlayer()->SetLaunched(true);
    float vsin = sin(angle);
    float vcos = cos(angle);

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 9 + 4 + 4 + 4 + 4 + 4);
    data << who->GetPackGUID();
    data << GetOrderCounter();
    data << float(vcos);                                // x direction
    data << float(vsin);                                // y direction
    data << float(horizontalSpeed);                     // Horizontal speed
    data << float(-verticalSpeed);                      // Z Movement speed (vertical)
    SendPacket(data);
    IncrementOrderCounter();

    m_anticheat->KnockBack(horizontalSpeed, -verticalSpeed, vcos, vsin);
}

#ifdef ENABLE_PLAYERBOTS
void WorldSession::SendTeleportToObservers(float x, float y, float z, float orientation)
{
    WorldPacket data(MSG_MOVE_TELEPORT, 64);
    data << _player->GetPackGUID();
    // copy move info to change position to where player is teleporting
    MovementInfo moveInfo = _player->m_movementInfo;
    moveInfo.ChangePosition(x, y, z, orientation);
    data << moveInfo;
    _player->SendMessageToSetExcept(data, _player);
}
#endif

void WorldSession::HandleMoveFlagChangeOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("%s", recv_data.GetOpcodeName());

    ObjectGuid guid;
    uint32 counter;
    MovementInfo movementInfo;
    uint32 isApplied;

    recv_data >> guid;
    recv_data >> counter;
    recv_data >> movementInfo;
    recv_data >> isApplied;

    m_anticheat->OrderAck(recv_data.GetOpcode(), counter);

    Unit* mover = _player->GetMover();

    if (!ProcessMovementInfo(movementInfo, mover, _player, recv_data))
        return;

    Opcodes response = MSG_NULL_ACTION;

    switch (recv_data.GetOpcode())
    {
        case CMSG_MOVE_HOVER_ACK: response = MSG_MOVE_HOVER; break;
        case CMSG_MOVE_FEATHER_FALL_ACK: response = MSG_MOVE_FEATHER_FALL; break;
        case CMSG_MOVE_WATER_WALK_ACK: response = MSG_MOVE_WATER_WALK; break;
        default: response = MSG_NULL_ACTION; break;
    }

    WorldPacket data(response, 8);
    data << guid.WriteAsPacked();
    data << movementInfo;
    mover->SendMessageToSetExcept(data, _player);
}

void WorldSession::HandleMoveRootAck(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode %s", recv_data.GetOpcodeName());
    // Pre-Wrath: broadcast root
    ObjectGuid guid;
    uint32 counter;
    MovementInfo movementInfo;
    recv_data >> guid;
    recv_data >> counter;
    recv_data >> movementInfo;

    m_anticheat->OrderAck(recv_data.GetOpcode(), counter);

    Unit* mover = _player->GetMover();

    if (mover->GetObjectGuid() != guid)
        return;

    if (recv_data.GetOpcode() == CMSG_FORCE_MOVE_UNROOT_ACK) // unroot case
    {
        if (!mover->m_movementInfo.HasMovementFlag(MOVEFLAG_ROOT))
            return;
    }
    else // root case
    {
        if (mover->m_movementInfo.HasMovementFlag(MOVEFLAG_ROOT))
            return;
    }

    if (!ProcessMovementInfo(movementInfo, mover, _player, recv_data))
        return;

    WorldPacket data(recv_data.GetOpcode() == CMSG_FORCE_MOVE_UNROOT_ACK ? MSG_MOVE_UNROOT : MSG_MOVE_ROOT);
    data << guid.WriteAsPacked();
    data << movementInfo;
    mover->SendMessageToSetExcept(data, _player);
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket& recv_data)
{
    if (!_player->IsAlive() || _player->IsInCombat())
        return;

    ObjectGuid summonerGuid;
    recv_data >> summonerGuid;

    _player->SummonIfPossible(true, summonerGuid);
}

bool WorldSession::VerifyMovementInfo(MovementInfo const& movementInfo, Unit* mover, bool unroot) const
{
    // ignore wrong guid (player attempt cheating own session for not own guid possible...)
    if (mover->GetObjectGuid() != _player->GetMover()->GetObjectGuid())
        return false;

    if (!MaNGOS::IsValidMapCoord(movementInfo.GetPos().x, movementInfo.GetPos().y, movementInfo.GetPos().z, movementInfo.GetPos().o))
        return false;

    // rooted mover sent packet without root or moving AND root - ignore, due to client crash possibility
    if (!unroot)
        if (mover->IsRooted() && (!movementInfo.HasMovementFlag(MOVEFLAG_ROOT) || movementInfo.HasMovementFlag(MOVEFLAG_MASK_MOVING)))
            return false;

    if (movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        // transports size limited
        // (also received at zeppelin/lift leave by some reason with t_* as absolute in continent coordinates, can be safely skipped)
        if (movementInfo.GetTransportPos().x > 50 || movementInfo.GetTransportPos().y > 50 || movementInfo.GetTransportPos().z > 100)
            return false;

        if (!MaNGOS::IsValidMapCoord(movementInfo.GetPos().x + movementInfo.GetTransportPos().x, movementInfo.GetPos().y + movementInfo.GetTransportPos().y,
                                     movementInfo.GetPos().z + movementInfo.GetTransportPos().z, movementInfo.GetPos().o + movementInfo.GetTransportPos().o))
        {
            return false;
        }
    }

    return true;
}

void WorldSession::HandleMoverRelocation(MovementInfo& movementInfo)
{
    SynchronizeMovement(movementInfo);

    Unit* mover = _player->GetMover();

    if (Player* plMover = mover->GetTypeId() == TYPEID_PLAYER ? (Player*)mover : nullptr)
    {
        plMover->m_movementInfo = movementInfo;
        if (plMover->m_movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            if (!plMover->m_transport)
                if (GenericTransport* transport = plMover->GetMap()->GetTransport(movementInfo.GetTransportGuid()))
                    transport->AddPassenger(plMover);
        }
        else if (plMover->m_transport)               // if we were on a transport, leave
        {
            plMover->m_transport->RemovePassenger(plMover);
            plMover->m_transport = nullptr;
            plMover->m_movementInfo.ClearTransportData();
        }

        plMover->SetPosition(movementInfo.GetPos().x, movementInfo.GetPos().y, movementInfo.GetPos().z, movementInfo.GetPos().o);

        if (movementInfo.GetPos().z < -500.0f)
        {
            if (plMover->GetBattleGround()
                    && plMover->GetBattleGround()->HandlePlayerUnderMap(_player))
            {
                // do nothing, the handle already did if returned true
            }
            else
            {
                // NOTE: this is actually called many times while falling
                // even after the player has been teleported away
                // TODO: discard movement packets after the player is rooted
                if (plMover->IsAlive())
                {
                    plMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, plMover->GetMaxHealth());
                    // pl can be alive if GM/etc
                    if (!plMover->IsAlive())
                    {
                        // change the death state to CORPSE to prevent the death timer from
                        // starting in the next player update
                        plMover->KillPlayer();
                        plMover->BuildPlayerRepop();
                    }
                }

                // cancel the death timer here if started
                plMover->RepopAtGraveyard();
            }
        }
    }
    else                                                    // creature charmed
    {
        if (mover->IsInWorld())
            mover->GetMap()->CreatureRelocation((Creature*)mover, movementInfo.GetPos().x, movementInfo.GetPos().y, movementInfo.GetPos().z, movementInfo.GetPos().o);
    }
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket& recv_data)
{
    /*  WorldSession::Update( WorldTimer::getMSTime() );*/
    DEBUG_LOG("WORLD: Received opcode CMSG_MOVE_TIME_SKIPPED");

    ObjectGuid guid;
    uint32 timeSkipped;
    recv_data >> guid;
    recv_data >> timeSkipped;

    Unit* mover = _player->GetMover();

    // Ignore updates not for current player
    if (mover == nullptr || guid != mover->GetObjectGuid())
        return;

    mover->m_movementInfo.stime += timeSkipped;
    mover->m_movementInfo.ctime += timeSkipped;
    m_anticheat->TimeSkipped(guid, timeSkipped);

    // Send to other players
    WorldPacket data(MSG_MOVE_TIME_SKIPPED, 16);
    data << mover->GetPackGUID();
    data << timeSkipped;
    mover->SendMessageToSetExcept(data, _player);
}

bool WorldSession::ProcessMovementInfo(MovementInfo& movementInfo, Unit* mover, Player* plMover, WorldPacket& recv_data)
{
    if (plMover && plMover->IsBeingTeleported())
        return false;

    if (!VerifyMovementInfo(movementInfo, mover, recv_data.GetOpcode() == CMSG_FORCE_MOVE_UNROOT_ACK))
        return false;

    if (!mover->movespline->Finalized())
        return false;

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND && plMover && !plMover->IsTaxiFlying())
        plMover->HandleFall(movementInfo);

    if (!m_anticheat->Movement(movementInfo, recv_data))
        return false;

    if (movementInfo.GetMovementFlags() & MOVEFLAG_MASK_MOVING_OR_TURN)
    {
        if (mover->IsSitState())
            mover->SetStandState(UNIT_STAND_STATE_STAND);
        mover->HandleEmoteState(0);
    }

    /* process position-change */
    HandleMoverRelocation(movementInfo);

    // just landed from a knockback? update status
    if (plMover && plMover->IsLaunched() && (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND || recv_data.GetOpcode() == MSG_MOVE_START_SWIM))
        plMover->SetLaunched(false);

    if (plMover && recv_data.GetOpcode() != CMSG_MOVE_KNOCK_BACK_ACK)
        plMover->UpdateFallInformationIfNeed(movementInfo, recv_data.GetOpcode());

    return true;
}
