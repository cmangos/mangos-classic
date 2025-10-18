#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/TravelNode.h"

namespace ai
{
    class LastMovement
    {
    public:
        LastMovement()
        {
            clear();
        }

        LastMovement(LastMovement& other)
        {
            taxiNodes = other.taxiNodes;
            taxiMaster = other.taxiMaster;
            lastFollow = other.lastFollow;
            lastAreaTrigger = other.lastAreaTrigger;
            lastMoveShort = other.lastMoveShort;
            lastMoveShortStart = other.lastMoveShortStart;
            lastPath = other.lastPath;
            nextTeleport = other.nextTeleport;
            /*
            lastMoveToMapId = other.lastMoveToMapId;
            lastMoveToX = other.lastMoveToX;
            lastMoveToY = other.lastMoveToY;
            lastMoveToZ = other.lastMoveToZ;
            lastMoveToOri = other.lastMoveToOri;
            */
        }

        void clear()
        {
            lastMoveShort = WorldPosition();
            lastMoveShortStart = WorldPosition();
            lastPath.clear();
            /*
            lastMoveToMapId = 0;
            lastMoveToX = 0;
            lastMoveToY = 0;
            lastMoveToZ = 0;
            lastMoveToOri = 0;
            */
            lastFollow = NULL;
            lastAreaTrigger = 0;
            lastFlee = 0;
            nextTeleport = 0;
        }

        void Set(Unit* lastFollow)
        {
            //Set(0, 0.0f, 0.0f, 0.0f, 0.0f);
            setShort(WorldPosition(),WorldPosition());
            setPath(TravelPath());
            this->lastFollow = lastFollow;
        }

        /*
        void Set(uint32 mapId, float x, float y, float z, float ori)
        {
            lastMoveToMapId = mapId;
            lastMoveToX = x;
            lastMoveToY = y;
            lastMoveToZ = z;
            lastMoveToOri = ori;
            lastMoveShort = WorldPosition(mapId, x, y, z, ori);
            lastFollow = NULL;
        }
        */

        void setShort(WorldPosition start, WorldPosition end) {
            lastMoveShortStart = start; lastMoveShort = end; lastFollow = NULL;
        }
        void setPath(TravelPath path) { lastPath = path; }

        LastMovement& operator=(const LastMovement& other) { 
            taxiNodes = other.taxiNodes;
            taxiMaster = other.taxiMaster;
            lastFollow = other.lastFollow;
            lastAreaTrigger = other.lastAreaTrigger;
            lastMoveShort = other.lastMoveShort;
            lastMoveShortStart = other.lastMoveShortStart;
            lastPath = other.lastPath;
            nextTeleport = other.nextTeleport;

            return *this;
        };
    public:
        std::vector<uint32> taxiNodes;
        ObjectGuid taxiMaster;
        Unit* lastFollow;
        uint32 lastAreaTrigger;
        time_t lastFlee;
        //uint32 lastMoveToMapId;
        //float lastMoveToX, lastMoveToY, lastMoveToZ, lastMoveToOri;
        WorldPosition lastMoveShortStart;
        WorldPosition lastMoveShort;
        TravelPath lastPath;
        time_t nextTeleport;
    };

    class LastMovementValue : public ManualSetValue<LastMovement&>
	{
	public:
        LastMovementValue(PlayerbotAI* ai) : ManualSetValue<LastMovement&>(ai, data) {}

    private:
        LastMovement data = LastMovement();
    };

    class StayTimeValue : public ManualSetValue<time_t>
	{
	public:
        StayTimeValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, 0) {}
    };

    class LastLongMoveValue : public CalculatedValue<WorldPosition>
    {
    public:
        LastLongMoveValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "last long move", 30) {}

        WorldPosition Calculate();
    };


    class HomeBindValue : public CalculatedValue<WorldPosition>
    {
    public:
        HomeBindValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "home bind", 30) {}

        WorldPosition Calculate();

        virtual std::string Format();
    };
}
