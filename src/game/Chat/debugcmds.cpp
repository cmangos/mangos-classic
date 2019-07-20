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
#include "Server/DBCStores.h"
#include "WorldPacket.h"
#include "Entities/Player.h"
#include "Server/Opcodes.h"
#include "Chat/Chat.h"
#include "Log.h"
#include "Entities/Unit.h"
#include "Entities/GossipDef.h"
#include "Tools/Language.h"
#include "BattleGround/BattleGroundMgr.h"
#include <fstream>
#include "Maps/MapManager.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Spells/SpellMgr.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Cinematics/M2Stores.h"

bool ChatHandler::HandleDebugSendSpellFailCommand(char* args)
{
    if (!*args)
        return false;

    uint32 failnum;
    if (!ExtractUInt32(&args, failnum) || failnum > 255)
        return false;

    uint32 failarg1;
    if (!ExtractOptUInt32(&args, failarg1, 0))
        return false;

    uint32 failarg2;
    if (!ExtractOptUInt32(&args, failarg2, 0))
        return false;

    WorldPacket data(SMSG_CAST_RESULT, 4 + 1 + 1);
    data << uint32(133);
    data << uint8(2);
    data << uint8(failnum);
    if (failarg1 || failarg2)
        data << uint32(failarg1);
    if (failarg2)
        data << uint32(failarg2);

    m_session->SendPacket(data);

    return true;
}

