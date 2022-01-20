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

#ifndef MANGOS_SPAWN_GROUP_H
#define MANGOS_SPAWN_GROUP_H

#include "Platform/Define.h"
#include "Entities/ObjectGuid.h"
#include "SpawnGroupDefines.h"
#include "MotionGenerators/MotionMaster.h"
#include <map>
#include <vector>
#include <memory>

class WorldObject;
class Creature;
class GameObject;
class Player;
class Map;
class Unit;
class CreatureGroup;
struct SpawnGroupEntry;

class SpawnGroup
{
    public:
        SpawnGroup(SpawnGroupEntry const& entry, Map& map, uint32 typeId);
        virtual void AddObject(uint32 dbGuid, uint32 entry);
        virtual void RemoveObject(WorldObject* wo);
        uint32 GetGuidEntry(uint32 dbGuid) const;
        virtual void Update();
        uint32 GetEligibleEntry(std::map<uint32, uint32>& existingEntries, std::map<uint32, uint32>& minEntries);
        virtual void Spawn(bool force);
        std::string to_string() const;
        uint32 GetObjectTypeId() const { return m_objectTypeId; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        SpawnGroupEntry const& GetGroupEntry() const { return m_entry; }
        uint32 GetGroupId() const { return m_entry.Id; }

        virtual CreatureGroup* GetCreatureGroup() { return nullptr; }

    protected:
        SpawnGroupEntry const& m_entry;
        Map& m_map;
        std::map<uint32, uint32> m_objects;
        std::map<uint32, uint32> m_chosenEntries; // dungeon saving for entry per dynguid
        uint32 m_objectTypeId;
        bool m_enabled;
};

class CreatureGroup : public SpawnGroup
{
    public:
        CreatureGroup(SpawnGroupEntry const& entry, Map& map);
        void RemoveObject(WorldObject* wo) override;
        void TriggerLinkingEvent(uint32 event, Unit* target);

        void SetFormationData(FormationEntrySPtr fEntry);
        FormationData* GetFormationData() { return m_formationData.get(); }
        FormationEntrySPtr GetFormationEntry() const { return m_entry.formationEntry; }

        virtual void Update() override;

        virtual CreatureGroup* GetCreatureGroup() override { return this; }

        Map& GetMap() { return m_map; }

        void MoveHome();

    private:
        void ClearRespawnTimes();
        FormationDataSPtr m_formationData;
};

struct RespawnPosistion
{
    float x, y, z, radius;
};

class GameObjectGroup : public SpawnGroup
{
    public:
        GameObjectGroup(SpawnGroupEntry const& entry, Map& map);
        void RemoveObject(WorldObject* wo) override;
};

class FormationSlotData
{
    public:
        FormationSlotData(uint32 slotId, uint32 _ownerDBGuid, CreatureGroup* creatureGrp, SpawnGroupFormationSlotType type = SpawnGroupFormationSlotType::SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC)
            : m_slotId(slotId), m_realOwnerGuid(_ownerDBGuid), m_creatureGroup(creatureGrp), m_slotType(type), m_owner(nullptr),
            m_realAngle(0), m_realDistance(1), m_recomputePosition(true), m_angleVariation(0), m_distanceVariation(0),
            m_maxAngleVariation(0), m_maxDistanceVariation(0), m_angleVariationDest(0), m_distanceVariationDest(0),
            m_canFollow(true)
        {}

        uint32 GetSlotId() const { return m_slotId; }
        Unit* GetMaster();
        bool IsFormationMaster();

        float GetAngle();
        float GetDistance() const;
        float GetRealAngle() const { return m_realAngle; }
        float GetRealDistance() const { return m_realDistance; }

        void SetAngle(float angle) { m_realAngle = angle; m_angleVariation = angle; }
        void SetDistance(float distance) { m_realDistance = distance; m_distanceVariation = distance; }
        void ResetVariation() { m_angleVariation = m_realAngle; m_distanceVariation = m_realDistance; }
        void SetMaxVariation(float angleMax, float distMax) { m_maxAngleVariation = angleMax; m_maxDistanceVariation = distMax; }
        void AddPositionVariation(bool now = false);
        void Update();
        bool& GetRecomputePosition() { return m_recomputePosition; }
        bool CanFollow() const { return m_canFollow; }
        void SetCanFollow(bool canFollow) { m_canFollow = canFollow; }

