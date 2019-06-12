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

#include "Combat/ThreatManager.h"
#include "Entities/Unit.h"
#include "Entities/Creature.h"
#include "AI/BaseAI/CreatureAI.h"
#include "Maps/Map.h"
#include "Entities/Player.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/UnitEvents.h"
#include "Spells/SpellAuras.h"

//==============================================================
//================= ThreatCalcHelper ===========================
//==============================================================

// The pHatingUnit is not used yet
float ThreatCalcHelper::CalcThreat(Unit* hatedUnit, Unit* /*pHatingUnit*/, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const* threatSpell)
{
    // all flat mods applied early
    if (!threat)
        return 0.0f;

    if (hatedUnit->GetNoThreatState()) // some NPCs cause no threat
        return 0.0f;

    if (threatSpell)
    {
        // Keep exception to calculate the real threat for SPELL_AURA_MOD_TOTAL_THREAT
        bool HasExceptionForNoThreat = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; i++)
        {
            if (threatSpell->EffectApplyAuraName[i] == SPELL_AURA_MOD_TOTAL_THREAT && threatSpell->EffectBasePoints[i] < 0)
            {
                HasExceptionForNoThreat = true;
                break;
            }
        }

        if (!HasExceptionForNoThreat)
        {
            if (threatSpell->HasAttribute(SPELL_ATTR_EX_NO_THREAT))
                return 0.0f;

            if (Player* modOwner = hatedUnit->GetSpellModOwner())
                modOwner->ApplySpellMod(threatSpell->Id, SPELLMOD_THREAT, threat);
        }

        if (crit)
            threat *= hatedUnit->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRITICAL_THREAT, schoolMask);
    }

    threat = hatedUnit->ApplyTotalThreatModifier(threat, schoolMask);
    return threat;
}

//============================================================
//================= HostileReference ==========================
//============================================================

HostileReference::HostileReference(Unit* unit, ThreatManager* threatManager, float threat) : 
    m_hostileState(STATE_NORMAL), m_tauntState(STATE_NONE)
{
    iThreat = threat;
    iFadeoutThreadReduction = 0.f;
    link(unit, threatManager);
    iUnitGuid = unit->GetObjectGuid();
    m_online = true;
    iAccessible = true;
}

//============================================================
// Tell our refTo (target) object that we have a link
void HostileReference::targetObjectBuildLink()
{
    getTarget()->addHatedBy(this);
}

//============================================================
// Tell our refTo (taget) object, that the link is cut
void HostileReference::targetObjectDestroyLink()
{
    getTarget()->removeHatedBy(this);
}

//============================================================
// Tell our refFrom (source) object, that the link is cut (Target destroyed)

void HostileReference::sourceObjectDestroyLink()
{
    setOnlineOfflineState(false);
}

//============================================================
// Inform the source, that the status of the reference changed

void HostileReference::fireStatusChanged(ThreatRefStatusChangeEvent& threatRefStatusChangeEvent)
{
    if (getSource())
        getSource()->processThreatEvent(threatRefStatusChangeEvent);
}

//============================================================

void HostileReference::addThreat(float mod)
{
    if (mod + iThreat < 0)
        mod = -iThreat;

    iThreat += mod;
    // If threat has changed, clear suppress
    if (m_suppresabilityToggle && mod != 0.f)
        SetHostileState(STATE_NORMAL);

    if (mod != 0.0f)
    {
        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_THREAT_CHANGE, this, mod);
        fireStatusChanged(event);
    }

    if (isValid() && mod >= 0)
    {
        Unit* target = getTarget();
        Unit* victim_owner = target->GetOwner();
        if (victim_owner && victim_owner->isAlive() && getSource()->getOwner()->CanAttack(victim_owner))
            getSource()->addThreat(victim_owner, 0.0f);     // create a threat to the owner of a pet, if the pet attacks
    }
}

//============================================================
// check, if source can reach target and set the status

void HostileReference::updateOnlineStatus()
{
    bool online = false;
    if (!isValid())
    {
        if (Unit* target = ObjectAccessor::GetUnit(*getSourceUnit(), getUnitGuid()))
            link(target, getSource());
    }
    // only check for online status if
    // ref is valid
    if (isValid())
    {
        Unit* unit = getSourceUnit();
        Unit* target = getTarget();
        if (!unit->IsOfflineTarget(target))
            online = true;
    }
    setOnlineOfflineState(online);
}

//============================================================
// set the status and fire the event on status change

