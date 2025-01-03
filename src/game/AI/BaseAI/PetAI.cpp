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

#include "PetAI.h"
#include "Util/Errors.h"
#include "Entities/Pet.h"
#include "Entities/Player.h"
#include "Server/DBCStores.h"
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "Entities/Creature.h"
#include "World/World.h"
#include "Util/Util.h"

int PetAI::Permissible(const Creature* creature)
{
    if (creature->IsPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature* creature) : PetAI(creature, 0)
{

}

PetAI::PetAI(Creature* creature, uint32 combatActions) : CreatureAI(creature, combatActions), inCombat(false)
{
    m_followAngle = M_PI_F / 2;
    m_followDist = 1.5f;
    m_AllySet.clear();
    UpdateAllies();

    if (creature->IsPet())
    {
        Pet* pet = dynamic_cast<Pet*>(creature);
        if (pet->isControlled())
        {
            if (sWorld.getConfig(CONFIG_BOOL_PET_ATTACK_FROM_BEHIND))
                m_attackAngle = M_PI_F;

            if (Unit* owner = m_unit->GetOwner())
                RequestFollow(owner);
        }
    }

    switch (creature->GetUInt32Value(UNIT_CREATED_BY_SPELL))
    {
        case 688: // imp
            m_attackDistance = 25.f;
            m_meleeEnabled = false;
            break;
    }

    if (m_creature->IsGuard() || m_unit->GetCharmInfo()) // guards and charmed targets
        m_visibilityDistance = sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER);
}

void PetAI::MoveInLineOfSight(Unit* who)
{
    if (Unit* victim = m_unit->GetVictim())
        if (victim->IsAlive())
            return;

    Pet* pet = (m_unit->GetTypeId() == TYPEID_UNIT && static_cast<Creature*>(m_unit)->IsPet()) ? static_cast<Pet*>(m_unit) : nullptr;

    if (HasReactState(REACT_AGGRESSIVE)
            && !(pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS)
            && !(who->GetTypeId() == TYPEID_UNIT && static_cast<Creature*>(who)->IsCivilian())
            && m_creature->CanAttackOnSight(who) && who->isInAccessablePlaceFor(m_unit)
            && m_unit->IsWithinDistInMap(who, m_unit->GetAttackDistance(who))
            && m_unit->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE_MELEE
            && m_unit->IsWithinLOSInMap(who, true))
    {
        AttackStart(who);

        if (Unit* owner = m_unit->GetOwner())
            owner->SetInCombatState(true, who);
    }
}

void PetAI::AttackStart(Unit* who)
{
    Pet* pet = (m_unit->GetTypeId() == TYPEID_UNIT && static_cast<Creature*>(m_unit)->IsPet()) ? static_cast<Pet*>(m_unit) : nullptr;

    if (!who || (pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS))
        return;

    // Do not start attack if target is moving home
    if (who->GetCombatManager().IsEvadingHome())
        return;

    if (m_unit->Attack(who, m_meleeEnabled))
    {
        // TMGs call CreatureRelocation which via MoveInLineOfSight can call this function
        // thus with the following clear the original TMG gets invalidated and crash, doh
        // hope it doesn't start to leak memory without this :-/
        // i_pet->Clear();
        HandleMovementOnAttackStart(who, false);

        inCombat = true;
    }
}

void PetAI::EnterEvadeMode()
{
    CharmInfo* charminfo = m_unit->GetCharmInfo();
    if (CanHandleCharm() && !charminfo)
    {
        CreatureAI::EnterEvadeMode();
        return;
    }

    // check for "chain pull" scenario - pet has already been sent to attack while exiting from an earlier combat
    // avoid AttackStop in CombatStop so that pet doesn't lose current target and return to follow owner in this case
    if (m_unit->GetTarget() && m_unit->GetVictim())
    {
        m_unit->RemoveAllAttackers();
        m_unit->DeleteThreatList();
        m_unit->GetCombatManager().StopCombatTimer();
        m_unit->ClearInCombat();
    }
    else
        m_unit->CombatStop();
}

