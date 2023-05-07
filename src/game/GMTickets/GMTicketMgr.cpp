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
#include "Database/DatabaseEnv.h"
#include "Server/SQLStorages.h"
#include "GMTickets/GMTicketMgr.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Util/ProgressBar.h"
#include "Policies/Singleton.h"
#include "Entities/Player.h"
#include "Chat/Chat.h"
#include "Mails/Mail.h"
#include "Tools/Language.h"
#include "World/World.h"

#include <memory>
#include <regex>

template <typename T>
static inline T clamp(T v, T lo, T hi) { return std::min(std::max(v, lo), hi); }

bool GMTicket::Compare(GMTicket* a, GMTicket* b)
{
    // 1) Open over closed (abandoned are stored in memory for recycling and should never appear in listings)
    if (a->IsOpen() && !b->IsOpen())
        return true;

    // 2) Escalated over normal: should be picked up by another GM asap
    if (a->IsEscalated() && !b->IsEscalated())
        return true;

    // 3) Read over unread: should be dealt with asap
    if (a->IsSeen() && !b->IsSeen())
        return true;

    // Final: FIFO - older tickets over newer ones
    return (a->GetCreatedAt() < b->GetCreatedAt());
}

GMTicket::GMTicket(Player* player, std::string message, uint8 category, time_t when/* = time(nullptr)*/)
{
    WorldSession* session = player->GetSession();

    m_id = sTicketMgr.GenerateTicketId();
    m_category = category;
    m_state = GMTICKET_STATE_OPEN;
    m_status = GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED;
    m_level = (session ? session->GetSecurity() : SEC_PLAYER);
    m_authorGuid = player->GetObjectGuid();
    m_authorName = player->GetName();
    m_locale = localeNames[session ? session->GetSessionDbcLocale() : DEFAULT_LOCALE];
    m_map = player->GetMapId();
    m_x = player->GetPositionX();
    m_y = player->GetPositionY();
    m_z = player->GetPositionZ();
    m_o = player->GetOrientation();
    m_text = message;
    m_created = when;
    m_updated = 0;
    m_seen = 0;
    m_answered = 0;
    m_closed = 0;
    m_assigneeName = "";
    m_assigneeGuid = ObjectGuid();
    m_conclusion = "";
    m_notes = "";

    utf8limit(m_text, 1999);    // Hardcoded client limit: 1999 bytes

    m_statsChatIn = 0;
    m_statsChatOut = 0;
    m_statsEscalations = 0;
    m_statsLastSeen = 0;
}

void GMTicket::Load(Field* fields)
{
    m_id = fields[0].GetUInt32();
    m_category = fields[1].GetUInt8();
    m_state = clamp(GMTicketState(fields[2].GetUInt8()), GMTICKET_STATE_OPEN, GMTICKET_STATE_ABANDONED);
    m_status = clamp(GMTicketEscalationStatus(fields[3].GetUInt8()), GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED, GMTICKET_ASSIGNEDTOGM_STATUS_ESCALATED);
    m_level = fields[4].GetUInt8();
    m_authorGuid = ObjectGuid(HIGHGUID_PLAYER, fields[5].GetUInt32());
    m_authorName = fields[6].GetString();
    m_locale = fields[7].GetString();
    m_map = fields[8].GetUInt32();
    m_x = fields[9].GetFloat();
    m_y = fields[10].GetFloat();
    m_z = fields[11].GetFloat();
    m_o = fields[12].GetFloat();
    m_text = fields[13].GetCppString();
    m_created = time_t(fields[14].GetUInt64());
    m_updated = time_t(fields[15].GetUInt64());
    m_seen = time_t(fields[16].GetUInt64());
    m_answered = time_t(fields[17].GetUInt64());
    m_closed = time_t(fields[18].GetUInt64());
    m_assigneeGuid = ObjectGuid(HIGHGUID_PLAYER, fields[19].GetUInt32());
    m_assigneeName = fields[20].GetString();
    m_conclusion = fields[21].GetCppString();
    m_notes = fields[22].GetCppString();

    utf8limit(m_text, 1999);    // Hardcoded client limit: 1999 bytes

    m_statsChatIn = 0;
    m_statsChatOut = 0;
    m_statsEscalations = IsEscalated();
    m_statsLastSeen = m_seen;
}

void GMTicket::Save(SqlStatement& stmt) const
{
    stmt.addUInt32(m_id);
    stmt.addUInt8(m_category);
    stmt.addUInt8(m_state);
    stmt.addUInt8(m_status);
    stmt.addUInt8(m_level);
    stmt.addUInt32(m_authorGuid.GetCounter());
    stmt.addString(m_authorName);
    stmt.addString(m_locale);
    stmt.addUInt32(m_map);
    stmt.addFloat(m_x);
    stmt.addFloat(m_y);
    stmt.addFloat(m_z);
    stmt.addFloat(m_o);
    stmt.addString(m_text);
    stmt.addUInt64(uint64(m_created));
    stmt.addUInt64(uint64(m_updated));
    stmt.addUInt64(uint64(m_seen));
    stmt.addUInt64(uint64(m_answered));
    stmt.addUInt64(uint64(m_closed));
    stmt.addUInt32(m_assigneeGuid.GetCounter());
    stmt.addString(m_assigneeName);
    stmt.addString(m_conclusion);
    stmt.addString(m_notes);
}

