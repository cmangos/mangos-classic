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
#include "Tools/Language.h"
#include "Database/DatabaseEnv.h"
#include "Database/DatabaseImpl.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Player.h"
#include "Entities/Corpse.h"
#include "Entities/NPCHandler.h"
#include "Server/SQLStorages.h"
#include "Maps/GridDefines.h"

void WorldSession::SendNameQueryResponse(CharacterNameQueryResponse& response) const
{

    // guess size
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 4 + 4 + 4 + 10));
    data << response.guid;
    data << (!response.name.empty() ? response.name : GetMangosString(LANG_NON_EXIST_CHARACTER));

    if (response.realm.empty())
        data << uint8(0);
    else
        data << response.realm;

    data << response.race;
    data << response.gender;
    data << response.classid;

    SendPacket(data, true);
}

void WorldSession::SendNameQueryResponseFromDB(ObjectGuid guid) const
{
    CharacterDatabase.AsyncPQuery(&WorldSession::SendNameQueryResponseFromDBCallBack, GetAccountId(),
                                  //          0     1     2     3       4
                                  "SELECT guid, name, race, gender, class "
                                  "FROM characters WHERE guid = '%u'",
                                  guid.GetCounter());
}

void WorldSession::SendNameQueryResponseFromDBCallBack(QueryResult* result, uint32 accountId)
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

    CharacterNameQueryResponse response;

    response.guid = ObjectGuid(HIGHGUID_PLAYER, fields[0].GetUInt32());
    response.name = fields[1].GetCppString();
    response.realm = "";

    if (!response.name.empty())
    {
        response.race = fields[2].GetUInt8();
        response.gender = fields[3].GetUInt8();
        response.classid = fields[4].GetUInt8();
    }

    if (session->m_sessionState != WORLD_SESSION_STATE_READY)
        session->m_offlineNameResponses.push_back(response);
    else
        session->SendNameQueryResponse(response);

    delete result;
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recv_data)
{
    ObjectGuid guid;

    recv_data >> guid;

    // When not logged in: check if name was already queried
    if (m_sessionState != WORLD_SESSION_STATE_READY)
    {
        auto result = m_offlineNameQueries.insert(guid);

        if (!result.second)
            return;
    }

    Player* pChar = sObjectMgr.GetPlayer(guid);

    if (pChar)
    {
        CharacterNameQueryResponse response;

        response.guid = pChar->GetObjectGuid();
        response.name = pChar->GetName();
        response.realm = "";
        response.race = uint32(pChar->getRace());
        response.gender = uint32(pChar->getGender());
        response.classid = uint32(pChar->getClass());

        if (m_sessionState != WORLD_SESSION_STATE_READY)
            m_offlineNameResponses.push_back(response);
        else
            SendNameQueryResponse(response);
    }
    else
        SendNameQueryResponseFromDB(guid);
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
    recv_data >> guid; // not checking guid for optimization purposes - when adding quest, guid is sent as 0 so its useless anyway

    //Creature* unit = _player->GetMap()->GetAnyTypeCreature(guid);

    //if (!unit || (unit->GetEntry() != entry && unit->GetObjectGuid().GetEntry() != entry))
    //    return;

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
        data << uint32(ci->TypeFlags);                      // flags
        data << uint32(ci->CreatureType);                   // CreatureType.dbc   wdbFeild8
        data << uint32(ci->Family);                         // CreatureFamily.dbc
        data << uint32(ci->Rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(0);                                  // unknown        wdbFeild11
        data << uint32(ci->PetSpellDataId);                 // Id from CreatureSpellData.dbc    wdbField12
        data << uint32(ci->DisplayId[0]);                   // DisplayID      wdbFeild13

        data << uint16(ci->Civilian);                       // wdbFeild14
        SendPacket(data);
        DEBUG_LOG("WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        DEBUG_LOG("WORLD: CMSG_CREATURE_QUERY - Guid: %s Entry: %u NO CREATURE INFO!",
                  guid.GetString().c_str(), entry);
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        SendPacket(data);
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
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4
        data << uint8(0);                                   // unknown
        data.append(info->raw.data, 24);
        SendPacket(data);
        DEBUG_LOG("WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        DEBUG_LOG("WORLD: CMSG_GAMEOBJECT_QUERY - Guid: %s Entry: %u Missing gameobject info!",
                  guid.GetString().c_str(), entryID);
        WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 4);
        data << uint32(entryID | 0x80000000);
        SendPacket(data);
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
        SendPacket(data);
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
            if (temp->ghost_entrance_map >= 0)
            {
                // if corpse map have entrance
                if (TerrainInfo const* entranceMap = sTerrainMgr.LoadTerrain(temp->ghost_entrance_map))
                {
                    mapid = temp->ghost_entrance_map;
                    x = temp->ghost_entrance_x;
                    y = temp->ghost_entrance_y;
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
    SendPacket(data);
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recv_data)
{
    uint32 textID;
    ObjectGuid guid;

    recv_data >> textID;
    recv_data >> guid;

    DETAIL_LOG("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    GossipText const* gossip = sObjectMgr.GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);            // guess size
    data << textID;

    if (!gossip)
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
        bool locales = true;
        int loc_idx = GetSessionDbLocaleIndex();
        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            if (gossip->Options[i].broadcastTextId)
            {
                locales = false;
                Text_0[i] = sObjectMgr.GetBroadcastText(gossip->Options[i].broadcastTextId)->GetText(loc_idx, GENDER_MALE);
                Text_1[i] = sObjectMgr.GetBroadcastText(gossip->Options[i].broadcastTextId)->GetText(loc_idx, GENDER_FEMALE);
            }
            else if (locales)
            {
                Text_0[i] = gossip->Options[i].Text_0;
                Text_1[i] = gossip->Options[i].Text_1;
            }
        }

        if (locales)
            sObjectMgr.GetNpcTextLocaleStringsAll(textID, loc_idx, &Text_0, &Text_1);

        for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << gossip->Options[i].Probability;

            if (Text_0[i].empty())
                data << Text_1[i];
            else
                data << Text_0[i];

            if (Text_1[i].empty())
                data << Text_0[i];
            else
                data << Text_1[i];

            data << gossip->Options[i].Language;

            for (auto Emote : gossip->Options[i].Emotes)
            {
                data << Emote._Delay;
                data << Emote._Emote;
            }
        }
    }

    SendPacket(data);

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
        SendPacket(data);

        DEBUG_LOG("WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}

void WorldSession::SendQueryTimeResponse() const
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4);
    data << uint32(time(nullptr));
    SendPacket(data);
}
