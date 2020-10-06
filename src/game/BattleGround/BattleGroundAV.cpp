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

#include "Entities/Player.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "BattleGroundMgr.h"
#include "Entities/Creature.h"
#include "Entities/GameObject.h"
#include "Tools/Language.h"
#include "WorldPacket.h"
#include "Globals/ObjectMgr.h"
#include "AI/ScriptDevAI/include/sc_grid_searchers.h"

BattleGroundAV::BattleGroundAV(): m_honorMapComplete(0), m_repTowerDestruction(0), m_repCaptain(0), m_repBoss(0), m_repOwnedGrave(0), m_repOwnedMine(0), m_repSurviveCaptain(0), m_repSurviveTower(0)
{
    m_startMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_startMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_AV_START_ONE_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_AV_START_HALF_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_AV_HAS_BEGUN;
}

void BattleGroundAV::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    BattleGround::HandleKillPlayer(player, killer);
    UpdateScore(GetTeamIndexByTeamId(player->GetTeam()), -1);
}

// Handle creature kill
void BattleGroundAV::HandleKillUnit(Creature* creature, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundAV: Handle creature entry %u kill by team %u.", creature->GetEntry(), killer->GetTeam());

    switch (creature->GetEntry())
    {
        case BG_AV_NPC_VANNDAR_STORMPIKE:
            CastSpellOnTeam(BG_AV_SPELL_COMPLETE_AV_QUEST, HORDE);   // this is a spell which finishes a quest where a player has to kill the boss
            RewardReputationToTeam(BG_AV_FACTION_H, m_repBoss, HORDE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_BOSS), HORDE);
            SendYellToAll(LANG_BG_AV_A_GENERAL_DEAD, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0));
            EndBattleGround(HORDE);
            break;
        case BG_AV_NPC_DREKTHAR:
            CastSpellOnTeam(BG_AV_SPELL_COMPLETE_AV_QUEST, ALLIANCE); // this is a spell which finishes a quest where a player has to kill the boss
            RewardReputationToTeam(BG_AV_FACTION_A, m_repBoss, ALLIANCE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_BOSS), ALLIANCE);
            SendYellToAll(LANG_BG_AV_H_GENERAL_DEAD, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0));
            EndBattleGround(ALLIANCE);
            break;
        case BG_AV_NPC_CAPTAIN_STONEHEARTH:
            if (IsActiveEvent(BG_AV_NODE_CAPTAIN_DEAD_A, 0))
                return;
            RewardReputationToTeam(BG_AV_FACTION_H, m_repCaptain, HORDE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_CAPTAIN), HORDE);
            UpdateScore(TEAM_INDEX_ALLIANCE, (-1) * BG_AV_RES_CAPTAIN);
            // spawn destroyed aura
            SpawnEvent(BG_AV_NODE_CAPTAIN_DEAD_A, 0, true);
            break;
        case BG_AV_NPC_CAPTAIN_GALVANGAR:
            if (IsActiveEvent(BG_AV_NODE_CAPTAIN_DEAD_H, 0))
                return;
            RewardReputationToTeam(BG_AV_FACTION_A, m_repCaptain, ALLIANCE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_CAPTAIN), ALLIANCE);
            UpdateScore(TEAM_INDEX_HORDE, (-1) * BG_AV_RES_CAPTAIN);
            // spawn destroyed aura
            SpawnEvent(BG_AV_NODE_CAPTAIN_DEAD_H, 0, true);
            break;
        case BG_AV_NPC_MORLOCH:
        case BG_AV_NPC_UMI_THORSON:
        case BG_AV_NPC_KEETAR:
            ChangeMineOwner(BG_AV_NORTH_MINE, GetTeamIndexByTeamId(killer->GetTeam()));
            break;
        case BG_AV_NPC_MASHA_SWIFTCUT:
        case BG_AV_NPC_AGGI_RUMBLESTOMP:
        case BG_AV_NPC_TASKMASTER_SNIVVLE:
            ChangeMineOwner(BG_AV_SOUTH_MINE, GetTeamIndexByTeamId(killer->GetTeam()));
            break;
    }
}

