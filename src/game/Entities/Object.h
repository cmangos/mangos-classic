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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "ByteBuffer.h"
#include "Entities/UpdateFields.h"
#include "Entities/UpdateData.h"
#include "Entities/ObjectGuid.h"
#include "Entities/EntitiesMgr.h"
#include "Globals/SharedDefines.h"
#include "Entities/Camera.h"
#include "Camera.h"
#include "Server/DBCStructure.h"

#include <set>

#define CONTACT_DISTANCE                0.5f
#define INTERACTION_DISTANCE            5.0f
#define ATTACK_DISTANCE                 5.0f
#define MELEE_LEEWAY                    8.0f / 3.0f // Melee attack and melee spell leeway when moving
#define AOE_LEEWAY                      2.0f        // AOE leeway when moving
#define INSPECT_DISTANCE                28.0f
#define TRADE_DISTANCE                  11.11f

#define MAX_VISIBILITY_DISTANCE         SIZE_OF_GRIDS               // max distance for visible object show, limited in 533 yards
#define VISIBILITY_DISTANCE_GIGANTIC    400.0f
#define VISIBILITY_DISTANCE_LARGE       200.0f
#define VISIBILITY_DISTANCE_NORMAL      100.0f
#define VISIBILITY_DISTANCE_SMALL       50.0f
#define VISIBILITY_DISTANCE_TINY        25.0f
#define DEFAULT_VISIBILITY_DISTANCE     VISIBILITY_DISTANCE_NORMAL  // default visible distance, 100 yards on continents
#define DEFAULT_VISIBILITY_INSTANCE     170.0f                      // default visible distance in instances, 170 yards
#define DEFAULT_VISIBILITY_BG           533.0f                      // default visible distance in BG/Arenas, 533 yards


#define DEFAULT_WORLD_OBJECT_SIZE       0.388999998569489f      // currently used (correctly?) for any non Unit world objects. This is actually the bounding_radius, like player/creature from creature_model_data
#define DEFAULT_OBJECT_SCALE            1.0f                    // player/item scale as default, npc/go from database, pets from dbc
#define DEFAULT_TAUREN_MALE_SCALE       1.35f                   // Tauren male player scale by default
#define DEFAULT_TAUREN_FEMALE_SCALE     1.25f                   // Tauren female player scale by default

#define MAX_STEALTH_DETECT_RANGE        45.0f
#define GRID_ACTIVATION_RANGE           45.0f

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

enum TempSpawnType
{
    TEMPSPAWN_MANUAL_DESPAWN              = 0,             // despawns when UnSummon() is called
    TEMPSPAWN_DEAD_DESPAWN                = 1,             // despawns when the creature disappears
    TEMPSPAWN_CORPSE_DESPAWN              = 2,             // despawns instantly after death
    TEMPSPAWN_CORPSE_TIMED_DESPAWN        = 3,             // despawns after a specified time after death (or when the creature disappears)
    TEMPSPAWN_TIMED_DESPAWN               = 4,             // despawns after a specified time
    TEMPSPAWN_TIMED_OOC_DESPAWN           = 5,             // despawns after a specified time after the creature is out of combat
    TEMPSPAWN_TIMED_OR_DEAD_DESPAWN       = 6,             // despawns after a specified time OR when the creature disappears
    TEMPSPAWN_TIMED_OR_CORPSE_DESPAWN     = 7,             // despawns after a specified time OR when the creature dies
    TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN   = 8,             // despawns after a specified time (OOC) OR when the creature disappears
    TEMPSPAWN_TIMED_OOC_OR_CORPSE_DESPAWN = 9,             // despawns after a specified time (OOC) OR when the creature dies
};

enum TempSpawnLinkedAura
{
    TEMPSPAWN_LINKED_AURA_OWNER_CHECK = 0x00000001,
    TEMPSPAWN_LINKED_AURA_REMOVE_OWNER = 0x00000002
};

enum PlayPacketSettings
{
    PLAY_SET,
    PLAY_TARGET,
    PLAY_MAP,
    PLAY_ZONE,
    PLAY_AREA,
};

enum DistanceCalculation
{
    DIST_CALC_NONE,
    DIST_CALC_BOUNDING_RADIUS,
    DIST_CALC_COMBAT_REACH,
    DIST_CALC_COMBAT_REACH_WITH_MELEE,
};

struct PlayPacketParameters
{
    PlayPacketParameters(PlayPacketSettings setting) : setting(setting) {}
    PlayPacketParameters(PlayPacketSettings setting, Player const* target) : setting(setting) { this->target.target = target; }
    PlayPacketParameters(PlayPacketSettings setting, uint32 id) : setting(setting) { this->areaOrZone.id = id; }
    PlayPacketSettings setting;
    union
    {
        struct
        {
            Player const* target;
        } target;

