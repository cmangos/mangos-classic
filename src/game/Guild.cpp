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

#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "Chat.h"
#include "SocialMgr.h"
#include "Util.h"
#include "Language.h"
#include "World.h"

Guild::Guild()
{
    m_Id = 0;
    m_Name = "";
    m_LeaderGuid = 0;
    GINFO = MOTD = "";
    m_EmblemStyle = 0;
    m_EmblemColor = 0;
    m_BorderStyle = 0;
    m_BorderColor = 0;
    m_BackgroundColor = 0;

    m_CreatedYear = 0;
    m_CreatedMonth = 0;
    m_CreatedDay = 0;

    m_EventLogLoaded = false;
    m_OnlineMembers = 0;

    m_GuildEventLogNextGuid = 0;
}

Guild::~Guild()
{

}

bool Guild::Create(Player* leader, std::string gname)
{
    if (sObjectMgr.GetGuildByName(gname))
        return false;

    WorldSession* lSession = leader->GetSession();
    if (!lSession)
        return false;

    m_LeaderGuid = leader->GetGUID();
    m_Name = gname;
    GINFO = "";
    MOTD = "No message set.";
    m_Id = sObjectMgr.GenerateGuildId();

    sLog.outDebug("GUILD: creating guild %s to leader: %u", gname.c_str(), GUID_LOPART(m_LeaderGuid));

    // gname already assigned to Guild::name, use it to encode string for DB
    CharacterDatabase.escape_string(gname);

    std::string dbGINFO = GINFO;
    std::string dbMOTD = MOTD;
    CharacterDatabase.escape_string(dbGINFO);
    CharacterDatabase.escape_string(dbMOTD);

    CharacterDatabase.BeginTransaction();
    // CharacterDatabase.PExecute("DELETE FROM guild WHERE guildid='%u'", Id); - MAX(guildid)+1 not exist
    CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guildid='%u'", m_Id);
    CharacterDatabase.PExecute("INSERT INTO guild (guildid,name,leaderguid,info,motd,createdate,EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor) "
        "VALUES('%u','%s','%u', '%s', '%s', UNIX_TIMESTAMP(NOW()),'%u','%u','%u','%u','%u')",
        m_Id, gname.c_str(), GUID_LOPART(m_LeaderGuid), dbGINFO.c_str(), dbMOTD.c_str(), m_EmblemStyle, m_EmblemColor, m_BorderStyle, m_BorderColor, m_BackgroundColor);
    CharacterDatabase.CommitTransaction();

    CreateDefaultGuildRanks(lSession->GetSessionDbLocaleIndex());

    return AddMember(m_LeaderGuid, (uint32)GR_GUILDMASTER);
}

