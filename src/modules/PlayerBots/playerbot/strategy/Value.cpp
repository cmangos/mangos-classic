
#include "playerbot/playerbot.h"
#include "Value.h"
#include "playerbot/PerformanceMonitor.h"
#include "playerbot/ChatHelper.h"

using namespace ai;

std::string ObjectGuidCalculatedValue::Format()
{
    GuidPosition guid = GuidPosition(this->Calculate(), bot);
    return guid ? chat->formatGuidPosition(guid, bot) : "<none>";
}

std::string ObjectGuidListCalculatedValue::Format()
{
    std::ostringstream out; out << "{";
    std::list<ObjectGuid> guids = this->Calculate();
    for (std::list<ObjectGuid>::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        GuidPosition guid = GuidPosition(*i, bot);
        out << chat->formatGuidPosition(guid,bot) << ",";
    }
    out << "}";
    return out.str();
}

std::string GuidPositionCalculatedValue::Format()
{
    std::ostringstream out;
    GuidPosition guidP = this->Calculate();
    return chat->formatGuidPosition(guidP,bot);
}

std::string GuidPositionListCalculatedValue::Format()
{
    std::ostringstream out; out << "{";
    std::list<GuidPosition> guids = this->Calculate();
    for (std::list<GuidPosition>::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        GuidPosition guidP = *i;
        out << chat->formatGuidPosition(guidP,bot) << ",";
    }
    out << "}";
    return out.str();
}

std::string GuidPositionManualSetValue::Format()
{
    return chat->formatGuidPosition(value,bot);
}
