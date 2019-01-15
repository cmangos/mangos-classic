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

#ifndef _HOSTILEREFMANAGER
#define _HOSTILEREFMANAGER

#include "Common.h"
#include "Utilities/LinkedReference/RefManager.h"

class Unit;
class ThreatManager;
class HostileReference;
struct SpellEntry;

//=================================================

class HostileRefManager : public RefManager<Unit, ThreatManager>
{
    public:
        explicit HostileRefManager(Unit* owner);
        ~HostileRefManager();

        Unit* getOwner() const { return iOwner; }

        // send threat to all my hateres for the pVictim
        // The pVictim is hated than by them as well
        // use for buffs and healing threat functionality
        void threatAssist(Unit* victim, float threat, SpellEntry const* threatSpell = nullptr, bool singleTarget = false);
        void threatTemporaryFade(Unit* victim, float threat, bool apply);

        void addThreatPercent(int32 threatPercent);

        // The references are not needed anymore
        // tell the source to remove them from the list and free the mem
        void deleteReferences();

        // Remove specific faction references
        void deleteReferencesForFaction(uint32 faction);

        HostileReference* getFirst();

        void updateThreatTables();

        void setOnlineOfflineState(bool isOnline);
        void updateOnlineOfflineState(bool pIsOnline);

        // set state for one reference, defined by Unit
        void setOnlineOfflineState(Unit* victim, bool isOnline);

        // delete one reference, defined by Unit
        void deleteReference(Unit* victim);

        // Suppression
        void HandleSuppressed(bool apply, bool immunity = false);

    private:
        Unit* iOwner;                                       // owner of manager variable, back ref. to it, always exist
};
//=================================================
#endif