void GMTicketMgr::Save(const GMTicket* ticket)
{
    CharacterDatabase.PExecute("DELETE FROM gm_tickets WHERE id=%u", ticket->GetId());

    static SqlStatementID id;

    SqlStatement stmt = CharacterDatabase.CreateStatement(id,
        "INSERT INTO gm_tickets ("
        "id, category, state, status, level, author_guid, author_name, locale, mapid, x, y, z, o, text, created, updated, seen, answered, closed, assignee_guid, assignee_name, conclusion, notes"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    ticket->Save(stmt);

    stmt.Execute();
}

void GMTicket::Recycle(Player* player, std::string message, uint8 category, time_t when/* = time(nullptr)*/)
{
    WorldSession* session = player->GetSession();

    m_category = category;
    m_status = GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED;
    m_level = (session ? session->GetSecurity() : SEC_PLAYER);
    m_authorGuid = player->GetObjectGuid();
    m_authorName = player->GetName();
    m_locale = localeNames[session ? session->GetSessionDbcLocale() : DEFAULT_LOCALE];
    m_map = player->GetMapId();
    m_x = player->GetPositionX();
    m_y = player->GetPositionY();
    m_z = player->GetPositionZ();
    m_o = player->GetOrientation();
    m_text = message;
    m_created = when;
    m_updated = 0;
    m_seen = 0;
    m_answered = 0;
    m_closed = 0;
    m_assigneeName = "";
    m_assigneeGuid = ObjectGuid();
    m_conclusion = "";
    m_notes = "";

    utf8limit(m_text, 1999);    // Hardcoded client limit: 1999 bytes

    m_statsChatIn = 0;
    m_statsChatOut = 0;
    m_statsEscalations = 0;
    m_statsLastSeen = 0;
}

LocaleConstant GMTicket::GetDbcLocale() const
{
    return sWorld.GetAvailableDbcLocale(GetLocaleByName(m_locale));
}

int GMTicket::GetDbLocaleIndex() const
{
    return sObjectMgr.GetStorageLocaleIndexFor(GetDbcLocale());
}

void GMTicket::SetState(GMTicketState state, time_t when)
{
    if (state == GMTICKET_STATE_CLOSED)
        m_closed = when;
    else if (state == GMTICKET_STATE_ABANDONED)
    {
        // Unload texts from memory:
        m_text = "";
        m_notes = "";
    }

    m_state = state;
}

void GMTicket::SetUpdated(const std::string& text, time_t when)
{
    if (!IsOpen())
        return;

    m_text = text;

    utf8limit(m_text, 1999);    // Hardcoded client limit: 1999 bytes

    m_updated = when;

    if (IsEscalated())          // De-escalate back to normal priority
        m_status = GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED;
}

void GMTicket::SetAssignee(uint8 level, WorldSession* session/* = nullptr*/)
{
    if (!session)          // Unassigning a present assignee: ticket escalation
    {
        if (!IsEscalated())
            ++m_statsEscalations;

        m_status = GMTICKET_ASSIGNEDTOGM_STATUS_ESCALATED;
        m_level = level;
    }
    else if (!IsAssigned())
    {
        m_status = GMTICKET_ASSIGNEDTOGM_STATUS_ASSIGNED;
        m_level = level;
        m_assigneeName = session->GetPlayerName();

        if (Player const* player = session->GetPlayer())
            m_assigneeGuid = player->GetObjectGuid();
        else
            m_assigneeGuid = ObjectGuid();
    }
}

void GMTicket::UpdateStatsChat(bool incoming)
{
    if (incoming)
    {
        if (m_statsChatIn < m_statsChatOut)
            ++m_statsChatIn;
    }
    else
    {
        if (m_statsChatOut == m_statsChatIn)
            ++m_statsChatOut;
    }
}

GMSurveyResult::GMSurveyResult(const GMSurveyEntry &entry, uint32 ticketId, const std::map<uint32, uint8> &answers, const std::string &feedback) :
    m_ticketId(ticketId), m_surveyId(entry.ID), m_comment(feedback)
{
    for (uint8 i = 0; i < MAX_GMSURVEY_QUESTIONS; ++i)
    {
        if (entry.questionID[i])
        {
            auto answer = answers.find(entry.questionID[i]);

            if (answer != answers.end())
                m_answers.push_back(answer->second);
            else
                m_answers.push_back(0);
        }
    }
}

void GMTicketMgr::Save(const GMSurveyResult* survey)
{
    static SqlStatementID id;

    std::string columns = "ticketid, surveyid";
    std::string values = std::to_string(survey->GetTicketId()) + ", " + std::to_string(survey->GetSurveyId());

    for (size_t i = 0; i < survey->GetAnswersCount(); ++i)
    {
        columns += ", answer" + std::to_string(i + 1);
        values += ", " + std::to_string(survey->GetAnswers()[i]);
    }

    if (survey->HasComment())
    {
        columns += ", comment";
        values += ", '" + survey->GetComment() + "'";
    }

    CharacterDatabase.PExecute("DELETE FROM gm_surveys WHERE ticketid=%u", survey->GetTicketId());
    CharacterDatabase.PExecute("INSERT INTO gm_surveys (%s) VALUES(%s)", columns.data(), values.data());
}

INSTANTIATE_SINGLETON_1(GMTicketMgr);

GMTicketMgr::GMTicketMgr()
{
    m_status = GMTicketSystemStatus(sWorld.getConfig(CONFIG_BOOL_GM_TICKETS_QUEUE_STATUS));
}

void GMTicketMgr::LoadGMTickets()
{
    // For reload case: unload all tickets
    RemoveAll();

    // Try to get highest ticket id to start counting from
    std::unique_ptr<QueryResult> max(CharacterDatabase.Query("SELECT MAX(id) FROM gm_tickets"));
    if (max)
        m_lastTicketId = max->Fetch()->GetUInt32();

    // Load open tickets into memory
    std::unique_ptr<QueryResult> result(CharacterDatabase.Query(
        "SELECT "
        "id, category, state, status, level, author_guid, author_name, locale, mapid, x, y, z, o, text, created, updated, seen, answered, closed, assignee_guid, assignee_name, conclusion, notes"
        " FROM gm_tickets WHERE state = 0"
    ));

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 GM tickets");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());
    
    do
    {
        bar.step();

        Field* fields = result->Fetch();

        GMTicket* ticket = new GMTicket(fields);
        Add(ticket, true);
    } while (result->NextRow());

    m_list.sort(GMTicket::Compare);

    sLog.outString(">> Loaded " SIZEFMTD " GM tickets", GetTicketCount());
    sLog.outString();
}

void GMTicketMgr::Print(GMTicket const& ticket, WorldSession* session, time_t now/* = time(nullptr)*/)
{
    const LocaleConstant locale = session->GetSessionDbcLocale();

    ChatHandler ch(session);

    const Player* author = sObjectMgr.GetPlayer(ticket.GetAuthorGuid());
    const bool offline = (!author);

    const double x = double(ticket.GetPositionX()), y = double(ticket.GetPositionY()), z = double(ticket.GetPositionZ());

    ch.SendSysMessage(LANG_TICKETS_PRINT_SPACER);
    ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_SUMMARY, PrintTicketStatus(ticket), uint32(ticket.GetLevel()), ticket.GetId(), PrintTicketCategory(ticket, locale), uint32(ticket.GetCategory()));
    ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_AUTHOR, ticket.GetAuthorName(), ticket.GetAuthorName(), ticket.GetAuthorGuid().GetCounter(), ticket.GetLocale(), (offline ? sObjectMgr.GetMangosString(LANG_OFFLINE, locale) : ""));
    ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_LOCATION, x, y, z, ticket.GetMapId());
    ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_SUBMITTED, TimeToTimestampStr(ticket.GetCreatedAt()).c_str(), secsToTimeString((now - ticket.GetCreatedAt()), true).c_str());
    if (auto updated = ticket.GetUpdatedAt())
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_UPDATED, TimeToTimestampStr(updated).c_str(), secsToTimeString((now - updated), true).c_str());
    if (auto seen = ticket.GetLastSeenAt())
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_SEEN, TimeToTimestampStr(seen).c_str(), secsToTimeString((now - seen), true).c_str());
    if (auto answered = ticket.GetAnsweredAt())
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_ANSWERED, TimeToTimestampStr(answered).c_str(), secsToTimeString((now - answered), true).c_str());
    if (auto closed = ticket.GetClosedAt())
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_CLOSED, TimeToTimestampStr(closed).c_str(), secsToTimeString((now - closed), true).c_str());
    if (!std::string(ticket.GetAssigneeName()).empty())
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_ASSIGNED, ticket.GetAssigneeName(), ticket.GetAssigneeName(), ticket.GetAssigneeGuid().GetCounter());
    ch.SendSysMessage(LANG_TICKET_DETAILS_LINE_TEXT);
    for (auto& line : StrSplit(ticket.GetText(), "\n"))
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_QUOTE, line.c_str());
    const std::string& conclusion = ticket.GetConclusion();
    if (!conclusion.empty())
    {
        ch.SendSysMessage(LANG_TICKET_DETAILS_LINE_CONCLUSION);
        for (auto& line : StrSplit(conclusion, "\n"))
            ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_QUOTE_GM, line.c_str());
    }
    const std::string& notes = ticket.GetNotes();
    if (!notes.empty())
    {
        ch.SendSysMessage(LANG_TICKET_DETAILS_LINE_GM_NOTES);
        for (auto& line : StrSplit(notes, "\n"))
            ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_QUOTE_GM, line.c_str());
    }
    if (ticket.IsOpen())
    {
        const std::string id = std::to_string(ticket.GetId());
        std::string color = (CanWhisper(&ticket, session) == COMMAND_RESULT_SUCCESS ? "FF1AFF1A" : "FFFF1A1A");
        const std::string whisper = "|Hplayer:T#" + id + "whisper|h{|c" + color + sObjectMgr.GetMangosString(LANG_TICKET_DETAILS_ACTION_WHISPER, locale) + "|r}|h";
        color = (CanClose(&ticket, true, session) == COMMAND_RESULT_SUCCESS ? "FF1AFF1A" : "FFFF1A1A");
        const std::string resolve = "|Hplayer:T#" + id + "resolve|h{|c" + color + sObjectMgr.GetMangosString(LANG_TICKET_DETAILS_ACTION_RESOLVE, locale) + "|r}|h";
        color = (CanClose(&ticket, false, session) == COMMAND_RESULT_SUCCESS ? "FF1AFF1A" : "FFFF1A1A");
        const std::string discard = "|Hplayer:T#" + id + "discard|h{|c" + color + sObjectMgr.GetMangosString(LANG_TICKET_DETAILS_ACTION_DISCARD, locale) + "|r}|h";
        color = (CanSort(&ticket, session) == COMMAND_RESULT_SUCCESS ? "FF1AFF1A" : "FFFF1A1A");
        const std::string sort = "|Hplayer:T#" + id + "sort|h{|c" + color + sObjectMgr.GetMangosString(LANG_TICKET_DETAILS_ACTION_SORT, locale) + "|r}|h";
        color = (CanComment(&ticket, session) == COMMAND_RESULT_SUCCESS ? "FF1AFF1A" : "FFFF1A1A");
        const std::string note = "|Hplayer:T#" + id + "note|h{|c" + color + sObjectMgr.GetMangosString(LANG_TICKET_DETAILS_ACTION_NOTE, locale) + "|r}|h";
        ch.PSendSysMessage(LANG_TICKET_DETAILS_LINE_ACTIONS, whisper.c_str(), resolve.c_str(), discard.c_str(), sort.c_str(), note.c_str());
    }
    ch.SendSysMessage(LANG_TICKETS_PRINT_SPACER);
}

