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
#include "BattleGroundEY.h"
#include "Entities/Creature.h"
#include "Globals/ObjectMgr.h"
#include "BattleGroundMgr.h"
#include "Tools/Language.h"
#include "WorldPacket.h"
#include "Util.h"
#include "Maps/MapManager.h"
#include "AI/ScriptDevAI/include/sc_grid_searchers.h"

BattleGroundEY::BattleGroundEY(): m_flagState(), m_honorTicks(0), m_flagRespawnTimer(0), m_resourceUpdateTimer(0)
{
    // set battleground start message id
    m_startMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_startMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_EY_START_ONE_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_EY_START_HALF_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_EY_HAS_BEGUN;
}

void BattleGroundEY::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // resource counter
    if (m_resourceUpdateTimer < diff)
    {
        UpdateResources();
        m_resourceUpdateTimer = EY_RESOURCES_UPDATE_TIME;
    }
    else
        m_resourceUpdateTimer -= diff;

    // flag respawn
    if (m_flagState == EY_FLAG_STATE_WAIT_RESPAWN || m_flagState == EY_FLAG_STATE_ON_GROUND)
    {
        if (m_flagRespawnTimer < diff)
        {
            m_flagRespawnTimer = 0;

            if (m_flagState == EY_FLAG_STATE_WAIT_RESPAWN)
                RespawnFlagAtCenter(true);
            else
                RespawnDroppedFlag();
        }
        else
            m_flagRespawnTimer -= diff;
    }
}

void BattleGroundEY::StartingEventOpenDoors()
{
    // eye-doors have a despawn animation
    OpenDoorEvent(BG_EVENT_DOOR);

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(GRAVEYARD_EY_MAIN_ALLIANCE, EY_ZONE_ID_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(GRAVEYARD_EY_MAIN_HORDE, EY_ZONE_ID_MAIN, HORDE);
}

void BattleGroundEY::AddPoints(Team team, uint32 points)
{
    PvpTeamIndex team_index = GetTeamIndexByTeamId(team);
    int32 worldStateId = team == ALLIANCE ? WORLD_STATE_EY_RESOURCES_ALLIANCE : WORLD_STATE_EY_RESOURCES_HORDE;
    GetBgMap()->GetVariableManager().SetVariable(worldStateId, GetBgMap()->GetVariableManager().GetVariable(worldStateId) + points);
    m_honorScoreTicks[team_index] += points;

    if (m_honorScoreTicks[team_index] >= m_honorTicks)
    {
        RewardHonorToTeam(GetBonusHonorFromKill(1), team);
        m_honorScoreTicks[team_index] -= m_honorTicks;
    }
}

void BattleGroundEY::SetFlagState(EYFlagState state)
{
    m_flagState = state;

    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_READY, m_flagState == EY_FLAG_STATE_ON_BASE);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_ALLIANCE, m_flagState == EY_FLAG_STATE_ON_ALLIANCE_PLAYER ? 2 : 1);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_HORDE, m_flagState == EY_FLAG_STATE_ON_HORDE_PLAYER ? 2 : 1);
}

