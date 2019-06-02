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
#include "BattleGroundWS.h"
#include "Entities/GameObject.h"
#include "Globals/ObjectMgr.h"
#include "BattleGroundMgr.h"
#include "WorldPacket.h"
#include "Tools/Language.h"

static const uint32 wsFlagIds[PVP_TEAM_COUNT] = {
    5912, // TEAM_INDEX_ALLIANCE
    5913  // TEAM_INDEX_HORDE
};

static const uint32 wsMessageIds[PVP_TEAM_COUNT][WS_FLAG_ACTIONS_TOTAL] = {
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

static const ChatMsg wsChatMessageTypes[PVP_TEAM_COUNT][WS_FLAG_ACTIONS_TOTAL] =
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

static const uint32 wsSounds[PVP_TEAM_COUNT][WS_FLAG_ACTIONS_TOTAL] =
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
static const uint32 wsSpellTypes[PVP_TEAM_COUNT][WS_FLAG_ACTIONS_TOTAL] =
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

static const uint32 wsStateUpdateId[PVP_TEAM_COUNT] = {
    BG_WS_FLAG_UNK_ALLIANCE,
    BG_WS_FLAG_UNK_HORDE
};

BattleGroundWS::BattleGroundWS(): m_ReputationCapture(0), m_HonorWinKills(0), m_HonorEndKills(0)
{
    m_StartMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;
}

void BattleGroundWS::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (m_FlagState[TEAM_INDEX_ALLIANCE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            m_FlagsTimer[TEAM_INDEX_ALLIANCE] -= diff;

            if (m_FlagsTimer[TEAM_INDEX_ALLIANCE] < 0)
            {
                m_FlagsTimer[TEAM_INDEX_ALLIANCE] = 0;
                RespawnFlag(ALLIANCE, true);
            }
        }
        else if (m_FlagState[TEAM_INDEX_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            m_FlagsDropTimer[TEAM_INDEX_ALLIANCE] -= diff;

            if (m_FlagsDropTimer[TEAM_INDEX_ALLIANCE] < 0)
            {
                m_FlagsDropTimer[TEAM_INDEX_ALLIANCE] = 0;
                RespawnDroppedFlag(ALLIANCE);
            }
        }
        
        if (m_FlagState[TEAM_INDEX_HORDE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            m_FlagsTimer[TEAM_INDEX_HORDE] -= diff;

            if (m_FlagsTimer[TEAM_INDEX_HORDE] < 0)
            {
                m_FlagsTimer[TEAM_INDEX_HORDE] = 0;
                RespawnFlag(HORDE, true);
            }
        }
        else if (m_FlagState[TEAM_INDEX_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            m_FlagsDropTimer[TEAM_INDEX_HORDE] -= diff;

            if (m_FlagsDropTimer[TEAM_INDEX_HORDE] < 0)
            {
                m_FlagsDropTimer[TEAM_INDEX_HORDE] = 0;
                RespawnDroppedFlag(HORDE);
            }
        }
    }
}

void BattleGroundWS::StartingEventOpenDoors()
{
    OpenDoorEvent(BG_EVENT_DOOR);

    // TODO implement timer to despawn doors after a short while

    SpawnEvent(WS_EVENT_SPIRITGUIDES_SPAWN, 0, true);
    SpawnEvent(WS_EVENT_FLAG_A, 0, true);
    SpawnEvent(WS_EVENT_FLAG_H, 0, true);
}

void BattleGroundWS::AddPlayer(Player* plr)
{
    BattleGround::AddPlayer(plr);

    // create score and add it to map, default values are set in constructor
    BattleGroundWGScore* sc = new BattleGroundWGScore;

    m_PlayerScores[plr->GetObjectGuid()] = sc;
}

void BattleGroundWS::RespawnFlag(Team team, bool captured)
{
    if (team == ALLIANCE)
        DEBUG_LOG("Respawn Alliance flag");
    else
        DEBUG_LOG("Respawn Horde flag");

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    m_FlagState[teamIdx] = BG_WS_FLAG_STATE_ON_BASE;
    SpawnEvent(teamIdx, 0, true);

    if (captured)
    {
        // when map_update will be allowed for battlegrounds this code will be useless
        SpawnEvent(WS_EVENT_FLAG_A, 0, true);
        SpawnEvent(WS_EVENT_FLAG_H, 0, true);
        SendMessageToAll(LANG_BG_WS_F_PLACED, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);        // flag respawned sound...
    }
}

void BattleGroundWS::RespawnDroppedFlag(Team team)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    RespawnFlag(team, false);

    SendMessageToAll(wsMessageIds[teamIdx][BG_WS_FLAG_ACTION_RESPAWN],
        wsChatMessageTypes[teamIdx][BG_WS_FLAG_ACTION_RESPAWN]);

    PlaySoundToAll(wsSounds[teamIdx][BG_WS_FLAG_ACTION_RESPAWN]);

    GameObject* obj = GetBgMap()->GetGameObject(GetDroppedFlagGuid(team));
    if (obj)
        obj->Delete();
    else
        sLog.outError("Unknown dropped flag bg: %s", GetDroppedFlagGuid(team).GetString().c_str());

    ClearDroppedFlagGuid(team);
}

void BattleGroundWS::EventPlayerCapturedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    uint8 otherTeamIdx = GetOtherTeamIndex(teamIdx);

    if (!IsFlagPickedUp(otherTeamIdx))
        return;

    ClearFlagCarrier(otherTeamIdx); // must be before aura remove to prevent 2 events (drop+capture) at the same time

    // Horde flag in base (but not respawned yet)
    m_FlagState[otherTeamIdx] = BG_WS_FLAG_STATE_WAIT_RESPAWN;
    // Drop Horde Flag from Player
    player->RemoveAurasDueToSpell(wsSpellTypes[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP]);

    if (m_TeamScores[teamIdx] < BG_WS_MAX_TEAM_SCORE)
        m_TeamScores[teamIdx] += 1;
    
    PlaySoundToAll(wsSounds[teamIdx][BG_WS_FLAG_ACTION_CAPTURED]);
    RewardReputationToTeam(890, m_ReputationCapture, team);

    // for flag capture is reward 2 honorable kills
    RewardHonorToTeam(GetBonusHonorFromKill(2), team);

    // despawn flags
    SpawnEvent(WS_EVENT_FLAG_A, 0, false);
    SpawnEvent(WS_EVENT_FLAG_H, 0, false);

    SendMessageToAll(wsMessageIds[otherTeamIdx][BG_WS_FLAG_ACTION_CAPTURED],
                     wsChatMessageTypes[teamIdx][BG_WS_FLAG_ACTION_CAPTURED], player);

    UpdateFlagState(team, 1);                  // flag state none
    UpdateTeamScore(team);
    // only flag capture should be updated
    UpdatePlayerScore(player, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures

    Team winner = TEAM_NONE;
    if (m_TeamScores[TEAM_INDEX_ALLIANCE] == BG_WS_MAX_TEAM_SCORE)
        winner = ALLIANCE;
    else if (m_TeamScores[TEAM_INDEX_HORDE] == BG_WS_MAX_TEAM_SCORE)
        winner = HORDE;

    if (winner == ALLIANCE || winner == HORDE)
    {
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 0);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 0);
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, 1);
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, 1);

        EndBattleGround(winner);
    }
    else
        m_FlagsTimer[otherTeamIdx] = BG_WS_FLAG_RESPAWN_TIME;
}

