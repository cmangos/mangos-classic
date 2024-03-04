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
#include "Server/WorldPacket.h"
#include "Log/Log.h"
#include "GMTickets/GMTicketMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/Player.h"
#include "Chat/Chat.h"

void WorldSession::SendGMTicketResult(uint32 opcode, uint32 result) const
{
    WorldPacket data(Opcodes(opcode), 4);
    data << result;
    SendPacket(data);
}

void WorldSession::SendGMTicket(const GMTicket& ticket, time_t now/* = time(nullptr)*/) const
{
    float daysTicketAge = sTicketMgr.GetTicketAgeDays(now, ticket.GetCreatedAt());
    float daysOldestTicketAge = sTicketMgr.GetOldestTicketAgeDays();
    float daysLastUpdated = sTicketMgr.GetOldestTicketAgeLastUpdateDays(now);

    WorldPacket data(SMSG_GMTICKET_GETTICKET, 4);
    data << uint32(GMTICKET_STATUS_HASTEXT);
    data << ticket.GetText();                   // Ticket text: data, should never exceed 1999 bytes
    data << uint8(ticket.GetCategory());        // Ticket category
    data << float(daysTicketAge);               // Days passed since the ticket got in the queue
    data << float(daysOldestTicketAge);         // Days passed since oldest ticket got in the queue
    data << float(daysLastUpdated);             // Days passed since the last update to oldestTicketTime
    data << uint8(ticket.GetStatus());          // Escalated status of the ticket
    data << uint8(ticket.IsSeen());             // Ticket has been read by a gm
    SendPacket(data);
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    SendGMTicketResult(SMSG_GMTICKET_SYSTEMSTATUS, sTicketMgr.GetSystemStatus());
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket& /*recv_data*/)
{
    if (sTicketMgr.GetSystemStatus() != GMTICKET_QUEUE_STATUS_ENABLED)
         return SendGMTicketResult(SMSG_GMTICKET_GETTICKET, GMTICKET_STATUS_DEFAULT);

    if (GMTicket* open = sTicketMgr.GetTicketByPlayer(GetPlayer()->GetObjectGuid()))
        SendGMTicket(*open);
    else
        SendGMTicketResult(SMSG_GMTICKET_GETTICKET, GMTICKET_STATUS_DEFAULT);
}

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recv_data)
{
    if (sTicketMgr.GetSystemStatus() != GMTICKET_QUEUE_STATUS_ENABLED || !m_ticketSquelchTimer.Passed())
    {
        recv_data.read_skip(recv_data.size());
        SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_ERROR);
        return;
    }

    uint8 category;                                         // Pre-TBC: parsed from the packet
    uint32 map;
    float x, y, z;
    std::string message;

    recv_data >> category;                                  // Pre-TBC

    recv_data >> map >> x >> y >> z;
    recv_data >> message;

    std::string reserved;
    recv_data >> reserved;                                  // Pre-TBC: "Reserved for future use"

    if (category == 2)                                      // Pre-TBC: "Behavior/Harassment"
    {
        uint32 chatDataLineCount;
        recv_data >> chatDataLineCount;

        uint32 chatDataSizeInflated;
        recv_data >> chatDataSizeInflated;

        if (size_t chatDataSizeDeflated = (recv_data.size() - recv_data.rpos()))
            recv_data.read_skip(chatDataSizeDeflated);          // Compressed chat data
    }

    Player* player = GetPlayer();
    const ObjectGuid& guid = player->GetObjectGuid();

    if (!CheckChatMessage(message))
    {
        SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_ERROR);
        return;
    }

    // Check if open ticket already exists
    if (GMTicket* existing = sTicketMgr.GetTicketByPlayer(guid))
    {
        m_ticketSquelchTimer.Reset(5000);
        SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_ALREADY_EXIST);
    }
    else
    {
        // First, try to recycle stored abandoned ticket as new: to avoid new/abandon db spam
        if (GMTicket* abandoned = sTicketMgr.GetTicketByPlayer(player->GetObjectGuid(), GMTICKET_STATE_ABANDONED))
        {
            abandoned->Recycle(GetPlayer(), message, category);

            if (sTicketMgr.Recycle(abandoned))
            {
                m_ticketSquelchTimer.Reset(5000);
                SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_SUCCESS);
            }
            else
                SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_ERROR);
        }
        else
        {
            GMTicket* ticket = new GMTicket(GetPlayer(), message, category);

            if (sTicketMgr.Add(ticket))
            {
                m_ticketSquelchTimer.Reset(5000);
                SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_SUCCESS);
            }
            else
            {
                delete ticket;
                SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_CREATE_ERROR);
            }
        }
    }
}

