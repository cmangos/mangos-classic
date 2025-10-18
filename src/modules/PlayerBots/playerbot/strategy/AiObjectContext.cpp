
#include "playerbot/playerbot.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "StrategyContext.h"
#include "triggers/TriggerContext.h"
#include "actions/ActionContext.h"
#include "triggers/ChatTriggerContext.h"
#include "actions/ChatActionContext.h"
#include "triggers/WorldPacketTriggerContext.h"
#include "actions/WorldPacketActionContext.h"
#include "values/ValueContext.h"
#include "values/SharedValueContext.h"


using namespace ai;

AiObjectContext::AiObjectContext(PlayerbotAI* ai) : PlayerbotAIAware(ai)
{
    strategyContexts.Add(new StrategyContext());
    strategyContexts.Add(new MovementStrategyContext());
    strategyContexts.Add(new AssistStrategyContext());
    strategyContexts.Add(new QuestStrategyContext());
    strategyContexts.Add(new FishStrategyContext());

    actionContexts.Add(new ActionContext());
    actionContexts.Add(new ChatActionContext());
    actionContexts.Add(new WorldPacketActionContext());

    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());

    valueContexts.Add(new ValueContext());

    //valueContexts.Add(&sSharedValueContext);
}

void AiObjectContext::ClearValues(std::string findName)
{
    std::set<std::string> names = valueContexts.GetCreated();
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        if (!findName.empty() && i->find(findName) == std::string::npos)
            continue;

        valueContexts.Erase(*i);
    }
}

void AiObjectContext::ClearExpiredValues(std::string findName, uint32 interval)
{
    std::vector<std::string> namesToErase;
    std::set<std::string> names = valueContexts.GetCreated();

    for (const auto& name : names)
    {
        UntypedValue* value = GetUntypedValue(name);
        if (!value || value->Protected())
            continue;

        if (!findName.empty() && name.find(findName) == std::string::npos)
            continue;

        if ((!interval && !value->Expired()) || (interval && !value->Expired(interval)))
            continue;

        namesToErase.push_back(name);
    }

    for (const auto& name : namesToErase)
    {
        valueContexts.Erase(name);
    }
}


std::string AiObjectContext::FormatValues(std::string findName)
{
    std::ostringstream out;
    std::set<std::string> names = valueContexts.GetCreated();
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        if (!findName.empty() && i->find(findName) == std::string::npos)
            continue;

        std::string text = value->Format();
        if (text == "?")
            continue;

        out << "{" << *i << "=" << text << "}|";
    }
    out.seekp(-1, out.cur);
    return out.str();
}

void AiObjectContext::Update()
{
    /* Disabled until there is actually a strategy, trigger, action or value that has the Update() method. Currently this takes 8% cpu and does 'NOTHING'.
    strategyContexts.Update();
    triggerContexts.Update();
    actionContexts.Update();
    valueContexts.Update();
    */
}

void AiObjectContext::Reset()
{
    strategyContexts.Reset();
    triggerContexts.Reset();
    actionContexts.Reset();
    valueContexts.Reset();
}

std::list<std::string> AiObjectContext::Save()
{
    std::list<std::string> result;

    std::set<std::string> names = valueContexts.GetCreated();
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        std::string data = value->Save();
        if (data == "?")
            continue;

        std::string name = *i;
        std::ostringstream out;
        out << name;

        out << ">" << data;
        result.push_back(out.str());
    }
    return result;
}

void AiObjectContext::Load(std::list<std::string> data)
{
    for (std::list<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        std::string row = *i;
        std::vector<std::string> parts = split(row, '>');
        if (parts.empty() || parts.size() > 2) continue;

        std::string name = parts[0];
        std::string text = (parts.size() == 2) ? parts[1] : "";

        UntypedValue* value = GetUntypedValue(name);
        if (!value) continue;

        value->Load(text);
    }
}
