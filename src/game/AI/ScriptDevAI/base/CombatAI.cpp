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
#include "MotionGenerators/MovementGenerator.h"

enum
{
    ACTION_CASTING_RESTORE = 1000,
    ACTION_ON_KILL_COOLDOWN = 1001,
};

CombatAI::CombatAI(Creature* creature, uint32 combatActions) : ScriptedAI(creature), CombatActions(combatActions), m_onKillCooldown(false), m_stopTargeting(false), m_teleportUnreachable(false)
{
    AddCustomAction(ACTION_CASTING_RESTORE, true, [&]() { HandleTargetRestoration(); });
    AddCustomAction(ACTION_ON_KILL_COOLDOWN, true, [&]() { m_onKillCooldown = false; });
}

void CombatAI::Reset()
{
    ResetAllTimers();
    m_onKillCooldown = false;
    m_storedTarget = ObjectGuid();
}

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

    if (m_teleportUnreachable)
        if (m_creature->GetVictim() && IsCombatMovement() && !m_creature->GetMotionMaster()->GetCurrent()->IsReachable())
            m_creature->CastSpell(m_creature->GetVictim(), 21727, TRIGGERED_OLD_TRIGGERED);
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

void CombatAI::AddUnreachabilityCheck()
{
    m_teleportUnreachable = true;
}

void CombatAI::UpdateAI(const uint32 diff)
{
    UpdateTimers(diff, m_creature->IsInCombat());

    if (!m_creature->SelectHostileTarget())
        return;

    ExecuteActions();

    DoMeleeAttackIfReady();
}

RangedCombatAI::RangedCombatAI(Creature* creature, uint32 combatActions) : CombatAI(creature, combatActions),
    m_rangedMode(false), m_rangedModeSetting(TYPE_NONE), m_chaseDistance(0.f), m_currentRangedMode(false),
    m_mainSpellId(0), m_mainSpellCost(0), m_mainSpellInfo(nullptr), m_mainSpellMinRange(0.f),
    m_mainAttackMask(SPELL_SCHOOL_MASK_NONE), m_distancingCooldown(false)
{
    AddCustomAction(GENERIC_ACTION_DISTANCE, true, [&]() { m_distancingCooldown = false; });
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
        m_mainAttackMask = SpellSchoolMask(m_mainAttackMask + GetSchoolMask(spellInfo->School));
        m_mainSpellInfo = spellInfo;
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

    if (m_creature->IsInCombat())
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
        m_attackDistance = m_chaseDistance;
        DoStartMovement(m_creature->GetVictim());
    }
    else
    {
        if (m_rangedModeSetting == TYPE_NO_MELEE_MODE)
            return;

        m_currentRangedMode = false;
        m_attackDistance = 0.f;
        DoStartMovement(m_creature->GetVictim());
        if (m_meleeEnabled && !m_unit->hasUnitState(UNIT_STAT_MELEE_ATTACKING))
            m_unit->MeleeAttackStart(m_unit->GetVictim());
    }
}

enum EAIPoints
{
    POINT_MOVE_DISTANCE
};

void RangedCombatAI::DistanceYourself()
{
    Unit* victim = m_creature->GetVictim();
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
    if (m_creature->GetVictim() != victim)
        return;
    if (m_distanceSpells.find(spellInfo->Id) != m_distanceSpells.end())
        DistanceYourself();
}

void RangedCombatAI::OnSpellInterrupt(SpellEntry const* spellInfo)
{
    if (m_mainSpells.find(spellInfo->Id) != m_mainSpells.end())
    {
        if (m_rangedMode && m_rangedModeSetting != TYPE_NO_MELEE_MODE && IsMainSpellPrevented(spellInfo))
        {
            // infrequently mobs have multiple main spells and only go into melee on interrupt when all are on cooldown
            if (m_mainSpells.size() > 1)
            {
                bool success = false;
                for (uint32 spellId : m_mainSpells)
                {
                    if (spellId != spellInfo->Id)
                    {
                        SpellEntry const* otherSpellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
                        if (!IsMainSpellPrevented(otherSpellInfo))
                        {
                            success = true;
                            break;
                        }
                    }
                }
                if (success)
                    return; // at least one main spell is off cooldown
            }
            SetCurrentRangedMode(false);
        }
    }
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
                    case CAST_FAIL_POWER:
                    case CAST_FAIL_TOO_CLOSE:
                    case CAST_FAIL_CAST_PREVENTED:
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
    UpdateTimers(diff, m_creature->IsInCombat());

    if (!m_creature->SelectHostileTarget())
        return;

    ExecuteActions();

    Unit* victim = m_creature->GetVictim();
    if (m_rangedMode && victim && CanExecuteCombatAction())
    {
        if (m_rangedModeSetting == TYPE_PROXIMITY || m_rangedModeSetting == TYPE_DISTANCER)
        {
            if (!m_currentRangedMode && victim->IsImmobilizedState() && IsCombatMovement() && m_mainSpellInfo && m_mainSpellCost * 2 < m_creature->GetPower(POWER_MANA) && !IsMainSpellPrevented(m_mainSpellInfo))
                DistanceYourself();
            else if (m_currentRangedMode && m_creature->CanReachWithMeleeAttack(victim))
                SetCurrentRangedMode(false);
            else if (!m_currentRangedMode && !m_creature->CanReachWithMeleeAttack(victim, 2.f) && m_mainSpellInfo && m_mainSpellCost * 2 < m_creature->GetPower(POWER_MANA) && !IsMainSpellPrevented(m_mainSpellInfo))
                SetCurrentRangedMode(true);
            else if (m_rangedModeSetting == TYPE_DISTANCER && !m_distancingCooldown)
            {
                m_distancingCooldown = true;
                ResetTimer(GENERIC_ACTION_DISTANCE, 5000);
            }
        }
        // casters only display melee animation when in ranged mode when someone is actually close enough
        if (m_currentRangedMode && m_meleeEnabled)
        {
            if (m_unit->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && !m_creature->CanReachWithMeleeAttack(victim))
                m_unit->MeleeAttackStop(m_unit->GetVictim());
            else if (!m_unit->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && m_creature->CanReachWithMeleeAttack(victim))
                m_unit->MeleeAttackStart(m_unit->GetVictim());
        }
    }

    DoMeleeAttackIfReady();
}

bool RangedCombatAI::IsMainSpellPrevented(SpellEntry const* spellInfo) const
{
    if (!m_creature->IsSpellReady(*spellInfo))
        return true;

    if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return true;
    if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
        return true;

    return false;
}
