#include "playerbot/playerbot.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundAV.h"
#include "BattleGroundTactics.h"

static std::tuple<uint32, uint32, std::string> AV_HordeAttackObjectives[] =
{
    // Attack
#ifndef MANGOSBOT_TWO  
    { BG_AV_NODES_STONEHEART_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STONEHEART_BUNKER"},
    { BG_AV_NODES_STONEHEART_GRAVE, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STONEHEART_GRAVEYARD" },
    { BG_AV_NODES_STONEHEART_GRAVE, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STONEHEART_GRAVEYARD" },
    { BG_AV_NODES_ICEWING_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_ICEWING_BUNKER" },
    { BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODES_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_DUNBALDAR_SOUTH" },
    { BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_DUNBALDAR_NORTH" },
    { BG_AV_NODES_FIRSTAID_STATION, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STORMPIKE_AID_STATION" },
    { BG_AV_NODES_FIRSTAID_STATION, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STORMPIKE_AID_STATION" },
#else
    { BG_AV_NODE_STONEHEART_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STONEHEART_BUNKER" },
    { BG_AV_NODE_GY_STONEHEARTH, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STONEHEART_GRAVEYARD" },
    { BG_AV_NODE_GY_STONEHEARTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STONEHEART_GRAVEYARD" },
    { BG_AV_NODE_ICEWING_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_ICEWING_BUNKER" },
    { BG_AV_NODE_GY_STORMPIKE, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODE_GY_STORMPIKE, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODE_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_DUNBALDAR_SOUTH" },
    { BG_AV_NODE_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_DUNBALDAR_NORTH" },
    { BG_AV_NODE_GY_DUN_BALDAR, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_STORMPIKE_AID_STATION" },
    { BG_AV_NODE_GY_DUN_BALDAR, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_STORMPIKE_AID_STATION" },
#endif
};

static std::tuple<uint32, uint32, std::string> AV_HordeDefendObjectives[] =
{
    // Defend
#ifndef MANGOSBOT_TWO
    { BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODES_FROSTWOLF_ETOWER, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_EAST_FROSTWOLF_TOWER" },
    { BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_WEST_FROSTWOLF_TOWER" },
    { BG_AV_NODES_TOWER_POINT, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_TOWERPOINT" },
    { BG_AV_NODES_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_ICEBLOOD_TOWER" },
#else
    { BG_AV_NODE_GY_FROSTWOLF, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODE_GY_FROSTWOLF, BG_AV_NODE_STATUS_ALLY_OCCUPIED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODE_FROSTWOLF_EAST, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_EAST_FROSTWOLF_TOWER" },
    { BG_AV_NODE_FROSTWOLF_WEST, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_WEST_FROSTWOLF_TOWER" },
    { BG_AV_NODE_TOWER_POINT, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_TOWERPOINT" },
    { BG_AV_NODE_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_ALLY_CONTESTED, "AV_ICEBLOOD_TOWER" },
#endif
};

static std::tuple<uint32, uint32, std::string> AV_AllianceAttackObjectives[] =
{
    // Attack
#ifndef MANGOSBOT_TWO  
    { BG_AV_NODES_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_ICEBLOOD_TOWER" },
    { BG_AV_NODES_ICEBLOOD_GRAVE, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_ICEBLOOD_GRAVEYARD" },
    { BG_AV_NODES_ICEBLOOD_GRAVE, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_ICEBLOOD_GRAVEYARD" },
    { BG_AV_NODES_TOWER_POINT, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_TOWERPOINT" },
    { BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODES_FROSTWOLF_ETOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_EAST_FROSTWOLF_TOWER" },
    { BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_WEST_FROSTWOLF_TOWER" },
    { BG_AV_NODES_FROSTWOLF_HUT, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_FROSTWOLF_RELIEF_HUT" },
    { BG_AV_NODES_FROSTWOLF_HUT, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_FROSTWOLF_RELIEF_HUT" },
#else
    { BG_AV_NODE_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_ICEBLOOD_TOWER" },
    { BG_AV_NODE_GY_ICEBLOOD, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_ICEBLOOD_GRAVEYARD" },
    { BG_AV_NODE_GY_ICEBLOOD, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_ICEBLOOD_GRAVEYARD" },
    { BG_AV_NODE_TOWER_POINT, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_TOWERPOINT" },
    { BG_AV_NODE_GY_FROSTWOLF, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODE_GY_FROSTWOLF, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_FROSTWOLF_GRAVEYARD" },
    { BG_AV_NODE_FROSTWOLF_EAST, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_EAST_FROSTWOLF_TOWER" },
    { BG_AV_NODE_FROSTWOLF_WEST, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_WEST_FROSTWOLF_TOWER" },
    { BG_AV_NODE_GY_FROSTWOLF_KEEP, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_FROSTWOLF_RELIEF_HUT" },
    { BG_AV_NODE_GY_FROSTWOLF_KEEP, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_FROSTWOLF_RELIEF_HUT" },
#endif
};

static std::tuple<uint32, uint32, std::string> AV_AllianceDefendObjectives[] =
{
    // Defend
#ifndef MANGOSBOT_TWO
    { BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODES_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_DUNBALDAR_SOUTH" },
    { BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_DUNBALDAR_NORTH" },
    { BG_AV_NODES_ICEWING_BUNKER, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_ICEWING_BUNKER" },
    { BG_AV_NODES_STONEHEART_BUNKER, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_STONEHEART_BUNKER" },
#else
    { BG_AV_NODE_GY_STORMPIKE, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODE_GY_STORMPIKE, BG_AV_NODE_STATUS_HORDE_OCCUPIED, "AV_STORMPIKE_GRAVEYARD" },
    { BG_AV_NODE_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_DUNBALDAR_SOUTH" },
    { BG_AV_NODE_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_DUNBALDAR_NORTH" },
    { BG_AV_NODE_ICEWING_BUNKER, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_ICEWING_BUNKER" },
    { BG_AV_NODE_STONEHEART_BUNKER, BG_AV_NODE_STATUS_HORDE_CONTESTED, "AV_STONEHEART_BUNKER" },
#endif
};

bool BGTactics::SelectAvObjectiveAlliance(WorldLocation& objectiveLocation)
{
    BattleGround* bg = bot->GetBattleGround();
    if (!bg)
    {
        return false;
    }

    // End boss
#ifndef MANGOSBOT_TWO  
    if (!bg->IsActiveEvent(BG_AV_NODES_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_TOWER_POINT, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_FROSTWOLF_ETOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_FROSTWOLF_HUT, BG_AV_NODE_STATUS_HORDE_OCCUPIED))
#else
    if (!bg->IsActiveEvent(BG_AV_NODE_ICEBLOOD_TOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_TOWER_POINT, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_FROSTWOLF_EAST, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_FROSTWOLF_WEST, BG_AV_NODE_STATUS_HORDE_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_GY_FROSTWOLF_KEEP, BG_AV_NODE_STATUS_HORDE_OCCUPIED))
#endif
    {
        if (Creature* pDrek = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_BOSS_H, 0)))
        {
            objectiveLocation = WorldLocation(pDrek->GetMapId(), pDrek->GetPosition());
            return true;
        }
    }

    bool strifeTime = bg->GetStartTime() < (uint32)(10 * MINUTE * IN_MILLISECONDS);
    uint32 role = context->GetValue<uint32>("bg role")->Get();
    bool supporter = role < (uint32)(strifeTime ? 4 : 2);

    // Only go to Snowfall Graveyard if already close to it.
#ifndef MANGOSBOT_TWO  
    if (supporter && (bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_HORDE_CONTESTED) || bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_HORDE_OCCUPIED) || bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED)))
    {
#else
    if (supporter && (bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_HORDE_CONTESTED) || bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_HORDE_OCCUPIED) || bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED)))
    {
#endif
        if (WorldLocation snowfallGraveyard; sRandomPlayerbotMgr.GetNamedLocation("AV_SNOWFALL_GRAVEYARD", snowfallGraveyard))
        {
            if (WorldPosition(bot).IsWithinDist(WorldPosition(snowfallGraveyard), VISIBILITY_DISTANCE_LARGE))
            {
                objectiveLocation = snowfallGraveyard;
                return true;
            }
        }
    }

    // Galv
    if (!supporter && !bg->IsActiveEvent(BG_AV_NODE_CAPTAIN_DEAD_H, 0))
    {
        if (Creature* pGalvangar = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_CAPTAIN_H, 0)))
        {
            if (pGalvangar->GetHealth() > 0)
            {
                if (WorldLocation icebloodGarrison; sRandomPlayerbotMgr.GetNamedLocation("AV_ICEBLOOD_GARRISON_WAITING_ALLIANCE", icebloodGarrison))
                {
                    uint32 attackCount = getDefendersCount(Position(icebloodGarrison.coord_x, icebloodGarrison.coord_y, icebloodGarrison.coord_z, icebloodGarrison.orientation), 10.0f, false);

                    // Prepare to attack Captain
                    if (attackCount < 5 && !sServerFacade.IsInCombat(pGalvangar))
                    {
                        objectiveLocation = icebloodGarrison;
                    }
                    else
                    {
                        objectiveLocation = WorldLocation(pGalvangar->GetMapId(), pGalvangar->GetPosition());
                    }

                    return true;
                }
            }
        }
    }

    // Chance to defend
    if (supporter)
    {
        std::vector<WorldLocation> objectiveLocations;

        for (auto const& [nodeId, nodeStatus, locationName] : AV_AllianceDefendObjectives)
        {
            if (!bg->IsActiveEvent(nodeId, nodeStatus))
            {
                continue;
            }

            if (WorldLocation location; sRandomPlayerbotMgr.GetNamedLocation(locationName, location))
            {
                objectiveLocations.push_back(location);
            }
        }

        if (!objectiveLocations.empty())
        {
            objectiveLocation = objectiveLocations[urand(0, objectiveLocations.size() - 1)];
            return true;
        }
    }

    // Mine capture
    if (!supporter && (bg->IsActiveEvent(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_HORDE) || bg->IsActiveEvent(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_NEUTRAL)))
    {
        if (Creature* neutralMineBoss = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_NEUTRAL)))
        {
            if (bot->IsWithinDist(neutralMineBoss, VISIBILITY_DISTANCE_LARGE) && neutralMineBoss->GetDeathState() != DEAD && bg->IsActiveEvent(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_NEUTRAL))
            {
                objectiveLocation = WorldLocation(neutralMineBoss->GetMapId(), neutralMineBoss->GetPosition());
                return true;
            }
        }

        if (Creature* hordeMineBoss = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_HORDE)))
        {
            if (bot->IsWithinDist(hordeMineBoss, VISIBILITY_DISTANCE_LARGE) && hordeMineBoss->GetDeathState() != DEAD && bg->IsActiveEvent(BG_AV_MINE_BOSSES_SOUTH, TEAM_INDEX_HORDE))
            {
                objectiveLocation = WorldLocation(hordeMineBoss->GetMapId(), hordeMineBoss->GetPosition());
                return true;
            }
        }
    }

    // Block without condition
    {
        std::vector<WorldLocation> objectiveLocations;

        for (auto const& [nodeId, nodeStatus, locationName] : AV_AllianceAttackObjectives)
        {
            if (!bg->IsActiveEvent(nodeId, nodeStatus))
            {
                continue;
            }

            // Split team to capture 2 towers at same time
#ifndef MANGOSBOT_TWO  
            if (urand(0, 1) && nodeId == BG_AV_NODES_FROSTWOLF_ETOWER && bg->IsActiveEvent(BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_NODE_STATUS_HORDE_OCCUPIED))
#else
            if (urand(0, 1) && nodeId == BG_AV_NODE_FROSTWOLF_EAST && bg->IsActiveEvent(BG_AV_NODE_FROSTWOLF_WEST, BG_AV_NODE_STATUS_HORDE_OCCUPIED))
#endif
                continue;

            if (WorldLocation location; sRandomPlayerbotMgr.GetNamedLocation(locationName, location))
            {
                objectiveLocations.push_back(location);
            }
        }

        if (!objectiveLocations.empty())
        {
            objectiveLocation = objectiveLocations[urand(0, objectiveLocations.size() - 1)];
            return true;
        }
    }

    return false;
}

bool BGTactics::SelectAvObjectiveHorde(WorldLocation& objectiveLocation)
{
    BattleGround* bg = bot->GetBattleGround();
    if (!bg)
    {
        return false;
    }

    // End Boss
#ifndef MANGOSBOT_TWO  
    if (!bg->IsActiveEvent(BG_AV_NODES_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_ICEWING_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_STONEHEART_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODES_FIRSTAID_STATION, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#else
    if (!bg->IsActiveEvent(BG_AV_NODE_DUNBALDAR_SOUTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_ICEWING_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_STONEHEART_BUNKER, BG_AV_NODE_STATUS_ALLY_OCCUPIED) &&
        !bg->IsActiveEvent(BG_AV_NODE_GY_DUN_BALDAR, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#endif
    {
        if (Creature* pVanndar = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_BOSS_A, 0)))
        {
            objectiveLocation = WorldLocation(pVanndar->GetMapId(), pVanndar->GetPosition());
            return true;
        }
    }

    bool strifeTime = bg->GetStartTime() < (uint32)(10 * MINUTE * IN_MILLISECONDS);
    uint32 role = context->GetValue<uint32>("bg role")->Get();
    bool supporter = role < (uint32)(strifeTime ? 4 : 2); // first bunker strike team

    // Only go to Snowfall Graveyard if already close to it.
#ifndef MANGOSBOT_TWO  
    if (supporter && (bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_ALLY_CONTESTED) || bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_ALLY_OCCUPIED) || bg->IsActiveEvent(BG_AV_NODES_SNOWFALL_GRAVE, BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED)))
    {
#else
    if (supporter && (bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_ALLY_CONTESTED) || bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_ALLY_OCCUPIED) || bg->IsActiveEvent(BG_AV_NODE_GY_SNOWFALL, BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED)))
    {
#endif
        if (WorldLocation snowfallGraveyard; sRandomPlayerbotMgr.GetNamedLocation("AV_SNOWFALL_GRAVEYARD", snowfallGraveyard))
        {
            if (WorldPosition(bot).IsWithinDist(WorldPosition(snowfallGraveyard), VISIBILITY_DISTANCE_LARGE))
            {
                objectiveLocation = snowfallGraveyard;
                return true;
            }
        }
    }

    // Balinda
    if (!supporter && !bg->IsActiveEvent(BG_AV_NODE_CAPTAIN_DEAD_A, 0))
    {
        if (Creature* pBalinda = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_CAPTAIN_A, 0)))
        {
            if (pBalinda->GetHealth() > 0)
            {
                if (WorldLocation stoneheartOutpost; sRandomPlayerbotMgr.GetNamedLocation("AV_STONEHEART_OUTPOST_WAITING_HORDE", stoneheartOutpost))
                {
                    uint32 attackCount = getDefendersCount(Position(stoneheartOutpost.coord_x, stoneheartOutpost.coord_y, stoneheartOutpost.coord_z, stoneheartOutpost.orientation), 10.0f, false);

                    // Prepare to attack Captain
                    if (attackCount < 5 && !sServerFacade.IsInCombat(pBalinda))
                    {
                        objectiveLocation = stoneheartOutpost;
                    }
                    else
                    {
                        objectiveLocation = WorldLocation(pBalinda->GetMapId(), pBalinda->GetPosition());
                    }

                    return true;
                }
            }
        }
    }

    // Chance to defend
    if (supporter)
    {
        std::vector<WorldLocation> objectiveLocations;

        for (auto const& [nodeId, nodeStatus, locationName] : AV_HordeDefendObjectives)
        {
            if (!bg->IsActiveEvent(nodeId, nodeStatus))
            {
                continue;
            }

            if (WorldLocation location; sRandomPlayerbotMgr.GetNamedLocation(locationName, location))
            {
                objectiveLocations.push_back(location);
            }
        }

        if (!objectiveLocations.empty())
        {
            objectiveLocation = objectiveLocations[urand(0, objectiveLocations.size() - 1)];
            return true;
        }
    }

    // Mine capture (need paths & script fix)
#ifndef MANGOSBOT_TWO  
    if (!supporter && (bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_ALLIANCE) || bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_NEUTRAL)) &&
        !bg->IsActiveEvent(BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#else
    if (!supporter && (bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_ALLIANCE) || bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_NEUTRAL)) &&
        !bg->IsActiveEvent(BG_AV_NODE_GY_STORMPIKE, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#endif
    {
        if (Creature* neutralMineBoss = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_NEUTRAL)))
        {
            if (bot->IsWithinDist(neutralMineBoss, VISIBILITY_DISTANCE_GIGANTIC) && neutralMineBoss->GetDeathState() != DEAD && bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_NEUTRAL))
            {
                objectiveLocation = WorldLocation(neutralMineBoss->GetMapId(), neutralMineBoss->GetPosition());
                return true;
            }
        }

        if (Creature* allianceMineBoss = bot->GetMap()->GetCreature(bg->GetSingleCreatureGuid(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_ALLIANCE)))
        {
            if (bot->IsWithinDist(allianceMineBoss, VISIBILITY_DISTANCE_GIGANTIC) && allianceMineBoss->GetDeathState() != DEAD && bg->IsActiveEvent(BG_AV_MINE_BOSSES_NORTH, TEAM_INDEX_ALLIANCE))
            {
                objectiveLocation = WorldLocation(allianceMineBoss->GetMapId(), allianceMineBoss->GetPosition());
                return true;
            }
        }
    }

    // Block without condition
    {
        std::vector<WorldLocation> objectiveLocations;

        for (auto const& [nodeId, nodeStatus, locationName] : AV_HordeAttackObjectives)
        {
            if (!bg->IsActiveEvent(nodeId, nodeStatus))
            {
                continue;
            }

            // Split team to capture 2 towers at same time
#ifndef MANGOSBOT_TWO  
            if (urand(0, 1) && nodeId == BG_AV_NODES_DUNBALDAR_SOUTH && bg->IsActiveEvent(BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#else
            if (urand(0, 1) && nodeId == BG_AV_NODE_DUNBALDAR_SOUTH && bg->IsActiveEvent(BG_AV_NODE_DUNBALDAR_NORTH, BG_AV_NODE_STATUS_ALLY_OCCUPIED))
#endif
                continue;

            if (WorldLocation location; sRandomPlayerbotMgr.GetNamedLocation(locationName, location))
            {
                objectiveLocations.push_back(location);
            }
        }

        if (!objectiveLocations.empty())
        {
            objectiveLocation = objectiveLocations[urand(0, objectiveLocations.size() - 1)];
            return true;
        }
    }

    return false;
}

std::vector<uint32> const FlagEntries =
{
    BG_AV_GO_BANNER_ALLIANCE,
    BG_AV_GO_BANNER_ALLIANCE_CONT,
    BG_AV_GO_BANNER_HORDE,
    BG_AV_GO_BANNER_HORDE_CONT,
    BG_AV_GO_GY_BANNER_ALLIANCE,
    BG_AV_GO_GY_BANNER_ALLIANCE_CONT,
    BG_AV_GO_GY_BANNER_HORDE,
    BG_AV_GO_GY_BANNER_HORDE_CONT,
    BG_AV_GO_GY_BANNER_SNOWFALL,
};

bool BGTactics::CheckFlagAv()
{
    BattleGround* bg = bot->GetBattleGround();
    if (!bg)
    {
        return false;
    }

    BattleGroundTypeId bgType = bg->GetTypeId();
#ifdef MANGOSBOT_TWO
    if (bgType == BATTLEGROUND_RB)
        bgType = bg->GetTypeId(true);
#endif

    if (bgType != BATTLEGROUND_AV)
    {
        return false;
    }

    for (auto closeGameObjectGuid : (*context->GetValue<std::list<ObjectGuid>>("closest game objects static los")).Get())
    {
        GameObject* go = ai->GetGameObject(closeGameObjectGuid);
        if (!go)
            continue;

        std::vector<uint32>::const_iterator f = std::find(FlagEntries.begin(), FlagEntries.end(), go->GetEntry());
        if (f == FlagEntries.end())
            continue;

        if (!sServerFacade.isSpawned(go) || go->IsInUse() || go->GetGoState() != GO_STATE_READY)
            continue;

        if (!bot->CanInteract(go))
            continue;

        if (!bot->IsWithinDistInMap(go, INTERACTION_DISTANCE))
            continue;

        if (bot->IsMounted())
            bot->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        if (bot->IsInDisallowedMountForm())
            bot->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        // cast banner spell
        ai->StopMoving();

        SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(SPELL_CAPTURE_BANNER);
        if (!spellInfo)
            return false;

        Spell* spell = new Spell(bot, spellInfo, false);
        spell->m_targets.setGOTarget(go);
        spell->SpellStart(&spell->m_targets);
        ai->WaitForSpellCast(spell);

        //WorldPacket data(CMSG_GAMEOBJ_USE);
        //data << go->GetObjectGuid();
        //bot->GetSession()->HandleGameObjectUseOpcode(data);

        resetObjective();

        return true;
    }

    return false;
}
