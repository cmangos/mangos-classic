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
#include "BattleGroundAB.h"
#include "Entities/GameObject.h"
#include "BattleGroundMgr.h"
#include "Tools/Language.h"
#include "Server/WorldPacket.h"
#include "Globals/ObjectMgr.h"

BattleGroundAB::BattleGroundAB(): m_isInformedNearVictory(false), m_honorTicks(0), m_reputationTics(0)
{
    m_startMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    m_startMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_AB_START_ONE_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_AB_START_HALF_MINUTE;
    m_startMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_AB_HAS_BEGUN;
}

void BattleGroundAB::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // start counting the team points on every update tick;
    // every time we start from 0 for each team
    int team_points[PVP_TEAM_COUNT] = { 0, 0 };

    for (uint8 node = 0; node < BG_AB_MAX_NODES; ++node)
    {
        // 3 sec delay to spawn new banner instead previous despawned one
        if (m_bannerTimers[node].timer)
        {
            if (m_bannerTimers[node].timer <= diff)
            {
                DoUpdateBanner(node, m_bannerTimers[node].status, false);
                m_bannerTimers[node].timer = 0;
            }
            else
                m_bannerTimers[node].timer -= diff;
        }

        // 1-minute to occupy a node from contested state
        if (m_nodeTimers[node])
        {
            if (m_nodeTimers[node] <= diff)
            {
                m_nodeTimers[node] = 0;

                // Process the changes from contested to occupied
                PvpTeamIndex teamIndex = PvpTeamIndex(m_nodeStatus[node] - 1);
                Team team = GetTeamIdByTeamIndex(PvpTeamIndex(teamIndex));

                m_prevNodeStatus[node] = m_nodeStatus[node];
                m_nodeStatus[node] = ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_OCCUPIED);

                // create new occupied banner
                DoUpdateBanner(node, m_nodeStatus[node], true);
                ProcessNodeCapture(node, teamIndex);
                DoUpdateNodeWorldstate(node, abOccupiedStates[teamIndex][node]);

                // Message to chatlog
                ChatMsg chatSystem = teamIndex == TEAM_INDEX_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE;
                uint32 factionStrig = teamIndex == TEAM_INDEX_ALLIANCE ? LANG_BG_ALLY : LANG_BG_HORDE;

                SendMessage2ToAll(LANG_BG_AB_NODE_TAKEN, chatSystem, nullptr, factionStrig, GetNodeMessageId(node));
                PlaySoundToAll(teamIndex == TEAM_INDEX_ALLIANCE ? BG_AB_SOUND_NODE_CAPTURED_ALLIANCE : BG_AB_SOUND_NODE_CAPTURED_HORDE);
            }
            else
                m_nodeTimers[node] -= diff;
        }

        // for each occupied node we add team points; 
        // this influences the abTickPoints and the abTickIntervals values
        for (uint8 teamIndex = 0; teamIndex < PVP_TEAM_COUNT; ++teamIndex)
            if (m_nodeStatus[node] == ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_OCCUPIED))
                ++team_points[teamIndex];
    }

    // Accumulate points
    for (uint8 teamIndex = 0; teamIndex < PVP_TEAM_COUNT; ++teamIndex)
    {
        int points = team_points[teamIndex];
        if (!points)
            continue;

        m_lastTick[teamIndex] += diff;

        if (m_lastTick[teamIndex] > abTickIntervals[points])
        {
            m_lastTick[teamIndex]            -= abTickIntervals[points];

            int32 worldstateId = teamIndex == TEAM_INDEX_ALLIANCE ? BG_AB_OP_RESOURCES_ALLY : BG_AB_OP_RESOURCES_HORDE;
            int32 newValue = GetBgMap()->GetVariableManager().GetVariable(worldstateId) + abTickPoints[points];

            if (newValue > BG_AB_MAX_TEAM_SCORE)
                newValue = BG_AB_MAX_TEAM_SCORE;

            m_honorScoreTicks[teamIndex]     += abTickPoints[points];
            m_reputationScoreTics[teamIndex] += abTickPoints[points];

            // update reputation
            if (m_reputationScoreTics[teamIndex] >= m_reputationTics)
            {
                (teamIndex == TEAM_INDEX_ALLIANCE) ? RewardReputationToTeam(509, 10, ALLIANCE) : RewardReputationToTeam(510, 10, HORDE);
                m_reputationScoreTics[teamIndex] -= m_reputationTics;
            }

            // update honor
            if (m_honorScoreTicks[teamIndex] >= m_honorTicks)
            {
                RewardHonorToTeam(GetBonusHonorFromKill(1), (teamIndex == TEAM_INDEX_ALLIANCE) ? ALLIANCE : HORDE);
                m_honorScoreTicks[teamIndex] -= m_honorTicks;
            }

            // send info for near victory
            if (!m_isInformedNearVictory && newValue > BG_AB_WARNING_NEAR_VICTORY_SCORE)
            {
                SendMessageToAll(teamIndex == TEAM_INDEX_ALLIANCE ? LANG_BG_AB_A_NEAR_VICTORY : LANG_BG_AB_H_NEAR_VICTORY, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                PlaySoundToAll(teamIndex == TEAM_INDEX_ALLIANCE ? BG_AB_SOUND_NEAR_VICTORY_ALLIANCE : BG_AB_SOUND_NEAR_VICTORY_HORDE);
                m_isInformedNearVictory = true;
            }

            // update resource world state
            GetBgMap()->GetVariableManager().SetVariable(worldstateId, newValue);
        }
    }

    // Test win condition
    if (GetBgMap()->GetVariableManager().GetVariable(BG_AB_OP_RESOURCES_ALLY) >= BG_AB_MAX_TEAM_SCORE)
        EndBattleGround(ALLIANCE);
    if (GetBgMap()->GetVariableManager().GetVariable(BG_AB_OP_RESOURCES_HORDE) >= BG_AB_MAX_TEAM_SCORE)
        EndBattleGround(HORDE);
}

