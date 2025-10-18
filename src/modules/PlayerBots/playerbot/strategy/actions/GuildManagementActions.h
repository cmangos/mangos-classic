#pragma once
#include "GenericActions.h"
#include "Guilds/GuildMgr.h"

namespace ai
{
    class GuidManageAction : public ChatCommandAction
    {
    public:
        GuidManageAction(PlayerbotAI* ai, std::string name = "guild manage", uint16 opcode = CMSG_GUILD_INVITE) : ChatCommandAction(ai, name), opcode(opcode) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override { return false; }
    
    protected:
        virtual WorldPacket GetPacket(Player* player) { WorldPacket data(Opcodes(opcode), 8); data << player->GetName(); return data; }
        virtual void SendPacket(WorldPacket data, Event event) {};
        virtual void SendPacket(WorldPacket data) { Event event = Event();  SendPacket(data, event); };
        virtual Player* GetPlayer(Event event);
        virtual bool PlayerIsValid(Player* member) { return !member->GetGuildId(); };
        virtual uint8 GetRankId(Player* member) { return sGuildMgr.GetGuildById(member->GetGuildId())->GetMemberSlot(member->GetObjectGuid())->RankId; }
        virtual bool GuildIsFull(uint32 GuildId) { Guild* guild = sGuildMgr.GetGuildById(GuildId); return  guild->GetMemberSize() >= 1000; }

        uint16 opcode;
    };

    class GuildInviteAction : public GuidManageAction 
    {
    public:
        GuildInviteAction(PlayerbotAI* ai, std::string name = "guild invite", uint16 opcode = CMSG_GUILD_INVITE) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return bot->GetGuildId() && sGuildMgr.GetGuildById(bot->GetGuildId())->HasRankRight(bot->GetRank(), GR_RIGHT_INVITE) && !GuildIsFull(bot->GetGuildId()); }
    
    protected:
        virtual void SendPacket(WorldPacket data, Event event) override { bot->GetSession()->HandleGuildInviteOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return !member->GetGuildId(); };
    };

    class GuildJoinAction : public GuidManageAction 
    {
    public:
        GuildJoinAction(PlayerbotAI* ai, std::string name = "guild join", uint16 opcode = CMSG_GUILD_INVITE) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return !bot->GetGuildId(); }
    
    protected:
        virtual WorldPacket GetPacket(Player* player) override { WorldPacket data(Opcodes(opcode), 8); data << bot->GetName(); return data; }
        virtual void SendPacket(WorldPacket data, Event event) override { if(GetPlayer(event)) GetPlayer(event)->GetSession()->HandleGuildInviteOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return !bot->GetGuildId() && member->GetGuildId() && sGuildMgr.GetGuildById(member->GetGuildId())->HasRankRight(member->GetRank(), GR_RIGHT_INVITE) && !GuildIsFull(member->GetGuildId()); };
    };

    class GuildPromoteAction : public GuidManageAction 
    {
    public:
        GuildPromoteAction(PlayerbotAI* ai, std::string name = "guild promote", uint16 opcode = CMSG_GUILD_PROMOTE) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return bot->GetGuildId() && sGuildMgr.GetGuildById(bot->GetGuildId())->HasRankRight(bot->GetRank(), GR_RIGHT_PROMOTE); }
    
    protected:
        virtual void SendPacket(WorldPacket data, Event event) override { bot->GetSession()->HandleGuildPromoteOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member) - 1; };
    };

    class GuildDemoteAction : public GuidManageAction 
    {
    public:
        GuildDemoteAction(PlayerbotAI* ai, std::string name = "guild demote", uint16 opcode = CMSG_GUILD_DEMOTE) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return bot->GetGuildId() && sGuildMgr.GetGuildById(bot->GetGuildId())->HasRankRight(bot->GetRank(), GR_RIGHT_DEMOTE); }
    
    protected:
        virtual void SendPacket(WorldPacket data, Event event) override { bot->GetSession()->HandleGuildDemoteOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member); };
    };

    class GuildLeaderAction : public GuidManageAction 
    {
    public:
        GuildLeaderAction(PlayerbotAI* ai, std::string name = "guild leader", uint16 opcode = CMSG_GUILD_LEADER) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return bot->GetGuildId() && sGuildMgr.GetGuildById(bot->GetGuildId())->GetLeaderGuid() == bot->GetObjectGuid(); }
    
    protected:
        virtual void SendPacket(WorldPacket data, Event event) override { bot->GetSession()->HandleGuildLeaderOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member) - 1; };
    };
    
    class GuildRemoveAction : public GuidManageAction 
    {
    public:
        GuildRemoveAction(PlayerbotAI* ai, std::string name = "guild remove", uint16 opcode = CMSG_GUILD_REMOVE) : GuidManageAction(ai, name, opcode) {}
        virtual bool isUseful() override { return bot->GetGuildId() && sGuildMgr.GetGuildById(bot->GetGuildId())->HasRankRight(bot->GetRank(), GR_RIGHT_REMOVE); }
    
    protected:
        virtual void SendPacket(WorldPacket data, Event event) override { bot->GetSession()->HandleGuildRemoveOpcode(data); };
        virtual bool PlayerIsValid(Player* member) override { return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member); };
    };

    class GuildManageNearbyAction : public ChatCommandAction
    {
    public:
        GuildManageNearbyAction(PlayerbotAI* ai) : ChatCommandAction(ai, "guild manage nearby") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };

    class GuildLeaveAction : public ChatCommandAction
    {
    public:
        GuildLeaveAction(PlayerbotAI* ai) : ChatCommandAction(ai, "guild leave") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override { return bot->GetGuildId(); }
    };
}