bool ChatHandler::HandleDebugSendPoiCommand(char* args)
{
    Player* pPlayer = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    uint32 icon;
    if (!ExtractUInt32(&args, icon))
        return false;

    uint32 flags;
    if (!ExtractUInt32(&args, flags))
        return false;

    DETAIL_LOG("Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUIDLow(), icon, flags);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleDebugSendEquipErrorCommand(char* args)
{
    if (!*args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendEquipError(InventoryResult(msg), nullptr, nullptr);
    return true;
}

bool ChatHandler::HandleDebugSendSellErrorCommand(char* args)
{
    if (!*args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendSellError(SellResult(msg), nullptr, ObjectGuid(), 0);
    return true;
}

bool ChatHandler::HandleDebugSendBuyErrorCommand(char* args)
{
    if (!*args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendBuyError(BuyResult(msg), nullptr, 0, 0);
    return true;
}

bool ChatHandler::HandleDebugSendOpcodeCommand(char* /*args*/)
{
    Unit* unit = getSelectedUnit();
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
        unit = m_session->GetPlayer();

    std::ifstream stream("opcode.txt");
    if (!stream.is_open())
        return false;

    uint32 opcode = 0;
    if (!(stream >> opcode))
    {
        stream.close();
        return false;
    }

    WorldPacket data(opcode, 0);

    std::string type;
    while (stream >> type)
    {
        if (type.empty())
            break;

        if (type == "uint8")
        {
            uint16 value;
            stream >> value;
            data << uint8(value);
        }
        else if (type == "uint16")
        {
            uint16 value;
            stream >> value;
            data << value;
        }
        else if (type == "uint32")
        {
            uint32 value;
            stream >> value;
            data << value;
        }
        else if (type == "uint64")
        {
            uint64 value;
            stream >> value;
            data << value;
        }
        else if (type == "float")
        {
            float value;
            stream >> value;
            data << value;
        }
        else if (type == "string")
        {
            std::string value;
            stream >> value;
            data << value;
        }
        else if (type == "pguid")
        {
            data << unit->GetPackGUID();
        }
        else
        {
            DEBUG_LOG("Sending opcode: unknown type '%s'", type.c_str());
            break;
        }
    }
    stream.close();

    DEBUG_LOG("Sending opcode %u, %s", data.GetOpcode(), data.GetOpcodeName());

    data.hexlike();
    ((Player*)unit)->GetSession()->SendPacket(data);

    PSendSysMessage(LANG_COMMAND_OPCODESENT, data.GetOpcode(), unit->GetName());

    return true;
}

bool ChatHandler::HandleDebugUpdateWorldStateCommand(char* args)
{
    uint32 world;
    if (!ExtractUInt32(&args, world))
        return false;

    uint32 state;
    if (!ExtractUInt32(&args, state))
        return false;

    m_session->GetPlayer()->SendUpdateWorldState(world, state);
    return true;
}

bool ChatHandler::HandleDebugPlayCinematicCommand(char* args)
{
    // USAGE: .debug play cinematic #cinematicid
    // #cinematicid - ID decimal number from CinemaicSequences.dbc (1st column)
    uint32 dwId;
    if (!ExtractUInt32(&args, dwId))
        return false;

    if (!sCinematicSequencesStore.LookupEntry(dwId))
    {
        PSendSysMessage(LANG_CINEMATIC_NOT_EXIST, dwId);
        SetSentErrorMessage(true);
        return false;
    }

    // Dump camera locations
    if (CinematicSequencesEntry const* cineSeq = sCinematicSequencesStore.LookupEntry(dwId))
    {
        std::unordered_map<uint32, FlyByCameraCollection>::const_iterator itr = sFlyByCameraStore.find(cineSeq->cinematicCamera);
        if (itr != sFlyByCameraStore.end())
        {
            PSendSysMessage("Waypoints for sequence %u, camera %u", dwId, cineSeq->cinematicCamera);
            uint32 count = 1;
            for (FlyByCamera cam : itr->second)
            {
                PSendSysMessage("%02u - %7ums [%f, %f, %f] Facing %f (%f degrees)", count, cam.timeStamp, cam.locations.x, cam.locations.y, cam.locations.z, cam.locations.w, cam.locations.w * (180 / M_PI));
                count++;
            }
            PSendSysMessage("%u waypoints dumped",uint32(itr->second.size()));
        }
    }

    m_session->GetPlayer()->SendCinematicStart(dwId);
    return true;
}

// Play sound
bool ChatHandler::HandleDebugPlaySoundCommand(char* args)
{
    // USAGE: .debug playsound #soundid
    // #soundid - ID decimal number from SoundEntries.dbc (1st column)
    uint32 dwSoundId;
    if (!ExtractUInt32(&args, dwSoundId))
        return false;

    if (!sSoundEntriesStore.LookupEntry(dwSoundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, dwSoundId);
        SetSentErrorMessage(true);
        return false;
    }

    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (m_session->GetPlayer()->GetSelectionGuid())
        unit->PlayDistanceSound(dwSoundId, PlayPacketParameters(PLAY_TARGET, m_session->GetPlayer()));
    else
        unit->PlayDirectSound(dwSoundId, PlayPacketParameters(PLAY_TARGET, m_session->GetPlayer()));

    PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
    return true;
}

// Play Music
bool ChatHandler::HandleDebugPlayMusicCommand(char* args)
{
    // USAGE: .debug playmusic #musicid
    // #musicid - ID decimal number from SoundEntries.dbc (1st column)
    uint32 dwMusicId;
    if (!ExtractUInt32(&args, dwMusicId))
        return false;

    if (!sSoundEntriesStore.LookupEntry(dwMusicId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, dwMusicId);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->PlayMusic(dwMusicId, PlayPacketParameters(PLAY_TARGET, dynamic_cast<Player*>(getSelectedUnit())));

    PSendSysMessage(LANG_YOU_HEAR_SOUND, dwMusicId);
    return true;
}

// Play pet dismiss sound
bool ChatHandler::HandleDebugPetDismissSound(char* args)
{
    uint32 petDismissSound;
    if (!ExtractUInt32(&args, petDismissSound))
        return false;

    m_session->GetPlayer()->SendPetDismiss(petDismissSound);
    PSendSysMessage(LANG_YOU_HEAR_SOUND, petDismissSound);
    return true;
}

// Send notification in channel
bool ChatHandler::HandleDebugSendChannelNotifyCommand(char* args)
{
    const char* name = "test";

    uint32 code;
    if (!ExtractUInt32(&args, code) || code > 255)
        return false;

    WorldPacket data(SMSG_CHANNEL_NOTIFY, (1 + 10));
    data << uint8(code);                                    // notify type
    data << name;                                           // channel name
    data << uint32(0);
    data << uint32(0);
    m_session->SendPacket(data);
    return true;
}

// Send notification in chat
bool ChatHandler::HandleDebugSendChatMsgCommand(char* args)
{
    const char* msg = "testtest";

    uint32 type;
    if (!ExtractUInt32(&args, type) || type > 255)
        return false;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, ChatMsg(type), msg, LANG_UNIVERSAL, CHAT_TAG_NONE, m_session->GetPlayer()->GetObjectGuid(), m_session->GetPlayerName());
    m_session->SendPacket(data);
    return true;
}

bool ChatHandler::HandleDebugSendQuestFailedMsgCommand(char* args)
{
    uint32 questId;
    if (!ExtractUInt32(&args, questId))
        return false;

    uint32 reason;
    if (!ExtractUInt32(&args, reason))
        return false;

    m_session->GetPlayer()->SendQuestFailed(questId, reason);
    return true;
}

bool ChatHandler::HandleDebugSendQuestPartyMsgCommand(char* args)
{
    uint32 msg;
    if (!ExtractUInt32(&args, msg))
        return false;

    m_session->GetPlayer()->SendPushToPartyResponse(m_session->GetPlayer(), msg);
    return true;
}

bool ChatHandler::HandleDebugGetLootRecipientCommand(char* /*args*/)
{
    Creature* target = getSelectedCreature();
    if (!target)
        return false;

    if (!target->HasLootRecipient())
        SendSysMessage("loot recipient: no loot recipient");
    else if (Player* recipient = target->GetLootRecipient())
        PSendSysMessage("loot recipient: %s with raw data %s from group %u",
                        recipient->GetGuidStr().c_str(),
                        target->GetLootRecipientGuid().GetString().c_str(),
                        target->GetLootGroupRecipientId());
    else
        SendSysMessage("loot recipient: offline ");

    return true;
}

bool ChatHandler::HandleDebugSendQuestInvalidMsgCommand(char* args)
{
    uint32 msg = std::stoul(args);
    m_session->GetPlayer()->SendCanTakeQuestResponse(msg);
    return true;
}

bool ChatHandler::HandleDebugGetItemStateCommand(char* args)
{
    if (!*args)
        return false;

    ItemUpdateState state = ITEM_UNCHANGED;
    bool list_queue = false, check_all = false;

    std::string state_str;

    if (strncmp(args, "unchanged", strlen(args)) == 0)
    {
        state = ITEM_UNCHANGED;
        state_str = "unchanged";
    }
    else if (strncmp(args, "changed", strlen(args)) == 0)
    {
        state = ITEM_CHANGED;
        state_str = "changed";
    }
    else if (strncmp(args, "new", strlen(args)) == 0)
    {
        state = ITEM_NEW;
        state_str = "new";
    }
    else if (strncmp(args, "removed", strlen(args)) == 0)
    {
        state = ITEM_REMOVED;
        state_str = "removed";
    }
    else if (strncmp(args, "queue", strlen(args)) == 0)
        list_queue = true;
    else if (strncmp(args, "all", strlen(args)) == 0)
        check_all = true;
    else
        return false;

    Player* player = getSelectedPlayer();
    if (!player) player = m_session->GetPlayer();

    if (!list_queue && !check_all)
    {
        state_str = "The player has the following " + state_str + " items: ";
        SendSysMessage(state_str.c_str());
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
        {
            if (i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) continue;
            if (!item->IsBag())
            {
                if (item->GetState() == state)
                    PSendSysMessage("%s bag: 255 slot: %u owner: %s",
                                    item->GetGuidStr().c_str(),  uint32(item->GetSlot()), item->GetOwnerGuid().GetString().c_str());
            }
            else
            {
                Bag* bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    Item* item2 = bag->GetItemByPos(j);
                    if (item2 && item2->GetState() == state)
                        PSendSysMessage("%s bag: %u slot: %u owner: %s",
                                        item2->GetGuidStr().c_str(), uint32(item2->GetBagSlot()), uint32(item2->GetSlot()),
                                        item2->GetOwnerGuid().GetString().c_str());
                }
            }
        }
    }

    if (list_queue)
    {
        std::vector<Item*>& updateQueue = player->GetItemUpdateQueue();
        for (auto item : updateQueue)
        {
            if (!item) continue;

            Bag* container = item->GetContainer();
            uint8 bag_slot = container ? container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);

            std::string st;
            switch (item->GetState())
            {
                case ITEM_UNCHANGED: st = "unchanged"; break;
                case ITEM_CHANGED: st = "changed"; break;
                case ITEM_NEW: st = "new"; break;
                case ITEM_REMOVED: st = "removed"; break;
            }

            PSendSysMessage("%s bag: %u slot: %u - state: %s",
                            item->GetGuidStr().c_str(), uint32(bag_slot), uint32(item->GetSlot()), st.c_str());
        }
        if (updateQueue.empty())
            PSendSysMessage("updatequeue empty");
    }

    if (check_all)
    {
        bool error = false;
        std::vector<Item*>& updateQueue = player->GetItemUpdateQueue();
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
        {
            if (i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) continue;

            if (item->GetSlot() != i)
            {
                PSendSysMessage("%s at slot %u has an incorrect slot value: %d",
                                item->GetGuidStr().c_str(), uint32(i), uint32(item->GetSlot()));
                error = true; continue;
            }

            if (item->GetOwnerGuid() != player->GetObjectGuid())
            {
                PSendSysMessage("%s at slot %u owner (%s) and inventory owner (%s) don't match!",
                                item->GetGuidStr().c_str(), uint32(item->GetSlot()),
                                item->GetOwnerGuid().GetString().c_str(), player->GetGuidStr().c_str());
                error = true; continue;
            }

            if (Bag* container = item->GetContainer())
            {
                PSendSysMessage("%s at slot %u has a container %s from slot %u but shouldnt!",
                                item->GetGuidStr().c_str(), uint32(item->GetSlot()),
                                container->GetGuidStr().c_str(), uint32(container->GetSlot()));
                error = true; continue;
            }

            if (item->IsInUpdateQueue())
            {
                uint16 qp = item->GetQueuePos();
                if (qp > updateQueue.size())
                {
                    PSendSysMessage("%s at slot %u has a queuepos (%d) larger than the update queue size! ",
                                    item->GetGuidStr().c_str(), uint32(item->GetSlot()), uint32(qp));
                    error = true; continue;
                }

                if (updateQueue[qp] == nullptr)
                {
                    PSendSysMessage("%s at slot %u has a queuepos (%d) that points to nullptr in the queue!",
                                    item->GetGuidStr().c_str(), uint32(item->GetSlot()), uint32(qp));
                    error = true; continue;
                }

                if (updateQueue[qp] != item)
                {
                    PSendSysMessage("%s at slot %u has a queuepos (%d) that points to %s in the queue (bag %u, slot %u)",
                                    item->GetGuidStr().c_str(), uint32(item->GetSlot()), uint32(qp),
                                    updateQueue[qp]->GetGuidStr().c_str(), uint32(updateQueue[qp]->GetBagSlot()), uint32(updateQueue[qp]->GetSlot()));
                    error = true; continue;
                }
            }
            else if (item->GetState() != ITEM_UNCHANGED)
            {
                PSendSysMessage("%s at slot %u is not in queue but should be (state: %d)!",
                                item->GetGuidStr().c_str(), uint32(item->GetSlot()), item->GetState());
                error = true; continue;
            }

            if (item->IsBag())
            {
                Bag* bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    Item* item2 = bag->GetItemByPos(j);
                    if (!item2) continue;

                    if (item2->GetSlot() != j)
                    {
                        PSendSysMessage("%s in bag %u at slot %u has an incorrect slot value: %u",
                                        item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(j), uint32(item2->GetSlot()));
                        error = true; continue;
                    }

                    if (item2->GetOwnerGuid() != player->GetObjectGuid())
                    {
                        PSendSysMessage("%s in bag %u at slot %u owner (%s) and inventory owner (%s) don't match!",
                                        item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()),
                                        item2->GetOwnerGuid().GetString().c_str(), player->GetGuidStr().c_str());
                        error = true; continue;
                    }

                    Bag* container = item2->GetContainer();
                    if (!container)
                    {
                        PSendSysMessage("%s in bag %u at slot %u has no container!",
                                        item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()));
                        error = true; continue;
                    }

                    if (container != bag)
                    {
                        PSendSysMessage("%s in bag %u at slot %u has a different container %s from slot %u!",
                                        item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()),
                                        container->GetGuidStr().c_str(), uint32(container->GetSlot()));
                        error = true; continue;
                    }

                    if (item2->IsInUpdateQueue())
                    {
                        uint16 qp = item2->GetQueuePos();
                        if (qp > updateQueue.size())
                        {
                            PSendSysMessage("%s in bag %u at slot %u has a queuepos (%d) larger than the update queue size! ",
                                            item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()), uint32(qp));
                            error = true; continue;
                        }

                        if (updateQueue[qp] == nullptr)
                        {
                            PSendSysMessage("%s in bag %u at slot %u has a queuepos (%d) that points to nullptr in the queue!",
                                            item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()), uint32(qp));
                            error = true; continue;
                        }

                        if (updateQueue[qp] != item2)
                        {
                            PSendSysMessage("%s in bag %u at slot %u has a queuepos (%d) that points to %s in the queue (bag %u slot %u)",
                                            item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()), uint32(qp),
                                            updateQueue[qp]->GetGuidStr().c_str(), uint32(updateQueue[qp]->GetBagSlot()), uint32(updateQueue[qp]->GetSlot()));
                            error = true;
                        }
                    }
                    else if (item2->GetState() != ITEM_UNCHANGED)
                    {
                        PSendSysMessage("%s in bag %u at slot %u is not in queue but should be (state: %d)!",
                                        item2->GetGuidStr().c_str(), uint32(bag->GetSlot()), uint32(item2->GetSlot()), item2->GetState());
                        error = true;
                    }
                }
            }
        }

        for (size_t i = 0; i < updateQueue.size(); ++i)
        {
            Item* item = updateQueue[i];
            if (!item) continue;

            if (item->GetOwnerGuid() != player->GetObjectGuid())
            {
                PSendSysMessage("queue(" SIZEFMTD "): %s has the owner (%s) and inventory owner (%s) don't match!",
                                i, item->GetGuidStr().c_str(),
                                item->GetOwnerGuid().GetString().c_str(), player->GetGuidStr().c_str());
                error = true; continue;
            }

            if (item->GetQueuePos() != i)
            {
                PSendSysMessage("queue(" SIZEFMTD "): %s has queuepos doesn't match it's position in the queue!",
                                i, item->GetGuidStr().c_str());
                error = true; continue;
            }

            if (item->GetState() == ITEM_REMOVED) continue;
            Item* test = player->GetItemByPos(item->GetBagSlot(), item->GetSlot());

            if (test == nullptr)
            {
                PSendSysMessage("queue(" SIZEFMTD "): %s has incorrect (bag %u slot %u) values, the player doesn't have an item at that position!",
                                i, item->GetGuidStr().c_str(), uint32(item->GetBagSlot()), uint32(item->GetSlot()));
                error = true; continue;
            }

            if (test != item)
            {
                PSendSysMessage("queue(" SIZEFMTD "): %s has incorrect (bag %u slot %u) values, the %s is there instead!",
                                i, item->GetGuidStr().c_str(), uint32(item->GetBagSlot()), uint32(item->GetSlot()),
                                test->GetGuidStr().c_str());
                error = true;
            }
        }
        if (!error)
            SendSysMessage("All OK!");
    }

    return true;
}

