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

#ifndef __BATTLEGROUNDWS_H
#define __BATTLEGROUNDWS_H

#include "BattleGround.h"
#include "Tools/Language.h"

class BattleGround;

enum WSTimers
{
    BG_WS_MAX_TEAM_SCORE                    = 3,
    BG_WS_FLAG_RESPAWN_TIME                 = 23 * IN_MILLISECONDS,
    BG_WS_FLAG_DROP_TIME                    = 10 * IN_MILLISECONDS,
};

enum WSVariables
{
    BG_WS_NORMAL_FLAG_CAPTURE_REPUTATION    = 35,
    BG_WS_WEEKEND_FLAG_CAPTURE_REPUTATION   = 45,
    BG_WS_NORMAL_WIN_KILLS                  = 1,
    BG_WS_WEEKEND_WIN_KILLS                 = 3,
    BG_WS_NORMAL_MAP_COMPLETE_KILLS         = 2,
    BG_WS_WEEKEND_MAP_COMPLETE_KILLS        = 4
};

enum WSSounds
{
    BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE  = 8173,
    BG_WS_SOUND_FLAG_CAPTURED_HORDE     = 8213,
    BG_WS_SOUND_FLAG_PLACED             = 8232,
    BG_WS_SOUND_FLAG_RETURNED           = 8192,
    BG_WS_SOUND_HORDE_FLAG_PICKED_UP    = 8212,
    BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP = 8174,
    BG_WS_SOUND_FLAGS_RESPAWNED         = 8232
};

enum WSSpells
{
    BG_WS_SPELL_WARSONG_FLAG            = 23333,
    BG_WS_SPELL_WARSONG_FLAG_DROPPED    = 23334,
    BG_WS_SPELL_SILVERWING_FLAG         = 23335,
    BG_WS_SPELL_SILVERWING_FLAG_DROPPED = 23336
};

enum WSWorldStates
{
    BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_ALLIANCE = 1545,
    BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_HORDE    = 1546,
//  FLAG_UNK                            = 1547,

    BG_WS_STATE_CAPTURES_ALLIANCE       = 1581,             // count the flag captures for each team
    BG_WS_STATE_CAPTURES_HORDE          = 1582,
    BG_WS_STATE_CAPTURES_MAX            = 1601,             // shows the max allowed flags

    BG_WS_STATE_FLAG_HORDE              = 2338,             // shows who captured the current flag
    BG_WS_STATE_FLAG_ALLIANCE           = 2339,
};

enum WSFlagActions
{
    BG_WS_FLAG_ACTION_NONE              = -1,
    BG_WS_FLAG_ACTION_PICKEDUP          = 0,
    BG_WS_FLAG_ACTION_RETURNED          = 1,
    BG_WS_FLAG_ACTION_DROPPED           = 2,
    BG_WS_FLAG_ACTION_CAPTURED          = 3,
    BG_WS_FLAG_ACTION_RESPAWN           = 4,

    BG_WS_FLAG_ACTIONS_TOTAL            = 5,
};

enum WSPickedUpFlagStates
{
    BG_WS_FLAG_STATE_ON_BASE            = 0,
    BG_WS_FLAG_STATE_ON_PLAYER          = 1,
    BG_WS_FLAG_STATE_ON_GROUND          = -1
};

enum WSFlagIconStates
{
    BG_WS_FLAG_ICON_INACTIVE            = 0,
    BG_WS_FLAG_ICON_INVISIBLE           = 1,
    BG_WS_FLAG_ICON_VISIBLE             = 2,
};

enum WSGraveyards
{
    WS_GRAVEYARD_FLAGROOM_ALLIANCE      = 769,
    WS_GRAVEYARD_FLAGROOM_HORDE         = 770,
    WS_GRAVEYARD_MAIN_ALLIANCE          = 771,
    WS_GRAVEYARD_MAIN_HORDE             = 772,

    BG_WS_ZONE_ID_MAIN                  = 3277,
};

enum WSEventIds
{
    WS_EVENT_ALLIANCE_FLAG_PICKUP       = 8504,             // triggered from flag events - source player, target go
    WS_EVENT_HORDE_FLAG_PICKUP          = 8505,

