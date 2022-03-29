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

#include "AI/BaseAI/CreatureAI.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "World/World.h"
#include "Entities/Creature.h"

CreatureAI::CreatureAI(Creature* creature) : CreatureAI(creature, 0) { }

CreatureAI::CreatureAI(Creature* creature, uint32 combatActions) :
    UnitAI(creature, combatActions),
    m_creature(creature),
    m_deathPrevention(false), m_deathPrevented(false)
{
    m_dismountOnAggro = !(m_creature->GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_MOUNTED_COMBAT);

    SetMeleeEnabled(!(m_creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_MELEE));
    if (m_creature->IsNoAggroOnSight())
        SetReactState(REACT_DEFENSIVE);
    if (m_creature->IsGuard() || m_unit->GetCharmInfo()) // guards and charmed targets
        m_visibilityDistance = sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER);
}

void CreatureAI::Reset()
{
    ResetAllTimers();
    m_currentRangedMode = m_rangedMode;
    m_attackDistance = m_chaseDistance;
}

void CreatureAI::EnterCombat(Unit* enemy)
{
    UnitAI::EnterCombat(enemy);
    // TODO: Monitor this condition to see if it conflicts with any pets
    if (m_creature->IsCritter() && !m_creature->IsPet() && !m_creature->IsInPanic() && enemy && enemy->IsPlayerControlled())
        DoFlee(30000);
    if (enemy && (m_creature->IsGuard() || m_creature->IsCivilian()))
    {
        // Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
        if (Player* pKiller = enemy->GetBeneficiaryPlayer())
            m_creature->SendZoneUnderAttackMessage(pKiller);
    }
}

void CreatureAI::AttackStart(Unit* who)
{
    if (!who || HasReactState(REACT_PASSIVE))
        return;

    bool targetChange = m_unit->GetVictim() != nullptr && m_unit->GetVictim() != who;
    if (m_creature->Attack(who, m_meleeEnabled))
    {
        m_creature->EngageInCombatWith(who);

        HandleMovementOnAttackStart(who, targetChange);
    }
}

void CreatureAI::DamageTaken(Unit* dealer, uint32& damage, DamageEffectType /*damageType*/, SpellEntry const* /*spellInfo*/)
{
    if (m_deathPrevention)
    {
        if (m_creature->GetHealth() <= damage)
        {
            damage = m_creature->GetHealth() - 1;
            if (!m_deathPrevented)
            {
                m_deathPrevented = true;
                JustPreventedDeath(dealer);
            }
        }        
    }
}

void CreatureAI::SetDeathPrevention(bool state)
{
    m_deathPrevention = state;
    if (state)
        m_deathPrevented = false;
}

void CreatureAI::DoFakeDeath(uint32 spellId)
{
    m_creature->InterruptNonMeleeSpells(false);
    m_creature->InterruptMoving();
    m_creature->ClearComboPointHolders();
    m_creature->RemoveAllAurasOnDeath();
    m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    m_creature->ClearAllReactives();
    m_creature->SetTarget(nullptr);
    m_creature->GetMotionMaster()->Clear();
    m_creature->GetMotionMaster()->MoveIdle();

    if (spellId)
        DoCastSpellIfCan(nullptr, spellId, CAST_INTERRUPT_PREVIOUS);
}

void CreatureAI::RetreatingArrived()
{
    m_creature->SetNoCallAssistance(false);
    m_creature->CallAssistance();
}

void CreatureAI::RetreatingEnded()
{
    if (GetAIOrder() != ORDER_RETREATING)
        return; // prevent stack overflow by cyclic calls - TODO: remove once Motion Master is human again
    SetAIOrder(ORDER_NONE);
    SetCombatScriptStatus(false);
    if (!m_creature->IsAlive())
        return;
    DoStartMovement(m_creature->GetVictim());
}

bool CreatureAI::DoRetreat()
{
    Unit* victim = m_creature->GetVictim();
    if (!victim)
        return false;

    float radius = sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS);
    if (radius <= 0)
        return false;

    Creature* ally = nullptr;

    MaNGOS::NearestAssistCreatureInCreatureRangeCheck check(m_creature, victim, radius);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestAssistCreatureInCreatureRangeCheck> searcher(ally, check);
    Cell::VisitGridObjects(m_creature, searcher, radius);

    // Check if an ally to call for was found
    if (!ally)
        return false;

    uint32 delay = sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_ASSISTANCE_DELAY);

    Position pos;
    ally->GetFirstCollisionPosition(pos, ally->GetCombatReach(), ally->GetAngle(m_creature));
    m_creature->GetMotionMaster()->MoveRetreat(pos.x, pos.y, pos.z, ally->GetAngle(victim), delay);

    SetAIOrder(ORDER_RETREATING);
    SetCombatScriptStatus(true);
    return true;
}

void CreatureAI::DoCallForHelp(float radius)
{
    m_creature->CallForHelp(radius);
}

void CreatureAI::OnCallForHelp(Unit* caller, Unit* enemy)
{
    if (FactionTemplateEntry const* factionTemplate = m_creature->GetFactionTemplateEntry())
    {
        if (factionTemplate->factionFlags & FACTION_TEMPLATE_FLEE_FROM_CALL_FOR_HELP)
        {
            if (m_creature->SetInPanic(10000))
                SetAIOrder(ORDER_FLEE_FROM_CALL_FOR_HELP);
            return;
        }
    }
    AttackStart(enemy);
}

void CreatureAI::HandleAssistanceCall(Unit* sender, Unit* invoker)
{
    if (m_creature->IsInCombat() || !invoker)
        return;
    if (m_creature->CanAssist(sender) && m_creature->CanAttackOnSight(invoker) && invoker->IsVisibleForOrDetect(m_creature, m_creature, false))
    {
        m_creature->SetNoCallAssistance(true);
        AttackStart(invoker);
    }
}

void CreatureAI::AddUnreachabilityCheck()
{
    m_teleportUnreachable = true;
}

CreatureSpellList const& CreatureAI::GetSpellList() const
{
    return m_creature->GetSpellList();
}

void CreatureAI::TimedFleeingEnded()
{
    UnitAI::TimedFleeingEnded();
    if (GetAIOrder() == ORDER_FLEE_FROM_CALL_FOR_HELP && m_creature->IsAlive())
    {
        if (FactionTemplateEntry const* factionTemplate = m_creature->GetFactionTemplateEntry())
            if (factionTemplate->factionFlags & FACTION_TEMPLATE_FLEE_FROM_CALL_FOR_HELP)
                EnterEvadeMode();
    }
}