void BattleGroundAB::StartingEventOpenDoors()
{
    OpenDoorEvent(BG_EVENT_DOOR);

    // setup graveyards
    // enable gy near base after bg start
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_ALLIANCE, BG_AB_ZONE_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_HORDE, BG_AB_ZONE_MAIN, HORDE);
    // disable gy inside base after bg start
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_ALLIANCE_BASE, BG_AB_ZONE_MAIN, TEAM_INVALID);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_HORDE_BASE, BG_AB_ZONE_MAIN, TEAM_INVALID);
}

void BattleGroundAB::AddPlayer(Player* player)
{
    BattleGround::AddPlayer(player);
    // create score and add it to map, default values are set in the constructor
    BattleGroundABScore* sc = new BattleGroundABScore;

    m_playerScores[player->GetObjectGuid()] = sc;
}

bool BattleGroundAB::HandleAreaTrigger(Player* source, uint32 trigger)
{
    switch (trigger)
    {
        case AB_AT_ALLIANCE_EXIT:                                       // Arathi Basin Alliance Exit.
            if (source->GetTeam() != ALLIANCE)
                source->GetSession()->SendNotification(LANG_BATTLEGROUND_ONLY_ALLIANCE_USE);
            else
                source->LeaveBattleground();
            break;
        case AB_AT_HORDE_EXIT:                                          // Arathi Basin Horde Exit.
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

// Method that updates the node banner (despawn old one; spawn new one)
void BattleGroundAB::DoUpdateBanner(uint8 node, ABNodeStatus status, bool delay)
{
    // Just put it into the queue
    if (delay)
    {
        // ToDo: maybe use the delay which is stored in the GameObject definition
        m_bannerTimers[node].timer = 3000;
        m_bannerTimers[node].status = status;
    }
    else
        SpawnEvent(node, status, true);                           // will automaticly despawn other events
}

// Function that returns the message id for each node
int32 BattleGroundAB::GetNodeMessageId(uint8 node) const
{
    switch (node)
    {
        case BG_AB_NODE_STABLES:     return LANG_BG_AB_NODE_STABLES;
        case BG_AB_NODE_BLACKSMITH:  return LANG_BG_AB_NODE_BLACKSMITH;
        case BG_AB_NODE_FARM:        return LANG_BG_AB_NODE_FARM;
        case BG_AB_NODE_LUMBER_MILL: return LANG_BG_AB_NODE_LUMBER_MILL;
        case BG_AB_NODE_GOLD_MINE:   return LANG_BG_AB_NODE_GOLD_MINE;
    }
    return 0;
}

// Method that sends the world state update for the given node
void BattleGroundAB::DoUpdateNodeWorldstate(uint8 node, uint32 newState)
{
    // Send node owner state update to refresh map icons on client
    GetBgMap()->GetVariableManager().SetVariable(m_nodeVisualState[node], WORLD_STATE_REMOVE);
    m_nodeVisualState[node] = newState;
    GetBgMap()->GetVariableManager().SetVariable(m_nodeVisualState[node], WORLD_STATE_ADD);
}

// Method that handles the node capture by a team
void BattleGroundAB::ProcessNodeCapture(uint8 node, PvpTeamIndex teamIdx)
{
    Team team = GetTeamIdByTeamIndex(teamIdx);
    int32 worldstateId = teamIdx == TEAM_INDEX_ALLIANCE ? BG_AB_OP_OCCUPIED_BASES_ALLY : BG_AB_OP_OCCUPIED_BASES_HORDE;
    int32 newValue = GetBgMap()->GetVariableManager().GetVariable(worldstateId) + 1;
    GetBgMap()->GetVariableManager().SetVariable(worldstateId, newValue);

    DEBUG_LOG("BattleGroundAB: Node with id %u was captured by team %u.", node, team);

    if (newValue >= 5)
        CastSpellOnTeam(BG_AB_SPELL_QUEST_REWARD_5_BASES, team);
    else if (newValue >= 4)
        CastSpellOnTeam(BG_AB_SPELL_QUEST_REWARD_4_BASES, team);

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(abGraveyardIds[node], BG_AB_ZONE_MAIN, team);
}

// Method that handles the banner click
void BattleGroundAB::HandlePlayerClickedOnFlag(Player* player, GameObject* go)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    DEBUG_LOG("BattleGroundAB: Handle flag clicked by team %u for gameobject entry %u.", player->GetTeam(), go->GetEntry());

    PvpTeamIndex teamIndex = GetTeamIndexByTeamId(player->GetTeam());
    uint32 scoreType    = 0;
    uint32 soundId      = 0;
    uint32 worldState   = 0;
    int32 textEntry     = 0;
    uint32 factionStrig = 0;

    // process battleground event
    uint8 event = (GetBgMap()->GetMapDataContainer().GetGameObjectEventIndex(go->GetDbGuid())).event1;
    if (event >= BG_AB_MAX_NODES)                           // not a node
        return;

    ABNodes node = ABNodes(event);

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);

    // Node is NEUTRAL -> change to CONTESTED
    if (m_nodeStatus[node] == ABNodeStatus(BG_AB_NODE_TYPE_NEUTRAL))
    {
        scoreType              = SCORE_BASES_ASSAULTED;
        worldState             = abContestedStates[teamIndex][node];

        m_prevNodeStatus[node] = m_nodeStatus[node];
        m_nodeStatus[node]     = ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_CONTESTED);

        m_nodeTimers[node]     = BG_AB_FLAG_CAPTURING_TIME;

        textEntry              = LANG_BG_AB_NODE_CLAIMED;
        factionStrig           = teamIndex == TEAM_INDEX_ALLIANCE ? LANG_BG_ALLY : LANG_BG_HORDE;
        soundId                = BG_AB_SOUND_NODE_CLAIMED;
    }
    // Node is CONTESTED -> change depends on previous status
    else if (m_nodeStatus[node] == ABNodeStatus(BG_AB_NODE_STATUS_ALLY_CONTESTED) || m_nodeStatus[node] == ABNodeStatus(BG_AB_NODE_STATUS_HORDE_CONTESTED))
    {
        // Last state is CONTESTED -> change to CONTESTED (change owner team)
        if (m_prevNodeStatus[node] < ABNodeStatus(BG_AB_NODE_TYPE_OCCUPIED))
        {
            scoreType               = SCORE_BASES_ASSAULTED;
            worldState              = abContestedStates[teamIndex][node];

            m_prevNodeStatus[node]  = m_nodeStatus[node];
            m_nodeStatus[node]      = ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_CONTESTED);

            m_nodeTimers[node]      = BG_AB_FLAG_CAPTURING_TIME;

            textEntry               = LANG_BG_AB_NODE_ASSAULTED;
        }
        // Last state is OCCUPIED -> change back to OCCUPIED (defend the node)
        else
        {
            scoreType               = SCORE_BASES_DEFENDED;
            worldState              = abOccupiedStates[teamIndex][node];

            m_prevNodeStatus[node]  = m_nodeStatus[node];
            m_nodeStatus[node]      = ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_OCCUPIED);

            m_nodeTimers[node]      = 0;

            textEntry               = LANG_BG_AB_NODE_DEFENDED;

            // apply the node capture benefits
            ProcessNodeCapture(node, teamIndex);
        }

        soundId = (teamIndex == TEAM_INDEX_ALLIANCE) ? BG_AB_SOUND_NODE_ASSAULTED_ALLIANCE : BG_AB_SOUND_NODE_ASSAULTED_HORDE;
    }
    // Node is OCCUPIED -> change to CONTESTED
    else
    {
        scoreType               = SCORE_BASES_ASSAULTED;
        worldState              = abContestedStates[teamIndex][node];

        m_prevNodeStatus[node]  = m_nodeStatus[node];
        m_nodeStatus[node]      = ABNodeStatus(teamIndex + BG_AB_NODE_TYPE_CONTESTED);

        m_nodeTimers[node]      = BG_AB_FLAG_CAPTURING_TIME;

        textEntry               = LANG_BG_AB_NODE_ASSAULTED;
        soundId                 = (teamIndex == TEAM_INDEX_ALLIANCE) ? BG_AB_SOUND_NODE_ASSAULTED_ALLIANCE : BG_AB_SOUND_NODE_ASSAULTED_HORDE;

        PvpTeamIndex otherTeamIndex = GetOtherTeamIndex(teamIndex);
        int32 worldstateId = otherTeamIndex == TEAM_INDEX_ALLIANCE ? BG_AB_OP_OCCUPIED_BASES_ALLY : BG_AB_OP_OCCUPIED_BASES_HORDE;
        int32 newValue = GetBgMap()->GetVariableManager().GetVariable(worldstateId) - 1;
        GetBgMap()->GetVariableManager().SetVariable(worldstateId, newValue);

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(abGraveyardIds[node], BG_AB_ZONE_MAIN, TEAM_INVALID);
    }

    // update score
    UpdatePlayerScore(player, scoreType, 1);

    // update banner object
    DoUpdateBanner(node, m_nodeStatus[node], true);

    // update world state
    DoUpdateNodeWorldstate(node, worldState);

    // send message; this depends on the node status
    ChatMsg chatSystem = teamIndex == TEAM_INDEX_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE;

    if (factionStrig)
        SendMessage2ToAll(textEntry, chatSystem, player, GetNodeMessageId(node), factionStrig);
    else
        SendMessage2ToAll(textEntry, chatSystem, player, GetNodeMessageId(node));

    // play sound to all players
    PlaySoundToAll(soundId);
}