void BattleGroundEY::SetTowerOwner(EYNodes node, Team team)
{
    m_towerOwner[node] = team;
    switch (node)
    {
        case NODE_BLOOD_ELF_TOWER:
        {
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_BLOOD_ELF_TOWER_ALLIANCE, m_towerOwner[NODE_BLOOD_ELF_TOWER] == ALLIANCE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_BLOOD_ELF_TOWER_HORDE, m_towerOwner[NODE_BLOOD_ELF_TOWER] == HORDE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_BLOOD_ELF_TOWER_NEUTRAL, m_towerOwner[NODE_BLOOD_ELF_TOWER] == TEAM_NONE);
            break;
        }
        case NODE_FEL_REAVER_RUINS:
        {
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_FEL_REAVER_RUINS_ALLIANCE, m_towerOwner[NODE_FEL_REAVER_RUINS] == ALLIANCE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_FEL_REAVER_RUINS_HORDE, m_towerOwner[NODE_FEL_REAVER_RUINS] == HORDE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_FEL_REAVER_RUINS_NEUTRAL, m_towerOwner[NODE_FEL_REAVER_RUINS] == TEAM_NONE);
            break;
        }
        case NODE_MAGE_TOWER:
        {
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_MAGE_TOWER_ALLIANCE, m_towerOwner[NODE_MAGE_TOWER] == ALLIANCE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_MAGE_TOWER_HORDE, m_towerOwner[NODE_MAGE_TOWER] == HORDE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_MAGE_TOWER_NEUTRAL, m_towerOwner[NODE_MAGE_TOWER] == TEAM_NONE);
            break;
        }
        case NODE_DRAENEI_RUINS:
        {
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_DRAENEI_RUINS_ALLIANCE, m_towerOwner[NODE_DRAENEI_RUINS] == ALLIANCE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_DRAENEI_RUINS_HORDE, m_towerOwner[NODE_DRAENEI_RUINS] == HORDE);
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_DRAENEI_RUINS_NEUTRAL, m_towerOwner[NODE_DRAENEI_RUINS] == TEAM_NONE);
            break;
        }
    }
}

// Update resources by team based on the owned towers
void BattleGroundEY::UpdateResources()
{
    if (GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) > 0)
    {
        AddPoints(ALLIANCE, eyTickPoints[GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) - 1]);
        CheckVictory(ALLIANCE);
    }

    if (GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) > 0)
    {
        AddPoints(HORDE, eyTickPoints[GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) - 1]);
        CheckVictory(HORDE);
    }
}

// Update team score
void BattleGroundEY::CheckVictory(Team team)
{
    uint32 score = GetBgMap()->GetVariableManager().GetVariable(team == ALLIANCE ? WORLD_STATE_EY_RESOURCES_ALLIANCE : WORLD_STATE_EY_RESOURCES_HORDE);

    if (score >= EY_MAX_TEAM_SCORE)
    {
        score = EY_MAX_TEAM_SCORE;
        EndBattleGround(team);
    }
}

// Handle battleground end
void BattleGroundEY::EndBattleGround(Team winner)
{
    // win reward
    RewardHonorToTeam(GetBonusHonorFromKill(1), winner);

    // complete map reward
    RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);

    // disable capture points
    for (auto m_tower : m_towers)
        if (GameObject* go = GetBgMap()->GetGameObject(m_tower))
            go->SetLootState(GO_JUST_DEACTIVATED);

    BattleGround::EndBattleGround(winner);
}

void BattleGroundEY::AddPlayer(Player* player)
{
    BattleGround::AddPlayer(player);

    // create score and add it to map
    BattleGroundEYScore* score = new BattleGroundEYScore;

    m_playerScores[player->GetObjectGuid()] = score;
}

void BattleGroundEY::RemovePlayer(Player* player, ObjectGuid guid)
{
    // sometimes flag aura not removed :(
    if (IsFlagPickedUp())
    {
        if (m_flagCarrier == guid)
        {
            if (player)
                HandlePlayerDroppedFlag(player);
            else
            {
                sLog.outError("BattleGroundEY: Removing offline player who unexpectendly carries the flag!");

                ClearFlagCarrier();
                RespawnFlagAtCenter(false);
            }
        }
    }
}

