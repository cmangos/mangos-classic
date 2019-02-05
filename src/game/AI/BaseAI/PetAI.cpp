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
#include "Errors.h"
#include "Entities/Pet.h"
#include "Entities/Player.h"
#include "Server/DBCStores.h"
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "Entities/Creature.h"
#include "World/World.h"
#include "Util.h"

int PetAI::Permissible(const Creature* creature)
{
    if (creature->IsPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature* creature) : UnitAI(creature), m_creature(creature), inCombat(false), m_followAngle(M_PI_F / 2), m_followDist(1.5f)
{
    m_AllySet.clear();
    UpdateAllies();

    if (creature->IsPet() && dynamic_cast<Pet*>(creature)->isControlled()
        && sWorld.getConfig(CONFIG_BOOL_PET_ATTACK_FROM_BEHIND))
        m_attackAngle = M_PI_F;

    switch (creature->GetUInt32Value(UNIT_CREATED_BY_SPELL))
    {
        case 688: // imp
            m_attackDistance = 25.f;
            m_meleeEnabled = false;
            break;
    }

    // TODO: Remove this when unit support is removed
    SetMeleeEnabled(!(m_creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_MELEE));
    if (m_creature->IsNoAggroOnSight())
        SetReactState(REACT_DEFENSIVE);
    if (m_creature->IsGuard() || m_unit->GetCharmInfo()) // guards and charmed targets
        m_visibilityDistance = sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER);
}

PetAI::PetAI(Unit* unit) : UnitAI(unit), inCombat(false), m_followAngle(PET_FOLLOW_ANGLE), m_followDist(PET_FOLLOW_DIST)
{
    m_AllySet.clear();
    UpdateAllies();
}

void PetAI::MoveInLineOfSight(Unit* who)
{
    if (Unit* victim = m_unit->getVictim())
        if (victim->isAlive())
            return;

    Pet* pet = (m_unit->GetTypeId() == TYPEID_UNIT && static_cast<Creature*>(m_unit)->IsPet()) ? static_cast<Pet*>(m_unit) : nullptr;

    if (HasReactState(REACT_AGGRESSIVE)
            && !(pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS)
            && m_creature->CanAttackOnSight(who) && who->isInAccessablePlaceFor(m_unit)
            && m_unit->IsWithinDistInMap(who, m_unit->GetAttackDistance(who))
            && m_unit->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE
            && m_unit->IsWithinLOSInMap(who))
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

    if (m_unit->Attack(who, m_meleeEnabled))
    {
        // TMGs call CreatureRelocation which via MoveInLineOfSight can call this function
        // thus with the following clear the original TMG gets invalidated and crash, doh
        // hope it doesn't start to leak memory without this :-/
        // i_pet->Clear();
        HandleMovementOnAttackStart(who);

        inCombat = true;
    }
}

void PetAI::EnterEvadeMode()
{
}