void BattleGroundWS::EventPlayerDroppedFlag(Player* player)
{
    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        if (IsFlagPickedUp(teamIdx) && GetFlagCarrierGuid(teamIdx) == player->GetObjectGuid())
        {
            ClearFlagCarrier(teamIdx);
            player->RemoveAurasDueToSpell(wsSpellTypes[teamIdx][BG_WS_FLAG_ACTION_PICKEDUP]);
        }
    }
    else if (IsFlagPickedUp(otherTeamIdx))
    {
        if (GetFlagCarrierGuid(otherTeamIdx) == player->GetObjectGuid())
        {
            ClearFlagCarrier(otherTeamIdx);
            player->RemoveAurasDueToSpell(wsSpellTypes[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP]);
            m_FlagState[otherTeamIdx] = BG_WS_FLAG_STATE_ON_GROUND;
            player->CastSpell(player, wsSpellTypes[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED], TRIGGERED_OLD_TRIGGERED);
        }

        UpdateFlagState(team, 1);

        SendMessageToAll(wsMessageIds[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED],
                         wsChatMessageTypes[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED], player);
        UpdateWorldState(wsStateUpdateId[otherTeamIdx], uint32(-1));
        m_FlagsDropTimer[otherTeamIdx] = BG_WS_FLAG_DROP_TIME;
    }
}

