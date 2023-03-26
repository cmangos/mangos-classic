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
#include "Server/WorldPacket.h"

BattleGroundWS::BattleGroundWS() : m_reputationCapture(0), m_honorWinKills(0), m_honorEndKills(0)
{
    // set battleground start message ids
    m_startMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_startMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;
}

void BattleGroundWS::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // compute flag respawn timers
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        // respawn flag after team score
        if (m_flagOnRespawn[i])
        {
            if (m_flagsTimer[i] <= diff)
            {
                Team playerTeam = GetTeamIdByTeamIndex((PvpTeamIndex)i);
                RespawnFlagAtBase(playerTeam, true);
                m_flagsTimer[i] = 0;
            }
            else
                m_flagsTimer[i] -= diff;
        }
        // respawn flag after dropped
        else if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[i]) == BG_WS_FLAG_STATE_ON_GROUND && m_flagsDropTimer[i])
        {
            if (m_flagsDropTimer[i] <= diff)
            {
                Team playerTeam = GetTeamIdByTeamIndex((PvpTeamIndex)i);
                m_flagsDropTimer[i] = 0;
                RespawnDroppedFlag(playerTeam);
            }
            else
                m_flagsDropTimer[i] -= diff;
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

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_MAIN_ALLIANCE,     BG_WS_ZONE_ID_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_MAIN_HORDE,        BG_WS_ZONE_ID_MAIN, HORDE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_FLAGROOM_ALLIANCE, BG_WS_ZONE_ID_MAIN, TEAM_INVALID);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_FLAGROOM_HORDE,    BG_WS_ZONE_ID_MAIN, TEAM_INVALID);
}

void BattleGroundWS::AddPlayer(Player* player)
{
    BattleGround::AddPlayer(player);

    // create score and add it to map, default values are set in constructor
    BattleGroundWGScore* score = new BattleGroundWGScore;

    m_playerScores[player->GetObjectGuid()] = score;
}

// Respawn the flag at the base
void BattleGroundWS::RespawnFlagAtBase(Team team, bool wasCaptured)
{
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);
    GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[teamIdx], BG_WS_FLAG_STATE_ON_BASE);
    SpawnEvent(teamIdx, 0, true);
    m_flagOnRespawn[teamIdx] = false;

    // if flag was captured process the following events
    if (wasCaptured)
    {
        DEBUG_LOG("BattleGroundWS: The main flag for team %u has respawned after enemy score.", team);

        // when map_update will be allowed for battlegrounds this code will be useless
        SpawnEvent(WS_EVENT_FLAG_A, 0, true);
        SpawnEvent(WS_EVENT_FLAG_H, 0, true);

        SendMessageToAll(LANG_BG_WS_F_PLACED, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);        // flag respawned sound...
    }
}

// Respawn the flag at the base
void BattleGroundWS::RespawnDroppedFlag(Team team)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundWS: The dropped flag for team %u has respawned.", team);

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);

    // respawn the flag at the base
    RespawnFlagAtBase(team, false);

    // inform players
    SendMessageToAll(wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_RESPAWN].messageId, wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_RESPAWN].chatType);
    PlaySoundToAll(wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_RESPAWN].soundId);

    // despawn the temp summoned flag
    if (GameObject* flag = GetBgMap()->GetGameObject(GetDroppedFlagGuid(team)))
        flag->SetLootState(GO_JUST_DEACTIVATED);

    ClearDroppedFlagGuid(team);
}

int32 BattleGroundWS::GetFlagState(Team team)
{
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    return GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[teamIdx]);
}

