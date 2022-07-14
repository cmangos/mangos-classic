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

#ifndef __BATTLEGROUNDAB_H
#define __BATTLEGROUNDAB_H

#include "Common.h"
#include "BattleGround.h"

enum ABWorldStates
{
    BG_AB_OP_OCCUPIED_BASES_HORDE       = 1778,
    BG_AB_OP_OCCUPIED_BASES_ALLY        = 1779,
    BG_AB_OP_RESOURCES_ALLY             = 1776,
    BG_AB_OP_RESOURCES_HORDE            = 1777,
    BG_AB_OP_RESOURCES_MAX              = 1780,
    BG_AB_OP_RESOURCES_WARNING          = 1955,

    BG_AB_OP_STABLE_ICON                = 1842,     // Stable map icon (NONE)
    BG_AB_OP_GOLDMINE_ICON              = 1843,     // Gold Mine map icon (NONE)
    BG_AB_OP_LUMBERMILL_ICON            = 1844,     // Lumber Mill map icon (NONE)
    BG_AB_OP_FARM_ICON                  = 1845,     // Farm map icon (NONE)
    BG_AB_OP_BLACKSMITH_ICON            = 1846,     // Blacksmith map icon (NONE)

    BG_AB_OP_STABLE_STATE_A             = 1767,     // Stable map state (ALLIANCE)
    BG_AB_OP_STABLE_STATE_H             = 1768,     // Stable map state (HORDE)
    BG_AB_OP_STABLE_STATE_CON_A         = 1769,     // Stable map state (CON ALLIANCE)
    BG_AB_OP_STABLE_STATE_CON_H         = 1770,     // Stable map state (CON HORDE)

    BG_AB_OP_FARM_STATE_A               = 1772,     // Farm state (ALLIANCE)
    BG_AB_OP_FARM_STATE_H               = 1773,     // Farm state (HORDE)
    BG_AB_OP_FARM_STATE_CON_A           = 1774,     // Farm state (CON ALLIANCE)
    BG_AB_OP_FARM_STATE_CON_H           = 1775,     // Farm state (CON HORDE)

    BG_AB_OP_BLACKSMITH_STATE_A         = 1782,     // Blacksmith map state (ALLIANCE)
    BG_AB_OP_BLACKSMITH_STATE_H         = 1783,     // Blacksmith map state (HORDE)
    BG_AB_OP_BLACKSMITH_STATE_CON_A     = 1784,     // Blacksmith map state (CON ALLIANCE)
    BG_AB_OP_BLACKSMITH_STATE_CON_H     = 1785,     // Blacksmith map state (CON HORDE)

    BG_AB_OP_LUMBERMILL_STATE_A         = 1792,     // Lumber Mill map state (ALLIANCE)
    BG_AB_OP_LUMBERMILL_STATE_H         = 1793,     // Lumber Mill map state (HORDE)
    BG_AB_OP_LUMBERMILL_STATE_CON_A     = 1794,     // Lumber Mill map state (CON ALLIANCE)
    BG_AB_OP_LUMBERMILL_STATE_CON_H     = 1795,     // Lumber Mill map state (CON HORDE)

    BG_AB_OP_GOLDMINE_STATE_A           = 1787,     // Gold Mine map state (ALLIANCE)
    BG_AB_OP_GOLDMINE_STATE_H           = 1788,     // Gold Mine map state (HORDE)
    BG_AB_OP_GOLDMINE_STATE_CON_A       = 1789,     // Gold Mine map state (CON ALLIANCE
    BG_AB_OP_GOLDMINE_STATE_CON_H       = 1790,     // Gold Mine map state (CON HORDE)
};

enum ABTimers
{
    BG_AB_FLAG_CAPTURING_TIME           = 60000,

    BG_AB_NORMAL_HONOR_INTERVAL         = 260,
    BG_AB_WEEKEND_HONOR_INTERVAL        = 160,
    BG_AB_NORMAL_REPUTATION_INTERVAL    = 160,
    BG_AB_WEEKEND_REPUTATION_INTERVAL   = 120,
};

enum ABScores
{
    BG_AB_WARNING_NEAR_VICTORY_SCORE    = 1800,
    BG_AB_MAX_TEAM_SCORE                = 2000
};

