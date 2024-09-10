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

#include "Entities/Object.h"
#include "Entities/Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Entities/Creature.h"
#include "Maps/MapManager.h"
#include "Tools/Language.h"
#include "Spells/SpellAuras.h"
#include "World/World.h"
#include "Groups/Group.h"
#include "Entities/ObjectGuid.h"
#include "Globals/ObjectMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Mails/Mail.h"
#include "Server/WorldPacket.h"
#include "Tools/Formulas.h"
#include "Grids/GridNotifiersImpl.h"
#include "Chat/Chat.h"

#include <cstdarg>

namespace MaNGOS
{
    class BattleGroundChatBuilder
    {
        public:
            BattleGroundChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, va_list* args = nullptr)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_args(args) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);

                ObjectGuid sourceGuid = i_source ? i_source ->GetObjectGuid() : ObjectGuid();
                std::string sourceName = i_source ? i_source ->GetName() : "";

                if (i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *i_args);

                    char str [2048];
                    vsnprintf(str, 2048, text, ap);
                    va_end(ap);

                    ChatHandler::BuildChatPacket(data, i_msgtype, &str[0], LANG_UNIVERSAL, CHAT_TAG_NONE, sourceGuid, sourceName.c_str());
                }
                else
                    ChatHandler::BuildChatPacket(data, i_msgtype, text, LANG_UNIVERSAL, CHAT_TAG_NONE, sourceGuid, sourceName.c_str(), sourceGuid, sourceName.c_str());
            }
        private:
            ChatMsg i_msgtype;
            int32 i_textId;
            Player const* i_source;
            va_list* i_args;
    };

    class BattleGroundYellBuilder
    {
        public:
            BattleGroundYellBuilder(Language language, int32 textId, Creature const* source, va_list* args = nullptr)
                : i_language(language), i_textId(textId), i_source(source), i_args(args) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);

                if (i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *i_args);

                    char str [2048];
                    vsnprintf(str, 2048, text, ap);
                    va_end(ap);

                    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, &str[0], i_language, CHAT_TAG_NONE, i_source->GetObjectGuid(), i_source->GetName());
                }
                else
                    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, text, i_language, CHAT_TAG_NONE, i_source->GetObjectGuid(), i_source->GetName());
            }
        private:
            Language i_language;
            int32 i_textId;
            Creature const* i_source;
            va_list* i_args;
    };

    class BattleGroundBroadcastBuilder
    {
        public:
            BattleGroundBroadcastBuilder(BroadcastText const* bcd, ChatMsg msgtype, Creature const* source, Unit const* target)
                : i_msgtype(msgtype), i_source(source), i_bcd(bcd), i_target(target) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                ChatHandler::BuildChatPacket(data, i_msgtype, i_bcd->GetText(loc_idx, i_source ? i_source->getGender() : GENDER_NONE).c_str(), i_bcd->languageId, CHAT_TAG_NONE, i_source ? i_source->GetObjectGuid() : ObjectGuid(), i_source ? i_source->GetName() : "", i_target ? i_target->GetObjectGuid() : ObjectGuid());
            }
        private:
            ChatMsg i_msgtype;
            Creature const* i_source;
            BroadcastText const* i_bcd;
            Unit const* i_target;
    };


    class BattleGround2ChatBuilder
    {
        public:
            BattleGround2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);
                char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1, loc_idx) : "";
                char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2, loc_idx) : "";

                char str [2048];
                snprintf(str, 2048, text, arg1str, arg2str);

                ObjectGuid guid;
                if (i_source)
                    guid = i_source->GetObjectGuid();

                ChatHandler::BuildChatPacket(data, i_msgtype, str, LANG_UNIVERSAL, CHAT_TAG_NONE, guid);
            }
        private:
            ChatMsg i_msgtype;
            int32 i_textId;
            Player const* i_source;
            int32 i_arg1;
            int32 i_arg2;
    };

    class BattleGround2YellBuilder
    {
        public:
            BattleGround2YellBuilder(Language language, int32 textId, Creature const* source, int32 arg1, int32 arg2)
                : i_language(language), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);
                char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1, loc_idx) : "";
                char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2, loc_idx) : "";

                char str [2048];
                snprintf(str, 2048, text, arg1str, arg2str);

                ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, str, i_language, CHAT_TAG_NONE, i_source ? i_source ->GetObjectGuid() : ObjectGuid(), i_source ? i_source ->GetName() : "");
            }
        private:

            Language i_language;
            int32 i_textId;
            Creature const* i_source;
            int32 i_arg1;
            int32 i_arg2;
    };
}                                                           // namespace MaNGOS

template<class Do>
void BattleGround::BroadcastWorker(Do& _do)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
        if (Player* plr = ObjectAccessor::FindPlayer(itr->first))
            _do(plr);
}