bool ChatHandler::HandleDebugBattlegroundCommand(char* /*args*/)
{
    sBattleGroundMgr.ToggleTesting();
    return true;
}

bool ChatHandler::HandleDebugBattlegroundStartCommand(char* /*args*/)
{
    if (auto bg = m_session->GetPlayer()->GetBattleGround())
    {
        bg->SetStartDelayTime(-1);
        return true;
    }

    return false;
}

bool ChatHandler::HandleDebugSpellCheckCommand(char* /*args*/)
{
    sLog.outString("Check expected in code spell properties base at table 'spell_check' content...");
    sSpellMgr.CheckUsedSpells("spell_check");
    return true;
}

// show animation
bool ChatHandler::HandleDebugAnimCommand(char* args)
{
    uint32 emote_id;
    if (!ExtractUInt32(&args, emote_id))
        return false;

    m_session->GetPlayer()->HandleEmoteCommand(emote_id);
    return true;
}

bool ChatHandler::HandleDebugSetAuraStateCommand(char* args)
{
    int32 state;
    if (!ExtractInt32(&args, state))
        return false;

    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!state)
    {
        // reset all states
        for (int i = 1; i <= 32; ++i)
            unit->ModifyAuraState(AuraState(i), false);
        return true;
    }

    unit->ModifyAuraState(AuraState(abs(state)), state > 0);
    return true;
}

