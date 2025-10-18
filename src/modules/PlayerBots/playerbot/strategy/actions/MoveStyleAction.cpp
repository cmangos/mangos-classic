
#include "playerbot/playerbot.h"
#include "MoveStyleAction.h"
#include "playerbot/strategy/values/SubStrategyValue.h"
#include "playerbot/strategy/values/MoveStyleValue.h"
#include "playerbot/PlayerbotAIAware.h"

using namespace ai;

bool MoveStyleAction::Execute(Event& event)
{
    std::string strategy = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    MoveStyleValue* value = (MoveStyleValue*)context->GetValue<std::string>("move style");

    if (strategy == "?")
    {
        {
            std::ostringstream out;
            out << "Move style: " << value->Get();
            ai->TellPlayer(requester, out);
        }
    }
    else
    {
        value->Set(strategy);
        
        {
            std::ostringstream out;
            out << "Move style set to: " << value->Get();
            ai->TellPlayer(requester, out);
        }
    }

    return true;
}