const std::string GMTicketMgr::PrintMailResponse(GMTicket const& ticket, bool resolved)
{
    const int32 localeIndex = GetLocaleByName(ticket.GetLocale());
    const MangosStrings textId = (resolved ? LANG_TICKET_CLOSED_LETTER_RESOLVED : LANG_TICKET_CLOSED_LETTER_DISCARDED);

    std::ostringstream ss;
    ss << sObjectMgr.GetMangosString(textId, localeIndex);

    ss << "\n\n\n\n";  // Vanilla spacer

    const std::string& conclusion = ticket.GetConclusion();
    const std::string assignee = std::string(ticket.GetAssigneeName());

    if (!conclusion.empty() && !assignee.empty())
    {
        ss << "\n________________________________\n";
        ss << sObjectMgr.GetMangosString(LANG_TICKET_CLOSED_LETTER_FOOTNOTE, localeIndex);
        ss << "\n|c000000FF[" << assignee.c_str() << "]: " << conclusion.c_str() << "|r";
    }

    return ss.str();
}

const char* GMTicketMgr::PrintTicketCategory(GMTicket const& ticket, LocaleConstant locale/* = LOCALE_DEFAULT*/)
{
    if (GMTicketCategoryEntry const* entry = sGMTicketCategoryStore.LookupEntry(ticket.GetCategory()))
    {
        if (!std::string(entry->name[locale]).empty())
            return entry->name[locale];
        else
            return PrintTicketCategory(ticket, sWorld.GetDefaultDbcLocale());
    }

    return sObjectMgr.GetMangosString(LANG_UNKNOWN, locale);
}

const char* GMTicketMgr::PrintTicketChatAlert(GMTicketMgrChatAlert alert, LocaleConstant locale/* = LOCALE_DEFAULT*/)
{
    MangosStrings textId = MangosStrings(0);

    switch (alert)
    {
        case GMTICKETMGR_CHAT_ONLINE:                           textId = LANG_TICKET_ASSIGNEE_ALERT_ONLINE;     break;
        case GMTICKETMGR_CHAT_OFFLINE:                          textId = LANG_TICKET_ASSIGNEE_ALERT_OFFLINE;    break;
        case GMTICKETMGR_CHAT_UPDATED:                          textId = LANG_TICKET_ASSIGNEE_ALERT_UPDATED;    break;
        case GMTICKETMGR_CHAT_ABANDONED:                        textId = LANG_TICKET_ASSIGNEE_ALERT_ABANDONED;  break;
        case GMTICKETMGR_CHAT_ESCALATED:                        textId = LANG_TICKET_ASSIGNEE_ALERT_ESCALATED;  break;
    }

    return sObjectMgr.GetMangosString(textId, locale);
}