        struct
        {
            uint32 id;
        } areaOrZone;
    };
};

class WorldPacket;
class UpdateData;
class WorldSession;
class Creature;
class Player;
class Unit;
class Group;
class Map;
class UpdateMask;
class InstanceData;
class TerrainInfo;
struct MangosStringLocale;
class Loot;
struct ItemPrototype;
class ChatHandler;
struct SpellEntry;

typedef std::unordered_map<Player*, UpdateData> UpdateDataMapType;

class CooldownData
{
        friend class CooldownContainer;
    public:
        CooldownData(TimePoint clockNow, uint32 spellId, uint32 duration, uint32 spellCategory, uint32 categoryDuration, uint32 itemId = 0, bool isPermanent = false) :
            m_spellId(spellId),
            m_category(spellCategory),
            m_expireTime(duration ? std::chrono::milliseconds(duration) + clockNow : TimePoint()),
            m_catExpireTime(spellCategory && categoryDuration ? std::chrono::milliseconds(categoryDuration) + clockNow : TimePoint()),
            m_typePermanent(isPermanent),
            m_itemId(itemId)
        {}

        // return false if permanent
        bool GetSpellCDExpireTime(TimePoint& expireTime) const
        {
            if (m_typePermanent)
                return false;

            expireTime = m_expireTime;
            return true;
        }

        // return false if permanent
        bool GetCatCDExpireTime(TimePoint& expireTime) const
        {
            if (m_typePermanent)
                return false;

            expireTime = m_catExpireTime;
            return true;
        }

        bool IsSpellCDExpired(TimePoint const& now) const
        {
            if (m_typePermanent)
                return false;

            return now >= m_expireTime;
        }

        bool IsCatCDExpired(TimePoint const& now) const
        {
            if (m_typePermanent)
                return false;

            if (!m_category)
                return true;

            if (now >= m_catExpireTime)
                return true;

            return false;
        }

        bool IsPermanent() const { return m_typePermanent; }
        uint32 GetItemId() const { return m_itemId; }
        uint32 GetSpellId() const { return m_spellId; }
        uint32 GetCategory() const { return m_category; }

    private:
        uint32            m_spellId;
        uint32            m_category;
        TimePoint         m_expireTime;
        TimePoint         m_catExpireTime;
        bool              m_typePermanent;
        uint32            m_itemId;
};

typedef std::unique_ptr<CooldownData> CooldownDataUPTR;
typedef std::map<uint32, TimePoint> GCDMap;
typedef std::map<SpellSchools, TimePoint> LockoutMap;

class CooldownContainer
{
    public:
        typedef std::map<uint32, CooldownDataUPTR> spellIdMap;
        typedef spellIdMap::const_iterator ConstIterator;
        typedef spellIdMap::iterator Iterator;
        typedef std::map<uint32, ConstIterator> categoryMap;

        void Update(TimePoint const& now)
        {
            auto spellCDItr = m_spellIdMap.begin();
            while (spellCDItr != m_spellIdMap.end())
            {
                auto& cd = spellCDItr->second;
                if (cd->IsSpellCDExpired(now) && cd->IsCatCDExpired(now)) // will not remove permanent CD
                    spellCDItr = erase(spellCDItr);
                else
                {
                    if (cd->m_category && cd->IsCatCDExpired(now))
                        m_categoryMap.erase(cd->m_category);
                    ++spellCDItr;
                }
            }
        }

        bool AddCooldown(TimePoint clockNow, uint32 spellId, uint32 duration, uint32 spellCategory = 0, uint32 categoryDuration = 0, uint32 itemId = 0, bool onHold = false)
        {
            auto resultItr = m_spellIdMap.emplace(spellId, std::unique_ptr<CooldownData>(new CooldownData(clockNow, spellId, duration, spellCategory, categoryDuration, itemId, onHold)));
            if (resultItr.second && spellCategory && categoryDuration)
                m_categoryMap.emplace(spellCategory, resultItr.first);

            return resultItr.second;
        }

        void RemoveBySpellId(uint32 spellId)
        {
            auto spellCDItr = m_spellIdMap.find(spellId);
            if (spellCDItr != m_spellIdMap.end())
            {
                auto& cdData = spellCDItr->second;
                if (cdData->m_category)
                {
                    auto catCDItr = m_categoryMap.find(cdData->m_category);
                    if (catCDItr != m_categoryMap.end())
                        m_categoryMap.erase(catCDItr);
                }
                m_spellIdMap.erase(spellCDItr);
            }
        }