void BattleGroundEY::HandleGameObjectCreate(GameObject* go)
{
    // set initial data and activate capture points
    switch (go->GetEntry())
    {
        case GO_CAPTURE_POINT_BLOOD_ELF_TOWER:
            m_towers[NODE_BLOOD_ELF_TOWER] = go->GetObjectGuid();
            go->SetCapturePointSlider(CAPTURE_SLIDER_MIDDLE, false);
            break;
        case GO_CAPTURE_POINT_FEL_REAVER_RUINS:
            m_towers[NODE_FEL_REAVER_RUINS] = go->GetObjectGuid();
            go->SetCapturePointSlider(CAPTURE_SLIDER_MIDDLE, false);
            break;
        case GO_CAPTURE_POINT_MAGE_TOWER:
            m_towers[NODE_MAGE_TOWER] = go->GetObjectGuid();
            go->SetCapturePointSlider(CAPTURE_SLIDER_MIDDLE, false);
            break;
        case GO_CAPTURE_POINT_DRAENEI_RUINS:
            m_towers[NODE_DRAENEI_RUINS] = go->GetObjectGuid();
            go->SetCapturePointSlider(CAPTURE_SLIDER_MIDDLE, false);
            break;
        case GO_EY_NETHERSTORM_FLAG_DROP:
            m_droppedFlagGuid = go->GetObjectGuid();
            break;
        case GO_EY_NETHERSTORM_FLAG:
            m_mainFlagDbGuid = go->GetDbGuid();
            break;
    }
}

// process the capture events
bool BattleGroundEY::HandleEvent(uint32 eventId, Object* source, Object* target)
{
    // event called when player picks up a dropped flag
    if (eventId == EVENT_NETHERSTORM_FLAG_SPELL && target->IsPlayer() && source->IsGameObject())
    {
        DEBUG_LOG("BattleGroundEY: Handle flag pickup event id %u", eventId);

        HandlePlayerClickedOnFlag(static_cast<Player*>(target), static_cast<GameObject*>(source));
        return true;
    }

    GameObject* go = dynamic_cast<GameObject*>(source);
    MANGOS_ASSERT(go); // if not go, blow up so we can check why

    // events called from the capture points
    for (uint8 i = 0; i < EY_MAX_NODES; ++i)
    {
        if (eyTowers[i] == go->GetEntry())
        {
            for (uint8 j = 0; j < 4; ++j)
            {
                if (eyTowerEvents[i][j].eventEntry == eventId)
                {
                    ProcessCaptureEvent(go, i, eyTowerEvents[i][j].team, eyTowerEvents[i][j].worldState, eyTowerEvents[i][j].message);

                    // no need to iterate other events or towers
                    return false;
                }
            }
            // no need to iterate other towers
            return false;
        }
    }

    return false;
}

// Method that handles the capture point capture events
void BattleGroundEY::ProcessCaptureEvent(GameObject* go, uint32 towerId, Team team, uint32 newWorldState, uint32 message)
{
    DEBUG_LOG("BattleGroundEY: Process capture point event from gameobject entry %u, captured by team %u", go->GetEntry(), team);

    if (team == ALLIANCE)
    {
        // update counter
        GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE, GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) + 1);

        SendMessageToAll(message, CHAT_MSG_BG_SYSTEM_ALLIANCE);

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(eyGraveyards[towerId], EY_ZONE_ID_MAIN, ALLIANCE);
    }
    else if (team == HORDE)
    {
        // update counter
        GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE, GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) + 1);

        SendMessageToAll(message, CHAT_MSG_BG_SYSTEM_HORDE);

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(eyGraveyards[towerId], EY_ZONE_ID_MAIN, HORDE);
    }
    else
    {
        if (m_towerOwner[towerId] == ALLIANCE)
        {
            // update counter
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE, GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) - 1);

            SendMessageToAll(message, CHAT_MSG_BG_SYSTEM_ALLIANCE);
        }
        else
        {
            // update counter
            GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE, GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) - 1);

            SendMessageToAll(message, CHAT_MSG_BG_SYSTEM_HORDE);
        }

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(eyGraveyards[towerId], EY_ZONE_ID_MAIN, TEAM_INVALID);
    }

    // update capture point owner
    Team oldTeam = m_towerOwner[towerId];
    SetTowerOwner(EYNodes(towerId), team);

    if (oldTeam == ALLIANCE || oldTeam == HORDE) // only on going to grey
    {
        // teleport players off of GY
        Creature* spiritHealer = GetClosestCreatureWithEntry(go, oldTeam == ALLIANCE ? BG_NPC_SPIRIT_GUIDE_ALLIANCE : BG_NPC_SPIRIT_GUIDE_HORDE, 100.f);

        if (spiritHealer)
            spiritHealer->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, spiritHealer, spiritHealer);
    }

    // replace the visual flags
    switch (team)
    {
        case ALLIANCE:  SpawnEvent(towerId, TEAM_INDEX_ALLIANCE, true); break;
        case HORDE:     SpawnEvent(towerId, TEAM_INDEX_HORDE,    true); break;
        case TEAM_NONE: SpawnEvent(towerId, TEAM_INDEX_NEUTRAL,  true); break;
        default: break;
    }
}

