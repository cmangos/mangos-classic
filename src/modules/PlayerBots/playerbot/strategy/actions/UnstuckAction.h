#ifndef _PLAYERBOT_UNSTUCKACTION_H
#define _PLAYERBOT_UNSTUCKACTION_H

#include "GenericActions.h"

class PlayerbotAI;

class UnstuckAction : public Action
{
public:
    UnstuckAction(PlayerbotAI* ai) : Action(ai, "unstuck") {}
    virtual bool Execute(Event& event);

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() { return "unstuck"; }
    virtual std::string GetHelpDescription()
    {
        return "This action attempts to unstuck the bot by choosing an appropriate method based on the event source.\n"
               "Methods include resetting position, using hearthstone, or repopping at the spirit healer.";
    }
    virtual std::vector<std::string> GetUsedActions() { return {"reset", "hearthstone", "repop"}; }
    virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif
};

#endif