        void RemoveByCategory(uint32 category)
        {
            auto spellCDItr = m_categoryMap.find(category);
            if (spellCDItr != m_categoryMap.end())
                m_categoryMap.erase(spellCDItr);
        }

        Iterator erase(ConstIterator spellCDItr)
        {
            auto& cdData = spellCDItr->second;
            if (cdData->m_category)
            {
                auto catCDItr = m_categoryMap.find(cdData->m_category);
                if (catCDItr != m_categoryMap.end())
                    m_categoryMap.erase(catCDItr);
            }
            return m_spellIdMap.erase(spellCDItr);
        }

        ConstIterator FindBySpellId(uint32 id) const { return m_spellIdMap.find(id); }

        ConstIterator FindByCategory(uint32 category) const
        {
            auto itr = m_categoryMap.find(category);
            return itr != m_categoryMap.end() ? itr->second : end();
        }

        void clear() { m_spellIdMap.clear(); m_categoryMap.clear(); }

        ConstIterator begin() const { return m_spellIdMap.begin(); }
        ConstIterator end() const { return m_spellIdMap.end(); }
        bool IsEmpty() const { return m_spellIdMap.empty(); }
        size_t size() const { return m_spellIdMap.size(); }

    private:
        spellIdMap m_spellIdMap;
        categoryMap m_categoryMap;
};

struct Position
{
    Position() : x(0.0f), y(0.0f), z(0.0f), o(0.0f) {}
    Position(float _x, float _y, float _z, float _o) : x(_x), y(_y), z(_z), o(_o) {}
    float x, y, z, o;
};

struct WorldLocation
{
    uint32 mapid;
    float coord_x;
    float coord_y;
    float coord_z;
    float orientation;
    explicit WorldLocation(uint32 _mapid = 0, float _x = 0, float _y = 0, float _z = 0, float _o = 0)
        : mapid(_mapid), coord_x(_x), coord_y(_y), coord_z(_z), orientation(_o) {}
    WorldLocation(WorldLocation const& loc)
        : mapid(loc.mapid), coord_x(loc.coord_x), coord_y(loc.coord_y), coord_z(loc.coord_z), orientation(loc.orientation) {}
};


// use this class to measure time between world update ticks
// essential for units updating their spells after cells become active
class WorldUpdateCounter
{
    public:
        WorldUpdateCounter() : m_tmStart(0) {}

        time_t timeElapsed()
        {
            if (!m_tmStart)
                m_tmStart = WorldTimer::tickPrevTime();

            return WorldTimer::getMSTimeDiff(m_tmStart, WorldTimer::tickTime());
        }

        void Reset() { m_tmStart = WorldTimer::tickTime(); }

    private:
        uint32 m_tmStart;
};

class Object
{
    public:
        virtual ~Object();

        const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld()
        {
            if (m_inWorld)
                return;

            m_inWorld = true;

            // synchronize values mirror with values array (changes will send in updatecreate opcode any way
            ClearUpdateMask(false);                         // false - we can't have update data in update queue before adding to world
        }
        virtual void RemoveFromWorld()
        {
            // if we remove from world then sending changes not required
            ClearUpdateMask(true);
            m_inWorld = false;
        }

        ObjectGuid const& GetObjectGuid() const { return GetGuidValue(OBJECT_FIELD_GUID); }
        const uint64& GetGUID() const { return GetUInt64Value(OBJECT_FIELD_GUID); } // DEPRECATED, not use, will removed soon
        uint32 GetGUIDLow() const { return GetObjectGuid().GetCounter(); }
        PackedGuid const& GetPackGUID() const { return m_PackGUID; }
        std::string GetGuidStr() const { return GetObjectGuid().GetString(); }

        uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
        void SetEntry(uint32 entry) { SetUInt32Value(OBJECT_FIELD_ENTRY, entry); }

        float GetObjectScale() const
        {
            return m_floatValues[OBJECT_FIELD_SCALE_X] ? m_floatValues[OBJECT_FIELD_SCALE_X] : DEFAULT_OBJECT_SCALE;
        }

        void SetObjectScale(float newScale);

        uint8 GetTypeId() const { return m_objectTypeId; }
        bool isType(TypeMask mask) const { return (mask & m_objectType) != 0; }

        virtual void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void SendCreateUpdateToPlayer(Player* player) const;

        // must be overwrite in appropriate subclasses (WorldObject, Item currently), or will crash
        virtual void AddToClientUpdateList();
        virtual void RemoveFromClientUpdateList();
        virtual void BuildUpdateData(UpdateDataMapType& update_players);
        void MarkForClientUpdate();
        void SendForcedObjectUpdate();