BattleGround::BattleGround(): m_buffChange(false), m_startDelayTime(0), m_startMaxDist(0), m_playerSkinReflootId(0)
{
    m_typeId            = BATTLEGROUND_TYPE_NONE;
    m_status            = STATUS_NONE;
    m_clientInstanceId  = 0;
    m_endTime           = 0;
    m_bracketId         = BG_BRACKET_ID_TEMPLATE;
    m_invitedAlliance   = 0;
    m_invitedHorde      = 0;
    m_winner            = WINNER_NONE;
    m_startTime         = 0;
    m_validStartPositionTimer = 0;
    m_events            = 0;
    m_name              = "";
    m_levelMin          = 0;
    m_levelMax          = 0;
    m_hasBgFreeSlotQueue = false;

    m_maxPlayersPerTeam = 0;
    m_maxPlayers        = 0;
    m_minPlayersPerTeam = 0;
    m_minPlayers        = 0;

    m_mapId             = 0;
    m_bgMap             = nullptr;

    m_teamStartLocX[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocX[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocY[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocY[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocZ[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocZ[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocO[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocO[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocX[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocY[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocY[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocZ[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocZ[TEAM_INDEX_HORDE]      = 0;

    m_teamStartLocO[TEAM_INDEX_ALLIANCE]   = 0;
    m_teamStartLocO[TEAM_INDEX_HORDE]      = 0;

    m_bgRaids[TEAM_INDEX_ALLIANCE]         = nullptr;
    m_bgRaids[TEAM_INDEX_HORDE]            = nullptr;

    m_playersCount[TEAM_INDEX_ALLIANCE]    = 0;
    m_playersCount[TEAM_INDEX_HORDE]       = 0;

    m_prematureCountDown = false;
    m_prematureCountDownTimer = 0;

    m_startDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_2M;
    m_startDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_1M;
    m_startDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_30S;
    m_startDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    // we must set to some default existing values
    m_startMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_startMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;
}

/**
  Deconstructor
*/
BattleGround::~BattleGround()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)

    // skip template bgs as they were never added to visible bg list
    BattleGroundBracketId bracketId = GetBracketId();
    if (bracketId != BG_BRACKET_ID_TEMPLATE)
    {
        sWorld.GetBGQueue().GetMessager().AddMessage([bgTypeId = GetTypeId(), bracketId, clientInstanceId = GetClientInstanceId()](BattleGroundQueue* queue)
        {
            queue->DeleteClientVisibleInstanceId(bgTypeId, bracketId, clientInstanceId);
        });
    }

    // unload map
    // map can be null at bg destruction
    if (m_bgMap)
        m_bgMap->SetUnload();

    // remove from bg free slot queue
    this->RemovedFromBgFreeSlotQueue(true);

    for (BattleGroundScoreMap::const_iterator itr = m_playerScores.begin(); itr != m_playerScores.end(); ++itr)
        delete itr->second;
}

/**
  Update method

  @param    diff
*/
void BattleGround::Update(uint32 diff)
{
    // remove offline players from bg after 5 minutes
    if (!m_offlineQueue.empty())
    {
        BattleGroundPlayerMap::iterator itr = m_players.find(*(m_offlineQueue.begin()));
        if (itr != m_players.end())
        {
            if (itr->second.offlineRemoveTime <= sWorld.GetGameTime())
            {
                // add deserter at next login
                CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", uint32(AT_LOGIN_ADD_BG_DESERTER), itr->first.GetCounter());

                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                m_offlineQueue.pop_front();                 // remove from offline queue
                // do not use itr for anything, because it is erased in RemovePlayerAtLeave()
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND BALLANCE SYSTEM            ***/
    /*********************************************************/

    // if less then minimum players are in on one side, then start premature finish timer
    if (GetStatus() == STATUS_IN_PROGRESS && sBattleGroundMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
    {
        if (!m_prematureCountDown)
        {
            m_prematureCountDown = true;
            m_prematureCountDownTimer = sBattleGroundMgr.GetPrematureFinishTime();
        }
        else if (m_prematureCountDownTimer < diff)
        {
            EndBattleGround(GetPrematureWinner());
            m_prematureCountDown = false;
        }
        else if (!sBattleGroundMgr.IsTesting())
        {
            uint32 newtime = m_prematureCountDownTimer - diff;
            // announce every minute
            if (newtime > (MINUTE * IN_MILLISECONDS))
            {
                if (newtime / (MINUTE * IN_MILLISECONDS) != m_prematureCountDownTimer / (MINUTE * IN_MILLISECONDS))
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING, CHAT_MSG_SYSTEM, nullptr, (uint32)(m_prematureCountDownTimer / (MINUTE * IN_MILLISECONDS)));
            }
            else
            {
                // announce every 15 seconds
                if (newtime / (15 * IN_MILLISECONDS) != m_prematureCountDownTimer / (15 * IN_MILLISECONDS))
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING_SECS, CHAT_MSG_SYSTEM, nullptr, (uint32)(m_prematureCountDownTimer / IN_MILLISECONDS));
            }
            m_prematureCountDownTimer = newtime;
        }
    }
    else if (m_prematureCountDown)
        m_prematureCountDown = false;

    /*********************************************************/
    /***           BATTLEGROUND STARTING SYSTEM            ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        float maxDist = GetStartMaxDist();
        if (maxDist > 0.0f)
        {
            if (m_validStartPositionTimer < diff)
            {
                for (const auto& itr : GetPlayers())
                {
                    if (Player* player = sObjectMgr.GetPlayer(itr.first))
                    {
                        float x, y, z, o;
                        GetTeamStartLoc(player->GetTeam(), x, y, z, o);
                        if (!player->IsWithinDist3d(x, y, z, maxDist))
                        {
                            player->TeleportTo(GetMapId(), x, y, z, o);
                        }
                    }
                }
                m_validStartPositionTimer = CHECK_PLAYER_POSITION_INVERVAL;
            }
            else
                m_validStartPositionTimer -= diff;
        }

        ModifyStartDelayTime(diff);

        if (!(m_events & BG_STARTING_EVENT_1))
        {
            m_events |= BG_STARTING_EVENT_1;

            StartingEventCloseDoors();
            SetStartDelayTime(m_startDelayTimes[BG_STARTING_EVENT_FIRST]);
            // first start warning - 2 or 1 minute, only if defined
            if (m_startMessageIds[BG_STARTING_EVENT_FIRST])
                SendMessageToAll(m_startMessageIds[BG_STARTING_EVENT_FIRST], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // After 1 minute or 30 seconds, warning is signalled
        else if (GetStartDelayTime() <= m_startDelayTimes[BG_STARTING_EVENT_SECOND] && !(m_events & BG_STARTING_EVENT_2))
        {
            m_events |= BG_STARTING_EVENT_2;
            SendMessageToAll(m_startMessageIds[BG_STARTING_EVENT_SECOND], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // After 30 or 15 seconds, warning is signalled
        else if (GetStartDelayTime() <= m_startDelayTimes[BG_STARTING_EVENT_THIRD] && !(m_events & BG_STARTING_EVENT_3))
        {
            m_events |= BG_STARTING_EVENT_3;
            SendMessageToAll(m_startMessageIds[BG_STARTING_EVENT_THIRD], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // delay expired (atfer 2 or 1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_events & BG_STARTING_EVENT_4))
        {
            m_events |= BG_STARTING_EVENT_4;

            StartingEventOpenDoors();

            SendMessageToAll(m_startMessageIds[BG_STARTING_EVENT_FOURTH], CHAT_MSG_BG_SYSTEM_NEUTRAL);
            SetStatus(STATUS_IN_PROGRESS);
            SetStartDelayTime(m_startDelayTimes[BG_STARTING_EVENT_FOURTH]);

            SetStartDelayTime(m_startDelayTimes[BG_STARTING_EVENT_FOURTH]);

            {
                PlaySoundToAll(SOUND_BG_START);

                // Announce BG starting
                if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_QUEUE_ANNOUNCER_START))
                {
                    sWorld.SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, GetName(), GetMinLevel(), GetMaxLevel());
                }
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND ENDING SYSTEM              ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_endTime -= diff;
        if (m_endTime <= 0)
        {
            m_endTime = 0;
            BattleGroundPlayerMap::iterator next;
            for (BattleGroundPlayerMap::iterator itr = m_players.begin(); itr != m_players.end(); itr = next)
            {
                next = itr;
                ++next;
                // itr is erased here!
                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                // do not change any battleground's private variables
            }
        }
    }

    // update start time
    m_startTime += diff;
}

/**
  Method that sets the team start location

  @param    team Id
  @param    x
  @param    y
  @param    z
  @param    o
*/
void BattleGround::SetTeamStartLoc(Team team, float x, float y, float z, float o)
{
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    m_teamStartLocX[teamIdx] = x;
    m_teamStartLocY[teamIdx] = y;
    m_teamStartLocZ[teamIdx] = z;
    m_teamStartLocO[teamIdx] = o;
}

/**
  Method that sends packet to all players

  @param    packet
*/
void BattleGround::SendPacketToAll(WorldPacket const& packet) const
{
    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        if (Player* plr = sObjectMgr.GetPlayer(itr->first))
            plr->GetSession()->SendPacket(packet);
        else
            sLog.outError("BattleGround:SendPacketToAll: %s not found!", itr->first.GetString().c_str());
    }
}

/**
  Method that sends packet to the team

  @param    team Id
  @param    packet
  @param    sender
  @param    self
*/
void BattleGround::SendPacketToTeam(Team teamId, WorldPacket const& packet, Player* sender, bool toSelf) const
{
    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        Player* player = sObjectMgr.GetPlayer(itr->first);
        if (!player)
        {
            sLog.outError("BattleGround:SendPacketToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        if (!toSelf && sender == player)
            continue;

        Team team = itr->second.playerTeam;
        if (team != ALLIANCE && team != HORDE) team = player->GetTeam();

        if (team == teamId)
            player->GetSession()->SendPacket(packet);
    }
}

/**
  Method that plays sound to all players

  @param    sound Id
  @param    team
*/
void BattleGround::PlaySoundToAll(uint32 soundId)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(data, soundId);
    SendPacketToAll(data);
}

/**
  Method that plays sound to the given team

  @param    sound Id
  @param    team
*/
void BattleGround::PlaySoundToTeam(uint32 soundId, Team teamId)
{
    WorldPacket data;

    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outError("BattleGround:PlaySoundToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.playerTeam;
        if (team != ALLIANCE && team != HORDE) team = plr->GetTeam();

        if (team == teamId)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(data, soundId);
            plr->GetSession()->SendPacket(data);
        }
    }
}

/**
  Method casts spell to an entire battleground team

  @param    spell Id
  @param    team
*/
void BattleGround::CastSpellOnTeam(uint32 spellId, Team teamId)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        Player* player = sObjectMgr.GetPlayer(itr->first);

        if (!player)
        {
            sLog.outError("BattleGround:CastSpellOnTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.playerTeam;
        if (team != ALLIANCE && team != HORDE) team = player->GetTeam();

        if (team == teamId)
            player->CastSpell(player, spellId, TRIGGERED_OLD_TRIGGERED);
    }
}

/**
  Method that rewards honor to team - called at the end of the battleground

  @param    honor
  @param    team
*/
void BattleGround::RewardHonorToTeam(uint32 honor, Team teamId)
{
    if (teamId == TEAM_NONE)
        return;

    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        Player* player = sObjectMgr.GetPlayer(itr->first);

        if (!player)
        {
            sLog.outError("BattleGround:RewardHonorToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.playerTeam;
        if (team != ALLIANCE && team != HORDE) team = player->GetTeam();

        if (team == teamId)
            UpdatePlayerScore(player, SCORE_BONUS_HONOR, honor);
    }
}

/**
  Method that rewards reputation to team - called at the end of the battleground

  @param    faction Id
  @param    reputation
  @param    team
*/
void BattleGround::RewardReputationToTeam(uint32 factionId, uint32 reputation, Team teamId)
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);

    if (!factionEntry)
        return;

    for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second.offlineRemoveTime)
            continue;

        Player* player = sObjectMgr.GetPlayer(itr->first);

        if (!player)
        {
            sLog.outError("BattleGround:RewardReputationToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.playerTeam;
        if (team != ALLIANCE && team != HORDE) team = player->GetTeam();

        if (team == teamId)
            player->GetReputationMgr().ModifyReputation(factionEntry, reputation);
    }
}

/**
  Method that updates world state for all players

  @param    field
  @param    value
*/
void BattleGround::UpdateWorldState(uint32 field, uint32 value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(data, field, value);
    SendPacketToAll(data);
}

/**
  Method that updates world state for a give player

  @param    field
  @param    value
  @param    player
*/
void BattleGround::UpdateWorldStateForPlayer(uint32 field, uint32 value, Player* player) const
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(data, field, value);
    player->GetSession()->SendPacket(data);
}

/**
  Method that ends battleground

  @param    winner team
*/
void BattleGround::EndBattleGround(Team winner)
{
    this->RemovedFromBgFreeSlotQueue(true);

    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    WorldPacket data;
    int32 winmsg_id = 0;

    uint32 bgScoresWinner = TEAM_INDEX_NEUTRAL;
    uint64 battleground_id = 1;

    if (winner == ALLIANCE)
    {
        winmsg_id = LANG_BG_A_WINS;
        PlaySoundToTeam(SOUND_ALLIANCE_WINS, winner);

        SetWinner(WINNER_ALLIANCE);

        // reversed index for the bg score storage system
        bgScoresWinner = TEAM_INDEX_HORDE;
    }
    else if (winner == HORDE)
    {
        winmsg_id = LANG_BG_H_WINS;
        PlaySoundToTeam(SOUND_HORDE_WINS, winner);

        SetWinner(WINNER_HORDE);

        // reversed index for the bg score storage system
        bgScoresWinner = TEAM_INDEX_ALLIANCE;
    }
    else
        SetWinner(WINNER_NONE);

    // store battleground scores
    if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_SCORE_STATISTICS))
    {
        static SqlStatementID insPvPstatsBattleground;

        SqlStatement stmt = CharacterDatabase.CreateStatement(insPvPstatsBattleground, "INSERT INTO pvpstats_battlegrounds (id, winner_team, bracket_id, type, date) VALUES (?, ?, ?, ?, " _NOW_ ")");

        uint8 battleground_bracket = GetMinLevel() / 10;
        uint8 battleground_type = (uint8)GetTypeId();

        // query next id
        auto queryResult = CharacterDatabase.Query("SELECT MAX(id) FROM pvpstats_battlegrounds");
        if (queryResult)
        {
            Field* fields = queryResult->Fetch();
            battleground_id = fields[0].GetUInt64() + 1;
        }

        stmt.PExecute(battleground_id, bgScoresWinner, battleground_bracket, battleground_type);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    // we must set it this way, because end time is sent in packet!
    m_endTime = TIME_TO_AUTOREMOVE;

    auto& objectStore = GetBgMap()->GetObjectsStore();
    for (auto itr = objectStore.begin<Creature>(); itr != objectStore.end<Creature>(); ++itr)
    {
        Creature* creature = itr->second;
        if (creature->IsClientControlled())
            continue;
        creature->SetImmuneToNPC(true);
        creature->SetImmuneToPlayer(true);
        creature->SetStunned(true);
    }

    for (auto& m_Player : m_players)
    {
        Team team = m_Player.second.playerTeam;

        if (m_Player.second.offlineRemoveTime)
            continue;

        Player* plr = sObjectMgr.GetPlayer(m_Player.first);
        if (!plr)
        {
            sLog.outError("BattleGround:EndBattleGround %s not found!", m_Player.first.GetString().c_str());
            continue;
        }

        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if (!plr->IsAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
        else
        {
            // needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();
        }

        // this line is obsolete - team is set ALWAYS
        // if(!team) team = plr->GetTeam();

        // store battleground score statistics for each player
        if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_SCORE_STATISTICS))
        {
            static SqlStatementID insPvPstatsPlayer;
            BattleGroundScoreMap::iterator score = m_playerScores.find(m_Player.first);
            SqlStatement stmt = CharacterDatabase.CreateStatement(insPvPstatsPlayer, "INSERT INTO pvpstats_players (battleground_id, character_guid, score_killing_blows, score_deaths, score_honorable_kills, score_bonus_honor, score_damage_done, attr_1, attr_2, attr_3, attr_4, attr_5) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            stmt.addUInt32(battleground_id);
            stmt.addUInt32(plr->GetGUIDLow());
            BattleGroundScore *pScore = score->second;
            stmt.addUInt32(pScore->GetKillingBlows());
            stmt.addUInt32(pScore->GetDeaths());
            stmt.addUInt32(pScore->GetHonorableKills());
            stmt.addUInt32(pScore->GetBonusHonor());
            stmt.addUInt32(pScore->GetDishonorableKills());
            stmt.addUInt32(pScore->GetAttr1());
            stmt.addUInt32(pScore->GetAttr2());
            stmt.addUInt32(pScore->GetAttr3());
            stmt.addUInt32(pScore->GetAttr4());
            stmt.addUInt32(pScore->GetAttr5());

            stmt.Execute();
        }

        if (team == winner)
        {
            RewardMark(plr, ITEM_WINNER_COUNT);
            RewardQuestComplete(plr);
        }
        else
            RewardMark(plr, ITEM_LOSER_COUNT);

        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(data, this);
        plr->GetSession()->SendPacket(data);

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(GetTypeId());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, GetTypeId(), GetClientInstanceId(), GetMapId(), plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->GetSession()->SendPacket(data);
    }

    // AV message is different - TODO: check if others are also wrong
    if (winmsg_id && GetTypeId() != BATTLEGROUND_AV)
        SendMessageToAll(winmsg_id, CHAT_MSG_BG_SYSTEM_NEUTRAL);
}

/**
  Method that calculates and rewars bonus honor from kill

  @param    kills
*/
uint32 BattleGround::GetBonusHonorFromKill(uint32 kills) const
{
    // variable kills means how many honorable kills you scored (so we need kills * honor_for_one_kill)
    return (uint32)MaNGOS::Honor::hk_honor_at_level(GetMaxLevel(), kills);
}

void BattleGround::SetStatus(BattleGroundStatus status)
{
    m_status = status;
    sWorld.GetBGQueue().GetMessager().AddMessage([status, bgTypeId = GetTypeId(), instanceId = GetInstanceId()](BattleGroundQueue* queue)
    {
        if (BattleGroundInQueueInfo* bgInstance = queue->GetFreeSlotInstance(bgTypeId, instanceId))
            bgInstance->status = status;
    });
}

/**
  Function that returns the battleground master entry
*/
uint32 BattleGround::GetBattlemasterEntry() const
{
    switch (GetTypeId())
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        default:              return 0;
    }
}

/**
  Method that rewards battleground mark - called at the end of the battleground

  @param    player
  @param    count
*/
void BattleGround::RewardMark(Player* player, uint32 count)
{
    switch (GetTypeId())
    {
        case BATTLEGROUND_AV:
            if (count == ITEM_WINNER_COUNT)
                RewardSpellCast(player, SPELL_AV_MARK_WINNER);
            else
                RewardSpellCast(player, SPELL_AV_MARK_LOSER);
            break;
        case BATTLEGROUND_WS:
            if (count == ITEM_WINNER_COUNT)
                RewardSpellCast(player, SPELL_WS_MARK_WINNER);
            else
                RewardSpellCast(player, SPELL_WS_MARK_LOSER);
            break;
        case BATTLEGROUND_AB:
            if (count == ITEM_WINNER_COUNT)
                RewardSpellCast(player, SPELL_AB_MARK_WINNER);
            else
                RewardSpellCast(player, SPELL_AB_MARK_LOSER);
            break;
        default:
            break;
    }
}

/**
  Method that rewards spell cast - called at the end of the battleground

  @param    player
  @param    spellId
*/
void BattleGround::RewardSpellCast(Player* player, uint32 spellId) const
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
    {
        sLog.outError("Battleground reward casting spell %u not exist.", spellId);
        return;
    }

    player->CastSpell(player, spellInfo, TRIGGERED_OLD_TRIGGERED);
}

/**
  Method that rewards item - called at the end of the battleground

  @param    player
  @param    mark
  @param    count
*/
void BattleGround::RewardItem(Player* player, uint32 itemId, uint32 count)
{
    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &no_space_count);

    if (msg == EQUIP_ERR_ITEM_NOT_FOUND)
    {
        sLog.outErrorDb("Battleground reward item (Entry %u) not exist in `item_template`.", itemId);
        return;
    }

    if (msg != EQUIP_ERR_OK)                                // convert to possible store amount
        count -= no_space_count;

    if (count != 0 && !dest.empty())                        // can add some
        if (Item* item = player->StoreNewItem(dest, itemId, true, 0))
            player->SendNewItem(item, count, true, false);

    if (no_space_count > 0)
        SendRewardMarkByMail(player, itemId, no_space_count);
}

/**
  Method that sends reward by mail - called at the end of the battleground

  @param    player
  @param    mark
  @param    count
*/
void BattleGround::SendRewardMarkByMail(Player* player, uint32 mark, uint32 count) const
{
    uint32 bmEntry = GetBattlemasterEntry();
    if (!bmEntry)
        return;

    ItemPrototype const* markProto = ObjectMgr::GetItemPrototype(mark);
    if (!markProto)
        return;

    if (Item* markItem = Item::CreateItem(mark, count, player))
    {
        // save new item before send
        markItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

        int loc_idx = player->GetSession()->GetSessionDbLocaleIndex();

        // subject: item name
        std::string subject = markProto->Name1;
        sObjectMgr.GetItemLocaleStrings(markProto->ItemId, loc_idx, &subject);

        // text
        std::string textFormat = player->GetSession()->GetMangosString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf, 300, textFormat.c_str(), GetName(), GetName());

        MailDraft(subject, textBuf).AddItem(markItem).SendMailTo(player, MailSender(MAIL_CREATURE, bmEntry));
    }
}

/**
  Method that rewards player quest complete - called at the end of the battleground

  @param    player
*/
void BattleGround::RewardQuestComplete(Player* player)
{
    uint32 quest;
    switch (GetTypeId())
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        default:
            return;
    }

    RewardSpellCast(player, quest);
}

/**
  Method that blocks player movement - called at the end of battleground
*/
void BattleGround::BlockMovement(Player* player)
{
    // TODO: This originally is meant to be applied/removed by the dummy aura casted on battleground end
    // NOTE: control will be automatically reset by client when the player changes the map, so not needed to restore in RemovePlayerAtLeave()
    player->UpdateClientControl(player, false);
    // Set the flag to indicate that server does not want player to have client control
    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CLIENT_CONTROL_LOST);
}

/**
  Method that handles player removal on leaving battleground

  @param    player guid
  @param    isOnTransport
  @param    send Packet update
*/
void BattleGround::RemovePlayerAtLeave(ObjectGuid playerGuid, bool isOnTransport, bool doSendPacket)
{
    Team team = GetPlayerTeam(playerGuid);
    bool participant = false;

    // Remove from lists/maps
    BattleGroundPlayerMap::iterator itr = m_players.find(playerGuid);
    if (itr != m_players.end())
    {
        UpdatePlayersCountByTeam(team, true);               // -1 player
        m_players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    BattleGroundScoreMap::iterator itr2 = m_playerScores.find(playerGuid);
    if (itr2 != m_playerScores.end())
    {
        delete itr2->second;                                // delete player's score
        m_playerScores.erase(itr2);
    }

    Player* player = sObjectMgr.GetPlayer(playerGuid);

    if (player)
    {
        // Remove flag set in BattleGround::BlockMovement()
        player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CLIENT_CONTROL_LOST);

        // should remove spirit of redemption
        if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            player->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        if (!player->IsAlive())                                // resurrect on exit
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }
    }

    RemovePlayer(player, playerGuid);                                // BG subclass specific code

    if (participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        BattleGroundTypeId bgTypeId = GetTypeId();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(GetTypeId());
        if (player)
        {
            if (team != ALLIANCE && team != HORDE) team = player->GetTeam();

            if (doSendPacket)
            {
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, GetTypeId(), GetClientInstanceId(), GetMapId(), player->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                player->GetSession()->SendPacket(data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            player->RemoveBattleGroundQueueId(bgQueueTypeId);
        }

        // remove from raid group if player is member
        if (Group* group = GetBgRaid(team))
        {
            if (!group->RemoveMember(playerGuid, 0))              // group was disbanded
            {
                SetBgRaid(team, nullptr);
                delete group;
            }
        }

        SetInvitedCount(team, GetInvitedCount(team) - 1); // change ahead of free slot queue - will be synched again after
        // we should update battleground queue, but only if bg isn't ending
        if (GetStatus() < STATUS_WAIT_LEAVE)
        {
            // a player has left the battleground, so there are free slots -> add to queue
            if (!AddToBgFreeSlotQueue()) // avoid setting two messages - if was already in queue, just update count
            {
                sWorld.GetBGQueue().GetMessager().AddMessage([bgTypeId, instanceId = GetInstanceId(), team](BattleGroundQueue* queue)
                {
                    if (BattleGroundInQueueInfo* bgInstance = queue->GetFreeSlotInstance(bgTypeId, instanceId))
                        bgInstance->DecreaseInvitedCount(team);
                });
            }
            sWorld.GetBGQueue().GetMessager().AddMessage([bgQueueTypeId, bgTypeId, bracketId = GetBracketId()](BattleGroundQueue* queue)
            {
                queue->ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, bracketId);
            });
        }

        // Let others know
        WorldPacket data;
        sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(data, playerGuid);
        SendPacketToTeam(team, data, player, false);
    }

    if (player)
    {
        // Do next only if found in battleground
        player->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
        // reset destination bg team
        player->SetBGTeam(TEAM_NONE);

        if (isOnTransport)
            player->TeleportToBGEntryPoint();

        DETAIL_LOG("BATTLEGROUND: Removed player %s from BattleGround.", player->GetName());
    }

    // battleground object will be deleted next BattleGround::Update() call
}

/**
  Method that is called when no players remains in battleground
*/
void BattleGround::Reset()
{
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);

    m_events = 0;

    // door-event2 is always 0
    m_activeEvents[BG_EVENT_DOOR] = 0;

    if (m_invitedAlliance > 0 || m_invitedHorde > 0)
        sLog.outError("BattleGround system: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_invitedAlliance, m_invitedHorde);

    m_invitedAlliance = 0;
    m_invitedHorde = 0;
    m_hasBgFreeSlotQueue = false;

    m_players.clear();

    for (BattleGroundScoreMap::const_iterator itr = m_playerScores.begin(); itr != m_playerScores.end(); ++itr)
        delete itr->second;
    m_playerScores.clear();
}

/**
  Method that starts the battleground
*/
void BattleGround::StartBattleGround()
{
    SetStartTime(0);

    // expects to be already added in free queue

    // add bg to update list
    // This must be done here, because we need to have already invited some players when first BG::Update() method is executed
    // and it doesn't matter if we call StartBattleGround() more times, because m_BattleGrounds is a map and instance id never changes
    sBattleGroundMgr.AddBattleGround(GetInstanceId(), GetTypeId(), this);
}

/**
  Method that is called when adding a player to the battleground

  @param    player
*/
void BattleGround::AddPlayer(Player* player)
{
    // remove afk from player
    if (player->isAFK())
        player->ToggleAFK();

    // score struct must be created in inherited class

    ObjectGuid guid = player->GetObjectGuid();
    Team team = player->GetBGTeam();

    BattleGroundPlayer bp;
    bp.offlineRemoveTime = 0;
    bp.playerTeam = team;

    // Add to list/maps
    m_players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(data, player);
    SendPacketToTeam(team, data, player, false);

    // setup BG group membership
    PlayerAddedToBgCheckIfBgIsRunning(player);
    AddOrSetPlayerToCorrectBgGroup(player, guid, team);

    // Log
    DETAIL_LOG("BATTLEGROUND: Player %s joined the battle.", player->GetName());
}

/* this method adds player to his team's bg group, or sets his correct group if player is already in bg group */
/**
  Method that handles adding players to the correct BG group

  @param    player
  @param    objectGuid
  @param    team
*/
void BattleGround::AddOrSetPlayerToCorrectBgGroup(Player* plr, ObjectGuid playerGuid, Team team)
{
    if (Group* group = GetBgRaid(team))                     // raid already exist
    {
        if (group->IsMember(playerGuid))
        {
            uint8 subgroup = group->GetMemberGroup(playerGuid);
            plr->SetBattleGroundRaid(group, subgroup);
        }
        else
        {
            group->AddMember(playerGuid, plr->GetName());
            if (Group* originalGroup = plr->GetOriginalGroup())
                if (originalGroup->IsLeader(playerGuid))
                    group->ChangeLeader(playerGuid);
        }
    }
    else                                                    // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(playerGuid, plr->GetName());
    }
}

/**
  Method that handles players logging out from ongoing battlegrounds

  @param    player
*/
void BattleGround::EventPlayerLoggedIn(Player* player)
{
    ObjectGuid playerGuid = player->GetObjectGuid();

    // player is correct pointer
    for (OfflineQueue::iterator itr = m_offlineQueue.begin(); itr != m_offlineQueue.end(); ++itr)
    {
        if (*itr == playerGuid)
        {
            m_offlineQueue.erase(itr);
            break;
        }
    }

    m_players[playerGuid].offlineRemoveTime = 0;
    PlayerAddedToBgCheckIfBgIsRunning(player);
    // if battleground is starting, then add preparation aura
    // we don't have to do that, because preparation aura isn't removed when player logs out
}

/**
  Method that handles players logging out from ongoing battlegrounds

  @param    player
*/
void BattleGround::EventPlayerLoggedOut(Player* player)
{
    // player is correct pointer, it is checked in WorldSession::LogoutPlayer()
    m_offlineQueue.push_back(player->GetObjectGuid());
    m_players[player->GetObjectGuid()].offlineRemoveTime = sWorld.GetGameTime() + MAX_OFFLINE_TIME;

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // drop flag and handle other cleanups
        RemovePlayer(player, player->GetObjectGuid());
    }
}