bool ChatHandler::HandleSetValueHelper(Object* target, uint32 field, char* typeStr, char* valStr)
{
    ObjectGuid guid = target->GetObjectGuid();

    // not allow access to nonexistent or critical for work field
    if (field >= target->GetValuesCount() || field <= OBJECT_FIELD_ENTRY)
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, field, guid.GetString().c_str(), uint32(target->GetValuesCount()));
        return false;
    }

    uint32 base;                                            // 0 -> float
    if (!typeStr)
        base = 10;
    else if (strncmp(typeStr, "int", strlen(typeStr)) == 0)
        base = 10;
    else if (strncmp(typeStr, "hex", strlen(typeStr)) == 0)
        base = 16;
    else if (strncmp(typeStr, "bit", strlen(typeStr)) == 0)
        base = 2;
    else if (strncmp(typeStr, "float", strlen(typeStr)) == 0)
        base = 0;
    else
        return false;

    if (base)
    {
        uint32 iValue;
        if (!ExtractUInt32Base(&valStr, iValue, base))
            return false;

        DEBUG_LOG(GetMangosString(LANG_SET_UINT), guid.GetString().c_str(), field, iValue);
        target->SetUInt32Value(field, iValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, guid.GetString().c_str(), field, iValue);
    }
    else
    {
        float fValue;
        if (!ExtractFloat(&valStr, fValue))
            return false;

        DEBUG_LOG(GetMangosString(LANG_SET_FLOAT), guid.GetString().c_str(), field, fValue);
        target->SetFloatValue(field, fValue);
        PSendSysMessage(LANG_SET_FLOAT_FIELD, guid.GetString().c_str(), field, fValue);
    }

    return true;
}

