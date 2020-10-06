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
    BG_WS_STATE_FLAG_UNK_ALLIANCE       = 1545,
    BG_WS_STATE_FLAG_UNK_HORDE          = 1546,
//  FLAG_UNK                            = 1547,
    BG_WS_STATE_CAPTURES_ALLIANCE       = 1581,
    BG_WS_STATE_CAPTURES_HORDE          = 1582,
    BG_WS_STATE_CAPTURES_MAX            = 1601,
    BG_WS_STATE_FLAG_HORDE              = 2338,
    BG_WS_STATE_FLAG_ALLIANCE           = 2339
};

enum WSFlagActions
{
    BG_WS_FLAG_ACTION_NONE     = -1,
    BG_WS_FLAG_ACTION_PICKEDUP = 0,
    BG_WS_FLAG_ACTION_RETURNED = 1,
    BG_WS_FLAG_ACTION_DROPPED  = 2,
    BG_WS_FLAG_ACTION_CAPTURED = 3,
    BG_WS_FLAG_ACTION_RESPAWN  = 4,

    BG_WS_FLAG_ACTIONS_TOTAL   = 5,
};

enum WSFlagStates
{
    BG_WS_FLAG_STATE_ON_BASE      = 0,
    BG_WS_FLAG_STATE_WAIT_RESPAWN = 1,
    BG_WS_FLAG_STATE_ON_PLAYER    = 2,
    BG_WS_FLAG_STATE_ON_GROUND    = 3
};

enum WSGraveyards
{
    WS_GRAVEYARD_FLAGROOM_ALLIANCE = 769,
    WS_GRAVEYARD_FLAGROOM_HORDE    = 770,
    WS_GRAVEYARD_MAIN_ALLIANCE     = 771,
    WS_GRAVEYARD_MAIN_HORDE        = 772
};

enum WSEventIds
{
    WS_EVENT_ALLIANCE_FLAG_DROP     = 8506,             // events used for flag handling
    WS_EVENT_HORDE_FLAG_DROP        = 8507,

    WS_EVENT_ALLIACE_FLAG_PICKUP    = 8623,
    WS_EVENT_HORDE_FLAG_PICKUP      = 8624,
};

enum WSScriptEvents
{
    WS_EVENT_FLAG_A             = 0,
    WS_EVENT_FLAG_H             = 1,
    // spiritguides will spawn (same moment, like WS_EVENT_DOOR_OPEN)
    WS_EVENT_SPIRITGUIDES_SPAWN = 2
};

static const uint32 wsStateUpdateId[PVP_TEAM_COUNT] = { BG_WS_STATE_FLAG_UNK_ALLIANCE, BG_WS_STATE_FLAG_UNK_HORDE };

static const uint32 wsFlagIds[PVP_TEAM_COUNT] = {
    5912, // TEAM_INDEX_ALLIANCE
    5913  // TEAM_INDEX_HORDE
};

static const uint32 wsMessageIds[PVP_TEAM_COUNT][BG_WS_FLAG_ACTIONS_TOTAL] = {
    // TEAM_INDEX_ALLIANCE
    {
        LANG_BG_WS_PICKEDUP_AF,            // BG_WS_FLAG_ACTION_PICKEDUP
        LANG_BG_WS_RETURNED_AF,            // BG_WS_FLAG_ACTION_RETURNED
        LANG_BG_WS_DROPPED_AF,             // BG_WS_FLAG_ACTION_DROPPED
        LANG_BG_WS_CAPTURED_AF,            // BG_WS_FLAG_ACTION_CAPTURED
        LANG_BG_WS_ALLIANCE_FLAG_RESPAWNED // BG_WS_FLAG_ACTION_RESPAWN
    },
    // TEAM_INDEX_HORDE
    {
        LANG_BG_WS_PICKEDUP_HF,            // BG_WS_FLAG_ACTION_PICKEDUP
        LANG_BG_WS_RETURNED_HF,            // BG_WS_FLAG_ACTION_RETURNED
        LANG_BG_WS_DROPPED_HF,             // BG_WS_FLAG_ACTION_DROPPED
        LANG_BG_WS_CAPTURED_HF,            // BG_WS_FLAG_ACTION_CAPTURED
        LANG_BG_WS_HORDE_FLAG_RESPAWNED    // BG_WS_FLAG_ACTION_RESPAWN
    }
};

