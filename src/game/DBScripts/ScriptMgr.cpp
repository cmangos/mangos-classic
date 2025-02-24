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

#include "DBScripts/ScriptMgr.h"
#include "Policies/Singleton.h"
#include "Log/Log.h"
#include "Util/ProgressBar.h"
#include "Globals/ObjectMgr.h"
#include "Globals/ObjectAccessor.h"
#include "MotionGenerators/WaypointManager.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Server/SQLStorages.h"
#include "BattleGround/BattleGround.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "MotionGenerators/WaypointMovementGenerator.h"
#include "Mails/Mail.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Maps/InstanceData.h"
#include "Entities/Object.h"
#include "Entities/Transports.h"

#include <string_view>

INSTANTIATE_SINGLETON_1(ScriptMgr);

constexpr std::array<std::string_view, SCRIPT_TYPE_MAX> scriptTableNames
{
    "dbscripts_on_quest_end",
    "dbscripts_on_quest_start",
    "dbscripts_on_spell",
    "dbscripts_on_go_use",
    "dbscripts_on_go_template_use",
    "dbscripts_on_event",
    "dbscripts_on_gossip",
    "dbscripts_on_creature_death",
    "dbscripts_on_creature_movement",
    "dbscripts_on_relay",
    "internal_use",
};

ScriptMgr::ScriptMgr()
{
}

// /////////////////////////////////////////////////////////
//              DB SCRIPTS (loaders of static data)
// /////////////////////////////////////////////////////////
// returns priority (0 == cannot start script)
uint8 GetSpellStartDBScriptPriority(SpellEntry const* spellinfo, SpellEffectIndex effIdx)
{
    if (spellinfo->Effect[effIdx] == SPELL_EFFECT_SCRIPT_EFFECT)
        return 10;

    if (spellinfo->Effect[effIdx] == SPELL_EFFECT_DUMMY)
        return 9;

    // NonExisting triggered spells can also start DB-Spell-Scripts
    if (spellinfo->Effect[effIdx] == SPELL_EFFECT_TRIGGER_SPELL && !sSpellTemplate.LookupEntry<SpellEntry>(spellinfo->EffectTriggerSpell[effIdx]))
        return 5;

    // NonExisting trigger missile spells can also start DB-Spell-Scripts
    if (spellinfo->Effect[effIdx] == SPELL_EFFECT_TRIGGER_MISSILE && !sSpellTemplate.LookupEntry<SpellEntry>(spellinfo->EffectTriggerSpell[effIdx]))
        return 4;

    // Can not start script
    return 0;
}

// Priorize: SCRIPT_EFFECT before DUMMY before Non-Existing triggered spell, for same priority the first effect with the priority triggers
bool ScriptMgr::CanSpellEffectStartDBScript(SpellEntry const* spellinfo, SpellEffectIndex effIdx)
{
    uint8 priority = GetSpellStartDBScriptPriority(spellinfo, effIdx);
    if (!priority)
        return false;

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint8 currentPriority = GetSpellStartDBScriptPriority(spellinfo, SpellEffectIndex(i));
        if (currentPriority < priority)                     // lower priority, continue checking
            continue;
        if (currentPriority > priority)                     // take other index with higher priority
            return false;
        if (i < effIdx)                                     // same priority at lower index
            return false;
    }

    return true;
}

