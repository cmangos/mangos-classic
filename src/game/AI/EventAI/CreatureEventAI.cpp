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
#include "CreatureEventAI.h"
#include "CreatureEventAIMgr.h"
#include "Globals/ObjectMgr.h"
#include "World/World.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Maps/InstanceData.h"
#include "Chat/Chat.h"
#include "Tools/Language.h"
#include "Entities/TemporarySpawn.h"
#include "Spells/Spell.h"
#include "MotionGenerators/MovementGenerator.h"

bool CreatureEventAIHolder::UpdateRepeatTimer(Creature* creature, uint32 repeatMin, uint32 repeatMax)
{
    if (repeatMin == repeatMax)
        timer = repeatMin;
    else if (repeatMax > repeatMin)
        timer = urand(repeatMin, repeatMax);
    else
    {
        sLog.outErrorEventAI("Creature %u using Event %u (Type = %u) has RandomMax < RandomMin. Event repeating disabled.", creature->GetEntry(), event.event_id, event.event_type);
        enabled = false;
        return false;
    }

    return true;
}

int CreatureEventAI::Permissible(const Creature* creature)
{
    if (creature->GetAIName() == "EventAI")
        return PERMIT_BASE_SPECIAL;

    if (creature->IsNoAggroOnSight() || creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    if (!creature->IsNoAggroOnSight() && !creature->IsNeutralToAll())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

void CreatureEventAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage(LANG_NPC_EVENTAI_PHASE, uint32(m_Phase));
    reader.PSendSysMessage(LANG_NPC_EVENTAI_MOVE, reader.GetOnOffStr(!m_creature->hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT)));
    reader.PSendSysMessage(LANG_NPC_EVENTAI_COMBAT, reader.GetOnOffStr(m_meleeEnabled));
    reader.PSendSysMessage("Ranged mode %s, Combat Script %s.", m_currentRangedMode ? "true" : "false", GetCombatScriptStatus() ? "true" : "false");

    if (sLog.HasLogFilter(LOG_FILTER_EVENT_AI_DEV))         // Give some more details if in EventAI Dev Mode
        return;

    reader.PSendSysMessage("Current events of this creature:");
    for (CreatureEventAIList::const_iterator itr = m_CreatureEventAIList.begin(); itr != m_CreatureEventAIList.end(); ++itr)
    {
        if (itr->event.action[2].type != ACTION_T_NONE)
            reader.PSendSysMessage("%u Type%3u (%s) Timer(%3us) actions[type(param1)]: %2u(%5u)  --  %2u(%u)  --  %2u(%5u)", itr->event.event_id, uint32(itr->event.event_type), itr->enabled ? "On" : "Off", itr->timer / 1000, itr->event.action[0].type, itr->event.action[0].raw.param1, itr->event.action[1].type, itr->event.action[1].raw.param1, itr->event.action[2].type, itr->event.action[2].raw.param1);
        else if (itr->event.action[1].type != ACTION_T_NONE)
            reader.PSendSysMessage("%u Type%3u (%s) Timer(%3us) actions[type(param1)]: %2u(%5u)  --  %2u(%5u)", itr->event.event_id, uint32(itr->event.event_type), itr->enabled ? "On" : "Off", itr->timer / 1000, itr->event.action[0].type, itr->event.action[0].raw.param1, itr->event.action[1].type, itr->event.action[1].raw.param1);
        else
            reader.PSendSysMessage("%u Type%3u (%s) Timer(%3us) action[type(param1)]:  %2u(%5u)", itr->event.event_id, uint32(itr->event.event_type), itr->enabled ? "On" : "Off", itr->timer / 1000, itr->event.action[0].type, itr->event.action[0].raw.param1);
    }
}

CreatureEventAI::CreatureEventAI(Creature* creature) : CreatureAI(creature),
    m_EventUpdateTime(0),
    m_EventDiff(0),
    m_depth(0),
    m_Phase(0),
    m_HasOOCLoSEvent(false),
    m_InvinceabilityHpLevel(0),
    m_throwAIEventMask(0),
    m_throwAIEventStep(0),
    m_LastSpellMaxRange(0),
    m_despawnAggregationMask(0)
{

}

void CreatureEventAI::InitAI()
{
    m_CreatureEventAIList.clear();

    auto processMap = [&](const CreatureEventAI_Event_Vec& creatureEvent)
    {
        uint32 events_count = 0;
        for (const auto& i : creatureEvent)
        {
            // Debug check
#ifndef MANGOS_DEBUG
            if (i.event_flags & EFLAG_DEBUG_ONLY)
                continue;
#endif

            ++events_count;
        }
        // EventMap had events but they were not added because they must be for instance
        if (events_count == 0)
            sLog.outErrorEventAI("Creature %u has events but no events added to list because of instance flags (spawned in map %u).", m_creature->GetEntry(), m_creature->GetMapId());
        else
        {
            m_CreatureEventAIList.reserve(events_count);
            for (const auto& aiEvent : creatureEvent)
            {
                // Debug check
#ifndef MANGOS_DEBUG
                if (aiEvent.event_flags & EFLAG_DEBUG_ONLY)
                    continue;
#endif
                // Indent for better compatibility with other cores
                    m_CreatureEventAIList.push_back(CreatureEventAIHolder(aiEvent));
                    // Cache for fast use
                    if (aiEvent.event_type == EVENT_T_OOC_LOS)
                        m_HasOOCLoSEvent = true;

                    for (uint32 actionIdx = 0; actionIdx < MAX_ACTIONS; ++actionIdx)
                        if (aiEvent.action[actionIdx].type == ACTION_T_CAST)
                        {
                            if (aiEvent.action[actionIdx].cast.castFlags & CAST_MAIN_SPELL)
                                AddMainSpell(aiEvent.action[actionIdx].cast.spellId);

                            if (aiEvent.action[actionIdx].cast.castFlags & CAST_DISTANCE_YOURSELF)
                                AddDistanceSpell(aiEvent.action[actionIdx].cast.spellId);
                        }
            }
        }
    };

    // Need make copy for filter unneeded steps and safe in case table reload
    auto creatureEventsItr = m_creature->GetMap()->GetMapDataContainer().GetCreatureEventEntryAIMap()->find(m_creature->GetEntry());
    if (creatureEventsItr != m_creature->GetMap()->GetMapDataContainer().GetCreatureEventEntryAIMap()->end())
    {
        const CreatureEventAI_Event_Vec& creatureEvent = creatureEventsItr->second;
        processMap(creatureEvent);
    }

    auto creatureEventsGuidItr = m_creature->GetMap()->GetMapDataContainer().GetCreatureEventGuidAIMap()->find(m_creature->GetDbGuid());
    if (creatureEventsGuidItr != m_creature->GetMap()->GetMapDataContainer().GetCreatureEventGuidAIMap()->end())
    {
        const CreatureEventAI_Event_Vec& creatureEvent = creatureEventsGuidItr->second;
        processMap(creatureEvent);
    }
}

bool CreatureEventAI::IsTimerExecutedEvent(EventAI_Type type) const
{
    switch (type)
    {
        case EVENT_T_TIMER_IN_COMBAT:
        case EVENT_T_TIMER_OOC:
        case EVENT_T_TIMER_GENERIC:
        case EVENT_T_MANA:
        case EVENT_T_HP:
        case EVENT_T_TARGET_HP:
        case EVENT_T_TARGET_CASTING:
        case EVENT_T_FRIENDLY_HP:
        case EVENT_T_FRIENDLY_IS_CC:
        case EVENT_T_FRIENDLY_MISSING_BUFF:
        case EVENT_T_TARGET_MANA:
        case EVENT_T_AURA:
        case EVENT_T_TARGET_AURA:
        case EVENT_T_MISSING_AURA:
        case EVENT_T_TARGET_MISSING_AURA:
        case EVENT_T_RANGE:
        case EVENT_T_ENERGY:
        case EVENT_T_SELECT_ATTACKING_TARGET:
        case EVENT_T_FACING_TARGET:
            return true;
        default:
            return false;
    }
}

bool CreatureEventAI::IsRepeatableEvent(EventAI_Type type) const
{
    switch (type)
    {
        case EVENT_T_SPAWNED: // Only happens once per spawn - repeatable doesnt make sense
        case EVENT_T_DEATH: // as previous
        case EVENT_T_AGGRO:
        case EVENT_T_EVADE:
        case EVENT_T_REACHED_HOME:
            return false;
        default:
            return true;
    }
}

