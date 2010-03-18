/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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
#include "Language.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"

// Charters ID in item_template
#define GUILD_CHARTER               5863
#define GUILD_CHARTER_COST          1000                    // 10 S
#define CHARTER_DISPLAY_ID          16161

void WorldSession::HandlePetitionBuyOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode CMSG_PETITION_BUY");
    //recv_data.hexlike();

    uint64 guidNPC;
    uint64 unk1, unk3, unk4, unk5, unk6, unk7;
    uint32 unk2;
    std::string name;
    uint16 unk8;
    uint8  unk9;
    uint32 unk10;                                           // selected index
    uint32 unk11;
    recv_data >> guidNPC;                                   // NPC GUID
    recv_data >> unk1;                                      // 0
    recv_data >> unk2;                                      // 0
    recv_data >> name;                                      // name

    recv_data >> unk3;                                      // 0
    recv_data >> unk4;                                      // 0
    recv_data >> unk5;                                      // 0
    recv_data >> unk6;                                      // 0
    recv_data >> unk7;                                      // 0
    recv_data >> unk8;                                      // 0
    recv_data >> unk9;                                      // 0
    recv_data >> unk10;                                     // index
    recv_data >> unk11;                                     // 0
    sLog.outDebug("Petitioner with GUID %u tried sell petition: name %s", GUID_LOPART(guidNPC), name.c_str());

    // prevent cheating
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guidNPC,UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        sLog.outDebug("WORLD: HandlePetitionBuyOpcode - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(guidNPC));
        return;
    }

    if (!pCreature->isTabardDesigner())
        return;

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // if tabard designer, then trying to buy a guild charter.
    // do not let if already in guild.
    if(_player->GetGuildId())
        return;

    uint32 charterid = GUILD_CHARTER;
    uint32 cost = GUILD_CHARTER_COST;

    if(sObjectMgr.GetGuildByName(name))
    {
        SendGuildCommandResult(GUILD_CREATE_S, name, ERR_GUILD_NAME_EXISTS_S);
        return;
    }
    if(sObjectMgr.IsReservedName(name) || !ObjectMgr::IsValidCharterName(name))
    {
        SendGuildCommandResult(GUILD_CREATE_S, name, ERR_GUILD_NAME_INVALID);
        return;
    }

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(charterid);
    if(!pProto)
    {
        _player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, charterid, 0);
        return;
    }

    if(_player->GetMoney() < cost)
    {                                                       //player hasn't got enough money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, charterid, 0);
        return;
    }

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->BuyCount );
    if(msg != EQUIP_ERR_OK)
    {
        _player->SendBuyError(msg, pCreature, charterid, 0);
        return;
    }

    _player->ModifyMoney(-(int32)cost);
    Item *charter = _player->StoreNewItem(dest, charterid, true);
    if(!charter)
        return;

    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, charter->GetGUIDLow());
    // ITEM_FIELD_ENCHANTMENT is guild/arenateam id
    // ITEM_FIELD_ENCHANTMENT+1 is current signatures count (showed on item)
    charter->SetState(ITEM_CHANGED, _player);
    _player->SendNewItem(charter, 1, true, false);

    // a petition is invalid, if both the owner and the type matches
    // we checked above, if this player is in an arenateam, so this must be data corruption
    QueryResult *result = CharacterDatabase.PQuery("SELECT petitionguid FROM petition WHERE ownerguid = '%u'", _player->GetGUIDLow());

    std::ostringstream ssInvalidPetitionGUIDs;

    if (result)
    {

        do
        {
            Field *fields = result->Fetch();
            ssInvalidPetitionGUIDs << "'" << fields[0].GetUInt32() << "' , ";
        } while (result->NextRow());

        delete result;
    }

    // delete petitions with the same guid as this one
    ssInvalidPetitionGUIDs << "'" << charter->GetGUIDLow() << "'";

    sLog.outDebug("Invalid petition GUIDs: %s", ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.escape_string(name);
    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM petition WHERE petitionguid IN ( %s )",  ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid IN ( %s )", ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.PExecute("INSERT INTO petition (ownerguid, petitionguid, name) VALUES ('%u', '%u', '%s')",
        _player->GetGUIDLow(), charter->GetGUIDLow(), name.c_str());
    CharacterDatabase.CommitTransaction();
}

