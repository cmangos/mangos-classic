#pragma once
#include "playerbot/strategy/Trigger.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/RtiTargetValue.h"

namespace ai
{
    class NoRtiTrigger : public Trigger
    {
    public:
        NoRtiTrigger(PlayerbotAI* ai) : Trigger(ai, "no rti target") {}

        virtual bool IsActive()
		{
            if (AI_VALUE(Unit*, "rti target"))
            {
                return false;
            }
            else
            {
                // Check for the default rti if the bot is setup to ignore rti targets
                std::string rti = AI_VALUE(std::string, "rti");
                if (rti == "none")
                {
                    Group* group = bot->GetGroup();
                    if (group)
                    {
                        const ObjectGuid guid = group->GetTargetIcon(RtiTargetValue::GetRtiIndex("skull"));
                        if (!guid.IsEmpty())
                        {
                            Unit* unit = ai->GetUnit(ObjectGuid(guid));
                            if (unit && !sServerFacade.UnitIsDead(unit) && bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            return true;
        }
    };
}
