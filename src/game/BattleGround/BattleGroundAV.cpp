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
#include "Server/WorldPacket.h"
#include "Globals/ObjectMgr.h"
#include "AI/ScriptDevAI/include/sc_grid_searchers.h"

BattleGroundAV::BattleGroundAV(): m_mineYellTimer(BG_AV_MINE_YELL), m_honorMapComplete(0), m_repTowerDestruction(0), m_repCaptain(0), m_repBoss(0), m_repOwnedGrave(0), m_repOwnedMine(0), m_repSurviveCaptain(0), m_repSurviveTower(0)
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
        {
            // cast quest complete spells
            CastSpellOnTeam(BG_AV_SPELL_COMPLETE_AV_QUEST, HORDE);

            RewardReputationToTeam(BG_AV_FACTION_ID_FROSTWOLF, m_repBoss, HORDE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_BOSS), HORDE);

            DoSendYellToTeam(TEAM_INDEX_ALLIANCE, LANG_BG_AV_A_GENERAL_DEAD, 0);

            EndBattleGround(HORDE);
            break;
        }
        case BG_AV_NPC_DREKTHAR:
        {
            // cast quest complete spells
            CastSpellOnTeam(BG_AV_SPELL_COMPLETE_AV_QUEST, ALLIANCE);

            RewardReputationToTeam(BG_AV_FACTION_ID_STORMPIKE, m_repBoss, ALLIANCE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_BOSS), ALLIANCE);

            DoSendYellToTeam(TEAM_INDEX_HORDE, LANG_BG_AV_H_GENERAL_DEAD, 0);

            EndBattleGround(ALLIANCE);
            break;
        }
        case BG_AV_NPC_CAPTAIN_STONEHEARTH:
        {
            RewardReputationToTeam(BG_AV_FACTION_ID_FROSTWOLF, m_repCaptain, HORDE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_CAPTAIN), HORDE);

            UpdateScore(TEAM_INDEX_ALLIANCE, (-1) * BG_AV_RES_CAPTAIN);

            // spawn fire objects
            SpawnEvent(BG_AV_NODE_CAPTAIN_DEAD_A, 0, true);
            break;
        }
        case BG_AV_NPC_CAPTAIN_GALVANGAR:
        {
            RewardReputationToTeam(BG_AV_FACTION_ID_STORMPIKE, m_repCaptain, ALLIANCE);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_CAPTAIN), ALLIANCE);

            UpdateScore(TEAM_INDEX_HORDE, (-1) * BG_AV_RES_CAPTAIN);

            // spawn fire objects
            SpawnEvent(BG_AV_NODE_CAPTAIN_DEAD_H, 0, true);
            break;
        }
        case BG_AV_NPC_MORLOCH:
        case BG_AV_NPC_UMI_THORSON:
        case BG_AV_NPC_KEETAR:
            ChangeMineOwner(BG_AV_IRONDEEP_MINE_ID, GetTeamIndexByTeamId(killer->GetTeam()));
            break;
        case BG_AV_NPC_TASKMASTER_SNIVVLE:
        case BG_AV_NPC_AGGI_RUMBLESTOMP:
        case BG_AV_NPC_MASHA_SWIFTCUT:
            ChangeMineOwner(BG_AV_COLDTOOTH_MINE_ID, GetTeamIndexByTeamId(killer->GetTeam()));
            break;
    }
}

void BattleGroundAV::HandleCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case BG_AV_NPC_ICEWING_MARSHAL:
        case BG_AV_NPC_STONEHEARTH_MARSHAL:
        case BG_AV_NPC_NORTH_MARSHAL:
        case BG_AV_NPC_SOUTH_MARSHAL:
        case BG_AV_NPC_CAPTAIN_STONEHEARTH:
        case BG_AV_NPC_EAST_WARMASTER:
        case BG_AV_NPC_ICEBLOOD_WARMASTER:
        case BG_AV_NPC_TOWERPOINT_WARMASTER:
        case BG_AV_NPC_WEST_WARMASTER:
        case BG_AV_NPC_CAPTAIN_GALVANGAR:
        case BG_AV_NPC_STORMPIKE_HERALD:
        case BG_AV_NPC_FROSTWOLF_HERALD:
        case BG_AV_NPC_HERALD:
        case BG_AV_NPC_MORLOCH:
        case BG_AV_NPC_TASKMASTER_SNIVVLE:
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
    }
}

