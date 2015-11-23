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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Database/DatabaseImpl.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "NPCHandler.h"
#include "SQLStorages.h"

void WorldSession::SendNameQueryOpcode(Player* p)
{
    if (!p)
        return;

    // guess size
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 4 + 4 + 4 + 10));
    data << p->GetObjectGuid();                             // player guid
    data << p->GetName();                                   // played name
    data << uint8(0);                                       // realm name for cross realm BG usage
    data << uint32(p->getRace());
    data << uint32(p->getGender());
    data << uint32(p->getClass());

    SendPacket(&data);
}

void WorldSession::SendNameQueryOpcodeFromDB(ObjectGuid guid)
{
    CharacterDatabase.AsyncPQuery(&WorldSession::SendNameQueryOpcodeFromDBCallBack, GetAccountId(),
                                  //          0     1     2     3       4
                                  "SELECT guid, name, race, gender, class "
                                  "FROM characters WHERE guid = '%u'",
                                  guid.GetCounter());
}

void WorldSession::SendNameQueryOpcodeFromDBCallBack(QueryResult* result, uint32 accountId)
{
    if (!result)
        return;

    WorldSession* session = sWorld.FindSession(accountId);
    if (!session)
    {
        delete result;
        return;
    }

    Field* fields = result->Fetch();
    uint32 lowguid      = fields[0].GetUInt32();
    std::string name = fields[1].GetCppString();
    uint8 pRace = 0, pGender = 0, pClass = 0;
    if (name.empty())
        name         = session->GetMangosString(LANG_NON_EXIST_CHARACTER);
    else
    {
        pRace        = fields[2].GetUInt8();
        pGender      = fields[3].GetUInt8();
        pClass       = fields[4].GetUInt8();
    }

    // guess size
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 4 + 4 + 4 + 10));
    data << ObjectGuid(HIGHGUID_PLAYER, lowguid);
    data << name;
    data << uint8(0);                                       // realm name for cross realm BG usage
    data << uint32(pRace);                                  // race
    data << uint32(pGender);                                // gender
    data << uint32(pClass);                                 // class

    session->SendPacket(&data);
    delete result;
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;

    recv_data >> guid;

    Player* pChar = sObjectMgr.GetPlayer(guid);

    if (pChar)
        SendNameQueryOpcode(pChar);
    else
        SendNameQueryOpcodeFromDB(guid);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket& /*recv_data*/)
{
    SendQueryTimeResponse();
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recv_data)
{
    uint32 entry;
    ObjectGuid guid;

    recv_data >> entry;
    recv_data >> guid;

    Creature* unit = _player->GetMap()->GetAnyTypeCreature(guid);

    // if (unit == nullptr)
    //    sLog.outDebug( "WORLD: HandleCreatureQueryOpcode - (%u) NO SUCH UNIT! (GUID: %u, ENTRY: %u)", uint32(GUID_LOPART(guid)), guid, entry );

    CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(entry);
    if (ci)
    {
        int loc_idx = GetSessionDbLocaleIndex();

        char const* name = ci->Name;
        char const* subName = ci->SubName;
        sObjectMgr.GetCreatureLocaleStrings(entry, loc_idx, &name, &subName);

        DETAIL_LOG("WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name, entry);
        // guess size
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 100);
        data << uint32(entry);                              // creature entry
        data << name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4, always empty
        data << subName;
        data << uint32(ci->CreatureTypeFlags);              // flags
        data << uint32(ci->CreatureType);                   // CreatureType.dbc   wdbFeild8
        data << uint32(ci->Family);                         // CreatureFamily.dbc
        data << uint32(ci->Rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(0);                                  // unknown        wdbFeild11
        data << uint32(ci->PetSpellDataId);                 // Id from CreatureSpellData.dbc    wdbField12
        if (unit)
            data << unit->GetUInt32Value(UNIT_FIELD_DISPLAYID); // DisplayID      wdbFeild13
        else
            data << uint32(Creature::ChooseDisplayId(ci));  // workaround, way to manage models must be fixed

        data << uint16(ci->civilian);                       // wdbFeild14
        SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        DEBUG_LOG("WORLD: CMSG_CREATURE_QUERY - Guid: %s Entry: %u NO CREATURE INFO!",
                  guid.GetString().c_str(), entry);
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recv_data)
{
    uint32 entryID;
    recv_data >> entryID;
    ObjectGuid guid;
    recv_data >> guid;

    const GameObjectInfo* info = ObjectMgr::GetGameObjectInfo(entryID);
    if (info)
    {
        std::string Name = info->name;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            GameObjectLocale const* gl = sObjectMgr.GetGameObjectLocale(entryID);
            if (gl)
            {
                if (gl->Name.size() > size_t(loc_idx) && !gl->Name[loc_idx].empty())
                    Name = gl->Name[loc_idx];
            }
        }
        DETAIL_LOG("WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u. ", info->name, entryID);
        WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 150);
        data << uint32(entryID);
        data << uint32(info->type);
        data << uint32(info->displayId);
        data << Name;
        data << uint16(0) << uint8(0) << uint8(0);          // name2, name3, name4
        data.append(info->raw.data, 24);
        // data << float(info->size);                       // go size , to check
        SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        DEBUG_LOG("WORLD: CMSG_GAMEOBJECT_QUERY - Guid: %s Entry: %u Missing gameobject info!",
                  guid.GetString().c_str(), entryID);
        WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 4);
        data << uint32(entryID | 0x80000000);
        SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recv_data*/)
{
    DETAIL_LOG("WORLD: Received opcode MSG_CORPSE_QUERY");

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
    {
        WorldPacket data(MSG_CORPSE_QUERY, 1);
        data << uint8(0);                                   // corpse not found
        SendPacket(&data);
        return;
    }

    uint32 corpsemapid = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    int32 mapid = corpsemapid;

    // if corpse at different map
    if (corpsemapid != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (InstanceTemplate const* temp = sObjectMgr.GetInstanceTemplate(mapid))
        {
            if (temp->ghostEntranceMap >= 0)
            {
                // if corpse map have entrance
                if (TerrainInfo const* entranceMap = sTerrainMgr.LoadTerrain(temp->ghostEntranceMap))
                {
                    mapid = temp->ghostEntranceMap;
                    x = temp->ghostEntranceX;
                    y = temp->ghostEntranceY;
                    z = entranceMap->GetHeightStatic(x, y, MAX_HEIGHT);
                }
            }
        }
    }

    WorldPacket data(MSG_CORPSE_QUERY, 1 + (5 * 4));
    data << uint8(1);                                       // corpse found
    data << int32(mapid);
    data << float(x);
    data << float(y);
    data << float(z);
    data << uint32(corpsemapid);
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recv_data)
{
    uint32 textID;
    ObjectGuid guid;

    recv_data >> textID;
    recv_data >> guid;

    DETAIL_LOG("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    _player->SetTargetGuid(guid);

    GossipText const* pGossip = sObjectMgr.GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);            // guess size
    data << textID;

    if (!pGossip)
    {
        for (uint32 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << float(0);
            data << "Greetings $N";
            data << "Greetings $N";
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        std::string Text_0[MAX_GOSSIP_TEXT_OPTIONS], Text_1[MAX_GOSSIP_TEXT_OPTIONS];
        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            Text_0[i] = pGossip->Options[i].Text_0;
            Text_1[i] = pGossip->Options[i].Text_1;
        }

        int loc_idx = GetSessionDbLocaleIndex();

        sObjectMgr.GetNpcTextLocaleStringsAll(textID, loc_idx, &Text_0, &Text_1);

        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << pGossip->Options[i].Probability;

            if (Text_0[i].empty())
                data << Text_1[i];
            else
                data << Text_0[i];

            if (Text_1[i].empty())
                data << Text_0[i];
            else
                data << Text_1[i];

            data << pGossip->Options[i].Language;

            for (int j = 0; j < 3; ++j)
            {
                data << pGossip->Options[i].Emotes[j]._Delay;
                data << pGossip->Options[i].Emotes[j]._Emote;
            }
        }
    }

    SendPacket(&data);

    DEBUG_LOG("WORLD: Sent SMSG_NPC_TEXT_UPDATE");
}

void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recv_data)
{
    DETAIL_LOG("WORLD: Received opcode CMSG_PAGE_TEXT_QUERY");

    uint32 pageID;
    recv_data >> pageID;

    while (pageID)
    {
        PageText const* pPage = sPageTextStore.LookupEntry<PageText>(pageID);
        // guess size
        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 50);
        data << pageID;

        if (!pPage)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            std::string Text = pPage->Text;

            int loc_idx = GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
            {
                PageTextLocale const* pl = sObjectMgr.GetPageTextLocale(pageID);
                if (pl)
                {
                    if (pl->Text.size() > size_t(loc_idx) && !pl->Text[loc_idx].empty())
                        Text = pl->Text[loc_idx];
                }
            }

            data << Text;
            data << uint32(pPage->Next_Page);
            pageID = pPage->Next_Page;
        }
        SendPacket(&data);

        DEBUG_LOG("WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4);
    data << uint32(time(nullptr));
    SendPacket(&data);
}