// Battleground Node Events
// node-events are just event1=ABNodes, event2=ABNodeStatus
// so we don't need to define the constants here :)
// do NOT change the order, else wrong behaviour
enum ABNodes
{
    BG_AB_NODE_STABLES                  = 0,                // node values used in event1 for SpawnEvent
    BG_AB_NODE_BLACKSMITH               = 1,
    BG_AB_NODE_FARM                     = 2,
    BG_AB_NODE_LUMBER_MILL              = 3,
    BG_AB_NODE_GOLD_MINE                = 4,

    BG_AB_MAX_NODES                     = 5,
};

enum ABNodeType
{
    BG_AB_NODE_TYPE_NEUTRAL             = 0,
    BG_AB_NODE_TYPE_CONTESTED           = 1,                // logic for contested and occupied is: teamIndex + current value
    BG_AB_NODE_TYPE_OCCUPIED            = 3,                // this gives us the event2 value for SpawnEvent
};

enum ABNodeStatus
{
    BG_AB_NODE_STATUS_NEUTRAL           = 0,                // values to be used for event2 in SpawnEventfunction
    BG_AB_NODE_STATUS_ALLY_CONTESTED    = 1,
    BG_AB_NODE_STATUS_HORDE_CONTESTED   = 2,
    BG_AB_NODE_STATUS_ALLY_OCCUPIED     = 3,
    BG_AB_NODE_STATUS_HORDE_OCCUPIED    = 4
};

enum ABBanners
{
    // neutral banners; all are GO type 10
    BG_AB_BANNER_MINE                   = 180091,           // triggers spell 23937 
    BG_AB_BANNER_LUMBER_MILL            = 180090,           // triggers spell 23938
    BG_AB_BANNER_FARM                   = 180089,           // triggers spell 23935
    BG_AB_BANNER_BLACKSMITH             = 180088,           // triggers spell 23936
    BG_AB_BANNER_STABLE                 = 180087,           // triggers spell 23932

    // team banners; all are GO type 1; can be clicked by the opposite faction only
    BG_AB_BANNER_ALLIANCE               = 180058,
    BG_AB_BANNER_CONTESTED_A            = 180059,
    BG_AB_BANNER_HORDE                  = 180060,
    BG_AB_BANNER_CONTESTED_H            = 180061,
};

enum ABSpells
{
    BG_AB_SPELL_QUEST_REWARD_4_BASES    = 24061,                // quests 8114 and 8121
    BG_AB_SPELL_QUEST_REWARD_5_BASES    = 24064,                // quests 8115 and 8122
};

enum ABSounds
{
    BG_AB_SOUND_NODE_CLAIMED            = 8192,
    BG_AB_SOUND_NODE_CAPTURED_ALLIANCE  = 8173,
    BG_AB_SOUND_NODE_CAPTURED_HORDE     = 8213,
    BG_AB_SOUND_NODE_ASSAULTED_ALLIANCE = 8212,
    BG_AB_SOUND_NODE_ASSAULTED_HORDE    = 8174,
    BG_AB_SOUND_NEAR_VICTORY_HORDE      = 8456,
    BG_AB_SOUND_NEAR_VICTORY_ALLIANCE   = 8457
};

enum ABGraveyards
{
    AB_GRAVEYARD_STABLES                = 895,
    AB_GRAVEYARD_BLACKSMITH             = 894,
    AB_GRAVEYARD_FARM                   = 893,
    AB_GRAVEYARD_LUMBER_MILL            = 897,
    AB_GRAVEYARD_GOLD_MINE              = 896,

    AB_GRAVEYARD_ALLIANCE               = 898,
    AB_GRAVEYARD_HORDE                  = 899,
    AB_GRAVEYARD_ALLIANCE_BASE = 890,
    AB_GRAVEYARD_HORDE_BASE = 889,

    BG_AB_ZONE_MAIN                     = 3358,
};

enum ABAreaTriggers
{
    AB_AT_ALLIANCE_EXIT                 = 3948,
    AB_AT_HORDE_EXIT                    = 3949,
};

// default neutral world states
const uint32 abDefaultStates[BG_AB_MAX_NODES] = { BG_AB_OP_STABLE_ICON, BG_AB_OP_BLACKSMITH_ICON, BG_AB_OP_FARM_ICON, BG_AB_OP_LUMBERMILL_ICON, BG_AB_OP_GOLDMINE_ICON };

