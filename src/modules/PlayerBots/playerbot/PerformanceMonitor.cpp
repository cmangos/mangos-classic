#include "playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "PerformanceMonitor.h"

#include "Database/DatabaseEnv.h"
#include "PlayerbotAI.h"

PerformanceMonitor::PerformanceMonitor() 
{
}

PerformanceMonitor::~PerformanceMonitor()
{
}

std::unique_ptr<PerformanceMonitorOperation> PerformanceMonitor::start(PerformanceMetric metric, std::string name, PerformanceStack* stack)
{
   if (!sPlayerbotAIConfig.perfMonEnabled)
   {
      return { };
   }

    std::string stackName = name;

    if (stack)
    {
        if (!stack->empty())
        {
            std::ostringstream out; out << stackName << " [";
            for (std::vector<std::string>::reverse_iterator i = stack->rbegin(); i != stack->rend(); ++i)
                out << *i << (std::next(i)==stack->rend()? "":"|");
            out << "]";
            stackName = out.str().c_str();
        }
        stack->push_back(name);
    }

#ifdef CMANGOS
	std::lock_guard<std::mutex> guard(lock);
    PerformanceData *pd = data[metric][stackName];
    if (!pd)
    {
        pd = new PerformanceData();
        pd->minTime = pd->maxTime = pd->totalTime = pd->count = 0;
        data[metric][stackName] = pd;
    }

	return std::make_unique<PerformanceMonitorOperation>(pd, name, stack);
#endif
}

std::unique_ptr<PerformanceMonitorOperation> PerformanceMonitor::start(PerformanceMetric metric, std::string name, PlayerbotAI * ai)
{
    if (!sPlayerbotAIConfig.perfMonEnabled) return NULL;

    if (ai->GetAiObjectContext())
        return start(metric, name, &ai->GetAiObjectContext()->performanceStack);
    else
        return start(metric, name);
}

void PerformanceMonitor::PrintStats(bool perTick, bool fullStack)
{
    if (data.empty())
        return;

    uint32 total = 0;

    if (!perTick)
    {

        for (auto& map : data[PERF_MON_TOTAL])
            if (map.first == "PlayerbotAI::UpdateAI")
                total += map.second->totalTime;

        sLog.outString("--------------------------------------[TOTAL BOT]------------------------------------------------------");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*> >::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string key;
            switch (i->first)
            {
            case PERF_MON_TRIGGER: key = "T"; break;
            case PERF_MON_VALUE: key = "V"; break;
            case PERF_MON_ACTION: key = "A"; break;
            case PERF_MON_RNDBOT: key = "RndBot"; break;
            case PERF_MON_TOTAL: key = "Total"; break;
            default: key = "?";
            }

            std::list<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                if (key == "Total" && j->first.find("PlayerbotAI") == std::string::npos)
                    continue;
                names.push_back(j->first);
            }

            names.sort([pdMap](std::string i, std::string j) {return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime; });

            float tPerc = 0;
            uint32 tMin = 99999, tMax = 0, tCount = 0, tTime = 0;

            sLog.outString("percentage   time    |   min  ..    max (     avg  of     count ) - type : name                        ");

            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;
                float secs = (float)pd->totalTime / 1000.0f;
                float avg = (float)pd->totalTime / (float)pd->count;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";

                if (perc > 0.1) //(avg >= 0.5f || pd->maxTime > 10)
                {
                    sLog.outString("%7.3f%% %10.3fs | %6u .. %6u (%9.4f of %10u) - %s    : %s"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , pd->count
                        , key.c_str()
                        , disName.c_str());
                }

                bool countTot = false;

                if (i->first == PERF_MON_VALUE)
                {
                    if (name.find("<") == std::string::npos)
                        countTot = count(name.begin(), name.end(), '|') < 1;
                    else
                        countTot = name.find("|") == std::string::npos;
                }
                else
                    countTot = name.find("|") == std::string::npos;

                if (countTot)
                {
                    tPerc += perc;
                    tMin = pd->minTime < tMin ? pd->minTime : tMin;
                    tMax = pd->maxTime > tMax ? pd->maxTime : tMax;
                    tCount += pd->count;
                    tTime += pd->totalTime;
                }
            }

            if (i->first != PERF_MON_TOTAL)
                sLog.outString("%7.3f%% %10.3fs | %6u .. %6u (%9.4d of %10u) - %s    : %s"
                    , tPerc
                    , tTime / 1000.0f
                    , tMin
                    , tMax
                    , tTime / tCount
                    , tCount
                    , key.c_str()
                    , "TOTAL");
            sLog.outString(" ");
        }

    }
    else
    {
        float totalCount = data[PERF_MON_TOTAL]["PlayerbotAIBase::FullTick"]->count;
        total = data[PERF_MON_TOTAL]["PlayerbotAIBase::FullTick"]->totalTime;

        sLog.outString(" ");
        sLog.outString(" ");
        sLog.outString("---------------------------------------[PER TICK]------------------------------------------------------");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*> >::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string key;
            switch (i->first)
            {
            case PERF_MON_TRIGGER: key = "T"; break;
            case PERF_MON_VALUE: key = "V"; break;
            case PERF_MON_ACTION: key = "A"; break;
            case PERF_MON_RNDBOT: key = "RndBot"; break;
            case PERF_MON_TOTAL: key = "Total"; break;
            default: key = "?";
            }

            std::list<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                names.push_back(j->first);
            }

            names.sort([pdMap](std::string i, std::string j) {return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime; });

            float tPerc = 0, tCount = 0;
            uint32 tMin = 99999, tMax = 0, tTime = 0;

            sLog.outString("percentage   time    |   min  ..    max (     avg  of     count ) - type : name                        ");

            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;
                uint32 secs = pd->totalTime / totalCount;
                float avg = (float)pd->totalTime / (float)pd->count;
                float amount = (float)pd->count / (float)totalCount;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";

                if (perc > 0.1) //(avg >= 0.5f || pd->maxTime > 10)
                {
                    sLog.outString("%7.3f%% %9ums | %6u .. %6u (%9.4f of %10.2f) - %s    : %s"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , amount
                        , key.c_str()
                        , disName.c_str());
                }

                bool countTot = false;

                if (i->first == PERF_MON_VALUE)
                {
                    if (name.find("<") == std::string::npos)
                        countTot = count(name.begin(), name.end(), '|') < 1;
                    else
                        countTot = name.find("|") == std::string::npos;
                }
                else
                    countTot = name.find("|") == std::string::npos;

                if (countTot)
                {
                    tPerc += perc;
                    tMin = pd->minTime < tMin ? pd->minTime : tMin;
                    tMax = pd->maxTime > tMax ? pd->maxTime : tMax;
                    tCount += amount;
                    tTime += pd->totalTime;
                }
            }

            uint32 secs = tTime / totalCount;
            float avg = tTime / tCount;

            if (i->first != PERF_MON_TOTAL)
                sLog.outString("%7.3f%% %9ums | %6u .. %6u (%9.2f of %10.2f) - %s    : %s"
                    , tPerc
                    , secs
                    , tMin
                    , tMax
                    , avg
                    , tCount
                    , key.c_str()
                    , "TOTAL");

            sLog.outString(" ");
        }
    }

}

