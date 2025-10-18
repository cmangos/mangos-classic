
#include "playerbot/playerbot.h"
#include "MoveStyleValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

bool MoveStyleValue::HasValue(PlayerbotAI* ai, const std::string& value)
{
    std::string styles = ai->GetAiObjectContext()->GetValue<std::string>("move style")->Get();
    return styles.find(value) != std::string::npos;
}