bool CreatureEventAI::IsTimerBasedEvent(EventAI_Type type) const
{
    switch (type)
    {
        case EVENT_T_TIMER_IN_COMBAT:
        case EVENT_T_TIMER_OOC:
        case EVENT_T_TIMER_GENERIC:
        case EVENT_T_HP:
        case EVENT_T_MANA:
        case EVENT_T_KILL:
        case EVENT_T_SPELLHIT:
        case EVENT_T_RANGE:
        case EVENT_T_OOC_LOS:
        case EVENT_T_TARGET_HP:
        case EVENT_T_TARGET_CASTING:
        case EVENT_T_FRIENDLY_HP:
        case EVENT_T_FRIENDLY_IS_CC:
        case EVENT_T_FRIENDLY_MISSING_BUFF:
        case EVENT_T_SUMMONED_UNIT:
        case EVENT_T_SUMMONED_JUST_DIED:
        case EVENT_T_SUMMONED_JUST_DESPAWN:
        case EVENT_T_TARGET_MANA:
        case EVENT_T_AURA:
        case EVENT_T_TARGET_AURA:
        case EVENT_T_MISSING_AURA:
        case EVENT_T_TARGET_MISSING_AURA:
        case EVENT_T_ENERGY:
        case EVENT_T_SELECT_ATTACKING_TARGET:
        case EVENT_T_FACING_TARGET:
        case EVENT_T_SPELLHIT_TARGET:
            return true;
        default: return false;
    }
}

void CreatureEventAI::ProcessEvents(Unit* actionInvoker, Unit* AIEventSender)
{
    const int curDepth = m_depth;
    ++m_depth;
    for (CreatureEventAIHolder& event : m_creatureEventAITempList[curDepth])
        ProcessEvent(event, actionInvoker, AIEventSender);

    m_creatureEventAITempList[m_depth - 1].clear();
    --m_depth;
}

bool CreatureEventAI::CheckEvent(CreatureEventAIHolder& holder, Unit* actionInvoker, Unit* /*AIEventSender =nullptr*/)
{
    if (!holder.enabled || holder.timer || holder.inProgress)
        return false;

    if (holder.event.event_flags & EFLAG_COMBAT_ACTION && !CanExecuteCombatAction())
        return false;

    // Check the inverse phase mask (event doesn't trigger if current phase bit is set in mask)
    if (holder.event.event_inverse_phase_mask & (1 << m_Phase))
    {
        if (!IsTimerExecutedEvent(holder.event.event_type))
            DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "%s: Event %u skipped because of phasemask %u. Current phase %u", GetAIName().data(), holder.event.event_id, holder.event.event_inverse_phase_mask, m_Phase);
        return false;
    }

    if (m_currentRangedMode)
    {
        if (holder.event.event_flags & EFLAG_MELEE_MODE_ONLY)
            return false;
    }
    else
    {
        if (holder.event.event_flags & EFLAG_RANGED_MODE_ONLY)
            return false;
    }

    LOG_PROCESS_EVENT;

    CreatureEventAI_Event const& event = holder.event;

    // Check event conditions based on the event type, also reset events
    switch (event.event_type)
    {
        case EVENT_T_TIMER_IN_COMBAT:
            if (!m_creature->IsInCombat())
                return false;
            break;
        case EVENT_T_TIMER_OOC:
            if (m_creature->IsInCombat() || m_creature->GetCombatManager().IsInEvadeMode())
                return false;
            break;
        case EVENT_T_TIMER_GENERIC:
            break;
        case EVENT_T_HP:
        {
            if (!event.percent_range.allowOutOfCombat && (!m_creature->IsInCombat() || !m_creature->GetMaxHealth()))
                return false;

            uint32 perc = (m_creature->GetHealth() * 100) / m_creature->GetMaxHealth();

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;
            break;
        }
        case EVENT_T_MANA:
        {
            if (!m_creature->IsInCombat() || !m_creature->HasMana() || !m_creature->GetMaxPower(POWER_MANA))
                return false;

            uint32 perc = (m_creature->GetPower(POWER_MANA) * 100) / m_creature->GetMaxPower(POWER_MANA);

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;
            break;
        }
        case EVENT_T_AGGRO:
            break;
        case EVENT_T_KILL:
            if (event.kill.playerOnly == 1 && actionInvoker->GetTypeId() != TYPEID_PLAYER)
                return false;
            break;
        case EVENT_T_DEATH:
            if (event.death.conditionId)
            {
                if (!actionInvoker)
                    return false;

                Unit const* controller = actionInvoker->GetControllingPlayer();
                if (!controller) // only allow going forward if we can evaluate condition
                    controller = actionInvoker;

                if (!sObjectMgr.IsConditionSatisfied(event.death.conditionId, controller, controller->GetMap(), m_creature, CONDITION_FROM_EVENTAI))
                    return false;
            }
            break;
        case EVENT_T_EVADE:
            break;
        case EVENT_T_SPELLHIT:
        case EVENT_T_SPELLHIT_TARGET:
        case EVENT_T_SPELL_CAST:
            break;
        case EVENT_T_RANGE:
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->IsInMap(m_creature->GetVictim()))
                return false;

            if (!m_creature->IsInRange(m_creature->GetVictim(), (float)event.range.minDist, (float)event.range.maxDist, true, true))
                return false;
            break;
        case EVENT_T_OOC_LOS:
            if (event.ooc_los.conditionId)
                if (Player* player = actionInvoker->GetBeneficiaryPlayer())
                    if (!sObjectMgr.IsConditionSatisfied(event.ooc_los.conditionId, player, player->GetMap(), m_creature, CONDITION_FROM_EVENTAI))
                        return false;
            break;
        case EVENT_T_SPAWNED:
            break;
        case EVENT_T_TARGET_HP:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->GetMaxHealth())
                return false;

            uint32 perc = (m_creature->GetVictim()->GetHealth() * 100) / m_creature->GetVictim()->GetMaxHealth();

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;
            break;
        }
        case EVENT_T_TARGET_CASTING:
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->IsNonMeleeSpellCasted(false, false, true))
                return false;
            break;
        case EVENT_T_FRIENDLY_HP:
        {
            if (!m_creature->IsInCombat())
                return false;

            CreatureEventAI_EventComputedData const& data = m_creature->GetMap()->GetMapDataContainer().GetEAIComputedDataMap()->find(event.event_id)->second; // always found
            Unit* pUnit = DoSelectLowestHpFriendly(float(event.friendly_hp.radius), float(event.friendly_hp.hpDeficit), event.friendly_hp.isPercent, data.friendlyHp.targetSelf);
            if (!pUnit)
                return false;

            holder.eventTarget = pUnit;
            break;
        }
        case EVENT_T_FRIENDLY_IS_CC:
        {
            if (!m_creature->IsInCombat())
                return false;

            CreatureList list = DoFindFriendlyEligibleDispel((float)event.friendly_is_cc.radius);

            // List is empty
            if (list.empty())
                return false;

            // We don't really care about the whole list, just return first available
            holder.eventTarget = *(list.begin());
            break;
        }
        case EVENT_T_FRIENDLY_MISSING_BUFF:
        {
            // 0 = Only in combat
            // 1 = Out and in combat
            // 2 = Only out of combat
            CreatureList pList;

            if (event.friendly_buff.inCombat == 0)
            {
                if (!m_creature->IsInCombat())
                    return false;
                
                pList = DoFindFriendlyMissingBuff((float)event.friendly_buff.radius, event.friendly_buff.spellId, false);
            }
            else if (event.friendly_buff.inCombat == 1)            
                pList = DoFindFriendlyMissingBuff((float)event.friendly_buff.radius, event.friendly_buff.spellId, m_creature->IsInCombat());
            else if (event.friendly_buff.inCombat == 2)
            {
                if (m_creature->IsInCombat())
                    return false;
                                
                pList = DoFindFriendlyMissingBuff((float)event.friendly_buff.radius, event.friendly_buff.spellId, true);
            }

            // List is empty
            if (pList.empty())
                return false;

            // We don't really care about the whole list, just return first available
            holder.eventTarget = *(pList.begin());
            break;
        }
        case EVENT_T_SUMMONED_UNIT:
        case EVENT_T_SUMMONED_JUST_DIED:
        case EVENT_T_SUMMONED_JUST_DESPAWN:
        {
            // Prevent event from occuring on no unit or non creatures
            if (!actionInvoker || actionInvoker->GetTypeId() != TYPEID_UNIT)
                return false;

            // Creature id doesn't match up
            if (actionInvoker->GetEntry() != event.summoned.creatureId)
                return false;
            break;
        }
        case EVENT_T_TARGET_MANA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->HasMana() || !m_creature->GetVictim()->GetMaxPower(POWER_MANA))
                return false;

            uint32 perc = (m_creature->GetVictim()->GetPower(POWER_MANA) * 100) / m_creature->GetVictim()->GetMaxPower(POWER_MANA);

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;
            break;
        }
        case EVENT_T_REACHED_HOME:
            break;
        case EVENT_T_RECEIVE_EMOTE:
            if (event.receive_emote.conditionId)
                if (Player* player = actionInvoker->GetBeneficiaryPlayer())
                    if (!sObjectMgr.IsConditionSatisfied(event.receive_emote.conditionId, player, player->GetMap(), m_creature, CONDITION_FROM_EVENTAI))
                        return false;

            break;
        case EVENT_T_AURA:
        {
            SpellAuraHolder* auraHolder = m_creature->GetSpellAuraHolder(event.buffed.spellId);
            if (!auraHolder || auraHolder->GetStackAmount() < event.buffed.amount)
                return false;
            break;
        }
        case EVENT_T_TARGET_AURA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim())
                return false;

            SpellAuraHolder* auraHolder = m_creature->GetVictim()->GetSpellAuraHolder(event.buffed.spellId);
            if (!auraHolder || auraHolder->GetStackAmount() < event.buffed.amount)
                return false;
            break;
        }
        case EVENT_T_MISSING_AURA:
        {
            SpellAuraHolder* auraHolder = m_creature->GetSpellAuraHolder(event.buffed.spellId);
            if (auraHolder && auraHolder->GetStackAmount() >= event.buffed.amount)
                return false;
            break;
        }
        case EVENT_T_TARGET_MISSING_AURA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim())
                return false;

            SpellAuraHolder* auraHolder = m_creature->GetVictim()->GetSpellAuraHolder(event.buffed.spellId);
            if (auraHolder && auraHolder->GetStackAmount() >= event.buffed.amount)
                return false;
            break;
        }
        case EVENT_T_RECEIVE_AI_EVENT:
            break;
        case EVENT_T_ENERGY:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetMaxPower(POWER_ENERGY))
                return false;

            uint32 perc = (m_creature->GetPower(POWER_ENERGY) * 100) / m_creature->GetMaxPower(POWER_ENERGY);

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;
            break;
        }
        case EVENT_T_SELECT_ATTACKING_TARGET:
        {
            SelectAttackingTargetParams parameters;
            parameters.range.minRange = event.selectTarget.minRange;
            parameters.range.maxRange = event.selectTarget.maxRange;
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_RANGE_RANGE, parameters))
                holder.eventTarget = target;
            else
                return false;
            break;
        }
        case EVENT_T_FACING_TARGET:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim())
                return false;

            // Creature expected in back of target (melee range)
            if (event.facingTarget.backOrFront == 0 && !m_creature->GetVictim()->isInBackInMap(m_creature, 5.0f))
                return false;
            // Creature expected in front of target (melee range)
            if (event.facingTarget.backOrFront == 1 && !m_creature->GetVictim()->isInFrontInMap(m_creature, 5.0f))
                return false;
            break;
        }
        case EVENT_T_DEATH_PREVENTED:
            break;
        case EVENT_T_TARGET_NOT_REACHABLE:
            if (!m_creature->GetVictim() || !IsCombatMovement() || m_creature->GetMotionMaster()->GetCurrent()->IsReachable())
                return false;
            break;
        default:
            sLog.outErrorEventAI("Creature %u using Event %u has invalid Event Type(%u), missing from ProcessEvent() Switch.", m_creature->GetEntry(), holder.event.event_id, holder.event.event_type);
            return false;
    }

    return true;
}

