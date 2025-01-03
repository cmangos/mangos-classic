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

#ifndef MANGOS_CREATUREAI_H
#define MANGOS_CREATUREAI_H

#include "AI/BaseAI/UnitAI.h"

class Creature;

class CreatureAI : public UnitAI
{
    public:
        explicit CreatureAI(Creature* creature);
        explicit CreatureAI(Creature* creature, uint32 combatActions);

        virtual void Reset();

        virtual void OnRemoveFromWorld() {}

        virtual void EnterCombat(Unit* enemy) override;
        virtual void EnterEvadeMode() override;
        virtual void AttackStart(Unit* who) override;
        virtual void DamageTaken(Unit* dealer, uint32& damage, DamageEffectType damageType, SpellEntry const* spellInfo) override;
        virtual void JustPreventedDeath(Unit* /*attacker*/) {}
        virtual void JustReachedHome() override;
        virtual void JustRespawned() override;

        void DoFakeDeath(uint32 spellId = 0);
        void SetDeathPrevention(bool state);
        void ResetDeathPrevented() { m_deathPrevented = false; }

        bool DoRetreat() override;
        void DoCallForHelp(float radius) override;

        void RetreatingArrived() override;
        void RetreatingEnded() override;

        void HandleAssistanceCall(Unit* sender, Unit* invoker) override;

        void AddUnreachabilityCheck(); // use in constructor

        CreatureSpellList const& GetSpellList() const override;

        void TimedFleeingEnded() override;

        void OnCallForHelp(Unit* enemy) override;

        // Starts follow and in case of player engages player based formation
        virtual void RequestFollow(Unit* followee) override;
        virtual void RelinquishFollow(ObjectGuid follower) override;
    protected:
        void HandleSpawnGuard();

        Creature* m_creature;
        bool m_deathPrevented;

        float m_followAngle;
        float m_followDist;
        ObjectGuid m_requestedFollower;
};

#endif