void WorldSession::HandlePetitionShowSignOpcode(WorldPacket & recv_data)
{
                                                            // ok
    sLog.outDebug("Received opcode CMSG_PETITION_SHOW_SIGNATURES");
    //recv_data.hexlike();

    uint8 signs = 0;
    uint64 petitionguid;
    recv_data >> petitionguid;                              // petition guid

    // solve (possible) some strange compile problems with explicit use GUID_LOPART(petitionguid) at some GCC versions (wrong code optimization in compiler?)
    uint32 petitionguid_low = GUID_LOPART(petitionguid);

    // if guild petition and has guild => error, return;
    if(_player->GetGuildId())
        return;

    QueryResult* result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitionguid_low);

    // result==NULL also correct in case no sign yet
    if(result)
        signs = (uint8)result->GetRowCount();

    sLog.outDebug("CMSG_PETITION_SHOW_SIGNATURES petition entry: '%u'", petitionguid_low);

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+1+signs*12));
    data << uint64(petitionguid);                           // petition guid
    data << uint64(_player->GetGUID());                     // owner guid
    data << uint32(petitionguid_low);                       // guild guid (in mangos always same as GUID_LOPART(petitionguid)
    data << uint8(signs);                                   // sign's count

    for(uint8 i = 1; i <= signs; ++i)
    {
        Field *fields2 = result->Fetch();
        uint64 plguid = fields2[0].GetUInt64();

        data << uint64(plguid);                             // Player GUID
        data << uint32(0);                                  // there 0 ...

        result->NextRow();
    }
    delete result;
    SendPacket(&data);
}

void WorldSession::HandlePetitionQueryOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode CMSG_PETITION_QUERY");   // ok
    //recv_data.hexlike();

    uint32 guildguid;
    uint64 petitionguid;
    recv_data >> guildguid;                                 // in mangos always same as GUID_LOPART(petitionguid)
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("CMSG_PETITION_QUERY Petition GUID %u Guild GUID %u", GUID_LOPART(petitionguid), guildguid);

    SendPetitionQueryOpcode(petitionguid);
}

void WorldSession::SendPetitionQueryOpcode(uint64 petitionguid)
{
    uint64 ownerguid = 0;
    std::string name = "NO_NAME_FOR_GUID";
    uint8 signs = 0;

    QueryResult *result = CharacterDatabase.PQuery(
        "SELECT ownerguid, name, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs "
        "FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if(result)
    {
        Field* fields = result->Fetch();
        ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        name      = fields[1].GetCppString();
        signs     = fields[2].GetUInt8();
        delete result;
    }
    else
    {
        sLog.outDebug("CMSG_PETITION_QUERY failed for petition (GUID: %u)", GUID_LOPART(petitionguid));
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, (4+8+name.size()+1+2+4*11));
    data << GUID_LOPART(petitionguid);                      // guild/team guid (in mangos always same as GUID_LOPART(petition guid)
    data << ownerguid;                                      // charter owner guid
    data << name;                                           // name (guild/arena team)
    data << uint8(0);                                       // 1
    data << uint32(1);
    data << uint32(9);
    data << uint32(9);                                      // bypass client - side limitation, a different value is needed here for each petition
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field
    data << uint32(0);                                      // 10
    data << uint32(0);                                      // 11
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    SendPacket(&data);
}

void WorldSession::HandlePetitionRenameOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode MSG_PETITION_RENAME");   // ok
    //recv_data.hexlike();

    uint64 petitionguid;
    std::string newname;

    recv_data >> petitionguid;                              // guid
    recv_data >> newname;                                   // new name

    Item *item = _player->GetItemByGuid(petitionguid);
    if(!item)
        return;

    if(sObjectMgr.GetGuildByName(newname))
    {
        SendGuildCommandResult(GUILD_CREATE_S, newname, ERR_GUILD_NAME_EXISTS_S);
        return;
    }
    if(sObjectMgr.IsReservedName(newname) || !ObjectMgr::IsValidCharterName(newname))
    {
        SendGuildCommandResult(GUILD_CREATE_S, newname, ERR_GUILD_NAME_INVALID);
        return;
    }

    std::string db_newname = newname;
    CharacterDatabase.escape_string(db_newname);
    CharacterDatabase.PExecute("UPDATE petition SET name = '%s' WHERE petitionguid = '%u'",
        db_newname.c_str(), GUID_LOPART(petitionguid));

    sLog.outDebug("Petition (GUID: %u) renamed to '%s'", GUID_LOPART(petitionguid), newname.c_str());

    WorldPacket data(MSG_PETITION_RENAME, (8+newname.size()+1));
    data << uint64(petitionguid);
    data << newname;
    SendPacket(&data);
}