// Handle quest completion for player
void BattleGroundAV::HandleQuestComplete(uint32 questid, Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundAV: Handle quest id %u complete by team %u.", questid, player->GetTeam());

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(player->GetTeam());

    uint32 reputation = 0;                                  // reputation for the whole team (other reputation must be done in db)
    // TODO add events (including quest not available anymore, next quest availabe, go/npc de/spawning)

    switch (questid)
    {
        case BG_AV_QUEST_ARMOR_SCRAPS_A:
        case BG_AV_QUEST_ARMOR_SCRAPS_A_REPEAT:
        case BG_AV_QUEST_ARMOR_SCRAPS_H:
        case BG_AV_QUEST_ARMOR_SCRAPS_H_REPEAT:
            m_teamQuestStatus[teamIdx][0] += 20;
            reputation = 1;
            if (m_teamQuestStatus[teamIdx][0] == 500 || m_teamQuestStatus[teamIdx][0] == 1000 || m_teamQuestStatus[teamIdx][0] == 1500)  // 25,50,75 turn ins
            {
                DEBUG_LOG("BattleGroundAV: Quest %i completed starting with unit upgrading..", questid);
                for (AVNodeIds i = BG_AV_NODES_FIRSTAID_STATION; i <= BG_AV_NODES_FROSTWOLF_HUT; ++i)
                    if (m_nodes[i].owner == teamIdx && m_nodes[i].state == POINT_CONTROLLED)
                        PopulateNode(i);
            }
            break;
        case BG_AV_QUEST_SLIDORES_FLEET:
        case BG_AV_QUEST_GUSES_FLEET:
            m_teamQuestStatus[teamIdx][1]++;
            reputation = 1;
            if (m_teamQuestStatus[teamIdx][1] == 120)
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
            break;
        case BG_AV_QUEST_VIPORES_FLEET:
        case BG_AV_QUEST_JEZTORS_FLEET:
            m_teamQuestStatus[teamIdx][2]++;
            reputation = 2;
            if (m_teamQuestStatus[teamIdx][2] == 60)
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
            break;
        case BG_AV_QUEST_ICHMANS_FLEET:
        case BG_AV_QUEST_MULVERICKS_FLEET:
            m_teamQuestStatus[teamIdx][3]++;
            reputation = 5;
            if (m_teamQuestStatus[teamIdx][1] == 30)
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
            break;
        case BG_AV_QUEST_CRYSTAL_CLUSTER:
        case BG_AV_QUEST_GALLON_OF_BLOOD:
            m_teamQuestStatus[teamIdx][4] += 4;            // there are 2 quests where you can turn in 5 or 1 item.. ( + 4 cause +1 will be done some lines below)
            reputation = 4;
        case BG_AV_QUEST_IVUS_FOREST_LORD:
        case BG_AV_QUEST_LOKHOLAR_ICE_LORD:
            m_teamQuestStatus[teamIdx][4]++;
            reputation += 1;
            if (m_teamQuestStatus[teamIdx][4] >= 200)
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
            break;
        case BG_AV_QUEST_IRONDEEP_SUPPLIES_A:
        case BG_AV_QUEST_COLDTOOTH_SUPPLIES_H:
            m_teamQuestStatus[teamIdx][5]++;
            reputation = 2;
            if (m_teamQuestStatus[teamIdx][5] == 28)
            {
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
                if (m_teamQuestStatus[teamIdx][6] == 7)
                    DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here - ground assault ready", questid);
            }
            break;
        case BG_AV_QUEST_COLDTOOTH_SUPPLIES_A:
        case BG_AV_QUEST_IRONDEEP_SUPPLIES_H:
            m_teamQuestStatus[teamIdx][6]++;
            reputation = 3;
            if (m_teamQuestStatus[teamIdx][6] == 7)
            {
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
                if (m_teamQuestStatus[teamIdx][5] == 20)
                    DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here - ground assault ready", questid);
            }
            break;
        case BG_AV_QUEST_RAM_RIDING:
        case BG_AV_QUEST_RAM_HIDE:
            m_teamQuestStatus[teamIdx][7]++;
            reputation = 1;
            if (m_teamQuestStatus[teamIdx][7] == 25)
            {
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
                if (m_teamQuestStatus[teamIdx][8] == 25)
                    DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here - rider assault ready", questid);
            }
            break;
        case BG_AV_QUEST_EMPTY_STABLES_A:
        case BG_AV_QUEST_EMPTY_STABLES_H:
            m_teamQuestStatus[teamIdx][8]++;
            reputation = 1;
            if (m_teamQuestStatus[teamIdx][8] == 25)
            {
                DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here", questid);
                if (m_teamQuestStatus[teamIdx][7] == 25)
                    DEBUG_LOG("BattleGroundAV: Quest %i completed (need to implement some events here - rider assault ready", questid);
            }
            break;
        default:
            DEBUG_LOG("BattleGroundAV: Quest %i completed but is not interesting for us", questid);
            return;
    }

    if (reputation)
        RewardReputationToTeam((player->GetTeam() == ALLIANCE) ? BG_AV_FACTION_A : BG_AV_FACTION_H, reputation, player->GetTeam());
}

// Method that updates team score
void BattleGroundAV::UpdateScore(PvpTeamIndex teamIdx, int32 points)
{
    // note: to remove reinforcements points must be negative, for adding reinforcements points must be positive
    MANGOS_ASSERT(teamIdx == TEAM_INDEX_ALLIANCE || teamIdx == TEAM_INDEX_HORDE);
    m_teamScores[teamIdx] += points;                      // m_TeamScores is int32 - so no problems here

    if (points < 0)
    {
        if (m_teamScores[teamIdx] < 1)
        {
            m_teamScores[teamIdx] = 0;
            // other team will win:
            EndBattleGround((teamIdx == TEAM_INDEX_ALLIANCE) ? HORDE : ALLIANCE);
        }
        else if (!m_wasInformedNearLose[teamIdx] && m_teamScores[teamIdx] < BG_AV_SCORE_NEAR_LOSE)
        {
            SendMessageToAll((teamIdx == TEAM_INDEX_HORDE) ? LANG_BG_AV_H_NEAR_LOSE : LANG_BG_AV_A_NEAR_LOSE, CHAT_MSG_BG_SYSTEM_NEUTRAL);
            PlaySoundToAll(BG_AV_SOUND_NEAR_LOSE);
            m_wasInformedNearLose[teamIdx] = true;
        }
    }
    // must be called here, else it could display a negative value
    UpdateWorldState(((teamIdx == TEAM_INDEX_HORDE) ? BG_AV_STATE_SCORE_H : BG_AV_STATE_SCORE_A), m_teamScores[teamIdx]);
}