void HostileReference::setOnlineOfflineState(bool isOnline)
{
    if (m_online != isOnline)
    {
        m_online = isOnline;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ONLINE_STATUS, this);
        fireStatusChanged(event);
    }
}

//============================================================

void HostileReference::setAccessibleState(bool isAccessible)
{
    if (iAccessible != isAccessible)
    {
        iAccessible = isAccessible;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ASSECCIBLE_STATUS, this);
        fireStatusChanged(event);
    }
}

void HostileReference::SetHostileState(HostileState state)
{
    if (m_hostileState != state)
    {
        m_hostileState = state;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_SUPPRESSED_STATUS, this);
        fireStatusChanged(event);
    }
    m_suppresabilityToggle = false;
}

//============================================================
// prepare the reference for deleting
// this is called be the target

void HostileReference::removeReference()
{
    invalidate();

    ThreatRefStatusChangeEvent event(UEV_THREAT_REF_REMOVE_FROM_LIST, this);
    fireStatusChanged(event);
}

//============================================================

Unit* HostileReference::getSourceUnit() const
{
    return getSource()->getOwner();
}

//============================================================
//================ ThreatContainer ===========================
//============================================================

void ThreatContainer::clearReferences()
{
    for (ThreatList::const_iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        (*i)->unlink();
        delete (*i);
    }
    iThreatList.clear();
}

