/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#include "Entities/Creature.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"

enum
{
    ACTION_CASTING_RESTORE = 1000,
    ACTION_ON_KILL_COOLDOWN = 1001,
};

CombatAI::CombatAI(Creature* creature, uint32 combatActions) : ScriptedAI(creature, combatActions), m_onKillCooldown(false), m_stopTargeting(false)
{
    AddCustomAction(ACTION_CASTING_RESTORE, true, [&]() { HandleTargetRestoration(); });
    AddCustomAction(ACTION_ON_KILL_COOLDOWN, true, [&]() { m_onKillCooldown = false; });
}

void CombatAI::Reset()
{
    ScriptedAI::Reset();
    m_onKillCooldown = false;
    m_storedTarget = ObjectGuid();
}

void CombatAI::HandleDelayedInstantAnimation(SpellEntry const* spellInfo)
{
    m_storedTarget = m_creature->GetTarget() ? m_creature->GetTarget()->GetObjectGuid() : ObjectGuid();
    if (m_storedTarget)
        ResetTimer(ACTION_CASTING_RESTORE, 2000);
}

void CombatAI::HandleTargetRestoration()
{
    ObjectGuid guid = m_unit->GetTarget() ? m_unit->GetTarget()->GetObjectGuid() : ObjectGuid();
    if (guid != m_storedTarget || m_unit->IsNonMeleeSpellCasted(false))
    {
        m_storedTarget = ObjectGuid();
        return;
    }

    if (m_unit->GetVictim() && !GetCombatScriptStatus())
        m_unit->SetTarget(m_unit->GetVictim());
    else
        m_unit->SetTarget(nullptr);

    m_storedTarget = ObjectGuid();
}

bool CombatAI::IsTargetingRestricted()
{
    return m_stopTargeting || m_storedTarget || ScriptedAI::IsTargetingRestricted();
}

void CombatAI::OnTaunt()
{
    if (m_storedTarget)
        ResetTimer(ACTION_CASTING_RESTORE, 1); // clear on next update
}

void CombatAI::AddOnKillText(int32 text)
{
    m_onDeathTexts.push_back(text);
}

void CombatAI::KilledUnit(Unit* victim)
{
    if (!m_creature->IsAlive() || !victim->IsPlayer())
        return;

    if (!m_onKillCooldown && m_onDeathTexts.size() > 0)
    {
        m_onKillCooldown = true;
        DoScriptText(m_onDeathTexts[urand(0, m_onDeathTexts.size() - 1)], m_creature, victim);
        ResetTimer(ACTION_ON_KILL_COOLDOWN, 10000);
    }
}