        void BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void BuildForcedValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void BuildOutOfRangeUpdateBlock(UpdateData* data) const;
        void BuildMovementUpdateBlock(UpdateData* data, uint8 flags = 0) const;

        virtual void DestroyForPlayer(Player* target) const;

        const int32& GetInt32Value(uint16 index) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return m_int32Values[ index ];
        }

        const uint32& GetUInt32Value(uint16 index) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return m_uint32Values[ index ];
        }

        const uint64& GetUInt64Value(uint16 index) const
        {
            MANGOS_ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
            return *((uint64*) & (m_uint32Values[ index ]));
        }

        const float& GetFloatValue(uint16 index) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return m_floatValues[ index ];
        }

        uint8 GetByteValue(uint16 index, uint8 offset) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            MANGOS_ASSERT(offset < 4);
            return *(((uint8*)&m_uint32Values[ index ]) + offset);
        }

        uint16 GetUInt16Value(uint16 index, uint8 offset) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            MANGOS_ASSERT(offset < 2);
            return *(((uint16*)&m_uint32Values[ index ]) + offset);
        }

        ObjectGuid const& GetGuidValue(uint16 index) const { return *reinterpret_cast<ObjectGuid const*>(&GetUInt64Value(index)); }

        void SetInt32Value(uint16 index,        int32  value);
        void SetUInt32Value(uint16 index,       uint32  value);
        void SetUInt64Value(uint16 index, const uint64& value);
        void SetFloatValue(uint16 index,       float   value);
        void SetByteValue(uint16 index, uint8 offset, uint8 value);
        void SetUInt16Value(uint16 index, uint8 offset, uint16 value);
        void SetInt16Value(uint16 index, uint8 offset, int16 value) { SetUInt16Value(index, offset, (uint16)value); }
        void SetGuidValue(uint16 index, ObjectGuid const& value) { SetUInt64Value(index, value.GetRawValue()); }
        void SetStatFloatValue(uint16 index, float value);
        void SetStatInt32Value(uint16 index, int32 value);
        void ForceValuesUpdateAtIndex(uint16 index);
        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModPositiveFloatValue(uint16 index, float val, bool apply);
        void ApplyModSignedFloatValue(uint16 index, float val, bool apply);

        void ApplyPercentModFloatValue(uint16 index, float val, bool apply)
        {
            val = val != -100.0f ? val : -99.9f ;
            SetFloatValue(index, GetFloatValue(index) * (apply ? (100.0f + val) / 100.0f : 100.0f / (100.0f + val)));
        }

        void SetFlag(uint16 index, uint32 newFlag);
        void RemoveFlag(uint16 index, uint32 oldFlag);

        void ToggleFlag(uint16 index, uint32 flag)
        {
            if (HasFlag(index, flag))
                RemoveFlag(index, flag);
            else
                SetFlag(index, flag);
        }

        bool HasFlag(uint16 index, uint32 flag) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return (m_uint32Values[ index ] & flag) != 0;
        }

        void ApplyModFlag(uint16 index, uint32 flag, bool apply)
        {
            if (apply)
                SetFlag(index, flag);
            else
                RemoveFlag(index, flag);
        }

        void SetByteFlag(uint16 index, uint8 offset, uint8 newFlag);
        void RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag);

        void ToggleByteFlag(uint16 index, uint8 offset, uint8 flag)
        {
            if (HasByteFlag(index, offset, flag))
                RemoveByteFlag(index, offset, flag);
            else
                SetByteFlag(index, offset, flag);
        }

        bool HasByteFlag(uint16 index, uint8 offset, uint8 flag) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            MANGOS_ASSERT(offset < 4);
            return (((uint8*)&m_uint32Values[index])[offset] & flag) != 0;
        }

        void ApplyModByteFlag(uint16 index, uint8 offset, uint32 flag, bool apply)
        {
            if (apply)
                SetByteFlag(index, offset, flag);
            else
                RemoveByteFlag(index, offset, flag);
        }

        void SetShortFlag(uint16 index, bool highpart, uint16 newFlag);
        void RemoveShortFlag(uint16 index, bool highpart, uint16 oldFlag);

        void ToggleShortFlag(uint16 index, bool highpart, uint8 flag)
        {
            if (HasShortFlag(index, highpart, flag))
                RemoveShortFlag(index, highpart, flag);
            else
                SetShortFlag(index, highpart, flag);
        }

        bool HasShortFlag(uint16 index, bool highpart, uint8 flag) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return (((uint16*)&m_uint32Values[index])[highpart ? 1 : 0] & flag) != 0;
        }

        void ApplyModShortFlag(uint16 index, bool highpart, uint32 flag, bool apply)
        {
            if (apply)
                SetShortFlag(index, highpart, flag);
            else
                RemoveShortFlag(index, highpart, flag);
        }

        void SetFlag64(uint16 index, uint64 newFlag)
        {
            uint64 oldval = GetUInt64Value(index);
            uint64 newval = oldval | newFlag;
            SetUInt64Value(index, newval);
        }

        void RemoveFlag64(uint16 index, uint64 oldFlag)
        {
            uint64 oldval = GetUInt64Value(index);
            uint64 newval = oldval & ~oldFlag;
            SetUInt64Value(index, newval);
        }

        void ToggleFlag64(uint16 index, uint64 flag)
        {
            if (HasFlag64(index, flag))
                RemoveFlag64(index, flag);
            else
                SetFlag64(index, flag);
        }

        bool HasFlag64(uint16 index, uint64 flag) const
        {
            MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, false));
            return (GetUInt64Value(index) & flag) != 0;
        }

        void ApplyModFlag64(uint16 index, uint64 flag, bool apply)
        {
            if (apply)
                SetFlag64(index, flag);
            else
                RemoveFlag64(index, flag);
        }

        void ClearUpdateMask(bool remove);

        bool LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        virtual bool HasQuest(uint32 /* quest_id */) const { return false; }
        virtual bool HasInvolvedQuest(uint32 /* quest_id */) const { return false; }
        void SetItsNewObject(bool enable) { m_itsNewObject = enable; }

        Loot* loot;

        inline bool IsPlayer() const { return GetTypeId() == TYPEID_PLAYER; }
        inline bool IsCreature() const { return GetTypeId() == TYPEID_UNIT; }
        inline bool IsUnit() const { return isType(TYPEMASK_UNIT); }
        inline bool IsGameObject() const { return GetTypeId() == TYPEID_GAMEOBJECT; }
        inline bool IsCorpse() const { return GetTypeId() == TYPEID_CORPSE; }

    protected:
        Object();

        void _InitValues();
        void _Create(uint32 guidlow, uint32 entry, HighGuid guidhigh);

        virtual void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;

        virtual void _SetCreateBits(UpdateMask* updateMask, Player* target) const;

        void BuildMovementUpdate(ByteBuffer* data, uint8 updateFlags) const;
        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, UpdateMask* updateMask, Player* target) const;
        void BuildUpdateDataForPlayer(Player* pl, UpdateDataMapType& update_players) const;

        uint16 m_objectType;

        uint8 m_objectTypeId;
        uint8 m_updateFlag;

        union
        {
            int32*  m_int32Values;
            uint32* m_uint32Values;
            float*  m_floatValues;
        };

        std::vector<bool> m_changedValues;

        uint16 m_valuesCount;

        bool m_objectUpdated;

    private:
        bool m_inWorld;
        bool m_itsNewObject;

        PackedGuid m_PackGUID;

        Object(const Object&);                              // prevent generation copy constructor
        Object& operator=(Object const&);                   // prevent generation assigment operator

    public:
        // for output helpfull error messages from ASSERTs
        bool PrintIndexError(uint32 index, bool set) const;
        bool PrintEntryError(char const* descr) const;
};

