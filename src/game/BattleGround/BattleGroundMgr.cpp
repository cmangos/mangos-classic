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

#include "Common.h"
#include "Globals/SharedDefines.h"
#include "Entities/Player.h"
#include "BattleGroundMgr.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundWS.h"
#include "Maps/MapManager.h"
#include "Maps/Map.h"
#include "Globals/ObjectMgr.h"
#include "Util/ProgressBar.h"
#include "Chat/Chat.h"
#include "World/World.h"
#include "Server/WorldPacket.h"
#include "Tools/Language.h"
#include "GameEvents/GameEventMgr.h"

#include "Policies/Singleton.h"

INSTANTIATE_SINGLETON_1(BattleGroundMgr);

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattleGroundMgr::BattleGroundMgr() : m_testing(false)
{
    for (uint8 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
        m_battleGrounds[i].clear();
}

BattleGroundMgr::~BattleGroundMgr()
{
    DeleteAllBattleGrounds();
}

/**
  Method that deletes all battlegrounds
*/
void BattleGroundMgr::DeleteAllBattleGrounds()
{
    // will also delete template bgs:
    for (uint8 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
        m_battleGrounds[i].clear();
}

/**
  Update method

  @param    diff
*/
void BattleGroundMgr::Update(uint32 /*diff*/)
{
    m_messager.Execute(this);
}

/**
  Send battleground status update

  @param    packet
  @param    battleground
  @param    queue slot
  @param    status id
  @param    time1
  @param    time2
*/
void BattleGroundMgr::BuildBattleGroundStatusPacket(WorldPacket & data, bool bgExists, uint32 bgTypeId, uint32 bgClientInstanceId, uint32 mapId, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2)
{
    // we can be in 3 queues in same time...

    if (statusId == 0 || !bgExists)
    {
        data.Initialize(SMSG_BATTLEFIELD_STATUS, 4 * 2);
        data << uint32(queueSlot);                         // queue id (0...2)
        data << uint32(0);
        return;
    }

    data.Initialize(SMSG_BATTLEFIELD_STATUS, (4 + 8 + 4 + 1 + 4 + 4 + 4));
    data << uint32(queueSlot);                             // queue id (0...2) - player can be in 3 queues in time
    // uint64 in client
    data << uint32(mapId);                        // MapID
    data << uint8(0);                                      // Unknown
    data << uint32(bgClientInstanceId);
    data << uint32(statusId);                              // status
    switch (statusId)
    {
        case STATUS_WAIT_QUEUE:                            // status_in_queue
            data << uint32(time1);                         // average wait time, milliseconds
            data << uint32(time2);                         // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:                              // status_invite
            data << uint32(time1);                         // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:                            // status_in_progress
            data << uint32(time1);                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            data << uint32(time2);                         // time from bg start, milliseconds
            break;
        default:
            sLog.outError("Unknown BG status!");
            break;
    }
}

/**
  Send battleground log update

  @param    packet
  @param    battleground
*/
void BattleGroundMgr::BuildPvpLogDataPacket(WorldPacket& data, BattleGround* bg)
{
    data.Initialize(MSG_PVP_LOG_DATA, (1 + 4 + 40 * bg->GetPlayerScoresSize()));

    if (bg->GetStatus() != STATUS_WAIT_LEAVE)
    {
        data << uint8(0);                                  // bg not ended
    }
    else
    {
        data << uint8(1);                                  // bg ended
        data << uint8(bg->GetWinner());                    // who won
    }

    data << (uint32)(bg->GetPlayerScoresSize());

    for (BattleGround::BattleGroundScoreMap::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        const BattleGroundScore* score = itr->second;

        data << ObjectGuid(itr->first);

        Player* plr = sObjectMgr.GetPlayer(itr->first);

        data << uint32(plr ? plr->GetHonorRankInfo().rank : 4);
        data << uint32(itr->second->killingBlows);
        data << uint32(itr->second->honorableKills);
        data << uint32(itr->second->deaths);
        data << uint32(itr->second->bonusHonor);

        switch (bg->GetTypeId())                            // battleground specific things
        {
            case BATTLEGROUND_AV:
                data << (uint32)0x00000007;                // count of next fields
                data << (uint32)((BattleGroundAVScore*)score)->graveyardsAssaulted;  // GraveyardsAssaulted
                data << (uint32)((BattleGroundAVScore*)score)->graveyardsDefended;   // GraveyardsDefended
                data << (uint32)((BattleGroundAVScore*)score)->towersAssaulted;      // TowersAssaulted
                data << (uint32)((BattleGroundAVScore*)score)->towersDefended;       // TowersDefended
                data << (uint32)((BattleGroundAVScore*)score)->secondaryObjectives;  // Mines Taken
                data << (uint32)((BattleGroundAVScore*)score)->lieutnantCount;       // Lieutnant kills
                data << (uint32)((BattleGroundAVScore*)score)->secondaryNPC;         // Secondary unit summons
                break;
            case BATTLEGROUND_WS:
                data << (uint32)0x00000002;                // count of next fields
                data << (uint32)((BattleGroundWGScore*)score)->flagCaptures;         // flag captures
                data << (uint32)((BattleGroundWGScore*)score)->flagReturns;          // flag returns
                break;
            case BATTLEGROUND_AB:
                data << (uint32)0x00000002;                // count of next fields
                data << (uint32)((BattleGroundABScore*)score)->basesAssaulted;       // bases asssulted
                data << (uint32)((BattleGroundABScore*)score)->basesDefended;        // bases defended
                break;
            default:
                DEBUG_LOG("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetTypeId());
                data << (uint32)0;
                break;
        }
    }
}

/**
  Send battleground joined packet

  @param    packet
  @param    battleground type id
  @param    battleground group join status
*/
void BattleGroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket& data, BattleGroundTypeId bgTypeId, BattleGroundGroupJoinStatus status)
{
    data.Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    switch (status)
    {
        case BG_GROUP_JOIN_STATUS_DESERTERS:    // You cannot join the battleground yet because you or one of your party members is flagged as a Deserter.
        case BG_GROUP_JOIN_STATUS_NOT_ELIGIBLE: // Your group has joined a battleground queue, but you are not eligible
            data << int32(status);
            break;
        case BG_GROUP_JOIN_STATUS_SUCCESS:      // Your group has joined the queue for [map]
            data << int32(GetBattleGrounMapIdByTypeId(bgTypeId));
            break;
    }
}

/**
  Send battleground world state packet

  @param    packet
  @param    field
  @param    value
*/
void BattleGroundMgr::BuildUpdateWorldStatePacket(WorldPacket& data, uint32 field, uint32 value)
{
    data.Initialize(SMSG_UPDATE_WORLD_STATE, 4 + 4);
    data << uint32(field);
    data << uint32(value);
}

/**
  Send battleground sound packet

  @param    packet
  @param    sound id
*/
void BattleGroundMgr::BuildPlaySoundPacket(WorldPacket& data, uint32 soundId)
{
    data.Initialize(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId);
}

/**
  Send player leave from battleground packet

  @param    packet
  @param    object guid
*/
void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket& data, ObjectGuid guid)
{
    data.Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    data << ObjectGuid(guid);
}

/**
  Send player join from battleground packet

  @param    packet
  @param    player
*/
void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket& data, Player* player)
{
    data.Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    data << player->GetObjectGuid();
}