// Method that process captured flag score
void BattleGroundWS::ProcessPlayerFlagScoreEvent(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundWS: Team %u has scored.", player->GetTeam());

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);

    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    uint8 otherTeamIdx = GetOtherTeamIndex(teamIdx);

    if (!IsFlagPickedUp(otherTeamIdx))
        return;

    ClearFlagCarrier(otherTeamIdx); // must be before aura remove to prevent 2 events (drop+capture) at the same time

    // Horde flag in base (but not respawned yet)
    GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[otherTeamIdx], BG_WS_FLAG_STATE_ON_BASE);
    m_flagOnRespawn[otherTeamIdx] = true;

    // Drop Horde Flag from Player
    player->RemoveAurasDueToSpell(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP].spellId);

    // play sound and reward reputation
    PlaySoundToAll(wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_CAPTURED].soundId);
    RewardReputationToTeam(team == ALLIANCE ? 890 : 889, m_reputationCapture, team);

    // for flag capture is reward 2 honorable kills
    RewardHonorToTeam(GetBonusHonorFromKill(2), team);

    // update score
    int32 pointsWorldState = team == ALLIANCE ? BG_WS_STATE_CAPTURES_ALLIANCE : BG_WS_STATE_CAPTURES_HORDE;
    GetBgMap()->GetVariableManager().SetVariable(pointsWorldState, GetBgMap()->GetVariableManager().GetVariable(pointsWorldState) + 1);

    GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[teamIdx], BG_WS_FLAG_STATE_ON_BASE);
    GetBgMap()->GetVariableManager().SetVariable(wsFlagHUDPickedUp[teamIdx], BG_WS_FLAG_ICON_INVISIBLE);

    // despawn flags
    SpawnEvent(WS_EVENT_FLAG_A, 0, false);
    SpawnEvent(WS_EVENT_FLAG_H, 0, false);

    SendMessageToAll(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_CAPTURED].messageId, wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_CAPTURED].chatType, player);

    // only flag capture should be updated
    UpdatePlayerScore(player, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures

    // Process match winner
    Team winner = GetBgMap()->GetVariableManager().GetVariable(pointsWorldState) == BG_WS_MAX_TEAM_SCORE ? GetTeamIdByTeamIndex(teamIdx) : TEAM_NONE;

    // end battleground or set respawn timer
    if (winner)
    {
        GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_ALLIANCE, 0);
        GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_HORDE, 0);
        GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_ALLIANCE, BG_WS_FLAG_ICON_INVISIBLE);
        GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_HORDE, BG_WS_FLAG_ICON_INVISIBLE);

        EndBattleGround(winner);
    }
    else
        m_flagsTimer[otherTeamIdx] = BG_WS_FLAG_RESPAWN_TIME;
}

// Method that handles the flag drop
void BattleGroundWS::HandlePlayerDroppedFlag(Player* player)
{
    DEBUG_LOG("BattleGroundWS: Team %u has dropped the flag.", player->GetTeam());

    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        if (IsFlagPickedUp(teamIdx) && GetFlagCarrierGuid(teamIdx) == player->GetObjectGuid())
        {
            ClearFlagCarrier(teamIdx);
            player->RemoveAurasDueToSpell(wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_PICKEDUP].spellId);
        }
    }
    // Check if the other team has picked the flag and if the flag carrier stored is the player which triggers this function
    else if (IsFlagPickedUp(otherTeamIdx) && GetFlagCarrierGuid(otherTeamIdx) == player->GetObjectGuid())
    {
        ClearFlagCarrier(otherTeamIdx);
        player->RemoveAurasDueToSpell(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP].spellId);

        GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[otherTeamIdx], BG_WS_FLAG_STATE_ON_GROUND);
        player->CastSpell(player, wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED].spellId, TRIGGERED_OLD_TRIGGERED);

        GetBgMap()->GetVariableManager().SetVariable(wsFlagHUDPickedUp[otherTeamIdx], BG_WS_FLAG_ICON_INVISIBLE);

        SendMessageToAll(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED].messageId, wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_DROPPED].chatType, player);

        m_flagsDropTimer[otherTeamIdx] = BG_WS_FLAG_DROP_TIME;
    }
}

