#pragma once
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Value.h"
#include "Groups/Group.h"
#include "TargetValue.h"

namespace ai
{
    class RtiTargetValue : public TargetValue
    {
    public:
        RtiTargetValue(PlayerbotAI* ai, std::string type = "rti", std::string name = "rti target") : type(type), TargetValue(ai,name)
        {}

    public:
        static int GetRtiIndex(std::string rti)
        {
            int index = -1;
            if(rti == "star") index = 0;
            else if(rti == "circle") index = 1;
            else if(rti == "diamond") index = 2;
            else if(rti == "triangle") index = 3;
            else if(rti == "moon") index = 4;
            else if(rti == "square") index = 5;
            else if(rti == "cross") index = 6;
            else if(rti == "skull") index = 7;
            return index;
        }

        Unit *Calculate()
        {
            Group *group = bot->GetGroup();
            if(!group)
                return NULL;

            std::string rti = AI_VALUE(std::string, type);
            int index = GetRtiIndex(rti);

            if (index == -1)
                return NULL;

            ObjectGuid guid = group->GetTargetIcon(index);
            if (!guid)
                return NULL;

            std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("possible targets")->Get();
            if (std::find(attackers.begin(), attackers.end(), guid) == attackers.end()) return NULL;

            Unit* unit = ai->GetUnit(ObjectGuid(guid));
            if (!unit || sServerFacade.UnitIsDead(unit) ||
                !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
                return NULL;

            return unit;
        }

    private:
    std::string type;
    };

    class RtiCcTargetValue : public RtiTargetValue
    {
    public:
        RtiCcTargetValue(PlayerbotAI* ai, std::string name = "rti cc target") : RtiTargetValue(ai, "rti cc", name) {}
    };
}
