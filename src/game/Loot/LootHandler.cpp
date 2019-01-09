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
#include "WorldPacket.h"
#include "Log.h"
#include "Entities/Player.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Server/WorldSession.h"
#include "Loot/LootMgr.h"
#include "Entities/Object.h"
#include "Groups/Group.h"

void WorldSession::HandleAutostoreLootItemOpcode(WorldPacket& recv_data)
{
    uint8 itemSlot;
    recv_data >> itemSlot;

    DEBUG_LOG("WORLD: CMSG_AUTOSTORE_LOOT_ITEM > requesting item in slot %u", uint32(itemSlot));

    Loot* loot = sLootMgr.GetLoot(_player);

    if (!loot)
    {
        sLog.outError("HandleAutostoreLootItemOpcode> Cannot retrieve loot for player %s", _player->GetGuidStr().c_str());
        return;
    }

    ObjectGuid const& lguid = loot->GetLootGuid();

    LootItem* lootItem = loot->GetLootItemInSlot(itemSlot);

    if (!lootItem)
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        loot->Release(_player);
        return;
    }

    // item may be blocked by roll system or already looted or another cheating possibility
    if (lootItem->isBlocked || lootItem->GetSlotTypeForSharedLoot(_player, loot) == MAX_LOOT_SLOT_TYPE)
    {
        sLog.outError("HandleAutostoreLootItemOpcode> %s have no right to loot itemId(%u)", _player->GetGuidStr().c_str(), lootItem->itemId);
        loot->Release(_player);
        return;
    }

    InventoryResult result = loot->SendItem(_player, lootItem);

    if (result == EQUIP_ERR_OK && lguid.IsItem())
    {
        if (Item* item = _player->GetItemByGuid(lguid))
            item->SetLootState(ITEM_LOOT_CHANGED);
    }
}

void WorldSession::HandleLootMoneyOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: CMSG_LOOT_MONEY");

    Loot* pLoot = sLootMgr.GetLoot(_player);

    if (!pLoot)
    {
        sLog.outError("HandleLootMoneyOpcode> Cannot retrieve loot for player %s", _player->GetGuidStr().c_str());
        return;
    }

    pLoot->SendGold(_player);
}

void WorldSession::HandleLootOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_LOOT");

    ObjectGuid lguid;
    recv_data >> lguid;

    // Check possible cheat
    if (!_player->isAlive())
        return;

    if (Loot* loot = sLootMgr.GetLoot(_player, lguid))
    {
        // remove stealth aura
        _player->DoInteraction(lguid);

        loot->ShowContentTo(_player);
    }

    // interrupt cast
    if (GetPlayer()->IsNonMeleeSpellCasted(false))
        GetPlayer()->InterruptNonMeleeSpells(false);
}

void WorldSession::HandleLootReleaseOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_LOOT_RELEASE");

    ObjectGuid lguid;
    recv_data >> lguid;

    if (Loot* loot = sLootMgr.GetLoot(_player, lguid))
        loot->Release(_player);
}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket& recv_data)
{
    uint8      itemSlot;        // slot sent in LOOT_RESPONSE
    ObjectGuid lootguid;        // the guid of the loot object owner
    ObjectGuid targetGuid;      // the item receiver guid

    recv_data >> lootguid >> itemSlot >> targetGuid;

    Player* target = ObjectAccessor::FindPlayer(targetGuid);
    if (!target)
    {
        _player->SendLootError(lootguid, LOOT_ERROR_PLAYER_NOT_FOUND);
        sLog.outError("WorldSession::HandleLootMasterGiveOpcode> Cannot retrieve target %s", targetGuid.GetString().c_str());
        return;
    }

    DEBUG_LOG("WorldSession::HandleLootMasterGiveOpcode> Giver = %s, Target = %s.", _player->GetGuidStr().c_str(), targetGuid.GetString().c_str());

    Loot* pLoot = sLootMgr.GetLoot(_player, lootguid);

    if (!pLoot)
    {
        sLog.outError("WorldSession::HandleLootMasterGiveOpcode> Cannot retrieve loot for player %s", _player->GetGuidStr().c_str());
        return;
    }

    if (_player->GetObjectGuid() != pLoot->GetMasterLootGuid())
    {
        _player->SendLootError(lootguid, LOOT_ERROR_DIDNT_KILL);
        sLog.outError("WorldSession::HandleLootMasterGiveOpcode> player %s is not the loot master!", _player->GetGuidStr().c_str());
        return;
    }

    if (!_player->IsInGroup(target) || !_player->IsInMap(target))
    {
        _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);
        sLog.outError("WorldSession::HandleLootMasterGiveOpcode> Player %s tried to give an item to ineligible player %s !", _player->GetGuidStr().c_str(), target->GetGuidStr().c_str());
        return;
    }

    LootItem* lootItem = pLoot->GetLootItemInSlot(itemSlot);

    if (!lootItem)
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return;
    }

    // item may be already looted or another cheating possibility
    if (lootItem->GetSlotTypeForSharedLoot(_player, pLoot) == MAX_LOOT_SLOT_TYPE)
    {
        sLog.outError("HandleAutostoreLootItemOpcode> %s have no right to loot itemId(%u)", _player->GetGuidStr().c_str(), lootItem->itemId);
        return;
    }

    if (!lootItem->IsAllowed(target, pLoot))
    {
        _player->SendEquipError(EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM, nullptr, nullptr);
        return;
    }

    InventoryResult result = pLoot->SendItem(target, lootItem);
    if (result != EQUIP_ERR_OK)
    {
        if (result == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_UNIQUE_ITEM);
        else if (result == EQUIP_ERR_INVENTORY_FULL)
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_INV_FULL);
        else
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);
    }
}

void WorldSession::HandleLootMethodOpcode(WorldPacket& recv_data)
{
    uint32 lootMethod;
    ObjectGuid lootMaster;
    uint32 lootThreshold;
    recv_data >> lootMethod >> lootMaster >> lootThreshold;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // everything is fine, do it
    group->SetLootMethod((LootMethod)lootMethod);
    group->SetMasterLooterGuid(lootMaster);
    group->SetLootThreshold((ItemQualities)lootThreshold);
    group->SendUpdate();
}

void WorldSession::HandleLootRoll(WorldPacket& recv_data)
{
    ObjectGuid lootedTarget;
    uint32 itemSlot;
    uint8  rollType;
    recv_data >> lootedTarget;                              // guid of the item rolled
    recv_data >> itemSlot;
    recv_data >> rollType;

    sLog.outDebug("WORLD RECIEVE CMSG_LOOT_ROLL, From:%s, rollType:%u", lootedTarget.GetString().c_str(), uint32(rollType));

    Group* group = _player->GetGroup();
    if (!group)
        return;

    if (rollType >= ROLL_NOT_EMITED_YET)
        return;

    sLootMgr.PlayerVote(GetPlayer(), lootedTarget, itemSlot, RollVote(rollType));
}