void Guild::CreateDefaultGuildRanks(int locale_idx)
{
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid='%u'", m_Id);

    CreateRank(sObjectMgr.GetMangosString(LANG_GUILD_MASTER, locale_idx),   GR_RIGHT_ALL);
    CreateRank(sObjectMgr.GetMangosString(LANG_GUILD_OFFICER, locale_idx),  GR_RIGHT_ALL);
    CreateRank(sObjectMgr.GetMangosString(LANG_GUILD_VETERAN, locale_idx),  GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    CreateRank(sObjectMgr.GetMangosString(LANG_GUILD_MEMBER, locale_idx),   GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    CreateRank(sObjectMgr.GetMangosString(LANG_GUILD_INITIATE, locale_idx), GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
}

bool Guild::AddMember(uint64 plGuid, uint32 plRank)
{
    Player* pl = sObjectMgr.GetPlayer(plGuid);
    if (pl)
    {
        if (pl->GetGuildId() != 0)
            return false;
    }
    else
    {
        if (Player::GetGuildIdFromDB(plGuid) != 0)          // player already in guild
            return false;
    }

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many guilds and corrupt guild data integrity
    Player::RemovePetitionsAndSigns(plGuid);

    // fill player data
    MemberSlot newmember;

    if (pl)
    {
        newmember.Name   = pl->GetName();
        newmember.Level  = pl->getLevel();
        newmember.Class  = pl->getClass();
        newmember.ZoneId = pl->GetZoneId();
    }
    else
    {
        //                                                     0    1     2     3
        QueryResult *result = CharacterDatabase.PQuery("SELECT name,level,class,zone FROM characters WHERE guid = '%u'", GUID_LOPART(plGuid));
        if (!result)
            return false;                                   // player doesn't exist

        Field *fields    = result->Fetch();
        newmember.Name   = fields[0].GetCppString();
        newmember.Level  = fields[1].GetUInt8();
        newmember.Class  = fields[2].GetUInt8();
        newmember.ZoneId = fields[3].GetUInt32();
        delete result;
        if (newmember.Level < 1 || newmember.Level > STRONG_MAX_LEVEL ||
            newmember.Class < CLASS_WARRIOR || newmember.Class >= MAX_CLASSES)
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `characters` table, cannot add him to guild.",GUID_LOPART(plGuid));
            return false;
        }
    }

    newmember.RankId  = plRank;
    newmember.OFFnote = (std::string)"";
    newmember.Pnote   = (std::string)"";
    newmember.LogoutTime = time(NULL);
    members[GUID_LOPART(plGuid)] = newmember;

    std::string dbPnote   = newmember.Pnote;
    std::string dbOFFnote = newmember.OFFnote;
    CharacterDatabase.escape_string(dbPnote);
    CharacterDatabase.escape_string(dbOFFnote);

    CharacterDatabase.PExecute("INSERT INTO guild_member (guildid,guid,rank,pnote,offnote) VALUES ('%u', '%u', '%u','%s','%s')",
        m_Id, GUID_LOPART(plGuid), newmember.RankId, dbPnote.c_str(), dbOFFnote.c_str());

    // If player not in game data in data field will be loaded from guild tables, no need to update it!!
    if (pl)
    {
        pl->SetInGuild(m_Id);
        pl->SetRank(newmember.RankId);
        pl->SetGuildIdInvited(0);
    }
    return true;
}

void Guild::SetMOTD(std::string motd)
{
    MOTD = motd;

    // motd now can be used for encoding to DB
    CharacterDatabase.escape_string(motd);
    CharacterDatabase.PExecute("UPDATE guild SET motd='%s' WHERE guildid='%u'", motd.c_str(), m_Id);
}

void Guild::SetGINFO(std::string ginfo)
{
    GINFO = ginfo;

    // ginfo now can be used for encoding to DB
    CharacterDatabase.escape_string(ginfo);
    CharacterDatabase.PExecute("UPDATE guild SET info='%s' WHERE guildid='%u'", ginfo.c_str(), m_Id);
}

bool Guild::LoadGuildFromDB(uint32 GuildId)
{
    //set m_Id in case guild data are broken in DB and Guild will be Disbanded (deleted from DB)
    m_Id = GuildId;

    if (!LoadRanksFromDB(GuildId))
        return false;

    if (!LoadMembersFromDB(GuildId))
        return false;

    //                                                     0     1           2            3            4            5
    QueryResult *result = CharacterDatabase.PQuery("SELECT name, leaderguid, EmblemStyle, EmblemColor, BorderStyle, BorderColor,"
    //   6                7     8     9
        "BackgroundColor, info, motd, createdate FROM guild WHERE guildid = '%u'", GuildId);

    if (!result)
        return false;

    Field *fields = result->Fetch();

    m_Name = fields[0].GetCppString();
    m_LeaderGuid  = MAKE_NEW_GUID(fields[1].GetUInt32(), 0, HIGHGUID_PLAYER);

    m_EmblemStyle = fields[2].GetUInt32();
    m_EmblemColor = fields[3].GetUInt32();
    m_BorderStyle = fields[4].GetUInt32();
    m_BorderColor = fields[5].GetUInt32();
    m_BackgroundColor = fields[6].GetUInt32();
    GINFO = fields[7].GetCppString();
    MOTD = fields[8].GetCppString();
    time_t time = fields[9].GetUInt64();

    delete result;

    if (time > 0)
    {
        tm local = *(localtime(&time));                     // dereference and assign
        m_CreatedDay   = local.tm_mday;
        m_CreatedMonth = local.tm_mon + 1;
        m_CreatedYear  = local.tm_year + 1900;
    }

    // Repair the structure of guild
    // If the guildmaster doesn't exist or isn't the member of guild
    // attempt to promote another member
    int32 GM_rights = GetRank(GUID_LOPART(m_LeaderGuid));
    if (GM_rights == -1)
    {
        DelMember(m_LeaderGuid);
        // check no members case (disbanded)
        if (members.empty())
            return false;
    }
    else if (GM_rights != GR_GUILDMASTER)
        SetLeader(m_LeaderGuid);

    // Allow only 1 guildmaster
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->second.RankId == GR_GUILDMASTER && GUID_LOPART(m_LeaderGuid) != itr->first)
            //set right of member to officer
            ChangeRank(itr->first, GR_OFFICER);
    }

    sLog.outDebug("Guild %u Creation time Loaded day: %u, month: %u, year: %u", GuildId, m_CreatedDay, m_CreatedMonth, m_CreatedYear);

    return true;
}

