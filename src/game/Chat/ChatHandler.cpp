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
#include "Log.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Server/Opcodes.h"
#include "Globals/ObjectMgr.h"
#include "Chat/Chat.h"
#include "Chat/ChannelMgr.h"
#include "Groups/Group.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"
#include "Entities/Player.h"
#include "Spells/SpellAuras.h"
#include "Tools/Language.h"
#include "Util.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "GMTickets/GMTicketMgr.h"

bool WorldSession::CheckChatMessage(std::string& msg, bool addon/* = false*/)
{
#ifdef BUILD_PLAYERBOT
    // bot check can be avoided
    if (_player->GetPlayerbotAI())
        return true;
#endif
    // check max length: as of pre-2.3.x disconnects the player
    if (msg.length() > 255)
    {
        KickPlayer();
        return false;
    }

    // skip remaining checks for addon messages or higher sec level accounts
    if (addon || GetSecurity() > SEC_PLAYER)
        return true;

    // strip invisible characters for non-addon messages
    if (sWorld.getConfig(CONFIG_BOOL_CHAT_FAKE_MESSAGE_PREVENTING))
        stripLineInvisibleChars(msg);

    // check links
    if (sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_SEVERITY))
    {
        if (!ChatHandler::CheckEscapeSequences(msg.c_str()))
        {
            sLog.outError("Player %s (GUID: %u) sent a chat message with an invalid escape sequence: \"%s\"", GetPlayer()->GetName(),
                          GetPlayer()->GetGUIDLow(), msg.c_str());

            if (sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();

            return false;
        }
    }
    return true;
}

