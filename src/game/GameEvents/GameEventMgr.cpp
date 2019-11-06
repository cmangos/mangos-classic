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

#include "GameEvents/GameEventMgr.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Pools/PoolManager.h"
#include "ProgressBar.h"
#include "Tools/Language.h"
#include "Log.h"
#include "Maps/MapManager.h"
#include "BattleGround/BattleGroundMgr.h"
#include "Mails/MassMailMgr.h"
#include "Server/SQLStorages.h"
#include "Policies/Singleton.h"

INSTANTIATE_SINGLETON_1(GameEventMgr);

bool GameEventMgr::CheckOneGameEvent(uint16 entry, time_t currenttime) const
{
    // Get the event information
    return m_gameEvents[entry].start < currenttime && currenttime < m_gameEvents[entry].end && ((currenttime - m_gameEvents[entry].start) % (m_gameEvents[entry].occurence * MINUTE)) < (m_gameEvents[entry].length * MINUTE);
}

uint32 GameEventMgr::NextCheck(uint16 entry) const
{
    time_t currenttime = time(nullptr);

    // outdated event: we return max
    if (currenttime > m_gameEvents[entry].end)
        return max_ge_check_delay;

    // never started event, we return delay before start
    if (m_gameEvents[entry].start > currenttime)
        return uint32(m_gameEvents[entry].start - currenttime);

    uint32 delay;
    // in event, we return the end of it
    if ((((currenttime - m_gameEvents[entry].start) % (m_gameEvents[entry].occurence * 60)) < (m_gameEvents[entry].length * 60)))
        // we return the delay before it ends
        delay = (m_gameEvents[entry].length * MINUTE) - ((currenttime - m_gameEvents[entry].start) % (m_gameEvents[entry].occurence * MINUTE));
    else                                                    // not in window, we return the delay before next start
        delay = (m_gameEvents[entry].occurence * MINUTE) - ((currenttime - m_gameEvents[entry].start) % (m_gameEvents[entry].occurence * MINUTE));
    // In case the end is before next check
    if (m_gameEvents[entry].end  < time_t(currenttime + delay))
        return uint32(m_gameEvents[entry].end - currenttime);
    return delay;
}

void GameEventMgr::StartEvent(uint16 event_id, bool overwrite /*=false*/, bool resume /*=false*/)
{
    ApplyNewEvent(event_id, resume);
    if (overwrite)
    {
        m_gameEvents[event_id].start = time(nullptr);
        if (m_gameEvents[event_id].end <= m_gameEvents[event_id].start)
            m_gameEvents[event_id].end = m_gameEvents[event_id].start + m_gameEvents[event_id].length;
    }
}

void GameEventMgr::StopEvent(uint16 event_id, bool overwrite)
{
    UnApplyEvent(event_id);
    if (overwrite)
    {
        m_gameEvents[event_id].start = time(nullptr) - m_gameEvents[event_id].length * MINUTE;
        if (m_gameEvents[event_id].end <= m_gameEvents[event_id].start)
            m_gameEvents[event_id].end = m_gameEvents[event_id].start + m_gameEvents[event_id].length;
    }
}