struct WorldObjectChangeAccumulator;

class WorldObject : public Object
{
        friend struct WorldObjectChangeAccumulator;

    public:
        virtual ~WorldObject() {}

        virtual void Update(const uint32 /*diff*/) {}

        void _Create(uint32 guidlow, HighGuid guidhigh);

        void Relocate(float x, float y, float z, float orientation);
        void Relocate(float x, float y, float z);

        void SetOrientation(float orientation);

        float GetPositionX() const { return m_position.x; }
        float GetPositionY() const { return m_position.y; }
        float GetPositionZ() const { return m_position.z; }
        void GetPosition(float& x, float& y, float& z) const
        { x = m_position.x; y = m_position.y; z = m_position.z; }
        void GetPosition(WorldLocation& loc) const
        { loc.mapid = m_mapId; GetPosition(loc.coord_x, loc.coord_y, loc.coord_z); loc.orientation = GetOrientation(); }
        float GetOrientation() const { return m_position.o; }

        /// Gives a 2d-point in distance distance2d in direction absAngle around the current position (point-to-point)
        void GetNearPoint2D(float& x, float& y, float distance2d, float absAngle) const;
        /** Gives a "free" spot for searcher in distance distance2d in direction absAngle on "good" height
         * @param searcher          -           for whom a spot is searched for
         * @param x, y, z           -           position for the found spot of the searcher
         * @param searcher_bounding_radius  -   how much space the searcher will require
         * @param distance2d        -           distance between the middle-points
         * @param absAngle          -           angle in which the spot is preferred
         */
        void GetNearPoint(WorldObject const* searcher, float& x, float& y, float& z, float searcher_bounding_radius, float distance2d, float absAngle, bool isInWater = false) const;
        /** Gives a "free" spot for a searcher on the distance (including bounding-radius calculation)
         * @param x, y, z           -           position for the found spot
         * @param bounding_radius   -           radius for the searcher
         * @param distance2d        -           range in which to find a free spot. Default = 0.0f (which usually means the units will have contact)
         * @param angle             -           direction in which to look for a free spot. Default = 0.0f (direction in which 'this' is looking
         * @param obj               -           for whom to look for a spot. Default = nullptr
         */
        void GetClosePoint(float& x, float& y, float& z, float bounding_radius, float distance2d = 0.0f, float angle = 0.0f, const WorldObject* obj = nullptr) const
        {
            // angle calculated from current orientation
            GetNearPoint(obj, x, y, z, bounding_radius, distance2d + GetObjectBoundingRadius() + bounding_radius, GetOrientation() + angle);
        }
        /** Gives a "free" spot for a searcher in contact-range of "this" (including bounding-radius calculation)
         * @param x, y, z           -           position for the found spot
         * @param obj               -           for whom to find a contact position. The position will be searched in direction from 'this' towards 'obj'
         * @param distance2d        -           distance which 'obj' and 'this' should have beetween their bounding radiuses. Default = CONTACT_DISTANCE
         */
        void GetContactPoint(const WorldObject* obj, float& x, float& y, float& z, float distance2d = CONTACT_DISTANCE) const
        {
            // angle to face `obj` to `this` using distance includes size of `obj`
            GetNearPoint(obj, x, y, z, obj->GetObjectBoundingRadius(), distance2d + GetObjectBoundingRadius() + obj->GetObjectBoundingRadius(), GetAngle(obj));
        }