void BattleGroundAV::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // add points from mine owning, and look if the neutral team can reclaim the mine
    for (uint8 mine = 0; mine < BG_AV_MAX_MINES; ++mine)
    {
        if (m_mineOwner[mine] != TEAM_INDEX_NEUTRAL)
        {
            m_mineTimer[mine] -= diff;
            if (m_mineTimer[mine] <= 0)
            {
                UpdateScore(PvpTeamIndex(m_mineOwner[mine]), WORLD_STATE_ADD);
                m_mineTimer[mine] = BG_AV_MINE_TICK_TIMER;
            }

            if (m_mineReclaimTimer[mine] > diff)
                m_mineReclaimTimer[mine] -= diff;
            else
                ChangeMineOwner(mine, TEAM_INDEX_NEUTRAL);
        }
    }

    // looks for all timers of the nodes and destroy the building (for graveyards the building wont get destroyed, it goes just to the other team
    for (AVNodeIds i = BG_AV_NODES_FIRSTAID_STATION; i < BG_AV_MAX_NODES; ++i)
    {
        if (m_nodes[i].state == POINT_ASSAULTED)
        {
            if (m_nodes[i].timer > diff)
                m_nodes[i].timer -= diff;
            else
                ProcessPlayerDestroyedPoint(i);
        }
    }
}

void BattleGroundAV::StartingEventOpenDoors()
{
    UpdateWorldState(BG_AV_STATE_SCORE_SHOW_H, WORLD_STATE_ADD);
    UpdateWorldState(BG_AV_STATE_SCORE_SHOW_A, WORLD_STATE_ADD);

    OpenDoorEvent(BG_EVENT_DOOR);
}

void BattleGroundAV::AddPlayer(Player* player)
{
    BattleGround::AddPlayer(player);
    // create score and add it to map, default values are set in constructor
    BattleGroundAVScore* sc = new BattleGroundAVScore;
    m_playerScores[player->GetObjectGuid()] = sc;
}

void BattleGroundAV::EndBattleGround(Team winner)
{
    // calculate bonuskills for both teams:
    uint32 tower_survived[PVP_TEAM_COUNT]  = {0, 0};
    uint32 graves_owned[PVP_TEAM_COUNT]    = {0, 0};
    uint32 mines_owned[PVP_TEAM_COUNT]     = {0, 0};

    // towers all not destroyed:
    for (AVNodeIds i = BG_AV_NODES_DUNBALDAR_SOUTH; i <= BG_AV_NODES_STONEHEART_BUNKER; ++i)
        if (m_nodes[i].state == POINT_CONTROLLED)
            if (m_nodes[i].totalOwner == TEAM_INDEX_ALLIANCE)
                ++tower_survived[TEAM_INDEX_ALLIANCE];

    for (AVNodeIds i = BG_AV_NODES_ICEBLOOD_TOWER; i <= BG_AV_NODES_FROSTWOLF_WTOWER; ++i)
        if (m_nodes[i].state == POINT_CONTROLLED)
            if (m_nodes[i].totalOwner == TEAM_INDEX_HORDE)
                ++tower_survived[TEAM_INDEX_HORDE];

    // graves all controlled
    for (AVNodeIds i = BG_AV_NODES_FIRSTAID_STATION; i < BG_AV_MAX_NODES; ++i)
        if (m_nodes[i].state == POINT_CONTROLLED && m_nodes[i].owner != TEAM_INDEX_NEUTRAL)
            ++graves_owned[m_nodes[i].owner];

    for (auto& i : m_mineOwner)
        if (i != TEAM_INDEX_NEUTRAL)
            ++mines_owned[i];

    // now we have the values give the honor/reputation to the teams:
    Team team[PVP_TEAM_COUNT]      = { ALLIANCE, HORDE };
    uint32 faction[PVP_TEAM_COUNT]   = { BG_AV_FACTION_A, BG_AV_FACTION_H };

    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        if (tower_survived[i])
        {
            RewardReputationToTeam(faction[i], tower_survived[i] * m_repSurviveTower, team[i]);
            RewardHonorToTeam(GetBonusHonorFromKill(tower_survived[i] * BG_AV_KILL_SURVIVING_TOWER), team[i]);
        }
        DEBUG_LOG("BattleGroundAV: EndbattleGround: bgteam: %u towers:%u honor:%u rep:%u", i, tower_survived[i], GetBonusHonorFromKill(tower_survived[i] * BG_AV_KILL_SURVIVING_TOWER), tower_survived[i] * BG_AV_REP_SURVIVING_TOWER);
        if (graves_owned[i])
            RewardReputationToTeam(faction[i], graves_owned[i] * m_repOwnedGrave, team[i]);
        if (mines_owned[i])
            RewardReputationToTeam(faction[i], mines_owned[i] * m_repOwnedMine, team[i]);
        // captain survived?:
        if (!IsActiveEvent(BG_AV_NODE_CAPTAIN_DEAD_A + GetTeamIndexByTeamId(team[i]), 0))
        {
            RewardReputationToTeam(faction[i], m_repSurviveCaptain, team[i]);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_SURVIVING_CAPTAIN), team[i]);
        }
    }

    // both teams:
    if (m_honorMapComplete)
    {
        RewardHonorToTeam(m_honorMapComplete, ALLIANCE);
        RewardHonorToTeam(m_honorMapComplete, HORDE);
    }

    BattleGround::EndBattleGround(winner);
}

