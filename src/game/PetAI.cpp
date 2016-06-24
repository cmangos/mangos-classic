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
#include "Pet.h"
#include "Player.h"
#include "DBCStores.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "World.h"
#include "Util.h"

int PetAI::Permissible(const Creature* creature)
{
    if (creature->IsPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature* c) : CreatureAI(c), i_tracker(TIME_INTERVAL_LOOK), inCombat(false)
{
    m_AllySet.clear();
    UpdateAllies();
}

void PetAI::MoveInLineOfSight(Unit* u)
{
    if (m_creature->getVictim())
        return;

    if (m_creature->IsPet() && ((Pet*)m_creature)->GetModeFlags() & PET_MODE_DISABLE_ACTIONS)
        return;

    if (!m_creature->GetCharmInfo() || !m_creature->GetCharmInfo()->HasReactState(REACT_AGGRESSIVE))
        return;

    if (u->isTargetableForAttack() && u->isInAccessablePlaceFor(m_creature) &&
           (m_creature->IsHostileTo(u) || u->IsHostileTo(m_creature->GetCharmerOrOwner())))
    {
        float attackRadius = m_creature->GetAttackDistance(u);
        if (m_creature->IsWithinDistInMap(u, attackRadius) && m_creature->GetDistanceZ(u) <= CREATURE_Z_ATTACK_RANGE)
        {
            if (m_creature->IsWithinLOSInMap(u))
            {
                AttackStart(u);
                u->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            }
        }
    }
}

void PetAI::AttackStart(Unit* u)
{
    if (!u || ((Pet*)m_creature)->getPetType() == MINI_PET)
        return;

    if (m_creature->Attack(u, true))
    {
        // TMGs call CreatureRelocation which via MoveInLineOfSight can call this function
        // thus with the following clear the original TMG gets invalidated and crash, doh
        // hope it doesn't start to leak memory without this :-/
        // i_pet->Clear();
        if (!m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
            HandleMovementOnAttackStart(u);

        inCombat = true;
    }
}

void PetAI::EnterEvadeMode()
{
}

bool PetAI::IsVisible(Unit* pl) const
{
    return _isVisible(pl);
}

bool PetAI::_needToStop() const
{
    // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
    if (m_creature->isCharmed() && m_creature->getVictim() == m_creature->GetCharmer())
        return true;

    return !m_creature->getVictim()->isTargetableForAttack();
}

void PetAI::_stopAttack()
{
    inCombat = false;

    bool useDefaultMovement = true;

    if (Unit* owner = m_creature->GetCharmerOrOwner())
    {
        if (CharmInfo* charmInfo = m_creature->GetCharmInfo())
        {
            if (Pet* pet = (Pet*)m_creature)
            {
                if (charmInfo->HasCommandState(COMMAND_FOLLOW))
                    pet->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

                else if (charmInfo->HasCommandState(COMMAND_STAY))
                {
                    //if stay command is already set but we dont have stay pos set then we need to establish current pos as stay position
                    if (!pet->IsStayPosSet())
                        pet->SetStayPosition();

                    pet->GetMotionMaster()->MovePoint(0, pet->GetStayPosX(), pet->GetStayPosY(), pet->GetStayPosZ(), false);
                }

                useDefaultMovement = false;
            }
        }
    }

    m_creature->AttackStop();

    if (useDefaultMovement)
    {
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MoveIdle();
    }
}

void PetAI::UpdateAI(const uint32 diff)
{
    if (!m_creature->isAlive())
        return;

    Unit* owner = m_creature->GetCharmerOrOwner();
    Unit* victim = m_creature->getVictim();

    if (m_updateAlliesTimer <= diff)
        // UpdateAllies self set update timer
        UpdateAllies();
    else
        m_updateAlliesTimer -= diff;

    if (inCombat && (!victim || (m_creature->IsPet() && ((Pet*)m_creature)->GetModeFlags() & PET_MODE_DISABLE_ACTIONS)))
        _stopAttack();

    if (((Pet*)m_creature)->GetIsRetreating())
    {
        if (!owner->_IsWithinDist(m_creature, (PET_FOLLOW_DIST * 2), true))
        {
            if (!m_creature->hasUnitState(UNIT_STAT_FOLLOW))
                m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

            return;
        }
        else
           ((Pet*)m_creature)->SetIsRetreating();
    }
    else if (((Pet*)m_creature)->GetSpellOpener() != 0) // have opener stored
    {
        uint32 minRange = ((Pet*)m_creature)->GetSpellOpenerMinRange();

        if (!(victim = m_creature->getVictim())
            || (minRange != 0 && m_creature->IsWithinDistInMap(victim, minRange)))
            ((Pet*)m_creature)->SetSpellOpener();
        else if (m_creature->IsWithinDistInMap(victim, ((Pet*)m_creature)->GetSpellOpenerMaxRange()) && m_creature->IsWithinLOSInMap(victim))
        {
            // stop moving
            m_creature->clearUnitState(UNIT_STAT_MOVING);

            // auto turn to target
            m_creature->SetInFront(victim);

            if (victim->GetTypeId() == TYPEID_PLAYER)
                m_creature->SendCreateUpdateToPlayer((Player*)victim);

            if (owner->GetTypeId() == TYPEID_PLAYER)
                m_creature->SendCreateUpdateToPlayer((Player*)owner);

            uint32 spell_id = ((Pet*)m_creature)->GetSpellOpener();
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);

            Spell* spell = new Spell(m_creature, spellInfo, false);

            SpellCastResult result = spell->CheckPetCast(victim);

            if (result == SPELL_CAST_OK)
            {
                m_creature->AddCreatureSpellCooldown(spell_id);

                m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                spell->SpellStart(&(spell->m_targets));
            }
            else
                delete spell;

            ((Pet*)m_creature)->SetSpellOpener();
        }

        else
            return;
    }
    // Autocast (casted only in combat or persistent spells in any state)
    else if (!m_creature->IsNonMeleeSpellCasted(false))
    {
        typedef std::vector<std::pair<Unit*, Spell*> > TargetSpellList;
        TargetSpellList targetSpellStore;

        for (uint8 i = 0; i < m_creature->GetPetAutoSpellSize(); ++i)
        {
            uint32 spellID = m_creature->GetPetAutoSpellOnPos(i);
            if (!spellID)
                continue;

            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellID);
            if (!spellInfo)
                continue;

            if (m_creature->GetCharmInfo() && m_creature->GetCharmInfo()->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo))
                continue;

            // ignore some combinations of combat state and combat/noncombat spells
            if (!inCombat)
            {
                // ignore attacking spells, and allow only self/around spells
                if (!IsPositiveSpell(spellInfo->Id))
                    continue;

                // non combat spells allowed
                // only pet spells have IsNonCombatSpell and not fit this reqs:
                // Consume Shadows, Lesser Invisibility, so ignore checks for its
                if (!IsNonCombatSpell(spellInfo))
                {
                    int32 duration = GetSpellDuration(spellInfo);
                    int32 cooldown = GetSpellRecoveryTime(spellInfo);

                    // allow only spell not on cooldown
                    if (cooldown != 0 && duration < cooldown)
                        continue;

                    // not allow instant kill autocasts as full health cost
                    if (IsSpellHaveEffect(spellInfo, SPELL_EFFECT_INSTAKILL))
                        continue;
                }
            }
            // just ignore non-combat spells
            else if (IsNonCombatSpell(spellInfo))
                continue;

            Spell* spell = new Spell(m_creature, spellInfo, false);

            if (inCombat && !m_creature->hasUnitState(UNIT_STAT_FOLLOW) && spell->CanAutoCast(m_creature->getVictim()))
            {
                targetSpellStore.push_back(TargetSpellList::value_type(m_creature->getVictim(), spell));
                continue;
            }
            else
            {
                bool spellUsed = false;
                for (GuidSet::const_iterator tar = m_AllySet.begin(); tar != m_AllySet.end(); ++tar)
                {
                    Unit* Target = m_creature->GetMap()->GetUnit(*tar);

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

        // found units to cast on to
        if (!targetSpellStore.empty())
        {
            uint32 index = urand(0, targetSpellStore.size() - 1);

            Spell* spell  = targetSpellStore[index].second;
            Unit*  target = targetSpellStore[index].first;

            targetSpellStore.erase(targetSpellStore.begin() + index);

            SpellCastTargets targets;
            targets.setUnitTarget(target);

            if (!m_creature->HasInArc(M_PI_F, target))
            {
                m_creature->SetInFront(target);
                if (target->GetTypeId() == TYPEID_PLAYER)
                    m_creature->SendCreateUpdateToPlayer((Player*)target);

                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                    m_creature->SendCreateUpdateToPlayer((Player*)owner);
            }

            m_creature->AddCreatureSpellCooldown(spell->m_spellInfo->Id);
            if (m_creature->IsPet())
                ((Pet*)m_creature)->CheckLearning(spell->m_spellInfo->Id);

            m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            spell->SpellStart(&targets);
        }

        // deleted cached Spell objects
        for (TargetSpellList::const_iterator itr = targetSpellStore.begin(); itr != targetSpellStore.end(); ++itr)
            delete itr->second;
    }
    else if (m_creature->hasUnitState(UNIT_STAT_FOLLOW_MOVE))
        m_creature->InterruptNonMeleeSpells(false);

    if ((victim = m_creature->getVictim()) && !m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE))
    {
        // i_pet.getVictim() can't be used for check in case stop fighting, i_pet.getVictim() clear at Unit death etc.
        if (_needToStop())
        {
            DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "PetAI (guid = %u) is stopping attack.", m_creature->GetGUIDLow());
            _stopAttack();
            return;
        }

        // required to be stopped cases
        if (m_creature->IsStopped() && m_creature->IsNonMeleeSpellCasted(false))
        {
            if (m_creature->hasUnitState(UNIT_STAT_FOLLOW_MOVE))
                m_creature->InterruptNonMeleeSpells(false);
        }
        // if pet misses its target, it will also be the first in threat list
        else if (m_creature->CanReachWithMeleeAttack(victim))
        {
            if (!m_creature->HasInArc(2 * M_PI_F / 3, victim))
            {
                m_creature->SetInFront(victim);
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    m_creature->SendCreateUpdateToPlayer((Player*)victim);

                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                    m_creature->SendCreateUpdateToPlayer((Player*)owner);
            }

            if (!(m_creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_MELEE)
                && DoMeleeAttackIfReady())
            {
                m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                victim->AddThreat(m_creature);
            }
            else
                AttackStart(victim);
        }
    }
    else if (owner)
    {
        CharmInfo* charmInfo = m_creature->GetCharmInfo();
        if (owner->isInCombat() && !(m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE)
            || (charmInfo && charmInfo->HasReactState(REACT_PASSIVE))))
            AttackStart(owner->getAttackerForHelper());
        else if (m_creature->hasUnitState(UNIT_STAT_FOLLOW) || m_creature->hasUnitState(UNIT_STAT_MOVING))
        {
            if (owner->IsWithinDistInMap(m_creature, PET_FOLLOW_DIST) || m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
            {
                m_creature->GetMotionMaster()->Clear(false);
                m_creature->GetMotionMaster()->MoveIdle();
            }
        }
        else if (!(m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE) || owner->IsWithinDistInMap(m_creature, (PET_FOLLOW_DIST * 2)))
            && charmInfo && charmInfo->HasCommandState(COMMAND_FOLLOW))
            m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
}

