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

#ifndef MANGOSSERVER_TOTEM_H
#define MANGOSSERVER_TOTEM_H

#include "Entities/Creature.h"

enum TotemType
{
    TOTEM_PASSIVE    = 0,
    TOTEM_ACTIVE     = 1,
    TOTEM_STATUE     = 2
};

class Totem : public Creature
{
    public:
        explicit Totem();
        virtual ~Totem() {}
        bool Create(uint32 guidlow, CreatureCreatePos& cPos, CreatureInfo const* cinfo, Unit* owner);
        void Update(const uint32 diff) override;
        void Summon(Unit* owner);
        void UnSummon();
        uint32 GetSpell() const;
        uint32 GetTotemDuration() const { return m_duration; }
        TotemType GetTotemType() const { return m_type; }
        void SetTypeBySummonSpell(SpellEntry const* spellProto);
        void SetDuration(uint32 dur) { m_duration = dur; }

        Player* GetSpellModOwner() const override;

        float GetCritChance(WeaponAttackType attackType) const override;
        float GetCritChance(SpellSchoolMask schoolMask) const override;
        float GetCritMultiplier(SpellSchoolMask dmgSchoolMask, uint32 creatureTypeMask, const SpellEntry* spell = nullptr, bool heal = false) const override;
        float GetHitChance(WeaponAttackType attackType) const override;
        float GetHitChance(SpellSchoolMask schoolMask) const override;
        float GetMissChance(WeaponAttackType attackType) const override;
        float GetMissChance(SpellSchoolMask schoolMask) const override;
        int32 GetResistancePenetration(SpellSchools school) const override;

        bool UpdateStats(Stats /*stat*/) override { return true; }
        bool UpdateAllStats() override { return true; }
        void UpdateResistances(uint32 /*school*/) override {}
        void UpdateArmor() override {}
        void UpdateMaxHealth() override {}
        void UpdateMaxPower(Powers /*power*/) override {}
        void UpdateAttackPowerAndDamage(bool /*ranged*/) override {}
        void UpdateDamagePhysical(WeaponAttackType /*attType*/) override {}

        bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const override;

        ObjectGuid const GetSpawnerGuid() const override { return GetOwnerGuid(); }

    protected:
        TotemType m_type;
        uint32 m_duration;
};
#endif
