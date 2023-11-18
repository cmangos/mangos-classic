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

#ifndef _THREATMANAGER
#define _THREATMANAGER

#include "Common.h"
#include "Globals/SharedDefines.h"
#include "Utilities/LinkedReference/Reference.h"
#include "Entities/UnitEvents.h"
#include "Entities/ObjectGuid.h"
#include <list>

//==============================================================

class Unit;
class ThreatManager;
struct SpellEntry;

//==============================================================
// Class to calculate the real threat based

class ThreatCalcHelper
{
    public:
        static float CalcThreat(Unit* hatedUnit, Unit* hatingUnit, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const* threatSpell);
};

enum HostileState : uint32
{
    STATE_SUPPRESSED,
    STATE_NORMAL,
};

enum TauntState : uint32
{
    STATE_NONE,
    STATE_TAUNTED,
    STATE_FIXATED = UINT32_MAX,
};

//==============================================================
class HostileReference : public Reference<Unit, ThreatManager>
{
    public:
        HostileReference(Unit* unit, ThreatManager* threatManager, float threat);

        //=================================================
        void addThreat(float mod);

        void setThreat(float threat) { addThreat(threat - getThreat()); }

        void addThreatPercent(int32 threatPercent)
        {
            // for special -100 case avoid rounding
            addThreat(threatPercent == -100 ? -iThreat : iThreat * threatPercent / 100.0f);
        }

        float getThreat() const { return iThreat; }
        float getFadeoutThreatReduction() const { return iFadeoutThreadReduction; }
        void setFadeoutThreatReduction(float value);
        void resetFadeoutThreatReduction();

        bool isOnline() const { return m_online; }

        // The Unit might be in water and the creature can not enter the water, but has range attack
        // in this case online = true, but accessable = false
        bool isAccessable() const { return iAccessible; }

        //=================================================
        // check, if source can reach target and set the status
        void updateOnlineStatus();

        void setOnlineOfflineState(bool isOnline);

        void setAccessibleState(bool isAccessible);
        //=================================================

        bool operator ==(const HostileReference& hostileReference) const { return hostileReference.getUnitGuid() == getUnitGuid(); }

        //=================================================

        ObjectGuid const& getUnitGuid() const { return iUnitGuid; }

        //=================================================
        // reference is not needed anymore. realy delete it !

        void removeReference();

        //=================================================

        HostileReference* next() { return static_cast<HostileReference*>(Reference<Unit, ThreatManager>::next()); }

        //=================================================

        // Tell our refTo (target) object that we have a link
        void targetObjectBuildLink() override;

        // Tell our refTo (taget) object, that the link is cut
        void targetObjectDestroyLink() override;

        // Tell our refFrom (source) object, that the link is cut (Target destroyed)
        void sourceObjectDestroyLink() override;

        // Priority alterations
        void SetSuppressabilityToggle() { m_suppresabilityToggle = true; }
        void SetHostileState(HostileState state);
        HostileState GetHostileState() const { return m_hostileState; }

        void SetTauntState(TauntState state) { m_tauntState = state; }
        TauntState GetTauntState() const { return m_tauntState; }
    protected:
        // Inform the source, that the status of that reference was changed
        void fireStatusChanged(ThreatRefStatusChangeEvent& threatRefStatusChangeEvent);

        Unit* getSourceUnit() const;
    private:
        float iThreat;
        HostileState m_hostileState;
        bool m_suppresabilityToggle;
        TauntState m_tauntState;
        float iFadeoutThreadReduction;                      // used for fade
        ObjectGuid iUnitGuid;
        bool m_online;
        bool iAccessible;
};

//==============================================================
class ThreatManager;

typedef std::list<HostileReference*> ThreatList;

class ThreatContainer
{
    public:
        ThreatContainer() { iDirty = false; }
        ~ThreatContainer() { clearReferences(); }

        HostileReference* addThreat(Unit* victim, float threat);

        void modifyThreatPercent(Unit* victim, int32 threatPercent);
        void modifyAllThreatPercent(int32 threatPercent);

        HostileReference* selectNextVictim(Unit* attacker, HostileReference* currentVictim);

        void setDirty(bool dirty) { iDirty = dirty; }

        bool isDirty() const { return iDirty; }

        bool empty() const { return iThreatList.empty(); }

        HostileReference* getMostHated() { return iThreatList.empty() ? nullptr : iThreatList.front(); }

        HostileReference* getReferenceByTarget(Unit* victim);

        ThreatList const& getThreatList() const { return iThreatList; }
    protected:
        friend class ThreatManager;

        void remove(HostileReference* ref) { iThreatList.remove(ref); }
        void addReference(HostileReference* hostileReference) { iThreatList.push_back(hostileReference); }
        void clearReferences();
        // Sort the list if necessary
        void update(bool force, bool isPlayer);

        ThreatList iThreatList;
    private:
        bool iDirty;
};

//=================================================

class ThreatManager
{
    public:
        friend class HostileReference;

        explicit ThreatManager(Unit* owner);

        ~ThreatManager() { clearReferences(); }

        void clearReferences();

        void addThreat(Unit* victim, float threat, bool crit, SpellSchoolMask schoolMask, SpellEntry const* threatSpell);
        void addThreat(Unit* victim, float threat) { addThreat(victim, threat, false, SPELL_SCHOOL_MASK_NONE, nullptr); }

        // add threat as raw value (ignore redirections and expection all mods applied already to it
        void addThreatDirectly(Unit* victim, float threat, bool noNew);

        void modifyThreatPercent(Unit* victim, int32 threatPercent); // -101 removes whole ref, -100 sets threat to 0, rest modifies it
        void modifyAllThreatPercent(int32 threatPercent);

        float getThreat(Unit* victim, bool alsoSearchOfflineList = false);
        float GetHighestThreat(); // for purpose of taunt effect

        bool HasThreat(Unit* victim, bool alsoSearchOfflineList = false);

        bool isThreatListEmpty() const { return iThreatContainer.empty(); }

        void processThreatEvent(ThreatRefStatusChangeEvent& threatRefStatusChangeEvent);

        HostileReference* getCurrentVictim() const { return iCurrentVictim; }

        Unit* getOwner() const { return iOwner; }

        Unit* getHostileTarget();

        void TauntUpdate();
        void FixateTarget(Unit* victim);

        void setCurrentVictim(HostileReference* hostileReference);
        void setCurrentVictimByTarget(Unit* target); // Used in SPELL_EFFECT_ATTACK_ME to set the current target to the taunter

        void setDirty(bool dirty) { iThreatContainer.setDirty(dirty); }

        // Don't must be used for explicit modify threat values in iterator return pointers
        ThreatList const& getThreatList() const { return iThreatContainer.getThreatList(); }

        void DeleteOutOfRangeReferences();

        // When a target is unreachable, we need to set someone as low priority
        void SetTargetSuppressed(Unit* target);
        void ClearSuppressed(HostileReference* except);
    private:
        void UpdateContainers();

        HostileReference* iCurrentVictim;
        Unit* iOwner;
        ThreatContainer iThreatContainer;
        ThreatContainer iThreatOfflineContainer;
};

//=================================================
#endif