/**
  Function that returns battleground from client instance id and battleground type

  @param    instance id
  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGroundThroughClientInstance(uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    // cause at HandleBattleGroundJoinOpcode the clients sends the instanceid he gets from
    // SMSG_BATTLEFIELD_LIST we need to find the battleground with this clientinstance-id
    BattleGround* bg = GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return nullptr;

    for (auto& itr : m_battleGrounds[bgTypeId])
    {
        if (itr.second->GetClientInstanceId() == instanceId)
            return itr.second.get();
    }

    return nullptr;
}

/**
  Function that returns battleground from instance id and battleground type

  @param    instance id
  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    // search if needed
    BattleGroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for (uint8 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
        {
            itr = m_battleGrounds[i].find(instanceId);
            if (itr != m_battleGrounds[i].end())
                return itr->second.get();
        }
        return nullptr;
    }

    itr = m_battleGrounds[bgTypeId].find(instanceId);
    return ((itr != m_battleGrounds[bgTypeId].end()) ? itr->second.get() : nullptr);
}

/**
  Function that returns battleground from template id

  @param    battleground type id
*/
BattleGround* BattleGroundMgr::GetBattleGroundTemplate(BattleGroundTypeId bgTypeId) const
{
    // map is sorted and we can be sure that lowest instance id has only BG template
    return m_battleGrounds[bgTypeId].empty() ? nullptr : m_battleGrounds[bgTypeId].begin()->second.get();
}