bool BattleGroundAV::HandleAreaTrigger(Player* source, uint32 trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    switch (trigger)
    {
        case BG_AV_AT_ENTRANCE_ALLIANCE1:
        case BG_AV_AT_ENTRANCE_ALLIANCE2:
            if (source->GetTeam() != ALLIANCE)
                source->GetSession()->SendNotification(LANG_BATTLEGROUND_ONLY_ALLIANCE_USE);
            else
                source->LeaveBattleground();
            break;
        case BG_AV_AT_ENTRANCE_HORDE:
            if (source->GetTeam() != HORDE)
                source->GetSession()->SendNotification(LANG_BATTLEGROUND_ONLY_HORDE_USE);
            else
                source->LeaveBattleground();
            break;
        default:
            return false;
    }
    return true;
}

void BattleGroundAV::UpdatePlayerScore(Player* source, uint32 type, uint32 value)
{
    BattleGroundScoreMap::iterator itr = m_playerScores.find(source->GetObjectGuid());
    if (itr == m_playerScores.end())                        // player not found...
        return;

    switch (type)
    {
        case SCORE_GRAVEYARDS_ASSAULTED:
            ((BattleGroundAVScore*)itr->second)->graveyardsAssaulted += value;
            break;
        case SCORE_GRAVEYARDS_DEFENDED:
            ((BattleGroundAVScore*)itr->second)->graveyardsDefended += value;
            break;
        case SCORE_TOWERS_ASSAULTED:
            ((BattleGroundAVScore*)itr->second)->towersAssaulted += value;
            break;
        case SCORE_TOWERS_DEFENDED:
            ((BattleGroundAVScore*)itr->second)->towersDefended += value;
            break;
        case SCORE_SECONDARY_OBJECTIVES:
            ((BattleGroundAVScore*)itr->second)->secondaryObjectives += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(source, type, value);
            break;
    }
}

// Process the destruction of a battleground node
void BattleGroundAV::ProcessPlayerDestroyedPoint(AVNodeIds node)
{
    DEBUG_LOG("BattleGroundAV: Node %i was destroyed", node);

    PvpTeamIndex ownerTeamIdx = PvpTeamIndex(m_nodes[node].owner);
    Team ownerTeam = GetTeamIdByTeamIndex(ownerTeamIdx);

    // despawn banner
    DestroyNode(node);
    PopulateNode(node);
    UpdateNodeWorldState(node);

    if (IsTower(node))
    {
        uint8 tmp = node - BG_AV_NODES_DUNBALDAR_SOUTH;
        // despawn marshal (one of those guys protecting the boss)
        SpawnEvent(BG_AV_MARSHAL_A_SOUTH + tmp, 0, false);

        UpdateScore(GetOtherTeamIndex(ownerTeamIdx), (-1) * BG_AV_RES_TOWER);
        RewardReputationToTeam((ownerTeam == ALLIANCE) ? BG_AV_FACTION_A : BG_AV_FACTION_H, m_repTowerDestruction, ownerTeam);
        RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_TOWER), ownerTeam);
        SendYell2ToAll(LANG_BG_AV_TOWER_TAKEN, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0), GetNodeMessageId(node), (ownerTeam == ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
    }
    else
        SendYell2ToAll(LANG_BG_AV_GRAVE_TAKEN, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0), GetNodeMessageId(node), (ownerTeam == ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
}

