#include "MemoryMonitor.h"
#include <iostream>     // std::cout, std::ios
#include <sstream>
#include <time.h>
#include "Util/Timer.h"
#include "Log/Log.h"
#define BOOST_STACKTRACE_LINK
#include <boost/stacktrace.hpp>

void MemoryMonitor::Add(std::string objectType, uint64_t object, int level, std::string stack)
{
    if(stack.empty())
        objectnumbers[std::this_thread::get_id()][objectType]++; 

    if (level && (int)object % level == 0)
    {
        std::ostringstream out;
        if (stack.empty())
            out << boost::stacktrace::stacktrace();
        else
            out << stack;
        adds[std::this_thread::get_id()][objectType][object] = make_pair(out.str(), time(0));
    }
}

void MemoryMonitor::Rem(std::string objectType, uint64_t object, int level)
{
    objectnumbers[std::this_thread::get_id()][objectType]--; 
    
    if (level && (int)object % level == 0)
    {
        if (adds[std::this_thread::get_id()][objectType].find(object) != adds[std::this_thread::get_id()][objectType].end())
            adds[std::this_thread::get_id()][objectType].erase(object);
        else
            rems[std::this_thread::get_id()][objectType][object] = std::make_pair("rem", time(0));
    }
}

void MemoryMonitor::Print()
{
    Browse();

    std::map<std::string, int> nums;

    for (auto& t : objectnumbers) // Aggregate object counts over all threads
        for (auto& object : t.second)
            nums[object.first]+=object.second;

    objectnumbersHist.push_back(nums);

    if (objectnumbersHist.size() > 10)
        objectnumbersHist.pop_front();

    for (auto& num : nums)
    {
        if(num.second > 1000000)
            sLog.outString("%s : %.2fm (%.2fk)", num.first.c_str(), num.second / 1000000.0f, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]) / 1000.0f);            
        else if(num.second > 1000)
            sLog.outString("%s : %.2fk (%.2fk)", num.first.c_str(), num.second / 1000.0f, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]) / 1000.0f);
        else
            sLog.outString("%s : %d (%d)", num.first.c_str(), num.second, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]));
    }
}

void MemoryMonitor::LogCount(std::string filename)
{
    FILE* file = fopen(filename.c_str(), headers ? "a" : "w");

    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    std::string timestamp = std::string(buf);

    std::map<std::string, int> nums;

    for (auto& t : objectnumbers) // Aggregate object counts over all threads
        for (auto& object : t.second)
            nums[object.first] += object.second;

    objectnumbersHist.push_back(nums);

    if (objectnumbersHist.size() > 10)
        objectnumbersHist.pop_front();

    std::string line = "time";

    if (!headers)
    {
        for (auto& num : nums)
        {
            line += "," + num.first;
        }

        fprintf(file, "%s", line.c_str());
        fprintf(file, "\n");
        fflush(file);

        fflush(stdout);
        headers = true;
    }
        
    //line = timestamp.c_str();
    line = std::to_string(static_cast<uint32>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch() - GetApplicationStartTime().time_since_epoch()).count()));

    for (auto& num : nums)
    {
        line += "," + std::to_string(num.second);
    }

    fprintf(file, "%s", line.c_str());
    fprintf(file, "\n");
    fflush(file);
    fflush(stdout);

    fclose(file);
}

void MemoryMonitor::Browse()
{
    std::unordered_map<std::string, std::unordered_map<std::string, double>> here;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<time_t>>> hereTime;
    std::unordered_map<std::string, std::vector<double>> notHere;

    for (auto& thre : adds) //loop over all threads
        for(auto typ : thre.second) //loop over all types
        {
            for (auto& a : typ.second) //loop over all additions
            {
                bool stillHere = true;
                for (auto& rem : rems) //loop over all threads of removed items.
                {
                    for (auto& r : rem.second[typ.first]) //loop over all items of the same type.
                        if (r.first == a.first) 
                        {
                            stillHere = false;
                            break;
                        }

                    if (!stillHere)
                        break;
                }

                if (stillHere) //Item was not removed somewhere, report it.
                {
                    here[typ.first][a.second.first]++;
                    hereTime[typ.first][a.second.first].push_back(a.second.second);
                }
                else //Item was removed. Clean it up.
                    notHere[typ.first].push_back(a.first);
            }           
        }

    //Clean up removed items.
    for (auto& thre : adds) 
    {
        for (auto& typ : thre.second)
        {
            for (auto& nh : notHere[typ.first])
            {
                adds[thre.first][typ.first].erase(nh);
                rems[thre.first][typ.first].erase(nh);
            }
        }
    }

    for (auto& t : here)
    {
        sLog.outError("[%s] = %zu", t.first.c_str(), t.second.size());

        std::vector<std::pair<std::string, int>> top(10);
        std::partial_sort_copy(here[t.first].begin(),
            here[t.first].end(),
            top.begin(),
            top.end(),
            [](std::pair<const std::string, int> const& l,
                std::pair<const std::string, int> const& r)
            {
                return l.second > r.second;
            });

        for (auto& p : top)
        {
            if (hereTime[t.first][p.first].empty())
                continue;

            uint32 f, l, a = 0;

            f = time(0) - hereTime[t.first][p.first].front();
            l = time(0) - hereTime[t.first][p.first].back();

            for (auto p : hereTime[t.first][p.first])
                a += (time(0) - p);

            if (hereTime[t.first][p.first].size())
                a /= hereTime[t.first][p.first].size();

            sLog.outError("<%d> \nfrom: %d\navg: %d\nto: %d\n%s", p.second, f, a, l, p.first.c_str());
        }
    }
}