void WorldSession::HandlePetitionSignOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode CMSG_PETITION_SIGN");    // ok
    //recv_data.hexlike();

    Field *fields;
    uint64 petitionguid;
    uint8 unk;
    recv_data >> petitionguid;                              // petition guid
    recv_data >> unk;

    QueryResult *result = CharacterDatabase.PQuery(
        "SELECT ownerguid, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs "
        "FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if(!result)
    {
        sLog.outError("any petition on server...");
        return;
    }

    fields = result->Fetch();
    uint64 ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
    uint8 signs = fields[1].GetUInt8();

    delete result;

    uint32 plguidlo = _player->GetGUIDLow();
    if(GUID_LOPART(ownerguid) == plguidlo)
        return;

    // not let enemies sign guild charter
    if(!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != sObjectMgr.GetPlayerTeamByGUID(ownerguid))
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_NOT_ALLIED);
        return;
    }

    if(_player->GetGuildId())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_IN_GUILD_S);
        return;
    }
    if(_player->GetGuildIdInvited())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    if(++signs > 9)                                         // client signs maximum
        return;

    //client doesn't allow to sign petition two times by one character, but not check sign by another character from same account
    //not allow sign another player from already sign player account
    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE player_account = '%u' AND petitionguid = '%u'", GetAccountId(), GUID_LOPART(petitionguid));

    if(result)
    {
        delete result;
        WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
        data << uint64(petitionguid);
        data << uint64(_player->GetGUID());
        data << uint32(PETITION_SIGN_ALREADY_SIGNED);

        // close at signer side
        SendPacket(&data);

        // update for owner if online
        if(Player *owner = sObjectMgr.GetPlayer(ownerguid))
            owner->GetSession()->SendPacket(&data);
        return;
    }

    CharacterDatabase.PExecute("INSERT INTO petition_sign (ownerguid,petitionguid, playerguid, player_account) VALUES ('%u', '%u', '%u','%u')", GUID_LOPART(ownerguid),GUID_LOPART(petitionguid), plguidlo,GetAccountId());

    sLog.outDebug("PETITION SIGN: GUID %u by player: %s (GUID: %u Account: %u)", GUID_LOPART(petitionguid), _player->GetName(),plguidlo,GetAccountId());

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
    data << uint64(petitionguid);
    data << uint64(_player->GetGUID());
    data << uint32(PETITION_SIGN_OK);

    // close at signer side
    SendPacket(&data);

    // update signs count on charter, required testing...
    //Item *item = _player->GetItemByGuid(petitionguid));
    //if(item)
    //    item->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+1, signs);

    // update for owner if online
    if(Player *owner = sObjectMgr.GetPlayer(ownerguid))
        owner->GetSession()->SendPacket(&data);
}