// Function that handles the flag pick up from the base
void BattleGroundWS::ProcessFlagPickUpFromBase(Player* player, Team attackerTeam)
{
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(attackerTeam);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[otherTeamIdx]) != BG_WS_FLAG_STATE_ON_BASE || m_flagOnRespawn[otherTeamIdx])
        return;

    DEBUG_LOG("BattleGroundWS: Team %u has taken the enemy flag.", attackerTeam);

    SpawnEvent(otherTeamIdx, 0, false);
    SetFlagCarrier(otherTeamIdx, player->GetObjectGuid());
    GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[otherTeamIdx], BG_WS_FLAG_STATE_ON_PLAYER);
    GetBgMap()->GetVariableManager().SetVariable(wsFlagHUDPickedUp[otherTeamIdx], BG_WS_FLAG_ICON_VISIBLE);

    // not meant to be a triggered cast - clears a lot during cast like stealth

    PlaySoundToAll(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP].soundId);
    SendMessageToAll(wsgFlagData[otherTeamIdx][BG_WS_FLAG_ACTION_PICKEDUP].messageId, wsgFlagData[teamIdx][BG_WS_FLAG_ACTION_PICKEDUP].chatType, player);
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);
}

// Function that handles the click action on the dropped flag
void BattleGroundWS::ProcessDroppedFlagActions(Player* player, GameObject* target)
{
    Team team = player->GetTeam();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);

    int32 actionId = BG_WS_FLAG_ACTION_NONE;

    // check if we are returning our flag
    if (wsDroppedFlagId[teamIdx] == target->GetEntry())
    {
        if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[teamIdx]) != BG_WS_FLAG_STATE_ON_GROUND)
            return;

        DEBUG_LOG("BattleGroundWS: Team %u has returned the dropped flag %u.", player->GetTeam(), target->GetEntry());

        actionId = BG_WS_FLAG_ACTION_RETURNED;

        GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[teamIdx], BG_WS_FLAG_STATE_ON_BASE);
        GetBgMap()->GetVariableManager().SetVariable(wsFlagHUDPickedUp[teamIdx], BG_WS_FLAG_ICON_INVISIBLE);

        RespawnFlagAtBase(team, false);
        UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);

        // send messages and sounds
        SendMessageToAll(wsgFlagData[teamIdx][actionId].messageId, wsgFlagData[teamIdx][actionId].chatType, player);
        PlaySoundToAll(wsgFlagData[teamIdx][actionId].soundId);
    }
    // check if we are picking up enemy flag
    else if (wsDroppedFlagId[otherTeamIdx] == target->GetEntry())
    {
        if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[otherTeamIdx]) != BG_WS_FLAG_STATE_ON_GROUND)
            return;

        DEBUG_LOG("BattleGroundWS: Team %u has recaptured the dropped flag %u.", player->GetTeam(), target->GetEntry());

        actionId = BG_WS_FLAG_ACTION_PICKEDUP;

        SpawnEvent(otherTeamIdx, 0, false);
        SetFlagCarrier(otherTeamIdx, player->GetObjectGuid());

        GetBgMap()->GetVariableManager().SetVariable(wsFlagPickedUp[otherTeamIdx], BG_WS_FLAG_STATE_ON_PLAYER);
        GetBgMap()->GetVariableManager().SetVariable(wsFlagHUDPickedUp[otherTeamIdx], BG_WS_FLAG_ICON_VISIBLE);

        // send messages and sounds
        SendMessageToAll(wsgFlagData[otherTeamIdx][actionId].messageId, wsgFlagData[teamIdx][actionId].chatType, player);
        PlaySoundToAll(wsgFlagData[otherTeamIdx][actionId].soundId);
    }

    if (actionId != BG_WS_FLAG_ACTION_NONE)
        player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);
}

// Handle flag click event
void BattleGroundWS::HandlePlayerClickedOnFlag(Player* player, GameObject* goTarget)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // assess the current flag state based on the gameobject entry
    switch (goTarget->GetEntry())
    {
        case GO_WS_SILVERWING_FLAG:
        case GO_WS_WARSONG_FLAG:
            ProcessFlagPickUpFromBase(player, player->GetTeam());
            break;
    }
}