static const ChatMsg wsChatMessageTypes[PVP_TEAM_COUNT][BG_WS_FLAG_ACTIONS_TOTAL] =
{
    // TEAM_INDEX_ALLIANCE
    {
        CHAT_MSG_BG_SYSTEM_ALLIANCE, // BG_WS_FLAG_ACTION_PICKEDUP
        CHAT_MSG_BG_SYSTEM_ALLIANCE, // BG_WS_FLAG_ACTION_RETURNED
        CHAT_MSG_BG_SYSTEM_ALLIANCE, // BG_WS_FLAG_ACTION_DROPPED
        CHAT_MSG_BG_SYSTEM_ALLIANCE, // BG_WS_FLAG_ACTION_CAPTURED
        CHAT_MSG_BG_SYSTEM_NEUTRAL   // BG_WS_FLAG_ACTION_RESPAWN
    },
    // TEAM_INDEX_HORDE
    {
        CHAT_MSG_BG_SYSTEM_HORDE,    // BG_WS_FLAG_ACTION_PICKEDUP
        CHAT_MSG_BG_SYSTEM_HORDE,    // BG_WS_FLAG_ACTION_RETURNED
        CHAT_MSG_BG_SYSTEM_HORDE,    // BG_WS_FLAG_ACTION_DROPPED
        CHAT_MSG_BG_SYSTEM_HORDE,    // BG_WS_FLAG_ACTION_CAPTURED
        CHAT_MSG_BG_SYSTEM_NEUTRAL   // BG_WS_FLAG_ACTION_RESPAWN
    }
};

static const uint32 wsSounds[PVP_TEAM_COUNT][BG_WS_FLAG_ACTIONS_TOTAL] =
{
    // TEAM_INDEX_ALLIANCE
    {
        BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP, // BG_WS_FLAG_ACTION_PICKEDUP
        BG_WS_SOUND_FLAG_RETURNED,           // BG_WS_FLAG_ACTION_RETURNED
        //BG_WS_SOUND_FLAG_RETURNED,         // BG_WS_FLAG_ACTION_DROPPED
        BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE,  // BG_WS_FLAG_ACTION_CAPTURED
        BG_WS_SOUND_FLAGS_RESPAWNED          // BG_WS_FLAG_ACTION_RESPAWN
    },
    // TEAM_INDEX_HORDE
    {
        BG_WS_SOUND_HORDE_FLAG_PICKED_UP,    // BG_WS_FLAG_ACTION_PICKEDUP
        BG_WS_SOUND_FLAG_RETURNED,           // BG_WS_FLAG_ACTION_RETURNED
        //BG_WS_SOUND_FLAG_RETURNED,         // BG_WS_FLAG_ACTION_DROPPED
        BG_WS_SOUND_FLAG_CAPTURED_HORDE,     // BG_WS_FLAG_ACTION_CAPTURED
        BG_WS_SOUND_FLAGS_RESPAWNED          // BG_WS_FLAG_ACTION_RESPAWN
    }
};

static const uint32 wsSpellTypes[PVP_TEAM_COUNT][BG_WS_FLAG_ACTIONS_TOTAL] =
{
    // TEAM_INDEX_ALLIANCE
    {
        BG_WS_SPELL_SILVERWING_FLAG,         // BG_WS_FLAG_ACTION_PICKEDUP
        BG_WS_SPELL_SILVERWING_FLAG,         // BG_WS_FLAG_ACTION_RETURNED
        BG_WS_SPELL_SILVERWING_FLAG_DROPPED, // BG_WS_FLAG_ACTION_DROPPED
        //BG_WS_SPELL_SILVERWING_FLAG,       // BG_WS_FLAG_ACTION_CAPTURED
        //BG_WS_SPELL_SILVERWING_FLAG        // BG_WS_FLAG_ACTION_RESPAWN
    },
    // TEAM_INDEX_HORDE
    {
        BG_WS_SPELL_WARSONG_FLAG,            // BG_WS_FLAG_ACTION_PICKEDUP
        BG_WS_SPELL_WARSONG_FLAG,            // BG_WS_FLAG_ACTION_RETURNED
        BG_WS_SPELL_WARSONG_FLAG_DROPPED,    // BG_WS_FLAG_ACTION_DROPPED
        //BG_WS_SPELL_WARSONG_FLAG,          // BG_WS_FLAG_ACTION_CAPTURED
        //BG_WS_SPELL_WARSONG_FLAG           // BG_WS_FLAG_ACTION_RESPAWN
    }
};