void WorldSession::HandleMessagechatOpcode(WorldPacket& recv_data)
{
    uint32 type;
    uint32 lang;

    recv_data >> type;
    recv_data >> lang;

    if (type >= MAX_CHAT_MSG_TYPE)
    {
        sLog.outError("CHAT: Wrong message type received: %u", type);
        return;
    }

    DEBUG_LOG("CHAT: packet received. type %u, lang %u", type, lang);

    // prevent talking at unknown language (cheating)
    LanguageDesc const* langDesc = GetLanguageDescByID(lang);
    if (!langDesc)
    {
        SendNotification(LANG_UNKNOWN_LANGUAGE);
        return;
    }
    if (langDesc->skill_id != 0 && !_player->HasSkill(langDesc->skill_id))
    {
        SendNotification(LANG_NOT_LEARNED_LANGUAGE);
        return;
    }

    if (lang == LANG_ADDON)
    {
        // Disabled addon channel?
        if (!sWorld.getConfig(CONFIG_BOOL_ADDON_CHANNEL))
            return;
    }
    // LANG_ADDON should not be changed nor be affected by flood control
    else
    {
        // send in universal language if player in .gmon mode (ignore spell effects)
        if (_player->isGameMaster())
            lang = LANG_UNIVERSAL;
        else
        {
            // send in universal language in two side iteration allowed mode
            if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                lang = LANG_UNIVERSAL;
            else
            {
                switch (type)
                {
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_RAID:
                    case CHAT_MSG_RAID_LEADER:
                    case CHAT_MSG_RAID_WARNING:
                        // allow two side chat at group channel if two side group allowed
                        if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                            lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_GUILD:
                    case CHAT_MSG_OFFICER:
                        // allow two side chat at guild channel if two side guild allowed
                        if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD))
                            lang = LANG_UNIVERSAL;
                        break;
                }
            }

            // but overwrite it by SPELL_AURA_MOD_LANGUAGE auras (only single case used)
            Unit::AuraList const& ModLangAuras = _player->GetAurasByType(SPELL_AURA_MOD_LANGUAGE);
            if (!ModLangAuras.empty())
                lang = ModLangAuras.front()->GetModifier()->m_miscvalue;
        }

        if (type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
        {
            if (!_player->CanSpeak())
            {
                std::string timeStr = secsToTimeString(m_muteTime - time(nullptr));
                SendNotification(GetMangosString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
                return;
            }

            GetPlayer()->UpdateSpeakTime();
        }
    }

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        {
            std::string msg;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg))
                return;

            if (type == CHAT_MSG_SAY)
                GetPlayer()->Say(msg, lang);
            else if (type == CHAT_MSG_EMOTE)
                GetPlayer()->TextEmote(msg);
            else if (type == CHAT_MSG_YELL)
                GetPlayer()->Yell(msg, lang);
        } break;

        case CHAT_MSG_WHISPER:
        {
            std::string to, msg;
            recv_data >> to;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            if (!normalizePlayerName(to))
            {
                SendPlayerNotFoundNotice(to);
                break;
            }

            Player* player = sObjectMgr.GetPlayer(to.c_str());

            std::pair<bool, bool> hookresult = sTicketMgr.HookGMTicketWhisper(GetPlayer(), to, player, msg, (lang == LANG_ADDON));

            if (hookresult.first)
            {
                if (!hookresult.second)
                    SendPlayerNotFoundNotice(to);
                return;
            }

            uint32 tSecurity = GetSecurity();
            uint32 pSecurity = player ? player->GetSession()->GetSecurity() : SEC_PLAYER;
            if (!player || (tSecurity == SEC_PLAYER && pSecurity > SEC_PLAYER && !player->isAcceptWhispers()))
            {
                SendPlayerNotFoundNotice(to);
                return;
            }

            if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHAT) && tSecurity == SEC_PLAYER && pSecurity == SEC_PLAYER)
            {
                if (GetPlayer()->GetTeam() != player->GetTeam())
                {
                    SendWrongFactionNotice();
                    return;
                }
            }

            GetPlayer()->Whisper(msg, lang, player->GetObjectGuid());
        } break;

        case CHAT_MSG_PARTY:
        {
            std::string msg;

            if (lang == LANG_ADDON)
                recv_data.read(msg , false);
            else
                recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            // if player is in battleground, party chat is sent only to members of normal group
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group)
                return;

            if (group->isBattleGroup())
                group = _player->GetOriginalGroup();

            if (!group)
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, ChatMsg(type), msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, false, group->GetMemberGroup(GetPlayer()->GetObjectGuid()));

            break;
        }
        case CHAT_MSG_GUILD:
        {
            std::string msg;

            if (lang == LANG_ADDON)
                recv_data.read(msg , false);
            else
                recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            if (GetPlayer()->GetGuildId())
                if (Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId()))
                    guild->BroadcastToGuild(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);

            break;
        }
        case CHAT_MSG_OFFICER:
        {
            std::string msg;

            if (lang == LANG_ADDON)
                recv_data.read(msg , false);
            else
                recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            if (GetPlayer()->GetGuildId())
                if (Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId()))
                    guild->BroadcastToOfficers(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);

            break;
        }
        case CHAT_MSG_RAID:
        {
            std::string msg;

            if (lang == LANG_ADDON)
                recv_data.read(msg , false);
            else
                recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            // if player is in battleground, raid chat is sent only to members of normal group
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group)
                return;

            if (group->isBattleGroup())
                group = _player->GetOriginalGroup();

            if (!group || !group->isRaidGroup())
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID, msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, false);
        } break;
        case CHAT_MSG_RAID_LEADER:
        {
            std::string msg;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (ChatHandler(this).ParseCommands(msg.c_str()))
                break;

            if (!CheckChatMessage(msg))
                return;

            // if player is in battleground, raid chat is sent only to members of normal group
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group)
                return;

            if (group->isBattleGroup())
                group = _player->GetOriginalGroup();

            if (!group || !group->isRaidGroup() || !group->IsLeader(_player->GetObjectGuid()))
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID_LEADER, msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, false);
        } break;

        case CHAT_MSG_RAID_WARNING:
        {
            std::string msg;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (!CheckChatMessage(msg))
                return;

            // if player is in battleground, raid warning is sent only to players in battleground
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group)
                return;
            else if (group->isRaidGroup())
            {
                if (!group->IsLeader(_player->GetObjectGuid()) && !group->IsAssistant(_player->GetObjectGuid()))
                    return;
            }
            else if (sWorld.getConfig(CONFIG_BOOL_CHAT_RESTRICTED_RAID_WARNINGS))
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID_WARNING, msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, true);
        } break;

        case CHAT_MSG_BATTLEGROUND:
        {
            std::string msg;

            if (lang == LANG_ADDON)
                recv_data.read(msg , false);
            else
                recv_data >> msg;

            if (msg.empty())
                break;

            if (!CheckChatMessage(msg, (lang == LANG_ADDON)))
                return;

            // battleground chat is sent only to players in battleground
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group || !group->isBattleGroup())
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_BATTLEGROUND, msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, false);
        } break;

        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            std::string msg;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (!CheckChatMessage(msg))
                return;

            // battleground chat is sent only to players in battleground
            // battleground raid is always in Player->GetGroup(), never in GetOriginalGroup()
            Group* group = _player->GetGroup();

            if (!group || !group->isBattleGroup() || !group->IsLeader(_player->GetObjectGuid()))
                return;

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_BATTLEGROUND_LEADER, msg.c_str(), Language(lang), _player->GetChatTag(), _player->GetObjectGuid(), _player->GetName());
            group->BroadcastPacket(data, false);
        } break;

        case CHAT_MSG_CHANNEL:
        {
            std::string channel, msg;
            recv_data >> channel;
            recv_data >> msg;

            if (msg.empty())
                break;

            if (!CheckChatMessage(msg))
                return;

            if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
                if (Channel* chn = cMgr->GetChannel(channel, _player))
                    chn->Say(_player, msg.c_str(), lang);
        } break;

        case CHAT_MSG_AFK:
        case CHAT_MSG_DND:
        {
            std::string msg;
            recv_data >> msg;

            const bool flagged = (type == CHAT_MSG_AFK ? _player->isAFK() : _player->isDND());

            if (flagged)                            // Already flagged
            {
                if (msg.empty())                    // Remove flag
                {
                    if (type == CHAT_MSG_AFK)
                        _player->ToggleAFK();
                    else
                        _player->ToggleDND();
                }
                else                                // Update message
                {
                    if (!CheckChatMessage(msg))
                        msg = GetMangosString(type == CHAT_MSG_AFK ? LANG_PLAYER_AFK_DEFAULT : LANG_PLAYER_DND_DEFAULT);

                    _player->autoReplyMsg = msg;
                }
            }
            else                                    // New AFK/DND mode
            {
                if (msg.empty() || !CheckChatMessage(msg))
                    msg = GetMangosString(type == CHAT_MSG_AFK ? LANG_PLAYER_AFK_DEFAULT : LANG_PLAYER_DND_DEFAULT);

                _player->autoReplyMsg = msg;

                if (type == CHAT_MSG_AFK)
                {
                    if (_player->isDND())
                        _player->ToggleDND();

                    _player->ToggleAFK();
                }
                else
                {
                    if (_player->isAFK())
                        _player->ToggleAFK();

                    _player->ToggleDND();
                }
            }
            break;
        }

        default:
            sLog.outError("CHAT: unknown message type %u, lang: %u", type, lang);
            break;
    }
}