bool Guild::LoadRanksFromDB(uint32 GuildId)
{
    Field *fields;
    //                                                     0   1     2
    QueryResult *result = CharacterDatabase.PQuery("SELECT rid,rname,rights FROM guild_rank WHERE guildid = '%u' ORDER BY rid ASC", GuildId);

    if (!result)
    {
        sLog.outError("Guild %u has broken `guild_rank` data, creating new...",GuildId);
        CreateDefaultGuildRanks(0);
        return true;
    }

    bool broken_ranks = false;

    // GUILD RANKS are sequence starting from 0 = GUILD_MASTER (ALL PRIVILEGES) to max 9 (lowest privileges)
    // the lower rank id is considered higher rank - so promotion does rank-- and demotion does rank++
    // between ranks in sequence cannot be gaps - so 0,1,2,4 cannot be
    // min ranks count is 5 and max is 10.

    do
    {
        fields = result->Fetch();

        uint32 rankID        = fields[0].GetUInt32();
        std::string rankName = fields[1].GetCppString();
        uint32 rankRights    = fields[2].GetUInt32();

        if (rankID != m_Ranks.size())                       // guild_rank.ids are sequence 0,1,2,3..
            broken_ranks =  true;

        //first rank is guildmaster, prevent loss leader rights
        if (m_Ranks.empty())
            rankRights |= GR_RIGHT_ALL;

        AddRank(rankName,rankRights);
    }while( result->NextRow() );
    delete result;

    if (m_Ranks.size() < GUILD_RANKS_MIN_COUNT)             // if too few ranks, renew them
    {
        m_Ranks.clear();
        sLog.outError("Guild %u has broken `guild_rank` data, creating new...",GuildId);
        CreateDefaultGuildRanks(0);                         // 0 is default locale_idx
        broken_ranks = false;
    }
    // guild_rank have wrong numbered ranks, repair
    if (broken_ranks)
    {
        sLog.outError("Guild %u has broken `guild_rank` data, repairing...",GuildId);
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid='%u'", GuildId);
        for(size_t i = 0; i < m_Ranks.size(); ++i)
        {
            std::string name = m_Ranks[i].Name;
            uint32 rights = m_Ranks[i].Rights;
            CharacterDatabase.escape_string(name);
            CharacterDatabase.PExecute( "INSERT INTO guild_rank (guildid,rid,rname,rights) VALUES ('%u', '%u', '%s', '%u')", GuildId, uint32(i), name.c_str(), rights);
        }
        CharacterDatabase.CommitTransaction();
    }

    return true;
}

bool Guild::LoadMembersFromDB(uint32 GuildId)
{
    //                                                     0                 1     2      3
    QueryResult *result = CharacterDatabase.PQuery("SELECT guild_member.guid,rank, pnote, offnote,"
    //   4               5                 6                7               8
        "characters.name, characters.level, characters.class, characters.zone, characters.logout_time "
        "FROM guild_member LEFT JOIN characters ON characters.guid = guild_member.guid WHERE guildid = '%u'", GuildId);

    if (!result)
        return false;

    do
    {
        Field *fields = result->Fetch();
        MemberSlot newmember;
        uint64 guid      = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        newmember.RankId = fields[1].GetUInt32();
        // don't allow member to have not existing rank!
        if (newmember.RankId >= m_Ranks.size())
            newmember.RankId = GetLowestRank();

        newmember.Pnote                 = fields[2].GetCppString();
        newmember.OFFnote               = fields[3].GetCppString();

        newmember.Name                  = fields[4].GetCppString();
        newmember.Level                 = fields[5].GetUInt8();
        newmember.Class                 = fields[6].GetUInt8();
        newmember.ZoneId                = fields[7].GetUInt32();
        newmember.LogoutTime            = fields[8].GetUInt64();

        // this code will remove not existing character guids from guild
        if (newmember.Level < 1 || newmember.Level > STRONG_MAX_LEVEL) // can be at broken `data` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `characters`.`data`, deleting him from guild!",GUID_LOPART(guid));
            CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guid = '%u'", GUID_LOPART(guid));
            continue;
        }
        if (!newmember.ZoneId)
        {
            sLog.outError("Player (GUID: %u) has broken zone-data", GUID_LOPART(guid));
            // here it will also try the same, to get the zone from characters-table, but additional it tries to find
            // the zone through xy coords .. this is a bit redundant, but shouldn't be called often
            newmember.ZoneId = Player::GetZoneIdFromDB(guid);
        }
        if (newmember.Class < CLASS_WARRIOR || newmember.Class >= MAX_CLASSES) // can be at broken `class` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `characters`.`class`, deleting him from guild!",GUID_LOPART(guid));
            CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guid = '%u'", GUID_LOPART(guid));
            continue;
        }

        members[GUID_LOPART(guid)]      = newmember;

    }while( result->NextRow() );
    delete result;

    if (members.empty())
        return false;

    return true;
}