    WS_EVENT_ALLIANCE_FLAG_DROP         = 8506,             // not used; events used for flag handling; triggered from spell
    WS_EVENT_HORDE_FLAG_DROP            = 8507,

    WS_EVENT_ALLIANCE_FLAG_DROPPED_PICKUP = 8623,
    WS_EVENT_HORDE_FLAG_DROPPED_PICKUP  = 8624,
};

enum WSGameObjects
{
    GO_WS_SILVERWING_FLAG               = 179830,
    GO_WS_WARSONG_FLAG                  = 179831,

    GO_WS_SILVERWING_FLAG_DROP          = 179785,           // temp summoned objects when main flag is dropped
    GO_WS_WARSONG_FLAG_DROP             = 179786,
};

enum WSAreaTriggers
{
    WS_AT_SILVERWING_ROOM               = 3646,
    WS_AT_WARSONG_ROOM                  = 3647,
};

enum WSScriptEvents
{
    WS_EVENT_FLAG_A                     = 0,
    WS_EVENT_FLAG_H                     = 1,
    // spiritguides will spawn (same moment, like WS_EVENT_DOOR_OPEN)
    WS_EVENT_SPIRITGUIDES_SPAWN         = 2
};

static const uint32 wsFlagPickedUp[PVP_TEAM_COUNT] = { BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_ALLIANCE, BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_HORDE };
static const uint32 wsFlagHUDPickedUp[PVP_TEAM_COUNT] = { BG_WS_STATE_FLAG_ALLIANCE, BG_WS_STATE_FLAG_HORDE };

static const uint32 wsDroppedFlagId[PVP_TEAM_COUNT] = { GO_WS_SILVERWING_FLAG_DROP, GO_WS_WARSONG_FLAG_DROP };

struct WarsongData
{
    uint8 flagAction;
    uint32 messageId, soundId, spellId;
    ChatMsg chatType;
};

// *** Battleground flag data *** //
static const WarsongData wsgFlagData[PVP_TEAM_COUNT][BG_WS_FLAG_ACTIONS_TOTAL] =
{
    {
        {BG_WS_FLAG_ACTION_PICKEDUP, LANG_BG_WS_PICKEDUP_AF,             BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP, BG_WS_SPELL_SILVERWING_FLAG,         CHAT_MSG_BG_SYSTEM_ALLIANCE},
        {BG_WS_FLAG_ACTION_RETURNED, LANG_BG_WS_RETURNED_AF,             BG_WS_SOUND_FLAG_RETURNED,           BG_WS_SPELL_SILVERWING_FLAG,         CHAT_MSG_BG_SYSTEM_ALLIANCE},
        {BG_WS_FLAG_ACTION_DROPPED,  LANG_BG_WS_DROPPED_AF,              0,                                   BG_WS_SPELL_SILVERWING_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_ALLIANCE},
        {BG_WS_FLAG_ACTION_CAPTURED, LANG_BG_WS_CAPTURED_AF,             BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE,  0,                                   CHAT_MSG_BG_SYSTEM_ALLIANCE},
        {BG_WS_FLAG_ACTION_RESPAWN,  LANG_BG_WS_ALLIANCE_FLAG_RESPAWNED, BG_WS_SOUND_FLAGS_RESPAWNED,         0,                                   CHAT_MSG_BG_SYSTEM_NEUTRAL},
    },
    {
        {BG_WS_FLAG_ACTION_PICKEDUP, LANG_BG_WS_PICKEDUP_HF,          BG_WS_SOUND_HORDE_FLAG_PICKED_UP, BG_WS_SPELL_WARSONG_FLAG,         CHAT_MSG_BG_SYSTEM_HORDE},
        {BG_WS_FLAG_ACTION_RETURNED, LANG_BG_WS_RETURNED_HF,          BG_WS_SOUND_FLAG_RETURNED,        BG_WS_SPELL_WARSONG_FLAG,         CHAT_MSG_BG_SYSTEM_HORDE},
        {BG_WS_FLAG_ACTION_DROPPED,  LANG_BG_WS_DROPPED_HF,           0,                                BG_WS_SPELL_WARSONG_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_HORDE},
        {BG_WS_FLAG_ACTION_CAPTURED, LANG_BG_WS_CAPTURED_HF,          BG_WS_SOUND_FLAG_CAPTURED_HORDE,  0,                                CHAT_MSG_BG_SYSTEM_HORDE},
        {BG_WS_FLAG_ACTION_RESPAWN,  LANG_BG_WS_HORDE_FLAG_RESPAWNED, BG_WS_SOUND_FLAGS_RESPAWNED,      0,                                CHAT_MSG_BG_SYSTEM_NEUTRAL},
    }
};