void BattleGroundAV::HandleGameObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case BG_AV_GO_IRONDEEP_SUPPLIES:
        case BG_AV_GO_COLDTOOTH_SUPPLIES:
        case BG_AV_GO_FROSTWOLF_BANNER:
        case BG_AV_GO_STORMPIKE_BANNER:
            m_goEntryGuidStore[go->GetEntry()] = go->GetObjectGuid();
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
                for (uint8 i = BG_AV_NODES_FIRSTAID_STATION; i < BG_AV_NODES_FROSTWOLF_HUT; ++i)
                    if (m_nodes[i].owner == teamIdx && m_nodes[i].state == POINT_CONTROLLED)
                        PopulateNode(AVNodeIds(i));
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
        RewardReputationToTeam((player->GetTeam() == ALLIANCE) ? BG_AV_FACTION_ID_STORMPIKE : BG_AV_FACTION_ID_FROSTWOLF, reputation, player->GetTeam());
}

// Method that updates team score
void BattleGroundAV::UpdateScore(PvpTeamIndex teamIdx, int32 points)
{
    int32 worldStateId = teamIdx == TEAM_INDEX_ALLIANCE ? BG_AV_STATE_SCORE_A : BG_AV_STATE_SCORE_H;

    // note: to remove reinforcements points must be negative, for adding reinforcements points must be positive
    int32 newVal = std::max(GetBgMap()->GetVariableManager().GetVariable(worldStateId) + points, 0);
    GetBgMap()->GetVariableManager().SetVariable(worldStateId, newVal);

    if (points < 0)
    {
        // end battleground and set the winner
        if (newVal < 1)
        {
            // other team will win:
            EndBattleGround((teamIdx == TEAM_INDEX_ALLIANCE) ? HORDE : ALLIANCE);
        }
        // inform the players about getting close to losing
        else if (!m_wasInformedNearLose[teamIdx] && newVal < BG_AV_SCORE_NEAR_LOSE)
        {
            SendMessageToAll((teamIdx == TEAM_INDEX_HORDE) ? LANG_BG_AV_H_NEAR_LOSE : LANG_BG_AV_A_NEAR_LOSE, CHAT_MSG_BG_SYSTEM_NEUTRAL);

            PlaySoundToAll(BG_AV_SOUND_NEAR_LOSE);
            m_wasInformedNearLose[teamIdx] = true;
        }
    }
}

void BattleGroundAV::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (m_mineYellTimer)
    {
        if (m_mineYellTimer <= diff)
        {
            if (Creature* morloch = GetSingleCreatureFromStorage(BG_AV_NPC_MORLOCH))
                SendBcdToTeam(YELL_MORLOCH_MINE, CHAT_MSG_MONSTER_YELL, morloch, ALLIANCE);
            if (Creature* snivvle = GetSingleCreatureFromStorage(BG_AV_NPC_TASKMASTER_SNIVVLE))
                SendBcdToTeam(YELL_SNIVVLE_MINE, CHAT_MSG_MONSTER_YELL, snivvle, HORDE);
            m_mineYellTimer = 0;
        }
        else
            m_mineYellTimer -= diff;
    }

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // add points from mine owning, and look if the neutral team can reclaim the mine
    for (uint8 mine = 0; mine < BG_AV_MAX_MINES; ++mine)
    {
        if (m_mineOwner[mine] != TEAM_INDEX_NEUTRAL)
        {
            if (m_mineTimer[mine] < diff)
            {
                UpdateScore(PvpTeamIndex(m_mineOwner[mine]), 1);
                m_mineTimer[mine] = BG_AV_MINE_TICK_TIMER;
            }
            else
                m_mineTimer[mine] -= diff;

            if (m_mineReclaimTimer[mine] > diff)
                m_mineReclaimTimer[mine] -= diff;
            else
                ChangeMineOwner(AVMineIds(mine), TEAM_INDEX_NEUTRAL);
        }
    }

    // looks for all timers of the nodes and destroy the building (for graveyards the building wont get destroyed, it goes just to the other team
    for (uint8 i = 0; i < BG_AV_MAX_NODES; ++i)
    {
        if (m_nodes[i].state == POINT_ASSAULTED)
        {
            if (m_nodes[i].timer > diff)
                m_nodes[i].timer -= diff;
            else
                ProcessPlayerDestroyedPoint(AVNodeIds(i));
        }
    }
}

