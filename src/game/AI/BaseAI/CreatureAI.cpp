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

#include "CreatureAI.h"
#include "Entities/Creature.h"
#include "Server/DBCStores.h"
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "World/World.h"

static_assert(MAXIMAL_AI_EVENT_EVENTAI <= 32, "Maximal 32 AI_EVENTs supported with EventAI");

CreatureAI::CreatureAI(Creature* creature) :
    m_creature(creature),
    m_unit(creature),
    m_isCombatMovement(true),
    m_attackDistance(0.0f),
    m_attackAngle(0.0f),
    m_reactState(REACT_AGGRESSIVE),
    m_meleeEnabled(true),
    m_visibilityDistance(VISIBLE_RANGE),
    m_moveFurther(true),
    m_combatScriptHappening(false)
{
    m_dismountOnAggro = !(m_creature->GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_MOUNTED_COMBAT);

    if (m_creature->IsNoAggroOnSight())
        SetReactState(REACT_DEFENSIVE);
    if (m_creature->IsGuard() || m_unit->GetCharmInfo()) // guards and charmed targets
        m_visibilityDistance = sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER);
}

CreatureAI::CreatureAI(Unit* unit) :
    m_creature(nullptr),
    m_unit(unit),
    m_isCombatMovement(true),
    m_attackDistance(0.0f),
    m_attackAngle(0.0f),
    m_reactState(REACT_AGGRESSIVE),
    m_meleeEnabled(true),
    m_visibilityDistance(VISIBLE_RANGE),
    m_moveFurther(true),
    m_combatScriptHappening(false)
{
}

CreatureAI::~CreatureAI()
{
}

void CreatureAI::MoveInLineOfSight(Unit* who)
{
    if (!HasReactState(REACT_AGGRESSIVE))
        return;

    if (!m_creature->CanFly() && m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
        return;

    if (m_creature->getVictim() && !m_creature->GetMap()->IsDungeon())
        return;

    if (m_creature->IsNoAggroOnSight() || m_creature->IsNeutralToAll())
        return;

    if (who->GetObjectGuid().IsCreature() && who->isInCombat())
        CheckForHelp(who, m_creature, 10.0);

    if (m_creature->CanInitiateAttack() && who->isInAccessablePlaceFor(m_creature))
    {
        if (AssistPlayerInCombat(who))
            return;

        if (m_creature->CanAttackOnSight(who))
            DetectOrAttack(who, m_creature);
    }
}

void CreatureAI::EnterCombat(Unit* enemy)
{
    if (enemy && (m_creature->IsGuard() || m_creature->IsCivilian()))
    {
        // Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
        if (Player* pKiller = enemy->GetBeneficiaryPlayer())
            m_creature->SendZoneUnderAttackMessage(pKiller);
    }
}

void CreatureAI::EnterEvadeMode()
{
    m_creature->RemoveAllAurasOnEvade();
    m_creature->DeleteThreatList();
    m_creature->CombatStop(true);

    // only alive creatures that are not on transport can return to home position
    if (GetReactState() != REACT_PASSIVE && m_creature->isAlive())
        m_creature->GetMotionMaster()->MoveTargetedHome();

    m_creature->SetLootRecipient(nullptr);

    m_creature->TriggerEvadeEvents();
}

void CreatureAI::AttackedBy(Unit* attacker)
{
    if (!m_unit->getVictim())
        AttackStart(attacker);
}

CanCastResult CreatureAI::CanCastSpell(Unit* target, const SpellEntry* spellInfo, bool isTriggered) const
{
    // If not triggered, we check
    if (!isTriggered)
    {
        // State does not allow
        if (m_unit->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
            return CAST_FAIL_STATE;

        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && m_unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            return CAST_FAIL_STATE;

        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && m_unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            return CAST_FAIL_STATE;

        // Check for power (also done by Spell::CheckCast())
        if (m_unit->GetPower((Powers)spellInfo->powerType) < Spell::CalculatePowerCost(spellInfo, m_unit))
            return CAST_FAIL_POWER;

        if (!spellInfo->HasAttribute(SPELL_ATTR_EX2_IGNORE_LOS) && !m_unit->IsWithinLOSInMap(target) && m_unit != target)
            return CAST_FAIL_NOT_IN_LOS;
    }

    if (const SpellRangeEntry* spellRange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex))
    {
        if (target != m_unit)
        {
            // pTarget is out of range of this spell (also done by Spell::CheckCast())
            float distance = m_unit->GetCombatDistance(target, spellInfo->rangeIndex == SPELL_RANGE_IDX_COMBAT);

            if (distance > spellRange->maxRange)
                return CAST_FAIL_TOO_FAR;

            float minRange = spellRange->minRange;

            if (minRange && distance < minRange)
                return CAST_FAIL_TOO_CLOSE;
        }

        return CAST_OK;
    }
    else
        return CAST_FAIL_OTHER;
}

CanCastResult CreatureAI::DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags, ObjectGuid originalCasterGUID) const
{
    Unit* caster = m_unit;

    if (!target)
        return CAST_FAIL_OTHER;

    if (castFlags & CAST_SWITCH_CASTER_TARGET)
        std::swap(caster, target);

    if (castFlags & CAST_FORCE_TARGET_SELF)
        caster = target;

    // Allowed to cast only if not casting (unless we interrupt ourself) or if spell is triggered
    if (!caster->IsNonMeleeSpellCasted(false) || (castFlags & (CAST_TRIGGERED | CAST_INTERRUPT_PREVIOUS)))
    {
        if (const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId))
        {
            // If cast flag CAST_AURA_NOT_PRESENT is active, check if target already has aura on them
            if (castFlags & CAST_AURA_NOT_PRESENT)
            {
                if (target->HasAura(spellId))
                    return CAST_FAIL_TARGET_AURA;
            }

            // Check if cannot cast spell
            if (!(castFlags & (CAST_FORCE_TARGET_SELF | CAST_FORCE_CAST)))
            {
                CanCastResult castResult = CanCastSpell(target, spellInfo, !!(castFlags & CAST_TRIGGERED));

                if (castResult != CAST_OK)
                    return castResult;
            }

            // Interrupt any previous spell
            if (castFlags & CAST_INTERRUPT_PREVIOUS && caster->IsNonMeleeSpellCasted(false))
                caster->InterruptNonMeleeSpells(false);

            // Creature should always stop before it will cast a non-instant spell
            if (GetSpellCastTime(spellInfo) || (IsChanneledSpell(spellInfo) && spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_MOVEMENT))
                caster->StopMoving();

            // Creature should interrupt any current melee spell
            caster->InterruptSpell(CURRENT_MELEE_SPELL);

            // Creature should stop wielding weapon while casting
            // caster->SetSheath(SHEATH_STATE_UNARMED);

            uint32 flags = (castFlags & CAST_TRIGGERED ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE) | (castFlags & CAST_IGNORE_UNSELECTABLE_TARGET ? TRIGGERED_IGNORE_UNSELECTABLE_FLAG : TRIGGERED_NONE);

            caster->CastSpell(target, spellInfo, flags, nullptr, nullptr, originalCasterGUID);
            return CAST_OK;
        }
        else
        {
            sLog.outErrorDb("DoCastSpellIfCan by %s attempt to cast spell %u but spell does not exist.", m_unit->GetObjectGuid().GetString().c_str(), spellId);
            return CAST_FAIL_OTHER;
        }
    }
    else
        return CAST_FAIL_IS_CASTING;
}