const char* GMTicketMgr::PrintTicketStatus(GMTicket const& ticket, LocaleConstant locale/* = LOCALE_DEFAULT*/)
{
    MangosStrings textId = LANG_UNKNOWN;

    // Condensed: prints only what you need to see about ticket's status
    switch (ticket.GetState())
    {
        case GMTICKET_STATE_CLOSED:                             textId = LANG_TICKET_STATUS_BRIEF_CLOSED;       break;
        case GMTICKET_STATE_ABANDONED:                          textId = LANG_TICKET_STATUS_BRIEF_ABANDONED;    break;
        case GMTICKET_STATE_OPEN:
            switch (ticket.GetStatus())
            {
                case GMTICKET_ASSIGNEDTOGM_STATUS_ASSIGNED:     textId = LANG_TICKET_STATUS_BRIEF_ASSIGNED;     break;
                case GMTICKET_ASSIGNEDTOGM_STATUS_ESCALATED:    textId = LANG_TICKET_STATUS_BRIEF_ESCALATED;    break;
                case GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED:
                    switch (uint8(ticket.IsSeen()))
                    {
                        case false:                             textId = LANG_TICKET_STATUS_BRIEF_UNSEEN;       break;
                        case true:                              textId = LANG_TICKET_STATUS_BRIEF_OPEN;         break;
                    }
            }
    }

    return sObjectMgr.GetMangosString(textId, locale);
}

const char* GMTicketMgr::PrintTicketStatusColorSequence(GMTicket const& ticket)
{
    switch (ticket.GetState())
    {
        case GMTICKET_STATE_CLOSED:                             return "|cFF808080";    // Gray
        case GMTICKET_STATE_ABANDONED:                          return "|cFFC0C0C0";    // Silver
        case GMTICKET_STATE_OPEN:
            switch (ticket.GetStatus())
            {
                case GMTICKET_ASSIGNEDTOGM_STATUS_ASSIGNED:     return "|cFF1AFF1A";    // Green
                case GMTICKET_ASSIGNEDTOGM_STATUS_ESCALATED:    return "|cFFFF1A1A";    // Red
                case GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED:
                    switch (uint8(ticket.IsSeen()))
                    {
                        case true:                              return "|cFFFF00FF";    // Fuchsia
                        case false:                             return "|cFFFFFF00";    // Yellow
                    }
            }
    }
    return "";
}

const std::string GMTicketMgr::PrintTicketSummaryLine(const GMTicket& ticket, LocaleConstant locale/* = LOCALE_DEFAULT*/)
{
    std::string id = std::to_string(ticket.GetId()), name = std::string(ticket.GetAuthorName());

    std::ostringstream ss;
    ss << "  " << std::string(PrintTicketStatusColorSequence(ticket));
    ss << "|Hplayer:T#" << id << "read|hT#" << id                                          << "  ";
    ss << "L" << std::to_string(ticket.GetLevel())                                         << "  ";
    ss << std::string(PrintTicketStatus(ticket, locale))  << "|h|r"                        << "  ";
    ss << "|c00FFFFFF<" << name << ">|r"                                                   << "  ";
    ss << "|c00FFFFFF[" << std::string(ticket.GetLocale()) << "]|r"                        << "  ";
    if (ticket.GetCategory())
        ss << "|c00FFFFFF\"" << std::string(PrintTicketCategory(ticket, locale)) << "\"|r" << "  ";
    if (!sObjectMgr.GetPlayer(ticket.GetAuthorGuid()))
        ss << "|c00FF1A1A" << sObjectMgr.GetMangosString(LANG_OFFLINE, locale) << "|r";
    ss << std::endl;
    return ss.str();
}

void GMTicketMgr::PrintTicketList(WorldSession* session, std::ostringstream& output, size_t max, GMTicketCategoryEntry const* category/* = nullptr*/, bool online/* = false*/) const
{
    size_t count = 0;
    std::ostringstream tickets;

    for (auto itr = m_list.begin(); (itr != m_list.end() && count < max); ++itr)
    {
        if ((*itr)->IsOpen() && (!category || (*itr)->GetCategory() == category->ID))
        {
            if (online && !sObjectMgr.GetPlayer((*itr)->GetAuthorGuid()))
                continue;

            tickets << "|" << PrintTicketSummaryLine(*(*itr), session->GetSessionDbcLocale());
            ++count;
        }
    }

    // No tickets found
    if (!count)
        return;

    output << std::string(session->GetMangosString(LANG_TICKETS_PRINT_SPACER)) << std::endl;
    output << tickets.str();
    output << std::string(session->GetMangosString(LANG_COMMAND_TICKETS_LISTING_FOOTER)) << std::endl;
    output << std::string(session->GetMangosString(LANG_TICKETS_PRINT_SPACER)) << std::endl;
}

bool GMTicketMgr::TicketChatIncoming(GMTicket* ticket, Player* player, WorldSession* gm, const std::string& message, bool addon/* = false*/)
{
    if (!ticket || !player || !player->IsInWorld() || !gm || message.empty())
        return false;

    // Do not send confirmations, afk, dnd or system notifications for addon messages
    if (!addon)
    {
        ticket->UpdateStatsChat(true);

        WorldPacket inform;
        ChatHandler::BuildChatPacket(inform, CHAT_MSG_WHISPER_INFORM, message.c_str(), LANG_UNIVERSAL, CHAT_TAG_GM, ticket->GetAssigneeGuid(), ticket->GetAssigneeName());
        player->GetSession()->SendPacket(inform);

        // Announce afk or dnd message
        if (Player* client = gm->GetPlayer())
        {
            if (client->isAFK() || client->isDND())
            {
                const ChatMsg msgType = (client->isAFK() ? CHAT_MSG_AFK : CHAT_MSG_DND);

                inform.clear();
                ChatHandler::BuildChatPacket(inform, msgType, client->autoReplyMsg.c_str(), LANG_UNIVERSAL, CHAT_TAG_GM, ticket->GetAssigneeGuid(), ticket->GetAssigneeName());
                player->GetSession()->SendPacket(inform);
            }
        }
    }

    const Language lang = (addon ? LANG_ADDON : LANG_UNIVERSAL);
    const ChatTagFlags tag = (player->GetChatTag() & ~uint32(CHAT_TAG_GM));

    std::ostringstream ss;

    if (!addon)
        ss << "|cFFFFFF00[" << ticket->GetIdTag() << "]:|r ";

    ss << message;

    WorldPacket whisper;
    ChatHandler::BuildChatPacket(whisper, CHAT_MSG_WHISPER, ss.str().c_str(), lang, tag, ticket->GetAuthorGuid(), ticket->GetAuthorName());
    gm->SendPacket(whisper);

    return true;
}