void PerformanceMonitor::Reset()
{
   for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*> >::iterator i = data.begin(); i != data.end(); ++i)
   {
      std::map<std::string, PerformanceData*> pdMap = i->second;
      for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
      {
#ifdef CMANGOS
         PerformanceData* pd = j->second;
         std::lock_guard<std::mutex> guard(pd->lock);
         pd->minTime = pd->maxTime = pd->totalTime = pd->count = 0;
#endif
      }
   }
}

PerformanceMonitorOperation::PerformanceMonitorOperation(PerformanceData* data, std::string name, PerformanceStack* stack) : data(data), name(name), stack(stack)
{
#ifdef CMANGOS
    started = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
#endif
}

PerformanceMonitorOperation::~PerformanceMonitorOperation()
{
   finish();
}

void PerformanceMonitorOperation::finish()
{
#ifdef CMANGOS
    std::chrono::milliseconds finished = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
    uint32 elapsed = (finished - started).count();

    std::lock_guard<std::mutex> guard(data->lock);
    if (elapsed > 0)
    {
        if (!data->minTime || data->minTime > elapsed) data->minTime = elapsed;
        if (!data->maxTime || data->maxTime < elapsed) data->maxTime = elapsed;
        data->totalTime += elapsed;
    }
    data->count++;
#endif

    if (stack)
    {
        stack->erase(std::remove(stack->begin(), stack->end(), name), stack->end());
    }
}

bool ChatHandler::HandlePerfMonCommand(char* args)
{
    if (!strcmp(args, "reset"))
    {
        sPerformanceMonitor.Reset();
        sLog.outString("Performance monitor reset");
        return true;
    }

    if (!strcmp(args, "tick"))
    {
        sPerformanceMonitor.PrintStats(true,false);
        return true;
    }

    if (!strcmp(args, "stack"))
    {
        sPerformanceMonitor.PrintStats(false,true);
        return true;
    }

    if (!strcmp(args, "full"))
    {
        sPerformanceMonitor.PrintStats(true, true);
        return true;
    }

    if (!strcmp(args, "toggle"))
    {
        sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
        if(sPlayerbotAIConfig.perfMonEnabled)
            sLog.outString("Performance monitor enabled");
        else
            sLog.outString("Performance monitor disabled");
        return true;
    }   

    sPerformanceMonitor.PrintStats();
    return true;
}