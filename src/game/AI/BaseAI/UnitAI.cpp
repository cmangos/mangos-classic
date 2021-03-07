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

#include "UnitAI.h"
#include "Entities/Creature.h"
#include "Server/DBCStores.h"
#include "Spells/Spell.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Spells/SpellMgr.h"
#include "World/World.h"
#include <limits>

static_assert(MAXIMAL_AI_EVENT_EVENTAI <= 32, "Maximal 32 AI_EVENTs supported with EventAI");

UnitAI::UnitAI(Unit* unit) :
    m_unit(unit),
    m_attackDistance(0.0f),
    m_attackAngle(0.0f),
    m_moveFurther(false),
    m_chaseRun(true),
    m_visibilityDistance(VISIBLE_RANGE),
    m_combatMovementStarted(false),
    m_dismountOnAggro(true),
    m_meleeEnabled(true),
    m_reactState(REACT_AGGRESSIVE),
    m_combatScriptHappening(false),
    m_currentAIOrder(ORDER_NONE),
    m_selfRooted(false),
    m_currentSpell(nullptr)
{
}

UnitAI::~UnitAI()
{
}

void UnitAI::MoveInLineOfSight(Unit* who)
{
    if (GetReactState() < REACT_DEFENSIVE)
        return;

    if (!m_unit->CanFly() && who->IsFlying())
        if (m_unit->GetDistanceZ(who) > (IsRangedUnit() ? CREATURE_Z_ATTACK_RANGE_RANGED : CREATURE_Z_ATTACK_RANGE_MELEE))
            return;

    if (m_unit->GetVictim() && !m_unit->GetMap()->IsDungeon())
        return;

    if (m_unit->IsNeutralToAll())
        return;

    if (who->GetObjectGuid().IsCreature() && who->IsInCombat())
        CheckForHelp(who, m_unit, 10.0);

    if (!HasReactState(REACT_AGGRESSIVE)) // mobs who are aggressive can still assist
        return;

    if (!m_unit->CanInitiateAttack())
        return;

    if (AssistPlayerInCombat(who))
        return;

    if (!m_unit->CanAttackOnSight(who))
        return;

    if (!who->isInAccessablePlaceFor(m_unit))
        return;

    DetectOrAttack(who);
}

void UnitAI::EnterEvadeMode()
{
    ClearSelfRoot();
    m_unit->RemoveAllAurasOnEvade();
    m_unit->CombatStopWithPets(true);

    // only alive creatures that are not on transport can return to home position
    if (GetReactState() != REACT_PASSIVE && m_unit->IsAlive())
    {
        if (!m_unit->IsImmobilizedState()) // if still rooted after aura removal - permarooted
            m_unit->GetMotionMaster()->MoveTargetedHome();
        else
            JustReachedHome();
    }


    m_unit->TriggerEvadeEvents();
}

void UnitAI::AttackedBy(Unit* attacker)
{
    if (!m_unit->IsInCombat() && !m_unit->GetVictim())
        AttackStart(attacker);
}

