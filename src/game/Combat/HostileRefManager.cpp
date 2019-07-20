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

#include "Combat/HostileRefManager.h"
#include "Combat/ThreatManager.h"
#include "Entities/Unit.h"
#include "Server/DBCStructure.h"
#include "Spells/SpellMgr.h"

HostileRefManager::HostileRefManager(Unit* owner) : iOwner(owner)
{
}

HostileRefManager::~HostileRefManager()
{
    deleteReferences();
}

//=================================================
// send threat to all my haters for the pVictim
// The pVictim is hated than by them as well
// use for buffs and healing threat functionality

void HostileRefManager::threatAssist(Unit* victim, float threat, SpellEntry const* threatSpell, bool singleTarget)
{
    HostileReference* ref = getFirst();
    if (!ref)
        return;

    std::vector<HostileReference*> validRefs;
    while (ref)
    {
        Unit* owner = ref->getSource()->getOwner();
        if (!owner->IsIncapacitated() && owner != victim && owner->CanAttack(victim))
            validRefs.push_back(ref);
        ref = ref->next();
    }

    uint32 size = singleTarget ? 1 : validRefs.size();            // if singleTarget do not devide threat
    float threatPerTarget = threat / size;
    for (HostileReference* validReference : validRefs)
        validReference->getSource()->addThreat(victim, threatPerTarget, false, (threatSpell ? GetSpellSchoolMask(threatSpell) : SPELL_SCHOOL_MASK_NORMAL), threatSpell);
}

//=================================================

void HostileRefManager::addThreatPercent(int32 threatPercent)
{
    HostileReference* ref = getFirst();
    while (ref != nullptr)
    {
        ref->addThreatPercent(threatPercent);
        ref = ref->next();
    }
}

void HostileRefManager::threatTemporaryFade(Unit* /*victim*/, float threat, bool apply)
{
    HostileReference* ref = getFirst();

    while (ref)
    {
        if (apply)
        {
            float curThreat = ref->getThreat();
            float reducedThreat = std::max(-curThreat, threat);
            ref->setFadeoutThreatReduction(reducedThreat);
        }
        else
            ref->resetFadeoutThreatReduction();

        ref = ref->next();
    }
}

//=================================================
// The online / offline status is given to the method. The calculation has to be done before

void HostileRefManager::setOnlineOfflineState(bool isOnline)
{
    HostileReference* ref = getFirst();
    while (ref != nullptr)
    {
        ref->setOnlineOfflineState(isOnline);
        ref = ref->next();
    }
}

void HostileRefManager::updateOnlineOfflineState(bool pIsOnline)
{
    if (pIsOnline && iOwner)
    {
        // Do not set online while feigning death in combat
        if (iOwner->IsFeigningDeathSuccessfully() && iOwner->isInCombat())
            return;
    }
    setOnlineOfflineState(pIsOnline);
}

//=================================================
// The online / offline status is calculated and set

void HostileRefManager::updateThreatTables()
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        ref->updateOnlineStatus();
        ref = ref->next();
    }
}

//=================================================
// The references are not needed anymore
// tell the source to remove them from the list and free the mem

void HostileRefManager::deleteReferences()
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        ref->removeReference();
        delete ref;
        ref = nextRef;
    }
}

//=================================================
// delete one reference, defined by faction

void HostileRefManager::deleteReferencesForFaction(uint32 faction)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner()->GetFactionTemplateEntry()->faction == faction)
        {
            ref->removeReference();
            delete ref;
        }
        ref = nextRef;
    }
}

//=================================================
// delete one reference, defined by Unit

void HostileRefManager::deleteReference(Unit* victim)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == victim)
        {
            ref->removeReference();
            delete ref;
            break;
        }
        ref = nextRef;
    }
}

//=================================================
// set state for one reference, defined by Unit

void HostileRefManager::setOnlineOfflineState(Unit* victim, bool isOnline)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == victim)
        {
            ref->setOnlineOfflineState(isOnline);
            break;
        }
        ref = nextRef;
    }
}

HostileReference* HostileRefManager::getFirst()
{
    return static_cast<HostileReference*>(RefManager<Unit, ThreatManager>::getFirst());
}

void HostileRefManager::HandleSuppressed(bool apply, bool immunity)
{
    if (apply)
    {
        for (auto& data : *this)
        {
            HostileReference& ref = static_cast<HostileReference&>(data);
            if (immunity)
            {
                Unit* source = ref.getSource()->getOwner();
                Unit* target = ref.getTarget();
                if (!target->IsImmuneToDamage(source->GetMeleeDamageSchoolMask()))
                    continue;
            }
            ref.SetHostileState(STATE_SUPPRESSED);
        }
    }
    else
    {
        for (auto& data : *this)
        {
            HostileReference& ref = static_cast<HostileReference&>(data);
            Unit* source = ref.getSource()->getOwner();
            Unit* target = ref.getTarget();
            if (!target->IsSuppressedTarget(source))
            {
                ref.SetSuppressabilityToggle();
                continue;
            }
            ref.SetHostileState(STATE_SUPPRESSED);
        }
    }
}

//=================================================
