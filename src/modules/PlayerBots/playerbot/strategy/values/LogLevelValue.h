#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class LogLevelValue : public ManualSetValue<LogLevel>
	{
	public:
        LogLevelValue(PlayerbotAI* ai, std::string name = "log level") :
            ManualSetValue<LogLevel>(ai, LOG_LVL_DEBUG, name) {}
	};
}
