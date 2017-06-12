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

#include "ReactorAI.h"
#include "Entities/Creature.h"
#include "Maps/Map.h"
#include "Log.h"

#define REACTOR_VISIBLE_RANGE (26.46f)

int ReactorAI::Permissible(const Creature* creature)
{
    if ((creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_AGGRO) || creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void ReactorAI::MoveInLineOfSight(Unit*)
{
}

void ReactorAI::AttackStart(Unit* p)
{
    if (!p)
        return;

    if (m_creature->Attack(p, true))
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Tag unit GUID: %u (TypeId: %u) as a victim", p->GetGUIDLow(), p->GetTypeId());
        i_victimGuid = p->GetObjectGuid();
        m_creature->AddThreat(p);

        m_creature->SetInCombatWith(p);
        p->SetInCombatWith(m_creature);

        HandleMovementOnAttackStart(p);
    }
}

bool ReactorAI::IsVisible(Unit*) const
{
    return false;
}

void ReactorAI::UpdateAI(const uint32 /*time_diff*/)
{
    // update i_victimGuid if i_creature.getVictim() !=0 and changed
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    i_victimGuid = m_creature->getVictim()->GetObjectGuid();

    DoMeleeAttackIfReady();
}

void ReactorAI::EnterCombat(Unit* enemy)
{
    if (m_creature->IsCivilian() && enemy->GetTypeId() == TYPEID_PLAYER)
    {
        if (Player* pAttacker = enemy->GetBeneficiaryPlayer())
            m_creature->SendZoneUnderAttackMessage(pAttacker);
    }
}
