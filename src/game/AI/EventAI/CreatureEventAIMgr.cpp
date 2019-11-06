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
#include "Database/DatabaseEnv.h"
#include "Server/SQLStorages.h"
#include "CreatureEventAI.h"
#include "CreatureEventAIMgr.h"
#include "Globals/ObjectMgr.h"
#include "ProgressBar.h"
#include "Policies/Singleton.h"
#include "Maps/GridDefines.h"
#include "Spells/SpellMgr.h"
#include "World/World.h"
#include "DBScripts/ScriptMgr.h"

INSTANTIATE_SINGLETON_1(CreatureEventAIMgr);

// -------------------
void CreatureEventAIMgr::LoadCreatureEventAI_Texts(bool check_entry_use)
{
    // Load EventAI Text
    sObjectMgr.LoadMangosStrings(WorldDatabase, "creature_ai_texts", MIN_CREATURE_AI_TEXT_STRING_ID, MAX_CREATURE_AI_TEXT_STRING_ID, true);

    if (check_entry_use)
        CheckUnusedAITexts();
}

void CreatureEventAIMgr::CheckUnusedAITexts()
{
    if (m_usedTextsAmount == sObjectMgr.GetLoadedStringsCount(MIN_CREATURE_AI_TEXT_STRING_ID))
        return;

    sLog.outString("Checking EventAI for unused texts, this might take a while");

    std::set<int32> idx_set;
    for (int32 i = MAX_CREATURE_AI_TEXT_STRING_ID + 1; i <= MIN_CREATURE_AI_TEXT_STRING_ID; ++i)
        if (sObjectMgr.GetMangosStringLocale(i))
            idx_set.insert(i);

    for (CreatureEventAI_Event_Map::const_iterator itr = m_CreatureEventAI_Event_Map.begin(); itr != m_CreatureEventAI_Event_Map.end(); ++itr)
    {
        for (const auto& event : itr->second)
        {
            for (auto action : event.action)
            {
                switch (action.type)
                {
                    case ACTION_T_TEXT:
                    case ACTION_T_CHANCED_TEXT:
                    {
                        // ACTION_T_CHANCED_TEXT contains a chance value in first param
                        int k = action.type == ACTION_T_TEXT ? 0 : 1;
                        for (; k < 3; ++k)
                            if (action.text.TextId[k])
                                idx_set.erase(action.text.TextId[k]);
                        break;
                    }
                    case ACTION_T_TEXT_NEW:
                        if (action.textNew.textId)
                            idx_set.erase(action.textNew.textId);
                        break;
                    default: break;
                }
            }
        }
    }

    sScriptMgr.CheckRandomStringTemplates(idx_set);

    for (int32 itr : idx_set)
    sLog.outErrorEventAI("Entry %i in table `creature_ai_texts` but not used in EventAI scripts.", itr);
}

// -------------------
void CreatureEventAIMgr::LoadCreatureEventAI_Summons(bool check_entry_use)
{
    // Drop Existing EventSummon Map
    m_CreatureEventAI_Summon_Map.clear();

    // Gather additional data for EventAI
    QueryResult* result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, spawntimesecs FROM creature_ai_summons");
    if (result)
    {
        BarGoLink bar(result->GetRowCount());
        uint32 Count = 0;

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            CreatureEventAI_Summon temp;

            temp.id             = fields[0].GetUInt32();
            temp.position_x     = fields[1].GetFloat();
            temp.position_y     = fields[2].GetFloat();
            temp.position_z     = fields[3].GetFloat();
            temp.orientation    = fields[4].GetFloat();
            temp.SpawnTimeSecs  = fields[5].GetUInt32();

            if (!MaNGOS::IsValidMapCoord(temp.position_x, temp.position_y, temp.position_z, temp.orientation))
            {
                sLog.outErrorEventAI("Summon id %u have wrong coordinates (%f, %f, %f, %f), skipping.", temp.id, temp.position_x, temp.position_y, temp.position_z, temp.orientation);
                continue;
            }

            // Add to map
            m_CreatureEventAI_Summon_Map[temp.id] = temp;
            ++Count;
        }
        while (result->NextRow());

        delete result;

        if (check_entry_use)
            CheckUnusedAISummons();

        sLog.outString(">> Loaded %u CreatureEventAI summon definitions", Count);
        sLog.outString();
    }
    else
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 CreatureEventAI Summon definitions. DB table `creature_ai_summons` is empty.");
        sLog.outString();
    }
}