// Handle player exit
void BattleGroundWS::RemovePlayer(Player* player, ObjectGuid guid)
{
    std::vector<std::pair<Team, PvpTeamIndex>> checkIndex;
    if (player)
    {
        Team playerTeam = player->GetTeam();
        PvpTeamIndex playerTeamIndex = GetTeamIndexByTeamId(playerTeam);
        PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(playerTeamIndex);
        checkIndex.emplace_back(playerTeam, otherTeamIdx);
    }
    else // if no player, check both
    {
        checkIndex.push_back({ HORDE, TEAM_INDEX_ALLIANCE });
        checkIndex.push_back({ ALLIANCE, TEAM_INDEX_HORDE });
    }

    for (auto& data : checkIndex)
    {
        // Clear flag carrier and respawn main flag
        if (IsFlagPickedUp(data.second) && m_flagCarrier[data.second] == guid)
        {
            if (!player) // recheck the validity of this, shouldnt flag be dropped when player is logged out?
            {
                sLog.outError("BattleGroundWS: Removing offline player who unexpectendly carries the flag!");

                ClearFlagCarrier(data.second);
                RespawnFlagAtBase(data.first, false);
            }
            else
                HandlePlayerDroppedFlag(player);
        }
    }
}

// Handle area trigger
bool BattleGroundWS::HandleAreaTrigger(Player* player, uint32 trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return false;

    switch (trigger)
    {
        case WS_AT_SILVERWING_ROOM:                                     // Alliance Flag spawn
            if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[TEAM_INDEX_HORDE])
                && !GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[TEAM_INDEX_ALLIANCE]))
                if (GetFlagCarrierGuid(TEAM_INDEX_HORDE) == player->GetObjectGuid())
                    ProcessPlayerFlagScoreEvent(player);
            break;
        case WS_AT_WARSONG_ROOM:                                        // Horde Flag spawn
            if (GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[TEAM_INDEX_ALLIANCE])
                && !GetBgMap()->GetVariableManager().GetVariable(wsFlagPickedUp[TEAM_INDEX_HORDE]))
                if (GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE) == player->GetObjectGuid())
                    ProcessPlayerFlagScoreEvent(player);
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

void BattleGroundWS::HandleGameObjectCreate(GameObject* go)
{
    // set initial data and activate capture points
    switch (go->GetEntry())
    {
        case GO_WS_SILVERWING_FLAG_DROP:
            m_droppedFlagGuid[TEAM_INDEX_ALLIANCE] = go->GetObjectGuid();
            break;
        case GO_WS_WARSONG_FLAG_DROP:
            m_droppedFlagGuid[TEAM_INDEX_HORDE] = go->GetObjectGuid();
            break;
    }
}

// process click on dropped flag events
bool BattleGroundWS::HandleEvent(uint32 eventId, Object* source, Object* target)
{
    switch (eventId)
    {
        case WS_EVENT_ALLIANCE_FLAG_DROPPED_PICKUP:
        case WS_EVENT_HORDE_FLAG_DROPPED_PICKUP:
        {
            if (!source || !source->IsGameObject())
                return true;

            GameObject* go = static_cast<GameObject*>(source);
            if (!target->IsPlayer())
                return true;

            Player* srcPlayer = static_cast<Player*>(target);

            // only handles return, not pickup of dropped
            Team team = srcPlayer->GetTeam();
            PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
            PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(teamIdx);
            if (wsDroppedFlagId[teamIdx] != go->GetEntry())
                return true;

            ProcessDroppedFlagActions(srcPlayer, go);

            // when clicked the flag despawns
            go->SetLootState(GO_JUST_DEACTIVATED);
            break;
        }
        case WS_EVENT_ALLIANCE_FLAG_PICKUP:
        case WS_EVENT_HORDE_FLAG_PICKUP:
        {
            if (!target || !target->IsGameObject())
                return true;

            GameObject* go = static_cast<GameObject*>(target);
            if (!source->IsPlayer())
                return true;

            Player* srcPlayer = static_cast<Player*>(source);

            if (go->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)
                ProcessDroppedFlagActions(srcPlayer, go);
            else
                HandlePlayerClickedOnFlag(srcPlayer, go);

            // when clicked the flag despawns
            go->SetLootState(GO_JUST_DEACTIVATED);
            break;
        }
    }

    return false;
}