void BattleGroundWS::PickUpFlagFromBase(Player* player)
{
    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    PlaySoundToAll(wsSounds[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP]);
    SpawnEvent(otherTeamIdx, 0, false);
    SetFlagCarrier(otherTeamIdx, player->GetObjectGuid());
    m_FlagState[otherTeamIdx] = BG_WS_FLAG_STATE_ON_PLAYER;

    // update world state to show correct flag carrier
    UpdateFlagState(team, BG_WS_FLAG_STATE_ON_PLAYER);
    UpdateWorldState(wsStateUpdateId[otherTeamIdx], 1);

    player->CastSpell(player, wsSpellTypes[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP], TRIGGERED_OLD_TRIGGERED);
    SendMessageToAll(wsMessageIds[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP],
                     wsChatMessageTypes[teamIdx][BG_WS_FLAG_ACTION_PICKEDUP], player);
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

uint32 BattleGroundWS::GroundFlagInteraction(Player* player, GameObject* target)
{
    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    int32 actionId = BG_WS_FLAG_ACTION_NONE;
    uint32 displayId = target->GetDisplayId();

    // check if we are returning our flag
    if (wsFlagIds[teamIdx] == displayId)
    {
        actionId = BG_WS_FLAG_ACTION_RETURNED;
        UpdateFlagState(GetOtherTeam(team), BG_WS_FLAG_STATE_WAIT_RESPAWN);
        RespawnFlag(team, false);
        UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);
        SendMessageToAll(wsMessageIds[teamIdx][actionId], wsChatMessageTypes[teamIdx][actionId], player);
        PlaySoundToAll(wsSounds[teamIdx][actionId]);
    }
    // check if we are picking up enemy flag
    else if (wsFlagIds[otherTeamIdx] == displayId)
    {
        actionId = BG_WS_FLAG_ACTION_PICKEDUP;
        SpawnEvent(otherTeamIdx, 0, false);
        SetFlagCarrier(otherTeamIdx, player->GetObjectGuid());
        player->CastSpell(player, wsSpellTypes[otherTeamIdx][actionId], TRIGGERED_OLD_TRIGGERED);
        m_FlagState[otherTeamIdx] = BG_WS_FLAG_STATE_ON_PLAYER;
        UpdateFlagState(team, BG_WS_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(wsStateUpdateId[otherTeamIdx], 1);
        SendMessageToAll(wsMessageIds[otherTeamIdx][actionId], wsChatMessageTypes[teamIdx][actionId], player);
        PlaySoundToAll(wsSounds[otherTeamIdx][actionId]);
    }

    if (actionId != BG_WS_FLAG_ACTION_NONE)
        player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    return actionId;
}

void BattleGroundWS::EventPlayerClickedOnFlag(Player* player, GameObject* target)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 event = sBattleGroundMgr.GetGameObjectEventIndex(target->GetGUIDLow()).event1;

    // Check if the flag is being picked up from base
    if ((event == WS_EVENT_FLAG_A && GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_BASE) ||
        (event == WS_EVENT_FLAG_H && GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_BASE))
    {
        PickUpFlagFromBase(player);
    }
    // Check if we are trying to pick up or return a flag from the ground
    else if (player->IsWithinDistInMap(target, 10) && GroundFlagInteraction(player, target) == BG_WS_FLAG_ACTION_NONE)
    {
        sLog.outError("Failed to action the WS flag from event '%d'.", event);
    }
}

