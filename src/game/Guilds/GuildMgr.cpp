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

#include "Guilds/GuildMgr.h"
#include "Guilds/Guild.h"
#include "Log/Log.h"
#include "Entities/ObjectGuid.h"
#include "Database/DatabaseEnv.h"
#include "Policies/Singleton.h"
#include "Util/ProgressBar.h"
#include "World/World.h"

INSTANTIATE_SINGLETON_1(GuildMgr);

GuildMgr::GuildMgr()
{
}

GuildMgr::~GuildMgr()
{
}

void GuildMgr::AddGuild(Guild* guild)
{
    MaNGOS::unique_trackable_ptr<Guild>& ptr = m_GuildMap[guild->GetId()];
    ptr.reset(guild);
    guild->SetWeakPtr(ptr);
}

void GuildMgr::RemoveGuild(uint32 guildId)
{
    m_GuildMap.erase(guildId);
}

Guild* GuildMgr::GetGuildById(uint32 guildId) const
{
    GuildMap::const_iterator itr = m_GuildMap.find(guildId);
    if (itr != m_GuildMap.end())
        return itr->second.get();

    return nullptr;
}

Guild* GuildMgr::GetGuildByName(std::string const& name) const
{
    for (const auto& itr : m_GuildMap)
        if (itr.second->GetName() == name)
            return itr.second.get();

    return nullptr;
}

Guild* GuildMgr::GetGuildByLeader(ObjectGuid const& guid) const
{
    for (const auto& itr : m_GuildMap)
        if (itr.second->GetLeaderGuid() == guid)
            return itr.second.get();

    return nullptr;
}

std::string GuildMgr::GetGuildNameById(uint32 guildId) const
{
    GuildMap::const_iterator itr = m_GuildMap.find(guildId);
    if (itr != m_GuildMap.end())
        return itr->second->GetName();

    return "";
}

void GuildMgr::LoadGuilds()
{
    Guild* newGuild;
    uint32 count = 0;

    //                                                 0             1          2          3           4           5           6
    auto queryResult = CharacterDatabase.Query("SELECT guild.guildid,guild.name,leaderguid,EmblemStyle,EmblemColor,BorderStyle,BorderColor,"
                          //   7               8    9    10
                          "BackgroundColor,info,motd,createdate FROM guild ORDER BY guildid ASC");

    if (!queryResult)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u guild definitions", count);
        sLog.outString();
        return;
    }

    // load guild ranks
    //                                                      0       1   2     3
    auto guildRanksResult = CharacterDatabase.Query("SELECT guildid,rid,rname,rights FROM guild_rank ORDER BY guildid ASC, rid ASC");

    // load guild members
    //                                                        0       1                  2     3     4
    auto guildMembersResult = CharacterDatabase.Query("SELECT guildid,guild_member.guid,`rank`,pnote,offnote,"
                                      //   5                6                 7                 8                9                       10
                                      "characters.name, characters.level, characters.class, characters.zone, characters.logout_time, characters.account "
                                      "FROM guild_member LEFT JOIN characters ON characters.guid = guild_member.guid ORDER BY guildid ASC");

    BarGoLink bar(queryResult->GetRowCount());

    do
    {
        // Field *fields = queryResult->Fetch();

        bar.step();
        ++count;

        newGuild = new Guild;
        if (!newGuild->LoadGuildFromDB(queryResult.get()) ||
            !newGuild->LoadRanksFromDB(guildRanksResult.get()) ||
            !newGuild->LoadMembersFromDB(guildMembersResult.get()) ||
            !newGuild->CheckGuildStructure()
           )
        {
            newGuild->Disband();
            delete newGuild;
            continue;
        }
        newGuild->LoadGuildEventLogFromDB();
        AddGuild(newGuild);
    }
    while (queryResult->NextRow());

    // delete unused LogGuid records in guild_eventlog table
    // you can comment these lines if you don't plan to change CONFIG_UINT32_GUILD_EVENT_LOG_COUNT
    CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE LogGuid > '%u'", sWorld.getConfig(CONFIG_UINT32_GUILD_EVENT_LOG_COUNT));

    sLog.outString(">> Loaded %u guild definitions", count);
    sLog.outString();
}
