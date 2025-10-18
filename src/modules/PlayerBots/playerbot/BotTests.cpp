#include "playerbot/PlayerbotAIConfig.h"
#include "BotTests.h"
#include <fstream>

using namespace ai;

void LogAnalysis::RunAnalysis()
{
    AnalysePid();
    AnalyseEvents();
    AnalyseQuests();
    AnalyseCounts();
}

void LogAnalysis::AnalysePid()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir+"activity_pid.csv", std::ifstream::in);

    std::vector<uint32> activeBots, totalBots, avgDiff;
    
    uint32 runTime, maxBots = 0, start=0;

    if (in.fail())
        return;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;



        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() < 7)
            continue;

        if (tokens.size() > 50)//Multiple servers running
            continue;
        
        if (tokens[0] == "Timestamp") //Header line
            continue;

        activeBots.push_back(stoi(tokens[6]));
        totalBots.push_back(stoi(tokens[7]));
        avgDiff.push_back(stoi(tokens[2]));
        if (tokens[0][0] == '.')
            tokens[0] = "0" + tokens[0];

        runTime = stoi(tokens[0]);

        if ((uint32)stoi(tokens[7]) > maxBots)
        {
            maxBots = (uint32)stoi(tokens[7]);
            start = avgDiff.size();
        }
    }
    while (in.good());

    if (activeBots.empty() || start == activeBots.size())
        return;

    uint32 aDiff = 0, aBots =0;
    for (uint32 i = start; i < activeBots.size(); i++)
    {
        aDiff += avgDiff[i];
        aBots += activeBots[i];
    }

    aDiff /= (activeBots.size()- start);
    aBots /= (activeBots.size()- start);

    using namespace std::chrono;
    std::chrono::milliseconds ms(runTime);
    auto secs = duration_cast<seconds>(ms);
    ms -= duration_cast<milliseconds>(secs);
    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);
    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    ss << hour.count() << " Hours : " << mins.count() << " Minutes : " << secs.count() << " Seconds ";
    
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr()  << "," << "PID" << "," << ss.str().c_str() << "," << aDiff << "," << aBots << "," << maxBots;

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());

    sLog.outString("========= LAST SERVER RUNTIME: %s", ss.str().c_str());
    sLog.outString("========= AVG DIFF [%d] & AVG ACTIVE BOTS [%d/%d]", aDiff, aBots, maxBots);
}

void LogAnalysis::AnalyseEvents()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);

    if (in.fail())
        return;

    std::map<std::string, uint32> eventCount, eventMin;

    eventMin["AcceptInvitationAction"] = 1;
    eventMin["AcceptQuestAction"] = 1;
    eventMin["AutoReleaseSpiritAction"] = 1;
    eventMin["AttackAnythingAction"] = 1;
    eventMin["AutoSetTalentsAction"] = 1;
    eventMin["BGJoinAction"] = 1;
    eventMin["CheckMountStateAction"] = 1;    
    eventMin["EquipAction"] = 1;
    eventMin["LeaveGroupAction"] = 1;
    eventMin["QueryItemUsageAction"] = 1;
    eventMin["QuestUpdateAddKillAction"] = 1;
    eventMin["ReviveFromCorpseAction"] = 1;
    eventMin["StoreLootAction"] = 1;
    eventMin["SellAction"] = 1;
    eventMin["BuyAction"] = 1;
    eventMin["AhAction"] = 1;
    eventMin["AhBidAction"] = 1;
    eventMin["MailAction"] = 1;
    eventMin["TalkToQuestGiverAction"] = 1;
    eventMin["TrainerAction"] = 1;
    eventMin["XpGainAction"] = 1;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        eventCount[tokens[2]]++;
    } while (in.good());

    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr() << "," << "EVENT";

    for (auto event : eventMin)
    {
        out << ",{" << event.first << "," << eventCount[event.first] << "}";

        if (eventCount[event.first] < event.second)
            sLog.outError("Event %s was only seen %d times", event.first.c_str(), eventCount[event.first]);
    }

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());
}

