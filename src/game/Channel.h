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

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "Common.h"
#include "ObjectGuid.h"
#include "WorldPacket.h"
#include "Player.h"

#include <map>

#define SPEAK_IN_LOCALDEFENSE_RANK 0
#define SPEAK_IN_WORLDDEFENSE_RANK 11

enum ChatNotify
{
    CHAT_JOINED_NOTICE                = 0x00,               //+ "%s joined channel.";
    CHAT_LEFT_NOTICE                  = 0x01,               //+ "%s left channel.";
    // CHAT_SUSPENDED_NOTICE             = 0x01,            // "%s left channel.";
    CHAT_YOU_JOINED_NOTICE            = 0x02,               //+ "Joined Channel: [%s]"; -- You joined
    // CHAT_YOU_CHANGED_NOTICE           = 0x02,            // "Changed Channel: [%s]";
    CHAT_YOU_LEFT_NOTICE              = 0x03,               //+ "Left Channel: [%s]"; -- You left
    CHAT_WRONG_PASSWORD_NOTICE        = 0x04,               //+ "Wrong password for %s.";
    CHAT_NOT_MEMBER_NOTICE            = 0x05,               //+ "Not on channel %s.";
    CHAT_NOT_MODERATOR_NOTICE         = 0x06,               //+ "Not a moderator of %s.";
    CHAT_PASSWORD_CHANGED_NOTICE      = 0x07,               //+ "[%s] Password changed by %s.";
    CHAT_OWNER_CHANGED_NOTICE         = 0x08,               //+ "[%s] Owner changed to %s.";
    CHAT_PLAYER_NOT_FOUND_NOTICE      = 0x09,               //+ "[%s] Player %s was not found.";
    CHAT_NOT_OWNER_NOTICE             = 0x0A,               //+ "[%s] You are not the channel owner.";
    CHAT_CHANNEL_OWNER_NOTICE         = 0x0B,               //+ "[%s] Channel owner is %s.";
    CHAT_MODE_CHANGE_NOTICE           = 0x0C,               //?
    CHAT_ANNOUNCEMENTS_ON_NOTICE      = 0x0D,               //+ "[%s] Channel announcements enabled by %s.";
    CHAT_ANNOUNCEMENTS_OFF_NOTICE     = 0x0E,               //+ "[%s] Channel announcements disabled by %s.";
    CHAT_MODERATION_ON_NOTICE         = 0x0F,               //+ "[%s] Channel moderation enabled by %s.";
    CHAT_MODERATION_OFF_NOTICE        = 0x10,               //+ "[%s] Channel moderation disabled by %s.";
    CHAT_MUTED_NOTICE                 = 0x11,               //+ "[%s] You do not have permission to speak.";
    CHAT_PLAYER_KICKED_NOTICE         = 0x12,               //? "[%s] Player %s kicked by %s.";
    CHAT_BANNED_NOTICE                = 0x13,               //+ "[%s] You are banned from that channel.";
    CHAT_PLAYER_BANNED_NOTICE         = 0x14,               //? "[%s] Player %s banned by %s.";
    CHAT_PLAYER_UNBANNED_NOTICE       = 0x15,               //? "[%s] Player %s unbanned by %s.";
    CHAT_PLAYER_NOT_BANNED_NOTICE     = 0x16,               //+ "[%s] Player %s is not banned.";
    CHAT_PLAYER_ALREADY_MEMBER_NOTICE = 0x17,               //+ "[%s] Player %s is already on the channel.";
    CHAT_INVITE_NOTICE                = 0x18,               //+ "%2$s has invited you to join the channel '%1$s'.";
    CHAT_INVITE_WRONG_FACTION_NOTICE  = 0x19,               //+ "Target is in the wrong alliance for %s.";
    CHAT_WRONG_FACTION_NOTICE         = 0x1A,               //+ "Wrong alliance for %s.";
    CHAT_INVALID_NAME_NOTICE          = 0x1B,               //+ "Invalid channel name";
    CHAT_NOT_MODERATED_NOTICE         = 0x1C,               //+ "%s is not moderated";
    CHAT_PLAYER_INVITED_NOTICE        = 0x1D,               //+ "[%s] You invited %s to join the channel";
    CHAT_PLAYER_INVITE_BANNED_NOTICE  = 0x1E,               //+ "[%s] %s has been banned.";
    CHAT_THROTTLED_NOTICE             = 0x1F,               //+ "[%s] The number of messages that can be sent to this channel is limited, please wait to send another message.";
};