bool GMTicketMgr::TicketChatOutgoing(GMTicket* ticket, WorldSession* gm, Player* player, const std::string& message, bool addon/* = false*/)
{
    if (!ticket || !gm || !player || !player->IsInWorld() || message.empty())
        return false;

    // Do not send confirmations, afk, dnd or system notifications for addon messages
    if (!addon)
    {
        ticket->UpdateStatsChat(false);

        std::ostringstream ss;

        ss << "|cFFFFFF00[" << ticket->GetIdTag() << "]:|r ";

        ss << message;

        WorldPacket inform;
        ChatHandler::BuildChatPacket(inform, CHAT_MSG_WHISPER_INFORM, ss.str().c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, ticket->GetAuthorGuid(), ticket->GetAuthorName());
        gm->SendPacket(inform);

        // Announce afk or dnd message
        if (player->isAFK() || player->isDND())
        {
            const ChatMsg msgType = (player->isAFK() ? CHAT_MSG_AFK : CHAT_MSG_DND);

            inform.clear();
            ChatHandler::BuildChatPacket(inform, msgType, player->autoReplyMsg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, ticket->GetAuthorGuid());
            gm->SendPacket(inform);
        }
    }

    Player* client = gm->GetPlayer();

    const Language lang = (addon ? LANG_ADDON : LANG_UNIVERSAL);
    const ChatTagFlags tag = ((client ? client->GetChatTag() : CHAT_TAG_NONE) | CHAT_TAG_GM);

    WorldPacket whisper;
    ChatHandler::BuildChatPacket(whisper, CHAT_MSG_WHISPER, message.c_str(), lang, tag, ticket->GetAssigneeGuid(), ticket->GetAssigneeName());
    player->GetSession()->SendPacket(whisper);

    return true;
}

bool GMTicketMgr::TicketChatAlert(const GMTicket* ticket, GMTicketMgrChatAlert alert)
{
    if (!ticket || !ticket->IsAssigned() || (ticket->IsAssignedTo(ticket->GetAuthorGuid()) && alert == GMTICKETMGR_CHAT_OFFLINE))
        return false;

    Player* gm = sObjectMgr.GetPlayer(ticket->GetAssigneeGuid());

    if (!gm || !gm->isAcceptTickets())
        return false;

    std::ostringstream ss;

    ss << "|cFFFFFF00[" << ticket->GetIdTag() << "]: " << PrintTicketChatAlert(alert, gm->GetSession()->GetSessionDbcLocale()) << "|r";

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, ss.str().c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, ticket->GetAuthorGuid(), ticket->GetAuthorName());
    gm->GetSession()->SendPacket(data);

    return true;
}

void GMTicketMgr::OnPlayerOnlineState(Player &player, bool online) const
{
    if (GMTicket* ticket = GetTicketByPlayer(player.GetObjectGuid()))
        TicketChatAlert(ticket, (online ? GMTICKETMGR_CHAT_ONLINE : GMTICKETMGR_CHAT_OFFLINE));

    if (!online)
    {
        // Foolproofing: just in case if we have old surveyed ticket, cancel survey by unloading it from memory
        if (GMTicket* surveyed = sTicketMgr.GetTicketByPlayer(player.GetObjectGuid(), GMTICKET_STATE_CLOSED))
            sTicketMgr.Survey(surveyed);
    }
    else if (WorldSession* session = player.GetSession())
    {
        if (session->GetSecurity() >= SEC_GAMEMASTER)
            sTicketMgr.ShowMOTD(player);
    }
}

void GMTicketMgr::ShowMOTD(Player &gm) const
{
    const bool notifications = gm.isAcceptTickets();

    if (WorldSession* session = gm.GetSession())
    {
        std::string status = (session->GetMangosString((notifications ? LANG_ON : LANG_OFF)));
        strToUpper(status);
        session->SendNotification("|cFFFFFF00%s: %s|r", session->GetMangosString(LANG_TICKETS_NOTIFICATIONS), status.c_str());
    }

    if (!notifications)
        return;

    ChatHandler ch(&gm);

    ch.PSendSysMessage(LANG_TICKET_MOTD_SYSTEM_STATUS, ch.GetMangosString(GetSystemStatus() ? LANG_ON : LANG_OFF));

    if (m_currentTicketCountClosed)
        ch.PSendSysMessage(LANG_TICKET_MOTD_SYSTEM_STATS_CLOSED, m_currentTicketCountClosed, secsToTimeString(uint32(GetAverageResolutionDays() * DAY)).c_str());

    if (m_currentTicketCountOpen)
    {
        const ObjectGuid& guid = gm.GetObjectGuid();

        uint32 assignedCount = 0, escalatedCount = 0;
        std::deque<GMTicket*> assignedOnline;

        for (auto itr = m_list.begin(); itr != m_list.end(); ++itr)
        {
            if ((*itr)->IsOpen())
            {
                if ((*itr)->IsEscalated())
                    ++escalatedCount;

                if ((*itr)->GetAssigneeGuid() == guid)
                {
                    ++assignedCount;

                    if (const Player* player = sObjectMgr.GetPlayer((*itr)->GetAuthorGuid()))
                        assignedOnline.push_back((*itr));
                }
            }
        }

        ch.PSendSysMessage(LANG_TICKET_MOTD_SYSTEM_STATS_OPEN, m_currentTicketCountOpen, escalatedCount);

        if (assignedCount)
        {
            ch.PSendSysMessage(LANG_TICKET_MOTD_SYSTEM_ASSIGNED, assignedCount, uint32(assignedOnline.size()));

            // Show notifications for each player awaiting gm's reponse right now:
            for (auto itr = assignedOnline.begin(); itr != assignedOnline.end(); ++itr)
            {
                if ((*itr)->GetAuthorGuid() != gm.GetObjectGuid())
                    TicketChatAlert((*itr), GMTICKETMGR_CHAT_ONLINE);
            }
        }
    }
}