/**
  Function that creates a new battleground that is actually used

  @param    battleground type id
  @param    bracket entry
  @param    arena type
  @param    isRated
*/
BattleGround* BattleGroundMgr::CreateNewBattleGround(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracketId, uint32 instanceId, uint32 clientInstanceId)
{
    // get the template BG
    BattleGround* bgTemplate = GetBattleGroundTemplate(bgTypeId);
    if (!bgTemplate)
    {
        sLog.outError("BattleGround: CreateNewBattleGround - bg template not found for %u", bgTypeId);
        return nullptr;
    }

    BattleGround* bg = nullptr;
    // create a copy of the BG template
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV(*(BattleGroundAV*)bgTemplate);
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS(*(BattleGroundWS*)bgTemplate);
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB(*(BattleGroundAB*)bgTemplate);
            break;
        default:
            // error, but it is handled few lines above
            return nullptr;
    }

    // will also set m_bgMap, instanceid
    sMapMgr.CreateBgMap(bg->GetMapId(), instanceId, bg);

    bg->SetClientInstanceId(clientInstanceId);

    // reset the new bg (set status to status_wait_queue from status_none)
    bg->Reset();

    // start the joining of the bg
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetBracketId(bracketId);

    return bg;
}

/**
  Function that creates battleground templates

  @param    battleground type id
  @param    min players per team
  @param    max players per team
  @param    level min
  @param    level max
  @param    battleground name
  @param    map id
  @param    team 1 start location X
  @param    team 1 start location Y
  @param    team 1 start location Z
  @param    team 1 start location O
  @param    team 2 start location X
  @param    team 2 start location Y
  @param    team 2 start location Z
  @param    team 2 start location O
  @param    start max distance
*/
uint32 BattleGroundMgr::CreateBattleGround(BattleGroundTypeId bgTypeId, uint32 minPlayersPerTeam, uint32 maxPlayersPerTeam, uint32 levelMin, uint32 levelMax, char const* battleGroundName, uint32 mapId, float team1StartLocX, float team1StartLocY, float team1StartLocZ, float team1StartLocO, float team2StartLocX, float team2StartLocY, float team2StartLocZ, float team2StartLocO, float startMaxDist, uint32 playerSkinReflootId)
{
    // Create the BG
    BattleGround* bg;
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: bg = new BattleGroundAV; break;
        case BATTLEGROUND_WS: bg = new BattleGroundWS; break;
        case BATTLEGROUND_AB: bg = new BattleGroundAB; break;
        default:              bg = new BattleGround;   break;                           // placeholder for non implemented BG
    }

    bg->SetMapId(mapId);
    bg->SetTypeId(bgTypeId);
    bg->SetMinPlayersPerTeam(minPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(maxPlayersPerTeam);
    bg->SetMinPlayers(minPlayersPerTeam * 2);
    bg->SetMaxPlayers(maxPlayersPerTeam * 2);
    bg->SetName(battleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, team1StartLocX, team1StartLocY, team1StartLocZ, team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    team2StartLocX, team2StartLocY, team2StartLocZ, team2StartLocO);
    bg->SetStartMaxDist(startMaxDist);
    bg->SetLevelRange(levelMin, levelMax);
    bg->SetPlayerSkinRefLootId(playerSkinReflootId);

    // add bg to update list
    AddBattleGround(bg->GetInstanceId(), bg->GetTypeId(), bg);

    // return some not-null value, bgTypeId is good enough for me
    return bgTypeId;
}

void BattleGroundMgr::AddBattleGround(uint32 instanceId, BattleGroundTypeId bgTypeId, BattleGround* bg)
{
    MaNGOS::unique_trackable_ptr<BattleGround>& ptr = m_battleGrounds[bgTypeId][instanceId];
    ptr.reset(bg);
    bg->SetWeakPtr(ptr);
}