bool ChatHandler::HandleDebugSetItemValueCommand(char* args)
{
    uint32 guid;
    if (!ExtractUInt32(&args, guid))
        return false;

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractOptNotLastArg(&args);
    if (!typeStr)
        return false;

    char* valStr = ExtractLiteralArg(&args);
    if (!valStr)
        return false;

    Item* item = m_session->GetPlayer()->GetItemByGuid(ObjectGuid(HIGHGUID_ITEM, guid));
    if (!item)
        return false;

    return HandleSetValueHelper(item, field, typeStr, valStr);
}

bool ChatHandler::HandleDebugSetValueCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractOptNotLastArg(&args);
    if (!typeStr)
        return false;

    char* valStr = ExtractLiteralArg(&args);
    if (!valStr)
        return false;

    return HandleSetValueHelper(target, field, typeStr, valStr);
}

bool ChatHandler::HandleGetValueHelper(Object* target, uint32 field, char* typeStr)
{
    ObjectGuid guid = target->GetObjectGuid();

    if (field >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, field, guid.GetString().c_str(), target->GetValuesCount());
        return false;
    }

    uint32 base;                                            // 0 -> float
    if (!typeStr)
        base = 10;
    else if (strncmp(typeStr, "int", strlen(typeStr)) == 0)
        base = 10;
    else if (strncmp(typeStr, "hex", strlen(typeStr)) == 0)
        base = 16;
    else if (strncmp(typeStr, "bit", strlen(typeStr)) == 0)
        base = 2;
    else if (strncmp(typeStr, "float", strlen(typeStr)) == 0)
        base = 0;
    else
        return false;

    if (base)
    {
        uint32 iValue = target->GetUInt32Value(field);

        switch (base)
        {
            case 2:
            {
                // starting 0 if need as required bitstring format
                std::string res;
                res.reserve(1 + 32 + 1);
                res = iValue & (1 << (32 - 1)) ? "0" : " ";
                for (int i = 32; i > 0; --i)
                    res += iValue & (1 << (i - 1)) ? "1" : "0";
                DEBUG_LOG(GetMangosString(LANG_GET_BITSTR), guid.GetString().c_str(), field, res.c_str());
                PSendSysMessage(LANG_GET_BITSTR_FIELD, guid.GetString().c_str(), field, res.c_str());
                break;
            }
            case 16:
                DEBUG_LOG(GetMangosString(LANG_GET_HEX), guid.GetString().c_str(), field, iValue);
                PSendSysMessage(LANG_GET_HEX_FIELD, guid.GetString().c_str(), field, iValue);
                break;
            case 10:
            default:
                DEBUG_LOG(GetMangosString(LANG_GET_UINT), guid.GetString().c_str(), field, iValue);
                PSendSysMessage(LANG_GET_UINT_FIELD, guid.GetString().c_str(), field, iValue);
        }
    }
    else
    {
        float fValue = target->GetFloatValue(field);
        DEBUG_LOG(GetMangosString(LANG_GET_FLOAT), guid.GetString().c_str(), field, fValue);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, guid.GetString().c_str(), field, fValue);
    }

    return true;
}

bool ChatHandler::HandleDebugGetItemValueCommand(char* args)
{
    uint32 guid;
    if (!ExtractUInt32(&args, guid))
        return false;

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr && *args)                                  // optional arg but check format fail case
        return false;

    Item* item = m_session->GetPlayer()->GetItemByGuid(ObjectGuid(HIGHGUID_ITEM, guid));
    if (!item)
        return false;

    return HandleGetValueHelper(item, field, typeStr);
}

bool ChatHandler::HandleDebugGetValueCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr && *args)                                  // optional arg but check format fail case
        return false;

    return HandleGetValueHelper(target, field, typeStr);
}