void BattleGroundWS::Reset()
{
    // call parent's class reset
    BattleGround::Reset();

    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_CAPTURES_ALLIANCE, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_CAPTURES_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_CAPTURES_HORDE, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_CAPTURES_HORDE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_CAPTURES_MAX, BG_WS_MAX_TEAM_SCORE);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_CAPTURES_MAX, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_HORDE, BG_WS_FLAG_ICON_INVISIBLE);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_FLAG_HORDE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_ALLIANCE, BG_WS_FLAG_ICON_INVISIBLE);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_FLAG_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_ALLIANCE, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_HORDE, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_WS_STATE_FLAG_PICKED_UP_FLAG_STATE_HORDE, true, 0, 0);

    // spiritguides and flags not spawned at beginning
    m_activeEvents[WS_EVENT_SPIRITGUIDES_SPAWN] = BG_EVENT_NONE;
    m_activeEvents[WS_EVENT_FLAG_A] = BG_EVENT_NONE;
    m_activeEvents[WS_EVENT_FLAG_H] = BG_EVENT_NONE;

    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        m_droppedFlagGuid[i].Clear();
        m_flagCarrier[i].Clear();
        m_flagOnRespawn[i]  = false;

        m_flagsDropTimer[i] = 0;
        m_flagsTimer[i]     = 0;
    }

    bool isBgWeekend = BattleGroundMgr::IsBgWeekend(GetTypeId());
    m_reputationCapture = (isBgWeekend) ? BG_WS_WEEKEND_FLAG_CAPTURE_REPUTATION : BG_WS_NORMAL_FLAG_CAPTURE_REPUTATION;
    m_honorWinKills = (isBgWeekend) ? BG_WS_WEEKEND_WIN_KILLS : BG_WS_NORMAL_WIN_KILLS;
    m_honorEndKills = (isBgWeekend) ? BG_WS_WEEKEND_MAP_COMPLETE_KILLS : BG_WS_NORMAL_MAP_COMPLETE_KILLS;

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_MAIN_ALLIANCE,     BG_WS_ZONE_ID_MAIN, TEAM_INVALID);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_MAIN_HORDE,        BG_WS_ZONE_ID_MAIN, TEAM_INVALID);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_FLAGROOM_ALLIANCE, BG_WS_ZONE_ID_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(WS_GRAVEYARD_FLAGROOM_HORDE,    BG_WS_ZONE_ID_MAIN, HORDE);
}

void BattleGroundWS::EndBattleGround(Team winner)
{
    // win reward
    RewardHonorToTeam(GetBonusHonorFromKill(m_honorWinKills), winner);

    BattleGround::EndBattleGround(winner);
}

void BattleGroundWS::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    HandlePlayerDroppedFlag(player);

    BattleGround::HandleKillPlayer(player, killer);
}

void BattleGroundWS::UpdatePlayerScore(Player* player, uint32 type, uint32 value)
{
    BattleGroundScoreMap::iterator itr = m_playerScores.find(player->GetObjectGuid());
    if (itr == m_playerScores.end())                        // player not found
        return;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattleGroundWGScore*)itr->second)->flagCaptures += value;
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            ((BattleGroundWGScore*)itr->second)->flagReturns += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(player, type, value);
            break;
    }
}

Team BattleGroundWS::GetPrematureWinner()
{
    int32 hordeScore = GetBgMap()->GetVariableManager().GetVariable(BG_WS_STATE_CAPTURES_HORDE);
    int32 allianceScore = GetBgMap()->GetVariableManager().GetVariable(BG_WS_STATE_CAPTURES_ALLIANCE);

    if (hordeScore > allianceScore)
        return HORDE;
    if (allianceScore > hordeScore)
        return ALLIANCE;

    // If the values are equal, fall back to number of players on each team
    return BattleGround::GetPrematureWinner();
}