CanCastResult UnitAI::CanCastSpell(Unit* target, const SpellEntry* spellInfo, bool isTriggered) const
{
    // If not triggered, we check
    if (!isTriggered)
    {
        if (target && !Spell::CheckTargetCreatureType(target, spellInfo))
            return CAST_FAIL_OTHER;

        // already active next melee swing spell
        if (IsNextMeleeSwingSpell(spellInfo))
            if (Spell* autorepeatSpell = m_unit->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
                if (autorepeatSpell->m_spellInfo->Id == spellInfo->Id)
                    return CAST_FAIL_OTHER;
    }
    return CAST_OK;
}

CanCastResult UnitAI::DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags)
{
    Unit* caster = m_unit;

    if (target)
    {
        if (castFlags & CAST_SWITCH_CASTER_TARGET)
            std::swap(caster, target);

        if (castFlags & CAST_FORCE_TARGET_SELF)
            caster = target;
    }
    else if (castFlags & (CAST_FORCE_TARGET_SELF | CAST_SWITCH_CASTER_TARGET))
        return CAST_FAIL_OTHER;

    if (GetAIOrder() == ORDER_EVADE && !(castFlags & CAST_TRIGGERED))
        return CAST_FAIL_EVADE;

    // Allowed to cast only if not casting (unless we interrupt ourself) or if spell is triggered
    if (!caster->IsNonMeleeSpellCasted(false) || (castFlags & (CAST_TRIGGERED | CAST_INTERRUPT_PREVIOUS)))
    {
        if (const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId))
        {
            // If cast flag CAST_AURA_NOT_PRESENT is active, check if target already has aura on them
            if (castFlags & CAST_AURA_NOT_PRESENT)
            {
                if (!target)
                {
                    if (caster->HasAura(spellId))
                        return CAST_FAIL_TARGET_AURA;
                }
                else if (target->HasAura(spellId))
                    return CAST_FAIL_TARGET_AURA;
            }

            // Check if cannot cast spell
            if (!(castFlags & (CAST_FORCE_TARGET_SELF | CAST_FORCE_CAST)))
            {
                CanCastResult castResult = CanCastSpell(target, spellInfo, (castFlags & CAST_TRIGGERED) != 0);

                if (castResult != CAST_OK)
                    return castResult;
            }

            // Interrupt any previous spell
            if (castFlags & CAST_INTERRUPT_PREVIOUS && caster->IsNonMeleeSpellCasted(false))
                caster->InterruptNonMeleeSpells(false);

            // Creature should interrupt any current melee spell
            caster->InterruptSpell(CURRENT_MELEE_SPELL);

            // Creature should stop wielding weapon while casting
            // caster->SetSheath(SHEATH_STATE_UNARMED);

            uint32 flags = (castFlags & CAST_TRIGGERED ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE) | (castFlags & CAST_IGNORE_UNSELECTABLE_TARGET ? TRIGGERED_IGNORE_UNSELECTABLE_FLAG : TRIGGERED_NONE);

            if (flags == TRIGGERED_NONE)
                flags |= TRIGGERED_NORMAL_COMBAT_CAST;

            SpellCastResult result;
            if (castFlags & CAST_ONLY_XYZ)
                result = caster->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spellInfo, flags);
            else
                result = caster->CastSpell(target, spellInfo, flags);
            if (result != SPELL_CAST_OK)
            {
                switch (result) // temporary adapter
                {
                    case SPELL_FAILED_OUT_OF_RANGE:
                        return CAST_FAIL_TOO_FAR;
                    case SPELL_FAILED_TOO_CLOSE:
                        return CAST_FAIL_TOO_CLOSE;
                    case SPELL_FAILED_LINE_OF_SIGHT:
                        return CAST_FAIL_NOT_IN_LOS;
                    case SPELL_FAILED_PACIFIED:
                    case SPELL_FAILED_SILENCED:
                    case SPELL_FAILED_STUNNED:
                    case SPELL_FAILED_CONFUSED:
                    case SPELL_FAILED_FLEEING:
                        return CAST_FAIL_STATE;
                    case SPELL_FAILED_NOT_READY:
                        return CAST_FAIL_COOLDOWN;
                    case SPELL_FAILED_NO_POWER:
                        return CAST_FAIL_POWER;
                    case SPELL_FAILED_CASTER_AURASTATE: // valid - doesnt need logging
                    case SPELL_FAILED_BAD_TARGETS:
                    case SPELL_FAILED_DONT_REPORT:
                        return CAST_FAIL_OTHER;
                    default: break;
                }
                sLog.outBasic("DoCastSpellIfCan by %s attempt to cast spell %u but spell failed due to unknown result %u.", m_unit->GetObjectGuid().GetString().c_str(), spellId, result);
                return CAST_FAIL_OTHER;
            }
            return CAST_OK;
        }
        sLog.outErrorDb("DoCastSpellIfCan by %s attempt to cast spell %u but spell does not exist.", m_unit->GetGuidStr().c_str(), spellId);
        return CAST_FAIL_OTHER;
    }
    return CAST_FAIL_IS_CASTING;
}

