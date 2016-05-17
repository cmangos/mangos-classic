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
#include "DBCStores.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "ScriptMgr.h"
#include "Totem.h"
#include "SpellAuras.h"
#include "LootMgr.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    uint8 bagIndex, slot;
    uint8 spell_count;                                      // number of spells at item, not used

    recvPacket >> bagIndex >> slot >> spell_count;

    // TODO: add targets.read() check
    Player* pUser = _player;

    // ignore for remote control state
    if (!pUser->IsSelfMover())
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        return;
    }

    Item* pItem = pUser->GetItemByPos(bagIndex, slot);
    if (!pItem)
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return;
    }

    DETAIL_LOG("WORLD: CMSG_USE_ITEM packet, bagIndex: %u, slot: %u, spell_count: %u , Item: %u, data length = %u", bagIndex, slot, spell_count, pItem->GetEntry(), (uint32)recvPacket.size());

    ItemPrototype const* proto = pItem->GetProto();
    if (!proto)
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return;
    }

    // some item classes can be used only in equipped state
    if (proto->InventoryType != INVTYPE_NON_EQUIP && !pItem->IsEquipped())
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return;
    }

    InventoryResult msg = pUser->CanUseItem(pItem);
    if (msg != EQUIP_ERR_OK)
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        pUser->SendEquipError(msg, pItem, nullptr);
        return;
    }

    // not allow use item from trade (cheat way only)
    if (pItem->IsInTrade())
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at not read packet tail
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return;
    }

    if (pUser->isInCombat())
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (SpellEntry const* spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId))
            {
                if (IsNonCombatSpell(spellInfo))
                {
                    recvPacket.rpos(recvPacket.wpos());     // prevent spam at not read packet tail
                    pUser->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT, pItem, nullptr);
                    return;
                }
            }
        }
    }

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if (pItem->GetProto()->Bonding == BIND_WHEN_USE || pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetProto()->Bonding == BIND_QUEST_ITEM)
    {
        if (!pItem->IsSoulBound())
        {
            pItem->SetState(ITEM_CHANGED, pUser);
            pItem->SetBinding(true);
        }
    }

    SpellCastTargets targets;

    recvPacket >> targets.ReadForCaster(pUser);

    targets.Update(pUser);

    if (!pItem->IsTargetValidForItemUse(targets.getUnitTarget()))
    {
        // free gray item after use fail
        pUser->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

        // search spell for spell error
        uint32 spellid = 0;
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE || proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
            {
                spellid = proto->Spells[i].SpellId;
                break;
            }
        }

        // send spell error
        if (SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellid))
            Spell::SendCastResult(_player, spellInfo, SPELL_FAILED_BAD_TARGETS);
        return;
    }

    // Note: If script stop casting it must send appropriate data to client to prevent stuck item in gray state.
    if (!sScriptMgr.OnItemUse(pUser, pItem, targets))
    {
        // no script or script not process request by self
        pUser->CastItemUseSpell(pItem, targets);
    }
}

#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleOpenItemOpcode(WorldPacket& recvPacket)
{
    DETAIL_LOG("WORLD: CMSG_OPEN_ITEM packet, data length = " SIZEFMTD, recvPacket.size());

    uint8 bagIndex, slot;

    recvPacket >> bagIndex >> slot;

    DETAIL_LOG("bagIndex: %u, slot: %u", bagIndex, slot);

    Player* pUser = _player;

    // ignore for remote control state
    if (!pUser->IsSelfMover())
        return;

    Item* pItem = pUser->GetItemByPos(bagIndex, slot);
    if (!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return;
    }

    ItemPrototype const* proto = pItem->GetProto();
    if (!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return;
    }

    // locked item
    uint32 lockId = proto->LockID;
    if (lockId && !pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_UNLOCKED))
    {
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);

        if (!lockInfo)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, nullptr);
            sLog.outError("WORLD::OpenItem: item [guid = %u] has an unknown lockId: %u!", pItem->GetGUIDLow() , lockId);
            return;
        }

        // required picklocking
        if (lockInfo->Skill[1] || lockInfo->Skill[0])
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, nullptr);
            return;
        }
    }

    if (pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_WRAPPED))// wrapped?
    {
        QueryResult* result = CharacterDatabase.PQuery("SELECT entry, flags FROM character_gifts WHERE item_guid = '%u'", pItem->GetGUIDLow());
        if (result)
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 flags = fields[1].GetUInt32();

            pItem->SetGuidValue(ITEM_FIELD_GIFTCREATOR, ObjectGuid());
            pItem->SetEntry(entry);
            pItem->SetUInt32Value(ITEM_FIELD_FLAGS, flags);
            pItem->SetState(ITEM_CHANGED, pUser);
            delete result;
        }
        else
        {
            sLog.outError("Wrapped item %u don't have record in character_gifts table and will deleted", pItem->GetGUIDLow());
            pUser->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
            return;
        }

        static SqlStatementID delGifts ;

        SqlStatement stmt = CharacterDatabase.CreateStatement(delGifts, "DELETE FROM character_gifts WHERE item_guid = ?");
        stmt.PExecute(pItem->GetGUIDLow());
    }
    else
    {
        Loot*& loot = pItem->loot;
        if (!loot)
            loot = new Loot(pUser, pItem, LOOT_PICKPOCKETING);

        loot->ShowContentTo(pUser);
    }
}