bool ChatHandler::HandlerDebugModValueHelper(Object* target, uint32 field, char* typeStr, char* valStr)
{
    ObjectGuid guid = target->GetObjectGuid();

    // not allow access to nonexistent or critical for work field
    if (field >= target->GetValuesCount() || field <= OBJECT_FIELD_ENTRY)
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, field, guid.GetString().c_str(), uint32(target->GetValuesCount()));
        return false;
    }

    uint32 type;                                            // 0 -> float 1 -> int add 2-> bit or 3 -> bit and  4 -> bit and not
    if (strncmp(typeStr, "int", strlen(typeStr)) == 0)
        type = 1;
    else if (strncmp(typeStr, "float", strlen(typeStr)) == 0)
        type = 0;
    else if (strncmp(typeStr, "|=", strlen("|=") + 1) == 0) // exactly copy
        type = 2;
    else if (strncmp(typeStr, "&=", strlen("&=") + 1) == 0) // exactly copy
        type = 3;
    else if (strncmp(typeStr, "&=~", strlen("&=~") + 1) == 0) // exactly copy
        type = 4;
    else
        return false;

    if (type)
    {
        uint32 iValue;
        if (!ExtractUInt32Base(&valStr, iValue, type == 1 ? 10 : 16))
            return false;

        uint32 value = target->GetUInt32Value(field);
        const char* guidString = guid.GetString().c_str();

        switch (type)
        {
            default:
            case 1:                                         // int +
                value = uint32(int32(value) + int32(iValue));
                DEBUG_LOG(GetMangosString(LANG_CHANGE_INT32), guidString, field, iValue, value, value);
                PSendSysMessage(LANG_CHANGE_INT32_FIELD, guidString, field, iValue, value, value);
                break;
            case 2:                                         // |= bit or
                value |= iValue;
                DEBUG_LOG(GetMangosString(LANG_CHANGE_HEX), guidString, field, typeStr, iValue, value);
                PSendSysMessage(LANG_CHANGE_HEX_FIELD, guidString, field, typeStr, iValue, value);
                break;
            case 3:                                         // &= bit and
                value &= iValue;
                DEBUG_LOG(GetMangosString(LANG_CHANGE_HEX), guidString, field, typeStr, iValue, value);
                PSendSysMessage(LANG_CHANGE_HEX_FIELD, guidString, field, typeStr, iValue, value);
                break;
            case 4:                                         // &=~ bit and not
                value &= ~iValue;
                DEBUG_LOG(GetMangosString(LANG_CHANGE_HEX), guidString, field, typeStr, iValue, value);
                PSendSysMessage(LANG_CHANGE_HEX_FIELD, guidString, field, typeStr, iValue, value);
                break;
        }

        target->SetUInt32Value(field, value);
    }
    else
    {
        float fValue;
        if (!ExtractFloat(&valStr, fValue))
            return false;

        float value = target->GetFloatValue(field);

        value += fValue;

        DEBUG_LOG(GetMangosString(LANG_CHANGE_FLOAT), guid.GetString().c_str(), field, fValue, value);
        PSendSysMessage(LANG_CHANGE_FLOAT_FIELD, guid.GetString().c_str(), field, fValue, value);

        target->SetFloatValue(field, value);
    }

    return true;
}

bool ChatHandler::HandleDebugModItemValueCommand(char* args)
{
    uint32 guid;
    if (!ExtractUInt32(&args, guid))
        return false;

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr)
        return false;

    char* valStr = ExtractLiteralArg(&args);
    if (!valStr)
        return false;

    Item* item = m_session->GetPlayer()->GetItemByGuid(ObjectGuid(HIGHGUID_ITEM, guid));
    if (!item)
        return false;

    return HandlerDebugModValueHelper(item, field, typeStr, valStr);
}

bool ChatHandler::HandleDebugModValueCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 field;
    if (!ExtractUInt32(&args, field))
        return false;

    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr && *args)                                  // optional arg but check format fail case
        return false;

    char* valStr = ExtractLiteralArg(&args);
    if (!valStr)
        return false;

    return HandlerDebugModValueHelper(target, field, typeStr, valStr);
}

bool ChatHandler::HandleDebugSpellCoefsCommand(char* args)
{
    uint32 spellid = ExtractSpellIdFromLink(&args);
    if (!spellid)
        return false;

    SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
    if (!spellEntry)
        return false;

    SpellBonusEntry const* bonus = sSpellMgr.GetSpellBonusData(spellid);

    float direct_calc = CalculateDefaultCoefficient(spellEntry, SPELL_DIRECT_DAMAGE);
    float dot_calc = CalculateDefaultCoefficient(spellEntry, DOT);

    bool isDirectHeal = false;
    for (int i = 0; i < 3; ++i)
    {
        // Heals (Also count Mana Shield and Absorb effects as heals)
        if (spellEntry->Effect[i] == SPELL_EFFECT_HEAL || spellEntry->Effect[i] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                (spellEntry->Effect[i] == SPELL_EFFECT_APPLY_AURA && (spellEntry->EffectApplyAuraName[i] == SPELL_AURA_SCHOOL_ABSORB || spellEntry->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_HEAL)))
        {
            isDirectHeal = true;
            break;
        }
    }

    bool isDotHeal = false;
    for (int i = 0; i < 3; ++i)
    {
        // Periodic Heals
        if (spellEntry->Effect[i] == SPELL_EFFECT_APPLY_AURA && spellEntry->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_HEAL)
        {
            isDotHeal = true;
            break;
        }
    }

    char const* directHealStr = GetMangosString(LANG_DIRECT_HEAL);
    char const* directDamageStr = GetMangosString(LANG_DIRECT_DAMAGE);
    char const* dotHealStr = GetMangosString(LANG_DOT_HEAL);
    char const* dotDamageStr = GetMangosString(LANG_DOT_DAMAGE);

    PSendSysMessage(LANG_SPELLCOEFS, spellid, isDirectHeal ? directHealStr : directDamageStr,
                    direct_calc, direct_calc * 1.88f, bonus ? bonus->direct_damage : 0.0f, bonus ? bonus->ap_bonus : 0.0f);
    PSendSysMessage(LANG_SPELLCOEFS, spellid, isDotHeal ? dotHealStr : dotDamageStr,
                    dot_calc, dot_calc * 1.88f, bonus ? bonus->dot_damage : 0.0f, bonus ? bonus->ap_dot_bonus : 0.0f);

    return true;
}