void UnitAI::AttackStart(Unit* who)
{
    if (!who || HasReactState(REACT_PASSIVE))
        return;

    if (m_unit->Attack(who, m_meleeEnabled))
    {
        m_unit->EngageInCombatWith(who);
        HandleMovementOnAttackStart(who);
    }
}

bool UnitAI::DoMeleeAttackIfReady() const
{
    return m_unit->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && GetAIOrder() == ORDER_NONE && m_unit->UpdateMeleeAttackingState();
}

void UnitAI::SetCombatMovement(bool enable, bool stopOrStartMovement /*=false*/)
{
    if (enable)
        m_unit->clearUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);
    else
        m_unit->addUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);

    if (stopOrStartMovement && m_unit->GetVictim())     // Only change current movement while in combat
    {
        if (!m_unit->IsCrowdControlled())
        {
            if (enable)
                DoStartMovement(m_unit->GetVictim());
            else if (m_unit->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
                m_unit->InterruptMoving();
        }
    }
}

bool UnitAI::IsCombatMovement() const
{
    return m_unit && !m_unit->hasUnitState(UNIT_STAT_NO_COMBAT_MOVEMENT);
}

void UnitAI::HandleMovementOnAttackStart(Unit* victim) const
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
            m_unit->InterruptMoving();
        }
    }
}

void UnitAI::OnSpellCastStateChange(Spell const* spell, bool state, WorldObject* target)
{
    if (!state && spell != m_currentSpell)
        return;

    SpellEntry const* spellInfo = spell->m_spellInfo;
    if (spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING) || spellInfo->HasAttribute(SPELL_ATTR_ON_NEXT_SWING_1) || spellInfo->HasAttribute(SPELL_ATTR_ON_NEXT_SWING_2))
        return;

    // Creature should always stop before it will cast a non-instant spell
    if (state)
        if ((spell->GetCastTime() && spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT) || (IsChanneledSpell(spellInfo) && spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_MOVEMENT))
            m_unit->InterruptMoving();

    bool forceTarget = false;

    // Targeting seems to be directly affected by eff index 0 targets, client does the same thing (spell id 38523 exception)
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch (spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0])
        {
            case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24: // ignores everything and keeps turning
                return;
            case TARGET_UNIT_FRIEND:
            case TARGET_UNIT_ENEMY: forceTarget = true; break;
            case TARGET_UNIT_SCRIPT_NEAR_CASTER:
            default: break;
        }
        if (forceTarget)
            break;
    }

    if (state)
    {
        if (target && (m_unit != target || forceTarget))
        {
            m_unit->SetTarget(target);
            if (m_unit != target)
                m_unit->SetOrientation(m_unit->GetAngle(target));
            else
                m_unit->SetOrientation(m_unit->GetOrientation());
        }
        else
        {
            if (m_unit->HasTarget())
            {
                m_unit->SetFacingTo(m_unit->GetOrientation());
                m_unit->SetTarget(nullptr);
            }
        }
    }
    else
    {
        if (spellInfo->Id == 31306)
        {
            HandleDelayedInstantAnimation(spellInfo);
        }
        else
        {
            if (m_unit->GetVictim() && !GetCombatScriptStatus())
                m_unit->SetTarget(m_unit->GetVictim());
            else
                m_unit->SetTarget(nullptr);
        }
    }

    if (state)
    {
        m_currentSpell = spell;
    }
}