void GetRepeatTimers(CreatureEventAIHolder& holder, uint32& repeatMin, uint32& repeatMax)
{
    switch (holder.event.event_type)
    {
        // 0th and 1st index
        case EVENT_T_KILL:
        case EVENT_T_TARGET_CASTING: repeatMin = holder.event.raw.params[0]; repeatMax = holder.event.raw.params[1]; return;
        // 1st and 2nd index
        case EVENT_T_SUMMONED_UNIT:
        case EVENT_T_SUMMONED_JUST_DIED:
        case EVENT_T_SUMMONED_JUST_DESPAWN: repeatMin = holder.event.raw.params[1]; repeatMax = holder.event.raw.params[2]; return;
        // Most events are on 2nd and 3rd index - TODO: completely unify this
        default: repeatMin = holder.event.raw.params[2]; repeatMax = holder.event.raw.params[3]; return;
    }
}

void CreatureEventAI::ResetEvent(CreatureEventAIHolder& holder)
{
    if (IsTimerBasedEvent(holder.event.event_type))
    {
        uint32 repeatMin, repeatMax;
        GetRepeatTimers(holder, repeatMin, repeatMax);
        holder.UpdateRepeatTimer(m_creature, repeatMin, repeatMax);
    }

    // Disable non-repeatable events
    if (IsRepeatableEvent(holder.event.event_type) && !(holder.event.event_flags & EFLAG_REPEATABLE))
        holder.enabled = false;
}

void CreatureEventAI::CheckAndReadyEventForExecution(CreatureEventAIHolder& holder, Unit* actionInvoker, Unit* AIEventSender)
{
    if (CheckEvent(holder, actionInvoker, AIEventSender))
    {
        holder.inProgress = true;
        m_creatureEventAITempList[m_depth].push_back(holder);
    }
}

bool CreatureEventAI::ProcessEvent(CreatureEventAIHolder& holder, Unit* actionInvoker, Unit* AIEventSender /*=nullptr*/)
{
    bool actionSuccess = false;
    if (holder.event.event_flags & EFLAG_COMBAT_ACTION && !CanExecuteCombatAction())
    {
        holder.inProgress = false;
        return false;
    }

    uint32 rnd = urand();

    // Reset timer if roll failed
    if (holder.event.event_chance <= rnd % 100)
    {
        ResetEvent(holder);
        holder.inProgress = false;
        return false;
    }

    // Process actions, normal case
    rnd = urand();
    if (!(holder.event.event_flags & EFLAG_RANDOM_ACTION))
    {
        actionSuccess = ProcessAction(holder.event.action[0], rnd, holder.event.event_id, actionInvoker, AIEventSender, holder.eventTarget);

        if (!(holder.event.event_flags & EFLAG_COMBAT_ACTION) || actionSuccess)
            for (uint32 j = 1; j < MAX_ACTIONS; ++j)
                ProcessAction(holder.event.action[j], rnd, holder.event.event_id, actionInvoker, AIEventSender, holder.eventTarget);
    }
    // Process actions, random case
    else
    {
        // amount of real actions
        uint32 count = 0;
        for (auto& j : holder.event.action)
            if (j.type != ACTION_T_NONE)
                ++count;

        if (count)
        {
            // select action number from found amount
            uint32 idx = rnd % count;

            // find selected action, skipping not used
            uint32 j = 0;
            for (; ; ++j)
            {
                if (holder.event.action[j].type != ACTION_T_NONE)
                {
                    if (!idx)
                        break;
                    --idx;
                }
            }

            rnd = urand(); // need to randomize again to prevent always same result when both event and action are randomized
            actionSuccess = ProcessAction(holder.event.action[j], rnd, holder.event.event_id, actionInvoker, AIEventSender, holder.eventTarget);
        }
    }

    if (!(holder.event.event_flags & EFLAG_COMBAT_ACTION) || actionSuccess)
        ResetEvent(holder);

    holder.inProgress = false;
    return true;
}