// DBC Channel ID's
enum ChannelId
{
    CHANNEL_ID_GENERAL              = 1,
    CHANNEL_ID_TRADE                = 2,
    CHANNEL_ID_LOCAL_DEFENSE        = 22,
    CHANNEL_ID_WORLD_DEFENSE        = 23,
    CHANNEL_ID_GUILD_RECRUITMENT    = 25,
    CHANNEL_ID_LOOKING_FOR_GROUP    = 26
};

class Channel
{
        enum ChannelFlags
        {
            CHANNEL_FLAG_NONE       = 0x00,
            CHANNEL_FLAG_CUSTOM     = 0x01,
            // 0x02
            CHANNEL_FLAG_TRADE      = 0x04,
            CHANNEL_FLAG_NOT_LFG    = 0x08,
            CHANNEL_FLAG_GENERAL    = 0x10,
            CHANNEL_FLAG_CITY       = 0x20,
            CHANNEL_FLAG_LFG        = 0x40,
            CHANNEL_FLAG_VOICE      = 0x80
                                      // General                  0x18 = 0x10 | 0x08
                                      // Trade                    0x3C = 0x20 | 0x10 | 0x08 | 0x04
                                      // LocalDefence             0x18 = 0x10 | 0x08
                                      // GuildRecruitment         0x38 = 0x20 | 0x10 | 0x08
                                      // LookingForGroup          0x50 = 0x40 | 0x10
        };

        enum ChannelDBCFlags
        {
            CHANNEL_DBC_FLAG_NONE       = 0x00000,
            CHANNEL_DBC_FLAG_INITIAL    = 0x00001,          // General, Trade, LocalDefense, LFG
            CHANNEL_DBC_FLAG_ZONE_DEP   = 0x00002,          // General, Trade, LocalDefense, GuildRecruitment
            CHANNEL_DBC_FLAG_GLOBAL     = 0x00004,          // WorldDefense
            CHANNEL_DBC_FLAG_TRADE      = 0x00008,          // Trade
            CHANNEL_DBC_FLAG_CITY_ONLY  = 0x00010,          // Trade, GuildRecruitment
            CHANNEL_DBC_FLAG_CITY_ONLY2 = 0x00020,          // Trade, GuildRecruitment
            CHANNEL_DBC_FLAG_DEFENSE    = 0x10000,          // LocalDefense, WorldDefense
            CHANNEL_DBC_FLAG_GUILD_REQ  = 0x20000,          // GuildRecruitment
            CHANNEL_DBC_FLAG_LFG        = 0x40000           // LookingForGroup
        };

        enum ChannelMemberFlags
        {
            MEMBER_FLAG_NONE        = 0x00,
            MEMBER_FLAG_OWNER       = 0x01,
            MEMBER_FLAG_MODERATOR   = 0x02,
            MEMBER_FLAG_VOICED      = 0x04,
            MEMBER_FLAG_MUTED       = 0x08,
            MEMBER_FLAG_CUSTOM      = 0x10,
            MEMBER_FLAG_MIC_MUTED   = 0x20,
            // 0x40
            // 0x80
        };

        struct PlayerInfo
        {
            ObjectGuid player;
            uint8 flags;