void CreatureAI::AttackStart(Unit* who)
{
    if (!who || HasReactState(REACT_PASSIVE))
        return;

    if (m_creature->Attack(who, m_meleeEnabled))
    {
        m_creature->AddThreat(who);
        m_creature->SetInCombatWith(who);
        who->SetInCombatWith(m_creature);

        // Cast "Spawn Guard" to help Civilian
        if (m_creature->IsCivilian())
            m_creature->CastSpell(m_creature, 43783, TRIGGERED_OLD_TRIGGERED);

        HandleMovementOnAttackStart(who);
    }
}

bool CreatureAI::DoMeleeAttackIfReady() const
{
    return m_unit->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && m_unit->UpdateMeleeAttackingState();
}

void CreatureAI::SetCombatMovement(bool enable, bool stopOrStartMovement /*=false*/)
{
    m_isCombatMovement = enable;

    if (enable)
        m_unit->clearUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);
    else
        m_unit->addUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);

    if (stopOrStartMovement && m_unit->getVictim())     // Only change current movement while in combat
    {
        if (!m_unit->IsIncapacitated())
        {
            if (enable)
                m_unit->GetMotionMaster()->MoveChase(m_unit->getVictim(), m_attackDistance, m_attackAngle, false);
            else if (m_unit->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
                m_unit->StopMoving();
        }
    }
}

