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

#ifndef MANGOSSERVER_TEMPSPAWN_H
#define MANGOSSERVER_TEMPSPAWN_H

#include "Entities/Creature.h"
#include "Globals/ObjectAccessor.h"

class TemporarySpawn : public Creature
{
    public:
        explicit TemporarySpawn(ObjectGuid summoner = ObjectGuid());
        virtual ~TemporarySpawn() {};

        void Update(const uint32 diff) override;
        void SetSummonProperties(TempSpawnType type, uint32 lifetime);
        void Summon(TempSpawnType type, uint32 lifetime);
        void UnSummon();
        void SaveToDB() override;
        ObjectGuid const GetSpawnerGuid() const override { return m_spawner ; }
        void SetLinkedToOwnerAura(uint32 flags) { m_linkedToOwnerAura |= flags; };

        void SetDeathState(DeathState state) override;
    private:
        bool IsExpired() const;

        bool CheckAuraOnOwner();
        void RemoveAuraFromOwner();
        TempSpawnType m_type;
        TimePoint m_expirationTimestamp;
        uint32 m_lifetime;
        ObjectGuid m_spawner;
        uint32 m_linkedToOwnerAura;
};

class TemporarySpawnWaypoint : public TemporarySpawn
{
    public:
        explicit TemporarySpawnWaypoint(ObjectGuid summoner, uint32 waypoint_id, int32 path_id, uint32 pathOrigin);

        uint32 GetWaypointId() const { return m_waypoint_id; }
        int32 GetPathId() const { return m_path_id; }
        uint32 GetPathOrigin() const { return m_pathOrigin; }

    private:
        uint32 m_waypoint_id;
        int32 m_path_id;
        uint32 m_pathOrigin;
};

#endif