void WorldSession::HandleGameObjectUseOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;

    recv_data >> guid;

    DEBUG_LOG("WORLD: Received opcode CMSG_GAMEOBJ_USE guid: %s", guid.GetString().c_str());

    // ignore for remote control state
    if (!_player->IsSelfMover())
        return;

    GameObject* obj = _player->GetMap()->GetGameObject(guid);
    if (!obj)
        return;

    if (!obj->IsWithinDistInMap(_player, obj->GetInteractionDistance()))
        return;

    // Additional check preventing exploits (ie loot despawned chests)
    if (!obj->isSpawned())
    {
        sLog.outError("HandleGameObjectUseOpcode: CMSG_GAMEOBJ_USE for despawned GameObject (Entry %u), didn't expect this to happen.", obj->GetEntry());
        return;
    }

    // Never expect this opcode for some type GO's
    if (obj->GetGoType() == GAMEOBJECT_TYPE_GENERIC)
    {
        sLog.outError("HandleGameObjectUseOpcode: CMSG_GAMEOBJ_USE for not allowed GameObject type %u (Entry %u), didn't expect this to happen.", obj->GetGoType(), obj->GetEntry());
        return;
    }

    // Never expect this opcode for non intractable GO's
    if (obj->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT))
    {
        sLog.outError("HandleGameObjectUseOpcode: CMSG_GAMEOBJ_USE for GameObject (Entry %u) with non intractable flag (Flags %u), didn't expect this to happen.", obj->GetEntry(), obj->GetUInt32Value(GAMEOBJECT_FLAGS));
        return;
    }

    obj->Use(_player);
}

void WorldSession::HandleMeetingStoneJoinOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;

    recv_data >> guid;

    DEBUG_LOG("WORLD: Recvd CMSG_MEETINGSTONE_JOIN Message guid: %s", guid.GetString().c_str());

    // ignore for remote control state
    if (!_player->IsSelfMover())
        return;

    GameObject* obj = GetPlayer()->GetMap()->GetGameObject(guid);
    if (!obj)
        return;

    // Never expect this opcode for some type GO's
    if (obj->GetGoType() != GAMEOBJECT_TYPE_MEETINGSTONE)
    {
        sLog.outError("HandleMeetingStoneJoinOpcode: CMSG_MEETINGSTONE_JOIN for not allowed GameObject type %u (Entry %u), didn't expect this to happen.", obj->GetGoType(), obj->GetEntry());
        return;
    }

    obj->Use(_player);
}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;

    // ignore for remote control state (for player case)
    Unit* mover = _player->GetMover();
    if (mover != _player && mover->GetTypeId() == TYPEID_PLAYER)
    {
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at ignore packet
        return;
    }

    DEBUG_LOG("WORLD: got cast spell packet, spellId - %u, data length = " SIZEFMTD,
              spellId, recvPacket.size());

    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellId);

    if (!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %u", spellId);
        recvPacket.rpos(recvPacket.wpos());                 // prevent spam at ignore packet
        return;
    }

    if (mover->GetTypeId() == TYPEID_PLAYER)
    {
        // not have spell in spellbook or spell passive and not casted by client
        if (!((Player*)mover)->HasActiveSpell(spellId) || IsPassiveSpell(spellInfo))
        {
            sLog.outError("World: Player %u casts spell %u which he shouldn't have", mover->GetGUIDLow(), spellId);
            // cheater? kick? ban?
            recvPacket.rpos(recvPacket.wpos());             // prevent spam at ignore packet
            return;
        }
    }
    else
    {
        // not have spell in spellbook or spell passive and not casted by client
        if (!((Creature*)mover)->HasSpell(spellId) || IsPassiveSpell(spellInfo))
        {
            // cheater? kick? ban?
            recvPacket.rpos(recvPacket.wpos());             // prevent spam at ignore packet
            return;
        }
    }

    // client provided targets
    SpellCastTargets targets;

    recvPacket >> targets.ReadForCaster(_player);

    // auto-selection buff level base at target level (in spellInfo)
    if (Unit* target = targets.getUnitTarget())
    {
        // if rank not found then function return nullptr but in explicit cast case original spell can be casted and later failed with appropriate error message
        if (SpellEntry const* actualSpellInfo = sSpellMgr.SelectAuraRankForLevel(spellInfo, target->getLevel()))
            spellInfo = actualSpellInfo;
    }

    Spell* spell = new Spell(_player, spellInfo, false);
    spell->SpellStart(&targets);
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;

    recvPacket >> spellId;

    // ignore for remote control state (for player case)
    Unit* mover = _player->GetMover();
    if (mover != _player && mover->GetTypeId() == TYPEID_PLAYER)
        return;

    if (_player->IsNonMeleeSpellCasted(false))
        _player->InterruptNonMeleeSpells(false, spellId);
}

