
#include "playerbot/playerbot.h"
#include "LogLevelAction.h"

using namespace ai;

bool LogLevelAction::Execute(Event& event)
{
    std::string param = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Value<LogLevel> *value = ai->GetAiObjectContext()->GetValue<LogLevel>("log level");

    std::ostringstream out; 
    if (param != "?")
    {
        value->Set(string2logLevel(param));
        out << "My log level set to " << logLevel2string(value->Get());
    }
    else
    {
        out << "My log level is " << logLevel2string(value->Get());
    }
    ai->TellPlayer(requester, out);
    return true;    
}

std::string LogLevelAction::logLevel2string(LogLevel level)
{
    switch (level) 
    {
    case LOG_LVL_BASIC:
        return "basic";
    case LOG_LVL_MINIMAL:
        return "minimal";
    case LOG_LVL_DETAIL:
        return "detail";
    default:
        return "debug";
    }
}
LogLevel LogLevelAction::string2logLevel(std::string level)
{
    if (level == "debug")
        return LOG_LVL_DEBUG;
    else if (level == "minimal")
        return LOG_LVL_MINIMAL;
    else if (level == "detail")
        return LOG_LVL_DETAIL;
    else 
        return LOG_LVL_BASIC;
}
