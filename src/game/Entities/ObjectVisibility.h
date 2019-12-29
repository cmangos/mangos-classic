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

#ifndef __OBJECT_VISIBILITY_H
#define __OBJECT_VISIBILITY_H

#include "Common.h"

class WorldObject;
class Unit;

enum class VisibilityDistanceType : uint32
{
    Normal = 0,
    Tiny = 1,
    Small = 2,
    Large = 3,
    Gigantic = 4,
    Infinite = 5,

    Max
};

enum StealthType : uint32
{
    STEALTH_UNIT = 0,
    STEALTH_TRAP = 1,
    STEALTH_TYPE_MAX,
};

enum InvisibilityType : uint32
{
    INVISIBILITY_TRAP = 3,
    INVISIBILITY_DRUNK = 6,
    INVISIBILITY_TYPE_MAX = 32,
};

class VisibilityData
{
    public:
        VisibilityData(WorldObject* owner);

        // visibility
        bool IsVisibilityOverridden() const { return m_visibilityDistanceOverride != 0.f; }
        void SetVisibilityDistanceOverride(VisibilityDistanceType type);

        float GetVisibilityDistance() const;
        float GetVisibilityDistanceFor(WorldObject* obj) const;
        // invisibility
        bool CanDetectInvisibilityOf(WorldObject const* u) const;
        uint32 GetInvisibilityDetectMask() const;
        void SetInvisibilityDetectMask(uint32 index, bool apply);
        uint32 GetInvisibilityMask() const;
        void SetInvisibilityMask(uint32 index, bool apply);
        void SetInvisibilityValue(uint32 index, int32 value) { m_invisibilityValues[index] = value; }
        void AddInvisibilityValue(uint32 index, int32 value) { m_invisibilityValues[index] += value; }
        void AddInvisibilityDetectValue(uint32 index, int32 value) { m_invisibilityDetectValues[index] += value; }
        int32 GetInvisibilityValue(uint32 index) const;
        int32 GetInvisibilityDetectValue(uint32 index) const;
        // stealth
        uint32 GetStealthMask() const { return m_stealthMask; }
        void SetStealthMask(uint32 index, bool apply);
        void AddStealthStrength(StealthType type, uint32 value) { m_stealthStrength[type] += value; }
        void AddStealthDetectionStrength(StealthType type, uint32 value) { m_stealthDetectStrength[type] += value; }
        uint32 GetStealthStrength(StealthType type) const { return m_stealthStrength[type]; }
        uint32 GetStealthDetectionStrength(StealthType type) const { return m_stealthDetectStrength[type]; }

        float GetStealthVisibilityDistance(Unit const* target, bool alert = false) const;
    private:
        // visibility
        float m_visibilityDistanceOverride;
        // invisibility
        uint32 m_invisibilityMask;
        uint32 m_detectInvisibilityMask; // is inherited from controller in PC case
        int32 m_invisibilityValues[INVISIBILITY_TYPE_MAX];
        int32 m_invisibilityDetectValues[INVISIBILITY_TYPE_MAX];
        // stealth
        uint32 m_stealthMask;
        uint32 m_stealthStrength[STEALTH_TYPE_MAX];
        uint32 m_stealthDetectStrength[STEALTH_TYPE_MAX];

        WorldObject* m_owner;
};

#endif