class BattleGroundWGScore : public BattleGroundScore
{
    public:
        BattleGroundWGScore() : flagCaptures(0), flagReturns(0) {};
        virtual ~BattleGroundWGScore() {};

        uint32 GetAttr1() const override { return flagCaptures; }
        uint32 GetAttr2() const override { return flagReturns; }

        uint32 flagCaptures;
        uint32 flagReturns;
};

// Honor granted depending on player's level
const uint32 BG_WSG_FlagCapturedHonor[MAX_BATTLEGROUND_BRACKETS] = {48, 82, 136, 226, 378, 396};
const uint32 BG_WSG_WinMatchHonor[MAX_BATTLEGROUND_BRACKETS] = {24, 41, 68, 113, 189, 198};

class BattleGroundWS : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        BattleGroundWS();
        void Reset() override;
        void Update(uint32 diff) override;

        // Main battleground functions
        void AddPlayer(Player* player) override;
        void RemovePlayer(Player* player, ObjectGuid guid) override;
        void StartingEventOpenDoors() override;
        void EndBattleGround(Team winner) override;

        // General functions
        void UpdatePlayerScore(Player* source, uint32 type, uint32 value) override;
        Team GetPrematureWinner() override;

        // Battleground event handlers
        bool HandleEvent(uint32 eventId, Object* source, Object* target) override;
        bool HandleAreaTrigger(Player* source, uint32 trigger) override;
        void HandleGameObjectCreate(GameObject* go) override;
        void HandleKillPlayer(Player* player, Player* killer) override;
        void HandlePlayerClickedOnFlag(Player* source, GameObject* go) override;
        void HandlePlayerDroppedFlag(Player* source) override;

        // Flag handler
        ObjectGuid const& GetFlagCarrierGuid(uint8 teamIdx) const { return m_flagCarrier[teamIdx]; }

    private:

        // Flag Carrier functions
        void SetFlagCarrier(uint8 teamIdx, ObjectGuid guid) { m_flagCarrier[teamIdx] = guid; }
        void ClearFlagCarrier(uint8 teamIdx) { m_flagCarrier[teamIdx].Clear(); }
        bool IsFlagPickedUp(uint8 teamIdx) const { return !m_flagCarrier[teamIdx].IsEmpty(); }

        // Flag interactions
        void ClearDroppedFlagGuid(Team team)  { m_droppedFlagGuid[GetTeamIndexByTeamId(team)].Clear();}
        ObjectGuid const& GetDroppedFlagGuid(Team team) const { return m_droppedFlagGuid[GetTeamIndexByTeamId(team)];}

        void RespawnFlagAtBase(Team team, bool wasCaptured);
        void RespawnDroppedFlag(Team team);
        int32 GetFlagState(Team team);

        void ProcessFlagPickUpFromBase(Player* player, Team attackerTeam);
        void ProcessDroppedFlagActions(Player* player, GameObject* target);

        // process score
        void ProcessPlayerFlagScoreEvent(Player* source);

        ObjectGuid m_droppedFlagGuid[PVP_TEAM_COUNT];
        ObjectGuid m_flagCarrier[PVP_TEAM_COUNT];

        bool m_flagOnRespawn[PVP_TEAM_COUNT];
        uint32 m_flagsTimer[PVP_TEAM_COUNT];
        uint32 m_flagsDropTimer[PVP_TEAM_COUNT];

        uint32 m_reputationCapture;
        uint32 m_honorWinKills;
        uint32 m_honorEndKills;
        Team   m_lastCapturedFlagTeam;
};
#endif