std::pair<bool, bool> GMTicketMgr::HookGMTicketWhisper(Player* sender, const std::string& recepient, Player* character, const std::string& msg, bool addon)
{
    // GM ticket chat: outgoing hook
    if (sender->isAcceptTickets())
    {
        GMTicket* ticket = nullptr;

        if (character)
            ticket = GetTicketByPlayer(character->GetObjectGuid(), GMTICKET_STATE_OPEN);
        else
        {
            auto const& result = GetTicketByIdTag(recepient, GMTICKET_STATE_OPEN);
            ticket = result.first;

            // Parse quick actions:
            if (ticket && !result.second.empty())
            {
                std::smatch sm;

                for (auto const& action : { "read", "whisper", "resolve", "discard", "sort", "note" })
                {
                    std::regex re(action, std::regex::icase);

                    if (std::regex_search(result.second, sm, re))
                    {
                        ChatHandler(sender).ParseCommands((".ticket " + std::string(action) + " " + std::to_string(ticket->GetId()) + " " + msg).c_str());
                        return {true, true};
                    }
                }
            }

            character = (ticket ? sObjectMgr.GetPlayer(ticket->GetAuthorName()) : nullptr);
        }

        if (ticket && CanWhisper(ticket, sender->GetSession()) == COMMAND_RESULT_SUCCESS)
        {
            if (!TicketChatOutgoing(ticket, sender->GetSession(), character, msg, addon))
                return {true, false};
            return {true, true};
        }
    }

    // GM ticket chat: incoming hook
    if (character && character->isAcceptTickets())
    {
        if (GMTicket* ticket = GetTicketByPlayer(sender->GetObjectGuid(), GMTICKET_STATE_OPEN, character->GetObjectGuid()))
        {
            if (!TicketChatIncoming(ticket, sender, character->GetSession(), msg, addon))
                return {true, false};
            return {true, true};
        }
    }

    return {false, false};
}

bool GMTicketMgr::HookGMTicketWhoQuery(const std::string& query, Player* gm) const
{
    if (gm && gm->isAcceptTickets())
    {
        if (WorldSession* session = gm->GetSession())
        {
            auto result = GetTicketByIdTag(query, GMTICKET_STATE_OPEN);

            if (GMTicket* ticket = result.first)
            {
                WorldPacket hooked(SMSG_WHO, (4 + 4));
                hooked << uint32(0);
                hooked << uint32(0);
                session->SendPacket(hooked);

                Print(*ticket, session);

                return true;
            }
        }
    }
    return false;
}

GMTicket* GMTicketMgr::GetTicketById(uint32 id, GMTicketState state/* = GMTICKET_STATE_OPEN*/) const
{
    auto itr = m_tickets.find(id);
    if (itr != m_tickets.end() && itr->second->GetState() == state)
        return itr->second;

    return nullptr;
}

GMTicket* GMTicketMgr::GetTicketByPlayer(ObjectGuid playerGuid, GMTicketState state/* = GMTICKET_STATE_OPEN*/, ObjectGuid assigneeGuid/* = ObjectGuid()*/) const
{
    if (playerGuid.IsPlayer())
    {
        GMTicket* ticket;

        for (auto itr = m_tickets.begin(); itr != m_tickets.end(); ++itr)
        {
            ticket = itr->second;
            if (ticket && ticket->GetState() == state && ticket->GetAuthorGuid() == playerGuid && (!assigneeGuid.IsPlayer() || (ticket->IsAssigned() && ticket->IsAssignedTo(assigneeGuid))))
                return ticket;
        }
    }

    return nullptr;
}

std::pair<GMTicket*, std::string> GMTicketMgr::GetTicketByIdTag(const std::string& tag, GMTicketState state/* = GMTICKET_STATE_OPEN*/, ObjectGuid assigneeGuid/* = ObjectGuid()*/) const
{
    uint32 ticketid = 0;
    std::string command;

    const std::regex regex("^[tT]#(\\d+)(\\w*)$");
    std::smatch smatch;

    if (std::regex_match(tag, smatch, regex))
    {
        ticketid = uint32(strtoul(smatch[1].str().c_str(), nullptr, 10));

        if (smatch.size() > 2)
            command = smatch[2].str();

        if (GMTicket* ticket = GetTicketById(ticketid))
        {
            if (ticket->GetState() == state && (!assigneeGuid.IsPlayer() || (ticket->IsAssigned() && ticket->IsAssignedTo(assigneeGuid))))
                return {ticket, command};
        }
    }
    return {nullptr, command};
}

bool GMTicketMgr::Add(GMTicket* ticket, bool loading/* = false*/)
{
    if (!ticket || !ticket->GetId())
        return false;

    // Add while checking if already added first
    if (!m_tickets.insert({ticket->GetId(), ticket}).second)
        return true;

    m_list.push_back(ticket);

    if (ticket->IsOpen())
        ++m_currentTicketCountOpen;

    if (!loading)
    {
        Save(ticket);

        sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_NEW, ticket->GetIdTag().c_str(), ticket->GetAuthorName());
    }

    return true;
}

bool GMTicketMgr::Recycle(GMTicket* ticket)
{
    if (!ticket || ticket->GetState() != GMTICKET_STATE_ABANDONED)
        return false;

    if (m_tickets.find(ticket->GetId()) == m_tickets.end())
        return false;

    ++m_currentTicketCountOpen;

    ticket->SetState(GMTICKET_STATE_OPEN);

    Save(ticket);

    m_list.sort(GMTicket::Compare);

    sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_NEW, ticket->GetIdTag().c_str(), ticket->GetAuthorName());

    return true;
}

void GMTicketMgr::Survey(GMTicket* ticket, const GMSurveyResult* result/* = nullptr*/)
{
    if (!ticket || ticket->IsOpen() || !ticket->IsSurveyed())
        return;

    if (result)
    {
        sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_SURVEY, ticket->GetIdTag().c_str());
        Save(result);
    }

    // Unload closed ticket from memory
    Remove(*ticket);
}

