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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
    uint32 guildId;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_QUERY");

    recvPacket >> guildId;

    if(Guild *guild = sObjectMgr.GetGuildById(guildId))
    {
        guild->Query(this);
        return;
    }

    SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildCreateOpcode(WorldPacket& recvPacket)
{
    std::string gname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_CREATE");

    recvPacket >> gname;

    if(GetPlayer()->GetGuildId())
        return;

    Guild *guild = new Guild;
    if(!guild->Create(GetPlayer(),gname))
    {
        delete guild;
        return;
    }

    sObjectMgr.AddGuild(guild);
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
    std::string Invitedname, plname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_INVITE");

    Player * player = NULL;

    recvPacket >> Invitedname;

    if(normalizePlayerName(Invitedname))
        player = ObjectAccessor::FindPlayerByName(Invitedname.c_str());

    if(!player)
    {
        SendGuildCommandResult(GUILD_INVITE_S, Invitedname, ERR_GUILD_PLAYER_NOT_FOUND_S);
        return;
    }

    Guild *guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    // OK result but not send invite
    if(player->GetSocial()->HasIgnore(GetPlayer()->GetGUIDLow()))
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != GetPlayer()->GetTeam())
    {
        SendGuildCommandResult(GUILD_INVITE_S, Invitedname, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if(player->GetGuildId())
    {
        plname = player->GetName();
        SendGuildCommandResult(GUILD_INVITE_S, plname, ERR_ALREADY_IN_GUILD_S);
        return;
    }

    if(player->GetGuildIdInvited())
    {
        plname = player->GetName();
        SendGuildCommandResult(GUILD_INVITE_S, plname, ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_INVITE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    sLog.outDebug("Player %s Invited %s to Join his Guild", GetPlayer()->GetName(), Invitedname.c_str());

    player->SetGuildIdInvited(GetPlayer()->GetGuildId());
    // Put record into guildlog
    guild->LogGuildEvent(GUILD_EVENT_LOG_INVITE_PLAYER, GetPlayer()->GetGUIDLow(), player->GetGUIDLow(), 0);

    WorldPacket data(SMSG_GUILD_INVITE, (8+10));            // guess size
    data << GetPlayer()->GetName();
    data << guild->GetName();
    player->GetSession()->SendPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_INVITE)");
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    std::string plName;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_REMOVE");

    recvPacket >> plName;

    if(!normalizePlayerName(plName))
        return;

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_REMOVE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 plGuid;
    MemberSlot* slot = guild->GetMemberSlot(plName, plGuid);
    if(!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if(slot->RankId == GR_GUILDMASTER)
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", ERR_GUILD_LEADER_LEAVE);
        return;
    }

    // do not allow to kick player with same or higher rights
    if(GetPlayer()->GetRank() >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_QUIT_S, plName, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    guild->DelMember(plGuid);
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_UNINVITE_PLAYER, GetPlayer()->GetGUIDLow(), GUID_LOPART(plGuid), 0);

    guild->BroadcastEvent(GE_REMOVED, 0, 2, plName, _player->GetName(), "");
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    Player *player = GetPlayer();

    //sLog.outDebug("WORLD: Received CMSG_GUILD_ACCEPT");

    guild = sObjectMgr.GetGuildById(player->GetGuildIdInvited());
    if(!guild || player->GetGuildId())
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != sObjectMgr.GetPlayerTeamByGUID(guild->GetLeader()))
        return;

    if(!guild->AddMember(GetPlayer()->GetGUID(),guild->GetLowestRank()))
        return;
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_JOIN_GUILD, GetPlayer()->GetGUIDLow(), 0, 0);

    guild->BroadcastEvent(GE_JOINED, player->GetGUID(), 1, player->GetName(), "", "");
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    //sLog.outDebug("WORLD: Received CMSG_GUILD_DECLINE");

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    //sLog.outDebug("WORLD: Received CMSG_GUILD_INFO");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    WorldPacket data(SMSG_GUILD_INFO, (5*4 + guild->GetName().size() + 1));
    data << guild->GetName();
    data << guild->GetCreatedDay();
    data << guild->GetCreatedMonth();
    data << guild->GetCreatedYear();
    data << guild->GetMemberSize();
    data << guild->GetMemberSize();

    SendPacket(&data);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& /*recvPacket*/)
{
    //sLog.outDebug("WORLD: Received CMSG_GUILD_ROSTER");

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
        return;

    guild->Roster(this);
}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recvPacket)
{
    std::string plName;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_PROMOTE");

    recvPacket >> plName;

    if(!normalizePlayerName(plName))
        return;

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_PROMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 plGuid;
    MemberSlot* slot = guild->GetMemberSlot(plName, plGuid);

    if(!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if(plGuid == GetPlayer()->GetGUID())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_NAME_INVALID);
        return;
    }

    // allow to promote only to lower rank than member's rank
    // guildmaster's rank = 0
    // GetPlayer()->GetRank() + 1 is highest rank that current player can promote to
    if(GetPlayer()->GetRank() + 1 >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    uint32 newRankId = slot->RankId - 1;                    //when promoting player, rank is decreased

    guild->ChangeRank(plGuid, newRankId);
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_PROMOTE_PLAYER, GetPlayer()->GetGUIDLow(), GUID_LOPART(plGuid), newRankId);

    guild->BroadcastEvent(GE_PROMOTION, 0, 3, _player->GetName(), plName, guild->GetRankName(newRankId));
}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recvPacket)
{
    std::string plName;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DEMOTE");

    recvPacket >> plName;

    if(!normalizePlayerName(plName))
        return;

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_DEMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 plGuid;
    MemberSlot* slot = guild->GetMemberSlot(plName, plGuid);

    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if(plGuid == GetPlayer()->GetGUID())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_NAME_INVALID);
        return;
    }

    // do not allow to demote same or higher rank
    if(GetPlayer()->GetRank() >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    // do not allow to demote lowest rank
    if(slot->RankId >= guild->GetLowestRank())
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_RANK_TOO_LOW_S);
        return;
    }

    uint32 newRankId = slot->RankId + 1;                    //when demoting player, rank is increased

    guild->ChangeRank(plGuid, newRankId);
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_DEMOTE_PLAYER, GetPlayer()->GetGUIDLow(), GUID_LOPART(plGuid), newRankId);

    guild->BroadcastEvent(GE_DEMOTION, 0, 3, _player->GetName(), plName, guild->GetRankName(slot->RankId));
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& /*recvPacket*/)
{
    std::string plName;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_LEAVE");

    guild = sObjectMgr.GetGuildById(_player->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(_player->GetGUID() == guild->GetLeader() && guild->GetMemberSize() > 1)
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", ERR_GUILD_LEADER_LEAVE);
        return;
    }

    if(_player->GetGUID() == guild->GetLeader())
    {
        guild->Disband();
        return;
    }

    plName = _player->GetName();

    guild->DelMember(_player->GetGUID());
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_LEAVE_GUILD, _player->GetGUIDLow(), 0, 0);

    guild->BroadcastEvent(GE_LEFT, _player->GetGUID(), 1, _player->GetName(), "", "");

    SendGuildCommandResult(GUILD_QUIT_S, guild->GetName(), ERR_PLAYER_NO_MORE_IN_GUILD);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    std::string name;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DISBAND");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->Disband();

    //sLog.outDebug("WORLD: Guild Sucefully Disbanded");
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
    std::string name;
    Player *oldLeader = GetPlayer();
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_LEADER");

    recvPacket >> name;

    if(!normalizePlayerName(name))
        return;

    guild = sObjectMgr.GetGuildById(oldLeader->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (oldLeader->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 newLeaderGUID;
    MemberSlot* slot = guild->GetMemberSlot(name, newLeaderGUID);

    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    guild->SetLeader(newLeaderGUID);
    guild->ChangeRank(oldLeader->GetGUID(), GR_OFFICER);

    guild->BroadcastEvent(GE_LEADER_CHANGED, 0, 2, oldLeader->GetName(), name, "");
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string MOTD;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_MOTD");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_SETMOTD))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    if(!recvPacket.empty())
        recvPacket >> MOTD;
    else
        MOTD = "";

    guild->SetMOTD(MOTD);

    guild->BroadcastEvent(GE_MOTD, 0, 1, MOTD, "", "");
}

void WorldSession::HandleGuildSetPublicNoteOpcode(WorldPacket& recvPacket)
{
    std::string name,PNOTE;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_SET_PUBLIC_NOTE");

    recvPacket >> name;

    if(!normalizePlayerName(name))
        return;

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_EPNOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 plGuid;
    MemberSlot* slot = guild->GetMemberSlot(name, plGuid);

    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    recvPacket >> PNOTE;
    guild->SetPNOTE(plGuid, PNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildSetOfficerNoteOpcode(WorldPacket& recvPacket)
{
    std::string plName, OFFNOTE;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_SET_OFFICER_NOTE");

    recvPacket >> plName;

    if (!normalizePlayerName(plName))
        return;

    Guild* guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_EOFFNOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    uint64 plGuid;
    MemberSlot* slot = guild->GetMemberSlot(plName, plGuid);

    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    recvPacket >> OFFNOTE;
    guild->SetOFFNOTE(plGuid, OFFNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildRankOpcode(WorldPacket& recvPacket)
{
    //recvPacket.hexlike();

    Guild *guild;
    std::string rankname;
    uint32 rankId;
    uint32 rights;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_RANK");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        recvPacket.rpos(recvPacket.wpos());                 // set to end to avoid warnings spam
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        recvPacket.rpos(recvPacket.wpos());                 // set to end to avoid warnings spam
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> rankId;
    recvPacket >> rights;
    recvPacket >> rankname;
    sLog.outDebug("WORLD: Changed RankName to %s , Rights to 0x%.4X", rankname.c_str(), rights);

    guild->SetRankName(rankId, rankname);

    if (rankId == GR_GUILDMASTER)                           // prevent loss leader rights
        rights = GR_RIGHT_ALL;

    guild->SetRankRights(rankId, rights);

    guild->Query(this);
    guild->Roster();                                        // broadcast for tab rights update
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string rankname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_ADD_RANK");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    if(guild->GetRanksSize() >= GUILD_RANKS_MAX_COUNT)      // client not let create more 10 than ranks
        return;

    recvPacket >> rankname;

    guild->CreateRank(rankname, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    guild->Query(this);
    guild->Roster();                                        // broadcast for tab rights update
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    std::string rankname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DEL_RANK");

    guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->DelRank();

    guild->Query(this);
    guild->Roster();                                        // broadcast for tab rights update
}

void WorldSession::SendGuildCommandResult(uint32 typecmd, const std::string& str,uint32 cmdresult)
{
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, (8+str.size()+1));
    data << typecmd;
    data << str;
    data << cmdresult;
    SendPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_COMMAND_RESULT)");
}

void WorldSession::HandleGuildChangeInfoTextOpcode(WorldPacket& recvPacket)
{
    //sLog.outDebug("WORLD: Received CMSG_GUILD_INFO_TEXT");

    std::string GINFO;

    recvPacket >> GINFO;

    Guild *guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_MODIFY_GUILD_INFO))
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->SetGINFO(GINFO);
}

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket)
{
    //sLog.outDebug("WORLD: Received MSG_SAVE_GUILD_EMBLEM");

    uint64 vendorGuid;

    uint32 EmblemStyle;
    uint32 EmblemColor;
    uint32 BorderStyle;
    uint32 BorderColor;
    uint32 BackgroundColor;

    recvPacket >> vendorGuid;

    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(vendorGuid,UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!pCreature)
    {
        //"That's not an emblem vendor!"
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_INVALIDVENDOR);
        sLog.outDebug("WORLD: HandleSaveGuildEmblemOpcode - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(vendorGuid));
        return;
    }

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    recvPacket >> EmblemStyle;
    recvPacket >> EmblemColor;
    recvPacket >> BorderStyle;
    recvPacket >> BorderColor;
    recvPacket >> BackgroundColor;

    Guild *guild = sObjectMgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        //"You are not part of a guild!";
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOGUILD);
        return;
    }

    if (guild->GetLeader() != GetPlayer()->GetGUID())
    {
        //"Only guild leaders can create emblems."
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOTGUILDMASTER);
        return;
    }

    if(GetPlayer()->GetMoney() < 10*GOLD)
    {
        //"You can't afford to do that."
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOTENOUGHMONEY);
        return;
    }

    GetPlayer()->ModifyMoney(-10*GOLD);
    guild->SetEmblem(EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor);

    //"Guild Emblem saved."
    SendSaveGuildEmblem(ERR_GUILDEMBLEM_SUCCESS);

    guild->Query(this);
}

void WorldSession::HandleGuildEventLogQueryOpcode(WorldPacket& /* recvPacket */)
{
                                                            // empty
    sLog.outDebug("WORLD: Received (MSG_GUILD_EVENT_LOG_QUERY)");
    //recvPacket.hexlike();

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = sObjectMgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    pGuild->DisplayGuildEventLog(this);
}

void WorldSession::SendSaveGuildEmblem( uint32 msg )
{
    WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
    data << uint32(msg);                                    // not part of guild
    SendPacket( &data );
}