bool CreatureEventAI::ProcessAction(CreatureEventAI_Action const& action, uint32 rnd, uint32 eventId, Unit* actionInvoker, Unit* AIEventSender, Unit* eventTarget)
{
    if (action.type == ACTION_T_NONE)
        return false;

    DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "%s: Process action %u (script %u) triggered for %s (invoked by %s)", GetAIName().data(),
                     action.type, eventId, m_creature->GetGuidStr().c_str(), actionInvoker ? actionInvoker->GetGuidStr().c_str() : "<no invoker>");

    bool failedTargetSelection = false;
    switch (action.type)
    {
        case ACTION_T_TEXT:
        case ACTION_T_CHANCED_TEXT:
        {
            if (!action.text.TextId[0])
                return false;

            int32 textId = 0;

            if (action.type == ACTION_T_TEXT)
            {
                if (action.text.TextId[1] && action.text.TextId[2])
                    textId = action.text.TextId[rnd % 3];
                else if (action.text.TextId[1] && (rnd % 2))
                    textId = action.text.TextId[1];
                else
                    textId = action.text.TextId[0];
            }
            // ACTION_T_CHANCED_TEXT, chance hits
            else if ((rnd % 100) < action.chanced_text.chance)
            {
                if (action.chanced_text.TextId[0] && action.chanced_text.TextId[1])
                    textId = action.chanced_text.TextId[rnd % 2];
                else
                    textId = action.chanced_text.TextId[0];
            }

            if (textId)
            {
                Unit* target = nullptr;

                if (actionInvoker)
                {
                    if (actionInvoker->GetTypeId() == TYPEID_PLAYER)
                        target = actionInvoker;
                    else if (Unit* owner = actionInvoker->GetOwner())
                    {
                        if (owner->GetTypeId() == TYPEID_PLAYER)
                            target = owner;
                    }
                }
                else
                {
                    target = m_creature->GetVictim();
                    if (target && target->GetTypeId() != TYPEID_PLAYER)
                    {
                        Unit* owner = target->GetOwner();
                        if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                            target = owner;
                    }
                }

                if (!DoDisplayText(m_creature, textId, target))
                    sLog.outErrorEventAI("Error attempting to display text %i, used by script %u", textId, eventId);
            }
            break;
        }
        case ACTION_T_SET_FACTION:
        {
            if (action.set_faction.factionId)
                m_creature->SetFactionTemporary(action.set_faction.factionId, action.set_faction.factionFlags);
            else                                            // no id provided, assume reset and then use default
                m_creature->ClearTemporaryFaction();

            break;
        }
        case ACTION_T_MORPH_TO_ENTRY_OR_MODEL:
        {
            if (action.morph.creatureId || action.morph.modelId)
            {
                // set model based on entry from creature_template
                if (action.morph.creatureId)
                {
                    if (CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(action.morph.creatureId))
                    {
                        uint32 display_id = Creature::ChooseDisplayId(ci);
                        m_creature->SetDisplayId(display_id);
                    }
                }
                // if no param1, then use value from param2 (modelId)
                else
                    m_creature->SetDisplayId(action.morph.modelId);
            }
            else
                m_creature->DeMorph();
            break;
        }
        case ACTION_T_SOUND:
            m_creature->PlayDirectSound(action.sound.soundId);
            break;
        case ACTION_T_EMOTE:
            m_creature->HandleEmote(action.emote.emoteId);
            break;
        case ACTION_T_RANDOM_SOUND:
        {
            int32 temp = GetRandActionParam(rnd, action.random_sound.soundId1, action.random_sound.soundId2, action.random_sound.soundId3);
            if (temp >= 0)
                m_creature->PlayDirectSound(temp);
            break;
        }
        case ACTION_T_RANDOM_EMOTE:
        {
            int32 temp = GetRandActionParam(rnd, action.random_emote.emoteId1, action.random_emote.emoteId2, action.random_emote.emoteId3);
            if (temp >= 0)
                m_creature->HandleEmote(temp);
            break;
        }
        case ACTION_T_CAST:
        {
            uint32 selectFlags = 0;
            uint32 spellId = 0;
            if (!(action.cast.castFlags & (CAST_TRIGGERED | CAST_FORCE_CAST | CAST_FORCE_TARGET_SELF)))
            {
                spellId = action.cast.spellId;
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
                if (!spellInfo)
                    return false;
                if (action.cast.castFlags & CAST_PLAYER_ONLY)
                    selectFlags |= SELECT_FLAG_PLAYER;
                if (action.cast.castFlags & CAST_AURA_NOT_PRESENT)
                    selectFlags |= SELECT_FLAG_NOT_AURA;
                if (action.cast.castFlags & CAST_TARGET_CASTING)
                    selectFlags |= SELECT_FLAG_CASTING;
            }

            Unit* target = GetTargetByType(action.cast.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection, spellId, selectFlags);
            if (failedTargetSelection)
                return false;
            uint32 castFlags = action.cast.castFlags &~ (CAST_MAIN_SPELL | CAST_PLAYER_ONLY | CAST_DISTANCE_YOURSELF);
            CanCastResult castResult = DoCastSpellIfCan(target, action.cast.spellId, action.cast.castFlags);

            // If spellcast fails, action did not succeed
            if (castResult != CAST_OK)
                return false;

            break;
        }
        case ACTION_T_SPAWN:
        {
            Unit* target = GetTargetByType(action.summon.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target && failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_SPAWN (%u), target-type %u", eventId, action.type, action.summon.target);

            Creature* pCreature;
            if (action.summon.duration)
                pCreature = m_creature->SummonCreature(action.summon.creatureId, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, action.summon.duration);
            else
                pCreature = m_creature->SummonCreature(action.summon.creatureId, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 0);

            if (!pCreature)
                sLog.outErrorEventAI("Failed to spawn creature %u. Spawn event %d is on creature %d", action.summon.creatureId, eventId, m_creature->GetEntry());
            else if (action.summon.target != TARGET_T_SELF && target)
                pCreature->AI()->AttackStart(target);
            break;
        }
        case ACTION_T_THREAT_SINGLE:
            if (Unit* target = GetTargetByType(action.threat_single.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection))
            {
                if (action.threat_single.isDirect)
                    m_creature->getThreatManager().addThreat(target, action.threat_single.value);
                else
                    m_creature->getThreatManager().modifyThreatPercent(target, action.threat_single.value);
            }
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_THREAT_SINGLE(%u), target-type %u", eventId, action.type, action.threat_single.target);
            break;
        case ACTION_T_THREAT_ALL_PCT:
        {
            ThreatList const& threatList = m_creature->getThreatManager().getThreatList();
            for (auto i : threatList)
                if (Unit* Temp = m_creature->GetMap()->GetUnit(i->getUnitGuid()))
                    m_creature->getThreatManager().modifyThreatPercent(Temp, action.threat_all_pct.percent);
            break;
        }
        case ACTION_T_QUEST_EVENT:
        {
            if (Unit* target = GetTargetByType(action.quest_event.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection))
            {
                if (target->GetTypeId() == TYPEID_PLAYER)
                {
                    Player* playerTarget = static_cast<Player*>(target);
                    if (action.quest_event.rewardGroup)
                        playerTarget->RewardPlayerAndGroupAtEventExplored(action.quest_event.questId, m_creature);
                    else
                        playerTarget->AreaExploredOrEventHappens(action.quest_event.questId);
                }
            }
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_QUEST_EVENT(%u), target-type %u", eventId, action.type, action.quest_event.target);
            break;
        }
        case ACTION_T_CAST_EVENT:
            if (Unit* target = GetTargetByType(action.cast_event.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection, 0, SELECT_FLAG_PLAYER))
            {
                if (target->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)target)->CastedCreatureOrGO(action.cast_event.creatureId, m_creature->GetObjectGuid(), action.cast_event.spellId);
            }
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_CST_EVENT(%u), target-type %u", eventId, action.type, action.cast_event.target);
            break;
        case ACTION_T_SET_UNIT_FIELD:
        {
            Unit* target = GetTargetByType(action.set_unit_field.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);

            // not allow modify important for integrity object fields
            if (action.set_unit_field.field < OBJECT_END || action.set_unit_field.field >= UNIT_END)
                return false;

            if (target)
                target->SetUInt32Value(action.set_unit_field.field, action.set_unit_field.value);
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_SET_UNIT_FIELD(%u), target-type %u", eventId, action.type, action.set_unit_field.target);

            break;
        }
        case ACTION_T_SET_UNIT_FLAG:
            if (Unit* target = GetTargetByType(action.unit_flag.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection))
                target->SetFlag(UNIT_FIELD_FLAGS, action.unit_flag.value);
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_SET_UNIT_FLAG(%u), target-type %u", eventId, action.type, action.unit_flag.target);
            break;
        case ACTION_T_REMOVE_UNIT_FLAG:
            if (Unit* target = GetTargetByType(action.unit_flag.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection))
                target->RemoveFlag(UNIT_FIELD_FLAGS, action.unit_flag.value);
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_REMOVE_UNIT_FLAG(%u), target-type %u", eventId, action.type, action.unit_flag.target);
            break;
        case ACTION_T_AUTO_ATTACK:
            SetMeleeEnabled(action.auto_attack.state != 0);
            break;
        case ACTION_T_COMBAT_MOVEMENT:
        {
            bool hasCombatMovement = !m_creature->hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);

            // ignore no affect case
            if (hasCombatMovement == (action.combat_movement.state != 0) || m_creature->IsNonMeleeSpellCasted(false))
                return false;

            SetCombatMovement(action.combat_movement.state != 0, true);

            if (hasCombatMovement && action.combat_movement.melee && m_creature->IsInCombat() && m_creature->GetVictim())
                m_creature->SendMeleeAttackStart(*m_creature->GetVictim());
            else if (action.combat_movement.melee && m_creature->IsInCombat() && m_creature->GetVictim())
                m_creature->SendMeleeAttackStop(*m_creature->GetVictim());
            break;
        }
        case ACTION_T_SET_PHASE:
            m_Phase = action.set_phase.phase;
            DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "%s: ACTION_T_SET_PHASE - script %u for %s, phase is now %u", GetAIName().data(), eventId, m_creature->GetGuidStr().c_str(), m_Phase);
            break;
        case ACTION_T_INC_PHASE:
        {
            int32 new_phase = int32(m_Phase) + action.set_inc_phase.step;
            if (new_phase < 0)
            {
                sLog.outErrorEventAI("Event %d decrease Phase under 0. CreatureEntry = %d", eventId, m_creature->GetEntry());
                m_Phase = 0;
            }
            else if (new_phase >= MAX_PHASE)
            {
                sLog.outErrorEventAI("Event %d incremented Phase above %u. Phase mask cannot be used with phases past %u. CreatureEntry = %d", eventId, MAX_PHASE - 1, MAX_PHASE - 1, m_creature->GetEntry());
                m_Phase = MAX_PHASE - 1;
            }
            else
                m_Phase = new_phase;

            DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "%s: ACTION_T_INC_PHASE - script %u for %s, phase is now %u", GetAIName().data(), eventId, m_creature->GetGuidStr().c_str(), m_Phase);
            break;
        }
        case ACTION_T_EVADE:
            if (action.evade.combatOnly)
                m_creature->CombatStopWithPets(true);
            else
                EnterEvadeMode();
            break;
        case ACTION_T_FLEE_FOR_ASSIST:
            if (!DoFlee())
                return false;
            break;
        case ACTION_T_QUEST_EVENT_ALL:
            if (action.quest_event_all.useThreatList)
            {
                ThreatList const& threatList = m_creature->getThreatManager().getThreatList();
                for (auto i : threatList)
                    if (Player* temp = m_creature->GetMap()->GetPlayer(i->getUnitGuid()))
                        temp->RewardPlayerAndGroupAtEventExplored(action.quest_event_all.questId, m_creature);
            }
            else if (actionInvoker && actionInvoker->GetTypeId() == TYPEID_PLAYER)
                ((Player*)actionInvoker)->RewardPlayerAndGroupAtEventExplored(action.quest_event_all.questId, m_creature);
            break;
        case ACTION_T_CAST_EVENT_ALL:
        {
            ThreatList const& threatList = m_creature->getThreatManager().getThreatList();
            for (auto i : threatList)
                if (Player* temp = m_creature->GetMap()->GetPlayer(i->getUnitGuid()))
                    temp->CastedCreatureOrGO(action.cast_event_all.creatureId, m_creature->GetObjectGuid(), action.cast_event_all.spellId);
            break;
        }
        case ACTION_T_REMOVEAURASFROMSPELL:
            if (Unit* target = GetTargetByType(action.remove_aura.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection))
                target->RemoveAurasDueToSpell(action.remove_aura.spellId);
            else if (failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_REMOVEAURASFROMSPELL(%u), target-type %u", eventId, action.type, action.remove_aura.target);
            break;
        case ACTION_T_RANGED_MOVEMENT:
            m_attackDistance = (float)action.ranged_movement.distance;
            m_attackAngle = action.ranged_movement.angle / 180.0f * M_PI_F;

            if (!m_creature->hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT | UNIT_STAT_CAN_NOT_REACT))
            {
                if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
                {
                    // Drop current movement gen
                    m_creature->GetMotionMaster()->Clear(false);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim(), m_attackDistance, m_attackAngle);
                }
            }
            break;
        case ACTION_T_RANDOM_PHASE:
            m_Phase = GetRandActionParam(rnd, action.random_phase.phase1, action.random_phase.phase2, action.random_phase.phase3);
            DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "%s: ACTION_T_RANDOM_PHASE - script %u for %s, phase is now %u", GetAIName().data(), eventId, m_creature->GetGuidStr().c_str(), m_Phase);
            break;
        case ACTION_T_RANDOM_PHASE_RANGE:
            if (action.random_phase_range.phaseMax > action.random_phase_range.phaseMin)
                m_Phase = rnd % (action.random_phase_range.phaseMax - action.random_phase_range.phaseMin + 1) + action.random_phase_range.phaseMin;
            else
                sLog.outErrorEventAI("ACTION_T_RANDOM_PHASE_RANGE cannot have Param2 <= Param1. Divide by Zero. Event = %d. CreatureEntry = %d", eventId, m_creature->GetEntry());
            break;
        case ACTION_T_SUMMON_ID:
        {
            Unit* target = GetTargetByType(action.summon_id.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target && failedTargetSelection)
                sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_SUMMON_ID(%u), target-type %u", eventId, action.type, action.summon_id.target);

            CreatureEventAI_Summon_Map::const_iterator i = sEventAIMgr.GetCreatureEventAISummonMap().find(action.summon_id.spawnId);
            if (i == sEventAIMgr.GetCreatureEventAISummonMap().end())
            {
                sLog.outErrorEventAI("Failed to spawn creature %u. Summon map index %u does not exist. EventID %d. CreatureID %d", action.summon_id.creatureId, action.summon_id.spawnId, eventId, m_creature->GetEntry());
                return false;
            }

            Creature* pCreature;
            const CreatureEventAI_Summon* cEventAI = &i->second;
            if (i->second.SpawnTimeSecs)
                pCreature = m_creature->SummonCreature(action.summon_id.creatureId, cEventAI->position_x, cEventAI->position_y, cEventAI->position_z, cEventAI->orientation, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, cEventAI->SpawnTimeSecs);
            else
                pCreature = m_creature->SummonCreature(action.summon_id.creatureId, cEventAI->position_x, cEventAI->position_y, cEventAI->position_z, cEventAI->orientation, TEMPSPAWN_TIMED_OOC_DESPAWN, 0);

            if (!pCreature)
                sLog.outErrorEventAI("Failed to spawn creature %u. EventId %d.Creature %d", action.summon_id.creatureId, eventId, m_creature->GetEntry());
            else if (action.summon_id.target != TARGET_T_SELF && target)
                pCreature->AI()->AttackStart(target);

            break;
        }
        case ACTION_T_KILLED_MONSTER:
            // first attempt player/group who tapped creature
            if (Player* pPlayer = m_creature->GetLootRecipient())
                pPlayer->RewardPlayerAndGroupAtEventCredit(action.killed_monster.creatureId, m_creature);
            else
            {
                // if not available, use pActionInvoker
                if (Unit* pTarget = GetTargetByType(action.killed_monster.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection, 0, SELECT_FLAG_PLAYER))
                {
                    if (Player* pPlayer2 = pTarget->GetBeneficiaryPlayer())
                        pPlayer2->RewardPlayerAndGroupAtEventCredit(action.killed_monster.creatureId, m_creature);
                }
                else if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %u - nullptr target for ACTION_T_KILLED_MONSTER(%u), target-type %u", eventId, action.type, action.killed_monster.target);
            }
            break;
        case ACTION_T_SET_INST_DATA:
        {
            InstanceData* pInst = m_creature->GetInstanceData();
            if (!pInst)
            {
                sLog.outErrorEventAI("Event %d attempt to set instance data without instance script. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            pInst->SetData(action.set_inst_data.field, action.set_inst_data.value);
            break;
        }
        case ACTION_T_SET_INST_DATA64:
        {
            Unit* target = GetTargetByType(action.set_inst_data64.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target)
            {
                if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %d attempt to set instance data64 but Target == nullptr. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            InstanceData* pInst = m_creature->GetInstanceData();
            if (!pInst)
            {
                sLog.outErrorEventAI("Event %d attempt to set instance data64 without instance script. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            pInst->SetData64(action.set_inst_data64.field, target->GetObjectGuid().GetRawValue());
            break;
        }
        case ACTION_T_UPDATE_TEMPLATE:
            if (m_creature->GetEntry() == action.update_template.creatureId)
            {
                sLog.outErrorEventAI("Event %d ACTION_T_UPDATE_TEMPLATE call with param1 == current entry. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            m_creature->UpdateEntry(action.update_template.creatureId);
            break;
        case ACTION_T_DIE:
        {
            if (m_creature->IsDead())
            {
                sLog.outErrorEventAI("Event %d ACTION_T_DIE on dead creature. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }
            m_creature->Suicide();
            break;
        }
        case ACTION_T_ZONE_COMBAT_PULSE:
        {
            m_creature->SetInCombatWithZone();
            if (!m_creature->GetVictim())
                AttackClosestEnemy();
            break;
        }
        case ACTION_T_CALL_FOR_HELP:
        {
            DoCallForHelp((float)action.call_for_help.radius);
            break;
        }
        case ACTION_T_SET_SHEATH:
        {
            m_creature->SetSheath(SheathState(action.set_sheath.sheath));
            break;
        }
        case ACTION_T_FORCE_DESPAWN:
        {
            m_creature->ForcedDespawn(action.forced_despawn.msDelay);
            break;
        }
        case ACTION_T_SET_DEATH_PREVENTION:
        {
            SetDeathPrevention(action.deathPrevention.state);
            break;
        }
        case ACTION_T_MOUNT_TO_ENTRY_OR_MODEL:
        {
            if (action.mount.creatureId || action.mount.modelId)
            {
                // set model based on entry from creature_template
                if (action.mount.creatureId)
                {
                    m_creature->MountEntry(action.mount.creatureId);
                }
                // if no param1, then use value from param2 (modelId)
                else
                    m_creature->Mount(action.mount.modelId);
            }
            else
                m_creature->Unmount();

            break;
        }
        case ACTION_T_THROW_AI_EVENT:
        {
            Unit* target = GetTargetByType(action.throwEvent.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target)
            {
                if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %d attempt to throw ai event but Target == nullptr. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }
            SendAIEventAround(AIEventType(action.throwEvent.eventType), target, 0, action.throwEvent.radius);
            break;
        }
        case ACTION_T_SET_THROW_MASK:
        {
            m_throwAIEventMask = action.setThrowMask.eventTypeMask;
            break;
        }
        case ACTION_T_SET_STAND_STATE:
        {
            m_creature->SetStandState(action.setStandState.standState);
            break;
        }
        case ACTION_T_CHANGE_MOVEMENT:
        {
            if (action.changeMovement.flags & CHANGE_MOVEMENT_FLAG_AS_DEFAULT)
                m_defaultMovement = MovementGeneratorType(action.changeMovement.movementType);
            switch (action.changeMovement.movementType)
            {
                case IDLE_MOTION_TYPE:
                    m_creature->GetMotionMaster()->MoveIdle();
                    break;
                case RANDOM_MOTION_TYPE:
                    m_creature->GetMotionMaster()->MoveRandomAroundPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), float(action.changeMovement.wanderORpathID));
                    break;
                case WAYPOINT_MOTION_TYPE:
                {
                    m_creature->StopMoving();
                    m_creature->GetMotionMaster()->Clear(false, true);
                    WaypointPathOrigin origin = PATH_NO_PATH;
                    if (action.changeMovement.flags & CHANGE_MOVEMENT_FLAG_WAYPOINT_PATH)
                        origin = PATH_FROM_WAYPOINT_PATH;
                    m_creature->GetMotionMaster()->MoveWaypoint(action.changeMovement.wanderORpathID, origin);
                    break;
                }
                case PATH_MOTION_TYPE:
                {
                    m_creature->StopMoving();
                    WaypointPathOrigin origin = PATH_NO_PATH;
                    if (action.changeMovement.flags & CHANGE_MOVEMENT_FLAG_WAYPOINT_PATH)
                        origin = PATH_FROM_WAYPOINT_PATH;
                    m_creature->GetMotionMaster()->MovePath(action.changeMovement.wanderORpathID, origin);
                    break;
                }
                case LINEAR_WP_MOTION_TYPE:
                {
                    m_creature->StopMoving();
                    m_creature->GetMotionMaster()->Clear(false, true);
                    WaypointPathOrigin origin = PATH_NO_PATH;
                    if (action.changeMovement.flags & CHANGE_MOVEMENT_FLAG_WAYPOINT_PATH)
                        origin = PATH_FROM_WAYPOINT_PATH;
                    m_creature->GetMotionMaster()->MoveLinearWP(action.changeMovement.wanderORpathID, origin);
                    break;
                }
            }
            break;
        }
        case ACTION_T_SET_REACT_STATE:
        {
            // only set this on spawn event for now (need more implementation to set it in another place)
            m_reactState = ReactStates(action.setReactState.reactState);
            break;
        }
        case ACTION_T_PAUSE_WAYPOINTS:
        {
            if (action.pauseWaypoint.doPause)
                m_creature->GetMotionMaster()->PauseWaypoints(0);
            else
                m_creature->GetMotionMaster()->UnpauseWaypoints();
            break;
        }
        case ACTION_T_INTERRUPT_SPELL:
        {
            m_creature->InterruptSpell((CurrentSpellTypes)action.interruptSpell.currentSpellType);
            break;
        }
        case ACTION_T_START_RELAY_SCRIPT:
        {
            Unit* target = GetTargetByType(action.relayScript.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target)
            {
                if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %d attempt to start relay script but Target == nullptr. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            if (action.relayScript.relayId < 0)
            {
                uint32 relayId = sScriptMgr.GetRandomRelayDbscriptFromTemplate(uint32(-action.relayScript.relayId));
                if (relayId == 0)
                    break;
                m_creature->GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, relayId, target, m_creature);
            }
            else
                m_creature->GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, action.relayScript.relayId, target, m_creature);
            break;
        }
        case ACTION_T_TEXT_NEW:
        {
            Unit* target = GetTargetByType(action.textNew.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target)
            {
                if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %d attempt to display text but Target == nullptr. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }

            int32 textId = 0;

            if (action.textNew.templateId)
            {
                textId = sScriptMgr.GetRandomScriptStringFromTemplate(action.textNew.templateId);
                if (textId == 0)
                    break;
            }
            else
                textId = action.textNew.textId;

            if (!DoDisplayText(m_creature, textId, target))
                sLog.outErrorEventAI("Error attempting to display text %i, used by script %u", textId, eventId);
            break;
        }
        case ACTION_T_ATTACK_START:
        {
            Unit* target = GetTargetByType(action.attackStart.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
            if (!target)
            {
                if (failedTargetSelection)
                    sLog.outErrorEventAI("Event %d attempt to attack nullptr target. Creature %d", eventId, m_creature->GetEntry());
                return false;
            }
            AttackStart(target);
            break;
        }
        case ACTION_T_DESPAWN_GUARDIANS:
        {
            if (action.despawnGuardians.entryId)
            {
                if (Pet* guardian = m_creature->FindGuardianWithEntry(action.despawnGuardians.entryId))
                {
                    guardian->Unsummon(PET_SAVE_AS_DELETED, m_creature); // can remove pet guid from m_guardianPets
                    m_creature->RemoveGuardian(guardian);
                }
                // not throwing error otherwise because guardian might as well be dead
            }
            else
                m_creature->RemoveGuardians();
            break;
        }
        case ACTION_T_SET_RANGED_MODE:
        {
            if (!m_mainSpellInfo)
            {
                sLog.outErrorDb("EventAI: Creature entry %u has ranged mode action but no main spell.", m_creature->GetEntry());
                break;
            }

            SetRangedMode(action.rangedMode.type != TYPE_NONE, float(action.rangedMode.chaseDistance), RangeModeType(action.rangedMode.type));
            break;
        }
        case ACTION_T_SET_FACING:
        {
            if (action.setFacing.reset)
            {
                float x, y, z, o;
                if (m_creature->GetMotionMaster()->empty() || !m_creature->GetMotionMaster()->top()->GetResetPosition(*m_creature, x, y, z, o))
                    m_creature->GetRespawnCoord(x, y, z, &o);
                m_creature->SetFacingTo(o);
            }
            else
            {
                Unit* target = GetTargetByType(action.attackStart.target, actionInvoker, AIEventSender, eventTarget, failedTargetSelection);
                if (!target)
                {
                    if (failedTargetSelection)
                        sLog.outErrorEventAI("Event %d attempt to face nullptr target. Creature %d", eventId, m_creature->GetEntry());
                    return false;
                }
                m_creature->SetFacingToObject(target);
            }
            break;
        }
        case ACTION_T_SET_WALK:
            switch (action.walkSetting.type)
            {
                case WALK_DEFAULT: m_creature->SetWalk(true, true); break;
                case RUN_DEFAULT: m_creature->SetWalk(false, true); break;
                case WALK_CHASE: m_chaseRun = false; break;
                case RUN_CHASE: m_chaseRun = true; break;
            }
            break;
        case ACTION_T_SET_SPELL_SET:
        {
            m_creature->SetSpellList(action.spellSet.setId);
            break;
        }
        case ACTION_T_SET_IMMOBILIZED_STATE:
        {
            SetAIImmobilizedState(action.immobilizedState.apply, action.immobilizedState.combatOnly);
            break;
        }
        case ACTION_T_SET_DESPAWN_AGGREGATION:
        {
            m_despawnAggregationMask = action.despawnAggregation.mask;
            if ((action.despawnAggregation.mask & AGGREGATION_ENABLED) == 0)
                m_despawnGuids.clear();
            if (action.despawnAggregation.entry)
                m_entriesForDespawn.insert(action.despawnAggregation.entry);
            if (action.despawnAggregation.entry2)
                m_entriesForDespawn.insert(action.despawnAggregation.entry2);
            break;
        }
        case ACTION_T_SET_IMMUNITY_SET:
        {
            m_creature->UpdateImmunitiesSet(action.immunitySet.setId);
            break;
        }
        case ACTION_T_SET_FOLLOW_MOVEMENT:
            SetFollowMovement(action.followMovement.state != 0);
            break;
        default:
            sLog.outError("%s::ProcessAction(): action(%u) not implemented", GetAIName().data(), static_cast<uint32>(action.type));
            return false;
    }

    return true;
}

void CreatureEventAI::JustRespawned()                       // NOTE that this is called from the AI's constructor as well
{
    CreatureAI::JustRespawned();
    InitAI();
    if (m_creature->IsNoAggroOnSight())
        SetReactState(REACT_DEFENSIVE);
    else
        SetReactState(REACT_AGGRESSIVE);
    if (m_creature->GetSettings().HasFlag(CreatureStaticFlags2::SPAWN_DEFENSIVE))
        SetReactState(REACT_DEFENSIVE);
    else if (m_creature->GetSettings().HasFlag(CreatureStaticFlags::IGNORE_COMBAT))
        m_creature->SetCanEnterCombat(false);
    m_EventUpdateTime = EVENT_UPDATE_TIME;
    m_EventDiff = 0;
    m_throwAIEventStep = 0;
    m_LastSpellMaxRange = 0;

    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        CreatureEventAI_Event const& event = i.event;
        switch (event.event_type)
        {
            // Handle Spawned Events
            case EVENT_T_SPAWNED:
                if (SpawnedEventConditionsCheck(i.event))
                    CheckAndReadyEventForExecution(i);
                break;
            case EVENT_T_TIMER_IN_COMBAT:
            case EVENT_T_TIMER_OOC:
            case EVENT_T_TIMER_GENERIC:
                if (i.UpdateRepeatTimer(m_creature, i.event.timer.initialMin, i.event.timer.initialMax))
                    i.enabled = true;
                break;
            default: // reset all events with initialMin/Max here
                i.enabled = true;
                i.timer = 0;
                break;
        }
    }
    ProcessEvents();
}

void CreatureEventAI::Reset()
{
    m_EventUpdateTime = EVENT_UPDATE_TIME;
    m_EventDiff = 0;
    m_throwAIEventStep = 0;
    m_LastSpellMaxRange = 0;
    m_currentRangedMode = m_rangedMode;
    m_attackDistance = m_chaseDistance;

    // reset AI state
    SetAIOrder(ORDER_NONE);
    SetCombatScriptStatus(false);
    ClearCombatOnlyRoot();

    // Reset all events to enabled
    for (auto& i : m_CreatureEventAIList)
    {
        CreatureEventAI_Event const& event = i.event;
        switch (event.event_type)
        {
            // Dont reset any combat timers
            case EVENT_T_TIMER_IN_COMBAT:
            case EVENT_T_TIMER_GENERIC:
            case EVENT_T_AGGRO:
                break;
            case EVENT_T_TIMER_OOC:
                if (i.UpdateRepeatTimer(m_creature, event.timer.initialMin, event.timer.initialMax))
                    i.enabled = true;
                break;
            default: // reset all events here, was previously done on enter combat
                i.enabled = true;
                i.timer = 0;
                break;
        }
    }
}

void CreatureEventAI::JustReachedHome()
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_REACHED_HOME)
            CheckAndReadyEventForExecution(i);
    }
    ProcessEvents();

    Reset();
}

void CreatureEventAI::EnterEvadeMode()
{
    UnitAI::EnterEvadeMode();

    // Handle Evade events
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_EVADE)
            CheckAndReadyEventForExecution(i);
    }
    ProcessEvents();

    if ((m_despawnAggregationMask & AGGREGATION_EVADE) != 0)
        DespawnGuids(m_despawnGuids);
}

