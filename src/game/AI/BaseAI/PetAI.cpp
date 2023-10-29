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

PetAI::PetAI(Creature* creature) : CreatureAI(creature), m_inCombat(false)
{
    m_followAngle = M_PI_F / 2;
    m_followDist = 1.5f;
    m_pet = creature->IsPet() ? static_cast<Pet*>(creature) : nullptr;

    if (m_pet && m_pet->isControlled())
    {
        if (sWorld.getConfig(CONFIG_BOOL_PET_ATTACK_FROM_BEHIND))
            m_attackAngle = M_PI_F;

        if (Unit* owner = m_unit->GetOwner())
            RequestFollow(owner);
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
    Unit* victim = m_unit->GetVictim();
    if (victim && victim->IsAlive())
        return;

    if (HasReactState(REACT_AGGRESSIVE)
            && !(m_pet && m_pet->HasActionsDisabled())
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
    if (!who || m_pet && m_pet->HasActionsDisabled())
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

        m_inCombat = true;
    }
}

void PetAI::EnterEvadeMode()
{
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
    UpdateTimers(diff, m_creature->IsInCombat());

    Unit* owner = m_unit->GetMaster();
    if (!owner)
        return;

    Unit* victim = m_pet && m_pet->HasActionsDisabled() ? nullptr : m_unit->GetVictim();

    // Do not continue attacking if victim is moving home
    if (victim && victim->GetCombatManager().IsEvadingHome())
        victim = nullptr;

    CharmInfo* charmInfo = m_unit->GetCharmInfo();
    MANGOS_ASSERT(charmInfo);

    // Stop auto attack and chase if victim was dropped
    if (m_inCombat && (!victim || (victim->IsCrowdControlled() && victim->HasAuraPetShouldAvoidBreaking(m_pet, charmInfo->GetPetLastAttackCommandTime()))))
    {
        m_unit->AttackStop(true, true);
        m_inCombat = false;

        MotionMaster* mm = m_unit->GetMotionMaster();

        if (mm->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
            mm->MovementExpired();
    }

    if (charmInfo->GetIsRetreating())
    {
        if (!owner->IsWithinDistInMap(m_unit, (PET_FOLLOW_DIST * 2)))
        {
            if (!m_unit->hasUnitState(UNIT_STAT_FOLLOW))
                m_unit->GetMotionMaster()->MoveFollow(owner, m_followDist, m_followAngle);

            return;
        }
        charmInfo->SetIsRetreating();
    }

    // Stop here if ranged spell casting is in progress
    if (m_unit->IsNonMeleeSpellCasted(false))
        return;

    // Cast either an opener spell or an autocast spell
    Cast(PickSpellWithTarget(owner, victim, charmInfo));

    if (victim)
    {
        // i_pet.getVictim() can't be used for check in case stop fighting, i_pet.GetVictim() clear at Unit death etc.
        // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
        if (!m_unit->CanAttack(victim))
        {
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "PetAI (guid = %u) is stopping attack.", m_unit->GetGUIDLow());
            m_unit->CombatStop();
            m_inCombat = false;

            return;
        }

        // if pet misses its target, it will also be the first in threat list
        if (m_meleeEnabled && m_unit->CanReachWithMeleeAttack(victim))
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
        const bool staying = charmInfo->HasCommandState(COMMAND_STAY);
        const bool following = !staying && charmInfo->HasCommandState(COMMAND_FOLLOW);

        // If not commanded to stay, try to assist owner first
        if (!staying && owner->IsInCombat() && !HasReactState(REACT_PASSIVE))
            AttackStart(owner->getAttackerForHelper());

        // Handle non-combat movement
        if (!m_inCombat && !m_unit->hasUnitState(UNIT_STAT_NO_FREE_MOVE | UNIT_STAT_CHASE))
        {
            MotionMaster* mm = m_unit->GetMotionMaster();

            if (staying && !m_unit->hasUnitState(UNIT_STAT_STAY))
            {
                // If stay command is set, but we don't have stay pos yet: establish current pos as stay position, adjust orientation
                if (charmInfo->UpdateStayPosition())
                    mm->MoveStay(charmInfo->GetStayPosX(), charmInfo->GetStayPosY(), charmInfo->GetStayPosZ(), charmInfo->GetStayPosO());
                 else
                    mm->MoveStay(charmInfo->GetStayPosX(), charmInfo->GetStayPosY(), charmInfo->GetStayPosZ());
            }
            else if (following && !m_unit->hasUnitState(UNIT_STAT_FOLLOW))
                mm->MoveFollow(owner, m_followDist, m_followAngle);
        }
    }
}

std::pair<Unit*, Spell*> PetAI::PickSpellWithTarget(Unit* owner, Unit* victim, CharmInfo* charmInfo)
{
    uint32 flags = TRIGGERED_NORMAL_COMBAT_CAST;

    // Cast an opener spell, if stored
    if (charmInfo->GetSpellOpener() != 0)
    {
        uint32 minRange = charmInfo->GetSpellOpenerMinRange();

        // Clear the spell opener in case target is too close
        if (minRange != 0 && m_unit->IsWithinDistInMap(victim, minRange))
            charmInfo->SetSpellOpener();
        // Use the spell opener if target is within the range and line of sight.
        // But do not drop the spell opener in case the condition is not met to give
        // opportunity to cast it when the victim gets visible or closer.
        else if (m_unit->IsWithinDistInMap(victim, charmInfo->GetSpellOpenerMaxRange())
                 && m_unit->IsWithinLOSInMap(victim, true))
        {
            uint32 spellId = charmInfo->GetSpellOpener();

            // Clear spell opener
            charmInfo->SetSpellOpener();

            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (!spellInfo)
                return { nullptr, nullptr };

            if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                flags |= TRIGGERED_PET_CAST;

            Spell* spell = new Spell(m_unit, spellInfo, flags);

            return { victim, spell };
        }
    }

    // Cast a spell from autocast selection
    for (uint8 i = 0; i < m_creature->GetPetAutoSpellSize(); ++i)
    {
        uint32 spellId = m_creature->GetPetAutoSpellOnPos(i);
        if (!spellId)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

        // Ignore spells that are:
        // 1. Not ready.
        // 2. Non-combat spells while in combat (f.e., Succubus's Lesser Invisibility).
        // 3. Combat spells while not in combat.
        if (!spellInfo || !m_unit->IsSpellReady(*spellInfo) || (m_inCombat && IsNonCombatSpell(spellInfo)) || (!m_inCombat && !IsPositiveSpell(spellInfo->Id)))
            continue;

        Spell* spell = new Spell(m_unit, spellInfo, flags);

        // Try to cast a spell on self if the spell allows only targeting self (like Lesser Invisibility and Blood Pact)
        if (IsOnlySelfTargeting(spellInfo)) {
            // Skip the spell in case it's already applied to self
            if (!spell->CanAutoCast(m_unit))
            {
                delete spell;
                continue;
            }

            return { m_unit, spell };
        }
        // Cast a spell on the victim in case the spell can be targeted at anyone
        // IsSpellRequireTarget() check is not really required since AoE spells (like Thunderstomp)
        // don't require target but need to be targeted at specific victim for distance check.
        else if (victim && spell->CanAutoCast(victim))
            return { victim, spell };
        // In all other cases, try to find a good use for the spell
        else
        {
            // Cast the spell on self, if applicable
            if (spell->CanAutoCast(m_unit))
                return { m_unit, spell };
            // Cast the spell on the pet owner, if applicable
            else if (spell->CanAutoCast(owner))
                return { owner, spell };
            // If pet owner is a player, get the group and cast the spell on anyone in the group
            else if (owner->GetTypeId() == TYPEID_PLAYER)
            {
                Player* target = PickGroupMemberForSpell(static_cast<Player*>(owner), spell);
                if (target)
                    return { target, spell };
            }
        }

        delete spell;
    }

    return { nullptr, nullptr };
}

Player* PetAI::PickGroupMemberForSpell(Player* player, Spell* spell)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (member && group->SameSubGroup(player, member) && member->GetObjectGuid() != player->GetObjectGuid() && spell->CanAutoCast(member))
            return member;
    }

    return nullptr;
}

void PetAI::Cast(std::pair<Unit*, Spell*> spellWithTarget)
{
    // Unpack the pair, check the spell and try to cast.
    Spell* spell  = spellWithTarget.second;
    Unit*  target = spellWithTarget.first;
    if (spell)
    {
        SpellCastTargets targets;
        if (target)
        {
            targets.setUnitTarget(target);

            if (!m_unit->HasInArc(target))
                m_unit->SetFacingToObject(target);
        }

        if (m_pet)
            m_pet->CheckLearning(spell->m_spellInfo->Id);

        spell->SpellStart(&targets);
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

void PetAI::AttackedBy(Unit* attacker)
{
    CharmInfo* charmInfo = m_unit->GetCharmInfo();
    MANGOS_ASSERT(charmInfo);

    // when attacked, fight back if no victim unless we have a charm state set to passive
    if (!(m_unit->GetVictim() || charmInfo->GetIsRetreating() || HasReactState(REACT_PASSIVE)))
        AttackStart(attacker);
}