bool PetAI::_isVisible(Unit* u) const
{
    return m_creature->IsWithinDist(u, sWorld.getConfig(CONFIG_FLOAT_SIGHT_GUARDER))
           && u->isVisibleForOrDetect(m_creature, m_creature, true);
}

void PetAI::UpdateAllies()
{
    Unit* owner = m_creature->GetCharmerOrOwner();
    Group* pGroup = nullptr;

    m_updateAlliesTimer = 10 * IN_MILLISECONDS;             // update friendly targets every 10 seconds, lesser checks increase performance

    if (!owner)
        return;
    else if (owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = ((Player*)owner)->GetGroup();

    // only pet and owner/not in group->ok
    if (m_AllySet.size() == 2 && !pGroup)
        return;
    // owner is in group; group members filled in already (no raid -> subgroupcount = whole count)
    if (pGroup && !pGroup->isRaidGroup() && m_AllySet.size() == (pGroup->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(m_creature->GetObjectGuid());
    if (pGroup)                                             // add group
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* target = itr->getSource();
            if (!target || !pGroup->SameSubGroup((Player*)owner, target))
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
    // when attacked, fight back if no victim unless we have a charm state set to passive
    if (!(m_creature->getVictim() || ((Pet*)m_creature)->GetIsRetreating() == true)
        && !(m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE)
        || (m_creature->GetCharmInfo() && m_creature->GetCharmInfo()->HasReactState(REACT_PASSIVE))))
        AttackStart(attacker);
}