/**
  Function that returns the number of players that can join a battleground based on the provided team
*/
bool BattleGround::AddToBgFreeSlotQueue()
{
    // make sure to add only once
    if (!m_hasBgFreeSlotQueue)
    {
        m_hasBgFreeSlotQueue = true;
        BattleGroundInQueueInfo bgInfo;
        bgInfo.Fill(this);
        sWorld.GetBGQueue().GetMessager().AddMessage([bgInfo](BattleGroundQueue* queue)
        {
            queue->AddBgToFreeSlots(bgInfo);
        });
        return true;
    }
    return false;
}

/**
  Method that removes this battleground from free queue - it must be called when deleting battleground
*/
void BattleGround::RemovedFromBgFreeSlotQueue(bool removeFromQueue)
{
    // set to be able to re-add if needed
    if (m_hasBgFreeSlotQueue && removeFromQueue)
    {
        sWorld.GetBGQueue().GetMessager().AddMessage([bgTypeId = GetTypeId(), instanceId = GetInstanceId()](BattleGroundQueue* queue)
        {
            queue->RemoveBgFromFreeSlots(bgTypeId, instanceId);
        });
    }
    m_hasBgFreeSlotQueue = false;
}

void BattleGround::SetInvitedCount(Team team, uint32 count)
{
    if (team == ALLIANCE)
        m_invitedAlliance = count;
    else
        m_invitedHorde = count;
}

