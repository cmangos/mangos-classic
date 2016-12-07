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

#include "GuardianAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

GuardianAI::GuardianAI(Creature* c) : CreatureEventAI(c)
{
    Unit* owner = c->GetOwner();
    MANGOS_ASSERT(owner);
}

void GuardianAI::JustRespawned()
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return;

    CreatureEventAI::JustRespawned();

    m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
}

void GuardianAI::UpdateAI(const uint32 diff)
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return;

    switch (m_reactState)
    {
        case REACT_AGGRESSIVE:
        case REACT_DEFENSIVE:
            if (!m_creature->isInCombat() && owner->isInCombat())
                AttackStart(owner->getAttackerForHelper());   // check for getAttackerForHelper() == nullpter in AttackStart()
            break;
        default:
            break;
    }

    CreatureEventAI::UpdateAI(diff);
}

int GuardianAI::Permissible(const Creature* creature)
{
    if (creature->IsPet() && ((Pet*)creature)->getPetType() == GUARDIAN_PET)
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

bool GuardianAI::ProcessEvent(CreatureEventAIHolder& pHolder, Unit* pActionInvoker, Creature* pAIEventSender /*=nullptr*/)
{
    if (!pHolder.Enabled || pHolder.Time)
        return false;

    // Check the inverse phase mask (event doesn't trigger if current phase bit is set in mask)
    if (pHolder.Event.event_inverse_phase_mask & (1 << m_Phase))
    {
        if (!IsTimerBasedEvent(pHolder.Event.event_type))
            DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "CreatureEventAI: Event %u skipped because of phasemask %u. Current phase %u", pHolder.Event.event_id, pHolder.Event.event_inverse_phase_mask, m_Phase);
        return false;
    }

    if (!IsTimerBasedEvent(pHolder.Event.event_type))
        LOG_PROCESS_EVENT;

    CreatureEventAI_Event const& event = pHolder.Event;

    // Check event conditions based on the event type, also reset events
    switch (event.event_type)
    {
        case EVENT_T_FRIENDLY_HP:
        {
            Unit* pUnit = DoSelectLowestHpFriendly((float)event.friendly_hp.radius, event.friendly_hp.hpDeficit, false);
            if (!pUnit)
                return false;

            pActionInvoker = pUnit;

            LOG_PROCESS_EVENT;
            // Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature, event.friendly_hp.repeatMin, event.friendly_hp.repeatMax);
            break;
        }
        default:
            return CreatureEventAI::ProcessEvent(pHolder, pActionInvoker, pAIEventSender);
    }

    // Disable non-repeatable events
    if (!(pHolder.Event.event_flags & EFLAG_REPEATABLE))
        pHolder.Enabled = false;

    // Store random here so that all random actions match up
    uint32 rnd = rand();

    // Return if chance for event is not met
    if (pHolder.Event.event_chance <= rnd % 100)
        return false;

    // Process actions, normal case
    if (!(pHolder.Event.event_flags & EFLAG_RANDOM_ACTION))
    {
        for (uint32 j = 0; j < MAX_ACTIONS; ++j)
            ProcessAction(pHolder.Event.action[j], rnd, pHolder.Event.event_id, pActionInvoker, pAIEventSender);
    }
    // Process actions, random case
    else
    {
        // amount of real actions
        uint32 count = 0;
        for (uint32 j = 0; j < MAX_ACTIONS; ++j)
            if (pHolder.Event.action[j].type != ACTION_T_NONE)
                ++count;

        if (count)
        {
            // select action number from found amount
            uint32 idx = rnd % count;

            // find selected action, skipping not used
            uint32 j = 0;
            for (; ; ++j)
            {
                if (pHolder.Event.action[j].type != ACTION_T_NONE)
                {
                    if (!idx)
                        break;
                    --idx;
                }
            }

            ProcessAction(pHolder.Event.action[j], rnd, pHolder.Event.event_id, pActionInvoker, pAIEventSender);
        }
    }
    return true;
}

void GuardianAI::ProcessAction(CreatureEventAI_Action const& action, uint32 rnd, uint32 EventId, Unit* pActionInvoker, Creature* pAIEventSender)
{
    if (action.type == ACTION_T_NONE)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_EVENT_AI_DEV, "GuardianAI: Process action %u (script %u) triggered for %s (invoked by %s)",
        action.type, EventId, m_creature->GetGuidStr().c_str(), pActionInvoker ? pActionInvoker->GetGuidStr().c_str() : "<no invoker>");

    CreatureEventAI::ProcessAction(action, rnd, EventId, pActionInvoker, pAIEventSender);
}

void GuardianAI::CombatStop()
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return;

    // only alive creatures that are not on transport can return to home position
    if (m_creature->isAlive())
        m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
}

void GuardianAI::EnterEvadeMode()
{
    m_creature->RemoveAllAurasOnEvade();
    m_creature->DeleteThreatList();
    m_creature->CombatStop(true);

    // Handle Evade events
    for (CreatureEventAIList::iterator i = m_CreatureEventAIList.begin(); i != m_CreatureEventAIList.end(); ++i)
    {
        if (i->Event.event_type == EVENT_T_EVADE)
            ProcessEvent(*i);
    }
}

Unit* GuardianAI::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff, bool onlyInCombat) const
{
    Unit* owner = m_creature->GetOwner();

    if (!owner)
        return nullptr;

    Unit* pUnit = nullptr;

    MaNGOS::MostHPMissingInRangeCheck u_check(m_creature, range, MinHPDiff, onlyInCombat);
    MaNGOS::UnitLastSearcher<MaNGOS::MostHPMissingInRangeCheck> searcher(pUnit, u_check);

    if (owner->GetTypeId() == TYPEID_PLAYER)
        Cell::VisitWorldObjects(m_creature, searcher, range);   // search all friendly unit including players
    else
        Cell::VisitGridObjects(m_creature, searcher, range);    // search only friendly creatures
    return pUnit;
}
