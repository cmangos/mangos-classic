#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    enum LineOfSight
    {
        LOS_IGNORE = 0,
        LOS_STATIC = 1,
        LOS_FULL   = 2,
    };

    class AnyGameObjectInObjectRangeCheck
    {
    public:
        AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(GameObject* u);
    private:
        WorldObject const* i_obj;
        float i_range;
    };

    class GameObjectsInObjectRangeCheck
    {
    public:
        GameObjectsInObjectRangeCheck(WorldObject const* obj, float range, uint32 gameObjectID) : i_obj(obj), i_range(range), i_gameObjectID(gameObjectID) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(GameObject* u);
    private:
        WorldObject const* i_obj;
        float i_range;
        uint32 i_gameObjectID;
    };

    class NearestGameObjects : public ObjectGuidListCalculatedValue, public Qualified
	{
	public:
        NearestGameObjects(PlayerbotAI* ai, float range = sPlayerbotAIConfig.sightDistance, LineOfSight lineOfSight = LOS_FULL, std::string name = "nearest game objects") :
            ObjectGuidListCalculatedValue(ai, name), range(range), lineOfSight(lineOfSight), Qualified() {}

    protected:
        virtual std::list<ObjectGuid> Calculate();

    private:
        float range;
        LineOfSight lineOfSight;
	};

    class NearestDynamicObjects : public ObjectGuidListCalculatedValue
    {
    public:
        NearestDynamicObjects(PlayerbotAI* ai, float range = sPlayerbotAIConfig.farDistance, LineOfSight lineOfSight = LOS_FULL, std::string name = "nearest dynamic objects") :
            ObjectGuidListCalculatedValue(ai, name), range(range), lineOfSight(lineOfSight) {}

    protected:
        virtual std::list<ObjectGuid> Calculate();

    private:
        float range;
        LineOfSight lineOfSight;
    };
}