// Function that handles area triggers; used to check for flag carriers
bool BattleGroundEY::HandleAreaTrigger(Player* source, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return false;

    if (!source->IsAlive())                                 // hack code, must be removed later
        return false;

    switch (trigger)
    {
        case AREATRIGGER_BLOOD_ELF_TOWER_POINT:
            if (m_towerOwner[NODE_BLOOD_ELF_TOWER] == source->GetTeam())
                ProcessPlayerFlagScoreEvent(source, NODE_BLOOD_ELF_TOWER);
            break;
        case AREATRIGGER_FEL_REAVER_RUINS_POINT:
            if (m_towerOwner[NODE_FEL_REAVER_RUINS] == source->GetTeam())
                ProcessPlayerFlagScoreEvent(source, NODE_FEL_REAVER_RUINS);
            break;
        case AREATRIGGER_MAGE_TOWER_POINT:
            if (m_towerOwner[NODE_MAGE_TOWER] == source->GetTeam())
                ProcessPlayerFlagScoreEvent(source, NODE_MAGE_TOWER);
            break;
        case AREATRIGGER_DRAENEI_RUINS_POINT:
            if (m_towerOwner[NODE_DRAENEI_RUINS] == source->GetTeam())
                ProcessPlayerFlagScoreEvent(source, NODE_DRAENEI_RUINS);
            break;
        default:
            return false;
    }
    return true;
}

void BattleGroundEY::Reset()
{
    // call parent's class reset
    BattleGround::Reset();

    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_CAPTURE_POINT_SLIDER_DISPLAY, WORLD_STATE_REMOVE);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_RESOURCES_ALLIANCE, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_RESOURCES_HORDE, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_READY, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_ALLIANCE, 0);
    GetBgMap()->GetVariableManager().SetVariable(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_HORDE, 0);


    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_CAPTURE_POINT_SLIDER_DISPLAY, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_RESOURCES_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_RESOURCES_HORDE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_TOWER_COUNT_HORDE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_NETHERSTORM_FLAG_READY, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_ALLIANCE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(WORLD_STATE_EY_NETHERSTORM_FLAG_STATE_HORDE, true, 0, 0);

    for (uint32 i = 0; i < EY_MAX_NODES; ++i)
        for (uint32 k = 0; k < 4; ++k)
            GetBgMap()->GetVariableManager().SetVariableData(eyTowerEvents[i][k].worldState, true, 0, 0);

    m_honorTicks = BattleGroundMgr::IsBgWeekend(GetTypeId()) ? EY_WEEKEND_HONOR_INTERVAL : EY_NORMAL_HONOR_INTERVAL;
    m_honorScoreTicks[TEAM_INDEX_ALLIANCE] = 0;
    m_honorScoreTicks[TEAM_INDEX_HORDE] = 0;

    SetFlagState(EY_FLAG_STATE_ON_BASE);
    m_flagCarrier.Clear();
    m_droppedFlagGuid.Clear();

    m_flagRespawnTimer = 0;
    m_resourceUpdateTimer = 0;

    for (uint8 i = 0; i < EY_MAX_NODES; ++i)
    {
        SetTowerOwner(EYNodes(i), TEAM_NONE);
        m_activeEvents[i] = TEAM_INDEX_NEUTRAL;

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(eyGraveyards[i], EY_ZONE_ID_MAIN, TEAM_INVALID);
    }

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(GRAVEYARD_EY_MAIN_ALLIANCE, EY_ZONE_ID_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(GRAVEYARD_EY_MAIN_HORDE, EY_ZONE_ID_MAIN, HORDE);

    // the flag in the middle is spawned at beginning
    m_activeEvents[EY_EVENT_CAPTURE_FLAG] = EY_EVENT2_FLAG_CENTER;
}