void PetAI::UpdateAI(const uint32 diff)
{
    if (!m_unit->isAlive())
        return;
    Creature* creature = (m_unit->GetTypeId() == TYPEID_UNIT) ? static_cast<Creature*>(m_unit) : nullptr;
    Pet* pet = (creature && creature->IsPet()) ? static_cast<Pet*>(m_unit) : nullptr;

    Unit* owner = m_unit->GetMaster();
    if (!owner)
        return;

    Unit* victim = (pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS) ? nullptr : m_unit->getVictim();

    if (m_updateAlliesTimer <= diff)
        // UpdateAllies self set update timer
        UpdateAllies();
    else
        m_updateAlliesTimer -= diff;

    if (inCombat && !victim)
    {
        m_unit->AttackStop(true, true);
        inCombat = false;
    }

    CharmInfo* charminfo = m_unit->GetCharmInfo();
    MANGOS_ASSERT(charminfo);

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
    else if (charminfo->GetSpellOpener() != 0) // have opener stored
    {
        uint32 minRange = charminfo->GetSpellOpenerMinRange();
        victim = m_unit->getVictim();

        if (!victim
                || (minRange != 0 && m_unit->IsWithinDistInMap(victim, minRange)))
            charminfo->SetSpellOpener();
        else if (m_unit->IsWithinDistInMap(victim, charminfo->GetSpellOpenerMaxRange())
                 && m_unit->IsWithinLOSInMap(victim))
        {
            // stop moving
            m_unit->clearUnitState(UNIT_STAT_MOVING);

            // auto turn to target
            m_unit->SetInFront(victim);

            if (victim->GetTypeId() == TYPEID_PLAYER)
                m_unit->SendCreateUpdateToPlayer((Player*)victim);

            if (owner->GetTypeId() == TYPEID_PLAYER)
                m_unit->SendCreateUpdateToPlayer((Player*)owner);

            uint32 spellId = charminfo->GetSpellOpener();
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

            Spell* spell = new Spell(m_unit, spellInfo, TRIGGERED_NONE);

            SpellCastResult result = spell->CheckPetCast(victim);

            if (result == SPELL_CAST_OK)
                spell->SpellStart(&(spell->m_targets));
            else
                delete spell;

            charminfo->SetSpellOpener();
        }
        else
            return;
    }
    // Auto cast (casted only in combat or persistent spells in any state)
    else if (!m_unit->IsNonMeleeSpellCasted(false))
    {
        typedef std::vector<std::pair<Unit*, Spell*> > TargetSpellList;
        TargetSpellList targetSpellStore;
        if (pet)
        {
            for (uint8 i = 0; i < pet->GetPetAutoSpellSize(); ++i)
            {
                uint32 spellId = pet->GetPetAutoSpellOnPos(i);
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

                        // not allow instant kill auto casts as full health cost
                        if (IsSpellHaveEffect(spellInfo, SPELL_EFFECT_INSTAKILL))
                            continue;
                    }
                }
                // just ignore non-combat spells
                else if (IsNonCombatSpell(spellInfo))
                    continue;

                Spell* spell = new Spell(m_unit, spellInfo, false);

                if (inCombat && spell->CanAutoCast(victim))
                {
                    targetSpellStore.push_back(TargetSpellList::value_type(victim, spell));
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
            targets.setUnitTarget(target);

            if (!m_unit->HasInArc(target))
            {
                m_unit->SetInFront(target);
                if (target->GetTypeId() == TYPEID_PLAYER)
                    m_unit->SendCreateUpdateToPlayer((Player*)target);

                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                    m_unit->SendCreateUpdateToPlayer((Player*)owner);
            }

            if (pet)
                pet->CheckLearning(spell->m_spellInfo->Id);

            spell->SpellStart(&targets);
        }

        // deleted cached Spell objects
        for (TargetSpellList::const_iterator itr = targetSpellStore.begin(); itr != targetSpellStore.end(); ++itr)
            delete itr->second;
    }

    // Stop here if casting spell (No melee and no movement)
    if (m_unit->IsNonMeleeSpellCasted(false))
        return;

    // we may get our actions disabled during spell casting, so do entire recheck for victim
    victim = (pet && pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS) ? nullptr : m_unit->getVictim();

    if (victim)
    {
        // i_pet.getVictim() can't be used for check in case stop fighting, i_pet.getVictim() clear at Unit death etc.
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
            {
                m_unit->SetInFront(victim);
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    m_unit->SendCreateUpdateToPlayer((Player*)victim);

                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                    m_unit->SendCreateUpdateToPlayer((Player*)owner);
            }

            DoMeleeAttackIfReady();
        }
        else if (!m_unit->hasUnitState(UNIT_STAT_MOVING))
            AttackStart(victim);
    }
    else if (!owner->IsIncapacitated())
    {
        CharmInfo* charmInfo = m_unit->GetCharmInfo();

        const bool staying = (charmInfo && charmInfo->HasCommandState(COMMAND_STAY));
        const bool following = (!staying && charmInfo && charmInfo->HasCommandState(COMMAND_FOLLOW));

        if (owner->isInCombat() && !HasReactState(REACT_PASSIVE) && !staying)
            AttackStart(owner->getAttackerForHelper());
        else
        {
            if (staying)
            {
                //if stay command is set but we don't have stay pos set then we need to establish current pos as stay position
                if (!charminfo->IsStayPosSet())
                    charminfo->SetStayPosition(true);

                float stayPosX = charminfo->GetStayPosX();
                float stayPosY = charminfo->GetStayPosY();
                float stayPosZ = charminfo->GetStayPosZ();

                if (int32(m_unit->GetPositionX()) == int32(stayPosX)
                        && int32(m_unit->GetPositionY()) == int32(stayPosY)
                        && int32(m_unit->GetPositionZ()) == int32(stayPosZ))
                {
                    float StayPosO = charminfo->GetStayPosO();

                    if (m_unit->hasUnitState(UNIT_STAT_MOVING))
                    {
                        m_unit->GetMotionMaster()->Clear(false);
                        m_unit->GetMotionMaster()->MoveIdle();
                    }
                    else if (int32(m_unit->GetOrientation()) != int32(StayPosO))
                        m_unit->SetOrientation(StayPosO);
                }
                else
                    m_unit->GetMotionMaster()->MovePoint(0, stayPosX, stayPosY, stayPosZ, false);
            }
            else if (m_unit->hasUnitState(UNIT_STAT_FOLLOW) && !m_unit->hasUnitState(UNIT_STAT_FOLLOW_MOVE) && owner->IsWithinDistInMap(m_unit, PET_FOLLOW_DIST))
            {
                m_unit->GetMotionMaster()->Clear(false);
                m_unit->GetMotionMaster()->MoveIdle();
            }
            else if (!m_unit->hasUnitState(UNIT_STAT_FOLLOW_MOVE) && !owner->IsWithinDistInMap(m_unit, (PET_FOLLOW_DIST * 2)))
            {
                if (following)
                {
                    m_unit->GetMotionMaster()->Clear(false);
                    m_unit->GetMotionMaster()->MoveFollow(owner, m_followDist, m_followAngle);
                }
            }
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
    if (group && !group->isRaidGroup() && m_AllySet.size() == (group->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(m_unit->GetObjectGuid());
    if (group)                                             // add group
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
    else                                                    // remove group
        m_AllySet.insert(owner->GetObjectGuid());
}

void PetAI::AttackedBy(Unit* attacker)
{
    CharmInfo* charminfo = m_unit->GetCharmInfo();
    MANGOS_ASSERT(charminfo);

    // when attacked, fight back if no victim unless we have a charm state set to passive
    if (!(m_unit->getVictim() || charminfo->GetIsRetreating() || HasReactState(REACT_PASSIVE)))
        AttackStart(attacker);
}