void GameEventMgr::LoadFromDB()
{
    {
        std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT MAX(entry) FROM game_event"));
        if (!result)
        {
            sLog.outString(">> Table game_event is empty.");
            sLog.outString();
            return;
        }

        Field* fields = result->Fetch();

        uint32 max_event_id = fields[0].GetUInt16();

        m_gameEvents.resize(max_event_id + 1);
    }

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT entry,schedule_type,occurence,length,holiday,linkedTo,description FROM game_event"));
    if (!result)
    {
        m_gameEvents.clear();
        sLog.outString(">> Table game_event is empty!");
        sLog.outString();
        return;
    }

    uint32 count = 0;

    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();

            uint16 event_id = fields[0].GetUInt16();
            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event` game event id (%i) is reserved and can't be used.", event_id);
                continue;
            }

            GameEventData& gameEvent = m_gameEvents[event_id];
            gameEvent.scheduleType = GameEventScheduleType(fields[1].GetUInt32());
            gameEvent.occurence    = fields[2].GetUInt32();
            gameEvent.length       = fields[3].GetUInt32();
            gameEvent.holiday_id   = HolidayIds(fields[4].GetUInt32());
            gameEvent.linkedTo     = fields[5].GetUInt32();
            gameEvent.description  = fields[6].GetCppString();

            switch (gameEvent.scheduleType)
            {
                case GAME_EVENT_SCHEDULE_SERVERSIDE:
                    gameEvent.start = time_t(FAR_FUTURE);
                    gameEvent.end = time_t(FAR_FUTURE);
                    break;
                case GAME_EVENT_SCHEDULE_DATE: break; // loaded from table
                case GAME_EVENT_SCHEDULE_DMF_1:
                case GAME_EVENT_SCHEDULE_DMF_2:
                case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_1:
                case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_2:
                case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_1:
                case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_2:
                    ComputeEventStartAndEndTime(gameEvent);
                    break;
            }

            if (gameEvent.occurence == 0)
            {
                sLog.outBasic("Event %u (%s) disabled", event_id, gameEvent.description.c_str());
                gameEvent.start = time_t(FAR_FUTURE);
                gameEvent.occurence = gameEvent.length;
            }
			
            if (gameEvent.length == 0)                     // length>0 is validity check
            {
                sLog.outErrorDb("`game_event` game event id (%i) have length 0 and can't be used.", event_id);
                gameEvent.start = time_t(FAR_FUTURE);
            }

            if (gameEvent.occurence < gameEvent.length)   // occurence < length is useless. This also asserts that occurence > 0!
            {
                sLog.outErrorDb("`game_event` game event id (%i) has occurence %u  < length %u and can't be used.", event_id, gameEvent.occurence, gameEvent.length);
                gameEvent.start = time_t(FAR_FUTURE);
            }

            ++count;
        }
        while (result->NextRow());

        for (uint16 itr = 1; itr < m_gameEvents.size(); ++itr)
        {
            if (m_gameEvents[itr].isValid() && m_gameEvents[itr].linkedTo != 0 && (m_gameEvents[itr].linkedTo >= m_gameEvents.size() || !m_gameEvents[m_gameEvents[itr].linkedTo].isValid()))
            {
                sLog.outErrorDb("`game_event` game event id (%i) is Linked to invalid event %u", itr, m_gameEvents[itr].linkedTo);
                m_gameEvents[itr].linkedTo = 0;
            }
        }
        sLog.outString();
        sLog.outString(">> Loaded %u game events", count);
    }

    result.reset(WorldDatabase.Query("SELECT entry,UNIX_TIMESTAMP(start_time),UNIX_TIMESTAMP(end_time) FROM game_event_time"));
    if (!result)
    {
        sLog.outString(">> Table game_event_time is empty!");
        sLog.outString();
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint16 event_id = fields[0].GetUInt16();
            GameEventData& gameEvent = m_gameEvents[event_id];

            if (gameEvent.scheduleType != GAME_EVENT_SCHEDULE_DATE)
            {
                sLog.outErrorDb("`game_event` game event id (%i) has game_event_time but is not date scheduled - ignoring", event_id);
                continue;
            }

            if (gameEvent.start == time_t(FAR_FUTURE)) // skip loading time if previous loading errored
                continue;

            uint64 starttime = fields[1].GetUInt64();
            gameEvent.start = time_t(starttime);
            uint64 endtime = fields[2].GetUInt64();
            gameEvent.end = time_t(endtime);
        }
        while (result->NextRow());
    }

    std::map<uint16, int16> pool2event;                     // for check unique spawn event associated with pool
    std::map<uint32, int16> creature2event;                 // for check unique spawn event associated with creature
    std::map<uint32, int16> go2event;                       // for check unique spawn event associated with gameobject

    // list only positive event top pools, filled at creature/gameobject loading
    m_gameEventSpawnPoolIds.resize(m_gameEvents.size());

    m_gameEventCreatureGuids.resize(m_gameEvents.size() * 2 - 1);
    //                                   1              2
    result.reset(WorldDatabase.Query("SELECT creature.guid, game_event_creature.event "
                                 "FROM creature JOIN game_event_creature ON creature.guid = game_event_creature.guid"));

    count = 0;
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u creatures in game events", count);
    }
    else
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();

            uint32 guid    = fields[0].GetUInt32();
            int16 event_id = fields[1].GetInt16();

            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event_creature` game event id (%i) not allowed", event_id);
                continue;
            }

            if (!IsValidEvent(std::abs(event_id)))
            {
                sLog.outErrorDb("`game_event_creature` game event id (%i) not exist in `game_event`", event_id);
                continue;
            }

            int32 internal_event_id = m_gameEvents.size() + event_id - 1;

            ++count;

            // spawn objects at event can be grouped in pools and then affected pools have stricter requirements for this case
            if (event_id > 0)
            {
                creature2event[guid] = event_id;

                // not list explicitly creatures from pools in event creature list
                if (uint16 topPoolId =  sPoolMgr.IsPartOfTopPool<Creature>(guid))
                {
                    int16& eventRef = pool2event[topPoolId];
                    if (eventRef != 0)
                    {
                        if (eventRef != event_id)
                            sLog.outErrorDb("`game_event_creature` have creature (GUID: %u) for event %i from pool or subpool of pool (ID: %u) but pool have already content from event %i. Pool don't must have content for different events!", guid, event_id, topPoolId, eventRef);
                    }
                    else
                    {
                        eventRef = event_id;
                        m_gameEventSpawnPoolIds[event_id].push_back(topPoolId);
                        sPoolMgr.RemoveAutoSpawnForPool(topPoolId);
                    }

                    continue;
                }
            }

            GuidList& crelist = m_gameEventCreatureGuids[internal_event_id];
            crelist.push_back(guid);
        }
        while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u creatures in game events", count);
    }

    m_gameEventGameobjectGuids.resize(m_gameEvents.size() * 2 - 1);
    //                                   1                2
    result.reset(WorldDatabase.Query("SELECT gameobject.guid, game_event_gameobject.event "
                                 "FROM gameobject JOIN game_event_gameobject ON gameobject.guid=game_event_gameobject.guid"));

    count = 0;
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u gameobjects in game events", count);
    }
    else
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();

            uint32 guid    = fields[0].GetUInt32();
            int16 event_id = fields[1].GetInt16();

            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event_gameobject` game event id (%i) not allowed", event_id);
                continue;
            }

            if (!IsValidEvent(std::abs(event_id)))
            {
                sLog.outErrorDb("`game_event_gameobject` game event id (%i) not exist in `game_event`", event_id);
                continue;
            }

            int32 internal_event_id = m_gameEvents.size() + event_id - 1;

            ++count;

            // spawn objects at event can be grouped in pools and then affected pools have stricter requirements for this case
            if (event_id > 0)
            {
                go2event[guid] = event_id;

                // not list explicitly gameobjects from pools in event gameobject list
                if (uint16 topPoolId =  sPoolMgr.IsPartOfTopPool<GameObject>(guid))
                {
                    int16& eventRef = pool2event[topPoolId];
                    if (eventRef != 0)
                    {
                        if (eventRef != event_id)
                            sLog.outErrorDb("`game_event_gameobject` have gameobject (GUID: %u) for event %i from pool or subpool of pool (ID: %u) but pool have already content from event %i. Pool don't must have content for different events!", guid, event_id, topPoolId, eventRef);
                    }
                    else
                    {
                        eventRef = event_id;
                        m_gameEventSpawnPoolIds[event_id].push_back(topPoolId);
                        sPoolMgr.RemoveAutoSpawnForPool(topPoolId);
                    }

                    continue;
                }
            }

            GuidList& golist = m_gameEventGameobjectGuids[internal_event_id];
            golist.push_back(guid);
        }
        while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u gameobjects in game events", count);
    }

    // now recheck that all eventPools linked with events after our skip pools with parents
    for (std::map<uint16, int16>::const_iterator itr = pool2event.begin(); itr != pool2event.end();  ++itr)
    {
        uint16 pool_id = itr->first;
        int16 event_id = itr->second;

        sPoolMgr.CheckEventLinkAndReport(pool_id, event_id, creature2event, go2event);
    }

    m_gameEventCreatureData.resize(m_gameEvents.size());
    //                                   0              1                             2
    result.reset(WorldDatabase.Query("SELECT creature.guid, game_event_creature_data.event, game_event_creature_data.modelid,"
                                 //   3                                      4
                                 "game_event_creature_data.equipment_id, game_event_creature_data.entry_id, "
                                 //   5                                     6
                                 "game_event_creature_data.spell_start, game_event_creature_data.spell_end "
                                 "FROM creature JOIN game_event_creature_data ON creature.guid=game_event_creature_data.guid"));

    count = 0;
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u creature reactions at game events", count);
    }
    else
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();
            uint32 guid     = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt16();

            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event_creature_data` game event id (%i) is reserved and can't be used.", event_id);
                continue;
            }

            if (!IsValidEvent(event_id))
            {
                sLog.outErrorDb("`game_event_creature_data` game event id (%u) not exist in `game_event`", event_id);
                continue;
            }

            ++count;
            GameEventCreatureDataList& equiplist = m_gameEventCreatureData[event_id];
            GameEventCreatureData newData;
            newData.modelid = fields[2].GetUInt32();
            newData.equipment_id = fields[3].GetUInt32();
            newData.entry_id = fields[4].GetUInt32();
            newData.spell_id_start = fields[5].GetUInt32();
            newData.spell_id_end = fields[6].GetUInt32();

            if (newData.equipment_id && !sObjectMgr.GetEquipmentInfo(newData.equipment_id))
            {
                sLog.outErrorDb("Table `game_event_creature_data` have creature (Guid: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", guid, newData.equipment_id);
                newData.equipment_id = 0;
            }

            if (newData.entry_id && !ObjectMgr::GetCreatureTemplate(newData.entry_id))
            {
                sLog.outErrorDb("Table `game_event_creature_data` have creature (Guid: %u) with event time entry %u not found in table `creature_template`, set to no 0.", guid, newData.entry_id);
                newData.entry_id = 0;
            }

            if (newData.spell_id_start && !sSpellTemplate.LookupEntry<SpellEntry>(newData.spell_id_start))
            {
                sLog.outErrorDb("Table `game_event_creature_data` have creature (Guid: %u) with nonexistent spell_start %u, set to no start spell.", guid, newData.spell_id_start);
                newData.spell_id_start = 0;
            }

            if (newData.spell_id_end && !sSpellTemplate.LookupEntry<SpellEntry>(newData.spell_id_end))
            {
                sLog.outErrorDb("Table `game_event_creature_data` have creature (Guid: %u) with nonexistent spell_end %u, set to no end spell.", guid, newData.spell_id_end);
                newData.spell_id_end = 0;
            }

            equiplist.push_back(GameEventCreatureDataPair(guid, newData));
            m_gameEventCreatureDataPerGuid.insert(GameEventCreatureDataPerGuidMap::value_type(guid, event_id));
        }
        while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u creature reactions at game events", count);
    }

    m_gameEventQuests.resize(m_gameEvents.size());

    result.reset(WorldDatabase.Query("SELECT quest, event FROM game_event_quest"));

    count = 0;
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u quests additions in game events", count);
    }
    else
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();
            uint32 quest    = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt16();

            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event_quest` game event id (%i) is reserved and can't be used.", event_id);
                continue;
            }

            if (!IsValidEvent(event_id))
            {
                sLog.outErrorDb("`game_event_quest` game event id (%u) not exist in `game_event`", event_id);
                continue;
            }

            const Quest* pQuest = sObjectMgr.GetQuestTemplate(quest);

            if (!pQuest)
            {
                sLog.outErrorDb("Table `game_event_quest` contain entry for quest %u (event %u) but this quest does not exist. Skipping.", quest, event_id);
                continue;
            }

            // disable any event specific quest (for cases where creature is spawned, but event not active).
            const_cast<Quest*>(pQuest)->SetQuestActiveState(false);

            ++count;

            QuestList& questlist = m_gameEventQuests[event_id];
            questlist.push_back(quest);
        }
        while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u quest additions in game events", count);
    }

    m_gameEventMails.resize(m_gameEvents.size() * 2 - 1);

    result.reset(WorldDatabase.Query("SELECT event, raceMask, quest, mailTemplateId, senderEntry FROM game_event_mail"));

    count = 0;
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u start/end game event mails", count);
    }
    else
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();

            bar.step();
            uint16 event_id = fields[0].GetUInt16();

            GameEventMail mail;
            mail.raceMask       = fields[1].GetUInt32();
            mail.questId        = fields[2].GetUInt32();
            mail.mailTemplateId = fields[3].GetUInt32();
            mail.senderEntry    = fields[4].GetUInt32();

            if (event_id == 0)
            {
                sLog.outErrorDb("`game_event_mail` game event id (%i) not allowed", event_id);
                continue;
            }

            if (!IsValidEvent(event_id))
            {
                sLog.outErrorDb("`game_event_mail` game event id (%u) not exist in `game_event`", event_id);
                continue;
            }

            int32 internal_event_id = m_gameEvents.size() + event_id - 1;

            if (!(mail.raceMask & RACEMASK_ALL_PLAYABLE))
            {
                sLog.outErrorDb("Table `game_event_mail` have raceMask (%u) requirement for game event %i that not include any player races, ignoring.", mail.raceMask, event_id);
                continue;
            }

            if (mail.questId && !sObjectMgr.GetQuestTemplate(mail.questId))
            {
                sLog.outErrorDb("Table `game_event_mail` have nonexistent quest (%u) requirement for game event %i, ignoring.", mail.questId, event_id);
                continue;
            }

            if (!sMailTemplateStore.LookupEntry(mail.mailTemplateId))
            {
                sLog.outErrorDb("Table `game_event_mail` have invalid mailTemplateId (%u) for game event %i that invalid not include any player races, ignoring.", mail.mailTemplateId, event_id);
                continue;
            }

            if (!ObjectMgr::GetCreatureTemplate(mail.senderEntry))
            {
                sLog.outErrorDb("Table `game_event_mail` have nonexistent sender creature entry (%u) for game event %i that invalid not include any player races, ignoring.", mail.senderEntry, event_id);
                continue;
            }

            ++count;

            MailList& maillist = m_gameEventMails[internal_event_id];
            maillist.push_back(mail);
        }
        while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u start/end game event mails", count);
    }
}

uint32 GameEventMgr::Initialize()                           // return the next event delay in ms
{
    m_activeEvents.clear();

    ActiveEvents activeAtShutdown;

    if (QueryResult* result = CharacterDatabase.Query("SELECT event FROM game_event_status"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint16 event_id = fields[0].GetUInt16();
            activeAtShutdown.insert(event_id);
        }
        while (result->NextRow());
        delete result;

        CharacterDatabase.Execute("TRUNCATE game_event_status");
    }

    uint32 delay = Update(&activeAtShutdown);
    BASIC_LOG("Game Event system initialized.");
    m_isGameEventsInit = true;
    return delay;
}

void GameEventMgr::Initialize(MapPersistentState* state)
{
    // At map persistent state creating need only apply pool spawn modifications
    // other data is global and will be auto-apply
    for (uint16 m_ActiveEvent : m_activeEvents)
        for (IdList::iterator pool_itr = m_gameEventSpawnPoolIds[m_ActiveEvent].begin(); pool_itr != m_gameEventSpawnPoolIds[m_ActiveEvent].end(); ++pool_itr)
            sPoolMgr.InitSpawnPool(*state, *pool_itr);
}

// return the next event delay in ms
uint32 GameEventMgr::Update(ActiveEvents const* activeAtShutdown /*= nullptr*/)
{
    time_t currenttime = time(nullptr);

    uint32 nextEventDelay = max_ge_check_delay;             // 1 day
    for (uint16 itr = 1; itr < m_gameEvents.size(); ++itr)
    {
        if (m_gameEvents[itr].occurence == 0 || m_gameEvents[itr].scheduleType == GAME_EVENT_SCHEDULE_SERVERSIDE)
            continue;
        // sLog.outErrorDb("Checking event %u",itr);
        if (CheckOneGameEvent(itr, currenttime))
        {
            // DEBUG_LOG("GameEvent %u is active",itr->first);
            if (!IsActiveEvent(itr))
            {
                if (m_gameEvents[itr].linkedTo == 0 || IsActiveEvent(m_gameEvents[itr].linkedTo))
                {
                    bool resume = activeAtShutdown && (activeAtShutdown->find(itr) != activeAtShutdown->end());
                    StartEvent(itr, false, resume);
                }
            }
        }
        else
        {
            // DEBUG_LOG("GameEvent %u is not active",itr->first);
            if (IsActiveEvent(itr))
            {
                StopEvent(itr);
                if (m_gameEvents[itr].linkedTo != 0)
                    StopEvent(m_gameEvents[itr].linkedTo);
            }
            else
            {
                if (!m_isGameEventsInit)
                {
                    int16 event_nid = (-1) * (itr);
                    // spawn all negative ones for this event
                    GameEventSpawn(event_nid);
                }
            }
        }
        uint32 calcDelay = NextCheck(itr);
        if (calcDelay < nextEventDelay)
            nextEventDelay = calcDelay;
    }
    BASIC_LOG("Next game event check in %u seconds.", nextEventDelay + 1);
    return (nextEventDelay + 1) * IN_MILLISECONDS;          // Add 1 second to be sure event has started/stopped at next call
}

void GameEventMgr::UnApplyEvent(uint16 event_id)
{
    m_activeEvents.erase(event_id);
    CharacterDatabase.PExecute("DELETE FROM game_event_status WHERE event = %u", event_id);

    sLog.outString("GameEvent %u \"%s\" removed.", event_id, m_gameEvents[event_id].description.c_str());
    // un-spawn positive event tagged objects
    GameEventUnspawn(event_id);
    // spawn negative event tagget objects
    int16 event_nid = (-1) * event_id;
    GameEventSpawn(event_nid);
    // restore equipment or model
    UpdateCreatureData(event_id, false);
    // Remove quests that are events only to non event npc
    UpdateEventQuests(event_id, false);
    SendEventMails(event_nid);

    OnEventHappened(event_id, false, false);
}

void GameEventMgr::ApplyNewEvent(uint16 event_id, bool resume)
{
    m_activeEvents.insert(event_id);

    if (event_id == 987) //weekly server restart event
    {
        sWorld.ShutdownServ(m_gameEvents[event_id].length * 60, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);
        return;
    }

    CharacterDatabase.PExecute("INSERT INTO game_event_status (event) VALUES (%u)", event_id);

    if (sWorld.getConfig(CONFIG_BOOL_EVENT_ANNOUNCE))
        sWorld.SendWorldText(LANG_EVENTMESSAGE, m_gameEvents[event_id].description.c_str());

    sLog.outString("GameEvent %u \"%s\" started.", event_id, m_gameEvents[event_id].description.c_str());
    // spawn positive event tagget objects
    GameEventSpawn(event_id);
    // un-spawn negative event tagged objects
    int16 event_nid = (-1) * event_id;
    GameEventUnspawn(event_nid);
    // Change equipement or model
    UpdateCreatureData(event_id, true);
    // Add quests that are events only to non event npc
    UpdateEventQuests(event_id, true);

    // Not send mails at game event startup, if game event just resume after server shutdown (has been active at server before shutdown)
    if (!resume)
        SendEventMails(event_id);

    OnEventHappened(event_id, true, resume);
}

void GameEventMgr::GameEventSpawn(int16 event_id)
{
    int32 internal_event_id = m_gameEvents.size() + event_id - 1;

    if (internal_event_id < 0 || (size_t)internal_event_id >= m_gameEventCreatureGuids.size())
    {
        sLog.outError("GameEventMgr::GameEventSpawn attempt access to out of range mGameEventCreatureGuids element %i (size: " SIZEFMTD ")", internal_event_id, m_gameEventCreatureGuids.size());
        return;
    }

    for (uint32& itr : m_gameEventCreatureGuids[internal_event_id])
    {
        // Add to correct cell
        CreatureData const* data = sObjectMgr.GetCreatureData(itr);
        if (data)
        {
            // negative event id for pool element meaning allow be used in next pool spawn
            if (event_id < 0)
            {
                if (uint16 pool_id = sPoolMgr.IsPartOfAPool<Creature>(itr))
                {
                    // will have chance at next pool update
                    sPoolMgr.SetExcludeObject<Creature>(pool_id, itr, false);
                    sPoolMgr.UpdatePoolInMaps<Creature>(pool_id);
                    continue;
                }
            }

            sObjectMgr.AddCreatureToGrid(itr, data);

            Creature::SpawnInMaps(itr, data);
        }
    }

    if (internal_event_id < 0 || (size_t)internal_event_id >= m_gameEventGameobjectGuids.size())
    {
        sLog.outError("GameEventMgr::GameEventSpawn attempt access to out of range mGameEventGameobjectGuids element %i (size: " SIZEFMTD ")", internal_event_id, m_gameEventGameobjectGuids.size());
        return;
    }

    for (uint32& itr : m_gameEventGameobjectGuids[internal_event_id])
    {
        // Add to correct cell
        GameObjectData const* data = sObjectMgr.GetGOData(itr);
        if (data)
        {
            // negative event id for pool element meaning allow be used in next pool spawn
            if (event_id < 0)
            {
                if (uint16 pool_id = sPoolMgr.IsPartOfAPool<GameObject>(itr))
                {
                    // will have chance at next pool update
                    sPoolMgr.SetExcludeObject<GameObject>(pool_id, itr, false);
                    sPoolMgr.UpdatePoolInMaps<GameObject>(pool_id);
                    continue;
                }
            }

            sObjectMgr.AddGameobjectToGrid(itr, data);

            GameObject::SpawnInMaps(itr, data);
        }
    }

    if (event_id > 0)
    {
        if ((size_t)event_id >= m_gameEventSpawnPoolIds.size())
        {
            sLog.outError("GameEventMgr::GameEventSpawn attempt access to out of range mGameEventSpawnPoolIds element %i (size: " SIZEFMTD ")", event_id, m_gameEventSpawnPoolIds.size());
            return;
        }

        for (uint16& itr : m_gameEventSpawnPoolIds[event_id])
        sPoolMgr.SpawnPoolInMaps(itr, true);
    }
}

void GameEventMgr::GameEventUnspawn(int16 event_id)
{
    int32 internal_event_id = m_gameEvents.size() + event_id - 1;

    if (internal_event_id < 0 || (size_t)internal_event_id >= m_gameEventCreatureGuids.size())
    {
        sLog.outError("GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventCreatureGuids element %i (size: " SIZEFMTD ")", internal_event_id, m_gameEventCreatureGuids.size());
        return;
    }

    for (uint32& itr : m_gameEventCreatureGuids[internal_event_id])
    {
        // Remove the creature from grid
        if (CreatureData const* data = sObjectMgr.GetCreatureData(itr))
        {
            // negative event id for pool element meaning unspawn in pool and exclude for next spawns
            if (event_id < 0)
            {
                if (uint16 poolid = sPoolMgr.IsPartOfAPool<Creature>(itr))
                {
                    sPoolMgr.SetExcludeObject<Creature>(poolid, itr, true);
                    sPoolMgr.UpdatePoolInMaps<Creature>(poolid, itr);
                    continue;
                }
            }

            // Remove spawn data
            sObjectMgr.RemoveCreatureFromGrid(itr, data);

            // Remove spawned cases
            Creature::AddToRemoveListInMaps(itr, data);
        }
    }

    if (internal_event_id < 0 || (size_t)internal_event_id >= m_gameEventGameobjectGuids.size())
    {
        sLog.outError("GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventGameobjectGuids element %i (size: " SIZEFMTD ")", internal_event_id, m_gameEventGameobjectGuids.size());
        return;
    }

    for (uint32& itr : m_gameEventGameobjectGuids[internal_event_id])
    {
        // Remove the gameobject from grid
        if (GameObjectData const* data = sObjectMgr.GetGOData(itr))
        {
            // negative event id for pool element meaning unspawn in pool and exclude for next spawns
            if (event_id < 0)
            {
                if (uint16 poolid = sPoolMgr.IsPartOfAPool<GameObject>(itr))
                {
                    sPoolMgr.SetExcludeObject<GameObject>(poolid, itr, true);
                    sPoolMgr.UpdatePoolInMaps<GameObject>(poolid, itr);
                    continue;
                }
            }

            // Remove spawn data
            sObjectMgr.RemoveGameobjectFromGrid(itr, data);

            // Remove spawned cases
            GameObject::AddToRemoveListInMaps(itr, data);
        }
    }

    if (event_id > 0)
    {
        if ((size_t)event_id >= m_gameEventSpawnPoolIds.size())
        {
            sLog.outError("GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventSpawnPoolIds element %i (size: " SIZEFMTD ")", event_id, m_gameEventSpawnPoolIds.size());
            return;
        }

        for (uint16& itr : m_gameEventSpawnPoolIds[event_id])
        {
            sPoolMgr.DespawnPoolInMaps(itr);
        }
    }
}

GameEventCreatureData const* GameEventMgr::GetCreatureUpdateDataForActiveEvent(uint32 lowguid) const
{
    // only for active event, creature can be listed for many so search all
    uint32 event_id = 0;
    GameEventCreatureDataPerGuidBounds bounds = m_gameEventCreatureDataPerGuid.equal_range(lowguid);
    for (GameEventCreatureDataPerGuidMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (IsActiveEvent(itr->second))
        {
            event_id = itr->second;
            break;
        }
    }

    if (!event_id)
        return nullptr;

    for (const auto& itr : m_gameEventCreatureData[event_id])
        if (itr.first == lowguid)
            return &itr.second;

    return nullptr;
}

struct GameEventUpdateCreatureDataInMapsWorker
{
    GameEventUpdateCreatureDataInMapsWorker(ObjectGuid guid, CreatureData const* data, GameEventCreatureData* event_data, bool activate)
        : i_guid(guid), i_data(data), i_event_data(event_data), i_activate(activate) {}

    void operator()(Map* map)
    {
        if (Creature* pCreature = map->GetCreature(i_guid))
        {
            pCreature->UpdateEntry(i_data->id, i_data, i_activate ? i_event_data : nullptr);

            // spells not casted for event remove case (sent nullptr into update), do it
            if (!i_activate)
                pCreature->ApplyGameEventSpells(i_event_data, false);
        }
    }

    ObjectGuid i_guid;
    CreatureData const* i_data;
    GameEventCreatureData* i_event_data;
    bool i_activate;
};

void GameEventMgr::UpdateCreatureData(int16 event_id, bool activate)
{
    for (auto& itr : m_gameEventCreatureData[event_id])
    {
        // Remove the creature from grid
        CreatureData const* data = sObjectMgr.GetCreatureData(itr.first);
        if (!data)
            continue;

        // Update if spawned
        GameEventUpdateCreatureDataInMapsWorker worker(data->GetObjectGuid(itr.first), data, &itr.second, activate);
        sMapMgr.DoForAllMapsWithMapId(data->mapid, worker);
    }
}

void GameEventMgr::UpdateEventQuests(uint16 event_id, bool Activate)
{
    for (uint32& itr : m_gameEventQuests[event_id])
    {
        const Quest* pQuest = sObjectMgr.GetQuestTemplate(itr);

        // if (Activate)
        //{
        // TODO: implement way to reset quests when event begin.
        //}

        const_cast<Quest*>(pQuest)->SetQuestActiveState(Activate);
    }
}

void GameEventMgr::SendEventMails(int16 event_id)
{
    int32 internal_event_id = m_gameEvents.size() + event_id - 1;

    MailList const& mails = m_gameEventMails[internal_event_id];

    for (auto mail : mails)
    {
        if (mail.questId)
        {
            // need special query
            std::ostringstream ss;
            ss << "SELECT characters.guid FROM characters, character_queststatus "
               "WHERE (1 << (characters.race - 1)) & "
               << mail.raceMask
               << " AND characters.deleteDate IS NULL AND character_queststatus.guid = characters.guid AND character_queststatus.quest = "
               << mail.questId
               << " AND character_queststatus.rewarded <> 0";
            sMassMailMgr.AddMassMailTask(new MailDraft(mail.mailTemplateId), MailSender(MAIL_CREATURE, mail.senderEntry), ss.str().c_str());
        }
        else
            sMassMailMgr.AddMassMailTask(new MailDraft(mail.mailTemplateId), MailSender(MAIL_CREATURE, mail.senderEntry), mail.raceMask);
    }
}

// Get the Game Event ID for Creature by guid
template <>
int16 GameEventMgr::GetGameEventId<Creature>(uint32 guid_or_poolid)
{
    for (uint16 i = 0; i < m_gameEventCreatureGuids.size(); ++i) // 0 <= i <= 2*(S := m_gameEvents.size()) - 2
        for (GuidList::const_iterator itr = m_gameEventCreatureGuids[i].begin(); itr != m_gameEventCreatureGuids[i].end(); ++itr)
            if (*itr == guid_or_poolid)
                return i + 1 - m_gameEvents.size();       // -S *1 + 1 <= . <= 1*S - 1
    return 0;
}

// Get the Game Event ID for GameObject by guid
template <>
int16 GameEventMgr::GetGameEventId<GameObject>(uint32 guid_or_poolid)
{
    for (uint16 i = 0; i < m_gameEventGameobjectGuids.size(); ++i)
        for (GuidList::const_iterator itr = m_gameEventGameobjectGuids[i].begin(); itr != m_gameEventGameobjectGuids[i].end(); ++itr)
            if (*itr == guid_or_poolid)
                return i + 1 - m_gameEvents.size();       // -S *1 + 1 <= . <= 1*S - 1
    return 0;
}

// Get the Game Event ID for Pool by pool ID
template <>
int16 GameEventMgr::GetGameEventId<Pool>(uint32 guid_or_poolid)
{
    for (uint16 i = 0; i < m_gameEventSpawnPoolIds.size(); ++i)
        for (IdList::const_iterator itr = m_gameEventSpawnPoolIds[i].begin(); itr != m_gameEventSpawnPoolIds[i].end(); ++itr)
            if (*itr == guid_or_poolid)
                return i;
    return 0;
}

GameEventMgr::GameEventMgr()
{
    m_isGameEventsInit = false;
}

bool GameEventMgr::IsActiveHoliday(HolidayIds id)
{
    if (id == HOLIDAY_NONE)
        return false;

    for (uint16 m_ActiveEvent : m_activeEvents)
        if (m_gameEvents[m_ActiveEvent].holiday_id == id)
            return true;

    return false;
}

bool IsHolidayActive(HolidayIds id)
{
    return sGameEventMgr.IsActiveHoliday(id);
}

void GameEventMgr::OnEventHappened(uint16 event_id, bool activate, bool resume)
{
    sMapMgr.DoForAllMaps([event_id, activate, resume](Map * map) -> void
    {
        if (map->GetInstanceData())
            map->OnEventHappened(event_id, activate, resume);
    });
}

void GameEventMgr::ComputeEventStartAndEndTime(GameEventData& data)
{
    time_t curTime = time(nullptr);
    const tm* t = localtime(&curTime);
    tm firstMonday = *t;
    firstMonday.tm_sec = 0;
    firstMonday.tm_min = 0;
    firstMonday.tm_hour = 0;
    int monthRemainder = firstMonday.tm_mon % 2;
    switch (data.scheduleType)
    {
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_1:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_1:
        case GAME_EVENT_SCHEDULE_DMF_1: firstMonday.tm_mon += (2 - monthRemainder) % 2; break;
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_2:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_2:
        case GAME_EVENT_SCHEDULE_DMF_2: firstMonday.tm_mon += (2 - monthRemainder + 1) % 2; break;
    }
    mktime(&firstMonday);
    firstMonday.tm_mday = (firstMonday.tm_mday - 1 - (firstMonday.tm_wday - 1) + 7) % 7 + 1; // get this weeks monday
    switch(data.scheduleType)
    {
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_1:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_2:
            firstMonday.tm_mday -= 2; break;
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_1:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_2:
            firstMonday.tm_mday -= 1; break;
    }
    data.start = mktime(&firstMonday);
    switch (data.scheduleType)
    {
        case GAME_EVENT_SCHEDULE_DMF_1:
        case GAME_EVENT_SCHEDULE_DMF_2:
            firstMonday.tm_mday += 7; break;
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_1:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_2:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_1:
        case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_2:
            firstMonday.tm_mday += 1; break;
    }
    data.end = mktime(&firstMonday);
}

void GameEventMgr::WeeklyEventTimerRecalculation()
{
    for (GameEventData& gameEvent : m_gameEvents)
    {
        switch (gameEvent.scheduleType)
        {
            case GAME_EVENT_SCHEDULE_DMF_1:
            case GAME_EVENT_SCHEDULE_DMF_2:
            case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_1:
            case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_1_2:
            case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_1:
            case GAME_EVENT_SCHEDULE_DMF_BUILDING_STAGE_2_2:
                ComputeEventStartAndEndTime(gameEvent);
                break;
            default: break;
        }
    }
}