void CreatureAI::HandleMovementOnAttackStart(Unit* victim) const
{
    if (!m_unit->hasUnitState(UNIT_STAT_CAN_NOT_REACT))
    {
        if (m_dismountOnAggro)
            m_unit->Unmount(); // all ais should unmount here

        MotionMaster* creatureMotion = m_unit->GetMotionMaster();

        if (!m_unit->hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT))
            creatureMotion->MoveChase(victim, m_attackDistance, m_attackAngle, m_moveFurther);
        // TODO - adapt this to only stop OOC-MMGens when MotionMaster rewrite is finished
        else if (creatureMotion->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE || creatureMotion->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
        {
            creatureMotion->MoveIdle();
            m_unit->StopMoving();
        }
    }
}

void CreatureAI::OnChannelStateChange(SpellEntry const * spellInfo, bool state, WorldObject* target)
{
    // TODO: Determine if CHANNEL_FLAG_MOVEMENT is worth implementing
    if (!spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET))
    {
        if (spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING))
            return;
    }

    if (state)
    {
        if (spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_TURNING && !spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET)) // 30166 changes target to none
        {
            m_unit->SetTurningOff(true);
            m_unit->SetFacingTo(m_creature->GetOrientation());
            m_unit->SetTarget(nullptr);
        }
        else if (target && m_creature != target)
        {
            m_unit->SetTarget(target);
            m_unit->SetOrientation(m_unit->GetAngle(target));
        }
        else
        {
            m_unit->SetFacingTo(m_creature->GetOrientation());
            m_unit->SetTarget(nullptr);
        }
    }
    else
    {
        if (spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_TURNING)
            m_unit->SetTurningOff(false);

        if (m_unit->getVictim())
            m_unit->SetTarget(m_unit->getVictim());
        else
            m_unit->SetTarget(nullptr);
    }
}

void CreatureAI::CheckForHelp(Unit* who, Creature* me, float distance)
{
    Unit* victim = who->getAttackerForHelper();

    if (!victim)
        return;

    if (me->GetMap()->Instanceable())
        distance = distance / 2.5f;

    if (me->CanInitiateAttack() && me->CanAttackOnSight(victim) && victim->isInAccessablePlaceFor(me))
    {
        if (me->IsWithinDistInMap(who, distance) && me->IsWithinLOSInMap(who))
        {
            if (!me->getVictim())
            {
                if (me->GetMap()->Instanceable()) // Instanceable case ignore family/faction checks
                    AttackStart(victim);
                else // In non-instanceable creature must belong to same family and faction to attack player.
                {
                    if (me->GetCreatureInfo()->Family == ((Creature*)who)->GetCreatureInfo()->Family &&
                            me->GetCreatureInfo()->FactionAlliance == ((Creature*)who)->GetCreatureInfo()->FactionAlliance &&
                            me->GetCreatureInfo()->FactionHorde == ((Creature*)who)->GetCreatureInfo()->FactionHorde)
                        AttackStart(victim);
                }
            }
        }
    }
}

void CreatureAI::DetectOrAttack(Unit* who, Creature* me)
{
    float attackRadius = me->GetAttackDistance(who);

    if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
    {
        if (!me->getVictim())
        {
            if (who->HasStealthAura() || who->HasInvisibilityAura())
            {
                if (!me->hasUnitState(UNIT_STAT_DISTRACTED) && !me->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
                {
                    me->GetMotionMaster()->MoveDistract(TIME_INTERVAL_LOOK);
                    me->SetFacingTo(me->GetAngle(who));
                }

                if (me->IsWithinDistInMap(who, who->GetVisibleDist(me) * 0.7f))
                    AttackStart(who);
            }
            else
                AttackStart(who);
        }
        else if (me->GetMap()->IsDungeon())
        {
            me->AddThreat(who);
            me->SetInCombatWith(who);
            who->SetInCombatWith(me);
        }
    }
}

// ////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Event system
// ////////////////////////////////////////////////////////////////////////////////////////////////

class AiDelayEventAround : public BasicEvent
{
    public:
        AiDelayEventAround(AIEventType eventType, ObjectGuid invokerGuid, Creature& owner, std::list<Creature*> const& receivers, uint32 miscValue) :
            BasicEvent(),
            m_eventType(eventType),
            m_invokerGuid(invokerGuid),
            m_owner(owner),
            m_miscValue(miscValue)
        {
            // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
            m_receiverGuids.reserve(receivers.size());
            for (std::list<Creature*>::const_iterator itr = receivers.begin(); itr != receivers.end(); ++itr)
                m_receiverGuids.push_back((*itr)->GetObjectGuid());
        }

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/) override
        {
            Unit* pInvoker = m_owner.GetMap()->GetUnit(m_invokerGuid);

            for (GuidVector::const_reverse_iterator itr = m_receiverGuids.rbegin(); itr != m_receiverGuids.rend(); ++itr)
            {
                if (Creature* pReceiver = m_owner.GetMap()->GetAnyTypeCreature(*itr))
                {
                    pReceiver->AI()->ReceiveAIEvent(m_eventType, &m_owner, pInvoker, m_miscValue);
                    // Special case for type 0 (call-assistance)
                    if (m_eventType == AI_EVENT_CALL_ASSISTANCE && pInvoker && pReceiver->CanAssist(&m_owner) && pReceiver->CanAttackOnSight(pInvoker))
                    {
                        pReceiver->SetNoCallAssistance(true);
                        pReceiver->AI()->AttackStart(pInvoker);
                    }
                }
            }
            m_receiverGuids.clear();

            return true;
        }

    private:
        AiDelayEventAround();

        AIEventType m_eventType;
        ObjectGuid m_invokerGuid;
        Creature&  m_owner;
        uint32 m_miscValue;

        GuidVector m_receiverGuids;
};