void CreatureEventAI::JustDied(Unit* killer)
{
    CreatureAI::JustDied(killer);
    Reset();

    if (m_throwAIEventMask & (1 << AI_EVENT_JUST_DIED))
        SendAIEventAround(AI_EVENT_JUST_DIED, killer, 0, AIEVENT_DEFAULT_THROW_RADIUS);

    // Handle On Death events
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_DEATH)
            CheckAndReadyEventForExecution(i, killer);
    }
    ProcessEvents(killer);

    // reset phase after any death state events
    m_Phase = 0;

    if ((m_despawnAggregationMask & AGGREGATION_DEATH) != 0)
        DespawnGuids(m_despawnGuids);
}

void CreatureEventAI::KilledUnit(Unit* victim)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_KILL)
            CheckAndReadyEventForExecution(i, victim);
    }
    ProcessEvents(victim);
}

void CreatureEventAI::JustSummoned(Creature* summoned)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_SUMMONED_UNIT)
            CheckAndReadyEventForExecution(i, summoned);
    }
    ProcessEvents(summoned);
    if ((m_despawnAggregationMask & AGGREGATION_ENABLED) != 0)
        if (m_entriesForDespawn.empty() || m_entriesForDespawn.find(summoned->GetEntry()) != m_entriesForDespawn.end())
            m_despawnGuids.push_back(summoned->GetObjectGuid());
}