            bool HasFlag(uint8 flag) const { return !!(flags & flag); }
            void SetFlag(uint8 flag) { if (!HasFlag(flag)) flags |= flag; }
            bool IsOwner() { return flags & MEMBER_FLAG_OWNER; }
            void SetOwner(bool state)
            {
                if (state) flags |= MEMBER_FLAG_OWNER;
                else flags &= ~MEMBER_FLAG_OWNER;
            }
            bool IsModerator() const { return !!(flags & MEMBER_FLAG_MODERATOR); }
            void SetModerator(bool state)
            {
                if (state) flags |= MEMBER_FLAG_MODERATOR;
                else flags &= ~MEMBER_FLAG_MODERATOR;
            }
            bool IsMuted() const { return !!(flags & MEMBER_FLAG_MUTED); }
            void SetMuted(bool state)
            {
                if (state) flags |= MEMBER_FLAG_MUTED;
                else flags &= ~MEMBER_FLAG_MUTED;
            }
        };

    public:
        Channel(const std::string& name);
        std::string GetName() const { return m_name; }
        uint32 GetChannelId() const { return m_channelId; }
        bool IsConstant() const { return m_channelId != 0; }
        bool IsAnnounce() const { return m_announce; }
        bool IsLFG() const { return !!(GetFlags() & CHANNEL_FLAG_LFG); }
        std::string GetPassword() const { return m_password; }
        void SetPassword(const std::string& npassword) { m_password = npassword; }
        void SetAnnounce(bool nannounce) { m_announce = nannounce; }
        uint32 GetNumPlayers() const { return m_players.size(); }
        uint8 GetFlags() const { return m_flags; }
        bool HasFlag(uint8 flag) const { return !!(m_flags & flag); }

        void Join(Player* player, const char* password);
        void Leave(Player* player, bool send = true);
        void KickOrBan(Player* player, const char* targetName, bool ban);
        void Kick(Player* player, const char* targetName) { KickOrBan(player, targetName, false); }
        void Ban(Player* player, const char* targetName) { KickOrBan(player, targetName, true); }
        void UnBan(Player* player, const char* targetName);
        void Password(Player* player, const char* password);
        void SetMode(Player* player, const char* targetName, bool moderator, bool set);
        void SetOwner(ObjectGuid guid, bool exclaim = true);
        void SetOwner(Player* player, const char* targetName);
        void SendWhoOwner(Player* player);
        void SetModerator(Player* player, const char* targetName) { SetMode(player, targetName, true, true); }
        void UnsetModerator(Player* player, const char* targetName) { SetMode(player, targetName, true, false); }
        void SetMute(Player* player, const char* targetName) { SetMode(player, targetName, false, true); }
        void UnsetMute(Player* player, const char* targetName) { SetMode(player, targetName, false, false); }
        void List(Player* player);
        void Announce(Player* player);
        void Moderate(Player* player);
        void Say(Player* player, const char* text, uint32 lang);
        void Invite(Player* player, const char* targetName);
        void Voice(ObjectGuid guid1, ObjectGuid guid2);
        void DeVoice(ObjectGuid guid1, ObjectGuid guid2);
        void JoinNotify(ObjectGuid guid);                   // invisible notify
        void LeaveNotify(ObjectGuid guid);                  // invisible notify

