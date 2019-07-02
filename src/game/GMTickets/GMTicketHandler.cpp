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
#include "WorldPacket.h"
#include "Log.h"
#include "GMTickets/GMTicketMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/Player.h"
#include "Chat/Chat.h"

void WorldSession::SendGMTicketGetTicket(uint32 status, GMTicket* ticket /*= nullptr*/) const
{
    std::string text = ticket ? ticket->GetText() : "";

    if (ticket && ticket->HasResponse())
    {
        text += "\n\n";

        std::string textFormat = GetMangosString(LANG_COMMAND_TICKETRESPONSE);
        char textBuf[1024];
        snprintf(textBuf, 1024, textFormat.c_str(), ticket->GetResponse());

        text += textBuf;
    }

    size_t len = text.size() + 1;
    WorldPacket data(SMSG_GMTICKET_GETTICKET, (4 + len + 1 + 4 + 2 + 4 + 4));
    data << uint32(status);                                 // standard 0x0A, 0x06 if text present
    if (status == 6)
    {
        data << text;                                       // ticket text
        data << uint8(0x7);                                 // ticket category
        data << float(0);                                   // tickets in queue?
        data << float(0);                                   // if > "tickets in queue" then "We are currently experiencing a high volume of petitions."
        data << float(0);                                   // 0 - "Your ticket will be serviced soon", 1 - "Wait time currently unavailable"
        data << uint8(0);                                   // if == 2 and next field == 1 then "Your ticket has been escalated"
        data << uint8(0);                                   // const
    }
    SendPacket(data);
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket& /*recv_data*/)
{
    SendQueryTimeResponse();

    GMTicket* ticket = sTicketMgr.GetGMTicket(GetPlayer()->GetObjectGuid());
    if (ticket)
        SendGMTicketGetTicket(0x06, ticket);
    else
        SendGMTicketGetTicket(0x0A);
}

void WorldSession::HandleGMTicketUpdateTextOpcode(WorldPacket& recv_data)
{
    std::string ticketText;
    recv_data >> ticketText;

    if (GMTicket* ticket = sTicketMgr.GetGMTicket(GetPlayer()->GetObjectGuid()))
        ticket->SetText(ticketText.c_str());
    else
        sLog.outError("Ticket update: Player %s (GUID: %u) doesn't have active ticket", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
}

void WorldSession::HandleGMTicketDeleteTicketOpcode(WorldPacket& /*recv_data*/)
{
    sTicketMgr.Delete(GetPlayer()->GetObjectGuid());

    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32(9);
    SendPacket(data);

    SendGMTicketGetTicket(0x0A);
}

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recv_data)
{
    uint32 map;
    float x, y, z;
    std::string ticketText;

    recv_data >> map >> x >> y >> z;                        // last check 2.4.3
    recv_data >> ticketText;

    recv_data.read_skip<uint32>();                          // unk1, 0
    recv_data.read_skip<uint32>();                          // unk2, 1
    recv_data.read_skip<uint32>();                          // unk3, 0

    DEBUG_LOG("TicketCreate: map %u, x %f, y %f, z %f, text %s", map, x, y, z, ticketText.c_str());

    if (sTicketMgr.GetGMTicket(GetPlayer()->GetObjectGuid()))
    {
        WorldPacket data(SMSG_GMTICKET_CREATE, 4);
        data << uint32(1);                                  // 1 - You already have GM ticket
        SendPacket(data);
        return;
    }

    sTicketMgr.Create(_player->GetObjectGuid(), ticketText.c_str());

    SendQueryTimeResponse();

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    data << uint32(2);                                      // 2 - nothing appears (3-error creating, 5-error updating)
    SendPacket(data);

    // TODO: Guard player map
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second->GetSession()->GetSecurity() >= SEC_GAMEMASTER && itr->second->isAcceptTickets())
            ChatHandler(itr->second).PSendSysMessage(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName());
    }
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32(1);                                      // we can also disable ticket system by sending 0 value

    SendPacket(data);
}

void WorldSession::HandleGMSurveySubmitOpcode(WorldPacket& recv_data)
{
    // GM survey is shown after SMSG_GM_TICKET_STATUS_UPDATE with status = 3
    uint32 x;
    recv_data >> x;                                         // answer range? (6 = 0-5?)
    DEBUG_LOG("SURVEY: X = %u", x);

    uint8 result[10];
    memset(result, 0, sizeof(result));
    for (unsigned char& i : result)
    {
        uint32 questionID;
        recv_data >> questionID;                            // GMSurveyQuestions.dbc
        if (!questionID)
            break;

        uint8 value;
        std::string unk_text;
        recv_data >> value;                                 // answer
        recv_data >> unk_text;                              // always empty?

        i = value;
        DEBUG_LOG("SURVEY: ID %u, value %u, text %s", questionID, value, unk_text.c_str());
    }

    std::string comment;
    recv_data >> comment;                                   // addional comment
    DEBUG_LOG("SURVEY: comment %s", comment.c_str());

    // TODO: chart this data in some way
}