// Method that resets main flag at the center
void BattleGroundEY::RespawnFlagAtCenter(bool wasCaptured)
{
    DEBUG_LOG("BattleGroundEY: Respawn flag at the center of the battleground.");

    SetFlagState(EY_FLAG_STATE_ON_BASE);

    // will despawn captured flags at the node and spawn in center
    if (wasCaptured)
        SpawnEvent(EY_EVENT_CAPTURE_FLAG, EY_EVENT2_FLAG_CENTER, true);
    // respawn because of flag drop has to be handled separately
    else
        ChangeBgObjectSpawnState(m_mainFlagDbGuid, RESPAWN_IMMEDIATELY);

    PlaySoundToAll(EY_SOUND_FLAG_RESET);
    SendMessageToAll(LANG_BG_EY_RESETED_FLAG, CHAT_MSG_BG_SYSTEM_NEUTRAL);
}

// Method that respawns dropped flag; called if nobody picks the dropped flag after 10 seconds
void BattleGroundEY::RespawnDroppedFlag()
{
    RespawnFlagAtCenter(false);

    if (GameObject* flag = GetBgMap()->GetGameObject(m_droppedFlagGuid))
        flag->SetLootState(GO_JUST_DEACTIVATED);

    m_droppedFlagGuid.Clear();
}

void BattleGroundEY::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    BattleGround::HandleKillPlayer(player, killer);

    // handle flag drop if necessary
    HandlePlayerDroppedFlag(player);
}

// Handle flag drop
void BattleGroundEY::HandlePlayerDroppedFlag(Player* source)
{
    if (!IsFlagPickedUp())
        return;

    if (GetFlagCarrierGuid() != source->GetObjectGuid())
        return;

    DEBUG_LOG("BattleGroundEY: Team %u has dropped the flag.", source->GetTeam());

    ClearFlagCarrier();
    source->RemoveAurasDueToSpell(EY_SPELL_NETHERSTORM_FLAG);

    // do not cast auras or send messages after match has ended
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    SetFlagState(EY_FLAG_STATE_ON_GROUND);
    m_flagRespawnTimer = EY_FLAG_RESPAWN_TIME;

    source->CastSpell(source, SPELL_RECENTLY_DROPPED_FLAG, TRIGGERED_OLD_TRIGGERED);
    source->CastSpell(source, EY_SPELL_PLAYER_DROPPED_FLAG, TRIGGERED_OLD_TRIGGERED);

    Team playerTeam = source->GetTeam();

    SendMessageToAll(LANG_BG_EY_DROPPED_FLAG, playerTeam == ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, nullptr);
}