GMTicketMgr::CommandResult GMTicketMgr::Abandon(GMTicket* ticket)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;

    TicketChatAlert(ticket, GMTICKETMGR_CHAT_ABANDONED);

    --m_currentTicketCountOpen;

    ticket->SetState(GMTICKET_STATE_ABANDONED);

    Save(ticket);

    sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_ABANDONED, ticket->GetIdTag().c_str());

    // Do not unload abandoned ticket from memory: leave for reuse
    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Escalate(GMTicket* ticket, WorldSession* session, uint8 level)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // Cannot escalate an unread ticket
    else if (!ticket->IsSeen())
        return COMMAND_RESULT_TICKET_UNSEEN;
    // Cannot escalate a ticket if ticket's access level is significantly higher
    else if (!session || uint8(session->GetSecurity()) < ticket->GetLevel())
        return COMMAND_RESULT_ACCESS_DENIED;
    // Cannot force-escalate an assigned ticket if assignee is currently online and you have same or lower access level
    else if (ticket->IsAssigned() && !ticket->IsAssignedTo(session->GetPlayerName()))
    {
        if (uint8(session->GetSecurity()) == ticket->GetLevel() && sObjectMgr.GetPlayer(ticket->GetAssigneeGuid()))
            return COMMAND_RESULT_TICKET_NOT_ASSIGNED;
    }

    // Cannot escalate to a lower level or above administrator level
    level = clamp(level, ticket->GetLevel(), uint8(SEC_ADMINISTRATOR));

    // If already escalated to that level: fail silently
    if (ticket->IsEscalated() && level == ticket->GetLevel())
        return COMMAND_RESULT_SUCCESS;
    // Cannot escalate to a level above own
    else if (level > session->GetSecurity())
        return COMMAND_RESULT_ACCESS_DENIED;

    // Check if we are actually escalating it and not just updating the level of arleady escalated ticket
    const bool initial = !ticket->IsEscalated();

    if (initial)
        TicketChatAlert(ticket, GMTICKETMGR_CHAT_ESCALATED);

    ticket->SetAssignee(level);

    Save(ticket);

    if (initial)
    {
        m_list.sort(GMTicket::Compare);

        if (Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid()))
            character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
    }

    sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_ESCALATION, ticket->GetIdTag().c_str(), uint32(ticket->GetLevel()), session->GetPlayerName());

    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Comment(GMTicket* ticket, WorldSession* session, const std::string& text)
{
    CommandResult result = CanComment(ticket, session);

    if (result != COMMAND_RESULT_SUCCESS)
        return result;

    time_t now = time(nullptr);

    for (auto& line : StrSplit(text, "\n"))
    {
        std::ostringstream output;
        output << TimeToTimestampStr(now) << " [" << session->GetPlayerName() << "]: " << line;
        ticket->AddNote(output.str());
    }

    Save(ticket);

    sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_COMMENTARY, ticket->GetIdTag().c_str(), session->GetPlayerName());

    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Read(GMTicket* ticket)
{
    if (!ticket)
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // If reading a closed ticket: no actions needed, fail silently
    else if (!ticket->IsOpen())
        return COMMAND_RESULT_SUCCESS;

    const bool unread = !ticket->IsSeen();

    ticket->SetSeen(time(nullptr));

    Save(ticket);

    if (unread)
    {
        m_list.sort(GMTicket::Compare);

        if (Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid()))
            character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
    }

    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Sort(GMTicket* ticket, const GMTicketCategoryEntry& category, WorldSession* session/* = nullptr*/)
{
    CommandResult result = CanSort(ticket, session);

    if (result != COMMAND_RESULT_SUCCESS)
        return result;

    if (ticket->GetCategory() != category.ID)
    {
        ticket->SetCategory(uint8(category.ID));

        Save(ticket);

        sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_CATEGORY, ticket->GetIdTag().c_str(), PrintTicketCategory(*ticket, sWorld.GetDefaultDbcLocale()), category.ID, session->GetPlayerName());

        if (Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid()))
            character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
    }

    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Update(GMTicket* ticket, const std::string& text)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;

    if (ticket->GetText() == text)
        return COMMAND_RESULT_SUCCESS;

    TicketChatAlert(ticket, GMTICKETMGR_CHAT_UPDATED);

    const bool deescalation = ticket->IsEscalated();

    ticket->SetUpdated(text);

    Save(ticket);

    m_list.sort(GMTicket::Compare);

    if (deescalation)
         sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_DEESCALATION, ticket->GetIdTag().c_str());

    if (Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid()))
        character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);

    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::Whisper(GMTicket *ticket, const std::string& message, WorldSession* session/* = nullptr*/)
{
    CommandResult result = CanWhisper(ticket, session);

    if (result != COMMAND_RESULT_SUCCESS)
        return result;

    Player* gm = session->GetPlayer();
    ObjectGuid guid = (gm ? gm->GetObjectGuid() : ObjectGuid());

    const bool assignment = !ticket->IsAssigned();

    Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid());

    if (guid)
    {
        ticket->SetAssignee(session->GetSecurity(), session);
        ticket->SetAnswered(time(nullptr));

        Save(ticket);

        if (assignment)
        {
            sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_ASSIGNMENT, ticket->GetIdTag().c_str(), ticket->GetAssigneeName());

            if (character)
                character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
        }

        TicketChatOutgoing(ticket, session, character, message);
    }

    return (character ? COMMAND_RESULT_SUCCESS : COMMAND_RESULT_PLAYER_OFFLINE);
}

bool GMTicketMgr::SetSystemStatus(bool status)
{
    if (bool(m_status) == status)
        return false;

    m_status = GMTicketSystemStatus(status);

    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32(status);
    sWorld.SendGlobalMessage(data);

    // Do loop over open tickets, poke online clients to initiate ticket update
    for (auto itr = m_tickets.begin(); itr != m_tickets.end(); ++itr)
    {
        GMTicket* t = itr->second;

        if (t && t->IsOpen())
        {
            if (Player* character = sObjectMgr.GetPlayer(t->GetAuthorGuid()))
                character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
        }
    }

    return true;
}

GMTicketMgr::CommandResult GMTicketMgr::CanComment(const GMTicket* ticket, WorldSession* who)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // Ticket neeeds to be read at least once
    else if (!ticket->IsSeen())
        return COMMAND_RESULT_TICKET_UNSEEN;
    else if (!who)
        return COMMAND_RESULT_ACCESS_DENIED;
    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::CanClose(const GMTicket* ticket, bool resolve, WorldSession* who)
{
    const bool self = (resolve && ticket && (strcmp(who->GetPlayerName(), ticket->GetAuthorName()) == 0));

    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // Cannot close an unread ticket, unless own
    else if (!self && !ticket->IsSeen())
        return COMMAND_RESULT_TICKET_UNSEEN;
    // Cannot close a ticket if ticket's access level is significantly higher, unless own
    else if (!who || (!self && uint8(who->GetSecurity()) < ticket->GetLevel()))
        return COMMAND_RESULT_ACCESS_DENIED;
    // Cannot resolve already assigned ticket unless its own ticket
    else if (resolve && !self && (!ticket->IsAssigned() || !ticket->IsAssignedTo(who->GetPlayerName())))
        return COMMAND_RESULT_TICKET_NOT_ASSIGNED;
    // Cannot discard already assigned ticket
    else if (!resolve && ticket->IsAssigned() && !ticket->IsAssignedTo(who->GetPlayerName()))
        return COMMAND_RESULT_TICKET_NOT_ASSIGNED;
    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::CanSort(const GMTicket *ticket, WorldSession *who)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // Ticket neeeds to be read at least once
    else if (!ticket->IsSeen())
        return COMMAND_RESULT_TICKET_UNSEEN;
    // Cannot change ticket's category if ticket's access level is significantly higher
    else if (!who || uint8(who->GetSecurity()) < ticket->GetLevel())
        return COMMAND_RESULT_ACCESS_DENIED;
    // Cannot change an assigned ticket's category
    else if (ticket->IsAssigned() && !ticket->IsAssignedTo(who->GetPlayerName()))
        return COMMAND_RESULT_TICKET_NOT_ASSIGNED;
    return COMMAND_RESULT_SUCCESS;
}

