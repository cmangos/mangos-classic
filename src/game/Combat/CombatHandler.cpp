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
#include "Log/Log.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Player.h"

void WorldSession::HandleAttackSwingOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;

    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "WORLD: Received opcode CMSG_ATTACKSWING %s", guid.GetString().c_str());

    if (!guid.IsUnit())
    {
        sLog.outError("WORLD: %s isn't unit", guid.GetString().c_str());
        return;
    }

    Unit* enemy = _player->GetMap()->GetUnit(guid);

    if (!enemy)
    {
        sLog.outError("WORLD: %s isn't unit", guid.GetString().c_str());
        return;
    }

    Unit* mover = _player->GetMover();

    if (!mover->CanAttackNow(enemy))
    {
        // stop attack state at client
        mover->SendMeleeAttackStop(enemy);
        return;
    }

    mover->Attack(enemy, true);

    // each new attack shows its swing error again, even the one last sent
    _player->SwingErrorMsg(SWING_ERROR_NONE);
}

void WorldSession::HandleAttackStopOpcode(WorldPacket& /*recv_data*/)
{
    Player* player = GetPlayer();
    // the client keeps its swing until an SMSG_ATTACKSTOP names it as attacker, so a stop without melee is answered with no victim
    bool const meleeOn = player->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && player->GetVictim();
    player->AttackStop(false, false, false, true);
    if (!meleeOn)
        player->SendMeleeAttackStop(nullptr);
}

void WorldSession::HandleSetSheathedOpcode(WorldPacket& recv_data)
{
    uint32 sheathed;
    recv_data >> sheathed;

    DEBUG_LOG("WORLD: Received opcode CMSG_SETSHEATHED for %s - value: %u", GetPlayer()->GetGuidStr().c_str(), sheathed);

    if (sheathed >= MAX_SHEATH_STATE)
    {
        sLog.outError("Unknown sheath state %u ??", sheathed);
        return;
    }

    GetPlayer()->SetSheath(SheathState(sheathed));
}