// Method that handles when a player clicks on the flag
void BattleGroundEY::HandlePlayerClickedOnFlag(Player* source, GameObject* go)
{
    if (GetStatus() != STATUS_IN_PROGRESS || IsFlagPickedUp() || !source->IsWithinDistInMap(go, 10))
        return;

    DEBUG_LOG("BattleGroundEY: Team %u has taken the flag, gameobject entry %u.", source->GetTeam(), go->GetEntry());

    // update for the team that picked up the flag
    if (source->GetTeam() == ALLIANCE)
    {
        PlaySoundToAll(EY_SOUND_FLAG_PICKED_UP_ALLIANCE);

        SetFlagState(EY_FLAG_STATE_ON_ALLIANCE_PLAYER);
    }
    else if (source->GetTeam() == HORDE)
    {
        PlaySoundToAll(EY_SOUND_FLAG_PICKED_UP_HORDE);

        SetFlagState(EY_FLAG_STATE_ON_HORDE_PLAYER);
    }

    // Note: flag despawn and spell cast are handled in GameObject code
    // Set flag carrier and set right auras 
    SetFlagCarrier(source->GetObjectGuid());
    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);

    // send message
    if (source->GetTeam() == ALLIANCE)
        PSendMessageToAll(LANG_BG_EY_HAS_TAKEN_FLAG, CHAT_MSG_BG_SYSTEM_ALLIANCE, nullptr, source->GetName());
    else
        PSendMessageToAll(LANG_BG_EY_HAS_TAKEN_FLAG, CHAT_MSG_BG_SYSTEM_HORDE, nullptr, source->GetName());

    // when clicked the flag despawns
    go->SetLootState(GO_JUST_DEACTIVATED);
}

// Method that handles the player score when flag is captured at one of controlled nodes
void BattleGroundEY::ProcessPlayerFlagScoreEvent(Player* source, EYNodes node)
{
    if (GetStatus() != STATUS_IN_PROGRESS || GetFlagCarrierGuid() != source->GetObjectGuid())
        return;

    DEBUG_LOG("BattleGroundEY: Team %u has successfully delivered the flag to the node id %u", source->GetTeam(), node);

    ClearFlagCarrier();

    SetFlagState(EY_FLAG_STATE_WAIT_RESPAWN);
    m_flagRespawnTimer = EY_FLAG_RESPAWN_TIME;

    source->RemoveAurasDueToSpell(EY_SPELL_NETHERSTORM_FLAG);
    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);

    // process score actions by team
    if (source->GetTeam() == ALLIANCE)
    {
        PlaySoundToAll(EY_SOUND_FLAG_CAPTURED_ALLIANCE);

        if (GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) > 0)
            AddPoints(ALLIANCE, eyFlagPoints[GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_ALLIANCE) - 1]);

        SendMessageToAll(LANG_BG_EY_CAPTURED_FLAG_A, CHAT_MSG_BG_SYSTEM_ALLIANCE, source);
    }
    else
    {
        PlaySoundToAll(EY_SOUND_FLAG_CAPTURED_HORDE);

        if (GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) > 0)
            AddPoints(HORDE, eyFlagPoints[GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_TOWER_COUNT_HORDE) - 1]);

        SendMessageToAll(LANG_BG_EY_CAPTURED_FLAG_H, CHAT_MSG_BG_SYSTEM_HORDE, source);
    }

    SpawnEvent(EY_EVENT_CAPTURE_FLAG, node, true);

    UpdatePlayerScore(source, SCORE_FLAG_CAPTURES, 1);
}

void BattleGroundEY::UpdatePlayerScore(Player* source, uint32 type, uint32 value)
{
    BattleGroundScoreMap::iterator itr = m_playerScores.find(source->GetObjectGuid());
    if (itr == m_playerScores.end())                        // player not found
        return;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattleGroundEYScore*)itr->second)->flagCaptures += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(source, type, value);
            break;
    }
}

Team BattleGroundEY::GetPrematureWinner()
{
    int32 hordeScore = GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_RESOURCES_HORDE);
    int32 allianceScore = GetBgMap()->GetVariableManager().GetVariable(WORLD_STATE_EY_RESOURCES_ALLIANCE);

    if (hordeScore > allianceScore)
        return HORDE;
    if (allianceScore > hordeScore)
        return ALLIANCE;

    // If the values are equal, fall back to number of players on each team
    return BattleGround::GetPrematureWinner();
}
