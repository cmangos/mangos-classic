#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class PositionEntry
    {
    public:
        PositionEntry() : valueSet(false), x(0), y(0), z(0), mapId(0) {}
        PositionEntry(float x, float y, float z, uint32 mapId, bool valueSet = true) : valueSet(valueSet), x(x), y(y), z(z), mapId(mapId) {}
        PositionEntry(const PositionEntry& other) : valueSet(other.valueSet), x(other.x), y(other.y), z(other.z), mapId(other.mapId) {}
        PositionEntry(const WorldPosition& pos) : valueSet(pos), x(pos.coord_x), y(pos.coord_y), z(pos.coord_z), mapId(pos.mapid) {}
        void Set(float x, float y, float z, uint32 mapId) { this->x = x; this->y = y; this->z = z; this->mapId = mapId; this->valueSet = true; }
        void Set(WorldPosition pos) { this->x = pos.coord_x; this->y = pos.coord_y; this->z = pos.coord_z; this->mapId = pos.mapid; this->valueSet = true; }
        WorldPosition Get() { return WorldPosition(mapId, x, y, z); }
        void Reset() { valueSet = false; }
        bool isSet() { return valueSet; }

    public:
        float x, y, z;
        bool valueSet;
        uint32 mapId;
    };

    typedef std::map<std::string, PositionEntry> PositionMap;

    class PositionValue : public ManualSetValue<PositionMap&>
	{
	public:
        PositionValue(PlayerbotAI* ai, std::string name = "position");
        virtual std::string Save();
        virtual bool Load(std::string value);

	private:
        PositionMap positions;
    };

    class SinglePositionValue : public CalculatedValue<PositionEntry>, public Qualified
    {
    public: 
        SinglePositionValue(PlayerbotAI* ai, std::string name = "pos") : CalculatedValue(ai, name), Qualified() {};
        virtual PositionEntry Calculate() override;
        virtual void Set(PositionEntry value) override;
        virtual void Reset() override;
    };

    class CurrentPositionValue : public LogCalculatedValue<WorldPosition>
    {
    public:
        CurrentPositionValue(PlayerbotAI* ai, std::string name = "current position", uint32 checkInterval = 1) : LogCalculatedValue<WorldPosition>(ai, name, checkInterval) { minChangeInterval = 60;  logLength = 30; };
        virtual bool EqualToLast(WorldPosition value) { return value.fDist(lastValue) < sPlayerbotAIConfig.tooCloseDistance; }
        virtual WorldPosition Calculate() { return WorldPosition(bot); };
    };  

    class MasterPositionValue : public MemoryCalculatedValue<WorldPosition>
    {
    public:
        MasterPositionValue(PlayerbotAI* ai, std::string name = "master position", uint32 checkInterval = 1) : MemoryCalculatedValue<WorldPosition>(ai, name, checkInterval) { minChangeInterval = 1; };
        virtual bool EqualToLast(WorldPosition value) { return value.fDist(lastValue) < sPlayerbotAIConfig.proximityDistance; }
        virtual WorldPosition Calculate() { Player* master = GetMaster();  if (master) return WorldPosition(master); return WorldPosition(); };
    };

    class CustomPositionValue : public ManualSetValue<WorldPosition>, public Qualified
    {
    public:
        CustomPositionValue(PlayerbotAI* ai, std::string name = "custom position") : ManualSetValue<WorldPosition>(ai, WorldPosition(), name), Qualified() { };
        virtual WorldPosition Calculate() { return WorldPosition(bot); };
    };
}