void BattleGroundWS::RemovePlayer(Player* player, ObjectGuid guid)
{
    if (IsFlagPickedUp(TEAM_INDEX_ALLIANCE) && m_FlagCarrier[TEAM_INDEX_ALLIANCE] == guid)
    {
        if (!player)
        {
            sLog.outError("BattleGroundWS: Removing offline player who has the FLAG!!");
            ClearFlagCarrier(TEAM_INDEX_ALLIANCE);
            RespawnFlag(ALLIANCE, false);
        }
        else
            EventPlayerDroppedFlag(player);
    }
    if (IsFlagPickedUp(TEAM_INDEX_HORDE) && m_FlagCarrier[TEAM_INDEX_HORDE] == guid)
    {
        if (!player)
        {
            sLog.outError("BattleGroundWS: Removing offline player who has the FLAG!!");
            ClearFlagCarrier(TEAM_INDEX_HORDE);
            RespawnFlag(HORDE, false);
        }
        else
            EventPlayerDroppedFlag(player);
    }
}

void BattleGroundWS::UpdateFlagState(Team team, uint32 value)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, value);
    else
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, value);
}

void BattleGroundWS::UpdateTeamScore(Team team)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_CAPTURES_ALLIANCE, m_TeamScores[TEAM_INDEX_ALLIANCE]);
    else
        UpdateWorldState(BG_WS_FLAG_CAPTURES_HORDE, m_TeamScores[TEAM_INDEX_HORDE]);
}

bool BattleGroundWS::HandleAreaTrigger(Player* player, uint32 trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return false;

    switch (trigger)
    {
        case 3646:                                          // Alliance Flag spawn
            if (m_FlagState[TEAM_INDEX_HORDE] && !m_FlagState[TEAM_INDEX_ALLIANCE])
                if (GetFlagCarrierGuid(TEAM_INDEX_HORDE) == player->GetObjectGuid())
                    EventPlayerCapturedFlag(player);
            break;
        case 3647:                                          // Horde Flag spawn
            if (m_FlagState[TEAM_INDEX_ALLIANCE] && !m_FlagState[TEAM_INDEX_HORDE])
                if (GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE) == player->GetObjectGuid())
                    EventPlayerCapturedFlag(player);
            break;
        case 3669: // horde portal
            if (player->GetTeam() != HORDE)
                player->GetSession()->SendNotification(LANG_BATTLEGROUND_ONLY_HORDE_USE);
            else
                player->LeaveBattleground();
            break;
        case 3671: // alliance portal
            if (player->GetTeam() != ALLIANCE)
                player->GetSession()->SendNotification(LANG_BATTLEGROUND_ONLY_ALLIANCE_USE);
            else
                player->LeaveBattleground();
            break;
        default:
            return false;
    }
    return true;
}

void BattleGroundWS::Reset()
{
    // call parent's class reset
    BattleGround::Reset();

    // spiritguides and flags not spawned at beginning
    m_ActiveEvents[WS_EVENT_SPIRITGUIDES_SPAWN] = BG_EVENT_NONE;
    m_ActiveEvents[WS_EVENT_FLAG_A] = BG_EVENT_NONE;
    m_ActiveEvents[WS_EVENT_FLAG_H] = BG_EVENT_NONE;

    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        m_DroppedFlagGuid[i].Clear();
        m_FlagState[i]       = BG_WS_FLAG_STATE_ON_BASE;
        m_TeamScores[i]      = 0;
    }

    m_FlagCarrier[TEAM_INDEX_ALLIANCE].Clear();
    m_FlagCarrier[TEAM_INDEX_HORDE].Clear();

    bool isBGWeekend = BattleGroundMgr::IsBGWeekend(GetTypeID());
    m_ReputationCapture = (isBGWeekend) ? WS_WEEKEND_FLAG_CAPTURE_REPUTATION : WS_NORMAL_FLAG_CAPTURE_REPUTATION;
    m_HonorWinKills = (isBGWeekend) ? WS_WEEKEND_WIN_KILLS : WS_NORMAL_WIN_KILLS;
    m_HonorEndKills = (isBGWeekend) ? WS_WEEKEND_MAP_COMPLETE_KILLS : WS_NORMAL_MAP_COMPLETE_KILLS;
}