void PetAI::UpdateAI(const uint32 diff)
{
    Unit* owner = m_unit->GetMaster();

    if (CanHandleCharm() && !owner)
    {
        CreatureAI::UpdateAI(diff);
        return;
    }

    UpdateTimers(diff, m_creature->IsInCombat());

    Creature* creature = (m_unit->GetTypeId() == TYPEID_UNIT) ? static_cast<Creature*>(m_unit) : nullptr;
    Pet* pet = (creature && creature->IsPet()) ? static_cast<Pet*>(m_unit) : nullptr;

    if (!owner)
        return;

    if (m_updateAlliesTimer <= diff)
        // UpdateAllies self set update timer
        UpdateAllies();
    else
        m_updateAlliesTimer -= diff;

    Unit* victim = (pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS) ? nullptr : m_unit->GetVictim();

    // Do not continue attacking if victim is moving home
    if (victim && victim->GetCombatManager().IsEvadingHome())
        victim = nullptr;

    CharmInfo* charminfo = m_unit->GetCharmInfo();
    MANGOS_ASSERT(charminfo);

    // Stop auto attack and chase if victim was dropped
    if (inCombat && (!victim || (victim->IsCrowdControlled() && victim->HasAuraPetShouldAvoidBreaking(pet, charminfo->GetPetLastAttackCommandTime()))))
    {
        m_unit->AttackStop(true, true);
        inCombat = false;

        MotionMaster* mm = m_unit->GetMotionMaster();

        if (mm->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
            mm->MovementExpired();
    }

    if (charminfo->GetIsRetreating())
    {
        if (!owner->IsWithinDistInMap(m_unit, (PET_FOLLOW_DIST * 2)))
        {
            if (!m_unit->hasUnitState(UNIT_STAT_FOLLOW))
                m_unit->GetMotionMaster()->MoveFollow(owner, m_followDist, m_followAngle);

            return;
        }
        charminfo->SetIsRetreating();
    }

    ExecuteActions();

    // Stop here if casting spell (No melee and no movement)
    if (m_unit->IsNonMeleeSpellCasted(false))
        return;
    // Auto cast (casted only in combat or persistent spells in any state)
    else
    {
        typedef std::vector<std::pair<Unit*, Spell*> > TargetSpellList;
        TargetSpellList targetSpellStore;
        if (charminfo->GetSpellOpener() != 0) // have opener stored
        {
            uint32 minRange = charminfo->GetSpellOpenerMinRange();
            victim = m_unit->GetVictim();

            if (!victim || (minRange != 0 && m_unit->IsWithinDistInMap(victim, minRange)))
                charminfo->SetSpellOpener();
            else if (m_unit->IsWithinDistInMap(victim, charminfo->GetSpellOpenerMaxRange())
                     && m_unit->IsWithinLOSInMap(victim, true))
            {
                uint32 spellId = charminfo->GetSpellOpener();
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
                uint32 flags = TRIGGERED_NORMAL_COMBAT_CAST;
                if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    flags |= TRIGGERED_PET_CAST;

                Spell* spell = new Spell(m_unit, spellInfo, flags);

                // Push back stored spell
                targetSpellStore.push_back(TargetSpellList::value_type(victim, spell));

                // Clear spell opener
                charminfo->SetSpellOpener();
            }
        }
        else if (creature)
        {
            for (uint8 i = 0; i < creature->GetPetAutoSpellSize(); ++i)
            {
                uint32 spellId = creature->GetPetAutoSpellOnPos(i);
                if (!spellId)
                    continue;

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
                if (!spellInfo)
                    continue;

                if (!m_unit->IsSpellReady(*spellInfo))
                    continue;

                // ignore some combinations of combat state and combat/non combat spells
                if (!inCombat)
                {
                    // ignore attacking spells, and allow only self/around spells
                    if (!IsPositiveSpell(spellInfo->Id))
                        continue;

                    // non combat spells allowed
                    // only pet spells have IsNonCombatSpell and not fit this requirements:
                    // Consume Shadows, Lesser Invisibility, so ignore checks for its
                    if (!IsNonCombatSpell(spellInfo))
                    {
                        int32 duration = GetSpellDuration(spellInfo);
                        int32 cooldown = GetSpellRecoveryTime(spellInfo);

                        // allow only spell not on cooldown
                        if (cooldown != 0 && duration < cooldown)
                            continue;
                    }
                }
                // just ignore non-combat spells
                else if (IsNonCombatSpell(spellInfo))
                    continue;

                Spell* spell = new Spell(m_unit, spellInfo, TRIGGERED_NORMAL_COMBAT_CAST);

                if (inCombat && spell->CanAutoCast(IsOnlySelfTargeting(spellInfo) ? m_unit : victim))
                {
                    targetSpellStore.push_back(TargetSpellList::value_type(victim, spell));
                }
                else if (!IsSpellRequireTarget(spellInfo))
                {
                    if (!spell->CanAutoCast(m_unit))
                    {
                        delete spell;
                        continue;
                    }

                    targetSpellStore.push_back(TargetSpellList::value_type(nullptr, spell));
                }
                else
                {
                    bool spellUsed = false;
                    for (auto tar : m_AllySet)
                    {
                        Unit* Target = m_unit->GetMap()->GetUnit(tar);

                        // only buff targets that are in combat, unless the spell can only be cast while out of combat
                        if (!Target)
                            continue;

                        if (spell->CanAutoCast(Target))
                        {
                            targetSpellStore.push_back(TargetSpellList::value_type(Target, spell));
                            spellUsed = true;
                            break;
                        }
                    }
                    if (!spellUsed)
                        delete spell;
                }
            }
        }

        // found units to cast on to
        if (!targetSpellStore.empty())
        {
            uint32 index = urand(0, targetSpellStore.size() - 1);

            Spell* spell  = targetSpellStore[index].second;
            Unit*  target = targetSpellStore[index].first;

            targetSpellStore.erase(targetSpellStore.begin() + index);

            SpellCastTargets targets;
            if (target)
            {
                targets.setUnitTarget(target);

                if (!m_unit->HasInArc(target))
                    m_unit->SetFacingToObject(target);
            }

            if (pet)
                pet->CheckLearning(spell->m_spellInfo->Id);

            spell->SpellStart(&targets);
        }

        // deleted cached Spell objects
        for (TargetSpellList::const_iterator itr = targetSpellStore.begin(); itr != targetSpellStore.end(); ++itr)
            delete itr->second;
    }

    // we may get our actions disabled during spell casting, so do entire recheck for victim
    victim = (pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS) ? nullptr : m_unit->GetVictim();

    if (victim)
    {
        // i_pet.getVictim() can't be used for check in case stop fighting, i_pet.GetVictim() clear at Unit death etc.
        // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
        if (!m_unit->CanAttack(victim))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "PetAI (guid = %u) is stopping attack.", m_unit->GetGUIDLow());
            m_unit->CombatStop();
            inCombat = false;

            return;
        }

        // if pet misses its target, it will also be the first in threat list
        if ((!creature || m_meleeEnabled)
                && m_unit->CanReachWithMeleeAttack(victim))
        {
            if (!m_unit->HasInArc(victim, 2 * M_PI_F / 3))
                m_unit->SetFacingToObject(victim);

            DoMeleeAttackIfReady();
        }
        else if (!m_unit->hasUnitState(UNIT_STAT_MOVING))
            AttackStart(victim);
    }
    else if (!m_unit->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
    {
        CharmInfo* charmInfo = m_unit->GetCharmInfo();

        const bool staying = (charmInfo && charmInfo->HasCommandState(COMMAND_STAY));
        const bool following = (!staying && charmInfo && charmInfo->HasCommandState(COMMAND_FOLLOW));

        // If not commanded to stay, try to assist owner first
        if (!staying && owner->IsInCombat() && !HasReactState(REACT_PASSIVE))
        {
            AttackStart(owner->getAttackerForHelper());

            // If target was acquired, skip non-combat movement handling
            if (inCombat)
                return;
        }

        // Handle non-combat movement
        if (!m_unit->hasUnitState(UNIT_STAT_NO_FREE_MOVE | UNIT_STAT_CHASE))
        {
            MotionMaster* mm = m_unit->GetMotionMaster();

            if (staying && !m_unit->hasUnitState(UNIT_STAT_STAY))
            {
                // If stay command is set, but we don't have stay pos yet: establish current pos as stay position, adjust orientation
                if (charminfo->UpdateStayPosition())
                    mm->MoveStay(charminfo->GetStayPosX(), charminfo->GetStayPosY(), charminfo->GetStayPosZ(), charminfo->GetStayPosO());
                 else
                    mm->MoveStay(charminfo->GetStayPosX(), charminfo->GetStayPosY(), charminfo->GetStayPosZ());
            }
            else if (following && !m_unit->hasUnitState(UNIT_STAT_FOLLOW))
                mm->MoveFollow(owner, m_followDist, m_followAngle);
        }
    }
}

