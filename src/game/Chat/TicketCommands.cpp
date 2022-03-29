/*
 *
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
#include "Chat.h"
#include "Globals/ObjectMgr.h"
#include "GMTickets/GMTicketMgr.h"
#include "Mails/Mail.h"
#include "Tools/Language.h"

inline static bool HandleTicketMgrCommandResult(ChatHandler &ch, uint32 ticketId, GMTicketMgr::CommandResult result)
{
    MangosStrings error;

    switch (result)
    {
        case GMTicketMgr::COMMAND_RESULT_TICKET_NOT_FOUND:      error = LANG_COMMAND_TICKET_NOT_FOUND;      break;
        case GMTicketMgr::COMMAND_RESULT_PLAYER_OFFLINE:        error = LANG_COMMAND_TICKET_PLAYER_OFFLINE; break;
        case GMTicketMgr::COMMAND_RESULT_TICKET_UNSEEN:         error = LANG_COMMAND_TICKET_UNSEEN;         break;
        case GMTicketMgr::COMMAND_RESULT_ACCESS_DENIED:         error = LANG_COMMAND_TICKET_ACCESS_DENIED;  break;
        case GMTicketMgr::COMMAND_RESULT_TICKET_NOT_ASSIGNED:   error = LANG_COMMAND_TICKET_NOT_ASSIGNED;   break;
        case GMTicketMgr::COMMAND_RESULT_SUCCESS:               return true;
    }

    ch.PSendSysMessage(error, GMTicket::GetIdTag(ticketId).c_str());
    return false;
}

bool ChatHandler::HandleTicketDiscardCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    std::string conclusion = args;
    if (!conclusion.empty())
    {
        const std::string find = "\\n";
        const std::string replace = "\n";

        for (std::string::size_type i = 0; (i = conclusion.find(find, i)) != std::string::npos;)
        {
            conclusion.replace(i, find.length(), replace);
            i += replace.length();
        }
    }

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Discard(sTicketMgr.GetTicketById(ticketId), conclusion, m_session)))
    {
        SetSentErrorMessage(true);
        return false;
    }
    return true;
}

bool ChatHandler::HandleTicketEscalateCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    uint32 level;
    ExtractOptUInt32(&args, level, SEC_PLAYER);

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Escalate(sTicketMgr.GetTicketById(ticketId), m_session, uint8(level))))
    {
        SetSentErrorMessage(true);
        return false;
    }
    return true;
}

bool ChatHandler::HandleTicketGoCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    GMTicket* ticket = sTicketMgr.GetTicketById(ticketId);
    if (!ticket)
    {
        PSendSysMessage(LANG_COMMAND_TICKET_NOT_FOUND, GMTicket::GetIdTag(ticketId).c_str());
        SetSentErrorMessage(true);
        return false;
    }

    float z = ticket->GetPositionZ();
    float o = ticket->GetPositionO();
    return HandleGoHelper(m_session->GetPlayer(), ticket->GetMapId(), ticket->GetPositionX(), ticket->GetPositionY(), &z, &o);
}

bool ChatHandler::HandleTicketGoNameCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    GMTicket* ticket = sTicketMgr.GetTicketById(ticketId);
    if (!ticket)
    {
        PSendSysMessage(LANG_COMMAND_TICKET_NOT_FOUND, GMTicket::GetIdTag(ticketId).c_str());
        SetSentErrorMessage(true);
        return false;
    }

    char* name = mangos_strdup(ticket->GetAuthorName());
    const bool result = HandleGonameCommand(name);
    delete[] name;
    return result;
}

bool ChatHandler::HandleTicketNoteCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    std::string text = args;
    if (text.empty())
        return false;

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Comment(sTicketMgr.GetTicketById(ticketId), m_session, text)))
    {
        SetSentErrorMessage(true);
        return false;
    }
    return true;
}

bool ChatHandler::HandleTicketReadCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    GMTicket* ticket = sTicketMgr.GetTicketById(ticketId);

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Read(ticket)))
    {
        SetSentErrorMessage(true);
        return false;
    }

    GMTicketMgr::Print(*ticket, m_session);

    return true;
}

bool ChatHandler::HandleTicketResolveCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    std::string conclusion = args;
    if (!conclusion.empty())
    {
        const std::string find = "\\n";
        const std::string replace = "\n";

        for (std::string::size_type i = 0; (i = conclusion.find(find, i)) != std::string::npos;)
        {
            conclusion.replace(i, find.length(), replace);
            i += replace.length();
        }
    }

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Resolve(sTicketMgr.GetTicketById(ticketId), conclusion, m_session)))
    {
        SetSentErrorMessage(true);
        return false;
    }
    return true;
}

bool ChatHandler::HandleTicketSortCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    uint32 categoryId;
    if (!ExtractUInt32(&args, categoryId))
        return false;

    if (GMTicketCategoryEntry const* category = sGMTicketCategoryStore.LookupEntry(categoryId))
    {
        if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Sort(sTicketMgr.GetTicketById(ticketId), *category, m_session)))
        {
            SetSentErrorMessage(true);
            return false;
        }
        return true;
    }

    PSendSysMessage(LANG_COMMAND_TICKETS_BAD_CATEGORY, categoryId);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleTicketWhisperCommand(char* args)
{
    uint32 ticketId;
    if (!ExtractUInt32(&args, ticketId))
        return false;

    std::string answer = args;
    if (answer.empty())
        return false;

    std::string find = "\\n";
    std::string replace = "\n";

    for (std::string::size_type i = 0; (i = answer.find(find, i)) != std::string::npos;)
    {
        answer.replace(i, find.length(), replace);
        i += replace.length();
    }

    if (!HandleTicketMgrCommandResult(*this, ticketId, sTicketMgr.Whisper(sTicketMgr.GetTicketById(ticketId), answer, m_session)))
    {
        SetSentErrorMessage(true);
        return false;
    }
    return true;
}

bool ChatHandler::HandleTicketCommand(char* args)
{
    return HandleTicketReadCommand(args);
}

bool ChatHandler::HandleTicketsListCommand(char* args)
{
    std::ostringstream output;

    uint32 arg1;
    uint32 max = 10;

    const bool first = ExtractUInt32(&args, arg1);
    const bool last = ExtractUInt32(&args, max);
    const bool online = ExtractLiteralArg(&args, "online");

    GMTicketCategoryEntry const* category = nullptr;

    if (first)
    {
        if (!last)
            max = arg1;
        else
        {
            category = sGMTicketCategoryStore.LookupEntry(arg1);

            if (!category)
            {
                PSendSysMessage(LANG_COMMAND_TICKETS_BAD_CATEGORY, arg1);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }
    sTicketMgr.PrintTicketList(m_session, output, max, category, online);

    std::string string = output.str();
    if (string.empty())
        PSendSysMessage(LANG_COMMAND_TICKETS_NO_TICKETS);
    else
    {
        PSendSysMessage(LANG_COMMAND_TICKETS_LISTING, max);
        PSendSysMessage("%s", string.c_str());
    }

    return true;
}

bool ChatHandler::HandleTicketsQueueCommand(char *args)
{
    bool value;

    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    sTicketMgr.SetSystemStatus(value);

    return true;
}

bool ChatHandler::HandleTicketsCommand(char* args)
{
    // Detect cases when used as an alias for "ticket list"
    {
        if (!args || !*args)
            return HandleTicketsListCommand(args);

        std::string string = args;
        char* input = const_cast<char*>(string.c_str());

        uint32 arg;

        bool listing = ExtractUInt32(&input, arg);

        if (!listing)
        {
            if (char* literal = ExtractLiteralArg(&input))
            {
                std::string online = literal;
                strToLower(online);
                listing = (strcmp(online.c_str(), "online") == 0);
            }
            else
                listing = true;
        }

        const bool result = (listing ? HandleTicketsListCommand(args) : false);

        if (listing)
            return result;
    }

    bool value;

    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();

    player->SetAcceptTicket(value);
    sTicketMgr.ShowMOTD(*player);

    return true;
}