void CreatureEventAIMgr::CheckUnusedAISummons()
{
    std::set<int32> idx_set;
    // check not used strings this is negative range
    for (CreatureEventAI_Summon_Map::const_iterator itr = m_CreatureEventAI_Summon_Map.begin(); itr != m_CreatureEventAI_Summon_Map.end(); ++itr)
        idx_set.insert(itr->first);

    for (CreatureEventAI_Event_Map::const_iterator itr = m_CreatureEventAI_Event_Map.begin(); itr != m_CreatureEventAI_Event_Map.end(); ++itr)
    {
        for (const auto& event : itr->second)
        {
            for (auto action : event.action)
            {
                switch (action.type)
                {
                    case ACTION_T_SUMMON_ID:
                    {
                        if (action.summon_id.spawnId)
                            idx_set.erase(action.summon_id.spawnId);
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    for (int32 itr : idx_set)
    sLog.outErrorEventAI("Entry %i in table `creature_ai_summons` but not used in EventAI scripts.", itr);
}

/// Helper function to check if a target-type is suitable for the event-type
bool IsValidTargetType(EventAI_Type eventType, EventAI_ActionType actionType, uint32 targetType, uint32 eventId, uint8 action)
{
    switch (targetType)
    {
        case TARGET_T_SELF:
            if (actionType == ACTION_T_QUEST_EVENT || actionType == ACTION_T_CAST_EVENT || actionType == ACTION_T_QUEST_EVENT_ALL || actionType == ACTION_T_KILLED_MONSTER)
            {
                sLog.outErrorEventAI("Event %u Action%u uses incorrect Target type %u for event-type %u (must target player)", eventId, action, targetType, eventType);
                return false;
            }
            return true;                                    // Can always be used
        case TARGET_T_HOSTILE_RANDOM:
        case TARGET_T_HOSTILE_RANDOM_NOT_TOP:
            if (actionType == ACTION_T_QUEST_EVENT || actionType == ACTION_T_CAST_EVENT || actionType == ACTION_T_QUEST_EVENT_ALL || actionType == ACTION_T_KILLED_MONSTER)
                sLog.outErrorEventAI("Event %u Action%u uses LIKELY bad Target type %u for event-type %u (must target player)", eventId, action, targetType, eventType);
        // no break, check if valid at all
        case TARGET_T_HOSTILE:
        case TARGET_T_HOSTILE_SECOND_AGGRO:
        case TARGET_T_HOSTILE_LAST_AGGRO:
        case TARGET_T_HOSTILE_RANDOM_PLAYER:
        case TARGET_T_HOSTILE_RANDOM_NOT_TOP_PLAYER:
            switch (eventType)
            {
                case EVENT_T_TIMER_OOC:
                case EVENT_T_OOC_LOS:
                case EVENT_T_REACHED_HOME:
                    sLog.outErrorEventAI("Event %u Action%u uses incorrect Target type %u for event-type %u (cannot be used OOC)", eventId, action, targetType, eventType);
                    return false;
                case EVENT_T_TIMER_GENERIC:
                    sLog.outErrorEventAI("Event %u Action%u uses LIKELY incorrect Target type %u for event-type %u (cannot be used OOC)", eventId, action, targetType, eventType);
                    return true;                            // Does not need to be an error
                default:
                    return true;
            }
        case TARGET_T_ACTION_INVOKER:                       // Unit who caused this Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
        case TARGET_T_ACTION_INVOKER_OWNER:                 // Unit who is responsible for Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
            switch (eventType)
            {
                case EVENT_T_AGGRO:
                case EVENT_T_KILL:
                case EVENT_T_DEATH:
                case EVENT_T_SPELLHIT:
                case EVENT_T_OOC_LOS:
                case EVENT_T_FRIENDLY_HP:
                case EVENT_T_FRIENDLY_IS_CC:
                case EVENT_T_FRIENDLY_MISSING_BUFF:
                case EVENT_T_RECEIVE_EMOTE:
                case EVENT_T_RECEIVE_AI_EVENT:
                case EVENT_T_SPELLHIT_TARGET:
                    return true;
                default:
                    sLog.outErrorEventAI("Event %u Action%u uses incorrect Target type %u for event-type %u", eventId, action, targetType, eventType);
                    return false;
            }
        case TARGET_T_EVENT_SENDER:                         // Unit who sent an AIEvent that was received with EVENT_T_RECEIVE_AI_EVENT
            if (eventType != EVENT_T_RECEIVE_AI_EVENT)
            {
                sLog.outErrorEventAI("Event %u Action%u uses incorrect Target type %u for event-type %u", eventId, action, targetType, eventType);
                return false;
            }
            return true;
        case TARGET_T_SPAWNER:
        case TARGET_T_EVENT_SPECIFIC:
        case TARGET_T_PLAYER_INVOKER:
        case TARGET_T_PLAYER_TAPPED:
        case TARGET_T_NONE:
        case TARGET_T_HOSTILE_RANDOM_MANA:
        case TARGET_T_NEAREST_AOE_TARGET:
        case TARGET_T_HOSTILE_FARTHEST_AWAY:
            return true;
        default:
            sLog.outErrorEventAI("Event %u Action%u uses incorrect Target type", eventId, action);
            return false;
    }
}

// -------------------
void CreatureEventAIMgr::LoadCreatureEventAI_Scripts()
{
    // Drop Existing EventAI List
    m_CreatureEventAI_Event_Map.clear();
    std::set<int32> usedTextIds;

    // Gather event data
    QueryResult* result = WorldDatabase.Query("SELECT id, creature_id, event_type, event_inverse_phase_mask, event_chance, event_flags, "
                          "event_param1, event_param2, event_param3, event_param4, event_param5, event_param6, "
                          "action1_type, action1_param1, action1_param2, action1_param3, "
                          "action2_type, action2_param1, action2_param2, action2_param3, "
                          "action3_type, action3_param1, action3_param2, action3_param3 "
                          "FROM creature_ai_scripts ORDER BY id");
    if (result)
    {
        BarGoLink bar(result->GetRowCount());
        uint32 Count = 0;

        do
        {
            bar.step();
            Field* fields = result->Fetch();

            CreatureEventAI_Event temp;
            temp.event_id = EventAI_Type(fields[0].GetUInt32());
            uint32 i = temp.event_id;

            temp.creature_id = fields[1].GetUInt32();
            uint32 creature_id = temp.creature_id;

            uint32 e_type = fields[2].GetUInt32();
            // Report any errors in event
            if (e_type >= EVENT_T_END)
            {
                sLog.outErrorEventAI("Event %u have wrong type (%u), skipping.", i, e_type);
                continue;
            }
            temp.event_type = EventAI_Type(e_type);

            temp.event_inverse_phase_mask = fields[3].GetUInt32();
            temp.event_chance = fields[4].GetUInt8();
            temp.event_flags  = fields[5].GetUInt32();
            for (uint32 i = 0; i < 6; ++i)
                temp.raw.params[i] = fields[6 + i].GetUInt32();

            // Creature does not exist in database
            if (!sCreatureStorage.LookupEntry<CreatureInfo>(temp.creature_id))
            {
                sLog.outErrorEventAI("Event %u has script for non-existing creature entry (%u), skipping.", i, temp.creature_id);
                continue;
            }

            // No chance of this event occuring
            if (temp.event_chance == 0)
                sLog.outErrorEventAI("Event %u has 0 percent chance. Event will never trigger!", i);
            // Chance above 100, force it to be 100
            else if (temp.event_chance > 100)
            {
                sLog.outErrorEventAI("Creature %u are using event %u with more than 100 percent chance. Adjusting to 100 percent.", temp.creature_id, i);
                temp.event_chance = 100;
            }

            // Individual event checks
            switch (temp.event_type)
            {
                case EVENT_T_TIMER_IN_COMBAT:
                case EVENT_T_TIMER_OOC:
                case EVENT_T_TIMER_GENERIC:
                    if (temp.timer.initialMax < temp.timer.initialMin)
                        sLog.outErrorEventAI("Creature %u are using timed event(%u) with param2 < param1 (InitialMax < InitialMin). Event will never repeat.", temp.creature_id, i);
                    if (temp.timer.repeatMax < temp.timer.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_HP:
                case EVENT_T_MANA:
                case EVENT_T_TARGET_HP:
                case EVENT_T_TARGET_MANA:
                case EVENT_T_ENERGY:
                    if (temp.percent_range.percentMax > 100)
                        sLog.outErrorEventAI("Creature %u are using percentage event(%u) with param2 (MinPercent) > 100. Event will never trigger! ", temp.creature_id, i);

                    if (temp.percent_range.percentMax <= temp.percent_range.percentMin)
                        sLog.outErrorEventAI("Creature %u are using percentage event(%u) with param1 <= param2 (MaxPercent <= MinPercent). Event will never trigger! ", temp.creature_id, i);

                    if (temp.event_flags & EFLAG_REPEATABLE && !temp.percent_range.repeatMin && !temp.percent_range.repeatMax)
                    {
                        sLog.outErrorEventAI("Creature %u has param3 and param4=0 (RepeatMin/RepeatMax) but cannot be repeatable without timers. Removing EFLAG_REPEATABLE for event %u.", temp.creature_id, i);
                        temp.event_flags &= ~EFLAG_REPEATABLE;
                    }
                    break;
                case EVENT_T_SPELLHIT:
                    if (temp.spell_hit.spellId)
                    {
                        SpellEntry const* pSpell = sSpellTemplate.LookupEntry<SpellEntry>(temp.spell_hit.spellId);
                        if (!pSpell)
                        {
                            sLog.outErrorEventAI("Creature %u has nonexistent SpellID(%u) defined in event %u.", temp.creature_id, temp.spell_hit.spellId, i);
                            continue;
                        }

                        if ((temp.spell_hit.schoolMask & GetSchoolMask(pSpell->School)) != GetSchoolMask(pSpell->School))
                            sLog.outErrorEventAI("Creature %u has param1(spellId %u) but param2 is not -1 and not equal to spell's school mask. Event %u can never trigger.", temp.creature_id, temp.spell_hit.schoolMask, i);
                    }

                    if (!temp.spell_hit.schoolMask)
                        sLog.outErrorEventAI("Creature %u is using invalid SpellSchoolMask(%u) defined in event %u.", temp.creature_id, temp.spell_hit.schoolMask, i);

                    if (temp.spell_hit.repeatMax < temp.spell_hit.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_RANGE:
                    if (temp.range.maxDist < temp.range.minDist)
                        sLog.outErrorEventAI("Creature %u are using event(%u) with param2 < param1 (MaxDist < MinDist). Event will never repeat.", temp.creature_id, i);
                    if (temp.range.repeatMax < temp.range.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_OOC_LOS:
                    if (temp.ooc_los.conditionId && !sConditionStorage.LookupEntry<PlayerCondition>(temp.ooc_los.conditionId))
                    {
                        sLog.outErrorDb("Creature %u has `ConditionId` = %u but does not exist. Setting ConditionId to 0 for event %u.", temp.creature_id, temp.ooc_los.conditionId, i);
                        temp.ooc_los.conditionId = 0;
                    }

                    if (temp.ooc_los.repeatMax < temp.ooc_los.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_SPAWNED:
                    switch (temp.spawned.condition)
                    {
                        case SPAWNED_EVENT_ALWAY:
                            break;
                        case SPAWNED_EVENT_MAP:
                            if (!sMapStore.LookupEntry(temp.spawned.conditionValue1))
                                sLog.outErrorEventAI("Creature %u are using spawned event(%u) with param1 = %u 'map specific' but map (param2: %u) does not exist. Event will never repeat.", temp.creature_id, i, temp.spawned.condition, temp.spawned.conditionValue1);
                            break;
                        case SPAWNED_EVENT_ZONE:
                            if (!GetAreaEntryByAreaID(temp.spawned.conditionValue1))
                                sLog.outErrorEventAI("Creature %u are using spawned event(%u) with param1 = %u 'area specific' but area (param2: %u) does not exist. Event will never repeat.", temp.creature_id, i, temp.spawned.condition, temp.spawned.conditionValue1);
                            break;
                        default:
                            sLog.outErrorEventAI("Creature %u are using invalid spawned event %u mode (%u) in param1", temp.creature_id, i, temp.spawned.condition);
                            break;
                    }
                    break;
                case EVENT_T_FRIENDLY_HP:
                    if (temp.friendly_hp.repeatMax < temp.friendly_hp.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_FRIENDLY_IS_CC:
                    if (temp.friendly_is_cc.repeatMax < temp.friendly_is_cc.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_FRIENDLY_MISSING_BUFF:
                {
                    SpellEntry const* pSpell = sSpellTemplate.LookupEntry<SpellEntry>(temp.friendly_buff.spellId);
                    if (!pSpell)
                    {
                        sLog.outErrorEventAI("Creature %u has nonexistent SpellID(%u) defined in event %u.", temp.creature_id, temp.friendly_buff.spellId, i);
                        continue;
                    }
                    if (temp.friendly_buff.radius <= 0)
                    {
                        sLog.outErrorEventAI("Creature %u has wrong radius (%u) for EVENT_T_FRIENDLY_MISSING_BUFF defined in event %u.", temp.creature_id, temp.friendly_buff.radius, i);
                        continue;
                    }
                    if (temp.friendly_buff.repeatMax < temp.friendly_buff.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                }
                case EVENT_T_KILL:
                    if (temp.kill.repeatMax < temp.kill.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using event(%u) with param2 < param1 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_TARGET_CASTING:
                    if (temp.target_casting.repeatMax < temp.target_casting.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using event(%u) with param2 < param1 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_SUMMONED_UNIT:
                case EVENT_T_SUMMONED_JUST_DIED:
                case EVENT_T_SUMMONED_JUST_DESPAWN:
                    if (!sCreatureStorage.LookupEntry<CreatureInfo>(temp.summoned.creatureId))
                        sLog.outErrorEventAI("Creature %u are using event(%u) with nonexistent creature template id (%u) in param1, skipped.", temp.creature_id, i, temp.summoned.creatureId);
                    if (temp.summoned.repeatMax < temp.summoned.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using event(%u) with param2 < param1 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_QUEST_ACCEPT:
                case EVENT_T_QUEST_COMPLETE:
                    if (!sObjectMgr.GetQuestTemplate(temp.quest.questId))
                        sLog.outErrorEventAI("Creature %u are using event(%u) with nonexistent quest id (%u) in param1, skipped.", temp.creature_id, i, temp.quest.questId);
                    sLog.outErrorEventAI("Creature %u using not implemented event (%u) in event %u.", temp.creature_id, temp.event_id, i);
                    continue;
                case EVENT_T_DEATH:
                {
                    if (temp.death.conditionId && !sConditionStorage.LookupEntry<PlayerCondition>(temp.death.conditionId))
                    {
                        // condition does not exist for some reason
                        sLog.outErrorDb("Creature %u has `ConditionId` = %u but does not exist. Setting ConditionId to 0 for event %u.", temp.creature_id, temp.death.conditionId, i);
                        temp.death.conditionId = 0;
                    }
                }
                case EVENT_T_AGGRO:
                case EVENT_T_EVADE:
                case EVENT_T_REACHED_HOME:
                {
                    if (temp.event_flags & EFLAG_REPEATABLE)
                    {
                        sLog.outErrorEventAI("Creature %u has EFLAG_REPEATABLE set. Event can never be repeatable. Removing flag for event %u.", temp.creature_id, i);
                        temp.event_flags &= ~EFLAG_REPEATABLE;
                    }

                    if (temp.event_flags & EFLAG_COMBAT_ACTION)
                    {
                        sLog.outErrorEventAI("Creature %u has EFLAG_COMBAT_ACTION set. Event can never be done during combat. Removing flag for event %u.", temp.creature_id, i);
                        temp.event_flags &= ~EFLAG_COMBAT_ACTION;
                    }

                    break;
                }

                case EVENT_T_RECEIVE_EMOTE:
                {
                    if (!sEmotesTextStore.LookupEntry(temp.receive_emote.emoteId))
                    {
                        sLog.outErrorEventAI("Creature %u using event %u: param1 (EmoteTextId: %u) are not valid.", temp.creature_id, i, temp.receive_emote.emoteId);
                        continue;
                    }

                    if (temp.receive_emote.conditionId && !sConditionStorage.LookupEntry<PlayerCondition>(temp.receive_emote.conditionId))
                    {
                        sLog.outErrorDb("Creature %u has `ConditionId` = %u but does not exist. Setting ConditionId to 0 for event %u.", temp.creature_id, temp.receive_emote.conditionId, i);
                        temp.receive_emote.conditionId = 0;
                        continue;
                    }

                    if (!(temp.event_flags & EFLAG_REPEATABLE))
                    {
                        sLog.outErrorEventAI("Creature %u using event %u: EFLAG_REPEATABLE not set. Event must always be repeatable. Flag applied.", temp.creature_id, i);
                        temp.event_flags |= EFLAG_REPEATABLE;
                    }

                    break;
                }

                case EVENT_T_AURA:
                case EVENT_T_TARGET_AURA:
                case EVENT_T_MISSING_AURA:
                case EVENT_T_TARGET_MISSING_AURA:
                {
                    SpellEntry const* pSpell = sSpellTemplate.LookupEntry<SpellEntry>(temp.buffed.spellId);
                    if (!pSpell)
                    {
                        sLog.outErrorEventAI("Creature %u has nonexistent SpellID(%u) defined in event %u.", temp.creature_id, temp.buffed.spellId, i);
                        continue;
                    }
                    if (temp.buffed.amount < 1)
                    {
                        sLog.outErrorEventAI("Creature %u has wrong spell stack size (%u) defined in event %u.", temp.creature_id, temp.buffed.amount, i);
                        continue;
                    }
                    if (temp.buffed.repeatMax < temp.buffed.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                }
                case EVENT_T_RECEIVE_AI_EVENT:
                {
                    // Sender-Creature does not exist in database
                    if (temp.receiveAIEvent.senderEntry && !sCreatureStorage.LookupEntry<CreatureInfo>(temp.receiveAIEvent.senderEntry))
                    {
                        sLog.outErrorEventAI("Event %u has nonexisting creature (%u) defined for event RECEIVE_AI_EVENT, skipping.", i, temp.receiveAIEvent.senderEntry);
                        continue;
                    }
                    // Event-Type is not defined
                    if (temp.receiveAIEvent.eventType >= MAXIMAL_AI_EVENT_EVENTAI)
                    {
                        sLog.outErrorEventAI("Event %u has unfitting event-type (%u) defined for event RECEIVE_AI_EVENT (must be less than %u), skipping.", i, temp.receiveAIEvent.eventType, MAXIMAL_AI_EVENT_EVENTAI);
                        continue;
                    }
                    break;
                }
                case EVENT_T_SELECT_ATTACKING_TARGET:
                {
                    if (temp.event_flags & EFLAG_REPEATABLE && !temp.selectTarget.repeatMin && !temp.selectTarget.repeatMax)
                    {
                        sLog.outErrorEventAI("Creature %u has param3 and param4=0 (RepeatMin/RepeatMax) but cannot be repeatable without timers. Removing EFLAG_REPEATABLE for event %u.", temp.creature_id, i);
                        temp.event_flags &= ~EFLAG_REPEATABLE;
                    }
                    break;
                }
                case EVENT_T_FACING_TARGET:
                {
                    // Position is invalid (0:in back, 1:in front)
                    if (temp.facingTarget.backOrFront && temp.facingTarget.backOrFront != 1)
                    {
                        sLog.outErrorEventAI("Event %u has unfitting value (%u) for param1 in event EVENT_T_FACING_TARGET (must be 0 or 1), skipping.", i, temp.facingTarget.backOrFront);
                        continue;
                    }
                    // Event has repeatable flag but no timer
                    if (temp.event_flags & EFLAG_REPEATABLE && !temp.facingTarget.repeatMin && !temp.facingTarget.repeatMax)
                    {
                        sLog.outErrorEventAI("Creature %u has param3 and param4=0 (RepeatMin/RepeatMax) but cannot be repeatable without timers. Removing EFLAG_REPEATABLE for event %u.", temp.creature_id, i);
                        temp.event_flags &= ~EFLAG_REPEATABLE;
                    }
                    // Event has repeatable flag but timer is invalid
                    if (temp.event_flags & EFLAG_REPEATABLE && temp.facingTarget.repeatMax < temp.facingTarget.repeatMin)
                        sLog.outErrorEventAI("Creature %u is using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                }
                case EVENT_T_SPELLHIT_TARGET:
                    if (temp.spell_hit_target.spellId)
                    {
                        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(temp.spell_hit_target.spellId);
                        if (!spellInfo)
                        {
                            sLog.outErrorEventAI("Creature %u has nonexistent SpellID(%u) defined in event %u.", temp.creature_id, temp.spell_hit_target.spellId, i);
                            continue;
                        }

                        if ((temp.spell_hit_target.schoolMask & GetSchoolMask(spellInfo->School)) != GetSchoolMask(spellInfo->School))
                            sLog.outErrorEventAI("Creature %u has param1(spellId %u) but param2 is not -1 and not equal to spell's school mask. Event %u can never trigger.", temp.creature_id, temp.spell_hit.schoolMask, i);
                    }

                    if (!temp.spell_hit_target.schoolMask)
                        sLog.outErrorEventAI("Creature %u is using invalid SpellSchoolMask(%u) defined in event %u.", temp.creature_id, temp.spell_hit_target.schoolMask, i);

                    if (temp.spell_hit_target.repeatMax < temp.spell_hit_target.repeatMin)
                        sLog.outErrorEventAI("Creature %u are using repeatable event(%u) with param4 < param3 (RepeatMax < RepeatMin). Event will never repeat.", temp.creature_id, i);
                    break;
                case EVENT_T_DEATH_PREVENTED:
                    break;
                default:
                    sLog.outErrorEventAI("Creature %u using not checked at load event (%u) in event %u. Need check code update?", temp.creature_id, temp.event_id, i);
                    break;
            }

            for (uint32 j = 0; j < MAX_ACTIONS; ++j)
            {
                uint16 action_type = fields[12 + (j * 4)].GetUInt16();
                if (action_type >= ACTION_T_END)
                {
                    sLog.outErrorEventAI("Event %u Action %u has incorrect action type (%u), replace by ACTION_T_NONE.", i, j + 1, action_type);
                    temp.action[j].type = ACTION_T_NONE;
                    continue;
                }

                CreatureEventAI_Action& action = temp.action[j];

                action.type = EventAI_ActionType(action_type);
                action.raw.param1 = fields[13 + (j * 4)].GetUInt32();
                action.raw.param2 = fields[14 + (j * 4)].GetUInt32();
                action.raw.param3 = fields[15 + (j * 4)].GetUInt32();

                // Report any errors in actions
                switch (action.type)
                {
                    case ACTION_T_NONE:
                        break;
                    case ACTION_T_CHANCED_TEXT:
                        // Check first param as chance
                        if (!action.chanced_text.chance)
                            sLog.outErrorEventAI("Event %u Action %u has not set chance param1. Text will not be displayed", i, j + 1);
                        else if (action.chanced_text.chance >= 100)
                            sLog.outErrorEventAI("Event %u Action %u has set chance param1 >= 100. Text will always be displayed", i, j + 1);
                    // no break here to check texts
                    case ACTION_T_TEXT:
                    {
                        bool not_set = false;
                        int firstTextParam = action.type == ACTION_T_TEXT ? 0 : 1;
                        for (int k = firstTextParam; k < 3; ++k)
                        {
                            if (action.text.TextId[k])
                            {
                                if (k > firstTextParam && not_set)
                                    sLog.outErrorEventAI("Event %u Action %u has param%d, but it follow after not set param. Required for randomized text.", i, j + 1, k + 1);

                                if (!action.text.TextId[k])
                                    not_set = true;
                                // range negative
                                else if (action.text.TextId[k] > MIN_CREATURE_AI_TEXT_STRING_ID || action.text.TextId[k] <= MAX_CREATURE_AI_TEXT_STRING_ID)
                                {
                                    sLog.outErrorEventAI("Event %u Action %u param%d references out-of-range entry (%i) in texts table.", i, j + 1, k + 1, action.text.TextId[k]);
                                    action.text.TextId[k] = 0;
                                }
                                else if (!sObjectMgr.GetMangosStringLocale(action.text.TextId[k]))
                                {
                                    sLog.outErrorEventAI("Event %u Action %u param%d references non-existing entry (%i) in texts table.", i, j + 1, k + 1, action.text.TextId[k]);
                                    action.text.TextId[k] = 0;
                                }
                                else
                                    usedTextIds.insert(action.text.TextId[k]);
                            }
                        }
                        break;
                    }
                    case ACTION_T_SET_FACTION:
                        if (action.set_faction.factionId != 0 && !sFactionTemplateStore.LookupEntry(action.set_faction.factionId))
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent FactionId %u.", i, j + 1, action.set_faction.factionId);
                            action.set_faction.factionId = 0;
                        }
                        break;
                    case ACTION_T_MORPH_TO_ENTRY_OR_MODEL:
                        if (action.morph.creatureId != 0 || action.morph.modelId != 0)
                        {
                            if (action.morph.creatureId && !sCreatureStorage.LookupEntry<CreatureInfo>(action.morph.creatureId))
                            {
                                sLog.outErrorEventAI("Event %u Action %u uses nonexistent Creature entry %u.", i, j + 1, action.morph.creatureId);
                                action.morph.creatureId = 0;
                            }

                            if (action.morph.modelId)
                            {
                                if (action.morph.creatureId)
                                {
                                    sLog.outErrorEventAI("Event %u Action %u have unused ModelId %u with also set creature id %u.", i, j + 1, action.morph.modelId, action.morph.creatureId);
                                    action.morph.modelId = 0;
                                }
                                else if (!sCreatureDisplayInfoStore.LookupEntry(action.morph.modelId))
                                {
                                    sLog.outErrorEventAI("Event %u Action %u uses nonexistent ModelId %u.", i, j + 1, action.morph.modelId);
                                    action.morph.modelId = 0;
                                }
                            }
                        }
                        break;
                    case ACTION_T_SOUND:
                        if (!sSoundEntriesStore.LookupEntry(action.sound.soundId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent SoundID %u.", i, j + 1, action.sound.soundId);
                        break;
                    case ACTION_T_EMOTE:
                        if (!sEmotesStore.LookupEntry(action.emote.emoteId))
                            sLog.outErrorEventAI("Event %u Action %u param1 (EmoteId: %u) are not valid.", i, j + 1, action.emote.emoteId);
                        break;
                    case ACTION_T_RANDOM_SOUND:
                        if (!sSoundEntriesStore.LookupEntry(action.random_sound.soundId1))
                            sLog.outErrorEventAI("Event %u Action %u param1 uses nonexistent SoundID %u.", i, j + 1, action.random_sound.soundId1);
                        if (action.random_sound.soundId2 >= 0 && !sSoundEntriesStore.LookupEntry(action.random_sound.soundId2))
                            sLog.outErrorEventAI("Event %u Action %u param2 uses nonexistent SoundID %u.", i, j + 1, action.random_sound.soundId2);
                        if (action.random_sound.soundId3 >= 0 && !sSoundEntriesStore.LookupEntry(action.random_sound.soundId3))
                            sLog.outErrorEventAI("Event %u Action %u param3 uses nonexistent SoundID %u.", i, j + 1, action.random_sound.soundId3);
                        break;
                    case ACTION_T_RANDOM_EMOTE:
                        if (!sEmotesStore.LookupEntry(action.random_emote.emoteId1))
                            sLog.outErrorEventAI("Event %u Action %u param1 (EmoteId: %u) are not valid.", i, j + 1, action.random_emote.emoteId1);
                        if (action.random_emote.emoteId2 >= 0 && !sEmotesStore.LookupEntry(action.random_emote.emoteId2))
                            sLog.outErrorEventAI("Event %u Action %u param2 (EmoteId: %u) are not valid.", i, j + 1, action.random_emote.emoteId2);
                        if (action.random_emote.emoteId3 >= 0 && !sEmotesStore.LookupEntry(action.random_emote.emoteId3))
                            sLog.outErrorEventAI("Event %u Action %u param3 (EmoteId: %u) are not valid.", i, j + 1, action.random_emote.emoteId3);
                        break;
                    case ACTION_T_CAST:
                    {
                        const SpellEntry* spell = sSpellTemplate.LookupEntry<SpellEntry>(action.cast.spellId);
                        if (!spell)
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent SpellID %u.", i, j + 1, action.cast.spellId);
                        /* FIXME: temp.raw.param3 not have event tipes with recovery time in it....
                        else
                        {
                            if (spell->RecoveryTime > 0 && temp.event_flags & EFLAG_REPEATABLE)
                            {
                                // output as debug for now, also because there's no general rule all spells have RecoveryTime
                                if (temp.event_param3 < spell->RecoveryTime)
                                    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "CreatureEventAI:  Event %u Action %u uses SpellID %u but cooldown is longer(%u) than minumum defined in event param3(%u).", i, j+1,action.cast.spellId, spell->RecoveryTime, temp.event_param3);
                            }
                        }
                        */

                        // Cast is always triggered if target is forced to cast on self
                        if (action.cast.castFlags & CAST_FORCE_TARGET_SELF)
                            action.cast.castFlags |= CAST_TRIGGERED;

                        IsValidTargetType(temp.event_type, action.type, action.cast.target, i, j + 1);

                        // Some Advanced target type checks - Can have false positives
                        if (!sLog.HasLogFilter(LOG_FILTER_EVENT_AI_DEV) && spell)
                        {
                            // spell must be cast on self, but is not
                            if ((IsOnlySelfTargeting(spell) || spell->rangeIndex == SPELL_RANGE_IDX_SELF_ONLY) && action.cast.target != TARGET_T_SELF && !(action.cast.castFlags & CAST_FORCE_TARGET_SELF))
                                sLog.outErrorEventAI("Event %u Action %u uses SpellID %u that must be self cast (target is %u)", i, j + 1, action.cast.spellId, action.cast.target);

                            // TODO: spell must be cast on enemy, but is not

                            // used TARGET_T_ACTION_INVOKER, but likely should be _INVOKER_OWNER instead
                            if (action.cast.target == TARGET_T_ACTION_INVOKER &&
                                    (IsSpellHaveEffect(spell, SPELL_EFFECT_QUEST_COMPLETE) || IsSpellHaveEffect(spell, SPELL_EFFECT_DUMMY)))
                                sLog.outErrorEventAI("Event %u Action %u has TARGET_T_ACTION_INVOKER(%u) target type, but should have TARGET_T_ACTION_INVOKER_OWNER(%u).", i, j + 1, TARGET_T_ACTION_INVOKER, TARGET_T_ACTION_INVOKER_OWNER);

                            // Spell that should only target players, but could get any
                            if (spell->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) &&
                                    (action.cast.target == TARGET_T_ACTION_INVOKER || action.cast.target == TARGET_T_HOSTILE_RANDOM || action.cast.target == TARGET_T_HOSTILE_RANDOM_NOT_TOP))
                                sLog.outErrorEventAI("Event %u Action %u uses Target type %u for a spell (%u) that should only target players. This could be wrong.", i, j + 1, action.cast.target, action.cast.spellId);

                            if (spell->Targets != 0 && (action.cast.target == TARGET_T_NONE || action.cast.target == TARGET_T_NEAREST_AOE_TARGET))
                            {
                                sLog.outErrorEventAI("Event %u Action %u uses Target type %u for a spell (%u) that needs target for casting. Resetting it to TARGET_T_HOSTILE.", i, j + 1, action.cast.target, action.cast.spellId);
                                action.cast.target = TARGET_T_HOSTILE;
                            }
                        }
                        break;
                    }
                    case ACTION_T_SPAWN :
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.summon.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.summon.creatureId);

                        IsValidTargetType(temp.event_type, action.type, action.summon.target, i, j + 1);
                        break;
                    case ACTION_T_THREAT_SINGLE:
                        if (std::abs(action.threat_single.value) > 100 && !action.threat_single.isDirect)
                            sLog.outErrorEventAI("Event %u Action %u uses invalid percent value %u.", i, j + 1, action.threat_single.value);
                        IsValidTargetType(temp.event_type, action.type, action.threat_single.target, i, j + 1);
                        break;
                    case ACTION_T_THREAT_ALL_PCT:
                        if (std::abs(action.threat_all_pct.percent) > 100)
                            sLog.outErrorEventAI("Event %u Action %u uses invalid percent value %u.", i, j + 1, action.threat_all_pct.percent);
                        break;
                    case ACTION_T_QUEST_EVENT:
                        if (Quest const* qid = sObjectMgr.GetQuestTemplate(action.quest_event.questId))
                        {
                            if (!qid->HasSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT))
                                sLog.outErrorEventAI("Event %u Action %u. SpecialFlags for quest entry %u does not include |2, Action will not have any effect.", i, j + 1, action.quest_event.questId);
                        }
                        else
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent Quest entry %u.", i, j + 1, action.quest_event.questId);

                        IsValidTargetType(temp.event_type, action.type, action.quest_event.target, i, j + 1);
                        break;
                    case ACTION_T_CAST_EVENT:
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.cast_event.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.cast_event.creatureId);
                        if (!sSpellTemplate.LookupEntry<SpellEntry>(action.cast_event.spellId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent SpellID %u.", i, j + 1, action.cast_event.spellId);
                        IsValidTargetType(temp.event_type, action.type, action.cast_event.target, i, j + 1);
                        break;
                    case ACTION_T_SET_UNIT_FIELD:
                        if (action.set_unit_field.field < OBJECT_END || action.set_unit_field.field >= UNIT_END)
                            sLog.outErrorEventAI("Event %u Action %u param1 (UNIT_FIELD*). Index out of range for intended use.", i, j + 1);
                        IsValidTargetType(temp.event_type, action.type, action.set_unit_field.target, i, j + 1);
                        break;
                    case ACTION_T_SET_UNIT_FLAG:
                    case ACTION_T_REMOVE_UNIT_FLAG:
                        IsValidTargetType(temp.event_type, action.type, action.unit_flag.target, i, j + 1);
                        break;
                    case ACTION_T_SET_PHASE:
                        if (action.set_phase.phase >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phase >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        break;
                    case ACTION_T_INC_PHASE:
                        if (action.set_inc_phase.step == 0)
                            sLog.outErrorEventAI("Event %u Action %u is incrementing phase by 0. Was this intended?", i, j + 1);
                        else if (std::abs(action.set_inc_phase.step) > MAX_PHASE - 1)
                            sLog.outErrorEventAI("Event %u Action %u is change phase by too large for any use %i.", i, j + 1, action.set_inc_phase.step);
                        break;
                    case ACTION_T_QUEST_EVENT_ALL:
                        if (Quest const* qid = sObjectMgr.GetQuestTemplate(action.quest_event_all.questId))
                        {
                            if (!qid->HasSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT))
                                sLog.outErrorEventAI("Event %u Action %u. SpecialFlags for quest entry %u does not include |2, Action will not have any effect.", i, j + 1, action.quest_event_all.questId);
                        }
                        else
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent Quest entry %u.", i, j + 1, action.quest_event_all.questId);
                        break;
                    case ACTION_T_CAST_EVENT_ALL:
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.cast_event_all.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.cast_event_all.creatureId);
                        if (!sSpellTemplate.LookupEntry<SpellEntry>(action.cast_event_all.spellId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent SpellID %u.", i, j + 1, action.cast_event_all.spellId);
                        break;
                    case ACTION_T_REMOVEAURASFROMSPELL:
                        if (!sSpellTemplate.LookupEntry<SpellEntry>(action.remove_aura.spellId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent SpellID %u.", i, j + 1, action.remove_aura.spellId);
                        IsValidTargetType(temp.event_type, action.type, action.remove_aura.target, i, j + 1);
                        break;
                    case ACTION_T_RANDOM_PHASE:             // PhaseId1, PhaseId2, PhaseId3
                        if (action.random_phase.phase1 >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phase1 >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        if (action.random_phase.phase2 >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phase2 >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        if (action.random_phase.phase3 >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phase3 >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        break;
                    case ACTION_T_RANDOM_PHASE_RANGE:       // PhaseMin, PhaseMax
                        if (action.random_phase_range.phaseMin >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phaseMin >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        if (action.random_phase_range.phaseMin >= MAX_PHASE)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phaseMax >= %u. Phase mask cannot be used past phase %u.", i, j + 1, MAX_PHASE, MAX_PHASE - 1);
                        if (action.random_phase_range.phaseMin >= action.random_phase_range.phaseMax)
                        {
                            sLog.outErrorEventAI("Event %u Action %u attempts to set phaseMax <= phaseMin.", i, j + 1);
                            std::swap(action.random_phase_range.phaseMin, action.random_phase_range.phaseMax);
                            // equal case processed at call
                        }
                        break;
                    case ACTION_T_SUMMON_ID:
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.summon_id.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.summon_id.creatureId);
                        IsValidTargetType(temp.event_type, action.type, action.summon_id.target, i, j + 1);
                        if (m_CreatureEventAI_Summon_Map.find(action.summon_id.spawnId) == m_CreatureEventAI_Summon_Map.end())
                            sLog.outErrorEventAI("Event %u Action %u summons missing CreatureEventAI_Summon %u", i, j + 1, action.summon_id.spawnId);
                        break;
                    case ACTION_T_KILLED_MONSTER:
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.killed_monster.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.killed_monster.creatureId);
                        IsValidTargetType(temp.event_type, action.type, action.killed_monster.target, i, j + 1);
                        break;
                    case ACTION_T_SET_INST_DATA:
                        if (action.set_inst_data.value > 4/*SPECIAL*/)
                            sLog.outErrorEventAI("Event %u Action %u attempts to set instance data above encounter state 4. Custom case?", i, j + 1);
                        break;
                    case ACTION_T_SET_INST_DATA64:
                        IsValidTargetType(temp.event_type, action.type, action.set_inst_data64.target, i, j + 1);
                        break;
                    case ACTION_T_UPDATE_TEMPLATE:
                        if (!sCreatureStorage.LookupEntry<CreatureInfo>(action.update_template.creatureId))
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent creature entry %u.", i, j + 1, action.update_template.creatureId);
                        break;
                    case ACTION_T_SET_SHEATH:
                        if (action.set_sheath.sheath >= MAX_SHEATH_STATE)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses wrong sheath state %u.", i, j + 1, action.set_sheath.sheath);
                            action.set_sheath.sheath = SHEATH_STATE_UNARMED;
                        }
                        break;
                    case ACTION_T_SET_DEATH_PREVENTION:
                        if (action.deathPrevention.state > 1)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid death prevention state %u. Setting to 1.", i, j + 1, action.deathPrevention.state);
                            action.deathPrevention.state = 1;
                        }
                        break;
                    case ACTION_T_MOUNT_TO_ENTRY_OR_MODEL:
                        if (action.mount.creatureId != 0 || action.mount.modelId != 0)
                        {
                            if (action.mount.creatureId && !sCreatureStorage.LookupEntry<CreatureInfo>(action.mount.creatureId))
                            {
                                sLog.outErrorEventAI("Event %u Action %u uses nonexistent Creature entry %u.", i, j + 1, action.mount.creatureId);
                                action.morph.creatureId = 0;
                            }

                            if (action.mount.modelId)
                            {
                                if (action.mount.creatureId)
                                {
                                    sLog.outErrorEventAI("Event %u Action %u have unused ModelId %u with also set creature id %u.", i, j + 1, action.mount.modelId, action.mount.creatureId);
                                    action.mount.modelId = 0;
                                }
                                else if (!sCreatureDisplayInfoStore.LookupEntry(action.mount.modelId))
                                {
                                    sLog.outErrorEventAI("Event %u Action %u uses nonexistent ModelId %u.", i, j + 1, action.mount.modelId);
                                    action.mount.modelId = 0;
                                }
                            }
                        }
                        break;
                    case ACTION_T_EVADE:                    // No Params
                    case ACTION_T_FLEE_FOR_ASSIST:          // No Params
                    case ACTION_T_DIE:                      // No Params
                    case ACTION_T_ZONE_COMBAT_PULSE:        // No Params
                    case ACTION_T_FORCE_DESPAWN:            // Delay
                    case ACTION_T_AUTO_ATTACK:              // AllowAttackState (0 = stop attack, anything else means continue attacking)
                    case ACTION_T_COMBAT_MOVEMENT:          // AllowCombatMovement (0 = stop combat based movement, anything else continue attacking)
                    case ACTION_T_RANGED_MOVEMENT:          // Distance, Angle
                    case ACTION_T_CALL_FOR_HELP:            // Distance
                        break;

                    case ACTION_T_RANDOM_SAY:
                    case ACTION_T_RANDOM_YELL:
                    case ACTION_T_RANDOM_TEXTEMOTE:
                        sLog.outErrorEventAI("Event %u Action %u currently unused ACTION type. Did you forget to update database?", i, j + 1);
                        break;

                    case ACTION_T_THROW_AI_EVENT:
                        if (action.throwEvent.eventType >= MAXIMAL_AI_EVENT_EVENTAI)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid event type %u (must be less than %u), skipping", i, j + 1, action.throwEvent.eventType, MAXIMAL_AI_EVENT_EVENTAI);
                            continue;
                        }
                        if (action.throwEvent.radius > (uint32)SIZE_OF_GRIDS)
                            sLog.outErrorEventAI("Event %u Action %u uses unexpectedly huge radius %u (expected to be less than %f)", i, j + 1, action.throwEvent.radius, SIZE_OF_GRIDS);

                        if (action.throwEvent.radius == 0)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses unexpected radius 0 (set to %f of CONFIG_FLOAT_CREATURE_FAMILY_ASSISTANCE_RADIUS)", i, j + 1, sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_ASSISTANCE_RADIUS));
                            action.throwEvent.radius = uint32(sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_ASSISTANCE_RADIUS));
                        }
                        IsValidTargetType(temp.event_type, action.type, action.throwEvent.target, i, j + 1);
                        break;
                    case ACTION_T_SET_THROW_MASK:
                        if (action.setThrowMask.eventTypeMask & ~((1 << MAXIMAL_AI_EVENT_EVENTAI) - 1))
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid AIEvent-typemask %u (must be smaller than %u)", i, j + 1, action.setThrowMask.eventTypeMask, MAXIMAL_AI_EVENT_EVENTAI << 1);
                        }
                        break;
                    case ACTION_T_SET_STAND_STATE:
                        if (action.setStandState.standState >= MAX_UNIT_STAND_STATE)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid unit stand state %u (must be smaller than %u)", i, j + 1, action.setStandState.standState, MAX_UNIT_STAND_STATE);
                        }
                        break;
                    case ACTION_T_CHANGE_MOVEMENT:
                        if (action.changeMovement.movementType >= MAX_DB_MOTION_TYPE)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid movement type %u (must be smaller than %u)", i, j + 1, action.changeMovement.movementType, MAX_DB_MOTION_TYPE);
                        }
                        if (action.changeMovement.asDefault > 1)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid default movement setting %u. Setting to 0.", i, j + 1, action.changeMovement.asDefault);
                            action.deathPrevention.state = 0;
                        }
                        break;
                    case ACTION_T_SET_REACT_STATE:
                        if (action.setReactState.reactState > REACT_AGGRESSIVE)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid react state %u (must be smaller than %u)", i, j + 1, action.setReactState.reactState, REACT_AGGRESSIVE);
                        }
                        break;
                    case ACTION_T_PAUSE_WAYPOINTS:
                        break;
                    case ACTION_T_INTERRUPT_SPELL:
                        if (action.interruptSpell.currentSpellType >= CURRENT_MAX_SPELL)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid current spell type %u (must be smaller or equal to %u)", i, j + 1, action.interruptSpell.currentSpellType, CURRENT_MAX_SPELL - 1);
                        }
                        break;
                    case ACTION_T_START_RELAY_SCRIPT:
                        if (action.relayScript.relayId > 0)
                        {
                            if (sRelayScripts.second.find(action.relayScript.relayId) == sRelayScripts.second.end())
                            {
                                sLog.outErrorEventAI("Event %u Action %u references invalid dbscript_on_relay id %u", i, j + 1, action.relayScript.relayId);
                            }
                        }
                        else
                        {
                            if (!sScriptMgr.CheckScriptRelayTemplateId(uint32(-action.relayScript.relayId)))
                            {
                                sLog.outErrorEventAI("Event %u Action %u references non-existing entry for relay Id template (%i) in dbscript_random_templates table.", i, j + 1, action.relayScript.relayId);
                                break;
                            }
                        }
                        break;
                    case ACTION_T_TEXT_NEW:
                        if (action.textNew.textId)
                        {
                            if (!sObjectMgr.GetMangosStringLocale(action.textNew.textId))
                            {
                                sLog.outErrorEventAI("Event %u Action %u references non-existing entry (%i) in texts table.", i, j + 1, action.textNew.textId);
                                action.textNew.textId = 0;
                            }
                            else
                                usedTextIds.insert(action.textNew.textId);
                        }
                        else
                        {
                            if (!sScriptMgr.CheckScriptStringTemplateId(action.textNew.templateId))
                            {
                                sLog.outErrorEventAI("Event %u Action %u references non-existing entry for text template (%i) in dbscript_random_templates table.", i, j + 1, action.textNew.textId);
                                break;
                            }
                            ScriptMgr::ScriptTemplateVector templateData;
                            sScriptMgr.GetScriptStringTemplate(action.textNew.templateId, templateData);
                            for (auto& data : templateData)
                                if (data.first)
                                    usedTextIds.insert(data.first);
                        }
                        break;
                    case ACTION_T_ATTACK_START:
                        IsValidTargetType(temp.event_type, action.type, action.attackStart.target, i, j + 1);
                        break;
                    case ACTION_T_DESPAWN_GUARDIANS:
                        if (action.despawnGuardians.entryId && !sCreatureStorage.LookupEntry<CreatureInfo>(action.despawnGuardians.entryId))
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent Creature entry %u.", i, j + 1, action.despawnGuardians.entryId);
                            action.despawnGuardians.entryId = 0;
                        }
                        break;
                    case ACTION_T_SET_RANGED_MODE:
                        if (action.rangedMode.type >= TYPE_MAX)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses nonexistent ranged mode type %u. Setting to 0.", i, j + 1, action.rangedMode.type);
                            action.rangedMode.type = 0;
                        }
                        if (action.rangedMode.chaseDistance > 200)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses too large chase distance %u. Setting to 30.", i, j + 1, action.rangedMode.chaseDistance);
                            action.rangedMode.chaseDistance = 30;
                        }
                        break;
                    case ACTION_T_SET_WALK:
                        switch (action.walkSetting.type)
                        {
                            case WALK_DEFAULT:
                            case RUN_DEFAULT:
                            case WALK_CHASE:
                            case RUN_CHASE:
                                break; // correct values
                            default:
                                sLog.outErrorEventAI("Event %u Action %u uses invalid walking mode %u. Setting to RUN_DEFAULT.", i, j + 1, action.walkSetting.type);
                                action.walkSetting.type = RUN_DEFAULT;
                                break;
                        }
                        break;
                    case ACTION_T_SET_FACING:
                        if (action.setFacing.reset > 1)
                        {
                            sLog.outErrorEventAI("Event %u Action %u uses invalid facing setting %u. Setting to 0.", i, j + 1, action.setFacing.reset);
                            action.setFacing.reset = 0;
                            break;
                        }
                        IsValidTargetType(temp.event_type, action.type, action.setFacing.target, i, j + 1);
                        break;
                    default:
                        sLog.outErrorEventAI("Event %u Action %u have currently not checked at load action type (%u). Need check code update?", i, j + 1, temp.action[j].type);
                        break;
                }
            }

            // Add to list
            m_CreatureEventAI_Event_Map[creature_id].push_back(temp);
            ++Count;

            switch (temp.event_type)
            {
                case EVENT_T_FRIENDLY_HP:
                {
                    // compute data at the end
                    CreatureEventAI_EventComputedData data;
                    data.friendlyHp.targetSelf = true;
                    for (uint32 j = 0; j < MAX_ACTIONS; ++j)
                    {
                        CreatureEventAI_Action& action = temp.action[j];
                        if (temp.action[j].type == ACTION_T_CAST && temp.action[j].cast.target == TARGET_T_EVENT_SPECIFIC)
                        {
                            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(temp.action[j].cast.spellId);
                            if (spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_TARGET_SELF))
                                data.friendlyHp.targetSelf = false;
                        }
                    }
                    m_creatureEventAI_ComputedDataMap.emplace(temp.event_id, data);
                    break;
                }
                default: break;
            }
        }
        while (result->NextRow());

        delete result;
        m_usedTextsAmount = usedTextIds.size();

        // post check
        for (uint32 i = 1; i < sCreatureStorage.GetMaxEntry(); ++i)
        {
            if (CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i))
            {
                bool ainame = strcmp(cInfo->AIName, "EventAI") == 0 || strcmp(cInfo->AIName, "GuardianAI") == 0;
                bool hasevent = m_CreatureEventAI_Event_Map.find(i) != m_CreatureEventAI_Event_Map.end();
                if (ainame && !hasevent)
                    sLog.outErrorEventAI("EventAI not has script for creature entry (%u), but AIName = '%s'.", i, cInfo->AIName);
                else if (!ainame && hasevent)
                    sLog.outErrorEventAI("EventAI has script for creature entry (%u), but AIName = '%s' instead 'EventAI'.", i, cInfo->AIName);
            }
        }

        CheckUnusedAITexts();
        CheckUnusedAISummons();

        sLog.outString(">> Loaded %u CreatureEventAI scripts", Count);
        sLog.outString();
    }
    else
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded 0 CreatureEventAI scripts. DB table `creature_ai_scripts` is empty.");
        sLog.outString();
    }
}