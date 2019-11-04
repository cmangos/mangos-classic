/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#ifndef COMBAT_AI_H
#define COMBAT_AI_H

#include "Entities/Creature.h"
#include "AI/ScriptDevAI/include/sc_creature.h"
#include "AI/ScriptDevAI/base/TimerAI.h"

/*
    Small wrapper class that reduces the amount of code needed to use CombatActions and adds proper spell action reset functionality
*/
class CombatAI : public ScriptedAI, public CombatActions
{
    public:
        CombatAI(Creature* creature, uint32 combatActions) : ScriptedAI(creature), CombatActions(combatActions) { }

        void Reset() override
        {
            ResetAllTimers();
        }

        virtual void ExecuteActions() override;

        virtual void ExecuteAction(uint32 action) = 0;

        void UpdateAI(const uint32 diff) override;
};

// Implementation is identical to EAI
class RangedCombatAI : public CombatAI
{
    public:
        RangedCombatAI(Creature* creature, uint32 combatActions, RangeModeType type) : CombatAI(creature, combatActions),
            m_rangedMode(false), m_rangedModeSetting(TYPE_NONE), m_chaseDistance(0.f), m_currentRangedMode(false), m_mainSpellId(0), m_mainSpellCost(0), m_mainSpellMinRange(0.f),
            m_mainAttackMask(SPELL_SCHOOL_MASK_NONE) {}

        virtual void OnSpellCooldownAdded(SpellEntry const* spellInfo) override;

        void AddMainSpell(uint32 spellId);
        void AddDistanceSpell(uint32 spellId) { m_distanceSpells.insert(spellId); }

        void SetRangedMode(bool state, float distance, RangeModeType type);
        void SetCurrentRangedMode(bool state);

        virtual void JustStoppedMovementOfTarget(SpellEntry const* spell, Unit* victim) override;
        virtual void OnSpellInterrupt(SpellEntry const* spellInfo) override;

        virtual void DistancingStarted() override;
        virtual void DistancingEnded() override;

        void DistanceYourself();

        bool IsRangedUnit() override { return m_currentRangedMode; }
        SpellSchoolMask GetMainAttackSchoolMask() const override { return m_currentRangedMode ? m_mainAttackMask : CombatAI::GetMainAttackSchoolMask(); }

        virtual CanCastResult DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags = 0) override;

        void UpdateAI(const uint32 diff) override;
    private:
        bool m_rangedMode;
        RangeModeType m_rangedModeSetting;
        float m_chaseDistance;
        bool m_currentRangedMode;
        std::unordered_set<uint32> m_mainSpells;
        std::unordered_set<uint32> m_distanceSpells;
        uint32 m_mainSpellId;
        uint32 m_mainSpellCost;
        float m_mainSpellMinRange;
        SpellSchoolMask m_mainAttackMask;
};

#endif