        virtual float GetObjectBoundingRadius() const { return DEFAULT_WORLD_OBJECT_SIZE; }
        virtual float GetCombatReach() const { return 0.f; }
        float GetCombinedCombatReach(WorldObject const* pVictim, bool forMeleeRange = true, float flat_mod = 0.0f) const;
        float GetCombinedCombatReach(bool forMeleeRange = true, float flat_mod = 0.0f) const;

        bool IsPositionValid() const;
        void UpdateGroundPositionZ(float x, float y, float& z) const;
        virtual void UpdateAllowedPositionZ(float x, float y, float& z, Map* atMap = nullptr) const;

        void MovePositionToFirstCollision(WorldLocation &pos, float dist, float angle);
        void GetFirstCollisionPosition(WorldLocation &pos, float dist, float angle)
        {
            GetPosition(pos);
            MovePositionToFirstCollision(pos, dist, angle);
        }
        void GetRandomPoint(float x, float y, float z, float distance, float& rand_x, float& rand_y, float& rand_z, float minDist = 0.0f, float const* ori = nullptr) const;

        uint32 GetMapId() const { return m_mapId; }
        uint32 GetInstanceId() const { return m_InstanceId; }

        uint32 GetZoneId() const;
        uint32 GetAreaId() const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const;

        InstanceData* GetInstanceData() const;

        const char* GetName() const { return m_name.c_str(); }
        void SetName(const std::string& newname) { m_name = newname; }

        virtual const char* GetNameForLocaleIdx(int32 /*locale_idx*/) const { return GetName(); }

        virtual ObjectGuid const& GetOwnerGuid() const { return GetGuidValue(OBJECT_FIELD_GUID); }
        virtual void SetOwnerGuid(ObjectGuid /*guid*/) { }

        float GetDistance(const WorldObject* obj, bool is3D = true, DistanceCalculation distcalc = DIST_CALC_BOUNDING_RADIUS) const;
        float GetDistance(float x, float y, float z, DistanceCalculation distcalc = DIST_CALC_BOUNDING_RADIUS) const;
        float GetDistance2d(float x, float y, DistanceCalculation distcalc = DIST_CALC_BOUNDING_RADIUS) const;
        float GetDistanceZ(const WorldObject* obj) const;
        bool IsInMap(const WorldObject* obj) const
        {
            return obj && IsInWorld() && obj->IsInWorld() && (GetMap() == obj->GetMap());
        }
        bool IsWithinCombatDist(WorldObject const* obj, float dist2compare, bool is3D = true) const
        {
            return obj && _IsWithinCombatDist(obj, dist2compare, is3D);
        }
        bool IsWithinCombatDistInMap(WorldObject const* obj, float dist2compare, bool is3D = true) const
        {
            return obj && IsInMap(obj) && _IsWithinCombatDist(obj, dist2compare, is3D);
        }
        bool IsWithinDist3d(float x, float y, float z, float dist2compare) const;
        bool IsWithinDist2d(float x, float y, float dist2compare) const;
        bool _IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const;
        bool _IsWithinCombatDist(WorldObject const* obj, float dist2compare, bool is3D) const;