void UnitAI::OnChannelStateChange(Spell const* spell, bool state, WorldObject* target)
{
    if (state)
        m_currentSpell = nullptr;

    SpellEntry const* spellInfo = spell->m_spellInfo;
    // TODO: Determine if CHANNEL_FLAG_MOVEMENT is worth implementing
    if (!spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET))
    {
        if (spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING))
            return;
    }

    bool forceTarget = true; // Different default than normal cast

    // Targeting seems to be directly affected by eff index 0 targets, client does the same thing
    switch (spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0])
    {
        case TARGET_UNIT:
        case TARGET_UNIT_SCRIPT_NEAR_CASTER: forceTarget = false; break;
        case TARGET_UNIT_ENEMY:
        default: break;
    }

    if (spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET))
        forceTarget = true;

    if (state)
    {
        if ((spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_TURNING && !spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET)) || !forceTarget)
        {
            m_unit->SetFacingTo(m_unit->GetOrientation());
            m_unit->SetTarget(nullptr);
        }
        else if (target && m_unit != target)
        {
            m_unit->SetTarget(target);
            m_unit->SetOrientation(m_unit->GetAngle(target));
        }
        else
        {
            m_unit->SetFacingTo(m_unit->GetOrientation());
            m_unit->SetTarget(nullptr);
        }
    }
    else
    {
        if (m_unit->GetVictim() && !GetCombatScriptStatus())
            m_unit->SetTarget(m_unit->GetVictim());
        else
            m_unit->SetTarget(nullptr);
    }
}

void UnitAI::CheckForHelp(Unit* who, Unit* me, float distance)
{
    Unit* victim = who->getAttackerForHelper();

    if (!victim)
        return;

    if (me->IsInCombat())
        return;

    // pulling happens once panic/retreating ends
    if (who->hasUnitState(UNIT_STAT_PANIC | UNIT_STAT_RETREATING))
        return;

    if (me->GetMap()->Instanceable())
        distance = distance / 2.5f;

    if (me->CanInitiateAttack() && me->CanAttackOnSight(victim) && victim->isInAccessablePlaceFor(me) && victim->IsVisibleForOrDetect(me, me, false))
    {
        if (me->IsWithinDistInMap(who, distance) && me->IsWithinLOSInMap(who, true))
        {
            if (me->CanAssistInCombatAgainst(who, victim))
            {
                AttackStart(victim);
                if (who->AI() && who->AI()->GetAIOrder() == ORDER_FLEEING)
                    who->GetMotionMaster()->InterruptPanic();
            }
        }
    }
}

void UnitAI::DetectOrAttack(Unit* who)
{
    float attackRadius = m_unit->GetAttackDistance(who);
    if (m_unit->GetDistance(who, true, DIST_CALC_NONE) > attackRadius * attackRadius)
        return;

    if (!m_unit->IsWithinLOSInMap(who, true))
        return;

    if (!m_unit->GetVictim() && !m_unit->IsInCombat())
    {
        if (CanTriggerStealthAlert(who, attackRadius))
        {
            m_unit->SendAIReaction(AI_REACTION_ALERT);
            m_unit->SetFacingTo(m_unit->GetAngle(who));
            m_unit->GetMotionMaster()->MoveDistract(TIME_INTERVAL_LOOK);
            OnStealthAlert(who);
            return;
        }

        AttackStart(who);
    }
    else if (m_unit->GetMap()->IsDungeon())
    {
        m_unit->EngageInCombatWith(who);
    }
}

bool UnitAI::CanTriggerStealthAlert(Unit* who, float /*attackRadius*/) const
{
    if (who->GetTypeId() != TYPEID_PLAYER)
        return false;
    if (m_unit->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return false;

    if (m_unit->hasUnitState(UNIT_STAT_DISTRACTED))
        return false;

    return who->HasStealthAura() && m_unit->GetDistance(who, true, DIST_CALC_NONE) > who->GetVisibilityData().GetStealthVisibilityDistance(m_unit);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Event system
// ////////////////////////////////////////////////////////////////////////////////////////////////

class AiDelayEventAround : public BasicEvent
{
    public:
        AiDelayEventAround(AIEventType eventType, ObjectGuid invokerGuid, Unit& owner, CreatureList const& receivers, uint32 miscValue) :
            BasicEvent(),
            m_eventType(eventType),
            m_invokerGuid(invokerGuid),
            m_owner(owner),
            m_miscValue(miscValue)
        {
            // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
            m_receiverGuids.reserve(receivers.size());
            for (auto receiver : receivers)
                m_receiverGuids.push_back(receiver->GetObjectGuid());
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
                    if (m_eventType == AI_EVENT_CALL_ASSISTANCE)
                        pReceiver->AI()->HandleAssistanceCall(&m_owner, pInvoker);
                }
            }
            m_receiverGuids.clear();

            return true;
        }

    private:
        AiDelayEventAround();

        AIEventType m_eventType;
        ObjectGuid m_invokerGuid;
        Unit&  m_owner;
        uint32 m_miscValue;

        GuidVector m_receiverGuids;
};