//============================================================
// Return the HostileReference of nullptr, if not found
HostileReference* ThreatContainer::getReferenceByTarget(Unit* victim)
{
    if (!victim)
        return nullptr;

    ObjectGuid guid = victim->GetObjectGuid();

    for (ThreatList::const_iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
        if ((*i)->getUnitGuid() == guid)
            return (*i);

    return nullptr;
}

//============================================================
// Add the threat, if we find the reference

HostileReference* ThreatContainer::addThreat(Unit* victim, float threat)
{
    HostileReference* ref = getReferenceByTarget(victim);
    if (ref)
        ref->addThreat(threat);
    return ref;
}

//============================================================

void ThreatContainer::modifyThreatPercent(Unit* victim, int32 threatPercent)
{
    if (HostileReference* ref = getReferenceByTarget(victim))
    {
        if (threatPercent < -100)
        {
            ref->removeReference();
            delete ref;
        }
        else
            ref->addThreatPercent(threatPercent);
    }
}

//============================================================

bool HostileReferenceSortPredicate(const HostileReference* lhs, const HostileReference* rhs)
{
    if (lhs->GetTauntState() != rhs->GetTauntState())
        return lhs->GetTauntState() > rhs->GetTauntState();
    if (lhs->GetHostileState() != rhs->GetHostileState())
        return lhs->GetHostileState() > rhs->GetHostileState();
    return lhs->getThreat() > rhs->getThreat();             // reverse sorting
}

//============================================================
// Check if the list is dirty and sort if necessary

void ThreatContainer::update()
{
    if (iDirty && iThreatList.size() > 1)
    {
        iThreatList.sort(HostileReferenceSortPredicate);
    }
    iDirty = false;
}

//============================================================
// return the next best victim
// could be the current victim

HostileReference* ThreatContainer::selectNextVictim(Unit* attacker, HostileReference* currentVictim)
{
    HostileReference* pCurrentRef = nullptr;
    bool found = false;
    bool onlySecondChoiceTargetsFound = false;
    bool checkedCurrentVictim = false;

    ThreatList::const_iterator lastRef = iThreatList.end();
    --lastRef;

    for (ThreatList::const_iterator iter = iThreatList.begin(); iter != iThreatList.end() && !found;)
    {
        pCurrentRef = (*iter);

        Unit* pTarget = pCurrentRef->getTarget();
        MANGOS_ASSERT(pTarget);                             // if the ref has status online the target must be there!

        bool isInMelee = attacker->CanReachWithMeleeAttack(pTarget);
        // Some bosses keep ranged targets in threat list but do not pick them with generic threat choice
        if (attacker->IsIgnoringRangedTargets() && !isInMelee)
        {
            ++iter;
            continue;
        }

        if (currentVictim) // select 1.3/1.1 better target in comparison current target
        {


            // normal case: pCurrentRef is still valid and most hated
            if (currentVictim == pCurrentRef)
            {
                found = true;
                break;
            }

            if (pCurrentRef->GetTauntState() > currentVictim->GetTauntState())
            {
                found = true;
                break;
            }

            if (pCurrentRef->GetHostileState() > currentVictim->GetHostileState())
            {
                found = true;
                break;
            }

            // list sorted and and we check current target, then this is best case
            if (pCurrentRef->getThreat() <= 1.1f * currentVictim->getThreat())
            {
                pCurrentRef = currentVictim;
                found = true;
                break;
            }

            if (pCurrentRef->getThreat() > 1.3f * currentVictim->getThreat() ||
                (pCurrentRef->getThreat() > 1.1f * currentVictim->getThreat() && isInMelee))
            {
                // implement 110% threat rule for targets in melee range
                found = true;                           // and 130% rule for targets in ranged distances
                break;                                  // for selecting alive targets
            }
        }
        else                                            // select any
        {
            found = true;
            break;
        }
        ++iter;
    }
    if (!found)
        pCurrentRef = nullptr;

    return pCurrentRef;
}

//============================================================
//=================== ThreatManager ==========================
//============================================================

ThreatManager::ThreatManager(Unit* owner)
    : iCurrentVictim(nullptr), iOwner(owner)
{
}

//============================================================

void ThreatManager::clearReferences()
{
    iThreatContainer.clearReferences();
    iThreatOfflineContainer.clearReferences();
    iCurrentVictim = nullptr;
}

//============================================================

void ThreatManager::addThreat(Unit* victim, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const* threatSpell)
{
    // function deals with adding threat and adding players and pets into ThreatList
    // mobs, NPCs, guards have ThreatList and HateOfflineList
    // players and pets have only InHateListOf
    // HateOfflineList is used co contain unattackable victims (in-flight, in-water, GM etc.)

    // not to self
    if (victim == getOwner())
        return;

    // not to GM
    if (!victim || (victim->GetTypeId() == TYPEID_PLAYER && static_cast<Player*>(victim)->isGameMaster()))
        return;

    // not to dead and not for dead
    if (!victim->isAlive() || !getOwner()->isAlive())
        return;

    float calculatedThreat = ThreatCalcHelper::CalcThreat(victim, iOwner, threat, crit, schoolMask, threatSpell);

    addThreatDirectly(victim, calculatedThreat);
}

void ThreatManager::addThreatDirectly(Unit* victim, float threat)
{
    HostileReference* ref = iThreatContainer.addThreat(victim, threat);
    // Ref is not in the online refs, search the offline refs next
    if (!ref)
        ref = iThreatOfflineContainer.addThreat(victim, threat);

    if (!ref)                                               // there was no ref => create a new one
    {
        // threat has to be 0 here
        HostileReference* hostileReference = new HostileReference(victim, this, 0);
        iThreatContainer.addReference(hostileReference);
        hostileReference->addThreat(threat);                // now we add the real threat
        if (victim->GetTypeId() == TYPEID_PLAYER && static_cast<Player*>(victim)->isGameMaster())
            hostileReference->setOnlineOfflineState(false); // GM is always offline
    }
}

//============================================================

void ThreatManager::modifyThreatPercent(Unit* victim, int32 threatPercent)
{
    iThreatContainer.modifyThreatPercent(victim, threatPercent);
}

//============================================================

void ThreatManager::UpdateContainers()
{
    iThreatContainer.update();
}

Unit* ThreatManager::getHostileTarget()
{
    UpdateContainers();
    HostileReference* nextVictim = iThreatContainer.selectNextVictim(getOwner(), getCurrentVictim());
    setCurrentVictim(nextVictim);
    return getCurrentVictim() != nullptr ? getCurrentVictim()->getTarget() : nullptr;
}

//============================================================

float ThreatManager::getThreat(Unit* victim, bool alsoSearchOfflineList)
{
    float threat = 0.0f;
    HostileReference* ref = iThreatContainer.getReferenceByTarget(victim);
    if (!ref && alsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(victim);
    if (ref)
        threat = ref->getThreat();
    else if (alsoSearchOfflineList)
        threat = iThreatOfflineContainer.getReferenceByTarget(victim)->getThreat();

    return threat;
}

bool ThreatManager::HasThreat(Unit * victim, bool alsoSearchOfflineList)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(victim);
    if (!ref && alsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(victim);
    return ref != nullptr;
}

//============================================================

void ThreatManager::TauntUpdate()
{
    const Unit::AuraList& tauntAuras = iOwner->GetAurasByType(SPELL_AURA_MOD_TAUNT);
    std::unordered_map<ObjectGuid, TauntState> tauntStates;
    uint32 state = STATE_TAUNTED;
    for (auto tauntAura : tauntAuras)
        tauntStates[tauntAura->GetCasterGuid()] = TauntState(state++);

    for (auto& ref : iThreatContainer.getThreatList())
    {
        if (ref->GetTauntState() == STATE_FIXATED)
            continue;
        auto iter = tauntStates.find(ref->getTarget()->GetObjectGuid());
        if (iter != tauntStates.end())
            ref->SetTauntState((*iter).second);
        else
            ref->SetTauntState(STATE_NONE);
    }
    setDirty(true);
}

void ThreatManager::FixateTarget(Unit* victim)
{
    HostileReference* fixateRef = iThreatContainer.getReferenceByTarget(victim);
    if (fixateRef)
        fixateRef->SetTauntState(STATE_FIXATED);

    for (auto& ref : iThreatContainer.getThreatList())
        if (ref != fixateRef && ref->GetTauntState() == STATE_FIXATED)
            ref->SetTauntState(STATE_NONE);

    TauntUpdate();
}

void ThreatManager::setCurrentVictim(HostileReference* hostileReference)
{
    iCurrentVictim = hostileReference;
}

void ThreatManager::setCurrentVictimByTarget(Unit* target)
{
    if (HostileReference* ref = iThreatContainer.getReferenceByTarget(target))
        setCurrentVictim(ref);
}

//============================================================
// The hated unit is gone, dead or deleted
// return true, if the event is consumed

void ThreatManager::processThreatEvent(ThreatRefStatusChangeEvent& threatRefStatusChangeEvent)
{
    threatRefStatusChangeEvent.setThreatManager(this);     // now we can set the threat manager

    HostileReference* hostileReference = threatRefStatusChangeEvent.getReference();

    switch (threatRefStatusChangeEvent.getType())
    {
        case UEV_THREAT_REF_THREAT_CHANGE:
            if ((getCurrentVictim() == hostileReference && threatRefStatusChangeEvent.getFValue() < 0.0f) ||
                    (getCurrentVictim() != hostileReference && threatRefStatusChangeEvent.getFValue() > 0.0f))
                setDirty(true);                             // the order in the threat list might have changed
            break;
        case UEV_THREAT_REF_ONLINE_STATUS:
            if (!hostileReference->isOnline())
            {
                if (hostileReference == getCurrentVictim())
                {
                    setCurrentVictim(nullptr);
                    setDirty(true);
                }
                iThreatContainer.remove(hostileReference);
                iThreatOfflineContainer.addReference(hostileReference);
            }
            else
            {
                if (getCurrentVictim() && hostileReference->getThreat() > (1.1f * getCurrentVictim()->getThreat()))
                    setDirty(true);
                iThreatContainer.addReference(hostileReference);
                iThreatOfflineContainer.remove(hostileReference);
            }
            break;
        case UEV_THREAT_REF_REMOVE_FROM_LIST:
            if (hostileReference == getCurrentVictim())
            {
                setCurrentVictim(nullptr);
                setDirty(true);
            }
            if (hostileReference->isOnline())
            {
                iThreatContainer.remove(hostileReference);
            }
            else
                iThreatOfflineContainer.remove(hostileReference);
            break;
        case UEV_THREAT_REF_SUPPRESSED_STATUS:
            // Clear suppressed on suppress change
            ClearSuppressed(hostileReference);
            setDirty(true);
            break;
    }
}

void ThreatManager::ClearSuppressed(HostileReference* except)
{
    for (HostileReference* const curRef : iThreatContainer.getThreatList())
        if (curRef->GetHostileState() == STATE_SUPPRESSED && curRef != except && !getOwner()->IsSuppressedTarget(curRef->getTarget()))
            curRef->SetHostileState(STATE_NORMAL);
}

void ThreatManager::SetTargetSuppressed(Unit* target)
{
    if (HostileReference* ref = iThreatContainer.getReferenceByTarget(target))
    {
        ref->SetHostileState(STATE_SUPPRESSED);
        ref->SetSuppressabilityToggle();
    }
}

void HostileReference::setFadeoutThreatReduction(float value)
{
    iFadeoutThreadReduction = value;
    addThreat(iFadeoutThreadReduction);
}

void HostileReference::resetFadeoutThreatReduction()
{
    addThreat(-iFadeoutThreadReduction);
    iFadeoutThreadReduction = 0.f;
}