        // use only if you will sure about placing both object at same map
        bool IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D = true) const
        {
            return obj && _IsWithinDist(obj, dist2compare, is3D);
        }

        bool IsWithinDistInMap(WorldObject const* obj, float dist2compare, bool is3D = true) const
        {
            return obj && IsInMap(obj) && _IsWithinDist(obj, dist2compare, is3D);
        }
        bool IsWithinLOS(float ox, float oy, float oz, bool ignoreM2Model = false) const;
        bool IsWithinLOSInMap(const WorldObject* obj, bool ignoreM2Model = false) const;
        bool GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D = true, DistanceCalculation distcalc = DIST_CALC_NONE) const;
        bool IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D = true, bool combat = false) const;
        bool IsInRange2d(float x, float y, float minRange, float maxRange, bool combat = false) const;
        bool IsInRange3d(float x, float y, float z, float minRange, float maxRange, bool combat = false) const;

        float GetAngle(const WorldObject* obj) const;
        float GetAngle(const float x, const float y) const;
        bool HasInArc(const WorldObject* target, const float arc = M_PI) const;
        bool isInFrontInMap(WorldObject const* target, float distance, float arc = M_PI) const;
        bool isInBackInMap(WorldObject const* target, float distance, float arc = M_PI) const;
        // Used in AOE - meant to ignore bounding radius of source
        bool isInFront(WorldObject const* target, float distance, float arc = M_PI) const;
        // Used in AOE - meant to ignore bounding radius of source
        bool isInBack(WorldObject const* target, float distance, float arc = M_PI) const;
        bool IsFacingTargetsBack(const WorldObject* target, float arc = M_PI_F) const;
        bool IsFacingTargetsFront(const WorldObject* target, float arc = M_PI_F) const;

        virtual void CleanupsBeforeDelete();                // used in destructor or explicitly before mass creature delete to remove cross-references to already deleted units

        virtual void SendMessageToSet(WorldPacket const& data, bool self) const;
        virtual void SendMessageToSetInRange(WorldPacket const& data, float dist, bool self) const;
        void SendMessageToSetExcept(WorldPacket& data, Player const* skipped_receiver) const;

        void MonsterSay(const char* text, uint32 language, Unit const* target = nullptr) const;
        void MonsterYell(const char* text, uint32 language, Unit const* target = nullptr) const;
        void MonsterTextEmote(const char* text, Unit const* target, bool IsBossEmote = false) const;
        void MonsterWhisper(const char* text, Unit const* target, bool IsBossWhisper = false) const;
        void MonsterText(MangosStringLocale const* textData, Unit const* target) const;

        void PlayDistanceSound(uint32 sound_id, PlayPacketParameters parameters = PlayPacketParameters(PLAY_SET)) const;
        void PlayDirectSound(uint32 sound_id, PlayPacketParameters parameters = PlayPacketParameters(PLAY_SET)) const;
        void PlayMusic(uint32 sound_id, PlayPacketParameters parameters = PlayPacketParameters(PLAY_SET)) const;
        void PlaySpellVisual(uint32 artKitId, PlayPacketParameters parameters = PlayPacketParameters(PLAY_SET)) const;
        void HandlePlayPacketSettings(WorldPacket& msg, PlayPacketParameters& parameters) const;

        void SendObjectDeSpawnAnim(ObjectGuid guid) const;
        void SendGameObjectCustomAnim(ObjectGuid guid, uint32 animId = 0) const;

        virtual ReputationRank GetReactionTo(Unit const* unit) const;
        virtual ReputationRank GetReactionTo(Corpse const* corpse) const;

        virtual bool IsEnemy(Unit const* unit) const;
        virtual bool IsFriend(Unit const* unit) const;

        bool IsControlledByPlayer() const;

        virtual void SaveRespawnTime() {}
        void AddObjectToRemoveList();

        void UpdateObjectVisibility();
        virtual void UpdateVisibilityAndView();             // update visibility for object and object for all around

        // main visibility check function in normal case (ignore grey zone distance check)
        bool isVisibleFor(Player const* u, WorldObject const* viewPoint) const { return isVisibleForInState(u, viewPoint, false); }

        // low level function for visibility change code, must be define in all main world object subclasses
        virtual bool isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const = 0;

        void SetMap(Map* map);
        Map* GetMap() const { MANGOS_ASSERT(m_currMap); return m_currMap; }
        // used to check all object's GetMap() calls when object is not in world!
        void ResetMap() { m_currMap = nullptr; }

        // obtain terrain data for map where this object belong...
        TerrainInfo const* GetTerrain() const;

        void AddToClientUpdateList() override;
        void RemoveFromClientUpdateList() override;
        void BuildUpdateData(UpdateDataMapType&) override;

        Creature* SummonCreature(uint32 id, float x, float y, float z, float ang, TempSpawnType spwtype, uint32 despwtime, bool asActiveObject = false, bool setRun = false, uint32 pathId = 0, uint32 faction = 0, bool spawnCounting = false, bool forcedOnTop = false);

        bool isActiveObject() const { return m_isActiveObject || m_viewPoint.hasViewers(); }
        void SetActiveObjectState(bool active);

        // Visibility stuff
        bool IsVisibilityOverridden() const { return m_visibilityDistanceOverride != 0.f; }
        void SetVisibilityDistanceOverride(VisibilityDistanceType type);

        float GetVisibilityDistance() const;
        float GetVisibilityDistanceFor(WorldObject* obj) const;

        ViewPoint& GetViewPoint() { return m_viewPoint; }

        // ASSERT print helper
        bool PrintCoordinatesError(float x, float y, float z, char const* descr) const;

        // Game Event Notification system
        virtual bool IsNotifyOnEventObject() { return m_isOnEventNotified; }
        virtual void OnEventHappened(uint16 /*event_id*/, bool /*activate*/, bool /*resume*/) {}
        void SetNotifyOnEventState(bool state);

        virtual void AddToWorld() override;
        virtual void RemoveFromWorld() override;

        // cooldown system
        virtual void AddGCD(SpellEntry const& spellEntry, uint32 forcedDuration = 0, bool updateClient = false);
        virtual bool HasGCD(SpellEntry const* spellEntry) const;
        void ResetGCD(SpellEntry const* spellEntry = nullptr);
        virtual void AddCooldown(SpellEntry const& spellEntry, ItemPrototype const* itemProto = nullptr, bool permanent = false, uint32 forcedDuration = 0);
        virtual void RemoveSpellCooldown(SpellEntry const& spellEntry, bool updateClient = true);
        void RemoveSpellCooldown(uint32 spellId, bool updateClient = true);
        virtual void RemoveSpellCategoryCooldown(uint32 category, bool updateClient = true);
        virtual void RemoveAllCooldowns(bool /*sendOnly*/ = false) { m_GCDCatMap.clear(); m_cooldownMap.clear(); m_lockoutMap.clear(); }
        bool IsSpellReady(SpellEntry const& spellEntry, ItemPrototype const* itemProto = nullptr) const;
        bool IsSpellReady(uint32 spellId, ItemPrototype const* itemProto = nullptr) const;
        virtual void LockOutSpells(SpellSchoolMask schoolMask, uint32 duration);
        void PrintCooldownList(ChatHandler& chat) const;

        virtual void InspectingLoot() {}

        virtual bool CanAttackSpell(Unit const* /*target*/, SpellEntry const* /*spellInfo*/ = nullptr, bool /*isAOE*/ = false) const { return true; }
        virtual bool CanAssistSpell(Unit const* /*target*/, SpellEntry const* /*spellInfo*/ = nullptr) const { return true; }

    protected:
        explicit WorldObject();

        // these functions are used mostly for Relocate() and Corpse/Player specific stuff...
        // use them ONLY in LoadFromDB()/Create() funcs and nowhere else!
        // mapId/instanceId should be set in SetMap() function!
        void SetLocationMapId(uint32 _mapId) { m_mapId = _mapId; }
        void SetLocationInstanceId(uint32 _instanceId) { m_InstanceId = _instanceId; }

        // cooldown system
        void UpdateCooldowns(TimePoint const& now);
        bool CheckLockout(SpellSchoolMask schoolMask) const;
        bool GetExpireTime(SpellEntry const& spellEntry, TimePoint& expireTime, bool& isPermanent) const;

        GCDMap            m_GCDCatMap;
        LockoutMap        m_lockoutMap;
        CooldownContainer m_cooldownMap;

        std::string m_name;

        bool m_isOnEventNotified;

        float m_visibilityDistanceOverride;

    private:
        Map* m_currMap;                                     // current object's Map location

        uint32 m_mapId;                                     // object at map with map_id
        uint32 m_InstanceId;                                // in map copy with instance id

        Position m_position;
        ViewPoint m_viewPoint;
        bool m_isActiveObject;
};

#endif