    private:
        // initial packet data (notify type and channel name)
        void MakeNotifyPacket(WorldPacket* data, uint8 notify_type);
        // type specific packet data
        void MakeJoined(WorldPacket* data, ObjectGuid guid);                    //+ 0x00
        void MakeLeft(WorldPacket* data, ObjectGuid guid);                      //+ 0x01
        void MakeYouJoined(WorldPacket* data);                                  //+ 0x02
        void MakeYouLeft(WorldPacket* data);                                    //+ 0x03
        void MakeWrongPassword(WorldPacket* data);                              //? 0x04
        void MakeNotMember(WorldPacket* data);                                  //? 0x05
        void MakeNotModerator(WorldPacket* data);                               //? 0x06
        void MakePasswordChanged(WorldPacket* data, ObjectGuid guid);           //+ 0x07
        void MakeOwnerChanged(WorldPacket* data, ObjectGuid guid);              //? 0x08
        void MakePlayerNotFound(WorldPacket* data, const std::string& name);    //+ 0x09
        void MakeNotOwner(WorldPacket* data);                                   //? 0x0A
        void MakeChannelOwner(WorldPacket* data);                               //? 0x0B
        void MakeModeChange(WorldPacket* data, ObjectGuid guid, uint8 oldflags);//+ 0x0C
        void MakeAnnouncementsOn(WorldPacket* data, ObjectGuid guid);           //+ 0x0D
        void MakeAnnouncementsOff(WorldPacket* data, ObjectGuid guid);          //+ 0x0E
        void MakeModerationOn(WorldPacket* data, ObjectGuid guid);              //+ 0x0F
        void MakeModerationOff(WorldPacket* data, ObjectGuid guid);             //+ 0x10
        void MakeMuted(WorldPacket* data);                                      //? 0x11
        void MakePlayerKicked(WorldPacket* data, ObjectGuid target, ObjectGuid source);//? 0x12
        void MakeBanned(WorldPacket* data);                                     //? 0x13
        void MakePlayerBanned(WorldPacket* data, ObjectGuid target, ObjectGuid source);//? 0x14
        void MakePlayerUnbanned(WorldPacket* data, ObjectGuid target, ObjectGuid source);//? 0x15
        void MakePlayerNotBanned(WorldPacket* data, const std::string& name);   //? 0x16
        void MakePlayerAlreadyMember(WorldPacket* data, ObjectGuid guid);       //+ 0x17
        void MakeInvite(WorldPacket* data, ObjectGuid guid);                    //? 0x18
        void MakeInviteWrongFaction(WorldPacket* data);                         //? 0x19
        void MakeWrongFaction(WorldPacket* data);                               //? 0x1A
        void MakeInvalidName(WorldPacket* data);                                //? 0x1B
        void MakeNotModerated(WorldPacket* data);                               //? 0x1C
        void MakePlayerInvited(WorldPacket* data, const std::string& name);     //+ 0x1D
        void MakePlayerInviteBanned(WorldPacket* data, const std::string& name);//? 0x1E
        void MakeThrottled(WorldPacket* data);                                  //? 0x1F

        void SendToAll(WorldPacket* data, ObjectGuid guid = ObjectGuid());
        void SendToOne(WorldPacket* data, ObjectGuid who);

        bool IsOn(ObjectGuid who) const { return m_players.find(who) != m_players.end(); }
        bool IsBanned(ObjectGuid guid) const { return m_banned.find(guid) != m_banned.end(); }

        uint8 GetPlayerFlags(ObjectGuid guid) const
        {
            PlayerList::const_iterator p_itr = m_players.find(guid);
            if (p_itr == m_players.end())
                return 0;

            return p_itr->second.flags;
        }

        void SetModerator(ObjectGuid guid, bool set)
        {
            if (m_players[guid].IsModerator() != set)
            {
                uint8 oldFlag = GetPlayerFlags(guid);
                m_players[guid].SetModerator(set);

                WorldPacket data;
                MakeModeChange(&data, guid, oldFlag);
                SendToAll(&data);
            }
        }

        void SetMute(ObjectGuid guid, bool set)
        {
            if (m_players[guid].IsMuted() != set)
            {
                uint8 oldFlag = GetPlayerFlags(guid);
                m_players[guid].SetMuted(set);

                WorldPacket data;
                MakeModeChange(&data, guid, oldFlag);
                SendToAll(&data);
            }
        }

    private:
        bool        m_announce;
        bool        m_moderate;
        std::string m_name;
        std::string m_password;
        uint8       m_flags;
        uint32      m_channelId;
        ObjectGuid  m_ownerGuid;

        typedef     std::map<ObjectGuid, PlayerInfo> PlayerList;
        PlayerList  m_players;
        GuidSet m_banned;
};
#endif