        CreatureGroup* GetCreatureGroup() { return m_creatureGroup; }
        FormationData* GetFormationData() { return m_creatureGroup->GetFormationData(); }
        void SetOwner(Unit* owner) { m_owner = owner; }
        Unit* GetOwner() { return m_owner; }
        uint32 GetRealOwnerGuid() const { return m_realOwnerGuid; }
        SpawnGroupFormationSlotType GetSlotType() const { return m_slotType; }
        bool IsRemovable() const
        {
            switch (m_slotType)
            {
                case SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC:
                    return false;
                case SPAWN_GROUP_FORMATION_SLOT_TYPE_SCRIPT:
                case SPAWN_GROUP_FORMATION_SLOT_TYPE_PLAYER:
                    return true;
                default:
                    break;
            }
            return false;
        }

    private:
        uint32 m_slotId;
        uint32 m_realOwnerGuid;
        CreatureGroup* m_creatureGroup;
        Unit* m_owner;
        float m_realAngle;
        float m_realDistance;
        float m_angleVariation;
        float m_distanceVariation;
        float m_maxAngleVariation;
        float m_maxDistanceVariation;
        float m_angleVariationDest;
        float m_distanceVariationDest;
        bool m_recomputePosition;
        bool m_canFollow;

        SpawnGroupFormationSlotType m_slotType;
};

class FormationData
{
    public:
        FormationData(CreatureGroup* gData, FormationEntrySPtr fEntry);
        FormationData() = delete;
        ~FormationData();

        Unit* GetMaster();
        void Add(Creature* creature);
        void Add(Player* player);
        void Remove(Creature* creature);
        void Remove(Player* player);
        void Remove(Unit* unit);
        void Reset();
        void Disband();
        void OnDeath(Creature* creature);
        void OnDelete(Creature* creature);
        void OnWPStartNode() { m_mirrorState = false; };
        void OnWPEndNode() { m_mirrorState = true; };
        bool GetMirrorState() const { return m_mirrorState; }
        void Compact(bool set = true);
        bool SwitchFormation(SpawnGroupFormationType newShape);
        SpawnGroupFormationType GetCurrentShape() const { return m_currentFormationShape; }
        void SetSpread(float spread) { m_currentSpread = spread; FixSlotsPositions(); }
        void SetOptions(uint32 options);
        bool CanUseMMap() const { return !HaveOption(SPAWN_GROUP_FORMATION_OPTION_FOLLOWERS_WILL_NOT_PATHFIND_TO_LOCATION); }
        void Update();

        FormationSlotDataSPtr SetFormationSlot(Creature* creature, SpawnGroupFormationSlotType slotType = SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC);
        std::string to_string() const;

    private:
        void SetMasterMovement();
        bool TrySetNewMaster(Unit* masterCandidat = nullptr);
        FormationSlotDataSPtr GetFirstEmptySlot();
        FormationSlotDataSPtr GetFirstAliveSlot();
        int32 GetDefaultSlotId(uint32 dbGuid);
        FormationSlotDataSPtr GetDefaultSlot(uint32 dbGuid, SpawnGroupFormationSlotType slotType = SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC);
        void FixSlotsPositions();
        void SwitchSlotOwner(FormationSlotDataSPtr slotA, FormationSlotDataSPtr slotB);
        bool SetFollowersMaster();
        void ClearMoveGen();
        bool FreeSlot(FormationSlotDataSPtr slot);
        bool AddInFormationSlot(Unit* newUnit, SpawnGroupFormationSlotType slotType = SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC);
        void StartFollower();
        void StopFollower();
        bool HaveOption(SpawGroupFormationOptions const& option, uint32 const& options) const { return (static_cast<uint32>(options) & option) != 0; }
        bool HaveOption(SpawGroupFormationOptions const& option) const { return (static_cast<uint32>(m_currentOptions) & option) != 0; }

        CreatureGroup* m_groupData;
        FormationEntrySPtr m_fEntry;
        SpawnGroupFormationType m_currentFormationShape;
        FormationSlotMap m_slotsMap;
        MovementGeneratorType m_masterMotionType;
        RespawnPosistion m_spawnPos;

        bool m_mirrorState;
        bool m_followerStopped;

        uint32 m_realMasterDBGuid;
        uint32 m_slotGuid;
        uint32 m_lastWP;
        uint32 m_wpPathId;
        uint32 m_currentOptions;
        TimePoint m_nextVariation;
        TimePoint m_nextVariationUpdate;
        TimePoint m_followerStartTime;

        float m_currentSpread;
};

#endif