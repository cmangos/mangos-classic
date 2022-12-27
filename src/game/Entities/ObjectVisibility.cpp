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

#include "ObjectVisibility.h"

#include "Util/Util.h"
#include "Entities/Player.h"
#include "Entities/GameObject.h"

constexpr float VisibilityDistances[AsUnderlyingType(VisibilityDistanceType::Max)] =
{
    DEFAULT_VISIBILITY_DISTANCE,
    VISIBILITY_DISTANCE_TINY,
    VISIBILITY_DISTANCE_SMALL,
    VISIBILITY_DISTANCE_LARGE,
    VISIBILITY_DISTANCE_GIGANTIC,
    MAX_VISIBILITY_DISTANCE
};

VisibilityData::VisibilityData(WorldObject* owner) : m_visibilityDistanceOverride(0.f), m_invisibilityMask(0), m_detectInvisibilityMask(0), m_owner(owner)
{
    memset(m_invisibilityValues, 0, sizeof(m_invisibilityValues));
    memset(m_invisibilityDetectValues, 0, sizeof(m_invisibilityDetectValues));

    memset(m_stealthStrength, 0, sizeof(m_stealthStrength));
    memset(m_stealthDetectStrength, 0, sizeof(m_stealthDetectStrength));
}

void VisibilityData::SetVisibilityDistanceOverride(VisibilityDistanceType type)
{
    MANGOS_ASSERT(type < VisibilityDistanceType::Max);
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    m_visibilityDistanceOverride = VisibilityDistances[AsUnderlyingType(type)];
}

float VisibilityData::GetVisibilityDistance() const
{
    if (IsVisibilityOverridden())
        return m_visibilityDistanceOverride;
    else
        return m_owner->GetMap()->GetVisibilityDistance();
}

float VisibilityData::GetVisibilityDistanceFor(WorldObject* obj) const
{
    if (IsVisibilityOverridden() && obj->GetTypeId() == TYPEID_PLAYER)
        return m_visibilityDistanceOverride;
    else
        return obj->GetVisibilityData().GetVisibilityDistance();
}

bool VisibilityData::CanDetectInvisibilityOf(WorldObject const* target) const
{
    if (uint32 mask = (GetInvisibilityDetectMask() & target->GetVisibilityData().GetInvisibilityMask()))
    {
        for (int32 i = 0; i < INVISIBILITY_TYPE_MAX; ++i)
        {
            if (((1 << i) & mask) == 0)
                continue;

            // Find target's invisibility level to compare it with caster invisibility detection level
            int32 invLevel = target->GetVisibilityData().GetInvisibilityValue(i);

            // Find invisibility detection level - this is taken from controlling player or self
            int32 detectLevel = GetInvisibilityDetectValue(i);

            if (invLevel <= detectLevel)
                return true;
        }
    }

    return false;
}

uint32 VisibilityData::GetInvisibilityDetectMask() const
{
    if (m_owner->isType(TYPEMASK_UNIT))
    {
        Unit* owner = static_cast<Unit*>(m_owner);
        if (owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            Player const* controller = owner->GetControllingPlayer();
            if (controller)
                return controller->GetVisibilityData().m_detectInvisibilityMask; // directly access value without bypass
        }
    }
    return m_detectInvisibilityMask;
}

void VisibilityData::SetInvisibilityDetectMask(uint32 index, bool apply)
{
    if (apply)
        m_detectInvisibilityMask |= (1 << index);
    else
        m_detectInvisibilityMask &= ~(1 << index);
}

uint32 VisibilityData::GetInvisibilityMask() const
{
    return m_invisibilityMask;
}

void VisibilityData::SetInvisibilityMask(uint32 index, bool apply)
{
    if (apply)
        m_invisibilityMask |= (1 << index);
    else
        m_invisibilityMask &= ~(1 << index);
}

int32 VisibilityData::GetInvisibilityValue(uint32 index) const
{
    return m_invisibilityValues[index];
}

int32 VisibilityData::GetInvisibilityDetectValue(uint32 index) const
{
    if (m_owner->isType(TYPEMASK_UNIT))
    {
        Unit* owner = static_cast<Unit*>(m_owner);
        if (owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            Player const* controller = owner->GetControllingPlayer();
            if (controller)
                return controller->GetVisibilityData().m_invisibilityDetectValues[index]; // directly access value without bypass
        }
    }
    return m_invisibilityDetectValues[index];
}

// stealth
void VisibilityData::SetStealthMask(uint32 index, bool apply)
{
    if (apply)
        m_stealthMask |= (1 << index);
    else
        m_stealthMask &= ~(1 << index);
}

float VisibilityData::GetStealthVisibilityDistance(Unit const* target, bool alert) const
{
    // Starting points
    int32 detectionValue = 30;

    // Level difference: 5 point / level, starting from level 1.
    // There may be spells for this and the starting points too, but
    // not in the DBCs of the client.
    detectionValue += int32(target->GetLevelForTarget(dynamic_cast<Unit*>(m_owner)) - 1) * 5;

    // Apply modifiers
    detectionValue += target->GetVisibilityData().GetStealthDetectionStrength(STEALTH_UNIT);
    if (m_owner->GetTypeId() == TYPEID_GAMEOBJECT) // trap case
        if (Unit* owner = static_cast<GameObject*>(m_owner)->GetOwner())
            detectionValue -= int32(owner->GetLevelForTarget(dynamic_cast<Unit*>(m_owner)) - 1) * 5;

    detectionValue -= GetStealthStrength(STEALTH_UNIT);

    // Calculate max distance
    float visibilityRange = float(detectionValue) * 0.3f;

    // If this unit is an NPC then player detect range doesn't apply
    if (target->GetTypeId() == TYPEID_PLAYER && visibilityRange > MAX_PLAYER_STEALTH_DETECT_RANGE)
        visibilityRange = MAX_PLAYER_STEALTH_DETECT_RANGE;

    // When checking for alert state, look 8% further, and then 1.5 yards more than that.
    if (alert)
        visibilityRange += (visibilityRange * 0.08f) + 1.5f;

    return std::max(target->GetCombatReach(), visibilityRange);
}