void CreatureEventAI::SummonedCreatureJustDied(Creature* summoned)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_SUMMONED_JUST_DIED)
            CheckAndReadyEventForExecution(i, summoned);
    }
    ProcessEvents(summoned);
}

void CreatureEventAI::SummonedCreatureDespawn(Creature* summoned)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.event.event_type == EVENT_T_SUMMONED_JUST_DESPAWN)
            CheckAndReadyEventForExecution(i, summoned);
    }
    ProcessEvents(summoned);
}

void CreatureEventAI::ReceiveAIEvent(AIEventType eventType, Unit* sender, Unit* invoker, uint32 /*miscValue*/)
{
    MANGOS_ASSERT(sender);

    IncreaseDepthIfNecessary();
    for (auto& itr : m_CreatureEventAIList)
    {
        if (itr.event.event_type == EVENT_T_RECEIVE_AI_EVENT &&
                itr.event.receiveAIEvent.eventType == uint32(eventType) && (!itr.event.receiveAIEvent.senderEntry || itr.event.receiveAIEvent.senderEntry == sender->GetEntry()))
            CheckAndReadyEventForExecution(itr, invoker, sender);
    }
    ProcessEvents(invoker, sender);
}

void CreatureEventAI::CorpseRemoved(uint32& respawnDelay)
{
    CreatureAI::CorpseRemoved(respawnDelay);
    // can happen due to forced despawn of allies
    if ((m_despawnAggregationMask & AGGREGATION_EVADE) != 0)
        DespawnGuids(m_despawnGuids);
}