void LogAnalysis::AnalyseQuests()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);


    if (in.fail())
        return;

    std::map<std::string, uint32> questId, questStartCount, questObjective, questCompletedCount, questEndCount;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() == 10) //Some quest names have a "," so add an extra element.
        {
            tokens[7] = tokens[7] + tokens[8];
            tokens[8] = tokens[9];
        }
        else if (tokens.size() == 11)
        {
            tokens[7] = tokens[7] + tokens[8] + tokens[9];
            tokens[8] = tokens[10];
        }


        for(auto& token : tokens)
            token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());

        if (tokens[2] == "AcceptQuestAction")
        {
            questId[tokens[7]] = stoi(tokens[8]);
            questStartCount[tokens[7]]++;
        }
        else if (tokens[2] == "QueryItemUsageAction" || tokens[3] == "QuestUpdateAddKillAction")
        {
            questId[tokens[7]] = questId[tokens[7]];
            questObjective[tokens[7]]++;
            if (stof(tokens[8]) == 1)
                questCompletedCount[tokens[7]]++;
        }
        else if (tokens[2] == "TalkToQuestGiverAction")
        {
            questId[tokens[7]] = stoi(tokens[8]);
            questEndCount[tokens[7]]++;
        }
    } while (in.good());

    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr() << "," << "QUEST";

    for (auto quest : questId)
    {
        out << ",{" << quest.second << "," << quest.first << "," << questStartCount[quest.first] << "," << questObjective[quest.first] << "," << questObjective[quest.first] << ","<< questEndCount[quest.first] <<"}";

        if (!questEndCount[quest.first])
        {
            if (questCompletedCount[quest.first] > 1)
                sLog.outError("Quest %s (%d) has %d objectives completed but was never turned in.", quest.first.c_str(), quest.second, questCompletedCount[quest.first]);
            else if (questObjective[quest.first] > 1)
                sLog.outError("Quest %s (%d) has %d objectives partially completed but was never turned in.", quest.first.c_str(), quest.second, questObjective[quest.first]);
            else if (questStartCount[quest.first] > 10)
                sLog.outError("Quest %s (%d) started %d times but never turned in.", quest.first.c_str(), quest.second, questStartCount[quest.first]);
        }
    }

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());
}

void LogAnalysis::AnalyseCounts()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);

    if (in.fail())
        return;

    uint32 maxShow = 10;

    std::map<std::string, std::string> countType;
    countType["TalkToQuestGiverAction"] = "Quest completed";
    countType["EquipAction"] = "Item equiped";
    countType["StoreLootAction"] = "Item looted";
    countType["BuyAction"] = "Item bought";
    countType["SellAction"] = "Item sold";
    countType["AhAction"] = "Item sold on AH";
    countType["AhBidAction"] = "Item bid on AH";
    countType["MailAction"] = "Item bought on AH";
    countType["TrainerAction"] = "Skill learned";   
    countType["AttackAnythingAction"] = "Mob attacked";
    countType["XpGainAction"] = "Mob killed";

    std::map<std::string, std::map<std::string, uint32>> counts;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() == 10) //Some quest names have a "," so add an extra element. 
        {
            tokens[7] = tokens[7] + tokens[8];
            tokens[8] = tokens[9];
            tokens.pop_back();
        }

        for (auto& token : tokens)
            token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());

        if (countType[tokens[2]].empty())
            continue;

        if (tokens[7].empty())
            continue;

        counts[tokens[2]][tokens[7]]++;        
    } while (in.good());
  
    for (auto& count : counts)
    {

        std::vector<std::pair<std::string, uint32>> list;
        std::ostringstream out;

        out << sPlayerbotAIConfig.GetTimestampStr() << "," << "COUNT-" << count.first;

        for (auto type : count.second)
            list.push_back(make_pair(type.first, type.second));

        std::sort(list.begin(), list.end(), [](std::pair<std::string, uint32> i, std::pair<std::string, uint32> j) { return i.second > j.second; });

        for(auto& l:list)
            out << ",{" << l.first << "," << l.second << "}";

        sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());

        sLog.outString("[Top %d %s]", maxShow, countType[count.first].c_str());
        for (uint32 i = 0; i < std::min(maxShow, (uint32)list.size()); i++)
            sLog.outString("%s (%d)", list[i].first.c_str(), list[i].second);
    }
}