void Guild::SetMemberStats(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    Player *pl = ObjectAccessor::FindPlayer(guid);
    if (!pl)
        return;
    itr->second.Name   = pl->GetName();
    itr->second.Level  = pl->getLevel();
    itr->second.Class  = pl->getClass();
    itr->second.ZoneId = pl->GetZoneId();
}

void Guild::SetLeader(uint64 guid)
{
    m_LeaderGuid = guid;
    ChangeRank(guid, GR_GUILDMASTER);

    CharacterDatabase.PExecute("UPDATE guild SET leaderguid='%u' WHERE guildid='%u'", GUID_LOPART(guid), m_Id);
}

void Guild::DelMember(uint64 guid, bool isDisbanding)
{
    // guild master can be deleted when loading guild and guid doesn't exist in characters table
    // or when he is removed from guild by gm command
    if (m_LeaderGuid == guid && !isDisbanding)
    {
        MemberSlot* oldLeader = NULL;
        MemberSlot* best = NULL;
        uint64 newLeaderGUID = 0;
        for (Guild::MemberList::iterator i = members.begin(); i != members.end(); ++i)
        {
            if (i->first == GUID_LOPART(guid))
            {
                oldLeader = &(i->second);
                continue;
            }

            if (!best || best->RankId > i->second.RankId)
            {
                best = &(i->second);
                newLeaderGUID = i->first;
            }
        }
        if (!best)
        {
            Disband();
            return;
        }

        SetLeader(newLeaderGUID);

        // If player not online data in data field will be loaded from guild tabs no need to update it !!
        if (Player *newLeader = sObjectMgr.GetPlayer(newLeaderGUID))
            newLeader->SetRank(GR_GUILDMASTER);

        // when leader non-exist (at guild load with deleted leader only) not send broadcasts
        if (oldLeader)
        {
            BroadcastEvent(GE_LEADER_CHANGED, 0, 2, oldLeader->Name, best->Name, "");

            BroadcastEvent(GE_LEFT, guid, 1, oldLeader->Name, "", "");
        }
    }

    members.erase(GUID_LOPART(guid));

    Player *player = sObjectMgr.GetPlayer(guid);
    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if (player)
    {
        player->SetInGuild(0);
        player->SetRank(0);
    }

    CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guid = '%u'", GUID_LOPART(guid));
}

void Guild::ChangeRank(uint64 guid, uint32 newRank)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr != members.end())
        itr->second.RankId = newRank;

    Player *player = sObjectMgr.GetPlayer(guid);
    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if (player)
        player->SetRank(newRank);

    CharacterDatabase.PExecute( "UPDATE guild_member SET rank='%u' WHERE guid='%u'", newRank, GUID_LOPART(guid) );
}

void Guild::SetPNOTE(uint64 guid,std::string pnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end())
        return;

    itr->second.Pnote = pnote;

    // pnote now can be used for encoding to DB
    CharacterDatabase.escape_string(pnote);
    CharacterDatabase.PExecute("UPDATE guild_member SET pnote = '%s' WHERE guid = '%u'", pnote.c_str(), itr->first);
}