void BattleGroundAV::StartingEventOpenDoors()
{
    GetBgMap()->GetVariableManager().SetVariable(BG_AV_STATE_SCORE_SHOW_H, WORLD_STATE_ADD);
    GetBgMap()->GetVariableManager().SetVariable(BG_AV_STATE_SCORE_SHOW_A, WORLD_STATE_ADD);

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
    uint32 gravesOwned[PVP_TEAM_COUNT]    = {0, 0};
    uint32 minesOwned[PVP_TEAM_COUNT]     = {0, 0};

    // graves all controlled
    for (uint8 i = 0; i < BG_AV_MAX_NODES; ++i)
        if (m_nodes[i].state == POINT_CONTROLLED && m_nodes[i].owner != TEAM_INDEX_NEUTRAL)
            ++gravesOwned[m_nodes[i].owner];

    for (auto& i : m_mineOwner)
        if (i != TEAM_INDEX_NEUTRAL)
            ++minesOwned[i];

    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        Team team = GetTeamIdByTeamIndex(PvpTeamIndex(i));

        uint32 bonusHonor = GetBonusHonorFromKill(m_homeTowersControlled[i] * BG_AV_KILL_SURVIVING_TOWER);
        uint32 bonusReputation = m_homeTowersControlled[i] * m_repSurviveTower;

        // bonus tower reputation and honor
        if (bonusReputation)
            RewardReputationToTeam(avTeamFactions[i], m_homeTowersControlled[i] * m_repSurviveTower, team);
        if (bonusHonor)
            RewardHonorToTeam(bonusHonor, team);

        DEBUG_LOG("BattleGroundAV: Team %u wins the battleground. Controlled towers: %u bonus towers honor: %u bonus towers reputation: %u", i, m_homeTowersControlled[i], bonusHonor, bonusReputation);

        // bonus graveyards and mines reputation
        if (gravesOwned[i])
            RewardReputationToTeam(avTeamFactions[i], gravesOwned[i] * m_repOwnedGrave, team);
        if (minesOwned[i])
            RewardReputationToTeam(avTeamFactions[i], minesOwned[i] * m_repOwnedMine, team);

        // captain survived; bonus reputation and honor
        Creature* captain = GetSingleCreatureFromStorage(avTeamCaptains[i]);
        if (captain && captain->IsAlive())
        {
            RewardReputationToTeam(avTeamFactions[i], m_repSurviveCaptain, team);
            RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_SURVIVING_CAPTAIN), team);
        }
    }

    // both teams get honor for completing the BG
    if (m_honorMapComplete)
    {
        RewardHonorToTeam(m_honorMapComplete, ALLIANCE);
        RewardHonorToTeam(m_honorMapComplete, HORDE);
    }

    BattleGround::EndBattleGround(winner);

    if (Creature* herald = GetSingleCreatureFromStorage(BG_AV_NPC_HERALD))
    {
        if (winner == ALLIANCE)
        {
            SendBcdToAll(YELL_FROSTWOLF_DEAD, CHAT_MSG_MONSTER_YELL, herald);
            SendBcdToAll(YELL_ALLIANCE_WINS, CHAT_MSG_MONSTER_YELL, herald);
        }
        else
        {
            SendBcdToAll(YELL_STORMPIKE_DEAD, CHAT_MSG_MONSTER_YELL, herald);
            SendBcdToAll(YELL_HORDE_WINS, CHAT_MSG_MONSTER_YELL, herald);
        }
    }

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
    PvpTeamIndex otherTeamIdx = GetOtherTeamIndex(ownerTeamIdx);
    Team ownerTeam = GetTeamIdByTeamIndex(ownerTeamIdx);

    bool isTower = m_nodes[node].graveyardId == 0;
    uint32 newState = ownerTeam == ALLIANCE ? avNodeWorldStates[node].worldStateAlly : avNodeWorldStates[node].worldStateHorde;

    // despawn banner
    DestroyNode(node);
    PopulateNode(node);
    UpdateNodeWorldState(node, newState);

    if (isTower)
    {
        uint8 tmp = node - BG_AV_NODES_DUNBALDAR_SOUTH;

        // despawn marshal (one of those guys protecting the boss)
        SpawnEvent(BG_AV_MARSHAL_A_SOUTH + tmp, 0, false);

        UpdateScore(GetOtherTeamIndex(ownerTeamIdx), (-1) * BG_AV_RES_TOWER);
        RewardReputationToTeam((ownerTeam == ALLIANCE) ? BG_AV_FACTION_ID_STORMPIKE : BG_AV_FACTION_ID_STORMPIKE, m_repTowerDestruction, ownerTeam);
        RewardHonorToTeam(GetBonusHonorFromKill(BG_AV_KILL_TOWER), ownerTeam);

        DoSendYellToTeam(ownerTeamIdx, LANG_BG_AV_TOWER_TAKEN, node);

        // update tower counters
        ++m_enemyTowersDestroyed[ownerTeamIdx];
        --m_homeTowersControlled[otherTeamIdx];
    }
    else
    {
        DoSendYellToTeam(ownerTeamIdx, LANG_BG_AV_GRAVE_TAKEN, node);

        // setup graveyard to new team
        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(avNodeDefaults[node].graveyardId, BG_AV_ZONE_MAIN, GetTeamIdByTeamIndex(ownerTeamIdx));
    }
}

