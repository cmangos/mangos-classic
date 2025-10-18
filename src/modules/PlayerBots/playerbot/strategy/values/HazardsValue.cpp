
#include "playerbot/playerbot.h"
#include "HazardsValue.h"
#include "playerbot/strategy/AiObjectContext.h"
#include "MotionGenerators/PathFinder.h"

using namespace ai;

Hazard::Hazard()
: position(WorldPosition())
, expirationTime(0)
, guid(ObjectGuid())
, radius(0.0f)
, hazardNavmeshArea(0)
, previousNavmeshArea(0)
{

}

Hazard::Hazard(const WorldPosition& inPosition, uint64 inExpiration, float inRadius)
: position(inPosition)
, expirationTime(time(0) + inExpiration)
, guid(ObjectGuid())
, radius(inRadius)
, hazardNavmeshArea(14)
, previousNavmeshArea(11)
{

}

Hazard::Hazard(const ObjectGuid& inGuid, uint64 inExpiration, float inRadius)
: position(WorldPosition())
, expirationTime(time(0) + inExpiration)
, guid(inGuid)
, radius(inRadius)
, hazardNavmeshArea(14)
, previousNavmeshArea(11)
{

}

bool Hazard::operator==(const Hazard& other) const
{
    if (!guid.IsEmpty())
    {
        return (guid == other.guid);
    }
    else if (position)
    {
        return position == other.position;
    }

    return false;
}

bool Hazard::operator<(const Hazard& other) const
{
    return radius < other.radius;
}

bool Hazard::GetPosition(PlayerbotAI* ai, WorldPosition& outPosition)
{
    const WorldObject* object = GetObject(ai);
    if (object)
    {
        outPosition = position = WorldPosition(object);
        return true;
    }
    else if(position)
    {
        outPosition = position;
        return true;
    }

    return false;
}

bool Hazard::IsExpired() const
{
    return time(0) > expirationTime;
}

bool Hazard::IsValid(PlayerbotAI* ai) const
{
    const WorldObject* object = GetObject(ai);
    return (object || position) && !IsExpired();
}

const WorldObject* Hazard::GetObject(PlayerbotAI* ai) const
{
    if (!guid.IsEmpty())
    {
        if (guid.IsUnit())
        {
            return ai->GetUnit(guid);
        }
        else if (guid.IsGameObject())
        {
            return ai->GetGameObject(guid);
        }
    }

    return nullptr;
}

void AddHazardValue::Set(Hazard hazard)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    std::list<Hazard> storedHazards = AI_VALUE(std::list<Hazard>, "stored hazards");

    // Check if the hazard already exists
    auto it = std::find(storedHazards.begin(), storedHazards.end(), hazard);
    if (it == storedHazards.end())
    {
        storedHazards.emplace_back(std::move(hazard));
        SET_AI_VALUE(std::list<Hazard>, "stored hazards", storedHazards);
    }
}

std::list<HazardPosition> HazardsValue::Calculate()
{
    AiObjectContext* context = ai->GetAiObjectContext();
    std::list<Hazard> storedHazards = AI_VALUE(std::list<Hazard>, "stored hazards");

    // Create an updated hazard positions list
    bool hazardsUpdated = false;
    std::list<HazardPosition> hazards;
    for (auto it = storedHazards.begin(); it != storedHazards.end();)
    {
        // Check if the hazard is valid
        bool validHazard = false;
        Hazard& hazard = (*it);
        if (hazard.IsValid(ai))
        {
            // Try to retrieve the hazard position
            WorldPosition hazardPosition;
            if (hazard.GetPosition(ai, hazardPosition))
            {
                hazards.emplace_back(std::make_pair(hazardPosition, hazard.GetRadius()));
                validHazard = true;
            }
        }

        // Remove the invalid hazards
        if (!validHazard)
        {
            it = storedHazards.erase(it);
            hazardsUpdated = true;
        }
        else
        {
            ++it;
        }
    }

    if (hazardsUpdated)
    {
        SET_AI_VALUE(std::list<Hazard>, "stored hazards", storedHazards);
    }

    return hazards;
}