void Guild::SetOFFNOTE(uint64 guid,std::string offnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end())
        return;
    itr->second.OFFnote = offnote;
    // offnote now can be used for encoding to DB
    CharacterDatabase.escape_string(offnote);
    CharacterDatabase.PExecute("UPDATE guild_member SET offnote = '%s' WHERE guid = '%u'", offnote.c_str(), itr->first);
}

void Guild::BroadcastToGuild(WorldSession *session, const std::string& msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket data;
        ChatHandler(session).FillMessageData(&data, CHAT_MSG_GUILD, language, 0, msg.c_str());

        for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_GCHATLISTEN) && !pl->GetSocial()->HasIgnore(session->GetPlayer()->GetGUIDLow()) )
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, const std::string& msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_OFFCHATSPEAK))
    {
        for(MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            WorldPacket data;
            ChatHandler::FillMessageData(&data, session, CHAT_MSG_OFFICER, language, NULL, 0, msg.c_str(),NULL);

            Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_OFFCHATLISTEN) && !pl->GetSocial()->HasIgnore(session->GetPlayer()->GetGUIDLow()))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastPacket(WorldPacket *packet)
{
    for(MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player *player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
        if (player)
            player->GetSession()->SendPacket(packet);
    }
}

void Guild::BroadcastPacketToRank(WorldPacket *packet, uint32 rankId)
{
    for(MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->second.RankId == rankId)
        {
            Player *player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
            if (player)
                player->GetSession()->SendPacket(packet);
        }
    }
}

void Guild::CreateRank(std::string name_,uint32 rights)
{
    if (m_Ranks.size() >= GUILD_RANKS_MAX_COUNT)
        return;

    // ranks are sequence 0,1,2,... where 0 means guildmaster
    uint32 new_rank_id = m_Ranks.size();

    AddRank(name_, rights);

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute( "INSERT INTO guild_rank (guildid,rid,rname,rights) VALUES ('%u', '%u', '%s', '%u')", m_Id, new_rank_id, name_.c_str(), rights );
}

void Guild::AddRank(const std::string& name_,uint32 rights)
{
    m_Ranks.push_back(RankInfo(name_,rights));
}

void Guild::DelRank()
{
    // client won't allow to have less than GUILD_RANKS_MIN_COUNT ranks in guild
    if (m_Ranks.size() <= GUILD_RANKS_MIN_COUNT)
        return;

    // delete lowest guild_rank
    uint32 rank = GetLowestRank();
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE rid>='%u' AND guildid='%u'", rank, m_Id);

    m_Ranks.pop_back();
}

std::string Guild::GetRankName(uint32 rankId)
{
    if (rankId >= m_Ranks.size())
        return "<unknown>";

    return m_Ranks[rankId].Name;
}

uint32 Guild::GetRankRights(uint32 rankId)
{
    if (rankId >= m_Ranks.size())
        return 0;

    return m_Ranks[rankId].Rights;
}

void Guild::SetRankName(uint32 rankId, std::string name_)
{
    if (rankId >= m_Ranks.size())
        return;

    m_Ranks[rankId].Name = name_;

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute("UPDATE guild_rank SET rname='%s' WHERE rid='%u' AND guildid='%u'", name_.c_str(), rankId, m_Id);
}

void Guild::SetRankRights(uint32 rankId, uint32 rights)
{
    if (rankId >= m_Ranks.size())
        return;

    m_Ranks[rankId].Rights = rights;

    CharacterDatabase.PExecute("UPDATE guild_rank SET rights='%u' WHERE rid='%u' AND guildid='%u'", rights, rankId, m_Id);
}

int32 Guild::GetRank(uint32 LowGuid)
{
    MemberList::const_iterator itr = members.find(LowGuid);
    if (itr == members.end())
        return -1;

    return itr->second.RankId;
}

void Guild::Disband()
{
    BroadcastEvent(GE_DISBANDED, 0, 0, "", "", "");

    while (!members.empty())
    {
        MemberList::const_iterator itr = members.begin();
        DelMember(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER), true);
    }

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM guild WHERE guildid = '%u'", m_Id);
    CharacterDatabase.PExecute("DELETE FROM guild_rank WHERE guildid = '%u'", m_Id);
    // TODO item_instance should be deleted ?
    CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE guildid = '%u'", m_Id);
    CharacterDatabase.CommitTransaction();
    sObjectMgr.RemoveGuild(m_Id);
}