void UnitAI::SendAIEventAround(AIEventType eventType, Unit* invoker, uint32 delay, float radius, uint32 miscValue /*=0*/) const
{
    if (radius > 0)
    {
        CreatureList receiverList;

        // Allow sending custom AI events to all units in range
        if ((eventType >= AI_EVENT_CUSTOM_EVENTAI_A && eventType <= AI_EVENT_CUSTOM_EVENTAI_F && eventType != AI_EVENT_GOT_CCED) || eventType > AI_EVENT_START_ESCORT)
        {
            MaNGOS::AnyUnitInObjectRangeCheck u_check(m_unit, radius);
            MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(receiverList, u_check);
            Cell::VisitAllObjects(m_unit, searcher, radius);
        }
        else // TODO: Expand functionality in future if needed
        {
            MaNGOS::AnyAssistCreatureInRangeCheck u_check(m_unit, invoker, radius);
            MaNGOS::CreatureListSearcher<MaNGOS::AnyAssistCreatureInRangeCheck> searcher(receiverList, u_check);
            Cell::VisitAllObjects(m_unit, searcher, radius);
        }

        if (!receiverList.empty())
        {
            if (delay)
            {
                AiDelayEventAround* e = new AiDelayEventAround(eventType, invoker ? invoker->GetObjectGuid() : ObjectGuid(), *m_unit, receiverList, miscValue);
                m_unit->m_events.AddEvent(e, m_unit->m_events.CalculateTime(delay));
            }
            else
            {
                for (Creature* receiver : receiverList)
                {
                    receiver->AI()->ReceiveAIEvent(eventType, m_unit, invoker, miscValue);
                    // Special case for type 0 (call-assistance)
                    if (eventType == AI_EVENT_CALL_ASSISTANCE)
                        receiver->AI()->HandleAssistanceCall(m_unit, invoker);
                }
            }
        }
    }
}

void UnitAI::SendAIEvent(AIEventType eventType, Unit* invoker, Unit* receiver, uint32 miscValue /*=0*/) const
{
    MANGOS_ASSERT(receiver);
    receiver->AI()->ReceiveAIEvent(eventType, m_unit, invoker, miscValue);
}

bool UnitAI::IsVisible(Unit* pl) const
{
    return m_unit->IsWithinDist(pl, m_visibilityDistance) && pl->IsVisibleForOrDetect(m_unit, m_unit, true);
}

Unit* UnitAI::DoSelectLowestHpFriendly(float range, float minMissing, bool percent, bool targetSelf)
{
    Unit* pUnit = nullptr;

    if (percent)
    {
        MaNGOS::MostHPPercentMissingInRangeCheck u_check(m_unit, range, minMissing, true, targetSelf);
        MaNGOS::UnitLastSearcher<MaNGOS::MostHPPercentMissingInRangeCheck> searcher(pUnit, u_check);

        Cell::VisitGridObjects(m_unit, searcher, range);
    }
    else
    {
        MaNGOS::MostHPMissingInRangeCheck u_check(m_unit, range, minMissing, true, targetSelf);
        MaNGOS::UnitLastSearcher<MaNGOS::MostHPMissingInRangeCheck> searcher(pUnit, u_check);

        Cell::VisitGridObjects(m_unit, searcher, range);
    }

    return pUnit;
}

