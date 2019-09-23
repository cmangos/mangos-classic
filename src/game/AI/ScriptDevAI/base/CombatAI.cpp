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

void CombatAI::ExecuteActions()
{
    if (!CanExecuteCombatAction())
        return;

    for (uint32 i = 0; i < GetCombatActionCount(); ++i)
    {
        // can be changed on any action - prevent all additional ones
        if (GetCombatScriptStatus())
            return;

        if (GetActionReadyStatus(i))
            ExecuteAction(i);
    }
}

void CombatAI::UpdateAI(const uint32 diff)
{
    UpdateTimers(diff, m_creature->isInCombat());

    if (!m_creature->SelectHostileTarget())
        return;

    ExecuteActions();

    DoMeleeAttackIfReady();
}

void RangedCombatAI::OnSpellCooldownAdded(SpellEntry const* spellInfo)
{
    CombatAI::OnSpellCooldownAdded(spellInfo);
    if (m_rangedModeSetting == TYPE_FULL_CASTER && m_mainSpells.find(spellInfo->Id) != m_mainSpells.end())
        SetCurrentRangedMode(true);
}

void RangedCombatAI::AddMainSpell(uint32 spellId)
{
    if (!m_mainSpellId) // only for first
    {
        m_mainSpellId = spellId;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(m_mainSpellId);
        m_mainSpellCost = Spell::CalculatePowerCost(spellInfo, m_creature);
        m_mainSpellMinRange = GetSpellMinRange(sSpellRangeStore.LookupEntry(spellInfo->rangeIndex));
    }
    m_mainSpells.insert(spellId);
}

void RangedCombatAI::SetRangedMode(bool state, float distance, RangeModeType type)
{
    if (m_rangedMode == state)
        return;

    m_rangedMode = state;
    m_chaseDistance = distance;
    m_rangedModeSetting = type;
    m_meleeEnabled = !state;

    if (m_creature->isInCombat())
        SetCurrentRangedMode(state);
    else
    {
        m_currentRangedMode = true;
        m_attackDistance = m_chaseDistance;
    }
}

void RangedCombatAI::SetCurrentRangedMode(bool state)
{
    if (state)
    {
        m_currentRangedMode = true;
        m_meleeEnabled = false;
        m_attackDistance = m_chaseDistance;
        m_creature->MeleeAttackStop(m_creature->getVictim());
        DoStartMovement(m_creature->getVictim());
    }
    else
    {
        if (m_rangedModeSetting == TYPE_NO_MELEE_MODE)
            return;

        m_currentRangedMode = false;
        m_meleeEnabled = true;
        m_attackDistance = 0.f;
        m_creature->MeleeAttackStart(m_creature->getVictim());
        DoStartMovement(m_creature->getVictim());
    }
}

enum EAIPoints
{
    POINT_MOVE_DISTANCE
};

void RangedCombatAI::DistanceYourself()
{
    Unit* victim = m_creature->getVictim();
    if (!victim->CanReachWithMeleeAttack(m_creature))
        return;

    if (m_mainSpellCost * 2 > m_creature->GetPower(POWER_MANA))
        return;

    float combatReach = m_creature->GetCombinedCombatReach(victim, true);
    float distance = DISTANCING_CONSTANT + std::max(combatReach * 1.5f, combatReach + m_mainSpellMinRange);
    m_creature->GetMotionMaster()->DistanceYourself(distance);
}

void RangedCombatAI::DistancingStarted()
{
    SetCombatScriptStatus(true);
}

void RangedCombatAI::DistancingEnded()
{
    SetCombatScriptStatus(false);
    if (!m_currentRangedMode)
        SetCurrentRangedMode(true);
}

void RangedCombatAI::JustStoppedMovementOfTarget(SpellEntry const* spellInfo, Unit* victim)
{
    if (m_creature->getVictim() != victim)
        return;
    if (m_distanceSpells.find(spellInfo->Id) != m_distanceSpells.end())
        DistanceYourself();
}

void RangedCombatAI::OnSpellInterrupt(SpellEntry const* spellInfo)
{
    if (m_mainSpells.find(spellInfo->Id) != m_mainSpells.end())
        if (m_rangedMode && m_rangedModeSetting != TYPE_NO_MELEE_MODE && !m_creature->IsSpellReady(*spellInfo))
            SetCurrentRangedMode(false);
}

CanCastResult RangedCombatAI::DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags)
{
    CanCastResult castResult = CombatAI::DoCastSpellIfCan(target, spellId, castFlags);
    if (m_rangedMode)
    {
        if (m_currentRangedMode)
        {
            if (m_mainSpells.find(spellId) != m_mainSpells.end())
            {
                switch (castResult)
                {
                    case CAST_FAIL_COOLDOWN:
                    case CAST_FAIL_POWER:
                    case CAST_FAIL_TOO_CLOSE:
                        SetCurrentRangedMode(false);
                        break;
                    case CAST_OK:
                    default:
                        break;
                }
            }
        }
    }
    return castResult;
}

void RangedCombatAI::UpdateAI(const uint32 diff)
{
    UpdateTimers(diff, m_creature->isInCombat());

    if (!m_creature->SelectHostileTarget())
        return;

    ExecuteActions();

    if (m_rangedMode && m_creature->getVictim() && CanExecuteCombatAction())
    {
        if (m_currentRangedMode && m_rangedModeSetting == TYPE_PROXIMITY && m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
            SetCurrentRangedMode(false);
    }

    DoMeleeAttackIfReady();
}
