#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/WorldPosition.h"

namespace ai
{
    class Hazard
    {
    public:
        Hazard();
        Hazard(const WorldPosition& position, uint64 expiration, float radius);
        Hazard(const ObjectGuid& guid, uint64 expiration, float radius);
        bool operator==(const Hazard& other) const;
        bool operator<(const Hazard& other) const;

        bool GetPosition(PlayerbotAI* ai, WorldPosition& outPosition);
        float GetRadius() const { return radius; }

        bool IsValid(PlayerbotAI* ai) const;
        bool IsExpired() const;

        uint32 GetNavmeshArea() const { return hazardNavmeshArea; }
        uint32 GetPreviousNavmeshArea() const { return previousNavmeshArea; }

    private:
        const WorldObject* GetObject(PlayerbotAI* ai) const;

    private:
        WorldPosition position;
        time_t expirationTime;
        ObjectGuid guid;
        float radius;

        uint32 hazardNavmeshArea;
        uint32 previousNavmeshArea;
    };

    // Hazard position, Hazard radius
    typedef std::pair<WorldPosition, float> HazardPosition;

    // DO NOT USE. Value to store hazards which the bot must avoid when moving around
    class StoredHazardsValue : public ManualSetValue<std::list<Hazard>>
	{
	public:
        StoredHazardsValue(PlayerbotAI* ai, std::string name = "stored hazards") : ManualSetValue<std::list<Hazard>>(ai, {}, name) {}
    };

    // Value to add hazards which the bot must avoid when moving around
    class AddHazardValue : public ManualSetValue<Hazard>
    {
    public:
        AddHazardValue(PlayerbotAI* ai, std::string name = "add hazard") : ManualSetValue<Hazard>(ai, Hazard(), name) {}

    private:
        void Set(Hazard hazard) override;
    };

    // Value to get hazards position and radius which the bot must avoid when moving around
    class HazardsValue : public CalculatedValue<std::list<HazardPosition>>
    {
    public:
        HazardsValue(PlayerbotAI* ai, std::string name = "hazards") : CalculatedValue<std::list<HazardPosition>>(ai, name, 1) {}

    private:
        std::list<HazardPosition> Calculate() override;
    };
}