void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;

    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return;

    if (spellInfo->HasAttribute(SPELL_ATTR_CANT_CANCEL))
        return;

    if (IsPassiveSpell(spellInfo))
        return;

    if (!IsPositiveSpell(spellId))
    {
        // ignore for remote control state
        if (!_player->IsSelfMover())
        {
            // except own aura spells
            bool allow = false;
            for (int k = 0; k < MAX_EFFECT_INDEX; ++k)
            {
                if (spellInfo->EffectApplyAuraName[k] == SPELL_AURA_MOD_POSSESS ||
                        spellInfo->EffectApplyAuraName[k] == SPELL_AURA_MOD_POSSESS_PET)
                {
                    allow = true;
                    break;
                }
            }

            // this also include case when aura not found
            if (!allow)
                return;
        }
        else
            return;
    }

    // channeled spell case (it currently casted then)
    if (IsChanneledSpell(spellInfo))
    {
        if (Spell* curSpell = _player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            if (curSpell->m_spellInfo->Id == spellId)
                _player->InterruptSpell(CURRENT_CHANNELED_SPELL);
        return;
    }

    SpellAuraHolder* holder = _player->GetSpellAuraHolder(spellId);

    // not own area auras can't be cancelled (note: maybe need to check for aura on holder and not general on spell)
    if (holder && holder->GetCasterGuid() != _player->GetObjectGuid() && HasAreaAuraEffect(holder->GetSpellProto()))
        return;

    // non channeled case
    _player->RemoveAurasDueToSpellByCancel(spellId);
}

void WorldSession::HandlePetCancelAuraOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guid;
    uint32 spellId;

    recvPacket >> guid;
    recvPacket >> spellId;

    // ignore for remote control state
    if (!_player->IsSelfMover())
        return;

    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
    {
        sLog.outError("WORLD: unknown PET spell id %u", spellId);
        return;
    }

    Creature* pet = GetPlayer()->GetMap()->GetAnyTypeCreature(guid);

    if (!pet)
    {
        sLog.outError("HandlePetCancelAuraOpcode - %s not exist.", guid.GetString().c_str());
        return;
    }

    if (guid != GetPlayer()->GetPetGuid() && guid != GetPlayer()->GetCharmGuid())
    {
        sLog.outError("HandlePetCancelAura. %s isn't pet of %s", guid.GetString().c_str(), GetPlayer()->GetGuidStr().c_str());
        return;
    }

    if (!pet->isAlive())
    {
        pet->SendPetActionFeedback(FEEDBACK_PET_DEAD);
        return;
    }

    pet->RemoveAurasDueToSpell(spellId);

    pet->AddCreatureSpellCooldown(spellId);
}

void WorldSession::HandleCancelGrowthAuraOpcode(WorldPacket& /*recvPacket*/)
{
    // nothing do
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/)
{
    // cancel and prepare for deleting
    // do not send SMSG_CANCEL_AUTO_REPEAT! client will send this Opcode again (loop)
    _player->GetMover()->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::HandleCancelChanneling(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32>();                          // spellid, not used

    // ignore for remote control state (for player case)
    Unit* mover = _player->GetMover();
    if (mover != _player && mover->GetTypeId() == TYPEID_PLAYER)
        return;

    _player->InterruptSpell(CURRENT_CHANNELED_SPELL);
}

void WorldSession::HandleTotemDestroyed(WorldPacket& recvPacket)
{
    uint8 slotId;

    recvPacket >> slotId;

    // ignore for remote control state
    if (!_player->IsSelfMover())
        return;

    if (int(slotId) >= MAX_TOTEM_SLOT)
        return;

    if (Totem* totem = GetPlayer()->GetTotem(TotemSlot(slotId)))
        totem->UnSummon();
}

void WorldSession::HandleSelfResOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "WORLD: CMSG_SELF_RES");                  // empty opcode

    if (_player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL));
        if (spellInfo)
            _player->CastSpell(_player, spellInfo, false);

        _player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
    }
}
