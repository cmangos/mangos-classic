#pragma once
#include "WorldPosition.h"

namespace ai
{
    class GuidPosition : public ObjectGuid, public WorldPosition
    {
    public:
        GuidPosition() : ObjectGuid(), WorldPosition() {}
        GuidPosition(ObjectGuid guid, WorldPosition pos) : ObjectGuid(guid), WorldPosition(pos) {};
        GuidPosition(ObjectGuid guid, const uint32 mapId, const uint32 instanceId) : ObjectGuid(guid) { WorldPosition::set(guid, mapId, instanceId); if (getX() == 0 && getY() == 0 && getZ() == 0) Set(0); }
        GuidPosition(ObjectGuid guid, const WorldObject* wo) : ObjectGuid(guid), WorldPosition(wo) {}
        GuidPosition(uint64 const& guid, WorldPosition const& pos) : ObjectGuid(guid), WorldPosition(pos) {};
        //template<class T>
        //GuidPosition(ObjectGuid guid, T) : ObjectGuid(guid) {WorldPosition::set(WorldPosition(T))};
        GuidPosition(CreatureDataPair const* dataPair) : ObjectGuid(HIGHGUID_UNIT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(GameObjectDataPair const* dataPair) : ObjectGuid(HIGHGUID_GAMEOBJECT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(const WorldObject* wo) : WorldPosition(wo) { ObjectGuid::Set(wo->GetObjectGuid()); };
        GuidPosition(HighGuid hi, uint32 entry, uint32 counter = 1, WorldPosition pos = WorldPosition()) : ObjectGuid(hi, entry, counter), WorldPosition(pos) {};
        GuidPosition(std::string qualifier);

        virtual std::string to_string() const override;

        CreatureData* GetCreatureData() const { return IsCreature() ? sObjectMgr.GetCreatureData(GetCounter()) : nullptr; }
        CreatureInfo const* GetCreatureTemplate() const { return IsCreature() ? sObjectMgr.GetCreatureTemplate(GetEntry()) : nullptr; };

        GameObjectData const* GetGameObjectData() const { return IsGameObject() ? sObjectMgr.GetGOData(GetCounter()) : nullptr; }
        GameObjectInfo const* GetGameObjectInfo() const { return IsGameObject() ? sObjectMgr.GetGameObjectInfo(GetEntry()) : nullptr; };

        WorldObject* GetWorldObject(uint32 m_instanceId) const { return getMap(m_instanceId) ? getMap(m_instanceId)->GetWorldObject(*this) : nullptr; }
        Creature* GetCreature(uint32 instanceId) const;
        Unit* GetUnit(uint32 instanceId) const;
        GameObject* GetGameObject(uint32 instanceId) const;
        Player* GetPlayer() const;

#ifdef MANGOSBOT_TWO
        uint32 GetPhaseMask() const { return IsCreature() ? (GetCreatureData() ? GetCreatureData()->phaseMask : 1) : (GetGameObjectData() ? GetGameObjectData()->phaseMask : 1);}
#endif

        void updatePosition(uint32 m_instanceId) { WorldObject* wo = GetWorldObject(m_instanceId); if (wo) WorldPosition::set(wo); }

        bool HasNpcFlag(NPCFlags flag) { return IsCreature() && GetCreatureTemplate()->NpcFlags & flag; }
        bool isGoType(GameobjectTypes type) { return IsGameObject() && GetGameObjectInfo()->type == type; }

        const FactionTemplateEntry* GetFactionTemplateEntry() const;
        const ReputationRank GetReactionTo(const GuidPosition& other, uint32 instanceId) const;
        bool IsFriendlyTo(const GuidPosition& other, const uint32 instanceId) { return (GetFactionTemplateEntry() && other.GetFactionTemplateEntry()) ? (GetReactionTo(other, instanceId) > REP_NEUTRAL) : false; }
        bool IsHostileTo(const GuidPosition& other, const uint32 instanceId) const { return (GetFactionTemplateEntry() && other.GetFactionTemplateEntry()) ? (GetReactionTo(other, instanceId) < REP_NEUTRAL) : false; }

        const ReputationRank GetReactionTo(WorldObject* object) { return GetReactionTo(GuidPosition(object), object->GetInstanceId()); }
        bool IsFriendlyTo(WorldObject* object) { return IsFriendlyTo(GuidPosition(object), object->GetInstanceId()); }
        bool IsHostileTo(const WorldObject* object) { return IsHostileTo(GuidPosition(object), object->GetInstanceId()); }

        bool isDead(uint32 instanceId); //For loaded grids check if the unit/object is unloaded/dead.

        uint16 IsPartOfAPool();
        uint16 GetGameEventId();
        bool IsEventUnspawned();

        virtual std::string print();

        operator bool() const { return getX() != 0 || getY() != 0 || getZ() != 0 || !IsEmpty(); }
        bool operator!() const { return getX() == 0 && getY() == 0 && getZ() == 0 && IsEmpty(); }
        bool operator== (ObjectGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
        bool operator!= (ObjectGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
        bool operator< (ObjectGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }
    };

    inline ByteBuffer& operator<<(ByteBuffer& b, GuidPosition& guidP)
    {
        b << (ObjectGuid)guidP;
        b << (WorldPosition)guidP;

        return b;
    }

    inline ByteBuffer& operator>>(ByteBuffer& b, GuidPosition& g)
    {
        ObjectGuid guid;
        WorldPosition pos;

        b >> guid;
        b >> pos;

        g = GuidPosition(guid, pos);

        return b;
    }

    class AsyncGuidPosition : public GuidPosition
    {
    public:
        AsyncGuidPosition() : GuidPosition() {}
        AsyncGuidPosition(GuidPosition guidP) : GuidPosition(guidP) {};
        virtual void setAreaFlag(int16 flag) { areaFlag = flag; }
        virtual void FetchArea() { areaFlag = WorldPosition::getAreaFlag(); }
        virtual uint16 getAreaFlag() const { return areaFlag.has_value() ? areaFlag.value() : WorldPosition::getAreaFlag(); }
    private:
        std::optional<int16> areaFlag;
    };
}