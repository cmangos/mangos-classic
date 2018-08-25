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

#include "TotemAI.h"
#include "Entities/Totem.h"
#include "Entities/Creature.h"
#include "Server/DBCStores.h"
#include "Spells/SpellMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

int TotemAI::Permissible(const Creature* creature)
{
    if (creature->IsTotem())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

TotemAI::TotemAI(Creature* creature) : CreatureEventAI(creature)
{
}

void TotemAI::MoveInLineOfSight(Unit* /*who*/)
{
}

void TotemAI::EnterEvadeMode()
{
    m_creature->CombatStop(true);

    // Handle Evade events
    for (auto& i : m_CreatureEventAIList)
    {
        if (i.Event.event_type == EVENT_T_EVADE)
            ProcessEvent(i);
    }
}

void TotemAI::UpdateAI(const uint32 diff)
{
    // Events are only updated once every EVENT_UPDATE_TIME ms to prevent lag with large amount of events
    if (m_EventUpdateTime < diff)
    {
        m_EventDiff += diff;

        // Check for time based events
        for (auto& i : m_CreatureEventAIList)
        {
            // Decrement Timers
            if (i.Time)
            {
                // Do not decrement timers if event cannot trigger in this phase
                if (!(i.Event.event_inverse_phase_mask & (1 << m_Phase)))
                {
                    if (i.Time > m_EventDiff)
                        i.Time -= m_EventDiff;
                    else
                        i.Time = 0;
                }
            }

            // Skip processing of events that have time remaining or are disabled
            if (!(i.Enabled) || i.Time)
                continue;

            if (IsTimerBasedEvent(i.Event.event_type))
                ProcessEvent(i);
        }

        m_EventDiff = 0;
        m_EventUpdateTime = EVENT_UPDATE_TIME;
    }
    else
    {
        m_EventDiff += diff;
        m_EventUpdateTime -= diff;
    }

    if (getTotem().GetTotemType() != TOTEM_ACTIVE)
        return;

    if (!m_creature->isAlive() || m_creature->IsNonMeleeSpellCasted(false))
        return;

    // Search spell
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(getTotem().GetSpell());
    if (!spellInfo)
        return;

    // Get spell rangy
    SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
    float maxRange = GetSpellMaxRange(srange);

    // SPELLMOD_RANGE not applied in this place just because nonexistent range mods for attacking totems

    // pointer to appropriate target if found any
    Unit* victim = m_creature->GetMap()->GetUnit(i_victimGuid);

    // Search victim if no, not attackable, or out of range, or friendly (possible in case duel end)
    if (!victim ||
            !m_creature->CanAttackOnSight(victim) || !m_creature->IsWithinDistInMap(victim, maxRange) ||
            m_creature->CanAssist(victim) || !victim->isVisibleForOrDetect(m_creature, m_creature, false))
    {
        victim = nullptr;
        
        if (maxRange != 0.0f)
        {
            MaNGOS::NearestAttackableUnitInObjectRangeCheck u_check(m_creature, maxRange);
            MaNGOS::UnitLastSearcher<MaNGOS::NearestAttackableUnitInObjectRangeCheck> checker(victim, u_check);
            Cell::VisitAllObjects(m_creature, checker, maxRange);
        }
        else
            victim = m_creature;
    }

    // If have target
    if (victim)
    {
        // remember
        i_victimGuid = victim->GetObjectGuid();

        // attack
        m_creature->SetInFront(victim);                     // client change orientation by self
        m_creature->CastSpell(victim, getTotem().GetSpell(), TRIGGERED_NONE);
    }
    else
        i_victimGuid.Clear();
}

bool TotemAI::IsVisible(Unit* /*who*/) const
{
    return false;
}

void TotemAI::AttackStart(Unit* /*who*/)
{
}

Totem& TotemAI::getTotem() const
{
    return static_cast<Totem&>(*m_creature);
}