/**
  Function that returns the number of players that can join a battleground based on the provided team

  @param    team
*/
uint32 BattleGround::GetFreeSlotsForTeam(Team team) const
{
    // return free slot count to MaxPlayerPerTeam
    if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
        return (GetInvitedCount(team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(team) : 0;

    return 0;
}

/**
  Method that updates player score

  @param    player
  @param    type
  @param    value
*/
void BattleGround::UpdatePlayerScore(Player* player, uint32 type, uint32 value)
{
    // this procedure is called from virtual function implemented in bg subclass
    BattleGroundScoreMap::const_iterator itr = m_playerScores.find(player->GetObjectGuid());

    if (itr == m_playerScores.end())                        // player not found...
        return;

    switch (type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->killingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->honorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // reward honor instantly
            if (player->AddHonorCP(value, HONORABLE))
                itr->second->bonusHonor += value;
            break;
        case SCORE_DISHONORABLE_KILLS:                      // Dishonorable kills
            itr->second->dishonorableKills += value;
            break;
        default:
            sLog.outError("BattleGround: Unknown player score type %u", type);
            break;
    }
}

/**
  Method that handles door closing

  @param    gameobject guid
*/
void BattleGround::DoorClose(uint32 dbGuid)
{
    GameObject* obj = GetBgMap()->GetGameObject(dbGuid);
    if (obj)
    {
        // if doors are open, close it
        if (obj->GetLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            // change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
        sLog.outError("BattleGround: Door %u not found (cannot close doors)", dbGuid);
}

/**
  Method that handles opens doors

  @param    gameobject guid
*/
void BattleGround::DoorOpen(uint32 dbGuid)
{
    GameObject* obj = GetBgMap()->GetGameObject(dbGuid);
    if (obj)
    {
        // change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
        sLog.outError("BattleGround: Door %u not found! - doors will be closed.", dbGuid);
}

/**
  Function that returns the premature winner
*/
Team BattleGround::GetPrematureWinner()
{
    uint32 hordePlayers = GetPlayersCountByTeam(HORDE);
    uint32 alliancePlayers = GetPlayersCountByTeam(ALLIANCE);

    if (hordePlayers > alliancePlayers)
        return HORDE;

    if (alliancePlayers > hordePlayers)
        return ALLIANCE;

    return TEAM_NONE;
}

/**
  Function that handles creature load from DB event map

  @param    creature
*/
void BattleGround::OnObjectDBLoad(Creature* creature)
{
    const BattleGroundEventIdx eventId = GetBgMap()->GetMapDataContainer().GetCreatureEventIndex(creature->GetDbGuid());
    if (eventId.event1 == BG_EVENT_NONE)
        return;

    m_eventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].creatures.push_back(creature->GetDbGuid());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
        ChangeBgCreatureSpawnState(creature->GetDbGuid(), RESPAWN_ONE_DAY);
}

/**
  Function returns a creature guid from event map

  @param    event1
  @param    event2
*/
uint32 BattleGround::GetSingleCreatureGuid(uint8 event1, uint8 event2)
{
    auto itr = m_eventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    if (itr != m_eventObjects[MAKE_PAIR32(event1, event2)].creatures.end())
        return *itr;

    return ObjectGuid();
}

/**
  Function returns a gameobject guid from event map

  @param    event1
  @param    event2
*/
uint32 BattleGround::GetSingleGameObjectGuid(uint8 event1, uint8 event2)
{
    auto itr = m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    if (itr != m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end())
    {
        return *itr;
    }

    return ObjectGuid();
}

/**
  Method that handles gameobject load from DB event map

  @param    gameobject
*/
void BattleGround::OnObjectDBLoad(GameObject* obj)
{
    const BattleGroundEventIdx eventId = GetBgMap()->GetMapDataContainer().GetGameObjectEventIndex(obj->GetDbGuid());
    if (eventId.event1 == BG_EVENT_NONE)
        return;

    m_eventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].gameobjects.push_back(obj->GetDbGuid());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
        ChangeBgObjectSpawnState(obj->GetDbGuid(), RESPAWN_ONE_DAY);
    else
    {
        // it's possible, that doors aren't spawned anymore (wsg)
        if (GetStatus() >= STATUS_IN_PROGRESS && IsDoorEvent(eventId.event1, eventId.event2))
            DoorOpen(obj->GetObjectGuid());
    }
}

/**
  Function that checks if event handles doors

  @param    event1
  @param    event2
*/
bool BattleGround::IsDoorEvent(uint8 event1, uint8 event2) const
{
    if (event1 == BG_EVENT_DOOR)
    {
        if (event2 > 0)
        {
            sLog.outError("BattleGround too high event2 for event1:%i", event1);
            return false;
        }
        return true;
    }
    return false;
}

/**
  Method that handles door opening

  @param    event1
  @param    event2
*/
void BattleGround::OpenDoorEvent(uint8 event1, uint8 event2 /*=0*/)
{
    if (!IsDoorEvent(event1, event2))
    {
        sLog.outError("BattleGround:OpenDoorEvent this is no door event1:%u event2:%u", event1, event2);
        return;
    }
    if (!IsActiveEvent(event1, event2))                 // maybe already despawned (eye)
    {
        sLog.outError("BattleGround:OpenDoorEvent this event isn't active event1:%u event2:%u", event1, event2);
        return;
    }

    auto itr = m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for (; itr != m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr)
        DoorOpen(*itr);
}

/**
  Method that handles event spawn

  @param    event1
  @param    event2
  @param    spawn
*/
void BattleGround::SpawnEvent(uint8 event1, uint8 event2, bool spawn)
{
    // stop if we want to spawn something which was already spawned
    // or despawn something which was already despawned
    if (event2 == BG_EVENT_NONE || (spawn && m_activeEvents[event1] == event2)
            || (!spawn && m_activeEvents[event1] != event2))
        return;

    if (spawn)
    {
        // if event gets spawned, the current active event mus get despawned
        SpawnEvent(event1, m_activeEvents[event1], false);
        m_activeEvents[event1] = event2;                    // set this event to active
    }
    else
        m_activeEvents[event1] = BG_EVENT_NONE;             // no event active if event2 gets despawned

    auto itr = m_eventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    for (; itr != m_eventObjects[MAKE_PAIR32(event1, event2)].creatures.end(); ++itr)
        ChangeBgCreatureSpawnState(*itr, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);

    auto itr2 = m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for (; itr2 != m_eventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr2)
        ChangeBgObjectSpawnState(*itr2, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
}

/**
  Method that changes the battleground gameobject spawn state

  @param    gameobject guid
  @param    respawn time (can be 0 or 1 day)
*/
void BattleGround::ChangeBgObjectSpawnState(uint32 dbGuid, uint32 respawntime)
{
    Map* map = GetBgMap();

    GameObject* obj = map->GetGameObject(dbGuid);
    if (!obj)
    {
        map->GetSpawnManager().RespawnGameObject(dbGuid, respawntime);
        return;
    }

    if (respawntime == 0)
    {
        // we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
        if (obj->GetLootState() == GO_JUST_DEACTIVATED)
            obj->SetLootState(GO_READY);
        obj->Respawn();

        // if despawned by default, set respawn delay
        if (!obj->IsSpawnedByDefault())
        {
            obj->SetRespawnTime(RESPAWN_ONE_DAY);
            obj->Refresh();
        }
    }
    else
    {
        obj->SetRespawnDelay(respawntime);
        obj->SetForcedDespawn();
        obj->SetLootState(GO_JUST_DEACTIVATED);
    }
}

/**
  Method that changes the battleground creature spawn state

  @param    gameobject guid
  @param    respawn time (can be 0 or 1 day)
*/
void BattleGround::ChangeBgCreatureSpawnState(uint32 dbGuid, uint32 respawntime)
{
    Map* map = GetBgMap();

    Creature* obj = map->GetCreature(dbGuid);
    if (!obj)
    {
        map->GetSpawnManager().RespawnCreature(dbGuid, respawntime);
        return;
    }

    if (respawntime == 0)
    {
        obj->Respawn();
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->SetRespawnDelay(respawntime);
        obj->ForcedDespawn();
    }
}

/**
  Method that sends message to all players

  @param    text entry
  @param    chat type
  @param    player source
*/
void BattleGround::SendMessageToAll(int32 entry, ChatMsg type, Player const* source)
{
    MaNGOS::BattleGroundChatBuilder bg_builder(type, entry, source);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
  Method that sends yell to all players

  @param    text entry
  @param    language
  @param    creature source
*/
void BattleGround::SendYellToAll(int32 entry, uint32 language, Creature const* source)
{
    MaNGOS::BattleGroundYellBuilder bg_builder(Language(language), entry, source);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundYellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void BattleGround::PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...)
{
    va_list ap;
    va_start(ap, source);

    MaNGOS::BattleGroundChatBuilder bg_builder(type, entry, source, &ap);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);

    va_end(ap);
}

/**
  Method that sends message to all players

  @param    text entry
  @param    chat type
  @param    player source
  @param    arg1
  @param    arg2
*/
void BattleGround::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
    MaNGOS::BattleGround2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGround2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
  Method that sends yell to all players

  @param    text entry
  @param    language
  @param    creature source
  @param    arg1
  @param    arg2
*/
void BattleGround::SendYell2ToAll(int32 entry, uint32 language, Creature const* source, int32 arg1, int32 arg2)
{
    MaNGOS::BattleGround2YellBuilder bg_builder(Language(language), entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGround2YellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void BattleGround::SendBcdToAll(int32 bcdEntry, ChatMsg msgtype, Creature const* source)
{
    MaNGOS::BattleGroundBroadcastBuilder bg_builder(sObjectMgr.GetBroadcastText(bcdEntry), msgtype, source, nullptr);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundBroadcastBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void BattleGround::SendBcdToTeam(int32 bcdEntry, ChatMsg msgtype, Creature const* source, Team team)
{
    MaNGOS::BattleGroundBroadcastBuilder bg_builder(sObjectMgr.GetBroadcastText(bcdEntry), msgtype, source, nullptr);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundBroadcastBuilder> bg_do(bg_builder);
    auto lambda = [&](Player* player)
    {
        if (player->GetTeam() == team)
            bg_do(player);
    };
    BroadcastWorker(lambda);
}

/**
  Function that ends the battleground immediately
*/
void BattleGround::EndNow()
{
    RemovedFromBgFreeSlotQueue(true);
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(0);
}

/**
  Method that handles buff triggering based on trap gameobjects

  @param    gameobject guid
*/
void BattleGround::HandleTriggerBuff(ObjectGuid go_guid)
{
    GameObject* obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->IsSpawned())
        return;

    obj->SetLootState(GO_JUST_DEACTIVATED);             // can be despawned or destroyed
}

/**
  Method that handles killing players

  @param    victim player
  @param    killer player
*/
void BattleGround::HandleKillPlayer(Player* player, Player* killer)
{
    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if (killer)
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for (BattleGroundPlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
        {
            Player* plr = sObjectMgr.GetPlayer(itr->first);

            if (!plr || plr == killer)
                continue;

            if (plr->GetTeam() == killer->GetTeam() && plr->IsAtGroupRewardDistance(player))
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
        }
    }

    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

/**
  Function that returns the team of the given player - used mainly in arenas

  @param    player objectguid
*/
Team BattleGround::GetPlayerTeam(ObjectGuid guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_players.find(guid);
    if (itr != m_players.end())
        return itr->second.playerTeam;
    return TEAM_NONE;
}

/**
  Function that returns returns if player is in battleground

  @param    player objectguid
*/
bool BattleGround::IsPlayerInBattleGround(ObjectGuid guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_players.find(guid);
    return itr != m_players.end();
}

/**
  Method that handles player added to battleground when battleground is in progress

  @param    player
*/
void BattleGround::PlayerAddedToBgCheckIfBgIsRunning(Player* player)
{
    if (GetStatus() != STATUS_WAIT_LEAVE)
        return;

    WorldPacket data;
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(GetTypeId());

    BlockMovement(player);

    sBattleGroundMgr.BuildPvpLogDataPacket(data, this);
    player->GetSession()->SendPacket(data);

    sBattleGroundMgr.BuildBattleGroundStatusPacket(data, true, GetTypeId(), GetClientInstanceId(), GetMapId(), player->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, GetEndTime(), GetStartTime());
    player->GetSession()->SendPacket(data);
}

/**
  Function that counts the alive players by team

  @param    team
*/
uint32 BattleGround::GetAlivePlayersCountByTeam(Team team) const
{
    int count = 0;
    for (const auto& m_Player : m_players)
    {
        if (m_Player.second.playerTeam == team)
        {
            Player* pl = sObjectMgr.GetPlayer(m_Player.first);
            if (pl && pl->IsAlive() && pl->GetShapeshiftForm() != FORM_SPIRITOFREDEMPTION)
                ++count;
        }
    }
    return count;
}

/**
  Method that sets battleground group as raid

  @param    team
  @param    battleground raid
*/
void BattleGround::SetBgRaid(Team team, Group* bgRaid)
{
    Group*& oldRaid = m_bgRaids[GetTeamIndexByTeamId(team)];

    if (oldRaid)
        oldRaid->SetBattlegroundGroup(nullptr);

    if (bgRaid)
        bgRaid->SetBattlegroundGroup(this);

    oldRaid = bgRaid;
}

/**
  Function that returns the gameobject pointer that was stored in m_goEntryGuidStore. Can return nullptr

  @param    gameobject entry
*/
GameObject* BattleGround::GetSingleGameObjectFromStorage(uint32 entry) const
{
    auto iter = m_goEntryGuidStore.find(entry);
    if (iter != m_goEntryGuidStore.end())
        return GetBgMap()->GetGameObject(iter->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    sLog.outError("BattleGround requested gameobject with entry %u, but no gameobject of this entry was created yet, or it was not stored by battleground script for map %u.", entry, GetBgMap()->GetId());

    return nullptr;
}

/**
  Function that returns pointer to a loaded Creature that was stored in m_goEntryGuidStore. Can return nullptr

  @param    creature entry
  @param    skip debug log
*/
Creature* BattleGround::GetSingleCreatureFromStorage(uint32 entry, bool skipDebugLog /*=false*/) const
{
    auto iter = m_npcEntryGuidStore.find(entry);
    if (iter != m_npcEntryGuidStore.end())
        return GetBgMap()->GetCreature(iter->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    if (!skipDebugLog)
        script_error_log("BattleGround requested creature with entry %u, but no npc of this entry was created yet, or it was not stored by script for map %u.", entry, GetBgMap()->GetId());

    return nullptr;
}