void UnitAI::DoResetThreat()
{
    if (!m_unit->CanHaveThreatList() || m_unit->getThreatManager().isThreatListEmpty())
    {
        script_error_log("DoResetThreat called for creature that either cannot have threat list or has empty threat list (m_creature entry = %d)", m_unit->GetEntry());
        return;
    }

    m_unit->getThreatManager().modifyAllThreatPercent(-100);
}

bool UnitAI::CanExecuteCombatAction()
{
    return m_unit->CanReactInCombat() && !(m_unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED) && m_unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED)) && !m_unit->hasUnitState(UNIT_STAT_PROPELLED | UNIT_STAT_RETREATING) && !m_unit->IsNonMeleeSpellCasted(false) && !m_combatScriptHappening;
}

void UnitAI::SetMeleeEnabled(bool state)
{
    if (state == m_meleeEnabled)
        return;

    m_meleeEnabled = state;
    if (m_unit->IsInCombat())
    {
        if (m_meleeEnabled)
        {
            if (m_unit->GetVictim())
                m_unit->MeleeAttackStart(m_unit->GetVictim());
        }
        else
            m_unit->MeleeAttackStop(m_unit->GetVictim());
    }
}

void UnitAI::DoStartMovement(Unit* victim)
{
    if (victim)
        m_unit->GetMotionMaster()->MoveChase(victim, m_attackDistance, m_attackAngle, m_moveFurther, !m_chaseRun);
}

SpellSchoolMask UnitAI::GetMainAttackSchoolMask() const
{
    return m_unit->GetMeleeDamageSchoolMask();
}

void UnitAI::TimedFleeingEnded()
{
    if (GetAIOrder() != ORDER_FLEEING)
        return; // prevent stack overflow by cyclic calls - TODO: remove once Motion Master is human again
    SetAIOrder(ORDER_NONE);
    SetCombatScriptStatus(false);
    if (!m_unit->IsAlive())
        return;
    DoStartMovement(m_unit->GetVictim());
}

bool UnitAI::DoFlee()
{
    Unit* victim = m_unit->GetVictim();
    if (!victim)
        return false;

    // call the fear method and check if fear method succeed
    if (!m_unit->SetInPanic(sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_FLEE_DELAY)))
        return false;

    // set the ai state to feared so it can reset movegen and ai state at the end of the fear
    SetAIOrder(ORDER_FLEEING);
    SetCombatScriptStatus(true);
    return true;
}

void UnitAI::DistancingStarted()
{
    SetCombatScriptStatus(true);
}

void UnitAI::DistancingEnded()
{
    SetCombatScriptStatus(false);
}

void UnitAI::AttackSpecificEnemy(std::function<void(Unit*, Unit*&)> check)
{
    Unit* chosenEnemy = nullptr;
    float distance = std::numeric_limits<float>::max();
    ThreatList const& list = m_unit->getThreatManager().getThreatList();
    for (auto& data : list)
    {
        Unit* enemy = data->getTarget();
        check(enemy, chosenEnemy);
    }

    AttackStart(chosenEnemy);
}

void UnitAI::AttackClosestEnemy()
{
    float distance = std::numeric_limits<float>::max();
    AttackSpecificEnemy([&](Unit* enemy, Unit*& closestEnemy)
    {
        float curDistance = enemy->GetDistance(m_unit, true, DIST_CALC_NONE);
        if (!closestEnemy || curDistance < distance)
        {
            closestEnemy = enemy;
            distance = curDistance;
        }
    });
}

void UnitAI::SetRootSelf(bool apply, bool combatOnly)
{
    if (combatOnly)
        m_selfRooted = apply;
    m_unit->SetImmobilizedState(apply);
}

void UnitAI::ClearSelfRoot()
{
    if (m_selfRooted)
    {
        m_unit->SetImmobilizedState(false);
        m_selfRooted = false;
    }
}

void UnitAI::DespawnGuids(GuidVector& spawns)
{
    for (ObjectGuid& guid : spawns)
        if (Creature* spawn = m_unit->GetMap()->GetAnyTypeCreature(guid))
            spawn->ForcedDespawn();
    spawns.clear();
}