void CreatureEventAI::OnSpellCast(SpellEntry const* spellInfo, Unit* target)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
        if (i.event.event_type == EVENT_T_SPELL_CAST)
            // If spell id matches (or no spell id) & if spell school matches (or no spell school)
            if (spellInfo->Id == i.event.spellCast.spellId)
                CheckAndReadyEventForExecution(i, target);

    ProcessEvents(target);
}

void CreatureEventAI::EnterCombat(Unit* enemy)
{
    CreatureAI::EnterCombat(enemy);
    // Check for on combat start events
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
    {
        CreatureEventAI_Event const& event = i.event;
        switch (event.event_type)
        {
            case EVENT_T_AGGRO:
                i.enabled = true;
                CheckAndReadyEventForExecution(i, enemy);
                break;
            // Reset all in combat timers
            case EVENT_T_TIMER_IN_COMBAT:
                if (i.UpdateRepeatTimer(m_creature, event.timer.initialMin, event.timer.initialMax))
                    i.enabled = true;
                break;
            // Reset some special combat timers using repeatMin/Max
            case EVENT_T_FRIENDLY_HP:
            case EVENT_T_FRIENDLY_IS_CC:
            case EVENT_T_FRIENDLY_MISSING_BUFF:
            case EVENT_T_SELECT_ATTACKING_TARGET:
                if (i.UpdateRepeatTimer(m_creature, event.timer.repeatMin, event.timer.repeatMax))
                    i.enabled = true;
                break;
            default:
                break;
        }
    }
    ProcessEvents(enemy);

    m_EventUpdateTime = EVENT_UPDATE_TIME;
    m_EventDiff = 0;
}

void CreatureEventAI::MoveInLineOfSight(Unit* who)
{
    // Check for OOC LOS Event
    IncreaseDepthIfNecessary();
    if (m_HasOOCLoSEvent && !m_creature->GetVictim())
    {
        for (auto& itr : m_CreatureEventAIList)
        {
            if (itr.event.event_type == EVENT_T_OOC_LOS)
            {
                // can trigger if closer than fMaxAllowedRange
                float fMaxAllowedRange = (float)itr.event.ooc_los.maxRange;

                // who must be player type if this option is turned on
                if (!itr.event.ooc_los.playerOnly || who->GetTypeId() == TYPEID_PLAYER)
                {
                    // if friendly event && who is not hostile OR hostile event && who is hostile
                    if ((itr.event.ooc_los.noHostile && !m_creature->IsEnemy(who)) ||
                            ((!itr.event.ooc_los.noHostile) && m_creature->IsEnemy(who)))
                    {
                        // if range is ok and we are actually in LOS
                        if (m_creature->IsWithinDistInMap(who, fMaxAllowedRange) && m_creature->IsWithinLOSInMap(who))
                            CheckAndReadyEventForExecution(itr, who);
                    }
                }
            }
        }
        ProcessEvents(who);
    }

    UnitAI::MoveInLineOfSight(who);
}

void CreatureEventAI::SpellHit(Unit* unit, const SpellEntry* spellInfo)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
        if (i.event.event_type == EVENT_T_SPELLHIT)
            // If spell id matches (or no spell id) & if spell school matches (or no spell school)
            if (!i.event.spell_hit.spellId || spellInfo->Id == i.event.spell_hit.spellId)
                if (GetSchoolMask(spellInfo->School) & i.event.spell_hit.schoolMask)
                    CheckAndReadyEventForExecution(i, unit);

    ProcessEvents(unit);
}