/**
  Method that loads battleground data from DB
*/
void BattleGroundMgr::CreateInitialBattleGrounds()
{
    uint32 count = 0;
    //                                             0   1                 2                 3      4      5                6             7            8
    auto queryResult = WorldDatabase.Query("SELECT id, MinPlayersPerTeam,MaxPlayersPerTeam,MinLvl,MaxLvl,AllianceStartLoc,HordeStartLoc,StartMaxDist,PlayerSkinReflootId FROM battleground_template");

    if (!queryResult)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        sLog.outString();
        return;
    }

    BarGoLink bar(queryResult->GetRowCount());

    do
    {
        Field* fields = queryResult->Fetch();
        bar.step();

        uint32 resultedBgTypeId = fields[0].GetUInt32();

        BattleGroundTypeId bgTypeId = BattleGroundTypeId(resultedBgTypeId);

        uint32 minPlayersPerTeam = fields[1].GetUInt32();
        uint32 maxPlayersPerTeam = fields[2].GetUInt32();
        uint32 minLvl = fields[3].GetUInt32();
        uint32 maxLvl = fields[4].GetUInt32();

        float allianceStartLoc[4];
        float hordeStartLoc[4];

        uint32 start1 = fields[5].GetUInt32();

        WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(start1);
        if (start)
        {
            allianceStartLoc[0] = start->x;
            allianceStartLoc[1] = start->y;
            allianceStartLoc[2] = start->z;
            allianceStartLoc[3] = start->o;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have nonexistent WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", bgTypeId, start1);
            continue;
        }

        uint32 start2 = fields[6].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(start2);
        if (start)
        {
            hordeStartLoc[0] = start->x;
            hordeStartLoc[1] = start->y;
            hordeStartLoc[2] = start->z;
            hordeStartLoc[3] = start->o;
        }
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", bgTypeId, start2);
            continue;
        }

        uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);
        char const* name;

        if (MapEntry const* mapEntry = sMapStore.LookupEntry(mapId))
            name = mapEntry->name[sWorld.GetDefaultDbcLocale()];
        else
        {
            sLog.outErrorDb("Table `battleground_template` for id %u associated with nonexistent map id %u.", bgTypeId, mapId);
            continue;
        }

        float startMaxDist = fields[7].GetFloat();
        uint32 playerSkinReflootId = fields[8].GetUInt32();
        if (playerSkinReflootId && !sLootMgr.ExistsRefLootTemplate(playerSkinReflootId))
        {
            playerSkinReflootId = 0;
            sLog.outErrorDb("Table `battleground_template` for id %u associated with nonexistent refloot id %u. Setting to 0.", bgTypeId, playerSkinReflootId);
        }

        if (playerSkinReflootId)
            m_usedRefloot.insert(playerSkinReflootId);

        // sLog.outDetail("Creating battleground %s, %u-%u", bl->name[sWorld.GetDBClang()], MinLvl, MaxLvl);
        if (!CreateBattleGround(bgTypeId, minPlayersPerTeam, maxPlayersPerTeam, minLvl, maxLvl, name, mapId, allianceStartLoc[0], allianceStartLoc[1], allianceStartLoc[2], allianceStartLoc[3], hordeStartLoc[0], hordeStartLoc[1], hordeStartLoc[2], hordeStartLoc[3], startMaxDist, playerSkinReflootId))
            continue;

        ++count;
    }
    while (queryResult->NextRow());

    sLog.outString(">> Loaded %u battlegrounds", count);
    sLog.outString();
}

/**
  Method that sends player to battleground

  @param    player
  @param    instance id
  @param    battleground type id
*/
void BattleGroundMgr::SendToBattleGround(Player* player, uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    BattleGround* bg = GetBattleGround(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        Team team = player->GetBGTeam();
        if (team == 0)
            team = player->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        DETAIL_LOG("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", player->GetName(), mapid, x, y, z, O);
        player->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog.outError("player %u trying to port to nonexistent bg instance %u", player->GetGUIDLow(), instanceId);
    }
}

/**
  Function that returns the queue type for battleground type id

  @param    battleground type id
  @param    arena type
*/
BattleGroundQueueTypeId BattleGroundMgr::BgQueueTypeId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

/**
  Function that returns the battleground type id from battleground queue id

  @param    battleground queue id
*/
BattleGroundTypeId BattleGroundMgr::BgTemplateId(BattleGroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        default:
            return BattleGroundTypeId(0);                   // used for unknown template (it exist and do nothing)
    }
}

/**
  Method that toggles battleground test mode on / off
*/
void BattleGroundMgr::ToggleTesting()
{
    m_testing = !m_testing;
    if (m_testing)
        sWorld.SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld.SendWorldText(LANG_DEBUG_BG_OFF);
    sWorld.GetBGQueue().GetMessager().AddMessage([testing = m_testing](BattleGroundQueue* queue)
    {
        queue->SetTesting(testing);
    });
}