// Process mine owner change; mineId = 0 => Irondeep Mine; mineId = 1 => Coldtooth Mine;
void BattleGroundAV::ChangeMineOwner(AVMineIds mineId, PvpTeamIndex newOwnerTeamIdx)
{
    // TODO changing the owner should result in setting respawntime to infinite for current creatures (they should fight the new ones), spawning new mine owners creatures and changing the chest - objects so that the current owning team can use them
    if (m_mineOwner[mineId] == newOwnerTeamIdx)
        return;

    DEBUG_LOG("BattleGroundAV: Mine id %i was claimed by team index %u", mineId, newOwnerTeamIdx);

    // update world states
    GetBgMap()->GetVariableManager().SetVariable(avMineWorldStates[mineId][m_mineOwner[mineId]], WORLD_STATE_REMOVE);
    m_mineOwner[mineId] = newOwnerTeamIdx;
    GetBgMap()->GetVariableManager().SetVariable(avMineWorldStates[mineId][m_mineOwner[mineId]], WORLD_STATE_ADD);

    SpawnEvent(BG_AV_MINE_EVENT + mineId, newOwnerTeamIdx, true);
    SpawnEvent(BG_AV_MINE_BOSSES + mineId, newOwnerTeamIdx, true);

    if (newOwnerTeamIdx == TEAM_INDEX_NEUTRAL)
        return;

    m_mineReclaimTimer[mineId] = BG_AV_MINE_RECLAIM_TIMER;
    m_mineTimer[mineId] = BG_AV_MINE_TICK_TIMER;

    DoSendYellToTeam(newOwnerTeamIdx, LANG_BG_AV_MINE_TAKEN, 0, true, (mineId == BG_AV_IRONDEEP_MINE_ID) ? LANG_BG_AV_MINE_NORTH : LANG_BG_AV_MINE_SOUTH);
    PlaySoundToAll((newOwnerTeamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_GOOD : BG_AV_SOUND_HORDE_GOOD);

    Team team = GetTeamIdByTeamIndex(newOwnerTeamIdx);

    // quest kill credit
    for (auto& m_player : m_players)
    {
        if (!m_player.first)
            continue;

        Player* player = sObjectMgr.GetPlayer(m_player.first);
        if (player && player->GetTeam() == team)
            player->KilledMonsterCredit(BG_AV_NPC_PVP_MINE_CREDIT);
    }
}

// Check if player is allowed to proceed with the mine quest
bool BattleGroundAV::CanPlayerDoMineQuest(uint32 goEntry, Team team)
{
    if (goEntry == BG_AV_GO_IRONDEEP_SUPPLIES)
        return (m_mineOwner[BG_AV_IRONDEEP_MINE_ID] == GetTeamIndexByTeamId(team));
    else if (goEntry == BG_AV_GO_COLDTOOTH_SUPPLIES)
        return (m_mineOwner[BG_AV_COLDTOOTH_MINE_ID] == GetTeamIndexByTeamId(team));

    return true;                                            // cause it's no mine'object it is ok if this is true
}

/// will spawn and despawn creatures around a node
/// more a wrapper around spawnevent cause graveyards are special
void BattleGroundAV::PopulateNode(AVNodeIds node)
{
    PvpTeamIndex teamIdx = m_nodes[node].owner;
    bool isTower = !m_nodes[node].graveyardId;

    if (!isTower && teamIdx != TEAM_INDEX_NEUTRAL)
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
void BattleGroundAV::HandlePlayerClickedOnFlag(Player* player, GameObject* go)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundAV: Player from team %u clicked on gameobject entry %u", player->GetTeam(), go->GetEntry());

    uint8 event = (GetBgMap()->GetMapDataContainer().GetGameObjectEventIndex(go->GetDbGuid())).event1;
    if (event >= BG_AV_MAX_NODES)                           // not a node
        return;
    AVNodeIds node = AVNodeIds(event);

    switch ((GetBgMap()->GetMapDataContainer().GetGameObjectEventIndex(go->GetDbGuid())).event2 % BG_AV_MAX_STATES)
    {
        case POINT_CONTROLLED:
            ProcessPlayerAssaultsPoint(player, node);
            break;
        case POINT_ASSAULTED:
            ProcessPlayerDefendsPoint(player, node);
            break;
        default:
            break;
    }
}

// Handle node defend
void BattleGroundAV::ProcessPlayerDefendsPoint(Player* player, AVNodeIds node)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(player->GetTeam());

    if (m_nodes[node].owner == teamIdx || m_nodes[node].state != POINT_ASSAULTED)
        return;

    // ToDo: improve this part; The middle graveyard should be handled directly as an assault
    if (m_nodes[node].totalOwner == TEAM_INDEX_NEUTRAL && node == BG_AV_NODES_SNOWFALL_GRAVE)     // initial snowfall capture
    {
        ProcessPlayerAssaultsPoint(player, node);
        return;
    }

    DEBUG_LOG("BattleGroundAV: Team %u defends node id %i", player->GetTeam(), node);

    if (m_nodes[node].prevOwner != teamIdx)
    {
        sLog.outError("BattleGroundAV: player defends point which doesn't belong to his team %i", node);
        return;
    }

    bool isTower = !m_nodes[node].graveyardId;

    uint32 newState  = teamIdx == TEAM_INDEX_ALLIANCE ? avNodeWorldStates[node].worldStateAlly : avNodeWorldStates[node].worldStateHorde;

    uint32 scoreType = isTower ? SCORE_TOWERS_DEFENDED : SCORE_GRAVEYARDS_DEFENDED;
    int32 yellId     = isTower ? LANG_BG_AV_TOWER_DEFENDED : LANG_BG_AV_GRAVE_DEFENDED;
    uint32 soundId   = isTower ? BG_AV_SOUND_BOTH_TOWER_DEFEND : (teamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_GOOD : BG_AV_SOUND_HORDE_GOOD;

    // process world state

    // send yell and sound
    DoSendYellToTeam(teamIdx, yellId, node);
    PlaySoundToAll(soundId);

    // update score
    UpdatePlayerScore(player, scoreType, 1);

    // process node events
    DefendNode(node, teamIdx);                              // set the right variables for nodeinfo
    PopulateNode(node);                                     // spawn node-creatures (defender for example)
    UpdateNodeWorldState(node, newState);                   // send new mapicon to the player
}

// Handle node assault
void BattleGroundAV::ProcessPlayerAssaultsPoint(Player* player, AVNodeIds node)
{
    DEBUG_LOG("BattleGroundAV: Team %u assaults node id %i", player->GetTeam(), node);

    PvpTeamIndex teamIdx  = GetTeamIndexByTeamId(player->GetTeam());

    if (m_nodes[node].owner == teamIdx || teamIdx == m_nodes[node].totalOwner)
        return;

    bool isTower = !m_nodes[node].graveyardId;

    uint32 newState     = teamIdx == TEAM_INDEX_ALLIANCE ? avNodeWorldStates[node].worldStateAllyGrey : avNodeWorldStates[node].worldStateHordeGrey;
    uint32 scoreType    = isTower ? SCORE_TOWERS_ASSAULTED : SCORE_GRAVEYARDS_ASSAULTED;
    uint32 kreditMarker = isTower ? BG_AV_NPC_PVP_TOWER_CREDIT : BG_AV_NPC_PVP_GRAVEYARD_CREDIT;
    int32 yellId        = isTower ? LANG_BG_AV_TOWER_ASSAULTED : LANG_BG_AV_GRAVE_ASSAULTED;

    // process new world state
    if (!isTower)
    {
        // before despawn of GY spirit healer - need to TP players - must be after AssaultNode; opposite team of current player
        if (Creature* spiritHealer = GetClosestCreatureWithEntry(player, teamIdx == TEAM_INDEX_ALLIANCE ? BG_NPC_SPIRIT_GUIDE_HORDE : BG_NPC_SPIRIT_GUIDE_ALLIANCE, 100.f))
            spiritHealer->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, spiritHealer, spiritHealer);

        // make graveyard invalid
        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(avNodeDefaults[node].graveyardId, BG_AV_ZONE_MAIN, TEAM_INVALID);
    }

    // send yell and sound
    DoSendYellToTeam(teamIdx, yellId, node);
    PlaySoundToAll((teamIdx == TEAM_INDEX_ALLIANCE) ? BG_AV_SOUND_ALLIANCE_ASSAULTS : BG_AV_SOUND_HORDE_ASSAULTS);

    // update score
    UpdatePlayerScore(player, scoreType, 1);

    // send reward
    player->RewardPlayerAndGroupAtEventCredit(kreditMarker, player);

    // process node events
    AssaultNode(node, teamIdx);                             // update nodeinfo variables
    UpdateNodeWorldState(node, newState);                   // send mapicon
    PopulateNode(node);
}