void WorldSession::HandleEmoteOpcode(WorldPacket& recv_data)
{
    if (!GetPlayer()->IsAlive() || GetPlayer()->IsFeigningDeath())
        return;

    uint32 emote;
    recv_data >> emote;

    // restrict to the only emotes hardcoded in client
    if (emote != EMOTE_ONESHOT_NONE && emote != EMOTE_ONESHOT_WAVE)
        return;

    GetPlayer()->HandleEmoteCommand(emote);
}

namespace MaNGOS
{
    class EmoteChatBuilder
    {
        public:
            EmoteChatBuilder(Player const& pl, uint32 text_emote, uint32 emote_num, Unit const* target)
                : i_player(pl), i_text_emote(text_emote), i_emote_num(emote_num), i_target(target) {}

            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* nam = i_target ? i_target->GetNameForLocaleIdx(loc_idx) : nullptr;
                uint32 namlen = (nam ? strlen(nam) : 0) + 1;

                data.Initialize(SMSG_TEXT_EMOTE, (20 + namlen));
                data << ObjectGuid(i_player.GetObjectGuid());
                data << uint32(i_text_emote);
                data << uint32(i_emote_num);
                data << uint32(namlen);
                if (namlen > 1)
                    data.append(nam, namlen);
                else
                    data << uint8(0x00);
            }

        private:
            Player const& i_player;
            uint32        i_text_emote;
            uint32        i_emote_num;
            Unit const*   i_target;
    };
}                                                           // namespace MaNGOS

void WorldSession::HandleTextEmoteOpcode(WorldPacket& recv_data)
{
    if (!GetPlayer()->IsAlive())
        return;

    if (!GetPlayer()->CanSpeak())
    {
        std::string timeStr = secsToTimeString(m_muteTime - time(nullptr));
        SendNotification(GetMangosString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
        return;
    }

    uint32 text_emote, emoteNum;
    ObjectGuid guid;

    recv_data >> text_emote;
    recv_data >> emoteNum;
    recv_data >> guid;

    EmotesTextEntry const* em = sEmotesTextStore.LookupEntry(text_emote);
    if (!em)
        return;

    uint32 emote_id = em->textid;

    switch (emote_id)
    {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        default:
        {
            // in feign death state allowed only text emotes.
            if (GetPlayer()->IsFeigningDeath())
                break;

            GetPlayer()->HandleEmoteCommand(emote_id);
            break;
        }
    }

    Unit* unit = GetPlayer()->GetMap()->GetUnit(guid);

    MaNGOS::EmoteChatBuilder emote_builder(*GetPlayer(), text_emote, emoteNum, unit);
    MaNGOS::LocalizedPacketDo<MaNGOS::EmoteChatBuilder > emote_do(emote_builder);
    MaNGOS::CameraDistWorker<MaNGOS::LocalizedPacketDo<MaNGOS::EmoteChatBuilder > > emote_worker(GetPlayer(), sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE), emote_do);
    Cell::VisitWorldObjects(GetPlayer(), emote_worker,  sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE));

    // Send scripted event call
    if (unit && unit->AI())
        unit->AI()->ReceiveEmote(GetPlayer(), text_emote);
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data)
{
    ObjectGuid iguid;
    // DEBUG_LOG("WORLD: Received opcode CMSG_CHAT_IGNORED");

    recv_data >> iguid;

    Player* player = sObjectMgr.GetPlayer(iguid);
    if (!player || !player->GetSession())
        return;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_IGNORED, _player->GetName(), LANG_UNIVERSAL, CHAT_TAG_NONE, _player->GetObjectGuid());
    player->GetSession()->SendPacket(data);
}

void WorldSession::SendPlayerNotFoundNotice(const std::string& name) const
{
    WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, name.size() + 1);
    data << name;
    SendPacket(data);
}

void WorldSession::SendWrongFactionNotice() const
{
    WorldPacket data(SMSG_CHAT_WRONG_FACTION, 0);
    SendPacket(data);
}

void WorldSession::SendChatRestrictedNotice() const
{
    WorldPacket data(SMSG_CHAT_RESTRICTED, 0);
    SendPacket(data);
}
