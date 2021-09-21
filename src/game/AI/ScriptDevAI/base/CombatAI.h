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
        CombatAI(Creature* creature, uint32 combatActions);

        void Reset() override;

        virtual void ExecuteActions() override;

        virtual void ExecuteAction(uint32 action) {}

        void HandleDelayedInstantAnimation(SpellEntry const* spellInfo) override;
        void HandleTargetRestoration();
        bool IsTargetingRestricted();
        void StopTargeting(bool state) { m_stopTargeting = state; }

        void AddOnKillText(int32 text);
        template<typename... Targs>
        void AddOnKillText(int32 value, Targs... fargs)
        {
            AddOnKillText(value);
            AddOnKillText(fargs...);
        }
        void KilledUnit(Unit* /*victim*/) override;

        void AddUnreachabilityCheck(); // use in constructor

        void UpdateAI(const uint32 diff) override;
    private:
        ObjectGuid m_storedTarget;

        std::vector<int32> m_onDeathTexts;
        bool m_onKillCooldown;

        bool m_stopTargeting;

        bool m_teleportUnreachable;
};

// Implementation is identical to EAI
class RangedCombatAI : public CombatAI
{
    public:
        RangedCombatAI(Creature* creature, uint32 combatActions);

        virtual void OnSpellCooldownAdded(SpellEntry const* spellInfo) override;

        void AddMainSpell(uint32 spellId);
        void AddDistanceSpell(uint32 spellId) { m_distanceSpells.insert(spellId); }

        void SetRangedMode(bool state, float distance, RangeModeType type);
        void SetCurrentRangedMode(bool state);

        bool GetCurrentRangedMode() { return m_currentRangedMode; }

        virtual void JustStoppedMovementOfTarget(SpellEntry const* spell, Unit* victim) override;
        virtual void OnSpellInterrupt(SpellEntry const* spellInfo) override;

        virtual void DistancingStarted() override;
        virtual void DistancingEnded() override;

        void DistanceYourself();

        bool IsRangedUnit() override { return m_currentRangedMode; }
        SpellSchoolMask GetMainAttackSchoolMask() const override { return m_currentRangedMode ? m_mainAttackMask : CombatAI::GetMainAttackSchoolMask(); }

        virtual CanCastResult DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags = 0) override;

        void UpdateAI(const uint32 diff) override;

        bool IsMainSpellPrevented(SpellEntry const* spellInfo) const;
    private:
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