// Update node world state
void BattleGroundAV::UpdateNodeWorldState(AVNodeIds node, uint32 newState)
{
    if (m_nodes[node].prevOwner == TEAM_INDEX_NEUTRAL)      // currently only snowfall is supported as neutral node
        GetBgMap()->GetVariableManager().SetVariable(BG_AV_STATE_GY_SNOWFALL_N, WORLD_STATE_REMOVE);
    else
        GetBgMap()->GetVariableManager().SetVariable(m_nodes[node].worldState, WORLD_STATE_REMOVE);

    m_nodes[node].worldState = newState;
    GetBgMap()->GetVariableManager().SetVariable(m_nodes[node].worldState, WORLD_STATE_ADD);
}

// Get text message id for each node
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
    }

    return 0;
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

// Initialize each battleground node
void BattleGroundAV::InitializeNode(AVNodeIds node)
{
    m_nodes[node].totalOwner = avNodeDefaults[node].initialOwner;
    m_nodes[node].owner      = avNodeDefaults[node].initialOwner;
    m_nodes[node].prevOwner  = avNodeDefaults[node].initialOwner;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].prevState  = m_nodes[node].state;
    m_nodes[node].state      = POINT_CONTROLLED;
    m_nodes[node].timer      = 0;
    m_nodes[node].graveyardId = avNodeDefaults[node].graveyardId;
    m_nodes[node].worldState = avNodeDefaults[node].defaultWorldState;
    GetBgMap()->GetVariableManager().SetVariable(m_nodes[node].worldState, WORLD_STATE_ADD);
    m_activeEvents[node] = avNodeDefaults[node].initialOwner * BG_AV_MAX_STATES + m_nodes[node].state;

    if (avNodeDefaults[node].graveyardId)                                      // grave-creatures are special cause of a quest
    {
        m_activeEvents[node + BG_AV_MAX_NODES]  = avNodeDefaults[node].initialOwner * BG_AV_MAX_GRAVETYPES;

        // initialize graveyards
        Team team = TEAM_INVALID;
        if (avNodeDefaults[node].initialOwner != TEAM_INDEX_NEUTRAL)
            team = GetTeamIdByTeamIndex(avNodeDefaults[node].initialOwner);

        // initalize graveyard
        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(avNodeDefaults[node].graveyardId, BG_AV_ZONE_MAIN, team);
    }
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

    // vanilla
    for (uint32 i = 0; i < BG_AV_MAX_MINES; ++i)
        for (uint32 k = 0; k < BG_AV_TEAMS_COUNT; ++k)
            GetBgMap()->GetVariableManager().SetVariableData(avMineWorldStates[i][k], true, 0, 0);

    for (auto& data : avNodeWorldStates)
    {
        GetBgMap()->GetVariableManager().SetVariableData(data.worldStateAlly, true, 0, 0);
        GetBgMap()->GetVariableManager().SetVariableData(data.worldStateHorde, true, 0, 0);
        GetBgMap()->GetVariableManager().SetVariableData(data.worldStateAllyGrey, true, 0, 0);
        GetBgMap()->GetVariableManager().SetVariableData(data.worldStateHordeGrey, true, 0, 0);
    }

    // tbc
    GetBgMap()->GetVariableManager().SetVariableData(BG_AV_STATE_SCORE_A, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AV_STATE_SCORE_H, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AV_STATE_SCORE_SHOW_H, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AV_STATE_SCORE_SHOW_A, true, 0, 0);

    GetBgMap()->GetVariableManager().SetVariable(BG_AV_STATE_SCORE_A, BG_AV_SCORE_INITIAL_POINTS);
    GetBgMap()->GetVariableManager().SetVariable(BG_AV_STATE_SCORE_H, BG_AV_SCORE_INITIAL_POINTS);

    // initialize team variables
    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        for (uint8 j = 0; j < 9; ++j)                       // 9 quests getting tracked
            m_teamQuestStatus[i][j] = 0;

        m_wasInformedNearLose[i] = false;

        m_enemyTowersDestroyed[i] = 0;
        m_homeTowersControlled[i] = BG_AV_MAX_TOWERS_PER_TEAM;
        m_activeEvents[BG_AV_NODE_CAPTAIN_DEAD_A + i] = BG_EVENT_NONE;
    }

    // initialize mine variables and active events
    for (uint8 i = 0; i < BG_AV_MAX_MINES; ++i)
    {
        m_mineOwner[i] = TEAM_INDEX_NEUTRAL;
        GetBgMap()->GetVariableManager().SetVariable(avMineWorldStates[i][TEAM_INDEX_NEUTRAL], WORLD_STATE_ADD);
        m_mineTimer[i] = BG_AV_MINE_TICK_TIMER;

        m_activeEvents[BG_AV_MINE_BOSSES + i] = TEAM_INDEX_NEUTRAL;
        m_activeEvents[BG_AV_MINE_EVENT + i] = TEAM_INDEX_NEUTRAL;
    }

    m_activeEvents[BG_AV_CAPTAIN_A] = 0;
    m_activeEvents[BG_AV_CAPTAIN_H] = 0;
    m_activeEvents[BG_AV_HERALD] = 0;
    m_activeEvents[BG_AV_BOSS_A] = 0;
    m_activeEvents[BG_AV_BOSS_H] = 0;

    for (uint8 i = BG_AV_NODES_DUNBALDAR_SOUTH; i < BG_AV_MAX_NODES; ++i)  // towers
        m_activeEvents[BG_AV_MARSHAL_A_SOUTH + i - BG_AV_NODES_DUNBALDAR_SOUTH] = 0;

    // initialize all nodes
    for (uint8 i = 0; i < BG_AV_MAX_NODES; ++i)
        InitializeNode(AVNodeIds(i));

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(BG_AV_GRAVE_MAIN_ALLIANCE, BG_AV_ZONE_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(BG_AV_GRAVE_MAIN_HORDE, BG_AV_ZONE_MAIN, HORDE);

    m_mineYellTimer = BG_AV_MINE_YELL;
}