void BattleGroundAB::Reset()
{
    // call parent's class reset
    BattleGround::Reset();

    for (auto Id : abDefaultStates)
        GetBgMap()->GetVariableManager().SetVariableData(Id, true, 0, 0);

    for (auto data : abOccupiedStates)
        for (uint32 i = 0; i < BG_AB_MAX_NODES; ++i)
            GetBgMap()->GetVariableManager().SetVariableData(data[i], true, 0, 0);

    for (auto data : abContestedStates)
        for (uint32 i = 0; i < BG_AB_MAX_NODES; ++i)
            GetBgMap()->GetVariableManager().SetVariableData(data[i], true, 0, 0);

    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_OCCUPIED_BASES_ALLY, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_OCCUPIED_BASES_HORDE, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_RESOURCES_MAX, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_RESOURCES_MAX, BG_AB_MAX_TEAM_SCORE);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_RESOURCES_WARNING, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_RESOURCES_WARNING, BG_AB_WARNING_NEAR_VICTORY_SCORE);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_RESOURCES_ALLY, true, 0, 0);
    GetBgMap()->GetVariableManager().SetVariableData(BG_AB_OP_RESOURCES_HORDE, true, 0, 0);
    // legacy FillInitialWorldState(data, count, 0x745, 0x2);         // 37 1861 unk - not using until we found out what its for

    for (uint8 i = 0; i < PVP_TEAM_COUNT; ++i)
    {
        m_lastTick[i] = 0;
        m_honorScoreTicks[i] = 0;
        m_reputationScoreTics[i] = 0;
    }

    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_RESOURCES_ALLY, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_RESOURCES_HORDE, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_OCCUPIED_BASES_ALLY, 0);
    GetBgMap()->GetVariableManager().SetVariable(BG_AB_OP_OCCUPIED_BASES_HORDE, 0);

    m_isInformedNearVictory = false;
    bool isBgWeekend = BattleGroundMgr::IsBgWeekend(GetTypeId());
    m_honorTicks     = isBgWeekend ? BG_AB_WEEKEND_HONOR_INTERVAL : BG_AB_NORMAL_HONOR_INTERVAL;
    m_reputationTics = isBgWeekend ? BG_AB_WEEKEND_REPUTATION_INTERVAL : BG_AB_NORMAL_REPUTATION_INTERVAL;

    for (uint8 i = 0; i < BG_AB_MAX_NODES; ++i)
    {
        m_nodeVisualState[i] = abDefaultStates[i];
        GetBgMap()->GetVariableManager().SetVariable(m_nodeVisualState[i], WORLD_STATE_ADD);

        m_nodeStatus[i]      = BG_AB_NODE_STATUS_NEUTRAL;
        m_prevNodeStatus[i]  = BG_AB_NODE_STATUS_NEUTRAL;

        m_nodeTimers[i]     = 0;
        m_bannerTimers[i].timer = 0;

        // all nodes owned by neutral team at beginning
        m_activeEvents[i] = BG_AB_NODE_TYPE_NEUTRAL;

        GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(abGraveyardIds[i], BG_AB_ZONE_MAIN, TEAM_INVALID);
    }

    // setup graveyards
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_ALLIANCE, BG_AB_ZONE_MAIN, ALLIANCE);
    GetBgMap()->GetGraveyardManager().SetGraveYardLinkTeam(AB_GRAVEYARD_HORDE, BG_AB_ZONE_MAIN, HORDE);
}