void CreatureEventAI::SpellHitTarget(Unit* target, const SpellEntry* spellInfo)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
        if (i.event.event_type == EVENT_T_SPELLHIT_TARGET)
            // If spell id matches (or no spell id) & if spell school matches (or no spell school)
            if (!i.event.spell_hit_target.spellId || spellInfo->Id == i.event.spell_hit_target.spellId)
                if (GetSchoolMask(spellInfo->School) & i.event.spell_hit_target.schoolMask)
                    CheckAndReadyEventForExecution(i, target);

    ProcessEvents(target);
}

inline uint32 CreatureEventAI::GetRandActionParam(uint32 rnd, uint32 param1, uint32 param2, uint32 param3) const
{
    switch (rnd % 3)
    {
        case 0: return param1;
        case 1: return param2;
        case 2: return param3;
    }
    return 0;
}

inline int32 CreatureEventAI::GetRandActionParam(uint32 rnd, int32 param1, int32 param2, int32 param3) const
{
    switch (rnd % 3)
    {
        case 0: return param1;
        case 1: return param2;
        case 2: return param3;
    }
    return 0;
}

inline Unit* CreatureEventAI::GetTargetByType(uint32 target, Unit* actionInvoker, Unit* AIEventSender, Unit* eventTarget, bool& isError, uint32 forSpellId, uint32 selectFlags) const
{
    Unit* resTarget;
    switch (target)
    {
        case TARGET_T_SELF:
            return m_creature;
        case TARGET_T_HOSTILE:
            resTarget = m_creature->GetVictim();
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_SECOND_AGGRO:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 1, forSpellId, selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_LAST_AGGRO:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_BOTTOMAGGRO, 0, forSpellId, selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_RANDOM:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, forSpellId, selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_RANDOM_NOT_TOP:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, forSpellId, selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_RANDOM_PLAYER:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, forSpellId, SELECT_FLAG_PLAYER | selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_RANDOM_NOT_TOP_PLAYER:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, forSpellId, SELECT_FLAG_PLAYER | selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_HOSTILE_RANDOM_MANA:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, forSpellId, SELECT_FLAG_POWER_MANA | selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_NEAREST_AOE_TARGET:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_NEAREST_BY, 0, forSpellId, SELECT_FLAG_USE_EFFECT_RADIUS | selectFlags);
            if (!resTarget)
                isError = true;
            return nullptr; // Only used to check if it exists
        case TARGET_T_HOSTILE_FARTHEST_AWAY:
            resTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_FARTHEST_AWAY, 0, forSpellId, SELECT_FLAG_NOT_IN_MELEE_RANGE | selectFlags);
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_ACTION_INVOKER:
            if (!actionInvoker)
                isError = true;
            return actionInvoker;
        case TARGET_T_ACTION_INVOKER_OWNER:
            resTarget = actionInvoker ? actionInvoker->GetBeneficiary() : nullptr;
            if (!resTarget)
                isError = true;
            return resTarget;
        case TARGET_T_EVENT_SENDER:
            if (!AIEventSender)
                isError = true;
            return AIEventSender;
        case TARGET_T_SPAWNER:
        {
            if (Unit* spawner = m_creature->GetSpawner())
                return spawner;

            isError = true;
            return nullptr;
        }
        case TARGET_T_EVENT_SPECIFIC:
            if (!eventTarget)
                isError = true;
            return eventTarget;
        case TARGET_T_PLAYER_INVOKER:
        {
            if (!m_creature->HasLootRecipient())
            {
                isError = true;
                return nullptr;
            }

            return m_creature->GetOriginalLootRecipient();
        }
        case TARGET_T_PLAYER_TAPPED:
        {
            if (!m_creature->HasLootRecipient())
            {
                isError = true;
                return nullptr;
            }

            return m_creature->GetLootRecipient();
        }
        case TARGET_T_NONE:
            return nullptr;
        default:
            isError = true;
            return nullptr;
    }
}

//*********************************
//*** Functions used globally ***

void CreatureEventAI::ReceiveEmote(Player* player, uint32 textEmote)
{
    IncreaseDepthIfNecessary();
    for (auto& itr : m_CreatureEventAIList)
    {
        if (itr.event.event_type == EVENT_T_RECEIVE_EMOTE)
        {
            if (itr.event.receive_emote.emoteId != textEmote)
                continue;

            CheckAndReadyEventForExecution(itr, player);
        }
    }
    ProcessEvents(player);
}

#define HEALTH_STEPS            3

void CreatureEventAI::DamageTaken(Unit* dealer, uint32& damage, DamageEffectType damagetype, SpellEntry const* spellInfo)
{
    CreatureAI::DamageTaken(dealer, damage, damagetype, spellInfo);

    uint32 step = m_throwAIEventStep != 100 ? m_throwAIEventStep : 0;
    if (step < HEALTH_STEPS)
    {
        // Throw at 90%, 50% and 10% health
        float healthSteps[HEALTH_STEPS] = { 90.0f, 50.0f, 10.0f };
        float newHealthPercent = (m_creature->GetHealth() - damage) * 100.0f / m_creature->GetMaxHealth();
        AIEventType sendEvent[HEALTH_STEPS] = { AI_EVENT_LOST_SOME_HEALTH, AI_EVENT_LOST_HEALTH, AI_EVENT_CRITICAL_HEALTH };

        if (newHealthPercent > healthSteps[step])
            return;                                         // Not reached the next mark

        // search for highest reached mark (with actual event attached)
        for (uint32 i = HEALTH_STEPS - 1; i > step; --i)
        {
            if (newHealthPercent < healthSteps[i] && (m_throwAIEventMask & (1 << sendEvent[i])))
            {
                step = i;
                break;
            }
        }

        if (m_throwAIEventMask & (1 << sendEvent[step]))
            SendAIEventAround(sendEvent[step], dealer, 0, AIEVENT_DEFAULT_THROW_RADIUS);

        m_throwAIEventStep = step + 1;
    }
}

void CreatureEventAI::JustPreventedDeath(Unit* attacker)
{
    IncreaseDepthIfNecessary();
    for (auto& i : m_CreatureEventAIList)
        if (i.event.event_type == EVENT_T_DEATH_PREVENTED)
            // If spell id matches (or no spell id) & if spell school matches (or no spell school)
            CheckAndReadyEventForExecution(i, attacker);

    ProcessEvents(attacker);
}

void CreatureEventAI::HealedBy(Unit* healer, uint32& healedAmount)
{
    if (m_throwAIEventStep == 100)
        return;

    if (m_creature->GetHealth() + healedAmount >= m_creature->GetMaxHealth())
    {
        if (m_throwAIEventMask & (1 << AI_EVENT_GOT_FULL_HEALTH))
            SendAIEventAround(AI_EVENT_GOT_FULL_HEALTH, healer, 0, AIEVENT_DEFAULT_THROW_RADIUS);
        m_throwAIEventStep = 100;
    }
}

bool CreatureEventAI::SpawnedEventConditionsCheck(CreatureEventAI_Event const& event) const
{
    if (event.event_type != EVENT_T_SPAWNED)
        return false;

    switch (event.spawned.condition)
    {
        case SPAWNED_EVENT_ALWAY:
            // always
            return true;
        case SPAWNED_EVENT_MAP:
            // map ID check
            return m_creature->GetMapId() == event.spawned.conditionValue1;
        case SPAWNED_EVENT_ZONE:
        {
            // zone ID check
            uint32 zone, area;
            m_creature->GetZoneAndAreaId(zone, area);
            return zone == event.spawned.conditionValue1 || area == event.spawned.conditionValue1;
        }
        default:
            break;
    }

    return false;
}

void CreatureEventAI::UpdateEventTimers(const uint32 diff)
{
    // Events are only updated once every EVENT_UPDATE_TIME ms to prevent lag with large amount of events
    if (m_EventUpdateTime < diff)
    {
        m_EventDiff += diff;

        // Check for time based events
        IncreaseDepthIfNecessary();
        for (CreatureEventAIList::iterator i = m_CreatureEventAIList.begin(); i != m_CreatureEventAIList.end(); ++i)
        {
            if (i->event.event_type == EVENT_T_TARGET_NOT_REACHABLE)
            {
                CheckAndReadyEventForExecution(*i);
                continue;
            }

            // Decrement Timers
            if (i->timer)
            {
                // Do not decrement timers if event cannot trigger in this phase
                if (!(i->event.event_inverse_phase_mask & (1 << m_Phase)))
                {
                    if (i->timer > m_EventDiff)
                        i->timer -= m_EventDiff;
                    else
                        i->timer = 0;
                }
            }

            // Skip processing of events that have time remaining or are disabled
            if (!(i->enabled) || i->timer)
                continue;

            if (IsTimerExecutedEvent(i->event.event_type))
                CheckAndReadyEventForExecution(*i);
        }
        ProcessEvents();

        m_EventDiff = 0;
        m_EventUpdateTime = EVENT_UPDATE_TIME;
    }
    else
    {
        m_EventDiff += diff;
        m_EventUpdateTime -= diff;
    }
}