void Guild::Roster(WorldSession *session /*= NULL*/)
{
                                                            // we can only guess size
    WorldPacket data(SMSG_GUILD_ROSTER, (4+MOTD.length()+1+GINFO.length()+1+4+m_Ranks.size()*4+members.size()*50));
    data << uint32(members.size());
    data << MOTD;
    data << GINFO;

    data << uint32(m_Ranks.size());
    for (RankList::const_iterator ritr = m_Ranks.begin(); ritr != m_Ranks.end(); ++ritr)
        data << uint32(ritr->Rights);

    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (Player *pl = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER)))
        {
            data << (uint64)pl->GetGUID();
            data << (uint8)1;
            data << (std::string)pl->GetName();
            data << (uint32)itr->second.RankId;
            data << (uint8)pl->getLevel();
            data << (uint8)pl->getClass();
            data << (uint32)pl->GetZoneId();
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
        else
        {
            data << uint64(MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER));
            data << uint8(0);
            data << itr->second.Name;
            data << uint32(itr->second.RankId);
            data << uint8(itr->second.Level);
            data << uint8(itr->second.Class);
            data << uint32(itr->second.ZoneId);
            data << float(float(time(NULL)-itr->second.LogoutTime) / DAY);
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
    }
    if (session)
        session->SendPacket(&data);
    else
        BroadcastPacket(&data);
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );
}

void Guild::Query(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, (8*32+200));// we can only guess size

    data << uint32(m_Id);
    data << m_Name;

    for (size_t i = 0 ; i < 10; ++i)                        // show always 10 ranks
    {
        if (i < m_Ranks.size())
            data << m_Ranks[i].Name;
        else
            data << uint8(0);                               // null string
    }

    data << uint32(m_EmblemStyle);
    data << uint32(m_EmblemColor);
    data << uint32(m_BorderStyle);
    data << uint32(m_BorderColor);
    data << uint32(m_BackgroundColor);

    session->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_QUERY_RESPONSE)" );
}

void Guild::SetEmblem(uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor)
{
    m_EmblemStyle = emblemStyle;
    m_EmblemColor = emblemColor;
    m_BorderStyle = borderStyle;
    m_BorderColor = borderColor;
    m_BackgroundColor = backgroundColor;

    CharacterDatabase.PExecute("UPDATE guild SET EmblemStyle=%u, EmblemColor=%u, BorderStyle=%u, BorderColor=%u, BackgroundColor=%u WHERE guildid = %u", m_EmblemStyle, m_EmblemColor, m_BorderStyle, m_BorderColor, m_BackgroundColor, m_Id);
}

void Guild::UpdateLogoutTime(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    itr->second.LogoutTime = time(NULL);

    if (m_OnlineMembers > 0)
        --m_OnlineMembers;
    else
        UnloadGuildEventLog();
}

// *************************************************
// Guild Eventlog part
// *************************************************
// Display guild eventlog
void Guild::DisplayGuildEventLog(WorldSession *session)
{
    // Load guild eventlog, if not already done
    if (!m_EventLogLoaded)
        LoadGuildEventLogFromDB();

    // Sending result
    WorldPacket data(MSG_GUILD_EVENT_LOG_QUERY, 0);
    // count, max count == 100
    data << uint8(m_GuildEventLog.size());
    for (GuildEventLog::const_iterator itr = m_GuildEventLog.begin(); itr != m_GuildEventLog.end(); ++itr)
    {
        // Event type
        data << uint8(itr->EventType);
        // Player 1
        data << uint64(itr->PlayerGuid1);
        // Player 2 not for left/join guild events
        if (itr->EventType != GUILD_EVENT_LOG_JOIN_GUILD && itr->EventType != GUILD_EVENT_LOG_LEAVE_GUILD)
            data << uint64(itr->PlayerGuid2);
        // New Rank - only for promote/demote guild events
        if (itr->EventType == GUILD_EVENT_LOG_PROMOTE_PLAYER || itr->EventType == GUILD_EVENT_LOG_DEMOTE_PLAYER)
            data << uint8(itr->NewRank);
        // Event timestamp
        data << uint32(time(NULL)-itr->TimeStamp);
    }
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_EVENT_LOG_QUERY)");
}