void BattleGroundWS::EndBattleGround(Team winner)
{
    // win reward
    RewardHonorToTeam(GetBonusHonorFromKill(m_HonorWinKills), winner);

    BattleGround::EndBattleGround(winner);
}

void BattleGroundWS::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    BattleGround::HandleKillPlayer(player, killer);
}

void BattleGroundWS::UpdatePlayerScore(Player* player, uint32 type, uint32 value)
{
    BattleGroundScoreMap::iterator itr = m_PlayerScores.find(player->GetObjectGuid());
    if (itr == m_PlayerScores.end())                        // player not found
        return;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattleGroundWGScore*)itr->second)->FlagCaptures += value;
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            ((BattleGroundWGScore*)itr->second)->FlagReturns += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(player, type, value);
            break;
    }
}

WorldSafeLocsEntry const* BattleGroundWS::GetClosestGraveYard(Player* player)
{
    // if status in progress, it returns main graveyards with spiritguides
    // else it will return the graveyard in the flagroom - this is especially good
    // if a player dies in preparation phase - then the player can't cheat
    // and teleport to the graveyard outside the flagroom
    // and start running around, while the doors are still closed
    if (player->GetTeam() == ALLIANCE)
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(WS_GRAVEYARD_MAIN_ALLIANCE);

        return sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(WS_GRAVEYARD_FLAGROOM_ALLIANCE);
    }
    if (GetStatus() == STATUS_IN_PROGRESS)
        return sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(WS_GRAVEYARD_MAIN_HORDE);

    return sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(WS_GRAVEYARD_FLAGROOM_HORDE);
}

void BattleGroundWS::FillInitialWorldStates(WorldPacket& data, uint32& count)
{
    FillInitialWorldState(data, count, BG_WS_FLAG_CAPTURES_ALLIANCE, m_TeamScores[TEAM_INDEX_ALLIANCE]);
    FillInitialWorldState(data, count, BG_WS_FLAG_CAPTURES_HORDE, m_TeamScores[TEAM_INDEX_HORDE]);

    if (m_FlagState[TEAM_INDEX_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_ALLIANCE, -1);
    else if (m_FlagState[TEAM_INDEX_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_ALLIANCE, 1);
    else
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_ALLIANCE, 0);

    if (m_FlagState[TEAM_INDEX_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_HORDE, -1);
    else if (m_FlagState[TEAM_INDEX_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_HORDE, 1);
    else
        FillInitialWorldState(data, count, BG_WS_FLAG_UNK_HORDE, 0);

    FillInitialWorldState(data, count, BG_WS_FLAG_CAPTURES_MAX, BG_WS_MAX_TEAM_SCORE);

    if (m_FlagState[TEAM_INDEX_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        FillInitialWorldState(data, count, BG_WS_FLAG_STATE_HORDE, 2);
    else
        FillInitialWorldState(data, count, BG_WS_FLAG_STATE_HORDE, 1);

    if (m_FlagState[TEAM_INDEX_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        FillInitialWorldState(data, count, BG_WS_FLAG_STATE_ALLIANCE, 2);
    else
        FillInitialWorldState(data, count, BG_WS_FLAG_STATE_ALLIANCE, 1);
}

Team BattleGroundWS::GetPrematureWinner()
{
    int32 hordeScore = m_TeamScores[TEAM_INDEX_HORDE];
    int32 allianceScore = m_TeamScores[TEAM_INDEX_ALLIANCE];

    if (hordeScore > allianceScore)
        return HORDE;
    if (allianceScore > hordeScore)
        return ALLIANCE;

    // If the values are equal, fall back to number of players on each team
    return BattleGround::GetPrematureWinner();
}