void ScriptMgr::LoadScripts(ScriptMapType scriptType)
{
    ScriptMapMapName scripts;

    const char* tablename = scriptTableNames[scriptType].data();

    scripts.first = tablename;
    scripts.second.clear();                                 // need for reload support

    //                                              0   1      2        3         4          5          6            7              8           9        10        11        12               13 14 15 16 17     18            19
    auto queryResult = WorldDatabase.PQuery("SELECT id, delay, command, datalong, datalong2, datalong3, buddy_entry, search_radius, data_flags, dataint, dataint2, dataint3, dataint4, datafloat, x, y, z, o, speed, condition_id FROM %s ORDER BY priority", tablename);

    uint32 count = 0;

    if (!queryResult)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u script definitions from table %s", count, tablename);
        sLog.outString();
        return;
    }

    BarGoLink bar(queryResult->GetRowCount());

    do
    {
        bar.step();

        Field* fields = queryResult->Fetch();

        ScriptInfo tmp;
        tmp.id                 = fields[0].GetUInt32();
        tmp.delay              = fields[1].GetUInt32();
        tmp.command            = fields[2].GetUInt32();
        tmp.raw.data[0]        = fields[3].GetUInt32();
        tmp.raw.data[1]        = fields[4].GetUInt32();
        tmp.raw.data[2]        = fields[5].GetUInt32();
        tmp.buddyEntry         = fields[6].GetUInt32();
        tmp.searchRadiusOrGuid = fields[7].GetUInt32();
        tmp.data_flags         = fields[8].GetUInt32();
        tmp.textId[0]          = fields[9].GetInt32();
        tmp.textId[1]          = fields[10].GetInt32();
        tmp.textId[2]          = fields[11].GetInt32();
        tmp.textId[3]          = fields[12].GetInt32();
        tmp.rawFloat.data[0]   = fields[13].GetFloat();
        tmp.x                  = fields[14].GetFloat();
        tmp.y                  = fields[15].GetFloat();
        tmp.z                  = fields[16].GetFloat();
        tmp.o                  = fields[17].GetFloat();
        tmp.speed              = fields[18].GetFloat();
        tmp.condition_id       = fields[19].GetUInt32();

        if (tmp.condition_id && !sConditionStorage.LookupEntry<ConditionEntry>(tmp.condition_id))
        {
            sLog.outErrorDb("Table `%s` has condition_id = %u in command %u for script id %u, but this condition does not exist, skipping.", tablename, tmp.condition_id, tmp.command, tmp.id);
            continue;
        }

        // generic command args check
        if (tmp.buddyEntry && !(tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_GUID) && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP) == 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_STRING_ID) == 0)
        {
            if (tmp.IsCreatureBuddy() && !ObjectMgr::GetCreatureTemplate(tmp.buddyEntry))
            {
                sLog.outErrorDb("Table `%s` has buddyEntry = %u in command %u for script id %u, but this creature_template does not exist, skipping.", tablename, tmp.buddyEntry, tmp.command, tmp.id);
                continue;
            }
            if (!tmp.IsCreatureBuddy() && !ObjectMgr::GetGameObjectInfo(tmp.buddyEntry))
            {
                sLog.outErrorDb("Table `%s` has buddyEntry = %u in command %u for script id %u, but this gameobject_template does not exist, skipping.", tablename, tmp.buddyEntry, tmp.command, tmp.id);
                continue;
            }
            if (!tmp.searchRadiusOrGuid)
            {
                sLog.outErrorDb("Table `%s` has searchRadius = 0 in command %u for script id %u for buddy %u, skipping.", tablename, tmp.command, tmp.id, tmp.buddyEntry);
                continue;
            }
        }

        if (tmp.data_flags)                                 // Check flags
        {
            if (tmp.data_flags & ~MAX_SCRIPT_FLAG_VALID)
            {
                sLog.outErrorDb("Table `%s` has invalid data_flags %u in command %u for script id %u, skipping.", tablename, tmp.data_flags, tmp.command, tmp.id);
                continue;
            }
            if (!tmp.HasAdditionalScriptFlag() && tmp.data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
            {
                sLog.outErrorDb("Table `%s` has invalid data_flags %u in command %u for script id %u, skipping.", tablename, tmp.data_flags, tmp.command, tmp.id);
                continue;
            }
            if ((tmp.data_flags & SCRIPT_FLAG_BUDDY_AS_TARGET) != 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_POOL) == 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP) == 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_STRING_ID) == 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_GUID) == 0 && (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_GO) == 0 && !tmp.buddyEntry)
            {
                sLog.outErrorDb("Table `%s` has buddy required in data_flags %u in command %u for script id %u, but no buddy defined, skipping.", tablename, tmp.data_flags, tmp.command, tmp.id);
                continue;
            }
            if (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_GUID) // Check guid
            {
                if (tmp.IsCreatureBuddy())
                {
                    CreatureData const* data = sObjectMgr.GetCreatureData(tmp.searchRadiusOrGuid);
                    if (!data)
                    {
                        sLog.outErrorDb("Table `%s` for script id %u has buddy defined by guid (SCRIPT_FLAG_BUDDY_BY_GUID %u set) but no npc spawned with guid %u, skipping.", tablename, tmp.id, SCRIPT_FLAG_BUDDY_BY_GUID,  tmp.searchRadiusOrGuid);
                        continue;
                    }
                }
                else
                {
                    GameObjectData const* data = sObjectMgr.GetGOData(tmp.searchRadiusOrGuid);
                    if (!data)
                    {
                        sLog.outErrorDb("Table `%s` for script id %u has go-buddy defined by guid (SCRIPT_FLAG_BUDDY_BY_GUID %u set) but no go spawned with guid %u, skipping.", tablename, tmp.id, SCRIPT_FLAG_BUDDY_BY_GUID,  tmp.searchRadiusOrGuid);
                        continue;
                    }
                }
            }
            else if (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_POOL)
            {
                if (tmp.IsCreatureBuddy())
                {
                    auto pool = sPoolMgr.GetPoolCreatures(tmp.searchRadiusOrGuid);
                    if (pool.isEmpty())
                    {
                        sLog.outErrorDb("Table `%s` for script id %u has buddy defined by pool (SCRIPT_FLAG_BUDDY_BY_POOL %u set) but pool %u is empty, skipping.", tablename, tmp.id, tmp.data_flags, tmp.searchRadiusOrGuid);
                        continue;
                    }
                }
                else
                {
                    auto pool = sPoolMgr.GetPoolGameObjects(tmp.searchRadiusOrGuid);
                    if (pool.isEmpty())
                    {
                        sLog.outErrorDb("Table `%s` for script id %u has go-buddy defined by pool (SCRIPT_FLAG_BUDDY_BY_POOL %u set) but pool %u is empty, skipping.", tablename, tmp.id, tmp.data_flags, tmp.searchRadiusOrGuid);
                        continue;
                    }
                }
            }
            else if (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP)
            {
                uint32 groupEntry = tmp.buddyEntry;
                if (sObjectMgr.GetSpawnGroupContainer()->spawnGroupMap.find(groupEntry) == sObjectMgr.GetSpawnGroupContainer()->spawnGroupMap.end())
                {
                    sLog.outErrorDb("Table `%s` for script id %u has buddy defined by group (SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP %u set) but group %u is empty, skipping.", tablename, tmp.id, tmp.data_flags, tmp.searchRadiusOrGuid);
                    continue;
                }
            }
            else if (tmp.data_flags & SCRIPT_FLAG_BUDDY_BY_STRING_ID)
            {
                uint32 stringId = tmp.buddyEntry;
                if (stringId && !sScriptMgr.ExistsStringId(stringId))
                {
                    sLog.outErrorDb("Table `%s` for script id %u has buddy defined by stringId (SCRIPT_FLAG_BUDDY_BY_STRING_ID) but stringId %u does not exist, skipping.", tablename, tmp.id, stringId);
                    continue;
                }
            }
        }

        switch (tmp.command)
        {
            case SCRIPT_COMMAND_TALK:                       // 0
            {
                if (!tmp.talk.stringTemplateId)             // template checked later after loading strings
                {
                    if (tmp.textId[0] == 0)
                    {
                        sLog.outErrorDb("Table `%s` has invalid talk text id (dataint = %i) in SCRIPT_COMMAND_TALK for script id %u, skipping", tablename, tmp.textId[0], tmp.id);
                        continue;
                    }
                }
                break;
            }
            case SCRIPT_COMMAND_EMOTE:                      // 1
            {
                if (!sEmotesStore.LookupEntry(tmp.emote.emoteId))
                {
                    sLog.outErrorDb("Table `%s` has invalid emote id (datalong = %u) in SCRIPT_COMMAND_EMOTE for script id %u, skipping", tablename, tmp.emote.emoteId, tmp.id);
                    continue;
                }
                for (int i = 0; i < MAX_TEXT_ID; ++i)
                {
                    if (tmp.textId[i] && !sEmotesStore.LookupEntry(tmp.textId[i]))
                    {
                        sLog.outErrorDb("Table `%s` has invalid emote id (text_id%u = %u) in SCRIPT_COMMAND_EMOTE for script id %u", tablename, i + 1, tmp.textId[i], tmp.id);
                    }
                }
                break;
            }
            case SCRIPT_COMMAND_FIELD_SET:                  // 2
            case SCRIPT_COMMAND_MOVE_TO:                    // 3
            case SCRIPT_COMMAND_FLAG_SET:                   // 4
            case SCRIPT_COMMAND_FLAG_REMOVE:                // 5
                break;
            case SCRIPT_COMMAND_TELEPORT_TO:                // 6
            {
                if (!sMapStore.LookupEntry(tmp.teleportTo.mapId))
                {
                    sLog.outErrorDb("Table `%s` has invalid map (Id: %u) in SCRIPT_COMMAND_TELEPORT_TO for script id %u, skipping", tablename, tmp.teleportTo.mapId, tmp.id);
                    continue;
                }

                if (!MaNGOS::IsValidMapCoord(tmp.x, tmp.y, tmp.z, tmp.o))
                {
                    sLog.outErrorDb("Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TELEPORT_TO for script id %u, skipping", tablename, tmp.x, tmp.y, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_QUEST_EXPLORED:             // 7
            {
                Quest const* quest = sObjectMgr.GetQuestTemplate(tmp.questExplored.questId);
                if (!quest)
                {
                    sLog.outErrorDb("Table `%s` has invalid quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, skipping", tablename, tmp.questExplored.questId, tmp.id);
                    continue;
                }

                if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT))
                {
                    sLog.outErrorDb("Table `%s` has quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, but quest not have flag QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT in quest flags. Script command or quest flags wrong. Quest modified to require objective.", tablename, tmp.questExplored.questId, tmp.id);

                    // this will prevent quest completing without objective
                    const_cast<Quest*>(quest)->SetSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT);

                    // continue; - quest objective requirement set and command can be allowed
                }

                if (float(tmp.questExplored.distance) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, skipping",
                                    tablename, tmp.questExplored.distance, tmp.id);
                    continue;
                }

                if (tmp.questExplored.distance && float(tmp.questExplored.distance) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, max distance is %f or 0 for disable distance check, skipping",
                                    tablename, tmp.questExplored.distance, tmp.id, DEFAULT_VISIBILITY_DISTANCE);
                    continue;
                }

                if (tmp.questExplored.distance && float(tmp.questExplored.distance) < INTERACTION_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too small distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, min distance is %f or 0 for disable distance check, skipping",
                                    tablename, tmp.questExplored.distance, tmp.id, INTERACTION_DISTANCE);
                    continue;
                }

                break;
            }
            case SCRIPT_COMMAND_KILL_CREDIT:                // 8
            {
                if (tmp.killCredit.creatureEntry && !ObjectMgr::GetCreatureTemplate(tmp.killCredit.creatureEntry))
                {
                    sLog.outErrorDb("Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_KILL_CREDIT for script id %u, skipping", tablename, tmp.killCredit.creatureEntry, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:         // 9
            {
                uint32 goEntry;
                if (!tmp.GetGOGuid())
                {
                    if (!tmp.buddyEntry)
                    {
                        sLog.outErrorDb("Table `%s` has no gameobject nor buddy defined in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u, skipping", tablename, tmp.id);
                        continue;
                    }
                    goEntry = tmp.buddyEntry;
                }
                else
                {
                    GameObjectData const* data = sObjectMgr.GetGOData(tmp.GetGOGuid());
                    if (!data)
                    {
                        sLog.outErrorDb("Table `%s` has invalid gameobject (GUID: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u, skipping", tablename, tmp.GetGOGuid(), tmp.id);
                        continue;
                    }
                    goEntry = data->id;
                }

                GameObjectInfo const* info = ObjectMgr::GetGameObjectInfo(goEntry);
                if (!info)
                {
                    sLog.outErrorDb("Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u, skipping", tablename, tmp.GetGOGuid(), goEntry, tmp.id);
                    continue;
                }

                if (info->type == GAMEOBJECT_TYPE_FISHINGNODE ||
                        info->type == GAMEOBJECT_TYPE_FISHINGHOLE ||
                        info->type == GAMEOBJECT_TYPE_DOOR)
                {
                    sLog.outErrorDb("Table `%s` have gameobject type (%u) unsupported by command SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u, skipping", tablename, info->type, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_TEMP_SPAWN_CREATURE:        // 10
            {
                if (!MaNGOS::IsValidMapCoord(tmp.x, tmp.y, tmp.z, tmp.o))
                {
                    sLog.outErrorDb("Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TEMP_SPAWN_CREATURE for script id %u, skipping", tablename, tmp.x, tmp.y, tmp.id);
                    continue;
                }

                if (!ObjectMgr::GetCreatureTemplate(tmp.summonCreature.creatureEntry))
                {
                    sLog.outErrorDb("Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_TEMP_SPAWN_CREATURE for script id %u, skipping", tablename, tmp.summonCreature.creatureEntry, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:                  // 11
            case SCRIPT_COMMAND_CLOSE_DOOR:                 // 12
            {
                uint32 goEntry;
                if (!tmp.GetGOGuid())
                {
                    if (!tmp.buddyEntry)
                    {
                        sLog.outErrorDb("Table `%s` has no gameobject nor buddy defined in %s for script id %u, skipping", tablename, (tmp.command == SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"), tmp.id);
                        continue;
                    }
                    goEntry = tmp.buddyEntry;
                }
                else
                {
                    GameObjectData const* data = sObjectMgr.GetGOData(tmp.GetGOGuid());
                    if (!data)
                    {
                        sLog.outErrorDb("Table `%s` has invalid gameobject (GUID: %u) in %s for script id %u, skipping", tablename, tmp.GetGOGuid(), (tmp.command == SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"), tmp.id);
                        continue;
                    }
                    goEntry = data->id;
                }

                GameObjectInfo const* info = ObjectMgr::GetGameObjectInfo(goEntry);
                if (!info)
                {
                    sLog.outErrorDb("Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in %s for script id %u, skipping", tablename, tmp.GetGOGuid(), goEntry, (tmp.command == SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"), tmp.id);
                    continue;
                }

                if (info->type != GAMEOBJECT_TYPE_DOOR && info->type != GAMEOBJECT_TYPE_BUTTON)
                {
                    sLog.outErrorDb("Table `%s` has gameobject type (%u) non supported by command %s for script id %u, skipping", tablename, info->id, (tmp.command == SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"), tmp.id);
                    continue;
                }

                break;
            }
            case SCRIPT_COMMAND_ACTIVATE_OBJECT:            // 13
                break;
            case SCRIPT_COMMAND_REMOVE_AURA:                // 14
            {
                if (!sSpellTemplate.LookupEntry<SpellEntry>(tmp.removeAura.spellId))
                {
                    sLog.outErrorDb("Table `%s` using nonexistent spell (id: %u) in SCRIPT_COMMAND_REMOVE_AURA or SCRIPT_COMMAND_CAST_SPELL for script id %u, skipping",
                                    tablename, tmp.removeAura.spellId, tmp.id);
                    continue;
                }
                if (tmp.removeAura.defaultOrChargeOrStack > 2)
                {
                    sLog.outErrorDb("Table `%s` using invalid defaultOrChargeOrStack (id: %u) in SCRIPT_COMMAND_REMOVE_AURA for script id %u, skipping",
                        tablename, tmp.removeAura.spellId, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_CAST_SPELL:                 // 15
            {
                if (!sSpellTemplate.LookupEntry<SpellEntry>(tmp.castSpell.spellId))
                {
                    sLog.outErrorDb("Table `%s` using nonexistent spell (id: %u) in SCRIPT_COMMAND_REMOVE_AURA or SCRIPT_COMMAND_CAST_SPELL for script id %u, skipping",
                                    tablename, tmp.castSpell.spellId, tmp.id);
                    continue;
                }
                bool hasErrored = false;
                for (uint8 i = 0; i < MAX_TEXT_ID; ++i)
                {
                    if (tmp.textId[i] && !sSpellTemplate.LookupEntry<SpellEntry>(uint32(tmp.textId[i])))
                    {
                        sLog.outErrorDb("Table `%s` using nonexistent spell (id: %u) in SCRIPT_COMMAND_CAST_SPELL for script id %u, dataint%u, skipping",
                                        tablename, uint32(tmp.textId[i]), tmp.id, i + 1);
                        hasErrored = true;
                    }
                }
                if (hasErrored)
                    continue;
                break;
            }
            case SCRIPT_COMMAND_PLAY_SOUND:                 // 16
            {
                if (!sSoundEntriesStore.LookupEntry(tmp.playSound.soundId))
                {
                    sLog.outErrorDb("Table `%s` using nonexistent sound (id: %u) in SCRIPT_COMMAND_PLAY_SOUND for script id %u, skipping",
                                    tablename, tmp.playSound.soundId, tmp.id);
                    continue;
                }
                // bitmask: 0/1=target-player, 0/2=with distance dependent, 0/4=map wide, 0/8=zone wide, 0/16=area wide
                if (tmp.playSound.flags & ~(1 | 2 | 4 | 8 | 16))
                    sLog.outErrorDb("Table `%s` using unsupported sound flags (datalong2: %u) in SCRIPT_COMMAND_PLAY_SOUND for script id %u, unsupported flags will be ignored", tablename, tmp.playSound.flags, tmp.id);
                if ((tmp.playSound.flags & (1 | 2)) > 0 && (tmp.playSound.flags & (4 | 8 | 16)) > 0)
                    sLog.outErrorDb("Table `%s` uses sound flags (datalong2: %u) in SCRIPT_COMMAND_PLAY_SOUND for script id %u, combining (1|2) with (4|8|16) makes no sense", tablename, tmp.playSound.flags, tmp.id);
                break;
            }
            case SCRIPT_COMMAND_CREATE_ITEM:                // 17
            {
                if (!ObjectMgr::GetItemPrototype(tmp.createItem.itemEntry))
                {
                    sLog.outErrorDb("Table `%s` has nonexistent item (entry: %u) in SCRIPT_COMMAND_CREATE_ITEM for script id %u, skipping",
                                    tablename, tmp.createItem.itemEntry, tmp.id);
                    continue;
                }
                if (!tmp.createItem.amount)
                {
                    sLog.outErrorDb("Table `%s` SCRIPT_COMMAND_CREATE_ITEM but amount is %u for script id %u, skipping",
                                    tablename, tmp.createItem.amount, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_DESPAWN_SELF:               // 18
            {
                // for later, we might consider despawn by database guid, and define in datalong2 as option to despawn self.
                break;
            }
            case SCRIPT_COMMAND_PLAY_MOVIE:                 // 19
            {
                sLog.outErrorDb("Table `%s` use unsupported SCRIPT_COMMAND_PLAY_MOVIE for script id %u, skipping",
                                tablename, tmp.id);
                continue;
            }
            case SCRIPT_COMMAND_MOVEMENT:                   // 20
            {
                if (tmp.movement.movementType >= MAX_DB_MOTION_TYPE && tmp.movement.movementType != EFFECT_MOTION_TYPE && tmp.movement.movementType != FALL_MOTION_TYPE)
                {
                    sLog.outErrorDb("Table `%s` SCRIPT_COMMAND_MOVEMENT has invalid MovementType %u for script id %u, skipping",
                                    tablename, tmp.movement.movementType, tmp.id);
                    continue;
                }

                if (tmp.textId[0] < 0 || tmp.textId[0] > ForcedMovement::FORCED_MOVEMENT_FLIGHT)
                {
                    sLog.outErrorDb("Table `%s` SCRIPT_COMMAND_MOVEMENT has invalid forced movement type %d for script id %u, skipping",
                        tablename, tmp.textId[0], tmp.id);
                    continue;
                }

                break;
            }
            case SCRIPT_COMMAND_SET_ACTIVEOBJECT:           // 21
                break;
            case SCRIPT_COMMAND_SET_FACTION:                // 22
            {
                if (tmp.faction.factionId && !sFactionTemplateStore.LookupEntry(tmp.faction.factionId))
                {
                    sLog.outErrorDb("Table `%s` has datalong = %u in SCRIPT_COMMAND_SET_FACTION for script id %u, but this faction-template does not exist, skipping", tablename, tmp.faction.factionId, tmp.id);
                    continue;
                }

                break;
            }
            case SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL:    // 23
            {
                if (tmp.data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                {
                    if (tmp.morph.creatureOrModelEntry && !sCreatureDisplayInfoStore.LookupEntry(tmp.morph.creatureOrModelEntry))
                    {
                        sLog.outErrorDb("Table `%s` has datalong2 = %u in SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL for script id %u, but this model does not exist, skipping", tablename, tmp.morph.creatureOrModelEntry, tmp.id);
                        continue;
                    }
                }
                else
                {
                    if (tmp.morph.creatureOrModelEntry && !ObjectMgr::GetCreatureTemplate(tmp.morph.creatureOrModelEntry))
                    {
                        sLog.outErrorDb("Table `%s` has datalong2 = %u in SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL for script id %u, but this creature_template does not exist, skipping", tablename, tmp.morph.creatureOrModelEntry, tmp.id);
                        continue;
                    }
                }

                break;
            }
            case SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL:    // 24
            {
                if (tmp.data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                {
                    if (tmp.mount.creatureOrModelEntry && !sCreatureDisplayInfoStore.LookupEntry(tmp.mount.creatureOrModelEntry))
                    {
                        sLog.outErrorDb("Table `%s` has datalong2 = %u in SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL for script id %u, but this model does not exist, skipping", tablename, tmp.mount.creatureOrModelEntry, tmp.id);
                        continue;
                    }
                }
                else
                {
                    if (tmp.mount.creatureOrModelEntry && !ObjectMgr::GetCreatureTemplate(tmp.mount.creatureOrModelEntry))
                    {
                        sLog.outErrorDb("Table `%s` has datalong2 = %u in SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL for script id %u, but this creature_template does not exist, skipping", tablename, tmp.mount.creatureOrModelEntry, tmp.id);
                        continue;
                    }
                }

                break;
            }
            case SCRIPT_COMMAND_SET_RUN:                    // 25
            case SCRIPT_COMMAND_ATTACK_START:               // 26
                break;
            case SCRIPT_COMMAND_GO_LOCK_STATE:              // 27
            {
                if (// lock(0x01) and unlock(0x02) together
                    ((tmp.goLockState.lockState & 0x01) && (tmp.goLockState.lockState & 0x02)) ||
                    // non-interact (0x4) and interact (0x08) together
                    ((tmp.goLockState.lockState & 0x04) && (tmp.goLockState.lockState & 0x08)) ||
                    // no setting
                    !tmp.goLockState.lockState ||
                    // invalid number
                    tmp.goLockState.lockState >= 0x10)
                {
                    sLog.outErrorDb("Table `%s` has invalid lock state (datalong = %u) in SCRIPT_COMMAND_GO_LOCK_STATE for script id %u, skipping", tablename, tmp.goLockState.lockState, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_STAND_STATE:                // 28
            {
                if (tmp.standState.stand_state >= MAX_UNIT_STAND_STATE)
                {
                    sLog.outErrorDb("Table `%s` has invalid stand state (datalong = %u) in SCRIPT_COMMAND_STAND_STATE for script id %u, skipping", tablename, tmp.standState.stand_state, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_MODIFY_NPC_FLAGS:           // 29
            {
                if (tmp.npcFlag.change_flag > 2)
                    sLog.outErrorDb("Table `%s` has invalid change flag (datalong2 = %u) in SCRIPT_COMMAND_MODIFY_NPC_FLAGS for script id %u, skipping", tablename, tmp.npcFlag.change_flag, tmp.id);
                break;
            }
            case SCRIPT_COMMAND_SEND_TAXI_PATH:             // 30
            {
                if (!sTaxiPathStore.LookupEntry(tmp.sendTaxiPath.taxiPathId))
                {
                    sLog.outErrorDb("Table `%s` has datalong = %u in SCRIPT_COMMAND_SEND_TAXI_PATH for script id %u, but this taxi path does not exist, skipping", tablename, tmp.sendTaxiPath.taxiPathId, tmp.id);
                    continue;
                }
                // Check if this taxi path can be triggered with a spell
                if (!sLog.HasLogFilter(LOG_FILTER_DB_STRICTED_CHECK))
                {
                    uint32 taxiSpell = 0;
                    for (uint32 i = 1; i < sSpellTemplate.GetMaxEntry() && taxiSpell == 0; ++i)
                    {
                        if (SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(i))
                            for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
                            {
                                if (spell->Effect[j] == SPELL_EFFECT_SEND_TAXI && spell->EffectMiscValue[j] == int32(tmp.sendTaxiPath.taxiPathId))
                                {
                                    taxiSpell = i;
                                    break;
                                }
                            }
                    }

                    if (taxiSpell)
                    {
                        sLog.outErrorDb("Table `%s` has datalong = %u in SCRIPT_COMMAND_SEND_TAXI_PATH for script id %u, but this taxi path can be triggered by spell %u, skipping", tablename, tmp.sendTaxiPath.taxiPathId, tmp.id, taxiSpell);
                        continue;
                    }
                }
                break;
            }
            case SCRIPT_COMMAND_TERMINATE_SCRIPT:           // 31
            {
                if (tmp.IsCreatureBuddy())
                {
                    if (tmp.terminateScript.npcOrGOEntry && !ObjectMgr::GetCreatureTemplate(tmp.terminateScript.npcOrGOEntry))
                    {
                        sLog.outErrorDb("Table `%s` has npc entry = '%u' in SCRIPT_COMMAND_TERMINATE_SCRIPT for script id %u, but this npc entry does not exist, skipping", tablename, tmp.terminateScript.npcOrGOEntry, tmp.id);
                        continue;
                    }
                }
                else
                {
                    if (tmp.terminateScript.npcOrGOEntry && !ObjectMgr::GetGameObjectInfo(tmp.terminateScript.npcOrGOEntry))
                    {
                        sLog.outErrorDb("Table `%s` has GO entry = '%u' in SCRIPT_COMMAND_TERMINATE_SCRIPT for script id %u, but this GO entry does not exist, skipping", tablename, tmp.terminateScript.npcOrGOEntry, tmp.id);
                        continue;
                    }
                }
                if (tmp.terminateScript.poolId && tmp.terminateScript.poolId > sPoolMgr.GetMaxPoolId())
                {
                    sLog.outErrorDb("Table `%s` has pool id = '%u' in SCRIPT_COMMAND_TERMINATE_SCRIPT for script id %u, but this pool id does not exist, skipping", tablename, tmp.terminateScript.poolId, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_PAUSE_WAYPOINTS:            // 32
                break;
            case SCRIPT_COMMAND_RESERVED_1:                 // 33
                break;
            case SCRIPT_COMMAND_TERMINATE_COND:             // 34
            {
                if (!sConditionStorage.LookupEntry<ConditionEntry>(tmp.terminateCond.conditionId))
                {
                    sLog.outErrorDb("Table `%s` has datalong = %u in SCRIPT_COMMAND_TERMINATE_COND for script id %u, but this condition_id does not exist, skipping", tablename, tmp.terminateCond.conditionId, tmp.id);
                    continue;
                }
                if (tmp.terminateCond.failQuest && !sObjectMgr.GetQuestTemplate(tmp.terminateCond.failQuest))
                {
                    sLog.outErrorDb("Table `%s` has datalong2 = %u in SCRIPT_COMMAND_TERMINATE_COND for script id %u, but this questId does not exist, skipping", tablename, tmp.terminateCond.failQuest, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_SEND_AI_EVENT:              // 35
                break;
            case SCRIPT_COMMAND_SET_FACING:                 // 36
                break;
            case SCRIPT_COMMAND_MOVE_DYNAMIC:               // 37
            {
                if (tmp.moveDynamic.maxDist < tmp.moveDynamic.minDist)
                {
                    sLog.outErrorDb("Table `%s` has invalid min-dist (datalong2 = %u) less than max-dist (datalong = %u) in SCRIPT_COMMAND_MOVE_DYNAMIC for script id %u, skipping", tablename, tmp.moveDynamic.minDist, tmp.moveDynamic.maxDist, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_SEND_MAIL:                  // 38
            {
                if (!sMailTemplateStore.LookupEntry(tmp.sendMail.mailTemplateId))
                {
                    sLog.outErrorDb("Table `%s` has invalid mailTemplateId (datalong = %u) in SCRIPT_COMMAND_SEND_MAIL for script id %u, skipping", tablename, tmp.sendMail.mailTemplateId, tmp.id);
                    continue;
                }
                if (tmp.sendMail.altSender && !ObjectMgr::GetCreatureTemplate(tmp.sendMail.altSender))
                {
                    sLog.outErrorDb("Table `%s` has invalid alternativeSender (datalong2 = %u) in SCRIPT_COMMAND_SEND_MAIL for script id %u, skipping", tablename, tmp.sendMail.altSender, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_SET_HOVER:                  // 39
            case SCRIPT_COMMAND_DESPAWN_GO:                 // 40
            case SCRIPT_COMMAND_RESPAWN:                    // 41
                break;
            case SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS:        // 42
            {
                if (tmp.textId[0] < 0 || tmp.textId[1] < 0 || tmp.textId[2] < 0)
                {
                    sLog.outErrorDb("Table `%s` has invalid equipment slot (dataint = %u, dataint2 = %u, dataint3 = %u) in SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS for script id %u, skipping", tablename, tmp.textId[0], tmp.textId[1], tmp.textId[2], tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_RESET_GO:                   // 43
                break;
            case SCRIPT_COMMAND_UPDATE_TEMPLATE:            // 44
            {
                if (!sCreatureStorage.LookupEntry<CreatureInfo>(tmp.updateTemplate.newTemplate))
                {
                    sLog.outErrorDb("Table `%s` uses nonexistent creature entry %u in SCRIPT_COMMAND_UPDATE_TEMPLATE for script id %u, skipping", tablename, tmp.updateTemplate.newTemplate, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_START_RELAY_SCRIPT:         // 45
            {
                if (scriptType != SCRIPT_TYPE_RELAY) // Relay scripts are done first and checked after load
                {
                    if (tmp.relayScript.relayId)
                    {
                        auto relayScript = GetScriptMap(SCRIPT_TYPE_RELAY);
                        if (relayScript->second.find(tmp.relayScript.relayId) == relayScript->second.end())
                        {
                            sLog.outErrorDb("Table `dbscripts_on_relay` uses nonexistent relay ID %u in SCRIPT_COMMAND_START_RELAY_SCRIPT for script id %u, skipping", tmp.relayScript.relayId, tmp.id);
                            continue;
                        }
                    }
                }
                if (tmp.relayScript.templateId)
                {
                    if (!sScriptMgr.CheckScriptRelayTemplateId(tmp.relayScript.templateId))
                    {
                        sLog.outErrorDb("Table `dbscripts_on_relay` uses nonexistent dbscript_random_template ID %u in SCRIPT_COMMAND_START_RELAY_SCRIPT for script id %u, skipping", tmp.relayScript.relayId, tmp.id);
                        continue;
                    }
                }
                break;
            }
            case SCRIPT_COMMAND_CAST_CUSTOM_SPELL:          // 46
            {
                if (!sSpellTemplate.LookupEntry<SpellEntry>(tmp.castSpell.spellId))
                {
                    sLog.outErrorDb("Table `%s` using nonexistent spell (id: %u) in SCRIPT_COMMAND_CAST_CUSTOM_SPELL for script id %u, skipping",
                        tablename, tmp.castSpell.spellId, tmp.id);
                    continue;
                }
                if (tmp.textId[0] == 0 && tmp.textId[1] == 0 && tmp.textId[2] == 0)
                {
                    sLog.outErrorDb("Table `%s` has invalid BP values (dataint = %u, dataint2 = %u, dataint3 = %u) in SCRIPT_COMMAND_CAST_CUSTOM_SPELL for script id %u. At least one field has to be populated, skipping", tablename, tmp.textId[0], tmp.textId[1], tmp.textId[2], tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_INTERRUPT_SPELL:            // 47
            {
                if (tmp.interruptSpell.currentSpellType >= CURRENT_MAX_SPELL)
                {
                    sLog.outErrorDb("Table `%s` uses invalid current spell type %u (must be smaller or equal to %u) for script id %u, skipping", tablename, tmp.interruptSpell.currentSpellType, CURRENT_MAX_SPELL - 1, tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_MODIFY_UNIT_FLAGS:          // 48
            {
                if (tmp.unitFlag.change_flag > 2)
                    sLog.outErrorDb("Table `%s` has invalid change flag (datalong2 = %u) in SCRIPT_COMMAND_MODIFY_UNIT_FLAGS for script id %u, skipping", tablename, tmp.unitFlag.change_flag, tmp.id);
                break;
            }
            case SCRIPT_COMMAND_SET_DATA_64:                // 49
                break;
            case SCRIPT_COMMAND_ZONE_PULSE:                 // 50
                break;

            case SCRIPT_COMMAND_SPAWN_GROUP:                // 51
            {
                switch (tmp.formationData.command)
                {
                     case 150: // SetFormation
                     {
                         if (tmp.textId[0] >= SpawnGroupFormationType::SPAWN_GROUP_FORMATION_TYPE_COUNT)
                         {
                             sLog.outErrorDb("Table `%s` uses invalid formation shape id(%u) for script id %u. Command[51], subcommand[%u], skipping",
                                 tablename, tmp.textId[0], tmp.id, tmp.formationData.command);
                             continue;
                         }
                         break;
                     }
                     case 151: // Remove formation
                     {
                         auto const& spgCont = sObjectMgr.GetSpawnGroupContainer()->spawnGroupMap;
                         if (spgCont.find(tmp.formationData.data1) == spgCont.end())
                         {
                             sLog.outErrorDb("Table `%s` uses invalid spawngroup id(%u) for script id %u. Command[51], subcommand[%u], skipping",
                                 tablename, tmp.formationData.data1, tmp.id, tmp.formationData.command);
                             continue;
                         }

                         break;
                     }
//                     case 3: // Add buddy to formation
//                     {
//                         if (!tmp.buddyEntry)
//                         {
//                             sLog.outErrorDb("Table `%s` has no buddy entry defined in SCRIPT_COMMAND_FORMATION for script id %u", tablename, tmp.id);
//                             continue;
//                         }
//                         break;
//                     }
// 
//                     case 4: // remove buddy to formation
//                     {
//                         if (!tmp.buddyEntry)
//                         {
//                             sLog.outErrorDb("Table `%s` has no buddy entry defined in SCRIPT_COMMAND_FORMATION for script id %u", tablename, tmp.id);
//                             continue;
//                         }
//                         break;
//                     }

                    case 100: // switch formation shape
                    {
                        if (tmp.formationData.data1 >= SpawnGroupFormationType::SPAWN_GROUP_FORMATION_TYPE_COUNT)
                        {
                            sLog.outErrorDb("Table `%s` uses invalid formation shape id(%u) for script id %u, skipping", tablename, tmp.formationData.data1, tmp.id);
                            continue;
                        }
                        break;
                    }

                    case 101: // change formation spread
                    {
                        if (tmp.x < 0.5f || tmp.x > 15.0f)
                        {
                            sLog.outErrorDb("Table `%s` uses invalid formation spread(%f) should be in 0.5 .. 15 range for script id %u, skipping", tablename, tmp.x, tmp.id);
                            continue;
                        }
                        break;
                    }

                    case 102: // change formation options
                    {
                        break;
                    }

                    default:
                        sLog.outErrorDb("Table `%s` unknown formation command %u, skipping", tablename, tmp.formationData.command);
                        continue;
                }

                break;
            }
            case SCRIPT_COMMAND_SET_GOSSIP_MENU:            // 52
            {
                // sScriptMgr.LoadGossipScripts() must be called first in order for this to work
                //if (!sObjectMgr.IsExistingGossipMenuId(tmp.setGossipMenu.gossipMenuId))
                //{
                //    sLog.outErrorDb("Table `%s` using nonexistent gossip menu (id: %u) in SCRIPT_COMMAND_SET_GOSSIP_MENU for script id %u",
                //        tablename, tmp.setGossipMenu.gossipMenuId, tmp.id);
                //    continue;
                //}
                break;
            }
            case SCRIPT_COMMAND_SET_WORLDSTATE:             // 53
            {
                if (!sObjectMgr.HasWorldStateName(tmp.textId[0])) // must be named
                {
                    sLog.outErrorDb("Table `%s` has no worldstate name assigned for worldstate %d, skipping", tablename, tmp.textId[0]);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_SET_SHEATHE:                // 54
            {
                if (tmp.setSheathe.sheatheState > SHEATH_STATE_RANGED)
                {
                    sLog.outErrorDb("Table `%s` has invalid sheathe state assigned %d, skipping", tablename, tmp.setSheathe.sheatheState);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_SET_STRING_ID:              // 55
            {
                if (!ExistsStringId(tmp.stringId.stringId))
                {
                    sLog.outErrorDb("Table `%s` has invalid string_id id assigned %d, skipping", tablename, tmp.stringId.stringId);
                    continue;
                }
                if (tmp.stringId.apply > 1)
                {
                    sLog.outErrorDb("Table `%s` has invalid apply (0 or 1) assigned %d, skipping", tablename, tmp.stringId.apply);
                    continue;
                }
                break;
            }
            default:
            {
                sLog.outErrorDb("Table `%s` unknown command %u, skipping", tablename, tmp.command);
                continue;
            }
            // ordered like this to prevent merge errors
            case SCRIPT_COMMAND_MEETINGSTONE:               // 200
            {
                if (!GetAreaEntryByAreaID(tmp.meetingstone.areaId))
                {
                    sLog.outErrorDb("Table `%s` has datalong = %u in  SCRIPT_COMMAND_MEETINGSTONE for script id %u, but there is no area with this Id.", tablename, tmp.meetingstone.areaId, tmp.id);
                    continue;
                }
                break;
            }
        }

        if (scripts.second.find(tmp.id) == scripts.second.end())
        {
            ScriptMap emptyMap;
            scripts.second[tmp.id] = emptyMap;
        }
        scripts.second[tmp.id].emplace(tmp.delay, std::make_shared<ScriptInfo>(tmp));

        ++count;
    }
    while (queryResult->NextRow());

    m_scriptMaps[scriptType] = std::make_shared<ScriptMapMapName>(scripts);

    sLog.outString(">> Loaded %u script definitions from table %s", count, tablename);
    sLog.outString();
}

void ScriptMgr::LoadScriptMap(ScriptMapType scriptType, bool reload)
{
    LoadScripts(scriptType);

    switch (scriptType)
    {
        case SCRIPT_TYPE_QUEST_END:
        {
            auto questEndScripts = GetScriptMap(scriptType);
            // check ids
            for (auto itr = questEndScripts->second.begin(); itr != questEndScripts->second.end(); ++itr)
            {
                if (!sObjectMgr.GetQuestTemplate(itr->first))
                    sLog.outErrorDb("Table `dbscripts_on_quest_end` has not existing quest (Id: %u) as script id", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_QUEST_START:
        {
            auto questStartScripts = GetScriptMap(scriptType);
            // check ids
            for (auto itr = questStartScripts->second.begin(); itr != questStartScripts->second.end(); ++itr)
            {
                if (!sObjectMgr.GetQuestTemplate(itr->first))
                    sLog.outErrorDb("Table `dbscripts_on_quest_start` has not existing quest (Id: %u) as script id", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_SPELL:
        {
            auto spellScripts = GetScriptMap(scriptType);
            // check ids
            for (auto itr = spellScripts->second.begin(); itr != spellScripts->second.end(); ++itr)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                if (!spellInfo)
                {
                    sLog.outErrorDb("Table `dbscripts_on_spell` has not existing spell (Id: %u) as script id", itr->first);
                    continue;
                }

                // check for correct spellEffect
                bool found = false;
                for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
                {
                    if (GetSpellStartDBScriptPriority(spellInfo, SpellEffectIndex(i)))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                    sLog.outErrorDb("Table `dbscripts_on_spell` has unsupported spell (Id: %u)", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_GAMEOBJECT:
        {
            auto goScripts = GetScriptMap(scriptType);
            // check ids
            for (auto itr = goScripts->second.begin(); itr != goScripts->second.end(); ++itr)
            {
                if (!sObjectMgr.GetGOData(itr->first))
                    sLog.outErrorDb("Table `dbscripts_on_go_use` has not existing gameobject (GUID: %u) as script id", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_GAMEOBJECT_TEMPLATE:
        {
            auto goTemplateScripts = GetScriptMap(scriptType);
            // check ids
            for (auto itr = goTemplateScripts->second.begin(); itr != goTemplateScripts->second.end(); ++itr)
            {
                if (!sObjectMgr.GetGameObjectInfo(itr->first))
                    sLog.outErrorDb("Table `dbscripts_on_go_template_use` has not existing gameobject (Entry: %u) as script id", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_EVENT:
        {
            std::set<uint32> eventIds; // Store possible event ids
            CollectPossibleEventIds(eventIds);

            auto eventScripts = GetScriptMap(scriptType);

            // Then check if all scripts are in above list of possible script entries
            for (auto itr = eventScripts->second.begin(); itr != eventScripts->second.end(); ++itr)
            {
                std::set<uint32>::const_iterator itr2 = eventIds.find(itr->first);
                if (itr2 == eventIds.end())
                    sLog.outErrorDb("Table `dbscripts_on_event` has script (Id: %u) not referring to any fitting gameobject_template or any spell effect %u or path taxi node data",
                        itr->first, SPELL_EFFECT_SEND_EVENT);
            }
            break;
        }
        case SCRIPT_TYPE_GOSSIP:
        {
            break;
        }
        case SCRIPT_TYPE_CREATURE_DEATH:
        {
            // check ids
            auto deathScripts = GetScriptMap(scriptType);
            for (auto itr = deathScripts->second.begin(); itr != deathScripts->second.end(); ++itr)
            {
                if (!sObjectMgr.GetCreatureTemplate(itr->first))
                    sLog.outErrorDb("Table `dbscripts_on_creature_death` has not existing creature (Entry: %u) as script id", itr->first);
            }
            break;
        }
        case SCRIPT_TYPE_CREATURE_MOVEMENT:
        {
            break;
        }
        case SCRIPT_TYPE_RELAY:
        {
            // check ids
            auto relayScripts = GetScriptMap(scriptType);
            for (auto itr = relayScripts->second.begin(); itr != relayScripts->second.end(); ++itr)
            {
                for (auto& data : itr->second) // need to check after load is complete, because of nesting
                {
                    if (data.second->command == SCRIPT_COMMAND_START_RELAY_SCRIPT)
                    {
                        bool hasErrored = false;
                        if (data.second->relayScript.relayId)
                        {
                            if (relayScripts->second.find(data.second->relayScript.relayId) == relayScripts->second.end())
                            {
                                sLog.outErrorDb("Table `dbscripts_on_relay` uses nonexistent relay ID %u in SCRIPT_COMMAND_START_RELAY_SCRIPT for script id %u.", data.second->relayScript.relayId, data.second->id);
                                hasErrored = true;
                            }
                        }

                        if (hasErrored)
                            continue;
                    }
                }
            }

            // String templates are checked on string loading
            CheckRandomRelayTemplates();
            break;
        }
        default: break;
    }

    if (reload)
    {
        auto scriptMap = GetScriptMap(scriptType);
        sMapMgr.DoForAllMaps([scriptType, scriptMap](Map* map)
        {
            map->GetMessager().AddMessage([scriptType, scriptMap](Map* map)
            {
                map->GetMapDataContainer().SetScriptMap(scriptType, scriptMap);
            });
        });
    }
}

void ScriptMgr::LoadStringIds(bool reload)
{
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT Id, Name FROM string_id"));
    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u script definitions from table string_id", count);
        sLog.outString();

        m_stringIds = std::make_shared<StringIdMap>();
        m_stringIdsByString = std::make_shared<StringIdMapByString>();
        return;
    }
    
    auto stringIdMap = std::make_shared<StringIdMap>();
    auto stringIdsByString = std::make_shared<StringIdMapByString>();

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();

        Field* fields = result->Fetch();

        StringId stringId;
        stringId.Id = fields[0].GetInt32();
        stringId.Name = fields[1].GetCppString();

        stringIdMap->emplace(stringId.Id, stringId);
        stringIdsByString->emplace(stringId.Name, stringId);
    }
    while(result->NextRow());        

    m_stringIds = stringIdMap;
    m_stringIdsByString = stringIdsByString;

    if (reload)
    {
        sMapMgr.DoForAllMaps([stringIdMap, stringIdsByString](Map* map)
        {
            map->GetMessager().AddMessage([stringIdMap, stringIdsByString](Map* map)
            {
                map->GetMapDataContainer().SetStringIdMaps(stringIdMap, stringIdsByString);
            });
        });
    }
}

void ScriptMgr::LoadDbScriptStrings()
{
    CheckScriptTexts(SCRIPT_TYPE_QUEST_END);
    CheckScriptTexts(SCRIPT_TYPE_QUEST_START);
    CheckScriptTexts(SCRIPT_TYPE_SPELL);
    CheckScriptTexts(SCRIPT_TYPE_GAMEOBJECT);
    CheckScriptTexts(SCRIPT_TYPE_GAMEOBJECT_TEMPLATE);
    CheckScriptTexts(SCRIPT_TYPE_EVENT);
    CheckScriptTexts(SCRIPT_TYPE_GOSSIP);
    CheckScriptTexts(SCRIPT_TYPE_CREATURE_DEATH);
    CheckScriptTexts(SCRIPT_TYPE_CREATURE_MOVEMENT);
    CheckScriptTexts(SCRIPT_TYPE_RELAY);
}

void ScriptMgr::LoadDbScriptRandomTemplates()
{
    auto queryResult = WorldDatabase.Query("SELECT id, type, target_id, chance FROM dbscript_random_templates");

    if (queryResult)
    {
        do
        {
            Field* fields = queryResult->Fetch();

            uint32 id = fields[0].GetUInt32();
            int32 type = fields[1].GetUInt32();
            int32 targetId = fields[2].GetInt32();
            uint32 chance = fields[3].GetUInt32();
            if (type < MAX_TYPE)
            {
                m_scriptTemplates[type][id].emplace_back(targetId, chance);
                if (chance)
                    m_scriptTemplatesExplicitlyChanced[type][id].emplace_back(targetId, chance);
                else
                    m_scriptTemplatesEquallyChanced[type][id].emplace_back(targetId, chance);
            }
            else
                sLog.outErrorDb("Table `dbscript_random_templates` entry (%u) uses invalid type (%u). Won't be used.", id, type);
        }
        while (queryResult->NextRow());
    }
}

void ScriptMgr::CheckRandomStringTemplates(std::set<int32>& ids)
{
    for (auto& templateData : m_scriptTemplates[STRING_TEMPLATE])
        for (auto& data : templateData.second)
            if (ids.find(data.first) != ids.end())
                ids.erase(data.first);
}

void ScriptMgr::CheckRandomRelayTemplates()
{
    auto relayScripts = GetScriptMap(SCRIPT_TYPE_RELAY);
    for (auto& templateData : m_scriptTemplates[RELAY_TEMPLATE])
        for (auto& data : templateData.second)
            if (data.first && relayScripts->second.find(data.first) == relayScripts->second.end())
                sLog.outErrorDb("Table `dbscript_random_templates` entry (%u) uses nonexistent relay ID (%u).", templateData.first, data.first);
}

void ScriptMgr::CheckScriptTexts(ScriptMapType scriptType)
{
    ScriptMapMapName const& scripts = *GetScriptMap(scriptType).get();
    for (auto itrMM = scripts.second.begin(); itrMM != scripts.second.end(); ++itrMM)
    {
        for (ScriptMap::const_iterator itrM = itrMM->second.begin(); itrM != itrMM->second.end(); ++itrM)
        {
            if (itrM->second->command == SCRIPT_COMMAND_TALK)
            {
                for (int i : itrM->second->textId)
                {
                    if (i && !sObjectMgr.GetBroadcastText(i))
                        sLog.outErrorDb("Table `broadcast_text` is missing string id %u, used in database script table %s id %u.", i, scripts.first, itrMM->first);
                }

                if (itrM->second->talk.stringTemplateId)
                {
                    auto& vector = m_scriptTemplates[STRING_TEMPLATE][itrM->second->talk.stringTemplateId];
                    for (auto& data : vector)
                    {
                        if (!sObjectMgr.GetBroadcastText(data.first))
                            sLog.outErrorDb("Table `broadcast_text` is missing string id %d, used in database script template table dbscript_random_templates id %u.", data.first, itrM->second->talk.stringTemplateId);
                    }
                }
            }
        }
    }
}

// /////////////////////////////////////////////////////////
//              DB SCRIPT ENGINE
// /////////////////////////////////////////////////////////

/// Helper function to get Object source or target for Script-Command
/// returns false iff an error happened
bool ScriptAction::GetScriptCommandObject(const ObjectGuid guid, bool includeItem, Object*& resultObject) const
{
    resultObject = nullptr;

    if (!guid)
        return true;

    switch (guid.GetHigh())
    {
        case HIGHGUID_UNIT:
            resultObject = m_map->GetCreature(guid);
            break;
        case HIGHGUID_PET:
            resultObject = m_map->GetPet(guid);
            break;
        case HIGHGUID_PLAYER:
            resultObject = m_map->GetPlayer(guid);
            break;
        case HIGHGUID_GAMEOBJECT:
            resultObject = m_map->GetGameObject(guid);
            break;
        case HIGHGUID_CORPSE:
            resultObject = HashMapHolder<Corpse>::Find(guid);
            break;
        case HIGHGUID_MO_TRANSPORT:
            resultObject = m_map->GetTransport(guid);
            break;
        case HIGHGUID_ITEM:
            // case HIGHGUID_CONTAINER: ==HIGHGUID_ITEM
        {
            if (includeItem)
            {
                if (Player* player = m_map->GetPlayer(m_ownerGuid))
                    resultObject = player->GetItemByGuid(guid);
                break;
            }
            [[fallthrough]];
        }
        default:
            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u with unsupported guid %s, skipping", m_table, m_script->id, m_script->command, guid.GetString().c_str());
            return false;
    }

    if (resultObject && !resultObject->IsInWorld())
        resultObject = nullptr;

    return true;
}

/// Select source and target for a script command
/// Returns false if an error happened
std::pair<bool, bool> ScriptAction::GetScriptProcessTargets(WorldObject* originalSource, WorldObject* originalTarget, std::vector<WorldObject*>& finalSources, std::vector<WorldObject*>& finalTargets) const
{
    std::vector<WorldObject*> buddies;

    if (m_script->buddyEntry || (m_script->data_flags & SCRIPT_FLAG_BUDDY_BY_POOL) != 0 || (m_script->data_flags & (SCRIPT_FLAG_BUDDY_BY_GUID)) != 0)
    {
        if (m_script->data_flags & (SCRIPT_FLAG_BUDDY_BY_GUID))
        {
            WorldObject* buddy = nullptr;
            if (m_script->IsCreatureBuddy())
            {
                buddy = m_map->GetCreature(m_script->searchRadiusOrGuid);

                if (buddy && ((Creature*)buddy)->IsAlive() == m_script->IsDeadOrDespawnedBuddy())
                {
                    if (m_script->command != SCRIPT_COMMAND_TERMINATE_SCRIPT)
                    {
                        sLog.outError(" DB-SCRIPTS: Process table `%s` id %u, command %u has buddy %u by guid %u but buddy is dead, skipping.", m_table, m_script->id, m_script->command, buddy->GetEntry(), m_script->searchRadiusOrGuid);
                        return { false, false };
                    }
                }
            }
            else
                buddy = m_map->GetGameObject(m_script->searchRadiusOrGuid);

            // TODO Maybe load related grid if not already done? How to handle multi-map case?
            if (!buddy && m_script->command != SCRIPT_COMMAND_TERMINATE_SCRIPT)
            {
                DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, " DB-SCRIPTS: Process table `%s` id %u, command %u has buddy by guid %u not loaded in map %u (data-flags %u), skipping.", m_table, m_script->id, m_script->command, m_script->searchRadiusOrGuid, m_map->GetId(), m_script->data_flags);
                return { false, false };
            }
            if (buddy)
                // this type can only have one buddy result
                buddies.push_back(buddy);
        }
        else if (m_script->data_flags & SCRIPT_FLAG_BUDDY_BY_POOL)
        {
            WorldObject* buddy = nullptr;
            if (m_script->IsCreatureBuddy())
            {
                PoolGroup<Creature> const& pool = sPoolMgr.GetPoolCreatures(m_script->searchRadiusOrGuid);
                auto equalChancedObjectList = pool.GetEqualChanced();
                for (auto objItr : equalChancedObjectList)
                {
                    CreatureData const* cData = sObjectMgr.GetCreatureData(objItr.guid);
                    if (Creature* creatureBuddy = m_map->GetCreature(cData->GetObjectGuid(objItr.guid)))
                    {
                        if (creatureBuddy->IsAlive() != m_script->IsDeadOrDespawnedBuddy())
                        {
                            buddy = creatureBuddy;
                            break;
                        }
                    }
                }

                if (!buddy)
                {
                    auto explicitlyChancedObjectList = pool.GetExplicitlyChanced();
                    for (auto objItr : explicitlyChancedObjectList)
                    {
                        CreatureData const* cData = sObjectMgr.GetCreatureData(objItr.guid);
                        if (Creature* creatureBuddy = m_map->GetCreature(cData->GetObjectGuid(objItr.guid)))
                        {
                            if (creatureBuddy->IsAlive() != m_script->IsDeadOrDespawnedBuddy())
                            {
                                buddy = creatureBuddy;
                                break;
                            }
                        }
                    }
                }
            }

            if (!buddy && m_script->command != SCRIPT_COMMAND_TERMINATE_SCRIPT)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u has buddy %u by pool id %u and no creature found in map %u (data-flags %u), skipping.", m_table, m_script->id, m_script->command, m_script->buddyEntry, m_script->searchRadiusOrGuid, m_map->GetId(), m_script->data_flags);
                return { false, false };
            }
            if (buddy)
                // this type can only have one buddy result
                buddies.push_back(buddy);
        }
        else if (m_script->data_flags & SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP) // Buddy by group
        {
            WorldObject* origin = originalSource ? originalSource : originalTarget;
            if (origin->GetTypeId() == TYPEID_PLAYER && originalSource && originalSource->GetTypeId() != TYPEID_PLAYER)
                origin = originalTarget;

            SpawnGroupEntry* entry = m_map->GetMapDataContainer().GetSpawnGroup(m_script->buddyEntry);
            WorldObject* closest = nullptr;
            if (entry->Type == SPAWN_GROUP_CREATURE)
            {
                for (auto& dbGuid : entry->DbGuids)
                {
                    if (Creature* creature = m_map->GetCreature(dbGuid.DbGuid))
                    {
                        if ((m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES) != 0)
                            buddies.push_back(creature);
                        else
                        {
                            if (!closest)
                                closest = creature;
                            else if (origin->GetDistance(creature) < origin->GetDistance(closest))
                                closest = creature;
                        }
                    }
                }

                if ((m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES) == 0 && closest)
                    buddies.push_back(closest);
            }
            else
            {
                for (auto& dbGuid : entry->DbGuids)
                {
                    if (GameObject* go = m_map->GetGameObject(dbGuid.DbGuid))
                    {
                        if ((m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES) != 0)
                            buddies.push_back(go);
                        else
                        {
                            if (!closest)
                                closest = go;
                            else if (origin->GetDistance(go) < origin->GetDistance(closest))
                                closest = go;
                        }
                    }
                }

                if ((m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES) == 0 && closest)
                    buddies.push_back(closest);
            }
        }
        else if (m_script->data_flags & SCRIPT_FLAG_BUDDY_BY_STRING_ID)
        {
            WorldObject* origin = originalSource ? originalSource : originalTarget;
            if (origin->GetTypeId() == TYPEID_PLAYER && originalSource && originalSource->GetTypeId() != TYPEID_PLAYER)
                origin = originalTarget;
            auto worldObjects = m_map->GetWorldObjects(m_script->buddyEntry);
            if (worldObjects == nullptr)
                return { false, false };
            if ((m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES) != 0)
            {
                for (WorldObject* wo : *worldObjects)
                    if (!wo->IsCreature() || static_cast<Creature*>(wo)->IsAlive() != m_script->IsDeadOrDespawnedBuddy())
                        buddies.push_back(wo);
            }
            else
            {
                WorldObject* closest = nullptr;
                for (WorldObject* wo : *worldObjects)
                {
                    if (wo->IsCreature() && static_cast<Creature*>(wo)->IsAlive() == m_script->IsDeadOrDespawnedBuddy())
                        continue;

                    if (!closest)
                        closest = wo;
                    else if (origin->GetDistance(wo, true, DIST_CALC_NONE) < origin->GetDistance(closest, true, DIST_CALC_NONE))
                        closest = wo;
                }
                if (closest)
                    buddies.push_back(closest);
            }
            if (m_script->searchRadiusOrGuid > 0)
                for (WorldObject* buddy : buddies)
                    if (!buddy->IsWithinDist(origin, m_script->searchRadiusOrGuid))
                        buddies.erase(std::remove(buddies.begin(), buddies.end(), buddy), buddies.end());

            if (buddies.empty() && m_script->command != SCRIPT_COMMAND_TERMINATE_SCRIPT)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u has buddy %u by pool id %u and no creature found in map %u (data-flags %u), skipping.", m_table, m_script->id, m_script->command, m_script->buddyEntry, m_script->searchRadiusOrGuid, m_map->GetId(), m_script->data_flags);
                return { false, false };
            }
        }
        else                                                // Buddy by entry
        {
            if (!originalSource && !originalTarget)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u called without buddy %u, but no source for search available, skipping.", m_table, m_script->id, m_script->command, m_script->buddyEntry);
                return { false, false };
            }

            // Prefer non-players as searcher
            WorldObject* origin = originalSource ? originalSource : originalTarget;
            if (origin->GetTypeId() == TYPEID_PLAYER && originalSource && originalSource->GetTypeId() != TYPEID_PLAYER)
                origin = originalTarget;

            if (m_script->IsCreatureBuddy())
            {
                Creature* creatureBuddy = nullptr;

                if (m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES)
                {
                    CreatureList creatures;
                    std::set<uint32> entries; // support for multiple entries
                    entries.insert(m_script->buddyEntry);
                    MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck u_check(*origin, entries, !m_script->IsDeadOrDespawnedBuddy(), m_script->searchRadiusOrGuid);
                    MaNGOS::CreatureListSearcher<MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck> searcher(creatures, u_check);
                    Cell::VisitAllObjects(origin, searcher, m_script->searchRadiusOrGuid); // Visit all, need to find also Pet* objects
                    for (Creature* creature : creatures)
                        buddies.push_back(creature);
                }
                else
                {
                    if (m_script->IsDeadOrDespawnedBuddy())
                    {
                        MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(origin, m_script->buddyEntry, m_script->searchRadiusOrGuid);
                        MaNGOS::CreatureLastSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatureBuddy, u_check);
                        Cell::VisitGridObjects(origin, searcher, m_script->searchRadiusOrGuid);
                    }
                    else
                    {
                        MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck u_check(*origin, m_script->buddyEntry, true, false, m_script->searchRadiusOrGuid, true);
                        MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creatureBuddy, u_check);

                        if (m_script->data_flags & SCRIPT_FLAG_BUDDY_IS_PET)
                            Cell::VisitWorldObjects(origin, searcher, m_script->searchRadiusOrGuid);
                        else                                        // Normal Creature
                            Cell::VisitGridObjects(origin, searcher, m_script->searchRadiusOrGuid);
                    }

                    if (creatureBuddy)
                        buddies.push_back(creatureBuddy);

                    // TODO: Remove this extra check output after a while - it might have false effects
                    if (!creatureBuddy && origin->GetEntry() == m_script->buddyEntry)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: WARNING: Process table `%s` id %u, command %u has no OTHER buddy %u found - maybe you need to update the script?", m_table, m_script->id, m_script->command, m_script->buddyEntry);
                        buddies.push_back(creatureBuddy);
                    }
                }
            }
            else
            {
                if (m_script->data_flags & SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES)
                {
                    GameObjectList gos;
                    std::set<uint32> entries; // support for multiple entries
                    entries.insert(m_script->buddyEntry);
                    MaNGOS::AllGameObjectEntriesListInObjectRangeCheck go_check(*origin, entries, m_script->searchRadiusOrGuid);
                    MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectEntriesListInObjectRangeCheck> checker(gos, go_check);
                    Cell::VisitGridObjects(origin, checker, m_script->searchRadiusOrGuid);
                    for (GameObject* go : gos)
                        buddies.push_back(go);
                }
                else
                {
                    GameObject* goBuddy = nullptr;

                    MaNGOS::NearestGameObjectEntryInObjectRangeCheck u_check(*origin, m_script->buddyEntry, m_script->searchRadiusOrGuid);
                    MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(goBuddy, u_check);

                    Cell::VisitGridObjects(origin, searcher, m_script->searchRadiusOrGuid);
                    if (goBuddy)
                        buddies.push_back(goBuddy);
                }                
            }

            if (buddies.empty() && m_script->command != SCRIPT_COMMAND_TERMINATE_SCRIPT)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u has buddy %u not found in range %u of searcher %s (data-flags %u), skipping.", m_table, m_script->id, m_script->command, m_script->buddyEntry, m_script->searchRadiusOrGuid, origin->GetGuidStr().c_str(), m_script->data_flags);
                return { false, false };
            }
        }
    }

    if (m_script->data_flags & SCRIPT_FLAG_BUDDY_AS_TARGET)
    {
        finalTargets = buddies;
    }
    else
    {
        if (!buddies.empty())
            finalSources = buddies;
    }

    if (m_script->data_flags & SCRIPT_FLAG_REVERSE_DIRECTION)
        std::swap(finalSources, finalTargets);

    if (m_script->data_flags & SCRIPT_FLAG_SOURCE_TARGETS_SELF)
        finalTargets = finalSources;

    return { true, buddies.size() };
}

/// Helper to log error information
bool ScriptAction::LogIfNotCreature(WorldObject* pWorldObject) const
{
    if (!pWorldObject || pWorldObject->GetTypeId() != TYPEID_UNIT)
    {
        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for non-creature, skipping.", m_table, m_script->id, m_script->command);
        return true;
    }
    return false;
}
bool ScriptAction::LogIfNotUnit(WorldObject* pWorldObject) const
{
    if (!pWorldObject || !pWorldObject->isType(TYPEMASK_UNIT))
    {
        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for non-unit, skipping.", m_table, m_script->id, m_script->command);
        return true;
    }
    return false;
}
bool ScriptAction::LogIfNotGameObject(WorldObject* pWorldObject) const
{
    if (!pWorldObject || pWorldObject->GetTypeId() != TYPEID_GAMEOBJECT)
    {
        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for non-gameobject, skipping.", m_table, m_script->id, m_script->command);
        return true;
    }
    return false;
}
bool ScriptAction::LogIfNotPlayer(WorldObject* pWorldObject) const
{
    if (!pWorldObject || pWorldObject->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for non-player, skipping.", m_table, m_script->id, m_script->command);
        return true;
    }
    return false;
}

/// Helper to get a player if possible (target preferred)
Player* ScriptAction::GetPlayerTargetOrSourceAndLog(WorldObject* pSource, WorldObject* pTarget) const
{
    if ((!pTarget || pTarget->GetTypeId() != TYPEID_PLAYER) && (!pSource || pSource->GetTypeId() != TYPEID_PLAYER))
    {
        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for non player, skipping.", m_table, m_script->id, m_script->command);
        return nullptr;
    }

    return pTarget && pTarget->GetTypeId() == TYPEID_PLAYER ? (Player*)pTarget : (Player*)pSource;
}

ScriptAction::ScriptAction(ScriptMapType scriptType, Map* _map, ObjectGuid _sourceGuid, ObjectGuid _targetGuid, ObjectGuid _ownerGuid, std::shared_ptr<ScriptInfo> _script)
    : m_scriptType(scriptType), m_table(scriptTableNames[scriptType].data()), m_map(_map), m_sourceGuid(_sourceGuid), m_targetGuid(_targetGuid), m_ownerGuid(_ownerGuid), m_script(_script)
{

}

/// Handle one Script Step
// Return true if and only if further parts of this script shall be skipped
bool ScriptAction::HandleScriptStep()
{
    std::vector<WorldObject*> sources;
    std::vector<WorldObject*> targets;

    Object* itemSource = nullptr;
    bool buddyFound;

    {
        // Add scope for source & target variables so that they are not used below
        Object* source = nullptr;
        Object* target = nullptr;
        if (!GetScriptCommandObject(m_sourceGuid, true, source))
            return false;
        if (!GetScriptCommandObject(m_targetGuid, false, target))
            return false;

        // Give some debug log output for easier use
        DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u, command %u for source %s (%sin world), target %s (%sin world)", m_table, m_script->id, m_script->command, m_sourceGuid.GetString().c_str(), source ? "" : "not ", m_targetGuid.GetString().c_str(), target ? "" : "not ");

        // Get expected source and target (if defined with buddy)
        if (source && source->isType(TYPEMASK_WORLDOBJECT))
            sources.push_back(static_cast<WorldObject*>(source));
        if (target && target->isType(TYPEMASK_WORLDOBJECT))
            targets.push_back(static_cast<WorldObject*>(target));
        bool success;
        std::tie(success, buddyFound) = GetScriptProcessTargets(dynamic_cast<WorldObject*>(source), dynamic_cast<WorldObject*>(target), sources, targets);
        if (!success)
            return false;

        if (source && source->isType(TYPEMASK_ITEM))
            itemSource = source;
    }

    bool finalResult = false;

    std::vector<std::pair<WorldObject*, WorldObject*>> eligiblePairs;
    if (!targets.empty())
    {
        for (auto source : sources)
            for (auto target : targets)
                eligiblePairs.emplace_back(source, target);
    }
    else
    {
        for (auto source : sources)
            eligiblePairs.emplace_back(source, nullptr);
    }

    for (auto& data : eligiblePairs)
    {
        WorldObject* pSource = data.first;
        WorldObject* pTarget = data.second;
        Object* pSourceOrItem = pSource ? pSource : itemSource;

        bool result = ExecuteDbscriptCommand(pSource, pTarget, pSourceOrItem, buddyFound);
        if (result == true)
            finalResult = true;
    }

    return finalResult;
}

bool ScriptAction::ExecuteDbscriptCommand(WorldObject* pSource, WorldObject* pTarget, Object* pSourceOrItem, bool buddyFound)
{
    if (m_script->condition_id && !sObjectMgr.IsConditionSatisfied(m_script->condition_id, pTarget, m_map, pSource, CONDITION_FROM_DBSCRIPTS))
        return false;

    switch (m_script->command)
    {
        case SCRIPT_COMMAND_TALK:                           // 0
        {
            if (!pSource)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u found no worldobject as source, skipping.", m_table, m_script->id, m_script->command);
                break;
            }

            Unit* unitTarget = pTarget && pTarget->isType(TYPEMASK_UNIT) ? static_cast<Unit*>(pTarget) : nullptr;
            int32 textId = m_script->textId[0];

            if (m_script->talk.stringTemplateId)
            {
                textId = sScriptMgr.GetRandomScriptStringFromTemplate(m_script->talk.stringTemplateId);
                if (textId == 0)
                    break;
            }
            else
            {
                // May have text for random
                if (m_script->textId[1])
                {
                    int i = 2;
                    for (; i < MAX_TEXT_ID; ++i)
                    {
                        if (!m_script->textId[i])
                            break;
                    }

                    // Use one random
                    textId = m_script->textId[urand(0, i - 1)];
                }
            }

            if (!DoDisplayText(pSource, textId, unitTarget))
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, could not display text %i properly", m_table, m_script->id, textId);
            break;
        }
        case SCRIPT_COMMAND_EMOTE:                          // 1
        {
            if (LogIfNotUnit(pSource))
                break;

            std::vector<uint32> emotes;
            emotes.push_back(m_script->emote.emoteId);
            for (int i : m_script->textId)
            {
                if (!i)
                    break;
                emotes.push_back(uint32(i));
            }

            ((Unit*)pSource)->HandleEmote(emotes[urand(0, emotes.size() - 1)]);
            break;
        }
        case SCRIPT_COMMAND_FIELD_SET:                      // 2
            if (!pSourceOrItem)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for nullptr object.", m_table, m_script->id, m_script->command);
                break;
            }
            if (m_script->setField.fieldId <= OBJECT_FIELD_ENTRY || m_script->setField.fieldId >= pSourceOrItem->GetValuesCount())
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for wrong field %u (max count: %u) in %s.",
                                m_table, m_script->id, m_script->command, m_script->setField.fieldId, pSourceOrItem->GetValuesCount(), pSourceOrItem->GetGuidStr().c_str());
                break;
            }
            pSourceOrItem->SetUInt32Value(m_script->setField.fieldId, m_script->setField.fieldValue);
            break;
        case SCRIPT_COMMAND_MOVE_TO:                        // 3
        {
            if (LogIfNotUnit(pSource))
                break;

            Creature* creature = static_cast<Creature*>(pSource);

            if (m_script->textId[0])
            {
                if (m_script->textId[0] == 1 || m_script->textId[0] == 2 && !creature->GetCreatureGroup())
                {
                    Position const& respPos = creature->GetRespawnPosition();
                    creature->GetMotionMaster()->MovePoint(0, respPos, ForcedMovement(m_script->moveTo.forcedMovement), 0.f, true, creature->GetObjectGuid(), m_script->moveTo.relayId);
                }
                else if (m_script->textId[0] == 2)
                {
                    creature->GetCreatureGroup()->MoveHome();
                }
                break;
            }

            // Just turn around
            if ((m_script->x == 0.0f && m_script->y == 0.0f && m_script->z == 0.0f) ||
                    // Check point-to-point distance, hence revert effect of bounding radius
                    ((Unit*)pSource)->IsWithinDist3d(m_script->x, m_script->y, m_script->z, 0.01f - ((Unit*)pSource)->GetObjectBoundingRadius()))
            {
                ((Unit*)pSource)->SetFacingTo(m_script->o);
                break;
            }

            // For command additional teleport the unit
            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
            {
                ((Unit*)pSource)->NearTeleportTo(m_script->x, m_script->y, m_script->z, m_script->o != 0.0f ? m_script->o : ((Unit*)pSource)->GetOrientation());
                break;
            }

            // Normal Movement
            if (m_script->moveTo.travelSpeed)
                ((Unit*)pSource)->GetMotionMaster()->MoveCharge(m_script->x, m_script->y, m_script->z, m_script->moveTo.travelSpeed * 0.01f, 0);
            else
            {
                ((Unit*)pSource)->GetMotionMaster()->Clear();
                ((Unit*)pSource)->GetMotionMaster()->MovePoint(0, Position(m_script->x, m_script->y, m_script->z, m_script->o), ForcedMovement(m_script->moveTo.forcedMovement), 0.f, true, pTarget ? pTarget->GetObjectGuid() : ObjectGuid(), m_script->moveTo.relayId);
            }
            break;
        }
        case SCRIPT_COMMAND_FLAG_SET:                       // 4
            if (!pSourceOrItem)
            {
                sLog.outErrorDb("SCRIPT_COMMAND_FLAG_SET (script id %u) call for nullptr object.", m_script->id);
                break;
            }
            if (m_script->setFlag.fieldId <= OBJECT_FIELD_ENTRY || m_script->setFlag.fieldId >= pSourceOrItem->GetValuesCount())
            {
                sLog.outErrorDb("SCRIPT_COMMAND_FLAG_SET (script id %u) call for wrong field %u (max count: %u) in %s.",
                                m_script->id, m_script->setFlag.fieldId, pSourceOrItem->GetValuesCount(), pSourceOrItem->GetGuidStr().c_str());
                break;
            }
            pSourceOrItem->SetFlag(m_script->setFlag.fieldId, m_script->setFlag.fieldValue);
            break;
        case SCRIPT_COMMAND_FLAG_REMOVE:                    // 5
            if (!pSourceOrItem)
            {
                sLog.outErrorDb("SCRIPT_COMMAND_FLAG_REMOVE (script id %u) call for nullptr object.", m_script->id);
                break;
            }
            if (m_script->removeFlag.fieldId <= OBJECT_FIELD_ENTRY || m_script->removeFlag.fieldId >= pSourceOrItem->GetValuesCount())
            {
                sLog.outErrorDb("SCRIPT_COMMAND_FLAG_REMOVE (script id %u) call for wrong field %u (max count: %u) in %s.",
                                m_script->id, m_script->removeFlag.fieldId, pSourceOrItem->GetValuesCount(), pSourceOrItem->GetGuidStr().c_str());
                break;
            }
            pSourceOrItem->RemoveFlag(m_script->removeFlag.fieldId, m_script->removeFlag.fieldValue);
            break;
        case SCRIPT_COMMAND_TELEPORT_TO:                    // 6
        {
            Player* pPlayer = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
            if (!pPlayer)
                break;

            pPlayer->TeleportTo(m_script->teleportTo.mapId, m_script->x, m_script->y, m_script->z, m_script->o);
            break;
        }
        case SCRIPT_COMMAND_QUEST_EXPLORED:                 // 7
        {
            Player* pPlayer = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
            if (!pPlayer)
                break;

            WorldObject* pWorldObject = nullptr;
            if (pSource && pSource->isType(TYPEMASK_CREATURE_OR_GAMEOBJECT))
                pWorldObject = pSource;
            else if (pTarget && pTarget->isType(TYPEMASK_CREATURE_OR_GAMEOBJECT))
                pWorldObject = pTarget;

            // if we have a distance, we must have a worldobject
            if (m_script->questExplored.distance != 0 && !pWorldObject)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u called without source worldobject, skipping.", m_table, m_script->id, m_script->command);
                break;
            }

            bool failQuest = false;
            // Creature must be alive for giving credit
            if (pWorldObject && pWorldObject->GetTypeId() == TYPEID_UNIT && !((Creature*) pWorldObject)->IsAlive())
                failQuest = true;
            else if (m_script->questExplored.distance != 0 && !pWorldObject->IsWithinDistInMap(pPlayer, float(m_script->questExplored.distance)))
                failQuest = true;

            // quest id and flags checked at script loading
            if (!failQuest)
                pPlayer->AreaExploredOrEventHappens(m_script->questExplored.questId);
            else
                pPlayer->FailQuest(m_script->questExplored.questId);

            break;
        }
        case SCRIPT_COMMAND_KILL_CREDIT:                    // 8
        {
            Player* pPlayer = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
            if (!pPlayer)
                break;

            uint32 creatureEntry = m_script->killCredit.creatureEntry;
            WorldObject* pRewardSource = pSource && pSource->GetTypeId() == TYPEID_UNIT ? pSource : (pTarget && pTarget->GetTypeId() == TYPEID_UNIT ? pTarget : nullptr);

            // dynamic effect, take entry of reward Source
            if (!creatureEntry)
            {
                if (pRewardSource)
                    creatureEntry =  pRewardSource->GetEntry();
                else
                {
                    sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u called for dynamic killcredit without creature partner, skipping.", m_table, m_script->id, m_script->command);
                    break;
                }
            }

            if (m_script->killCredit.isGroupCredit)
            {
                WorldObject* pSearcher = pRewardSource ? pRewardSource : (pSource ? pSource : pTarget);
                if (pSearcher != pRewardSource)
                    sLog.outDebug(" DB-SCRIPTS: Process table `%s` id %u, SCRIPT_COMMAND_KILL_CREDIT called for groupCredit without creature as searcher, script might need adjustment.", m_table, m_script->id);
                pPlayer->RewardPlayerAndGroupAtEventCredit(creatureEntry, pSearcher);
            }
            else
                pPlayer->KilledMonsterCredit(creatureEntry, pRewardSource ? pRewardSource->GetObjectGuid() : ObjectGuid());

            break;
        }
        case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:             // 9
        {
            GameObject* pGo = nullptr;
            uint32 time_to_despawn = m_script->respawnGo.despawnDelay;

            if (m_script->respawnGo.goGuid)
            {
                GameObjectData const* goData = sObjectMgr.GetGOData(m_script->respawnGo.goGuid);
                if (!goData)
                    break;                                  // checked at load

                pGo = m_map->GetGameObject(m_script->respawnGo.goGuid);
            }
            else
            {
                if (LogIfNotGameObject(pSource))
                    break;

                pGo = (GameObject*)pSource;
            }

            if (!pGo)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed for gameobject(guid: %u, buddyEntry: %u).", m_table, m_script->id, m_script->command, m_script->respawnGo.goGuid, m_script->buddyEntry);
                break;
            }

            if (pGo->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE ||
                    pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u can not be used with gameobject of type %u (guid: %u, buddyEntry: %u).", m_table, m_script->id, m_script->command, uint32(pGo->GetGoType()), m_script->respawnGo.goGuid, m_script->buddyEntry);
                break;
            }

            if (pGo->IsSpawned())
                break;                                      // gameobject already spawned

            if (pGo->IsSpawnedByDefault()) // static spawned go - can only respawn
                pGo->Respawn();
            else
            {
                pGo->SetLootState(GO_READY);
                pGo->SetRespawnTime(time_to_despawn);           // despawn object in ? seconds
                pGo->Refresh();
            }
            break;
        }
        case SCRIPT_COMMAND_TEMP_SPAWN_CREATURE:            // 10
        {
            if (!pSource)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u found no worldobject as source, skipping.", m_table, m_script->id, m_script->command);
                break;
            }

            float x = m_script->x;
            float y = m_script->y;
            float z = m_script->z;
            float o = m_script->o;
            bool run = m_script->textId[0] == 1;

            TempSpawnSettings settings(pSource, m_script->summonCreature.creatureEntry, x, y, z, o, m_script->summonCreature.despawnDelay ? TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN : TEMPSPAWN_DEAD_DESPAWN, m_script->summonCreature.despawnDelay, (m_script->data_flags& SCRIPT_FLAG_COMMAND_ADDITIONAL) != 0, run, m_script->summonCreature.pathId);
            settings.spawnDataEntry = m_script->textId[3];
            settings.dbscriptTarget = pTarget;

            Creature* pCreature = WorldObject::SummonCreature(settings, pSource->GetMap());
            if (!pCreature)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed for creature (entry: %u).", m_table, m_script->id, m_script->command, m_script->summonCreature.creatureEntry);
                break;
            }
            break;
        }
        case SCRIPT_COMMAND_OPEN_DOOR:                      // 11
        case SCRIPT_COMMAND_CLOSE_DOOR:                     // 12
        {
            GameObject* door;
            uint32 time_to_reset = m_script->changeDoor.resetDelay < 15 ? 15 : m_script->changeDoor.resetDelay;

            if (m_script->changeDoor.goGuid)
            {
                door = m_map->GetGameObject(m_script->changeDoor.goGuid);
            }
            else
            {
                if (LogIfNotGameObject(pSource))
                    break;

                door = static_cast<GameObject*>(pSource);
            }

            if (!door)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed for gameobject(guid: %u, buddyEntry: %u).", m_table, m_script->id, m_script->command, m_script->changeDoor.goGuid, m_script->buddyEntry);
                break;
            }

            if (door->GetGoType() != GAMEOBJECT_TYPE_DOOR && door->GetGoType() != GAMEOBJECT_TYPE_BUTTON)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed for non-door(GoType: %u).", m_table, m_script->id, m_script->command, door->GetGoType());
                break;
            }

            if ((m_script->command == SCRIPT_COMMAND_OPEN_DOOR && door->GetGoState() != GO_STATE_READY) ||
                    (m_script->command == SCRIPT_COMMAND_CLOSE_DOOR && door->GetGoState() == GO_STATE_READY))
                break;                                      // to be opened door already open, or to be closed door already closed

            door->UseOpenableObject(m_script->command == SCRIPT_COMMAND_OPEN_DOOR, time_to_reset, m_script->changeDoor.alternate);

            if (pTarget && pTarget->IsGameObject() && static_cast<GameObject*>(pTarget)->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
                static_cast<GameObject*>(pTarget)->UseOpenableObject(m_script->command == SCRIPT_COMMAND_OPEN_DOOR, time_to_reset, m_script->changeDoor.alternate);

            break;
        }
        case SCRIPT_COMMAND_ACTIVATE_OBJECT:                // 13
        {
            if (LogIfNotUnit(pSource))
                break;
            if (LogIfNotGameObject(pTarget))
                break;

            GameObject* go = static_cast<GameObject*>(pTarget);

            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                go->SendGameObjectCustomAnim(go->GetObjectGuid(), m_script->activateObject.animId);
            else
                go->Use((Unit*)pSource);
            break;
        }
        case SCRIPT_COMMAND_REMOVE_AURA:                    // 14
        {
            if (LogIfNotUnit(pSource))
                break;

            Unit* uSource = static_cast<Unit*>(pSource);
            if (m_script->removeAura.defaultOrChargeOrStack == 0)
            {
                // Flag Command Additional removes aura by caster
                if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                {
                    if (LogIfNotUnit(pTarget))
                        break;
                    uSource->RemoveAurasByCasterSpell(m_script->removeAura.spellId, pTarget->GetObjectGuid());
                }
                else
                    uSource->RemoveAurasDueToSpell(m_script->removeAura.spellId);
            }
            else if (m_script->removeAura.defaultOrChargeOrStack == 1)
                uSource->RemoveAuraCharge(m_script->removeAura.spellId);
            else if (m_script->removeAura.defaultOrChargeOrStack == 2)
                uSource->RemoveAuraStack(m_script->removeAura.spellId);

            break;
        }
        case SCRIPT_COMMAND_CAST_SPELL:                     // 15
        {
            // Select Spell
            uint32 spellId = m_script->castSpell.spellId;
            uint32 filledCount = 0;
            while (filledCount < MAX_TEXT_ID && m_script->textId[filledCount])  // Count which dataint fields are filled
                ++filledCount;
            if (filledCount > 0)
                if (uint32 randomField = urand(0, filledCount))               // Random selection resulted in one of the dataint fields
                    spellId = m_script->textId[randomField - 1];

            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (spellInfo->HasAttribute(SPELL_ATTR_EX_EXCLUDE_CASTER) && pTarget == pSource)
                pTarget = nullptr;
            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                pTarget = nullptr;

            // TODO: when GO cast implemented, code below must be updated accordingly to also allow GO spell cast
            if (pSource && pSource->IsGameObject())
            {
                if (LogIfNotUnit(pTarget))
                    break;

                static_cast<Unit*>(pTarget)->CastSpell(static_cast<Unit*>(pTarget), spellId, TRIGGERED_OLD_TRIGGERED | TRIGGERED_DO_NOT_PROC, nullptr, nullptr, pSource->GetObjectGuid());
                break;
            }

            if (LogIfNotUnit(pSource))
                break;

            SpellCastTargets spellCastTargets;
            if (pTarget)
            {
                if (pTarget->IsUnit())
                  spellCastTargets.setUnitTarget(static_cast<Unit*>(pTarget));
                else if (pTarget->IsGameObject())
                    spellCastTargets.setGOTarget(static_cast<GameObject*>(pTarget));

                if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
                    spellCastTargets.setDestination(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ());
            }

            if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
                spellCastTargets.setSource(pSource->GetPositionX(), pSource->GetPositionY(), pSource->GetPositionZ());

            static_cast<Unit*>(pSource)->CastSpell(spellCastTargets, spellInfo, m_script->castSpell.castFlags | TRIGGERED_DO_NOT_PROC);
            break;
        }
        case SCRIPT_COMMAND_PLAY_SOUND:                     // 16
        {
            if (!pSource)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u could not find proper source", m_table, m_script->id, m_script->command);
                break;
            }

            // bitmask: 0/1=target-player, 0/2=with distance dependent, 0/4=map wide, 0/8=zone wide
            Player* pSoundTarget = nullptr;
            if (m_script->playSound.flags & 1)
            {
                pSoundTarget = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
                if (!pSoundTarget)
                    break;
            }

            PlayPacketParameters params(PLAY_SET);
            if (pSoundTarget)
                params = PlayPacketParameters(PLAY_TARGET, pSoundTarget);
            if (m_script->playSound.flags & 8) // playParameter is zoneId
                params = PlayPacketParameters(PLAY_ZONE, m_script->playSound.playParameter ? m_script->playSound.playParameter : pSource->GetZoneId());
            if (m_script->playSound.flags & 16) // playParameter is areaId
                params = PlayPacketParameters(PLAY_AREA, m_script->playSound.playParameter ? m_script->playSound.playParameter : pSource->GetAreaId());

            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                pSource->PlayMusic(m_script->playSound.soundId, params);
            else
            {
                if (m_script->playSound.flags & 2)
                    pSource->PlayDistanceSound(m_script->playSound.soundId, params);
                else if (m_script->playSound.flags & 4)
                    m_map->PlayDirectSoundToMap(m_script->playSound.soundId);
                else
                    pSource->PlayDirectSound(m_script->playSound.soundId, params);
            }
            break;
        }
        case SCRIPT_COMMAND_CREATE_ITEM:                    // 17
        {
            Player* pPlayer = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
            if (!pPlayer)
                break;

            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                pPlayer->DestroyItemCount(m_script->createItem.itemEntry, m_script->createItem.amount, true, false);
            else
            {
                if (Item* pItem = pPlayer->StoreNewItemInInventorySlot(m_script->createItem.itemEntry, m_script->createItem.amount))
                    pPlayer->SendNewItem(pItem, m_script->createItem.amount, true, false);
            }

            break;
        }
        case SCRIPT_COMMAND_DESPAWN_SELF:                   // 18
        {
            // TODO - Remove this check after a while
            if (pTarget && pTarget->GetTypeId() != TYPEID_UNIT && pSource && pSource->GetTypeId() == TYPEID_UNIT)
            {
                sLog.outErrorDb("DB-SCRIPTS: Process table `%s` id %u, command %u target must be creature, but (only) source is, use data_flags to fix", m_table, m_script->id, m_script->command);
                pTarget = pSource;
            }

            if (LogIfNotCreature(pTarget))
                break;

            ((Creature*)pTarget)->ForcedDespawn(m_script->despawn.despawnDelay);

            break;
        }
        case SCRIPT_COMMAND_PLAY_MOVIE:                     // 19
        {
            break;                                      // must be skipped at loading
        }
        case SCRIPT_COMMAND_MOVEMENT:                       // 20
        {
            if (LogIfNotCreature(pSource))
                break;

            Creature* source = static_cast<Creature*>(pSource);

            // Consider add additional checks for cases where creature should not change movementType
            // (pet? already using same MMgen as script try to apply?)

            if (source->IsInCombat())
            {
                DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, " DB-SCRIPTS: Process table `%s` id %u, SCRIPT_COMMAND_MOVEMENT called for movement change to %u with source guid %s but source is in combat and may lead to wrong behaviour: skipping.", m_table, m_script->id, m_script->movement.movementType, pSource->GetGuidStr().c_str());
                break;
            }

            uint32 wanderORpathId = m_script->movement.wanderORpathId;

            WaypointPathOrigin wp_origin = PATH_NO_PATH;
            if (m_script->movement.timerOrPassTarget & 0x2)
                wp_origin = PATH_FROM_WAYPOINT_PATH;

            ObjectGuid targetGuid;

            ForcedMovement forcedMovement = ForcedMovement(m_script->textId[0]);

            auto fSlot = source->GetFormationSlot();
            if (fSlot)
            {
                if (!fSlot->IsFormationMaster())
                {
                    sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for creature in formation, skipping.", m_table, m_script->id, m_script->command);
                    break;
                }

                fSlot->GetFormationData()->SetMovementInfo(MovementGeneratorType(m_script->movement.movementType), m_script->movement.wanderORpathId);
            }

            if (m_script->movement.movementType == WAYPOINT_MOTION_TYPE || m_script->movement.movementType == PATH_MOTION_TYPE)
            {
                if (pTarget)
                    targetGuid = pTarget->GetObjectGuid();
                else
                {
                    if ((m_script->movement.timerOrPassTarget & 0x1) != 0)
                    {
                        DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, " DB-SCRIPTS: Process table `%s` id %u, SCRIPT_COMMAND_MOVEMENT called for movement change to %u with source guid %s, pass target true and target nullptr: skipping.", m_table, m_script->id, m_script->movement.movementType, pSource->GetGuidStr().c_str());
                        break;
                    }
                }
            }

            switch (m_script->movement.movementType)
            {
                case IDLE_MOTION_TYPE:
                    source->StopMoving();
                    source->GetMotionMaster()->Clear(false, true);
                    source->GetMotionMaster()->MoveIdle();
                    break;
                case RANDOM_MOTION_TYPE:
                    if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                        source->GetMotionMaster()->MoveRandomAroundPoint(pSource->GetPositionX(), pSource->GetPositionY(), pSource->GetPositionZ(), float(m_script->movement.wanderORpathId), 0.f, m_script->movement.timerOrPassTarget);
                    else
                    {
                        float respX, respY, respZ, respO, wander_distance;
                        source->GetRespawnCoord(respX, respY, respZ, &respO, &wander_distance);
                        wander_distance = wanderORpathId ? wanderORpathId : wander_distance;
                        source->GetMotionMaster()->MoveRandomAroundPoint(respX, respY, respZ, wander_distance, 0.f, m_script->movement.timerOrPassTarget);
                    }
                    break;
                case WAYPOINT_MOTION_TYPE:
                {
                    source->StopMoving();
                    source->GetMotionMaster()->Clear(false, true);
                    source->GetMotionMaster()->MoveWaypoint(wanderORpathId, wp_origin, 0, 0, forcedMovement, targetGuid);
                    break;
                }
                case PATH_MOTION_TYPE:
                {
                    source->StopMoving();
                    source->GetMotionMaster()->MovePath(wanderORpathId, wp_origin, forcedMovement, false, 0.f, false, targetGuid);
                    break;
                }
                case LINEAR_WP_MOTION_TYPE:
                {
                    source->StopMoving();
                    source->GetMotionMaster()->Clear(false, true);
                    source->GetMotionMaster()->MoveLinearWP(wanderORpathId, wp_origin, 0, 0, forcedMovement, targetGuid);
                    break;
                }
                case FALL_MOTION_TYPE:
                {
                    source->StopMoving();
                    source->GetMotionMaster()->MoveFall(targetGuid, wanderORpathId);
                    break;
                }
            }
            break;
        }
        case SCRIPT_COMMAND_SET_ACTIVEOBJECT:               // 21
        {
            if (pSource->GetTypeId() == TYPEID_PLAYER)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for player, skipping.", m_table, m_script->id, m_script->command);
                break;;
            }

            pSource->SetActiveObjectState(m_script->activeObject.activate != 0);
            break;
        }
        case SCRIPT_COMMAND_SET_FACTION:                    // 22
        {
            if (LogIfNotCreature(pSource))
                break;

            if (m_script->faction.factionId)
                ((Creature*)pSource)->SetFactionTemporary(m_script->faction.factionId, m_script->faction.flags);
            else
                ((Creature*)pSource)->ClearTemporaryFaction();

            break;
        }
        case SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL:        // 23
        {
            if (LogIfNotCreature(pSource))
                break;

            if (!m_script->morph.creatureOrModelEntry)
                ((Creature*)pSource)->DeMorph();
            else if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                ((Creature*)pSource)->SetDisplayId(m_script->morph.creatureOrModelEntry);
            else
            {
                CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(m_script->morph.creatureOrModelEntry);
                uint32 display_id = Creature::ChooseDisplayId(ci);

                ((Creature*)pSource)->SetDisplayId(display_id);
            }

            break;
        }
        case SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL:        // 24
        {
            if (LogIfNotCreature(pSource))
                break;

            Creature* creatureSource = static_cast<Creature*>(pSource);
            if (m_script->mount.speedChange) // new flow
            {
                if (m_script->mount.creatureOrModelEntry)
                    creatureSource->MountEntry(m_script->mount.creatureOrModelEntry);
                else
                    creatureSource->UnmountEntry();
            }
            else
            {
                if (!m_script->mount.creatureOrModelEntry)
                    creatureSource->Unmount();
                else if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                    creatureSource->Mount(m_script->mount.creatureOrModelEntry);
                else
                {
                    CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(m_script->mount.creatureOrModelEntry);
                    uint32 display_id = Creature::ChooseDisplayId(ci);

                    creatureSource->Mount(display_id);
                }
            }

            break;
        }
        case SCRIPT_COMMAND_SET_RUN:                        // 25
        {
            if (LogIfNotCreature(pSource))
                break;

            Creature* creature = static_cast<Creature*>(pSource);
            if (creature->IsWalking())
            {
                if (m_script->run.run)
                    creature->SetWalk(false, true);
            }
            else
            {
                if (!m_script->run.run)
                    creature->SetWalk(true, true);
            }

            break;
        }
        case SCRIPT_COMMAND_ATTACK_START:                   // 26
        {
            if (LogIfNotCreature(pSource))
                break;
            if (LogIfNotUnit(pTarget))
                break;

            Creature* pAttacker = static_cast<Creature*>(pSource);
            Unit* unitTarget = static_cast<Unit*>(pTarget);

            if (!pAttacker->CanAttackNow(unitTarget))
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u source can not attack (Attacker: %s, Target: %s)", m_table, m_script->id, m_script->command, pAttacker->GetGuidStr().c_str(), unitTarget->GetGuidStr().c_str());
                break;
            }

            pAttacker->AI()->AttackStart(unitTarget);

            break;
        }
        case SCRIPT_COMMAND_GO_LOCK_STATE:                  // 27
        {
            if (LogIfNotGameObject(pSource))
                break;

            GameObject* pGo = static_cast<GameObject*>(pSource);

            /* flag lockState
             * go_lock          0x01
             * go_unlock        0x02
             * go_nonInteract   0x04
             * go_Interact      0x08
             */

            // Lock or Unlock
            if (m_script->goLockState.lockState & 0x01)
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            else if (m_script->goLockState.lockState & 0x02)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            // Set Non Interactable or Set Interactable
            if (m_script->goLockState.lockState & 0x04)
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            else if (m_script->goLockState.lockState & 0x08)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);

            break;
        }
        case SCRIPT_COMMAND_STAND_STATE:                    // 28
        {
            if (LogIfNotCreature(pSource))
                break;

            // Must be safe cast to Unit* here
            ((Unit*)pSource)->SetStandState(m_script->standState.stand_state);
            break;
        }
        case SCRIPT_COMMAND_MODIFY_NPC_FLAGS:               // 29
        {
            if (LogIfNotCreature(pSource))
                break;

            // Remove Flags
            if (m_script->npcFlag.change_flag == 0)
                pSource->RemoveFlag(UNIT_NPC_FLAGS, m_script->npcFlag.flag);
            // Add Flags
            else if (m_script->npcFlag.change_flag == 1)
                pSource->SetFlag(UNIT_NPC_FLAGS, m_script->npcFlag.flag);
            // Toggle Flags
            else if (m_script->npcFlag.change_flag == 2)
            {
                if (pSource->HasFlag(UNIT_NPC_FLAGS, m_script->npcFlag.flag))
                    pSource->RemoveFlag(UNIT_NPC_FLAGS, m_script->npcFlag.flag);
                else
                    pSource->SetFlag(UNIT_NPC_FLAGS, m_script->npcFlag.flag);
            }
            else
                sLog.outErrorDb(" DB-SCRIPTS: Unexpected value %u used for script id %u, command %u.", m_script->npcFlag.flag, m_script->id, m_script->command);

            break;
        }
        case SCRIPT_COMMAND_SEND_TAXI_PATH:                 // 30
        {
            // only Player
            Player* pPlayer = GetPlayerTargetOrSourceAndLog(pSource, pTarget);
            if (!pPlayer)
                break;

            pPlayer->ActivateTaxiPathTo(m_script->sendTaxiPath.taxiPathId);
            break;
        }
        case SCRIPT_COMMAND_TERMINATE_SCRIPT:               // 31
        {
            if (!pSource && (!pTarget && ((m_script->data_flags & SCRIPT_FLAG_BUDDY_BY_GUID) == 0)))
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call for nullptr, skipping.", m_table, m_script->id, m_script->command);
                return true;
            }

            bool result = false;
            bool terminationBuddyFound = false;
            if (m_script->terminateScript.npcOrGOEntry || m_script->terminateScript.poolId || (m_script->data_flags & (SCRIPT_FLAG_BUDDY_BY_GUID)))
            {
                WorldObject* terminationBuddy = nullptr;
                WorldObject* pSearcher = pSource ? pSource : pTarget;
                if (!pSearcher)
                {
                    sLog.outError("DB - SCRIPTS: Process table `%s` id %u, command %u no pSource provided", m_table, m_script->id, m_script->command);
                    return false;
                }

                if (pSearcher->GetTypeId() == TYPEID_PLAYER && pTarget && pTarget->GetTypeId() != TYPEID_PLAYER)
                    pSearcher = pTarget;

                if (m_script->data_flags & (SCRIPT_FLAG_BUDDY_BY_GUID))
                {
                    if (m_script->IsCreatureBuddy())
                    {
                        if (pTarget && pTarget->IsUnit() && pTarget->GetDbGuid() == m_script->searchRadiusOrGuid)
                            terminationBuddy = pTarget;
                    }
                    else
                    {
                        if (pTarget && pTarget->IsGameObject() && pTarget->GetDbGuid() == m_script->searchRadiusOrGuid)
                            terminationBuddy = pTarget;
                    }
                }
                else if (m_script->terminateScript.npcOrGOEntry)
                {
                    if (m_script->IsCreatureBuddy())
                    {
                        // npc entry is provided
                        Creature* creatureBuddy = nullptr;
                        MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck u_check(*pSearcher, m_script->terminateScript.npcOrGOEntry, true, false, m_script->terminateScript.searchDist, true);
                        MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creatureBuddy, u_check);
                        Cell::VisitGridObjects(pSearcher, searcher, m_script->terminateScript.searchDist);
                        terminationBuddy = creatureBuddy;
                    }
                    else
                    {
                        GameObject* goBuddy = nullptr;
                        MaNGOS::NearestGameObjectEntryInObjectRangeCheck u_check(*pSearcher, m_script->terminateScript.npcOrGOEntry, m_script->terminateScript.searchDist);
                        MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(goBuddy, u_check);
                        Cell::VisitGridObjects(pSearcher, searcher, m_script->terminateScript.searchDist);
                        terminationBuddy = goBuddy;
                    }
                }
                else
                {
                    // poolId is provided
                    // this is supposed to work only on situation where only one of the provided creature in pool can be alive at a time
                    PoolGroup<Creature> const& pool = sPoolMgr.GetPoolCreatures(m_script->terminateScript.poolId);

                    // fist try to find buddy npc in equal chanced list
                    auto equalChancedObjectList = pool.GetEqualChanced();
                    for (auto objItr : equalChancedObjectList)
                    {
                        CreatureData const* cData = sObjectMgr.GetCreatureData(objItr.guid);
                        if (Creature* buddy = m_map->GetCreature(cData->GetObjectGuid(objItr.guid)))
                        {
                            // buddy should be alive and in search dist range
                            if (buddy->IsAlive() &&
                                (!m_script->terminateScript.searchDist || pSearcher->IsWithinDist3d(buddy->GetPositionX(), buddy->GetPositionY(), buddy->GetPositionZ(), m_script->terminateScript.searchDist)))
                            {
                                terminationBuddy = buddy;
                                break;
                            }
                        }
                    }

                    if (!terminationBuddy)
                    {
                        // buddy was not found so try explicitly chanced list
                        auto explicitlyChancedObjectList = pool.GetExplicitlyChanced();
                        for (auto objItr : explicitlyChancedObjectList)
                        {
                            CreatureData const* cData = sObjectMgr.GetCreatureData(objItr.guid);
                            if (Creature* buddy = m_map->GetCreature(cData->GetObjectGuid(objItr.guid)))
                            {
                                // buddy should be alive and in search dist range
                                if (buddy->IsAlive() &&
                                    (!m_script->terminateScript.searchDist || pSearcher->IsWithinDist3d(buddy->GetPositionX(), buddy->GetPositionY(), buddy->GetPositionZ(), m_script->terminateScript.searchDist)))
                                {
                                    terminationBuddy = buddy;
                                    break;
                                }
                            }
                        }
                    }
                }

                terminationBuddyFound = terminationBuddy;
            }
            else
                terminationBuddyFound = buddyFound;

            if (!(m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL) && !terminationBuddyFound)
            {
                if (m_script->terminateScript.npcOrGOEntry)
                    DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u, terminate further steps of this script! (as searched npc entry(%u) was not found alive)", m_table, m_script->id, m_script->terminateScript.npcOrGOEntry);
                else
                    DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u, terminate further steps of this script! (as no npc in pool id(%u) was found alive)", m_table, m_script->id, m_script->terminateScript.poolId);
                result = true;
            }
            else if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL && terminationBuddyFound)
            {
                if (m_script->terminateScript.npcOrGOEntry)
                    DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u, terminate further steps of this script! (as searched npc entry(%u) was found alive)", m_table, m_script->id, m_script->terminateScript.npcOrGOEntry);
                else
                    DETAIL_FILTER_LOG(LOG_FILTER_DB_SCRIPT, "DB-SCRIPTS: Process table `%s` id %u, terminate further steps of this script! (as searched npc in pool id(%u) was found alive)", m_table, m_script->id, m_script->terminateScript.poolId);
                result = true;
            }

            if (result)                                    // Terminate further steps of this script
            {
                if (m_script->textId[0] && !LogIfNotCreature(pSource))
                {
                    Creature* cSource = static_cast<Creature*>(pSource);
                    if (cSource->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                        (static_cast<WaypointMovementGenerator<Creature>*>(cSource->GetMotionMaster()->top()))->AddToWaypointPauseTime(m_script->textId[0]);
                }

                return true;
            }

            break;
        }
        case SCRIPT_COMMAND_PAUSE_WAYPOINTS:                // 32
        {
            if (LogIfNotCreature(pSource))
                return false;
            if (m_script->pauseWaypoint.doPause)
                ((Creature*)pSource)->GetMotionMaster()->PauseWaypoints(0);
            else
                ((Creature*)pSource)->GetMotionMaster()->UnpauseWaypoints();
            break;
        }
        case SCRIPT_COMMAND_RESERVED_1:                     // 33
        {
            sLog.outError(" DB-SCRIPTS: Process table `%s` id %u, command %u not supported.", m_table, m_script->id, m_script->command);
            break;
        }
        case SCRIPT_COMMAND_TERMINATE_COND:
        {
            Player* player = nullptr;
            WorldObject* second = pSource;
            // First case: target is player
            if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
                player = static_cast<Player*>(pTarget);
            // Second case: source is player
            else if (pSource && pSource->GetTypeId() == TYPEID_PLAYER)
            {
                player = static_cast<Player*>(pSource);
                second = pTarget;
            }

            bool terminateResult;
            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                terminateResult = !sObjectMgr.IsConditionSatisfied(m_script->terminateCond.conditionId, player, m_map, second, CONDITION_FROM_DBSCRIPTS);
            else
                terminateResult = sObjectMgr.IsConditionSatisfied(m_script->terminateCond.conditionId, player, m_map, second, CONDITION_FROM_DBSCRIPTS);

            if (terminateResult && m_script->terminateCond.failQuest && player)
            {
                if (Group* group = player->GetGroup())
                {
                    for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                    {
                        Player* member = groupRef->getSource();
                        if (member->GetQuestStatus(m_script->terminateCond.failQuest) == QUEST_STATUS_INCOMPLETE)
                            member->FailQuest(m_script->terminateCond.failQuest);
                    }
                }
                else
                {
                    if (player->GetQuestStatus(m_script->terminateCond.failQuest) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(m_script->terminateCond.failQuest);
                }
            }
            return terminateResult;
        }
        case SCRIPT_COMMAND_SEND_AI_EVENT:                  // 35
        {
            if (LogIfNotCreature(pSource))
                return false;
            if (LogIfNotUnit(pTarget))
                break;

            // if radius is provided send AI event around
            if (m_script->sendAIEvent.radius)
                ((Creature*)pSource)->AI()->SendAIEventAround(AIEventType(m_script->sendAIEvent.eventType), (Unit*)pTarget, 0, float(m_script->sendAIEvent.radius), m_script->sendAIEvent.value);
            // else if no radius and target is creature send AI event to target
            else if (pTarget->GetTypeId() == TYPEID_UNIT)
                static_cast<Unit*>(pSource)->AI()->SendAIEvent(AIEventType(m_script->sendAIEvent.eventType), nullptr, (Unit*)pTarget, m_script->sendAIEvent.value);
            else if (pSource->IsCreature() && pTarget->IsPlayer())
                static_cast<Unit*>(pSource)->AI()->ReceiveAIEvent(AIEventType(m_script->sendAIEvent.eventType), (Unit*)pTarget, (Unit*)pTarget, m_script->sendAIEvent.value);
            else if (pSource->IsPlayer() && pTarget->IsCreature())
                static_cast<Unit*>(pTarget)->AI()->ReceiveAIEvent(AIEventType(m_script->sendAIEvent.eventType), (Unit*)pSource, (Unit*)pSource, m_script->sendAIEvent.value);
            break;
        }
        case SCRIPT_COMMAND_SET_FACING:                     // 36
        {
            if (LogIfNotCreature(pSource))
                return false;
            Creature* pCSource = static_cast<Creature*>(pSource);
            if (!pTarget)
            {
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command _SET_FACING (%u): No target found.", m_table, m_script->id, m_script->command);
                return false;
            }
            if (m_script->setFacing.resetFacing)
            {
                float x, y, z, o;
                if (pCSource->GetMotionMaster()->empty() || !pCSource->GetMotionMaster()->top()->GetResetPosition(*pCSource, x, y, z, o))
                    pCSource->GetRespawnCoord(x, y, z, &o);
                pCSource->SetFacingTo(o);
            }
            else
                pCSource->SetFacingToObject(pTarget);
            break;
        }
        case SCRIPT_COMMAND_MOVE_DYNAMIC:                   // 37
        {
            if (LogIfNotCreature(pSource))
                return false;
            if (!pTarget)
            {
                sLog.outDebug(" DB-SCRIPTS: Process table `%s` id %u, SCRIPT_COMMAND_MOVE_DYNAMIC called but target doesnt exist: skipping.", m_table, m_script->id);
                return false;
            }

            Creature* source = static_cast<Creature*>(pSource);
            if (source->IsInCombat())
            {
                sLog.outDebug(" DB-SCRIPTS: Process table `%s` id %u, SCRIPT_COMMAND_MOVE_DYNAMIC called for source guid %s but source is in combat and may lead to wrong behaviour: skipping.", m_table, m_script->id, pSource->GetGuidStr().c_str());
                break;
            }

            float x, y, z;
            if (m_script->moveDynamic.maxDist == 0)         // Move to pTarget
            {
                if (pTarget == source)
                {
                    sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, _MOVE_DYNAMIC called with maxDist == 0, but resultingSource == resultingTarget (== %s)", m_table, m_script->id, source->GetGuidStr().c_str());
                    break;
                }
                if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL) // no bounding radius
                {
                    pTarget->GetNearPoint2dAt(pTarget->GetPositionX(), pTarget->GetPositionY(), x, y, m_script->moveDynamic.fixedDist, pTarget->GetAngle(source));
                    if (source)
                        source->UpdateAllowedPositionZ(x, y, z, pTarget->GetMap()); // update to LOS height if available
                }
                else
                    pTarget->GetContactPoint(source, x, y, z, m_script->moveDynamic.fixedDist);
            }
            else                                            // Calculate position
            {
                float orientation;
                if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
                    orientation = source->GetOrientation() + m_script->o + 2 * M_PI_F;
                else
                    orientation = m_script->o;

                source->GetRandomPoint(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), m_script->moveDynamic.maxDist, x, y, z,
                                        m_script->moveDynamic.minDist, (orientation == 0.0f ? nullptr : &orientation));
                z = std::max(z, pTarget->GetPositionZ());
                source->UpdateAllowedPositionZ(x, y, z);
            }
            source->GetMotionMaster()->MovePoint(1, Position(x, y, z, 0.f), ForcedMovement(m_script->textId[0]), 0.f, true, pTarget ? pTarget->GetObjectGuid() : ObjectGuid(), m_script->textId[1]);
            break;
        }
        case SCRIPT_COMMAND_SEND_MAIL:                      // 38
        {
            if (LogIfNotPlayer(pTarget))
                return false;
            if (!m_script->sendMail.altSender && LogIfNotCreature(pSource))
                return false;

            MailSender sender;
            if (m_script->sendMail.altSender)
                sender = MailSender(MAIL_CREATURE, m_script->sendMail.altSender);
            else
                sender = MailSender(pSource);
            uint32 deliverDelay = m_script->textId[0] > 0 ? (uint32)m_script->textId[0] : 0;

            MailDraft(m_script->sendMail.mailTemplateId).SetMoney(m_script->sendMail.money).SendMailTo(static_cast<Player*>(pTarget), sender, MAIL_CHECK_MASK_HAS_BODY, deliverDelay);
            break;
        }
        case SCRIPT_COMMAND_SET_HOVER:                      // 39
        {
            if (LogIfNotCreature(pSource))
                break;

            // enable / disable the fly anim flag (wotlk+)
            /*
            if (m_script->data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)
            {
                if (m_script->fly.fly)
                    pSource->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
                else
                    pSource->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
            }
            */

            ((Creature*)pSource)->SetHover(m_script->fly.fly);
            break;
        }
        case SCRIPT_COMMAND_DESPAWN_GO:                     // 40
        {
            if (LogIfNotGameObject(pTarget))
                break;

            static_cast<GameObject*>(pTarget)->ForcedDespawn(m_script->despawnGo.despawnDelay);

            break;
        }
        case SCRIPT_COMMAND_RESPAWN:                        // 41
        {
            if (LogIfNotCreature(pTarget))
                break;

            ((Creature*)pTarget)->Respawn();
            break;
        }
        case SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS:            // 42
        {
            if (LogIfNotCreature(pSource))
                return false;

            Creature* pCSource = static_cast<Creature*>(pSource);
            // reset default
            if (m_script->setEquipment.resetDefault)
            {
                pCSource->LoadEquipment(pCSource->GetCreatureInfo()->EquipmentTemplateId, true);
                break;
            }

            // main hand
            if (m_script->textId[0] >= 0)
                pCSource->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, m_script->textId[0]);

            // off hand
            if (m_script->textId[1] >= 0)
                pCSource->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, m_script->textId[1]);

            // ranged
            if (m_script->textId[2] >= 0)
                pCSource->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, m_script->textId[2]);
            break;
        }
        case SCRIPT_COMMAND_RESET_GO:                       // 43
        {
            if (LogIfNotGameObject(pTarget))
                break;

            GameObject* pGoTarget = static_cast<GameObject*>(pTarget);
            switch (pGoTarget->GetGoType())
            {
                case GAMEOBJECT_TYPE_DOOR:
                case GAMEOBJECT_TYPE_BUTTON:
                    pGoTarget->ResetDoorOrButton();
                    break;
                default:
                    sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed for gameobject(buddyEntry: %u). Gameobject is not a door or button", m_table, m_script->id, m_script->command, m_script->buddyEntry);
                    break;
            }
            break;
        }
        case SCRIPT_COMMAND_UPDATE_TEMPLATE:                // 44
        {
            if (LogIfNotCreature(pSource))
                return false;

            Creature* pCSource = static_cast<Creature*>(pSource);

            if (pCSource->GetEntry() != m_script->updateTemplate.newTemplate)
                pCSource->UpdateEntry(m_script->updateTemplate.newTemplate);
            else
                sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. Source already has specified creature entry.", m_table, m_script->id, m_script->command);
            break;
        }
        case SCRIPT_COMMAND_START_RELAY_SCRIPT:             // 45
        {
            uint32 chosenId;
            if (m_script->relayScript.templateId)
                chosenId = sScriptMgr.GetRandomRelayDbscriptFromTemplate(m_script->relayScript.templateId);
            else
                chosenId = m_script->relayScript.relayId;

            if (chosenId)
                m_map->ScriptsStart(SCRIPT_TYPE_RELAY, chosenId, pSource, pTarget);
            break;
        }
        case SCRIPT_COMMAND_CAST_CUSTOM_SPELL:              // 46
        {
            if (LogIfNotUnit(pTarget))
                break;
            if (LogIfNotUnit(pSource))
                break;

            ((Unit*)pSource)->CastCustomSpell((Unit*)pTarget, m_script->castCustomSpell.spellId, &m_script->textId[0], &m_script->textId[1], &m_script->textId[2], m_script->castCustomSpell.castFlags);
            break;
        }
        case SCRIPT_COMMAND_INTERRUPT_SPELL:                // 47
        {
            if (LogIfNotUnit(pSource))
                return false;

            Unit* unitSource = static_cast<Unit*>(pSource);

            unitSource->InterruptSpell((CurrentSpellTypes)m_script->interruptSpell.currentSpellType);
            break;
        }
        case SCRIPT_COMMAND_MODIFY_UNIT_FLAGS:              // 48
        {
            if (LogIfNotCreature(pSource))
                break;

            // Remove Flags
            if (m_script->unitFlag.change_flag == 0)
                pSource->RemoveFlag(UNIT_FIELD_FLAGS, m_script->unitFlag.flag);
            // Add Flags
            else if (m_script->unitFlag.change_flag == 1)
                pSource->SetFlag(UNIT_FIELD_FLAGS, m_script->unitFlag.flag);
            // Toggle Flags
            else if (m_script->unitFlag.change_flag == 2)
            {
                if (pSource->HasFlag(UNIT_FIELD_FLAGS, m_script->unitFlag.flag))
                    pSource->RemoveFlag(UNIT_FIELD_FLAGS, m_script->unitFlag.flag);
                else
                    pSource->SetFlag(UNIT_FIELD_FLAGS, m_script->unitFlag.flag);
            }
            else
                sLog.outErrorDb(" DB-SCRIPTS: Unexpected value %u used for script id %u, command %u.", m_script->unitFlag.flag, m_script->id, m_script->command);

            break;
        }
        case SCRIPT_COMMAND_SET_DATA_64:                    // 49
        {
            InstanceData* data = pSource->GetInstanceData();
            if (data)
                data->SetData64(m_script->setData64.param1, m_script->setData64.param2);
            break;
        }
        case SCRIPT_COMMAND_ZONE_PULSE:                     // 50
        {
            if (LogIfNotCreature(pSource))
                break;

            Creature* creature = static_cast<Creature*>(pSource);
            creature->SetInCombatWithZone();
            creature->AI()->AttackClosestEnemy();
            break;
        }
        case SCRIPT_COMMAND_SPAWN_GROUP:                    // 51
        {
            switch (m_script->formationData.command)
            {
                case 150:                         // Create formation
                {
                    if (!pSource && !pTarget)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call with no target, skipping.", m_table, m_script->id, m_script->command);
                        return false;
                    }

                    WorldObject* target = nullptr;

                    target = pTarget ? pTarget : pSource;

                    if (LogIfNotCreature(target))
                        return false;

                    Creature* leader = static_cast<Creature*>(target);

                    CreatureGroup* leaderGroup = leader->GetCreatureGroup();
                    CreatureGroup* targetGroup = nullptr;

                    if (!m_script->formationData.data1)
                    {
                        if (!leaderGroup)
                        {
                            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation create(1) failed. Target group(%u) not found!",
                                m_table, m_script->id, m_script->command, m_script->formationData.data1);
                            break;
                        }
                        targetGroup = leaderGroup;
                    }
                    else
                    {
                        auto sgData = leader->GetMap()->GetSpawnManager().GetSpawnGroup(m_script->formationData.data1);
                        if (!sgData || !sgData->GetCreatureGroup())
                        {
                            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation create(1) failed. Target group(%u) not found!",
                                m_table, m_script->id, m_script->command, m_script->formationData.data1);
                            break;
                        }
                        targetGroup = sgData->GetCreatureGroup();
                    }

                    if (targetGroup->GetFormationData())
                    {
                        // better fail here, the user have to remove the previous formation first
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation create(1) failed. Target group(%u) have already a formation!",
                            m_table, m_script->id, m_script->command, m_script->formationData.data1);
                        break;
                    }

                    FormationEntrySPtr fEntry = std::make_shared<FormationEntry>();
                    fEntry->GroupId = targetGroup->GetGroupId();
                    fEntry->Type = static_cast<SpawnGroupFormationType>(m_script->textId[0]);
                    fEntry->Spread = m_script->x;
                    fEntry->Options = m_script->textId[1];
                    fEntry->MovementType = 0;
                    fEntry->MovementIdOrWander = 0;
                    fEntry->Comment = "Dynamically created formation!";
                    fEntry->IsDynamic = true;

                    targetGroup->SetFormationData(fEntry);
                    break;
                }

                case 151:                         // Remove formation
                {
                    if (!pSource && !pTarget)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u call with no target, skipping.", m_table, m_script->id, m_script->command);
                        return false;
                    }

                    WorldObject* target = nullptr;

                    target = pTarget ? pTarget : pSource;

                    if (LogIfNotCreature(target))
                        return false;

                    Creature* leader = static_cast<Creature*>(target);

                    CreatureGroup* leaderGroup = leader->GetCreatureGroup();
                    CreatureGroup* targetGroup = nullptr;

                    if (!m_script->formationData.data1)
                    {
                        if (!leaderGroup)
                        {
                            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation remove(2) failed. Target group(%u) not found!",
                                m_table, m_script->id, m_script->command, m_script->formationData.data1);
                            break;
                        }
                        targetGroup = leaderGroup;
                    }
                    else
                    {
                        auto sgData = leader->GetMap()->GetSpawnManager().GetSpawnGroup(m_script->formationData.data1);
                        if (!sgData || !sgData->GetCreatureGroup())
                        {
                            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation create(1) failed. Target group(%u) not found!",
                                m_table, m_script->id, m_script->command, m_script->formationData.data1);
                            break;
                        }
                        targetGroup = sgData->GetCreatureGroup();
                    }

                    if (!targetGroup->GetFormationData())
                    {
                        // better fail with message for now, maybe this can be removed in some futur
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` script id %u, command %u and subcommand formation remove(2) failed. Target group(%u) have already a formation!",
                            m_table, m_script->id, m_script->command, m_script->formationData.data1);
                        break;
                    }

                    targetGroup->SetFormationData(nullptr);
                    break;
                }

                //                 case 3: // add creature to the formation
                //                 {
                //                     if (LogIfNotCreature(pSource))
                //                         return false;
                // 
                //                     if (!leaderFormation)
                //                     {
                //                         sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. %s is not in formation!",
                //                             m_table, m_script->id, m_script->command, leader->GetGuidStr().c_str());
                //                         break;
                //                     }
                //                     leaderFormation->Add(static_cast<Creature*>(pSource));
                //                     break;
                //                 }
                //                 case 4: // remove creature from the formation
                //                 {
                //                     if (LogIfNotCreature(pSource))
                //                         return false;
                // 
                //                     if (!leaderFormation)
                //                     {
                //                         sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. %s is not in formation!",
                //                             m_table, m_script->id, m_script->command, leader->GetGuidStr().c_str());
                //                         break;
                //                     }
                //                     leaderFormation->Remove(static_cast<Creature*>(pSource));
                //                     break;
                //                 }
                case 100: // switch formation shape
                {
                    if (LogIfNotCreature(pTarget))
                        return false;

                    Creature* leader = static_cast<Creature*>(pTarget);

                    FormationSlotDataSPtr leaderSlot = leader->GetFormationSlot();
                    FormationData* leaderFormation = nullptr;
                    if (leaderSlot)
                        leaderFormation = leaderSlot->GetFormationData();

                    if (!leaderFormation)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. %s is not in formation!",
                            m_table, m_script->id, m_script->command, leader->GetGuidStr().c_str());
                        break;
                    }

                    if (m_script->formationData.data1 < static_cast<uint32>(SpawnGroupFormationType::SPAWN_GROUP_FORMATION_TYPE_COUNT))
                    {
                        if (!leaderFormation->SwitchFormation(static_cast<SpawnGroupFormationType>(m_script->formationData.data1)))
                            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u subcommand changeShape(%u) failed.",
                                m_table, m_script->id, m_script->command, m_script->formationData.command);
                    }
                    else
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u subcommand switchShape(%u) failed, wrong shape id(%u).",
                            m_table, m_script->id, m_script->command, m_script->formationData.command, m_script->formationData.data1);
                        return true;
                    }
                    break;
                }
                case 101:  // set formation spread
                {
                    if (LogIfNotCreature(pTarget))
                        return false;

                    Creature* leader = static_cast<Creature*>(pTarget);

                    FormationSlotDataSPtr leaderSlot = leader->GetFormationSlot();
                    FormationData* leaderFormation = nullptr;
                    if (leaderSlot)
                        leaderFormation = leaderSlot->GetFormationData();

                    if (!leaderFormation)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. %s is not in formation!",
                            m_table, m_script->id, m_script->command, leader->GetGuidStr().c_str());
                        break;
                    }

                    if (m_script->x <= 15)
                    {
                        leaderFormation->SetSpread(m_script->x);
                    }
                    else
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u subcommand changeSpread(%u) failed, wrong shape id(%f).",
                            m_table, m_script->id, m_script->command, m_script->formationData.command, m_script->x);
                        return true;
                    }
                    break;
                }
                case 102:  // set formation options
                {
                    if (LogIfNotCreature(pTarget))
                        return false;

                    Creature* leader = static_cast<Creature*>(pTarget);

                    FormationSlotDataSPtr leaderSlot = leader->GetFormationSlot();
                    FormationData* leaderFormation = nullptr;
                    if (leaderSlot)
                        leaderFormation = leaderSlot->GetFormationData();

                    if (!leaderFormation)
                    {
                        sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. %s is not in formation!",
                            m_table, m_script->id, m_script->command, leader->GetGuidStr().c_str());
                        break;
                    }

                    leaderFormation->SetOptions(m_script->formationData.data1);
                    break;
                }

                default:
                    sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u failed. Invalid value for formation command(%u)!",
                        m_table, m_script->id, m_script->command, m_script->formationData.command);
                    break;
            }

            break;
        }
        case SCRIPT_COMMAND_SET_GOSSIP_MENU:                // 52
        {
            if (LogIfNotCreature(pTarget))
                break;

            static_cast<Creature*>(pTarget)->SetDefaultGossipMenuId(m_script->setGossipMenu.gossipMenuId);
            break;
        }
        case SCRIPT_COMMAND_SET_WORLDSTATE:                 // 53
        {
            m_map->GetVariableManager().SetVariable(m_script->textId[0], m_script->textId[1]);
            break;
        }
        case SCRIPT_COMMAND_SET_SHEATHE:
        {
            if (LogIfNotUnit(pSource))
                break;

            static_cast<Unit*>(pSource)->SetSheath(SheathState(m_script->setSheathe.sheatheState));
            break;
        }
        case SCRIPT_COMMAND_SET_STRING_ID:
        {
            pSource->SetStringId(m_script->stringId.stringId, m_script->stringId.apply);
            break;
        }
        default:
            sLog.outErrorDb(" DB-SCRIPTS: Process table `%s` id %u, command %u unknown command used.",
                m_table, m_script->id, m_script->command);
            break;
        // ordered like this to prevent merge errors
        case SCRIPT_COMMAND_MEETINGSTONE:                   // 200
        {
            if (LogIfNotPlayer(pTarget))
                return false;

            Player* player = nullptr;

            if (pTarget && pTarget->IsPlayer())
                player = static_cast<Player*>(pTarget);
            else if (pSource && pSource->IsPlayer())
                player = static_cast<Player*>(pSource);

            // only Player
            if (!player)
            {
                sLog.outError("SCRIPT_COMMAND_MEETINGSTONE (script id %u) call for non-player, skipping.", m_script->id);
                break;
            }

            sLFGMgr.AddToQueue(player, m_script->meetingstone.areaId);
            break;
        }
    }

    return false;
}

int32 ScriptMgr::GetRandomScriptTemplateId(uint32 id, uint8 templateType)
{
    if (m_scriptTemplates[templateType][id].empty())
        return 0;

    auto& randomChanced = m_scriptTemplatesExplicitlyChanced[templateType][id];
    if (!randomChanced.empty())
    {
        float random = (float)rand_chance();
        uint32 cumulativeChance = 0;
        for (auto& data : randomChanced)
        {
            cumulativeChance += data.second;
            if (cumulativeChance >= random)
                return data.first;
        }
    }

    auto& equallyChanced = m_scriptTemplatesEquallyChanced[templateType][id];
    if (!equallyChanced.empty())
    {
        uint32 random = urand(0, equallyChanced.size() - 1);
        return equallyChanced[random].first;
    }
    return 0;
}

int32 ScriptMgr::GetRandomScriptStringFromTemplate(uint32 id)
{
    return GetRandomScriptTemplateId(id, STRING_TEMPLATE);
}

int32 ScriptMgr::GetRandomRelayDbscriptFromTemplate(uint32 id)
{
    return GetRandomScriptTemplateId(id, RELAY_TEMPLATE);
}

// /////////////////////////////////////////////////////////
//              Scripting Library Hooks
// /////////////////////////////////////////////////////////

void ScriptMgr::CollectPossibleEventIds(std::set<uint32>& eventIds)
{
    // Load all possible script entries from gameobjects
    for (SQLStorageBase::SQLSIterator<GameObjectInfo> itr = sGOStorage.getDataBegin<GameObjectInfo>(); itr < sGOStorage.getDataEnd<GameObjectInfo>(); ++itr)
    {
        switch (itr->type)
        {
            case GAMEOBJECT_TYPE_GOOBER:
                eventIds.insert(itr->goober.eventId);
                break;
            case GAMEOBJECT_TYPE_CHEST:
                eventIds.insert(itr->chest.eventId);
                break;
            case GAMEOBJECT_TYPE_CAMERA:
                eventIds.insert(itr->camera.eventID);
                break;
            case GAMEOBJECT_TYPE_CAPTURE_POINT:
                eventIds.insert(itr->capturePoint.neutralEventID1);
                eventIds.insert(itr->capturePoint.neutralEventID2);
                eventIds.insert(itr->capturePoint.contestedEventID1);
                eventIds.insert(itr->capturePoint.contestedEventID2);
                eventIds.insert(itr->capturePoint.progressEventID1);
                eventIds.insert(itr->capturePoint.progressEventID2);
                eventIds.insert(itr->capturePoint.winEventID1);
                eventIds.insert(itr->capturePoint.winEventID2);
                break;
            default:
                break;
        }
    }

    // Load all possible script entries from spells
    for (uint32 i = 1; i < sSpellTemplate.GetMaxEntry(); ++i)
    {
        SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(i);
        if (spell)
        {
            for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
            {
                if (spell->Effect[j] == SPELL_EFFECT_SEND_EVENT)
                {
                    if (spell->EffectMiscValue[j])
                        eventIds.insert(spell->EffectMiscValue[j]);
                }
            }
        }
    }
}

std::shared_ptr<ScriptMapMapName> ScriptMgr::GetScriptMap(ScriptMapType scriptMapType)
{
    return m_scriptMaps[scriptMapType];
}

// Starters for events
bool StartEvents_Event(Map* map, uint32 id, Object* source, Object* target, bool isStart/*=true*/)
{
    MANGOS_ASSERT(source);

    // Handle SD2 script
    if (sScriptDevAIMgr.OnProcessEvent(id, source, target, isStart))
        return true;

    // Handle PvP Calls
    if (source->IsGameObject() || source->IsUnit())
    {
        BattleGround* bg = nullptr;
        OutdoorPvP* opvp = nullptr;
        uint32 zoneId = 0;
        if (source->IsPlayer())
            zoneId = static_cast<Player*>(source)->GetCachedZoneId();
        else
            zoneId = static_cast<WorldObject*>(source)->GetZoneId();

        if (map->IsBattleGround())
            bg = static_cast<BattleGroundMap*>(map)->GetBG();
        else                                            // Use the go, because GOs don't move
            opvp = sOutdoorPvPMgr.GetScript(zoneId);

        if (bg && bg->HandleEvent(id, source, target))
            return true;

        if (opvp && opvp->HandleEvent(id, source, target))
            return true;
    }

    Map::ScriptExecutionParam execParam = Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE_TARGET;
    if (source->isType(TYPEMASK_CREATURE_OR_GAMEOBJECT))
        execParam = Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE;
    else if (target && target->isType(TYPEMASK_CREATURE_OR_GAMEOBJECT))
        execParam = Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_TARGET;

    return map->ScriptsStart(SCRIPT_TYPE_EVENT, id, source, target, execParam);
}

bool ScriptMgr::ExistsStringId(uint32 stringId)
{
    return m_stringIds->find(stringId) != m_stringIds->end();
}