// Load guild eventlog from DB
void Guild::LoadGuildEventLogFromDB()
{
    // Return if already loaded
    if (m_EventLogLoaded)
        return;

    //                                                     0        1          2            3            4        5
    QueryResult *result = CharacterDatabase.PQuery("SELECT LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp FROM guild_eventlog WHERE guildid=%u ORDER BY TimeStamp DESC,LogGuid DESC LIMIT %u", m_Id, GUILD_EVENTLOG_MAX_RECORDS);
    if (!result)
        return;
    bool isNextLogGuidSet = false;
    //uint32 configCount = sWorld.getConfig(CONFIG_UINT32_GUILD_EVENT_LOG_COUNT);
    // First event in list will be the oldest and the latest event is last event in list
    do
    {
        Field *fields = result->Fetch();
        if (!isNextLogGuidSet)
        {
            m_GuildEventLogNextGuid = fields[0].GetUInt32();
            isNextLogGuidSet = true;
        }
        // Fill entry
        GuildEventLogEntry NewEvent;
        NewEvent.EventType = fields[1].GetUInt8();
        NewEvent.PlayerGuid1 = fields[2].GetUInt32();
        NewEvent.PlayerGuid2 = fields[3].GetUInt32();
        NewEvent.NewRank = fields[4].GetUInt8();
        NewEvent.TimeStamp = fields[5].GetUInt64();

        // There can be a problem if more events have same TimeStamp the ORDER can be broken when fields[0].GetUInt32() == configCount, but
        // events with same timestamp can appear when there is lag, and we naivly suppose that mangos isn't laggy
        // but if problem appears, player will see set of guild events that have same timestamp in bad order

        // Add entry to list
        m_GuildEventLog.push_front(NewEvent);

    } while( result->NextRow() );
    delete result;

    m_EventLogLoaded = true;
}

// Unload guild eventlog
void Guild::UnloadGuildEventLog()
{
    if (!m_EventLogLoaded)
        return;

    m_GuildEventLog.clear();
    m_EventLogLoaded = false;
}

// Add entry to guild eventlog
void Guild::LogGuildEvent(uint8 EventType, uint32 PlayerGuid1, uint32 PlayerGuid2, uint8 NewRank)
{
    GuildEventLogEntry NewEvent;
    // Create event
    NewEvent.EventType = EventType;
    NewEvent.PlayerGuid1 = PlayerGuid1;
    NewEvent.PlayerGuid2 = PlayerGuid2;
    NewEvent.NewRank = NewRank;
    NewEvent.TimeStamp = uint32(time(NULL));
    // Count new LogGuid
    m_GuildEventLogNextGuid = (m_GuildEventLogNextGuid + 1) % sWorld.getConfig(CONFIG_UINT32_GUILD_EVENT_LOG_COUNT);
    // Check max records limit
    if (m_GuildEventLog.size() >= GUILD_EVENTLOG_MAX_RECORDS)
        m_GuildEventLog.pop_front();
    // Add event to list
    m_GuildEventLog.push_back(NewEvent);
    // Save event to DB
    CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE guildid='%u' AND LogGuid='%u'", m_Id, m_GuildEventLogNextGuid);
    CharacterDatabase.PExecute("INSERT INTO guild_eventlog (guildid, LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp) VALUES ('%u','%u','%u','%u','%u','%u','" UI64FMTD "')",
        m_Id, m_GuildEventLogNextGuid, uint32(NewEvent.EventType), NewEvent.PlayerGuid1, NewEvent.PlayerGuid2, uint32(NewEvent.NewRank), NewEvent.TimeStamp);
}

void Guild::BroadcastEvent(GuildEvents event, uint64 guid, uint8 strCount, std::string str1, std::string str2, std::string str3)
{
    WorldPacket data(SMSG_GUILD_EVENT, 1+1+(guid ? 8 : 0));
    data << uint8(event);
    data << uint8(strCount);

    switch(strCount)
    {
    case 0:
        break;
    case 1:
        data << str1;
        break;
    case 2:
        data << str1 << str2;
        break;
    case 3:
        data << str1 << str2 << str3;
        break;
    default:
        sLog.outError("Guild::BroadcastEvent: incorrect strings count %u!", strCount);
        break;
    }

    if(guid)
        data << uint64(guid);

    BroadcastPacket(&data);

    sLog.outDebug("WORLD: Sent SMSG_GUILD_EVENT");
}