uint32 BattleGroundMgr::GetPrematureFinishTime() const
{
    return sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

/**
  Method that loads battlemaster entries from DB
*/
void BattleGroundMgr::LoadBattleMastersEntry(bool reload)
{
    std::shared_ptr<BattleMastersMap> newBattleMastersMap = std::make_shared<BattleMastersMap>();

    auto queryResult = WorldDatabase.Query("SELECT entry,bg_template FROM battlemaster_entry");

    uint32 count = 0;

    if (!queryResult)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 battlemaster entries - table is empty!");
        sLog.outString();
        return;
    }

    BarGoLink bar(queryResult->GetRowCount());

    do
    {
        ++count;
        bar.step();

        Field* fields = queryResult->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
        {
            sLog.outErrorDb("Table `battlemaster_entry` contain entry %u for nonexistent battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        (*newBattleMastersMap)[entry] = BattleGroundTypeId(bgTypeId);
    }
    while (queryResult->NextRow());

    m_battleMastersMap = newBattleMastersMap;

    if (reload)
    {
        sMapMgr.DoForAllMaps([battleMasters = newBattleMastersMap](Map* map)
        {
            map->GetMessager().AddMessage([battleMasters](Map* map)
            {
                map->GetMapDataContainer().SetBattleMastersMap(battleMasters);
            });
        });
    }

    sLog.outString(">> Loaded %u battlemaster entries", count);
    sLog.outString();
}

/**
  Function that returns the holiday id from battleground type id

  @param    battleground type id
*/
HolidayIds BattleGroundMgr::BgTypeToWeekendHolidayId(BattleGroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_WS: return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_AB: return HOLIDAY_CALL_TO_ARMS_AB;
        default: return HOLIDAY_NONE;
    }
}

/**
  Function that returns the battleground type id from holiday id

  @param    holiday id
*/
BattleGroundTypeId BattleGroundMgr::WeekendHolidayIdToBgType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV: return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_WS: return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_AB: return BATTLEGROUND_AB;
        default: return BATTLEGROUND_TYPE_NONE;
    }
}

/**
  Function that checks if it's an active battleground holiday weekend

  @param    battleground type id
*/
bool BattleGroundMgr::IsBgWeekend(BattleGroundTypeId bgTypeId)
{
    return sGameEventMgr.IsActiveHoliday(BgTypeToWeekendHolidayId(bgTypeId));
}

