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
#include "AI/ScriptDevAI/base/TimerAI.h"

class Creature;

class CreatureAI : public UnitAI, public CombatActions
{
    public:
        explicit CreatureAI(Creature* creature);
        explicit CreatureAI(Creature* creature, uint32 combatActions);

        virtual void Reset();

        virtual void EnterCombat(Unit* enemy) override;
        virtual void AttackStart(Unit* who) override;
        virtual void DamageTaken(Unit* dealer, uint32& damage, DamageEffectType damageType, SpellEntry const* spellInfo) override;
        virtual void JustPreventedDeath(Unit* /*attacker*/) {}

        void DoFakeDeath(uint32 spellId = 0);
        void SetDeathPrevention(bool state);
        bool IsPreventingDeath() const override { return m_deathPrevention; }
        void ResetDeathPrevented() { m_deathPrevented = false; }

        bool DoRetreat() override;
        void DoCallForHelp(float radius) override;

        void RetreatingArrived() override;
        void RetreatingEnded() override;

        void HandleAssistanceCall(Unit* sender, Unit* invoker) override;

        void UpdateAI(const uint32 diff) override;

        void AddUnreachabilityCheck(); // use in constructor

        // Combat AI components
        virtual void ExecuteActions() override;
        virtual void ExecuteAction(uint32 action) {}

        // EAI compatibility layer
        virtual void UpdateEventTimers(const uint32 diff) {}

        // Caster AI components
        void AddMainSpell(uint32 spellId);
        void AddDistanceSpell(uint32 spellId) { m_distanceSpells.insert(spellId); }

        void SetRangedMode(bool state, float distance, RangeModeType type);
        void SetCurrentRangedMode(bool state);

        bool GetCurrentRangedMode() { return m_currentRangedMode; }

        virtual void JustStoppedMovementOfTarget(SpellEntry const* spell, Unit* victim) override;
        virtual void OnSpellInterrupt(SpellEntry const* spellInfo) override;
        virtual void OnSpellCooldownAdded(SpellEntry const* spellInfo) override;

        virtual void DistancingStarted() override;
        virtual void DistancingEnded() override;

        void DistanceYourself();

        bool IsRangedUnit() override { return m_currentRangedMode; }
        SpellSchoolMask GetMainAttackSchoolMask() const override { return m_currentRangedMode ? m_mainAttackMask : UnitAI::GetMainAttackSchoolMask(); }

        virtual CanCastResult DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags = 0) override;

        bool IsMainSpellPrevented(SpellEntry const* spellInfo) const;
    protected:
        Creature* m_creature;
        bool m_deathPrevention;
        bool m_deathPrevented;

        bool m_teleportUnreachable;

        // Caster AI components
        bool m_rangedMode;
        RangeModeType m_rangedModeSetting;
        float m_chaseDistance;
        bool m_currentRangedMode;
        std::unordered_set<uint32> m_mainSpells;
        std::unordered_set<uint32> m_distanceSpells;
        uint32 m_mainSpellId;
        uint32 m_mainSpellCost;
        SpellEntry const* m_mainSpellInfo;
        float m_mainSpellMinRange;
        SpellSchoolMask m_mainAttackMask;
        bool m_distancingCooldown;
};

#endif