// occupied world states
const uint32 abOccupiedStates[PVP_TEAM_COUNT][BG_AB_MAX_NODES] =
{
    {BG_AB_OP_STABLE_STATE_A, BG_AB_OP_BLACKSMITH_STATE_A, BG_AB_OP_FARM_STATE_A, BG_AB_OP_LUMBERMILL_STATE_A, BG_AB_OP_GOLDMINE_STATE_A },
    {BG_AB_OP_STABLE_STATE_H, BG_AB_OP_BLACKSMITH_STATE_H, BG_AB_OP_FARM_STATE_H, BG_AB_OP_LUMBERMILL_STATE_H, BG_AB_OP_GOLDMINE_STATE_H }
};

// contested world states
const uint32 abContestedStates[PVP_TEAM_COUNT][BG_AB_MAX_NODES] =
{
    {BG_AB_OP_STABLE_STATE_CON_A, BG_AB_OP_BLACKSMITH_STATE_CON_A, BG_AB_OP_FARM_STATE_CON_A, BG_AB_OP_LUMBERMILL_STATE_CON_A, BG_AB_OP_GOLDMINE_STATE_CON_A },
    {BG_AB_OP_STABLE_STATE_CON_H, BG_AB_OP_BLACKSMITH_STATE_CON_H, BG_AB_OP_FARM_STATE_CON_H, BG_AB_OP_LUMBERMILL_STATE_CON_H, BG_AB_OP_GOLDMINE_STATE_CON_H }
};

// Tick intervals and given points: case 0,1,2,3,4,5 captured nodes
const uint32 abTickIntervals[6] = {0, 12000, 9000, 6000, 3000, 1000};
const uint32 abTickPoints[6] = {0, 10, 10, 10, 10, 30};

// WorldSafeLocs ids
const uint32 abGraveyardIds[BG_AB_MAX_NODES] = { AB_GRAVEYARD_STABLES, AB_GRAVEYARD_BLACKSMITH, AB_GRAVEYARD_FARM, AB_GRAVEYARD_LUMBER_MILL, AB_GRAVEYARD_GOLD_MINE };

struct ArathiBannerTimer
{
    uint32          timer;
    ABNodeStatus    status;
};

class BattleGroundABScore : public BattleGroundScore
{
    public:
        BattleGroundABScore(): basesAssaulted(0), basesDefended(0) {};
        virtual ~BattleGroundABScore() {};

        uint32 GetAttr1() const override { return basesAssaulted; }
        uint32 GetAttr2() const override { return basesDefended; }

        uint32 basesAssaulted;
        uint32 basesDefended;
};

class BattleGroundAB : public BattleGround
{
        friend class BattleGroundMgr;

    public:
        BattleGroundAB();
        void Reset() override;
        void Update(uint32 diff) override;

        // Main battleground functions
        void AddPlayer(Player* player) override;
        void StartingEventOpenDoors() override;
        void EndBattleGround(Team winner) override;

        // General functions
        void UpdatePlayerScore(Player* source, uint32 type, uint32 value) override;
        Team GetPrematureWinner() override;

        // Battleground event handlers
        bool HandleAreaTrigger(Player* source, uint32 trigger) override;
        void HandlePlayerClickedOnFlag(Player* source, GameObject* target_obj) override;

    private:
        // Banner update functions
        void DoUpdateBanner(uint8 node, ABNodeStatus nodeStatus, bool delay);
        void DoUpdateNodeWorldstate(uint8 node, uint32 newState);

        // Process node capture
        void ProcessNodeCapture(uint8 node, PvpTeamIndex teamIdx);

        // Get text id for node
        int32 GetNodeMessageId(uint8 node) const;

        ABNodeStatus m_nodeStatus[BG_AB_MAX_NODES];         // store the current node status
        ABNodeStatus m_prevNodeStatus[BG_AB_MAX_NODES];     // store the previous node status
        ArathiBannerTimer m_bannerTimers[BG_AB_MAX_NODES];

        uint32 m_nodeVisualState[BG_AB_MAX_NODES];
        uint32 m_nodeTimers[BG_AB_MAX_NODES];
        uint32 m_lastTick[PVP_TEAM_COUNT];                  // timer that handles the points update
        uint32 m_honorScoreTicks[PVP_TEAM_COUNT];
        uint32 m_reputationScoreTics[PVP_TEAM_COUNT];

        bool   m_isInformedNearVictory;
        uint32 m_honorTicks;
        uint32 m_reputationTics;
};
#endif
