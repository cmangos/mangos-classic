
#include "playerbot/playerbot.h"
#include "CustomStrategy.h"
#include <regex>

using namespace ai;

std::map<std::string, std::string> CustomStrategy::actionLinesCache;

NextAction* toNextAction(std::string action)
{
    std::vector<std::string> tokens = split(action, '!');
    if (tokens.size() == 2 && !tokens[0].empty())
        return new NextAction(tokens[0], atof(tokens[1].c_str()));
    else if (tokens.size() == 1 && !tokens[0].empty())
        return new NextAction(tokens[0], ACTION_NORMAL);

    sLog.outError("Invalid action '%s'", action.c_str());
    return NULL;
}

NextAction** toNextActionArray(std::string actions)
{
    std::vector<std::string> tokens;
    tokens.push_back("");
    int stack = 0;
    for (char a : actions)
    {
       if (a == '{') ++stack;
       if (a == '}') --stack;
       if (a == ',' && stack == 0) tokens.push_back("");
       tokens.back() += a;
    }

    NextAction** res = new NextAction*[tokens.size() + 1];
    int index = 0;
    for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        NextAction* na = toNextAction(*i);
        if (na)
            res[index++] = na;
    }
	res[index++] = NULL;
    return res;
}

TriggerNode* toTriggerNode(std::string actionLine)
{
    std::vector<std::string> tokens = split(actionLine, '>');
    if (tokens.size() == 2)
        return new TriggerNode(tokens[0], toNextActionArray(tokens[1]));

    sLog.outError("Invalid action line '%s'", actionLine.c_str());
    return NULL;
}

void CustomStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    if (actionLines.empty())
    {
        if (actionLinesCache[qualifier].empty())
        {
            LoadActionLines((uint32)ai->GetBot()->GetGUIDLow());
            if (this->actionLines.empty())
                LoadActionLines(0);
        }
        else
        {
            std::vector<std::string> tokens = split(actionLinesCache[qualifier], '\n');
            std::regex tpl("\\(NULL,\\s*'.+',\\s*'(.+)'\\)(,|;)");
            for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
            {
                std::string line = *i;
                for (std::sregex_iterator j = std::sregex_iterator(line.begin(), line.end(), tpl); j != std::sregex_iterator(); ++j)
                {
                    std::smatch match = *j;
                    std::string actionLine = match[1].str();
                    if (!actionLine.empty())
                        this->actionLines.push_back(actionLine);
                }
            }
        }
    }

    for (std::list<std::string>::iterator i = actionLines.begin(); i != actionLines.end(); ++i)
    {
        TriggerNode* tn = toTriggerNode(*i);
        if (tn)
            triggers.push_back(tn);
    }
}

void CustomStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void CustomStrategy::LoadActionLines(uint32 owner)
{
    auto results = CharacterDatabase.PQuery("SELECT action_line FROM ai_playerbot_custom_strategy WHERE name = '%s' and owner = '%u' order by idx",
            qualifier.c_str(), owner);
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            std::string action = fields[0].GetString();
            this->actionLines.push_back(action);
        } while (results->NextRow());
    }
}

void CustomStrategy::Reset()
{
    actionLines.clear();
    actionLinesCache[qualifier].clear();
}