// Process mine owner change
void BattleGroundAV::ChangeMineOwner(uint8 mine, PvpTeamIndex teamIdx)
{
    m_mineTimer[mine] = BG_AV_MINE_TICK_TIMER;
    // TODO implement quest 7122
    // mine=0 northmine, mine=1 southmine
    // TODO changing the owner should result in setting respawntime to infinite for current creatures (they should fight the new ones), spawning new mine owners creatures and changing the chest - objects so that the current owning team can use them
    MANGOS_ASSERT(mine == BG_AV_NORTH_MINE || mine == BG_AV_SOUTH_MINE);
    if (m_mineOwner[mine] == teamIdx)
        return;

    m_minePrevOwner[mine] = m_mineOwner[mine];
    m_mineOwner[mine] = teamIdx;

    SendMineWorldStates(mine);

    SpawnEvent(BG_AV_MINE_EVENT + mine, teamIdx, true);
    SpawnEvent(BG_AV_MINE_BOSSES + mine, teamIdx, true);

    if (teamIdx != TEAM_INDEX_NEUTRAL)
    {
        PlaySoundToAll((teamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_GOOD : BG_AV_SOUND_HORDE_GOOD);
        m_mineReclaimTimer[mine] = BG_AV_MINE_RECLAIM_TIMER;
        SendYell2ToAll(LANG_BG_AV_MINE_TAKEN, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0),
                       (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE,
                       (mine == BG_AV_NORTH_MINE) ? LANG_BG_AV_MINE_NORTH : LANG_BG_AV_MINE_SOUTH);
    }
}

// Check if player is allowed to proceed with the mine quest
bool BattleGroundAV::CanPlayerDoMineQuest(uint32 gameobjectEntry, Team team)
{
    if (gameobjectEntry == BG_AV_GO_IRONDEEP_SUPPLIES)
        return (m_mineOwner[BG_AV_NORTH_MINE] == GetTeamIndexByTeamId(team));
    if (gameobjectEntry == BG_AV_GO_COLDTOOTH_SUPPLIES)
        return (m_mineOwner[BG_AV_SOUTH_MINE] == GetTeamIndexByTeamId(team));
    return true;                                            // cause it's no mine'object it is ok if this is true
}

/// will spawn and despawn creatures around a node
/// more a wrapper around spawnevent cause graveyards are special
void BattleGroundAV::PopulateNode(AVNodeIds node)
{
    PvpTeamIndex teamIdx = m_nodes[node].owner;

    if (IsGrave(node) && teamIdx != TEAM_INDEX_NEUTRAL)
    {
        uint32 graveDefenderType;
        if (m_teamQuestStatus[teamIdx][0] < 500)
            graveDefenderType = 0;
        else if (m_teamQuestStatus[teamIdx][0] < 1000)
            graveDefenderType = 1;
        else if (m_teamQuestStatus[teamIdx][0] < 1500)
            graveDefenderType = 2;
        else
            graveDefenderType = 3;

        if (m_nodes[node].state == POINT_CONTROLLED) // we can spawn the current owner event
            SpawnEvent(BG_AV_MAX_NODES + node, teamIdx * BG_AV_MAX_GRAVETYPES + graveDefenderType, true);
        else // we despawn the event from the prevowner
            SpawnEvent(BG_AV_MAX_NODES + node, m_nodes[node].prevOwner * BG_AV_MAX_GRAVETYPES + graveDefenderType, false);
    }
    SpawnEvent(node, (teamIdx * BG_AV_MAX_STATES) + m_nodes[node].state, true);
}

// Handle banner click
void BattleGroundAV::HandlePlayerClickedOnFlag(Player* source, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;
    DEBUG_LOG("BattleGroundAV: using gameobject %i", target_obj->GetEntry());
    uint8 event = (sBattleGroundMgr.GetGameObjectEventIndex(target_obj->GetGUIDLow())).event1;
    if (event >= BG_AV_MAX_NODES)                           // not a node
        return;
    AVNodeIds node = AVNodeIds(event);

    switch ((sBattleGroundMgr.GetGameObjectEventIndex(target_obj->GetGUIDLow())).event2 % BG_AV_MAX_STATES)
    {
        case POINT_CONTROLLED:
            ProcessPlayerAssaultsPoint(source, node);
            break;
        case POINT_ASSAULTED:
            ProcessPlayerDefendsPoint(source, node);
            break;
        default:
            break;
    }
}

// Handle node defend
void BattleGroundAV::ProcessPlayerDefendsPoint(Player* player, AVNodeIds node)
{
    MANGOS_ASSERT(GetStatus() == STATUS_IN_PROGRESS);

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(player->GetTeam());

    if (m_nodes[node].owner == teamIdx || m_nodes[node].state != POINT_ASSAULTED)
        return;
    if (m_nodes[node].totalOwner == TEAM_INDEX_NEUTRAL)     // initial snowfall capture
    {
        // until snowfall doesn't belong to anyone it is better handled in assault - code (best would be to have a special function
        // for neutral nodes.. but doing this just for snowfall will be a bit to much i think
        MANGOS_ASSERT(node == BG_AV_NODES_SNOWFALL_GRAVE);  // currently the only neutral grave
        ProcessPlayerAssaultsPoint(player, node);
        return;
    }

    DEBUG_LOG("BattleGroundAV: player defends node: %i", node);
    if (m_nodes[node].prevOwner != teamIdx)
    {
        sLog.outError("BattleGroundAV: player defends point which doesn't belong to his team %i", node);
        return;
    }

    DefendNode(node, teamIdx);                              // set the right variables for nodeinfo
    PopulateNode(node);                                     // spawn node-creatures (defender for example)
    UpdateNodeWorldState(node);                             // send new mapicon to the player

    if (IsTower(node))
    {
        SendYell2ToAll(LANG_BG_AV_TOWER_DEFENDED, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0),
                        GetNodeMessageId(node),
                       (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
        UpdatePlayerScore(player, SCORE_TOWERS_DEFENDED, 1);
        PlaySoundToAll(BG_AV_SOUND_BOTH_TOWER_DEFEND);
    }
    else
    {
        SendYell2ToAll(LANG_BG_AV_GRAVE_DEFENDED, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0),
                        GetNodeMessageId(node),
                       (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
        UpdatePlayerScore(player, SCORE_GRAVEYARDS_DEFENDED, 1);
        // update the statistic for the defending player
        PlaySoundToAll((teamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_GOOD : BG_AV_SOUND_HORDE_GOOD);
    }
}

enum CreditMarkers
{
    NPC_PVP_GRAVEYARD_CREDIT_MARKER = 13756,
    NPC_PVP_TOWER_CREDIT_MARKER = 13778,
};

// Handle node assault
void BattleGroundAV::ProcessPlayerAssaultsPoint(Player* player, AVNodeIds node)
{
    PvpTeamIndex teamIdx  = GetTeamIndexByTeamId(player->GetTeam());
    DEBUG_LOG("BattleGroundAV: player assaults node %i", node);
    if (m_nodes[node].owner == teamIdx || teamIdx == m_nodes[node].totalOwner)
        return;

    bool isTower = IsTower(node);

    AssaultNode(node, teamIdx);                             // update nodeinfo variables
    UpdateNodeWorldState(node);                             // send mapicon
    if (!isTower)
    {
        // before despawn of GY spirit healer - need to TP players - must be after AssaultNode
        // opposite team of current player
        Creature* spiritHealer = GetClosestCreatureWithEntry(player, teamIdx == TEAM_INDEX_ALLIANCE ? BG_NPC_SPIRIT_GUIDE_HORDE : BG_NPC_SPIRIT_GUIDE_ALLIANCE, 100.f);

        if (spiritHealer)
            spiritHealer->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, spiritHealer, spiritHealer);
    }
    PopulateNode(node);

    if (isTower)
    {
        SendYell2ToAll(LANG_BG_AV_TOWER_ASSAULTED, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0),
                        GetNodeMessageId(node),
                       (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
        UpdatePlayerScore(player, SCORE_TOWERS_ASSAULTED, 1);
        // these markers are spawned on retail on each GY/Tower and then their guid is used
        // TODO: Add all markers and fetch them for rewardSource instead
        player->RewardPlayerAndGroupAtEventCredit(NPC_PVP_TOWER_CREDIT_MARKER, player);
    }
    else
    {

        SendYell2ToAll(LANG_BG_AV_GRAVE_ASSAULTED, LANG_UNIVERSAL, GetSingleCreatureGuid(BG_AV_HERALD, 0),
                        GetNodeMessageId(node),
                       (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);

        // update the statistic for the assaulting player
        UpdatePlayerScore(player, SCORE_GRAVEYARDS_ASSAULTED, 1);
        player->RewardPlayerAndGroupAtEventCredit(NPC_PVP_GRAVEYARD_CREDIT_MARKER, player);
    }

    PlaySoundToAll((teamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_ASSAULTS : BG_AV_SOUND_HORDE_ASSAULTS);
}

void BattleGroundAV::FillInitialWorldStates(WorldPacket& data, uint32& count)
{
    for (uint8 i = 0; i < BG_AV_MAX_NODES; ++i)
    {
        for (uint8 j = 0; j < BG_AV_MAX_STATES; ++j)
        {
            bool stateok = (m_nodes[i].state == j);
            FillInitialWorldState(data, count, avMineWorldStates[i][GetWorldStateType(j, TEAM_INDEX_ALLIANCE)], m_nodes[i].owner == TEAM_INDEX_ALLIANCE && stateok);
            FillInitialWorldState(data, count, avMineWorldStates[i][GetWorldStateType(j, TEAM_INDEX_HORDE)], m_nodes[i].owner == TEAM_INDEX_HORDE && stateok);
        }
    }

    if (m_nodes[BG_AV_NODES_SNOWFALL_GRAVE].owner == TEAM_INDEX_NEUTRAL)    // cause neutral teams aren't handled generic
        FillInitialWorldState(data, count, BG_AV_STATE_GY_SNOWFALL_N, WORLD_STATE_ADD);

    FillInitialWorldState(data, count, BG_AV_STATE_SCORE_A, m_teamScores[TEAM_INDEX_ALLIANCE]);
    FillInitialWorldState(data, count, BG_AV_STATE_SCORE_H, m_teamScores[TEAM_INDEX_HORDE]);

    if (GetStatus() == STATUS_IN_PROGRESS)                  // only if game is running the teamscores are displayed
    {
        FillInitialWorldState(data, count, BG_AV_STATE_SCORE_SHOW_A, WORLD_STATE_ADD);
        FillInitialWorldState(data, count, BG_AV_STATE_SCORE_SHOW_H, WORLD_STATE_ADD);
    }
    else
    {
        FillInitialWorldState(data, count, BG_AV_STATE_SCORE_SHOW_A, WORLD_STATE_REMOVE);
        FillInitialWorldState(data, count, BG_AV_STATE_SCORE_SHOW_H, WORLD_STATE_REMOVE);
    }

    FillInitialWorldState(data, count, avMineWorldStates[BG_AV_NORTH_MINE][m_mineOwner[BG_AV_NORTH_MINE]], WORLD_STATE_ADD);
    if (m_mineOwner[BG_AV_NORTH_MINE] != m_minePrevOwner[BG_AV_NORTH_MINE])
        FillInitialWorldState(data, count, avMineWorldStates[BG_AV_NORTH_MINE][m_minePrevOwner[BG_AV_NORTH_MINE]], WORLD_STATE_REMOVE);

    FillInitialWorldState(data, count, avMineWorldStates[BG_AV_SOUTH_MINE][m_mineOwner[BG_AV_SOUTH_MINE]], WORLD_STATE_ADD);
    if (m_mineOwner[BG_AV_SOUTH_MINE] != m_minePrevOwner[BG_AV_SOUTH_MINE])
        FillInitialWorldState(data, count, avMineWorldStates[BG_AV_SOUTH_MINE][m_minePrevOwner[BG_AV_SOUTH_MINE]], WORLD_STATE_REMOVE);
}

// Update node world state
void BattleGroundAV::UpdateNodeWorldState(AVNodeIds node)
{
    UpdateWorldState(avMineWorldStates[node][GetWorldStateType(m_nodes[node].state, m_nodes[node].owner)], WORLD_STATE_ADD);

    if (m_nodes[node].prevOwner == TEAM_INDEX_NEUTRAL)      // currently only snowfall is supported as neutral node
        UpdateWorldState(BG_AV_STATE_GY_SNOWFALL_N, WORLD_STATE_REMOVE);
    else
        UpdateWorldState(avMineWorldStates[node][GetWorldStateType(m_nodes[node].prevState, m_nodes[node].prevOwner)], WORLD_STATE_REMOVE);
}

// Update mine world state
void BattleGroundAV::SendMineWorldStates(uint32 mine)
{
    MANGOS_ASSERT(mine == BG_AV_NORTH_MINE || mine == BG_AV_SOUTH_MINE);

    UpdateWorldState(avMineWorldStates[mine][m_mineOwner[mine]], WORLD_STATE_ADD);

    if (m_mineOwner[mine] != m_minePrevOwner[mine])
        UpdateWorldState(avMineWorldStates[mine][m_minePrevOwner[mine]], WORLD_STATE_REMOVE);
}

WorldSafeLocsEntry const* BattleGroundAV::GetClosestGraveYard(Player* plr)
{
    float x = plr->GetPositionX();
    float y = plr->GetPositionY();
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(plr->GetTeam());
    WorldSafeLocsEntry const* good_entry = nullptr;
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // Is there any occupied node for this team?
        float mindist = 9999999.0f;
        for (uint8 i = BG_AV_NODES_FIRSTAID_STATION; i <= BG_AV_NODES_FROSTWOLF_HUT; ++i)
        {
            if (m_nodes[i].owner != teamIdx || m_nodes[i].state != POINT_CONTROLLED)
                continue;
            WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(avGraveyardEntries[i]);
            if (!entry)
                continue;

            float dist = (entry->x - x) * (entry->x - x) + (entry->y - y) * (entry->y - y);
            if (mindist > dist)
            {
                mindist = dist;
                good_entry = entry;
            }
        }
    }
    // If not, place ghost in the starting-cave
    if (!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(avGraveyardEntries[teamIdx + 7]);

    return good_entry;
}

int32 BattleGroundAV::GetNodeMessageId(AVNodeIds node) const
{
    switch (node)
    {
        case BG_AV_NODES_FIRSTAID_STATION:  return LANG_BG_AV_NODE_GRAVE_STORM_AID;
        case BG_AV_NODES_DUNBALDAR_SOUTH:   return LANG_BG_AV_NODE_TOWER_DUN_S;
        case BG_AV_NODES_DUNBALDAR_NORTH:   return LANG_BG_AV_NODE_TOWER_DUN_N;
        case BG_AV_NODES_STORMPIKE_GRAVE:   return LANG_BG_AV_NODE_GRAVE_STORMPIKE;
        case BG_AV_NODES_ICEWING_BUNKER:    return LANG_BG_AV_NODE_TOWER_ICEWING;
        case BG_AV_NODES_STONEHEART_GRAVE:  return LANG_BG_AV_NODE_GRAVE_STONE;
        case BG_AV_NODES_STONEHEART_BUNKER: return LANG_BG_AV_NODE_TOWER_STONE;
        case BG_AV_NODES_SNOWFALL_GRAVE:    return LANG_BG_AV_NODE_GRAVE_SNOW;
        case BG_AV_NODES_ICEBLOOD_TOWER:    return LANG_BG_AV_NODE_TOWER_ICE;
        case BG_AV_NODES_ICEBLOOD_GRAVE:    return LANG_BG_AV_NODE_GRAVE_ICE;
        case BG_AV_NODES_TOWER_POINT:       return LANG_BG_AV_NODE_TOWER_POINT;
        case BG_AV_NODES_FROSTWOLF_GRAVE:   return LANG_BG_AV_NODE_GRAVE_FROST;
        case BG_AV_NODES_FROSTWOLF_ETOWER:  return LANG_BG_AV_NODE_TOWER_FROST_E;
        case BG_AV_NODES_FROSTWOLF_WTOWER:  return LANG_BG_AV_NODE_TOWER_FROST_W;
        case BG_AV_NODES_FROSTWOLF_HUT:     return LANG_BG_AV_NODE_GRAVE_FROST_HUT;
        default: return 0;
    }
}

void BattleGroundAV::AssaultNode(AVNodeIds node, PvpTeamIndex teamIdx)
{
    MANGOS_ASSERT(m_nodes[node].totalOwner != teamIdx);
    MANGOS_ASSERT(m_nodes[node].owner != teamIdx);
    // only assault an assaulted node if no totalowner exists:
    MANGOS_ASSERT(m_nodes[node].state != POINT_ASSAULTED || m_nodes[node].totalOwner == TEAM_INDEX_NEUTRAL);
    // the timer gets another time, if the previous owner was 0 == Neutral
    m_nodes[node].timer      = (m_nodes[node].prevOwner != TEAM_INDEX_NEUTRAL) ? BG_AV_CAPTIME : BG_AV_SNOWFALL_FIRSTCAP;
    m_nodes[node].prevOwner  = m_nodes[node].owner;
    m_nodes[node].owner      = teamIdx;
    m_nodes[node].prevState  = m_nodes[node].state;
    m_nodes[node].state      = POINT_ASSAULTED;
}

void BattleGroundAV::DestroyNode(AVNodeIds node)
{
    MANGOS_ASSERT(m_nodes[node].state == POINT_ASSAULTED);

    m_nodes[node].totalOwner = m_nodes[node].owner;
    m_nodes[node].prevOwner  = m_nodes[node].owner;
    m_nodes[node].prevState  = m_nodes[node].state;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].timer      = 0;
}

void BattleGroundAV::InitNode(AVNodeIds node, PvpTeamIndex teamIdx, bool isTower)
{
    m_nodes[node].totalOwner = teamIdx;
    m_nodes[node].owner      = teamIdx;
    m_nodes[node].prevOwner  = teamIdx;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].prevState  = m_nodes[node].state;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].timer      = 0;
    m_nodes[node].isTower    = isTower;
    m_activeEvents[node] = teamIdx * BG_AV_MAX_STATES + m_nodes[node].state;

    if (IsGrave(node))                                      // grave-creatures are special cause of a quest
        m_activeEvents[node + BG_AV_MAX_NODES]  = teamIdx * BG_AV_MAX_GRAVETYPES;
}

void BattleGroundAV::DefendNode(AVNodeIds node, PvpTeamIndex teamIdx)
{
    MANGOS_ASSERT(m_nodes[node].totalOwner == teamIdx);
    MANGOS_ASSERT(m_nodes[node].owner != teamIdx);
    MANGOS_ASSERT(m_nodes[node].state != POINT_CONTROLLED);
    m_nodes[node].prevOwner  = m_nodes[node].owner;
    m_nodes[node].owner      = teamIdx;
    m_nodes[node].prevState  = m_nodes[node].state;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].timer      = 0;
}

void BattleGroundAV::Reset()
{
    BattleGround::Reset();
    // set the reputation and honor variables:
    bool isBgWeekend = BattleGroundMgr::IsBgWeekend(GetTypeId());

    m_honorMapComplete    = (isBgWeekend) ? BG_AV_KILL_MAP_COMPLETE_HOLIDAY : BG_AV_KILL_MAP_COMPLETE;
    m_repTowerDestruction = (isBgWeekend) ? BG_AV_REP_TOWER_HOLIDAY         : BG_AV_REP_TOWER;
    m_repCaptain          = (isBgWeekend) ? BG_AV_REP_CAPTAIN_HOLIDAY       : BG_AV_REP_CAPTAIN;
    m_repBoss             = (isBgWeekend) ? BG_AV_REP_BOSS_HOLIDAY          : BG_AV_REP_BOSS;
    m_repOwnedGrave       = (isBgWeekend) ? BG_AV_REP_OWNED_GRAVE_HOLIDAY   : BG_AV_REP_OWNED_GRAVE;
    m_repSurviveCaptain   = (isBgWeekend) ? BG_AV_REP_SURVIVING_CAPTAIN_HOLIDAY : BG_AV_REP_SURVIVING_CAPTAIN;
    m_repSurviveTower     = (isBgWeekend) ? BG_AV_REP_SURVIVING_TOWER_HOLIDAY : BG_AV_REP_SURVIVING_TOWER;
    m_repOwnedMine        = (isBgWeekend) ? BG_AV_REP_OWNED_MINE_HOLIDAY    : BG_AV_REP_OWNED_MINE;

    // initialize team variables
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        for (uint8 j = 0; j < 9; ++j)                       // 9 quests getting tracked
            m_teamQuestStatus[i][j] = 0;

        m_teamScores[i]         = BG_AV_SCORE_INITIAL_POINTS;
        m_wasInformedNearLose[i] = false;
        m_activeEvents[BG_AV_NODE_CAPTAIN_DEAD_A + i] = BG_EVENT_NONE;
    }

    // initialize mine variables
    for (uint8 i = 0; i < BG_AV_MAX_MINES; ++i)
    {
        m_mineOwner[i] = TEAM_INDEX_NEUTRAL;
        m_minePrevOwner[i] = m_mineOwner[i];
        m_activeEvents[BG_AV_MINE_BOSSES + i] = TEAM_INDEX_NEUTRAL;
        m_activeEvents[BG_AV_MINE_EVENT + i] = TEAM_INDEX_NEUTRAL;
        m_mineTimer[i] = BG_AV_MINE_TICK_TIMER;
    }

    m_activeEvents[BG_AV_CAPTAIN_A] = 0;
    m_activeEvents[BG_AV_CAPTAIN_H] = 0;
    m_activeEvents[BG_AV_HERALD] = 0;
    m_activeEvents[BG_AV_BOSS_A] = 0;
    m_activeEvents[BG_AV_BOSS_H] = 0;

    for (AVNodeIds i = BG_AV_NODES_DUNBALDAR_SOUTH; i <= BG_AV_NODES_FROSTWOLF_WTOWER; ++i)  // towers
        m_activeEvents[BG_AV_MARSHAL_A_SOUTH + i - BG_AV_NODES_DUNBALDAR_SOUTH] = 0;

    for (AVNodeIds i = BG_AV_NODES_FIRSTAID_STATION; i <= BG_AV_NODES_STONEHEART_GRAVE; ++i)  // alliance graves
        InitNode(i, TEAM_INDEX_ALLIANCE, false);

    for (AVNodeIds i = BG_AV_NODES_DUNBALDAR_SOUTH; i <= BG_AV_NODES_STONEHEART_BUNKER; ++i)  // alliance towers
        InitNode(i, TEAM_INDEX_ALLIANCE, true);

    for (AVNodeIds i = BG_AV_NODES_ICEBLOOD_GRAVE; i <= BG_AV_NODES_FROSTWOLF_HUT; ++i)       // horde graves
        InitNode(i, TEAM_INDEX_HORDE, false);

    for (AVNodeIds i = BG_AV_NODES_ICEBLOOD_TOWER; i <= BG_AV_NODES_FROSTWOLF_WTOWER; ++i)    // horde towers
        InitNode(i, TEAM_INDEX_HORDE, true);

    InitNode(BG_AV_NODES_SNOWFALL_GRAVE, TEAM_INDEX_NEUTRAL, false);                            // give snowfall neutral owner
}

Team BattleGroundAV::GetPrematureWinner()
{
    int32 hordeScore = m_teamScores[TEAM_INDEX_HORDE];
    int32 allianceScore = m_teamScores[TEAM_INDEX_ALLIANCE];

    if (hordeScore > allianceScore)
        return HORDE;
    if (allianceScore > hordeScore)
        return ALLIANCE;

    // If the values are equal, fall back to number of players on each team
    return BattleGround::GetPrematureWinner();
}

bool BattleGroundAV::IsConditionFulfilled(Player const* source, uint32 conditionId, WorldObject const* conditionSource, uint32 conditionSourceType)
{
    // ToDo: check condition for mine loot; check conditions for quest end script

    return false;
}