void CreatureAI::SendAIEventAround(AIEventType eventType, Unit* invoker, uint32 delay, float radius, uint32 miscValue /*=0*/) const
{
    if (radius > 0)
    {
        std::list<Creature*> receiverList;

        // Allow sending custom AI events to all units in range
        if (eventType >= AI_EVENT_CUSTOM_EVENTAI_A && eventType <= AI_EVENT_CUSTOM_EVENTAI_F && eventType != AI_EVENT_GOT_CCED)
        {
            MaNGOS::AnyUnitInObjectRangeCheck u_check(m_creature, radius);
            MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(receiverList, u_check);
            Cell::VisitGridObjects(m_creature, searcher, radius);
        }
        else
        {
            // Use this check here to collect only assitable creatures in case of CALL_ASSISTANCE, else be less strict
            MaNGOS::AnyAssistCreatureInRangeCheck u_check(m_creature, eventType == AI_EVENT_CALL_ASSISTANCE ? invoker : nullptr, radius);
            MaNGOS::CreatureListSearcher<MaNGOS::AnyAssistCreatureInRangeCheck> searcher(receiverList, u_check);
            Cell::VisitGridObjects(m_creature, searcher, radius);
        }

        if (!receiverList.empty())
        {
            AiDelayEventAround* e = new AiDelayEventAround(eventType, invoker ? invoker->GetObjectGuid() : ObjectGuid(), *m_creature, receiverList, miscValue);
            m_creature->m_Events.AddEvent(e, m_creature->m_Events.CalculateTime(delay));
        }
    }
}

void CreatureAI::SendAIEvent(AIEventType eventType, Unit* invoker, Creature* receiver, uint32 miscValue /*=0*/) const
{
    MANGOS_ASSERT(receiver);
    receiver->AI()->ReceiveAIEvent(eventType, m_creature, invoker, miscValue);
}

bool CreatureAI::IsVisible(Unit* pl) const
{
    return m_creature->IsWithinDist(pl, m_visibilityDistance) && pl->isVisibleForOrDetect(m_creature, m_creature, true);
}

Unit* CreatureAI::DoSelectLowestHpFriendly(float range, float minMissing, bool percent)
{
    Unit* pUnit = nullptr;

    if (percent)
    {
        MaNGOS::MostHPPercentMissingInRangeCheck u_check(m_creature, range, minMissing, true);
        MaNGOS::UnitLastSearcher<MaNGOS::MostHPPercentMissingInRangeCheck> searcher(pUnit, u_check);

        Cell::VisitGridObjects(m_creature, searcher, range);
    }
    else
    {
        MaNGOS::MostHPMissingInRangeCheck u_check(m_creature, range, minMissing, true);
        MaNGOS::UnitLastSearcher<MaNGOS::MostHPMissingInRangeCheck> searcher(pUnit, u_check);

        Cell::VisitGridObjects(m_creature, searcher, range);
    }

    return pUnit;
}

bool CreatureAI::CanExecuteCombatAction()
{
    return m_unit->CanReactInCombat() && !m_unit->hasUnitState(UNIT_STAT_DONT_TURN | UNIT_STAT_SEEKING_ASSISTANCE | UNIT_STAT_CHANNELING) && !m_combatScriptHappening;
}

void CreatureAI::SetMeleeEnabled(bool state)
{
    if (state == m_meleeEnabled)
        return;

    m_meleeEnabled = state;
    if (m_creature->isInCombat())
    {
        if (m_meleeEnabled)
        {
            if (m_creature->getVictim())
                m_creature->MeleeAttackStart(m_creature->getVictim());
        }
        else
            m_creature->MeleeAttackStop(m_creature->getVictim());
    }
}