void WorldSession::HandlePetitionDeclineOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode MSG_PETITION_DECLINE");  // ok
    //recv_data.hexlike();

    uint64 petitionguid;
    uint64 ownerguid;
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("Petition %u declined by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    QueryResult *result = CharacterDatabase.PQuery("SELECT ownerguid FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if(!result)
        return;

    Field *fields = result->Fetch();
    ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
    delete result;

    Player *owner = sObjectMgr.GetPlayer(ownerguid);
    if(owner)                                               // petition owner online
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << uint64(_player->GetGUID());
        owner->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleOfferPetitionOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode CMSG_OFFER_PETITION");   // ok
    //recv_data.hexlike();

    uint8 signs = 0;
    uint64 petitionguid, plguid;
    Player *player;                                     // this is not petition type!
    recv_data >> petitionguid;                              // petition guid
    recv_data >> plguid;                                    // player guid

    player = ObjectAccessor::FindPlayer(plguid);
    if (!player)
        return;

    sLog.outDebug("OFFER PETITION: GUID1 %u, to player id: %u", GUID_LOPART(petitionguid), GUID_LOPART(plguid));

    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != player->GetTeam() )
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_NOT_ALLIED);
        return;
    }

    if(player->GetGuildId())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_IN_GUILD_S);
        return;
    }

    if(player->GetGuildIdInvited())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    QueryResult* result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    // result==NULL also correct charter without signs
    if(result)
        signs = (uint8)result->GetRowCount();

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+signs+signs*12));
    data << uint64(petitionguid);                           // petition guid
    data << uint64(_player->GetGUID());                     // owner guid
    data << uint32(GUID_LOPART(petitionguid));              // guild guid (in mangos always same as GUID_LOPART(petition guid)
    data << uint8(signs);                                   // sign's count

    for(uint8 i = 1; i <= signs; ++i)
    {
        Field *fields2 = result->Fetch();
        plguid = fields2[0].GetUInt64();

        data << uint64(plguid);                             // Player GUID
        data << uint32(0);                                  // there 0 ...

        result->NextRow();
    }

    delete result;
    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleTurnInPetitionOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received opcode CMSG_TURN_IN_PETITION"); // ok
    //recv_data.hexlike();

    WorldPacket data;
    uint64 petitionguid;

    uint32 ownerguidlo;
    std::string name;

    recv_data >> petitionguid;

    sLog.outDebug("Petition %u turned in by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    // data
    QueryResult *result = CharacterDatabase.PQuery("SELECT ownerguid, name FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if(result)
    {
        Field *fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt32();
        name = fields[1].GetCppString();
        delete result;
    }
    else
    {
        sLog.outError("petition table has broken data!");
        return;
    }

    if(_player->GetGuildId())
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << uint32(PETITION_TURN_ALREADY_IN_GUILD);     // already in guild
        _player->GetSession()->SendPacket(&data);
        return;
    }

    if(_player->GetGUIDLow() != ownerguidlo)
        return;

    // signs
    uint8 signs;
    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if(result)
        signs = (uint8)result->GetRowCount();
    else
        signs = 0;

    uint32 count = sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS);

    if(signs < count)
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << uint32(PETITION_TURN_NEED_MORE_SIGNATURES); // need more signatures...
        SendPacket(&data);
        delete result;
        return;
    }

    if(sObjectMgr.GetGuildByName(name))
    {
        SendGuildCommandResult(GUILD_CREATE_S, name, ERR_GUILD_NAME_EXISTS_S);
        delete result;
        return;
    }

    // and at last charter item check
    Item *item = _player->GetItemByGuid(petitionguid);
    if(!item)
    {
        delete result;
        return;
    }

    // OK!

    // delete charter item
    _player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);

    Guild* guild = new Guild;
    if(!guild->Create(_player, name))
    {
        delete guild;
        delete result;
        return;
    }

    // register guild and add guildmaster
    sObjectMgr.AddGuild(guild);

    // add members
    for(uint8 i = 0; i < signs; ++i)
    {
        Field* fields = result->Fetch();
        guild->AddMember(fields[0].GetUInt64(), guild->GetLowestRank());
        result->NextRow();
    }

    delete result;

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    CharacterDatabase.CommitTransaction();

    // created
    sLog.outDebug("TURN IN PETITION GUID %u", GUID_LOPART(petitionguid));

    data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << uint32(PETITION_TURN_OK);
    SendPacket(&data);
}

void WorldSession::HandlePetitionShowListOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("Received CMSG_PETITION_SHOWLIST");       // ok
    //recv_data.hexlike();

    uint64 guid;
    recv_data >> guid;

    SendPetitionShowList(guid);
}

void WorldSession::SendPetitionShowList(uint64 guid)
{
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        sLog.outDebug("WORLD: HandlePetitionShowListOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    uint8 count = 1;

    WorldPacket data(SMSG_PETITION_SHOWLIST, 8+1+4*6);
    data << guid;                                           // npc guid
    data << count;                                          // count
    data << uint32(1);                                  // index
    data << uint32(GUILD_CHARTER);                      // charter entry
    data << uint32(CHARTER_DISPLAY_ID);                 // charter display id
    data << uint32(GUILD_CHARTER_COST);                 // charter cost
    data << uint32(1);                                  // unknown
    data << uint32(9);                                  // required signs?
    //for(uint8 i = 0; i < count; ++i)
    //{
    //    data << uint32(i);                        // index
    //    data << uint32(GUILD_CHARTER);            // charter entry
    //    data << uint32(CHARTER_DISPLAY_ID);       // charter display id
    //    data << uint32(GUILD_CHARTER_COST+i);     // charter cost
    //    data << uint32(0);                        // unknown
    //    data << uint32(9);                        // required signs?
    //}
    SendPacket(&data);
    sLog.outDebug("Sent SMSG_PETITION_SHOWLIST");
}