bool ChatHandler::HandleDebugSpellModsCommand(char* args)
{
    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr)
        return false;

    uint16 opcode;
    if (strncmp(typeStr, "flat", strlen(typeStr)) == 0)
        opcode = SMSG_SET_FLAT_SPELL_MODIFIER;
    else if (strncmp(typeStr, "pct", strlen(typeStr)) == 0)
        opcode = SMSG_SET_PCT_SPELL_MODIFIER;
    else
        return false;

    uint32 effidx;
    if (!ExtractUInt32(&args, effidx) || effidx >= 64)
        return false;

    uint32 spellmodop;
    if (!ExtractUInt32(&args, spellmodop) || spellmodop >= MAX_SPELLMOD)
        return false;

    int32 value;
    if (!ExtractInt32(&args, value))
        return false;

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_SPELLMODS, opcode == SMSG_SET_FLAT_SPELL_MODIFIER ? "flat" : "pct",
                    spellmodop, value, effidx, GetNameLink(chr).c_str());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPELLMODS_CHANGED, GetNameLink().c_str(),
                                         opcode == SMSG_SET_FLAT_SPELL_MODIFIER ? "flat" : "pct", spellmodop, value, effidx);

    WorldPacket data(opcode, (1 + 1 + 2 + 2));
    data << uint8(effidx);
    data << uint8(spellmodop);
    data << int32(value);
    chr->GetSession()->SendPacket(data);

    return true;
}

bool ChatHandler::HandleDebugTaxiCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    player->ToggleTaxiDebug();
    PSendSysMessage(LANG_COMMAND_TAXI_DEBUG, (player->IsTaxiDebug() ? GetMangosString(LANG_ON) : GetMangosString(LANG_OFF)));
    return true;
}

bool ChatHandler::HandleDebugMaps(char* /*args*/)
{
    PSendSysMessage("Update time statistics:");
    PSendSysMessage("Map[0] >> Min: %ums, Max: %ums, Avg: %ums",
        sMapMgr.GetMapUpdateMinTime(0), sMapMgr.GetMapUpdateMaxTime(0), sMapMgr.GetMapUpdateAvgTime(0));
    PSendSysMessage("Map[1] >> Min: %ums, Max: %ums, Avg: %ums",
        sMapMgr.GetMapUpdateMinTime(1), sMapMgr.GetMapUpdateMaxTime(1), sMapMgr.GetMapUpdateAvgTime(1));
    PSendSysMessage("Map[530] >> Min: %ums, Max: %ums, Avg: %ums",
        sMapMgr.GetMapUpdateMinTime(530), sMapMgr.GetMapUpdateMaxTime(530), sMapMgr.GetMapUpdateAvgTime(530));

    if (m_session)
    {
        Player* player = m_session->GetPlayer();
        if (!player)
            return true;

        if (player->GetMap()->IsContinent())
            return true;

        uint32 mapId = player->GetMap()->GetId();
        uint32 instance = player->GetMap()->GetInstanceId();
        PSendSysMessage("Instance update time statistics:");
        PSendSysMessage("Map[%u] (Instance: %u) >> Min: %ums, Max: %ums, Avg: %ums",
            mapId, instance, sMapMgr.GetMapUpdateMinTime(mapId, instance), sMapMgr.GetMapUpdateMaxTime(mapId, instance), sMapMgr.GetMapUpdateAvgTime(mapId, instance));
    }

    return true;
}

bool ChatHandler::HandleShowTemporarySpawnList(char* /*args*/)
{
    Player* pPlayer = m_session->GetPlayer();

    std::map<uint32, uint32> temp_creature = pPlayer->GetMap()->GetTempCreatures();

    SendSysMessage("Current temporary creatures in player map.");

    for (std::map<uint32, uint32>::iterator it = temp_creature.begin(); it != temp_creature.end(); ++it)
        PSendSysMessage("Entry: %u, Count: %u ", (*it).first, (*it).second);

    std::map<uint32, uint32> temp_pet = pPlayer->GetMap()->GetTempPets();
    SendSysMessage("Current temporary pets in player map.");

    for (std::map<uint32, uint32>::iterator it = temp_pet.begin(); it != temp_pet.end(); ++it)
        PSendSysMessage("Entry: %u, Count: %u ", (*it).first, (*it).second);

    return true;
}

bool ChatHandler::HandleGridsLoadedCount(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    if (!player)
        return false;

    PSendSysMessage("There are currently %u loaded grids.", player->GetMap()->GetLoadedGridsCount());
    return true;
}

bool ChatHandler::HandleDebugWaypoint(char* args)
{
    Creature* target = getSelectedCreature();
    if (!target)
        return false;

    uint32 pathId;
    if (!ExtractUInt32(&args, pathId) || pathId >= 256)
    {
        PSendSysMessage("Current target path ID: %d", target->GetMotionMaster()->GetPathId());
        return true;
    }

    target->GetMotionMaster()->MoveWaypoint(pathId, 2);

    return true;
}