/**
  Method that loads battleground events used in battleground scripts
*/
void BattleGroundMgr::LoadBattleEventIndexes(bool reload)
{
    BattleGroundEventIdx events;
    events.event1 = BG_EVENT_NONE;
    events.event2 = BG_EVENT_NONE;
    std::shared_ptr<GameObjectBattleEventIndexesMap> newGameObjectIndexes = std::make_shared<GameObjectBattleEventIndexesMap>();
    (*newGameObjectIndexes)[static_cast<uint32>(-1)] = events;
    std::shared_ptr<CreatureBattleEventIndexesMap> newCreatureIndexes = std::make_shared<CreatureBattleEventIndexesMap>();
    (*newCreatureIndexes)[static_cast<uint32>(-1)] = events;

    uint32 count = 0;

    auto queryResult =
        //                           0         1           2                3                4              5           6
        WorldDatabase.Query("SELECT data.typ, data.guid1, data.ev1 AS ev1, data.ev2 AS ev2, data.map AS m, data.guid2, description.map, "
                            //                              7                  8                   9
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "RIGHT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            // full outer join doesn't work in mysql :-/ so just UNION-select the same again and add a left outer join
                            "UNION "
                            "SELECT data.typ, data.guid1, data.ev1, data.ev2, data.map, data.guid2, description.map, "
                            "description.event1, description.event2, description.description "
                            "FROM "
                            "(SELECT '1' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM gameobject_battleground AS a "
                            "LEFT OUTER JOIN gameobject AS b ON a.guid = b.guid "
                            "UNION "
                            "SELECT '2' AS typ, a.guid AS guid1, a.event1 AS ev1, a.event2 AS ev2, b.map AS map, b.guid AS guid2 "
                            "FROM creature_battleground AS a "
                            "LEFT OUTER JOIN creature AS b ON a.guid = b.guid "
                            ") data "
                            "LEFT OUTER JOIN battleground_events AS description ON data.map = description.map "
                            "AND data.ev1 = description.event1 AND data.ev2 = description.event2 "
                            "ORDER BY m, ev1, ev2");
    if (!queryResult)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded 0 battleground eventindexes.");
        sLog.outString();
        return;
    }

    BarGoLink bar(queryResult->GetRowCount());

    do
    {
        bar.step();
        Field* fields = queryResult->Fetch();
        if (fields[2].GetUInt8() == BG_EVENT_NONE || fields[3].GetUInt8() == BG_EVENT_NONE)
            continue;                                       // we don't need to add those to the eventmap

        bool gameobject         = (fields[0].GetUInt8() == 1);
        uint32 dbTableGuidLow   = fields[1].GetUInt32();
        events.event1           = fields[2].GetUInt8();
        events.event2           = fields[3].GetUInt8();
        uint32 map              = fields[4].GetUInt32();

        uint32 desc_map = fields[6].GetUInt32();
        uint8 desc_event1 = fields[7].GetUInt8();
        uint8 desc_event2 = fields[8].GetUInt8();
        const char* description = fields[9].GetString();

        // checking for nullptr - through right outer join this will mean following:
        if (fields[5].GetUInt32() != dbTableGuidLow)
        {
            sLog.outErrorDb("BattleGroundEvent: %s with nonexistent guid %u for event: map:%u, event1:%u, event2:%u (\"%s\")",
                            (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2, description);
            continue;
        }

        // checking for nullptr - through full outer join this can mean 2 things:
        if (desc_map != map)
        {
            // there is an event missing
            if (dbTableGuidLow == 0)
            {
                sLog.outErrorDb("BattleGroundEvent: missing db-data for map:%u, event1:%u, event2:%u (\"%s\")", desc_map, desc_event1, desc_event2, description);
                continue;
            }
            // we have an event which shouldn't exist
            sLog.outErrorDb("BattleGroundEvent: %s with guid %u is registered, for a nonexistent event: map:%u, event1:%u, event2:%u",
                    (gameobject) ? "gameobject" : "creature", dbTableGuidLow, map, events.event1, events.event2);
            continue;
        }

        if (gameobject)
            (*newGameObjectIndexes)[dbTableGuidLow] = events;
        else
            (*newCreatureIndexes)[dbTableGuidLow] = events;

        ++count;
    }
    while (queryResult->NextRow());

    m_gameObjectBattleEventIndexMap = newGameObjectIndexes;
    m_creatureBattleEventIndexMap = newCreatureIndexes;

    if (reload)
    {
        sMapMgr.DoForAllMaps([gameobjects = newGameObjectIndexes, creatures = newCreatureIndexes](Map* map)
        {
            map->GetMessager().AddMessage([gameobjects, creatures](Map* map)
            {
                map->GetMapDataContainer().SetGameObjectEventIndexes(gameobjects);
                map->GetMapDataContainer().SetCreatureEventIndexes(creatures);
            });
        });
    }

    sLog.outString(">> Loaded %u battleground eventindexes", count);
    sLog.outString();
}

uint32 BattleGroundMgr::GetMinLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId) const
{
    if (bracket_id < 1)
        return 0;

    if (bracket_id > BG_BRACKET_ID_LAST)
        bracket_id = BG_BRACKET_ID_LAST;

    BattleGround* bg = GetBattleGroundTemplate(bgTypeId);
    assert(bg);
    return 10 * bracket_id + bg->GetMinLevel();
}

uint32 BattleGroundMgr::GetMaxLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId) const
{
    if (bracket_id >= BG_BRACKET_ID_LAST)
        return 255;                                         // hardcoded max level

    return GetMinLevelForBattleGroundBracketId(bracket_id, bgTypeId) + 10;
}

BattleGroundBracketId BattleGroundMgr::GetBattleGroundBracketIdFromLevel(BattleGroundTypeId bgTypeId, uint32 playerLevel) const
{
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    assert(bg);
    if (playerLevel < bg->GetMinLevel())
        return BG_BRACKET_ID_FIRST;

    uint32 bracket_id = (playerLevel - bg->GetMinLevel()) / 10;
    if (bracket_id > MAX_BATTLEGROUND_BRACKETS)
        return BG_BRACKET_ID_LAST;

    return BattleGroundBracketId(bracket_id);
}