void PetAI::UpdateAllies()
{
    Unit* owner = m_unit->GetMaster();
    Group* group = nullptr;

    m_updateAlliesTimer = 10 * IN_MILLISECONDS;             // update friendly targets every 10 seconds, fewer checks increase performance

    if (!owner)
        return;
    if (owner->GetTypeId() == TYPEID_PLAYER)
        group = ((Player*)owner)->GetGroup();

    // only pet and owner/not in group->ok
    if (m_AllySet.size() == 2 && !group)
        return;
    // owner is in group; group members filled in already (no raid -> subgroupcount = whole count)
    if (group && !group->IsRaidGroup() && m_AllySet.size() == (group->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(m_unit->GetObjectGuid());  // add self
    m_AllySet.insert(owner->GetObjectGuid());   // add owner

    if (group)                                  // add group
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* target = itr->getSource();
            if (!target || !group->SameSubGroup((Player*)owner, target))
                continue;

            if (target->GetObjectGuid() == owner->GetObjectGuid())
                continue;

            m_AllySet.insert(target->GetObjectGuid());
        }
    }
}

void PetAI::OnUnsummon()
{
    CreatureAI::OnUnsummon();
    RelinquishFollow(m_unit->GetOwnerGuid());
}

void PetAI::JustDied(Unit* killer)
{
    CreatureAI::JustDied(killer);
    RelinquishFollow(m_unit->GetOwnerGuid());
}

void PetAI::JustRespawned()
{
    if (CanHandleCharm())
    {
        CreatureAI::JustRespawned();
        return;
    }
}

void PetAI::AttackedBy(Unit* attacker)
{
    CharmInfo* charminfo = m_unit->GetCharmInfo();
    if (CanHandleCharm() && !charminfo)
    {
        CreatureAI::AttackedBy(attacker);
        return;
    }

    MANGOS_ASSERT(charminfo);

    // when attacked, fight back if no victim unless we have a charm state set to passive
    if (!(m_unit->GetVictim() || charminfo->GetIsRetreating() || HasReactState(REACT_PASSIVE)))
        AttackStart(attacker);
}