// Handle battleground yells
void BattleGroundAV::DoSendYellToTeam(PvpTeamIndex teamIdx, int32 textId, uint8 nodeId, bool useNode /*= true*/, int32 mineLangId /*= 0*/)
{
    // Get the specific herald for each team
    Creature* pHerald = GetSingleCreatureFromStorage(teamIdx == TEAM_INDEX_ALLIANCE ? BG_AV_NPC_STORMPIKE_HERALD : BG_AV_NPC_FROSTWOLF_HERALD);
    if (!pHerald)
        return;

    // different types of yells, depending on parameters
    if (useNode)
    {
        if (mineLangId)
            SendYell2ToAll(textId, LANG_UNIVERSAL, pHerald, (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE, mineLangId);
        else
            SendYell2ToAll(textId, LANG_UNIVERSAL, pHerald, GetNodeMessageId(AVNodeIds(nodeId)), (teamIdx == TEAM_INDEX_ALLIANCE) ? LANG_BG_ALLY : LANG_BG_HORDE);
    }
    else
        SendYellToAll(textId, LANG_UNIVERSAL, pHerald);
}

Team BattleGroundAV::GetPrematureWinner()
{
    int32 hordeScore = GetBgMap()->GetVariableManager().GetVariable(BG_AV_STATE_SCORE_H);
    int32 allianceScore = GetBgMap()->GetVariableManager().GetVariable(BG_AV_STATE_SCORE_A);

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