class BattleGroundWGScore : public BattleGroundScore
{
    public:
        BattleGroundWGScore() : flagCaptures(0), flagReturns(0) {};
        virtual ~BattleGroundWGScore() {};

        uint32 GetAttr1() const { return flagCaptures; }
        uint32 GetAttr2() const { return flagReturns; }

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
        /* Construction */
        BattleGroundWS();
        void Update(uint32 diff) override;

        /* inherited from BattlegroundClass */
        void AddPlayer(Player* player) override;
        void StartingEventOpenDoors() override;

        /* BG Flags */
        ObjectGuid GetFlagCarrierGuid(uint8 teamIdx) const { return m_flagCarrier[teamIdx]; }
        void SetFlagCarrier(uint8 teamIdx, ObjectGuid guid) { m_flagCarrier[teamIdx] = guid; }
        void ClearFlagCarrier(uint8 teamIdx) { m_flagCarrier[teamIdx].Clear(); }
        bool IsFlagPickedUp(uint8 teamIdx) const { return !m_flagCarrier[teamIdx].IsEmpty(); }

        void RespawnFlag(Team team, bool captured);
        void RespawnDroppedFlag(Team team);
        uint8 GetFlagState(Team team) { return m_flagState[GetTeamIndexByTeamId(team)]; }

        /* Battleground Events */
        void EventPlayerDroppedFlag(Player* source) override;
        void EventPlayerClickedOnFlag(Player* source, GameObject* go) override;
        void EventPlayerCapturedFlag(Player* source) override;

        void RemovePlayer(Player* player, ObjectGuid guid) override;
        bool HandleAreaTrigger(Player* source, uint32 trigger) override;
        void HandleKillPlayer(Player* player, Player* killer) override;
        void Reset() override;
        void EndBattleGround(Team winner) override;

        WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;

        // Flag interactions
        void PickUpFlagFromBase(Player* source);
        uint32 GroundFlagInteraction(Player* source, GameObject* target);

        void UpdateFlagState(Team team, uint32 value);
        void UpdateTeamScore(Team team);
        void UpdatePlayerScore(Player* source, uint32 type, uint32 value) override;
        void SetDroppedFlagGuid(ObjectGuid guid, Team team)  { m_droppedFlagGuid[GetTeamIndexByTeamId(team)] = guid;}
        void ClearDroppedFlagGuid(Team team)  { m_droppedFlagGuid[GetTeamIndexByTeamId(team)].Clear();}
        ObjectGuid const& GetDroppedFlagGuid(Team team) const { return m_droppedFlagGuid[GetTeamIndexByTeamId(team)];}
        void FillInitialWorldStates(WorldPacket& data, uint32& count) override;
        Team GetPrematureWinner() override;

    private:
        ObjectGuid m_droppedFlagGuid[PVP_TEAM_COUNT];
        ObjectGuid m_flagCarrier[PVP_TEAM_COUNT];

        uint8 m_flagState[PVP_TEAM_COUNT];
        uint32 m_flagsTimer[PVP_TEAM_COUNT];
        uint32 m_flagsDropTimer[PVP_TEAM_COUNT];

        uint32 m_reputationCapture;
        uint32 m_honorWinKills;
        uint32 m_honorEndKills;
        Team   m_lastCapturedFlagTeam;
};
#endif