void BattleGroundAB::EndBattleGround(Team winner)
{
    // win reward
    RewardHonorToTeam(GetBonusHonorFromKill(1), winner);

    // complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);
    RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);

    BattleGround::EndBattleGround(winner);
}

void BattleGroundAB::UpdatePlayerScore(Player* source, uint32 type, uint32 value)
{
    BattleGroundScoreMap::iterator itr = m_playerScores.find(source->GetObjectGuid());
    if (itr == m_playerScores.end())                        // player not found...
        return;

    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            ((BattleGroundABScore*)itr->second)->basesAssaulted += value;
            break;
        case SCORE_BASES_DEFENDED:
            ((BattleGroundABScore*)itr->second)->basesDefended += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(source, type, value);
            break;
    }
}

Team BattleGroundAB::GetPrematureWinner()
{
    int32 hordeScore = GetBgMap()->GetVariableManager().GetVariable(BG_AB_OP_RESOURCES_HORDE);
    int32 allianceScore = GetBgMap()->GetVariableManager().GetVariable(BG_AB_OP_RESOURCES_ALLY);

    if (hordeScore > allianceScore)
        return HORDE;
    if (allianceScore > hordeScore)
        return ALLIANCE;

    // If the values are equal, fall back to number of players on each team
    return BattleGround::GetPrematureWinner();
}