GMTicketMgr::CommandResult GMTicketMgr::CanWhisper(const GMTicket* ticket, WorldSession* who)
{
    if (!ticket || !ticket->IsOpen())
        return COMMAND_RESULT_TICKET_NOT_FOUND;
    // Ticket neeeds to be read at least once
    else if (!ticket->IsSeen())
        return COMMAND_RESULT_TICKET_UNSEEN;
    // Cannot answer a ticket if ticket's access level is significantly higher
    else if (!who || uint8(who->GetSecurity()) < ticket->GetLevel())
        return COMMAND_RESULT_ACCESS_DENIED;
    // Cannot answer already assigned ticket
    else if (ticket->IsAssigned() && !ticket->IsAssignedTo(who->GetPlayerName()))
        return COMMAND_RESULT_TICKET_NOT_ASSIGNED;
    return COMMAND_RESULT_SUCCESS;
}

bool GMTicketMgr::Remove(GMTicket& ticket)
{
    m_list.remove(&ticket);

    auto itr = m_tickets.find(ticket.GetId());

    if (itr != m_tickets.end())
    {
        m_tickets.erase(itr);

        if (ticket.IsOpen())
            --m_currentTicketCountOpen;

        delete &ticket;

        return true;
    }
    return false;
}

void GMTicketMgr::RemoveAll()
{
    for (auto itr = m_tickets.begin(); itr != m_tickets.end(); ++itr)
        delete itr->second;

    m_tickets.clear();
    m_list.clear();

    m_currentTicketCountOpen = 0;
}

GMTicketMgr::CommandResult GMTicketMgr::Close(GMTicket* ticket, const std::string& conclusion, bool resolved, WorldSession* session/* = nullptr*/)
{
    CommandResult result = CanClose(ticket, resolved, session);

    if (result != COMMAND_RESULT_SUCCESS)
        return result;

    const bool self = (resolved && (strcmp(session->GetPlayerName(), ticket->GetAuthorName()) == 0));

    // Foolproofing: just in case if we have old surveyed ticket, cancel survey by unloading it from memory
    if (resolved && ticket->IsSurveyed())
    {
        if (GMTicket* surveyed = sTicketMgr.GetTicketByPlayer(ticket->GetAuthorGuid(), GMTICKET_STATE_CLOSED))
            sTicketMgr.Survey(surveyed);
    }

    --m_currentTicketCountOpen;
    ++m_currentTicketCountClosed;

    UpdateTicketQueueStats(*ticket, time(nullptr));

    ticket->SetState(GMTICKET_STATE_CLOSED);

    if (!self)
        ticket->SetAssignee(session->GetSecurity(), session);

    ticket->SetConclusion(conclusion);

    Save(ticket);

    Player* character = sObjectMgr.GetPlayer(ticket->GetAuthorGuid());

    sWorld.SendWorldTextToAcceptingTickets(LANG_TICKET_BROADCAST_CLOSED, ticket->GetIdTag().c_str(), ticket->GetAssigneeName());

    // Prepare in-game mail response
    {
        std::string title = sObjectMgr.GetMangosString(LANG_TICKET_CLOSED_LETTER_TITLE, GetLocaleByName(ticket->GetLocale()));
        const std::string text = PrintMailResponse(*ticket, resolved);

        MailDraft draft(title, text);
        MailSender sender(MAIL_NORMAL, ticket->GetAssigneeGuid().GetCounter(), MAIL_STATIONERY_GM);
        MailReceiver receiver(character, ticket->GetAuthorGuid());
        draft.SendMailTo(receiver, sender);
    }

    if (character)
    {
        if (resolved && ticket->IsSurveyed())
        {
            character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_SURVEY);
            return COMMAND_RESULT_SUCCESS;  // Delay unloading surveyed ticket from memory until survey is complete
        }
        else
            character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_CLOSED);
    }

    Remove(*ticket);                        // Unload closed ticket from memory
    return COMMAND_RESULT_SUCCESS;
}

float GMTicketMgr::GetOldestTicketAgeLastUpdateDays(time_t now) const
{
    return (m_statsOldestTicketAgeLastUpdate ? GetDaysPassed(now, m_statsOldestTicketAgeLastUpdate) : -1);
}

void GMTicketMgr::UpdateTicketQueueStats(GMTicket& closed, time_t when)
{
    MANGOS_ASSERT(m_currentTicketCountClosed)

    m_statsCurrentTotalResolutionDays += GetDaysPassed(when, closed.GetCreatedAt());
    m_statsAverageResolutionDays = (m_statsCurrentTotalResolutionDays / m_currentTicketCountClosed);

    // Order of the list is the FIFO queue's order, declared in GMTicket::Compare
    // Do loop over queued tickets, record first (oldest) new non-escalated queued ticket's time since creation
    for (auto itr = m_list.begin(); itr != m_list.end(); ++itr)
    {
        if ((*itr) && (*itr)->IsOpen() && !(*itr)->IsEscalated() && !(*itr)->IsSeen())
        {
            m_statsOldestTicketAgeLastUpdate = when;
            m_statsOldestTicketAgeDays = GetTicketAgeDays(when, (*itr)->GetCreatedAt());
            UpdateTicketQueueTimers(&closed);
            break;
        }
    }
}

void GMTicketMgr::UpdateTicketQueueTimers(GMTicket* except/* = nullptr*/) const
{
    // Do loop over open tickets, poke online clients to initiate ticket waiting time update
    for (auto itr = m_tickets.begin(); itr != m_tickets.end(); ++itr)
    {
        GMTicket* t = itr->second;

        if (t && t != except && t->IsOpen() && !t->IsSeen() && !t->IsEscalated())
        {
            if (Player* character = sObjectMgr.GetPlayer(t->GetAuthorGuid()))
                character->GetSession()->SendGMTicketResult(SMSG_GM_TICKET_STATUS_UPDATE, GMTICKET_STATUS_UPDATED);
        }
    }
}
