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

#ifndef MANGOSSERVER_DYNAMICOBJECT_H
#define MANGOSSERVER_DYNAMICOBJECT_H

#include "Entities/Object.h"
#include "Server/DBCEnums.h"
#include "Spells/SpellTargetDefines.h"
#include "Entities/Unit.h"

enum DynamicObjectType
{
    DYNAMIC_OBJECT_PORTAL           = 0x0,      // unused
    DYNAMIC_OBJECT_AREA_SPELL       = 0x1,
    DYNAMIC_OBJECT_FARSIGHT_FOCUS   = 0x2,
};

struct SpellEntry;

class DynamicObject : public WorldObject
{
    public:
        explicit DynamicObject();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(uint32 guidlow, Unit* caster, uint32 spellId, SpellEffectIndex effIndex, float x, float y, float z, int32 duration, float radius, DynamicObjectType type, SpellTarget target, int32 damage, int32 basePoints);
        void Update(const uint32 diff) override;
        void Delete();
        uint32 GetSpellId() const { return m_spellId; }
        SpellEffectIndex GetEffIndex() const { return m_effIndex; }
        uint32 GetDuration() const { return m_aliveDuration; }
        ObjectGuid const& GetOwnerGuid() const override { return GetCasterGuid(); }
        void SetOwnerGuid(ObjectGuid guid) override { SetCasterGuid(guid); }
        ObjectGuid const& GetCasterGuid() const { return GetGuidValue(DYNAMICOBJECT_CASTER); }
        void SetCasterGuid(ObjectGuid guid) { SetGuidValue(DYNAMICOBJECT_CASTER, guid); }
        Unit* GetCaster() const;
        float GetRadius() const { return m_radius; }
        int32 const& GetDamage() const { return m_damage; }
        int32 const& GetBasePoints() const { return m_basePoints; }
        DynamicObjectType GetType() const { return (DynamicObjectType)GetByteValue(DYNAMICOBJECT_BYTES, 0); }
        bool IsAffecting(Unit* unit) const { return m_affected.find(unit->GetObjectGuid()) != m_affected.end(); }
        void AddAffected(Unit* unit) { m_affected.insert(unit->GetObjectGuid()); }
        void RemoveAffected(Unit* unit) { m_affected.erase(unit->GetObjectGuid()); }
        void Delay(int32 delaytime);

        ReputationRank GetReactionTo(Unit const* unit) const override;

        bool IsEnemy(Unit const* unit) const override;
        bool IsFriend(Unit const* unit) const override;

        bool CanAttackSpell(Unit const* target, SpellEntry const* spellInfo = nullptr, bool isAOE = false) const override;
        bool CanAssistSpell(Unit const* target, SpellEntry const* spellInfo = nullptr) const override;

        void OnPersistentAreaAuraEnd();

        float GetObjectBoundingRadius() const override      // overwrite WorldObject version
        {
            return 0.0f;                                    // dynamic object not have real interact size
        }

        bool isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const override;
        SpellTarget GetTarget() const { return m_target; }

        GridReference<DynamicObject>& GetGridRef() { return m_gridRef; }

    protected:
        uint32 m_spellId;
        SpellEffectIndex m_effIndex;
        int32 m_aliveDuration;
        float m_radius;                                     // radius apply persistent effect, 0 = no persistent effect
        bool m_positive;
        GuidSet m_affected;
        SpellTarget m_target;
        int32 m_damage;
        int32 m_basePoints;
    private:
        GridReference<DynamicObject> m_gridRef;
};
#endif