bool ChatHandler::HandleDebugByteFields(char* args)
{
    Creature* target = getSelectedCreature();
    if (!target)
        return false;

    int32 fieldNum;
    if (!ExtractInt32(&args, fieldNum))
        return false;

    uint32 byte;
    if (!ExtractUInt32(&args, byte))
        return false;

    uint32 value;
    if (!ExtractUInt32(&args, value))
        return false;

    switch (fieldNum)
    {
        case 0:
            target->SetByteFlag(UNIT_FIELD_BYTES_0, byte, value);
            break;
        case 1:
            target->SetByteFlag(UNIT_FIELD_BYTES_1, byte, value);
            break;
        case 2:
            target->SetByteFlag(UNIT_FIELD_BYTES_2, byte, value);
            break;
        case -10:
            target->RemoveByteFlag(UNIT_FIELD_BYTES_0, byte, value);
            break;
        case -1:
            target->RemoveByteFlag(UNIT_FIELD_BYTES_1, byte, value);
            break;
        case -2:
            target->RemoveByteFlag(UNIT_FIELD_BYTES_2, byte, value);
            break;
        default:
            break;
    }

    return true;
}

bool ChatHandler::HandleDebugSpellVisual(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
        return false;

    uint32 spellVisualID;
    if (!ExtractUInt32(&args, spellVisualID))
        return false;

    target->PlaySpellVisual(spellVisualID);
    return true;
}

bool ChatHandler::HandleDebugMoveflags(char* /*args*/)
{
    Unit* target = getSelectedUnit();
    if (!target)
        return false;

    PSendSysMessage("Moveflags on target %u", target->m_movementInfo.GetMovementFlags());
    return true;
}

bool ChatHandler::HandleDebugLootDropStats(char* args)
{
    uint32 amountOfCheck = 100000;
    uint32 lootId = 0;
    std::string lootStore;

    Creature* target = getSelectedCreature();
    if (!target)
    {
        bool usageError = false;
        char* storeStr = nullptr;
        if (!ExtractUInt32(&args, lootId))
            usageError = true;

        if (!usageError)
        {
            storeStr = ExtractLiteralArg(&args);
            if (!storeStr && *args)
                usageError = true;

            if (!usageError && *args && !ExtractUInt32(&args, amountOfCheck))
                usageError = true;
        }

        if (usageError)
        {
            SendSysMessage("Usage: .debug lootdropstats lootId [lootTemplate amountOfCheck]");
            SetSentErrorMessage(true);
            return false;
        }

        if (storeStr)
        {
            lootStore = storeStr;
            if (lootStore == "creature" || lootStore == "c")
                lootStore = "creature";
            else if (lootStore == "gameobject" || lootStore == "gob")
                lootStore = "gameobject";
            else if (lootStore == "fishing" || lootStore == "f")
                lootStore = "fishing";
            else if (lootStore == "item" || lootStore == "i")
                lootStore = "item";
            else if (lootStore == "pickpocketing" || lootStore == "pick")
                lootStore = "pickpocketing";
            else if (lootStore == "skinning" || lootStore == "skin")
                lootStore = "skinning";
            else if (lootStore == "disenchanting" || lootStore == "dis")
                lootStore = "disenchanting";
            else if (lootStore == "mail" || lootStore == "m")
                lootStore = "mail";
            else
            {
                PSendSysMessage("Provided loot template is not valid should be:");
                PSendSysMessage("creature");
                PSendSysMessage("gameobject");
                PSendSysMessage("fishing");
                PSendSysMessage("item");
                PSendSysMessage("pickpocketing");
                PSendSysMessage("skinning");
                PSendSysMessage("disenchanting");
                PSendSysMessage("mail");
                return true;
            }
        }
        else
            lootStore = "creature";
    }
    else
    {
        lootStore = "creature";
        lootId = target->GetCreatureInfo()->LootId;
        ExtractUInt32(&args, amountOfCheck);
    }

    sLootMgr.CheckDropStats(*this, amountOfCheck, lootId, lootStore);
    return true;
}

bool ChatHandler::HandleDebugSendWorldState(char* args)
{
    Player* player = m_session->GetPlayer();
    uint32 worldState;
    if (!ExtractUInt32(&args, worldState))
        return false;

    uint32 value;
    if (!ExtractUInt32(&args, value))
        return false;

    player->SendUpdateWorldState(worldState, value);
    return true;
}

bool ChatHandler::HandleDebugHaveAtClientCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return false;
    }

    if (player->HaveAtClient(target))
        PSendSysMessage("Target %s is at your client.", target->GetName());
    else
        PSendSysMessage("Target %s is not at your client.", target->GetName());

    return true;
}

bool ChatHandler::HandleDebugIsVisibleCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return false;
    }

    Camera& camera = player->GetCamera();
    if (target->isVisibleForInState(player, camera.GetBody(), player->HaveAtClient(target)))
        PSendSysMessage("Target %s should be visible at client.", target->GetName());
    else
        PSendSysMessage("Target %s should not be visible at client.", target->GetName());

    return true;
}

bool ChatHandler::HandleDebugOverflowCommand(char* /*args*/)
{
    std::string name("\360\222\214\245\360\222\221\243\360\222\221\251\360\223\213\215\360\223\213\210\360\223\211\241\360\222\214\245\360\222\221\243\360\222\221\251\360\223\213\215\360\223\213\210\360\223\211\241");
    // Overflow: \xd808\xdf25\xd809\xdc63\xd809\xdc69\xd80c\xdecd\xd80c\xdec8\xd80c\xde61\000\xdf25\xd809\xdc63

    normalizePlayerName(name);
    return true;
}