void WorldSession::HandleGMTicketUpdateTextOpcode(WorldPacket& recv_data)
{
    if (sTicketMgr.GetSystemStatus() != GMTICKET_QUEUE_STATUS_ENABLED || !m_ticketSquelchTimer.Passed())
    {
        recv_data.read_skip(recv_data.size());
        SendGMTicketResult(SMSG_GMTICKET_CREATE, GMTICKET_RESPONSE_UPDATE_ERROR);
        return;
    }

    std::string message;
    recv_data >> message;

    if (!CheckChatMessage(message))
    {
        SendGMTicketResult(SMSG_GMTICKET_UPDATETEXT, GMTICKET_RESPONSE_UPDATE_ERROR);
        return;
    }

    if (sTicketMgr.Update(sTicketMgr.GetTicketByPlayer(GetPlayer()->GetObjectGuid()), message) == GMTicketMgr::COMMAND_RESULT_SUCCESS)
    {
        m_ticketSquelchTimer.Reset(5000);
        SendGMTicketResult(SMSG_GMTICKET_UPDATETEXT, GMTICKET_RESPONSE_UPDATE_SUCCESS);
    }
    else
        SendGMTicketResult(SMSG_GMTICKET_UPDATETEXT, GMTICKET_RESPONSE_UPDATE_ERROR);
}

void WorldSession::HandleGMTicketDeleteTicketOpcode(WorldPacket& /*recv_data*/)
{
    if (sTicketMgr.GetSystemStatus() != GMTICKET_QUEUE_STATUS_ENABLED)
    {
        SendGMTicketResult(SMSG_GMTICKET_DELETETICKET, GMTICKET_RESPONSE_NOT_EXIST);
        return;
    }

    if (sTicketMgr.Abandon(sTicketMgr.GetTicketByPlayer(GetPlayer()->GetObjectGuid())) == GMTicketMgr::COMMAND_RESULT_SUCCESS)
        SendGMTicketResult(SMSG_GMTICKET_DELETETICKET, GMTICKET_RESPONSE_TICKET_DELETED);
    else
        SendGMTicketResult(SMSG_GMTICKET_DELETETICKET, GMTICKET_RESPONSE_NOT_EXIST);
}

void WorldSession::HandleGMSurveySubmitOpcode(WorldPacket& recv_data)
{
    if (sTicketMgr.GetSystemStatus() != GMTICKET_QUEUE_STATUS_ENABLED || !m_ticketSquelchTimer.Passed())
    {
        recv_data.read_skip(recv_data.size());
        return;
    }

    m_ticketSquelchTimer.Reset(5000);

    // GM survey is shown after SMSG_GM_TICKET_STATUS_UPDATE with status 3

    uint32 surveyId;
    recv_data >> surveyId;                                  // Survey ID: found in GMSurveySurveys.dbc

    // Amount of question-answer pairs has hardcoded a limit of 10
    std::map<uint32, uint8> answers;

    for (uint8 i = 0; i < MAX_GMSURVEY_QUESTIONS; ++i)
    {
        uint32 questionId;
        recv_data >> questionId;                            // Question ID: questions found in GMSurveyQuestions.dbc

        // Detect end of sequence
        if (!questionId)
            break;

        uint8 answer;
        recv_data >> answer;                                // Rating: hardcoded limit of 0-5 in pre-Wrath, ranges defined in GMSurveyAnswers.dbc Wrath+

        answers.insert({questionId, answer});

        std::string unused;
        recv_data >> unused;                                // Answer comment: Unused in stock UI, can be only set by calling Lua function

        // utf8limit(unused, 11458);                        // Answer comment max sizes in bytes: Vanilla - 8106:8110, TBC - 11459:11463, Wrath - 582:586
    }

    std::string comment;
    recv_data >> comment;                                   // Comment

    if (!CheckChatMessage(comment))
        return;

    utf8limit(comment, 11469);                              // Comment max sizes in bytes: Vanilla - 8117:8121, TBC - 11470:11474, Wrath - 593:597

    // Perform some sanity checks on the input:

    GMSurveyCurrentSurveyEntry const* currentSurvey = sGMSurveyCurrentSurveyStore.LookupEntry(uint32(m_sessionDbcLocale));

    // Check if survey id is correct
    if (!currentSurvey || currentSurvey->surveyID != surveyId)
        return;

    GMSurveyEntry const* survey = sGMSurveySurveysStore.LookupEntry(surveyId);

    // Check if survey entry with provided id exists
    if (!survey)
        return;

    // Check if all provided question ids match the survey entry
    {
        for (auto itr = answers.begin(); itr != answers.end(); ++itr)
        {
            bool exists = false;

            for (uint8 j = 0; (j < MAX_GMSURVEY_QUESTIONS && survey->questionID[j] && !exists); ++j)
                exists = (survey->questionID[j] == itr->first);

            if (!exists)
                return;

            // Pre-WotLK: answers have hardcoded range of 0-5
            if (itr->second > 5)
                return;
        }
    }

    // We only store closed tickets in memory if they are surveyed, so we lookup one:
    if (GMTicket* surveyed = sTicketMgr.GetTicketByPlayer(GetPlayer()->GetObjectGuid(), GMTICKET_STATE_CLOSED))
    {
        GMSurveyResult* result = new GMSurveyResult(*survey, surveyed->GetId(), answers, comment);
        sTicketMgr.Survey(surveyed, result);
        delete result;
    }
}
