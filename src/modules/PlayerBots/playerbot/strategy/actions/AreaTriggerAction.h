#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class ReachAreaTriggerAction : public MovementAction {
    public:
        ReachAreaTriggerAction(PlayerbotAI* ai) : MovementAction(ai, "reach area trigger") {}

        virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "reach area trigger"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot move towards a specific area trigger, often an instance portal.\n"
                   "These portals allow the bot to teleport into or out of an instance when reached, if it's not too far away.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"last area trigger"}; }
#endif 
    };

    class AreaTriggerAction : public MovementAction {
    public:
        AreaTriggerAction(PlayerbotAI* ai) : MovementAction(ai, "area trigger") {}

        virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "area trigger"; }
        virtual std::string GetHelpDescription()
        {
            return "This action allows the bot to interact with an area trigger, typically an instance portal, once it has reached the location.\n"
                   "This interaction enables the bot to teleport into or out of an instance.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"last area trigger"}; }
#endif 
    };

}
