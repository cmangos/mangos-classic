#pragma once

#include "playerbot/PlayerbotAIAware.h"
#include "Action.h"
#include "Value.h"
#include "NamedObjectContext.h"
#include "Strategy.h"
#include <set>

namespace ai
{
    class UntypedValue;
    template<class T> class Value;
}

namespace ai
{
    class AiObjectContext : public PlayerbotAIAware
    {
    public:
        AiObjectContext(PlayerbotAI* ai);
        virtual ~AiObjectContext() {}

    public:
        virtual Strategy* GetStrategy(const std::string& name) { return strategyContexts.GetObject(name, ai); }
        virtual std::set<std::string> GetSiblingStrategy(const std::string& name) { return strategyContexts.GetSiblings(name); }
        virtual Trigger* GetTrigger(const std::string& name) { return triggerContexts.GetObject(name, ai); }
        virtual Action* GetAction(const std::string& name) { return actionContexts.GetObject(name, ai); }
        virtual UntypedValue* GetUntypedValue(const std::string& name) { return valueContexts.GetObject(name, ai); }

        template<class T>
        Value<T>* GetValue(const std::string& name)
        {
            return dynamic_cast<Value<T>*>(GetUntypedValue(name));
        }

        template<class T>
        Value<T>* GetValue(const std::string& name, const std::string& param)
        {
            return GetValue<T>((std::string(name) + "::" + param));
        }

        template<class T>
        Value<T>* GetValue(const std::string& name, int32 param)
        {
        	std::ostringstream out; out << param;
            return GetValue<T>(name, out.str());
        }

        bool HasValue(const std::string& name)
        {
            return valueContexts.IsCreated(name);
        }

        bool HasValue(const std::string& name, const std::string& param)
        {
            return HasValue((std::string(name) + "::" + param));
        }

        bool HasValue(const std::string& name, int32 param)
        {
            std::ostringstream out; out << param;
            return HasValue(name, out.str());
        }


        std::set<std::string> GetValues()
        {
            return valueContexts.GetCreated();
        }

        std::set<std::string> GetSupportedStrategies()
        {
            return strategyContexts.supports();
        }

        std::set<std::string> GetSupportedTriggers()
        {
            return triggerContexts.supports();
        }

        std::set<std::string> GetSupportedActions()
        {
            return actionContexts.supports();
        }

        std::set<std::string> GetSupportedValues ()
        {
            return valueContexts.supports();
        }

        void ClearValues(std::string findName = "");

        void ClearExpiredValues(std::string findName = "", uint32 interval = 0);

        std::string FormatValues(std::string findName = "");

    public:
        virtual void Update();
        virtual void Reset();
        virtual void AddShared(NamedObjectContext<UntypedValue>* sharedValues)
        {
            valueContexts.Add(sharedValues);
        }
        std::list<std::string> Save();
        void Load(std::list<std::string> data);

        std::vector<std::string> performanceStack;
    protected:
        NamedObjectContextList<Strategy> strategyContexts;
        NamedObjectContextList<Action> actionContexts;
        NamedObjectContextList<Trigger> triggerContexts;
        NamedObjectContextList<UntypedValue> valueContexts;
    };
}


#define AI_VALUE(type, name) context->GetValue<type>(name)->Get()
#define AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Get()

#define AI_VALUE_SAFE(type, name) context->GetValue<type>(name) ? context->GetValue<type>(name)->Get() : type()
#define AI_VALUE2_SAFE(type, name, param) context->GetValue<type>(name, param) ? context->GetValue<type>(name, param)->Get() : type()

#define AI_VALUE_LAZY(type, name) context->GetValue<type>(name)->LazyGet()
#define AI_VALUE2_LAZY(type, name, param) context->GetValue<type>(name, param)->LazyGet()

#define HAS_AI_VALUE(name) context->HasValue(name)
#define HAS_AI_VALUE2(name, param) context->HasValue(name, param)
#define AI_VALUE_EXISTS(type, name, emptyval) (HAS_AI_VALUE(name) ? AI_VALUE(type, name) : emptyval)
#define AI_VALUE2_EXISTS(type, name, param, emptyval) (HAS_AI_VALUE2(name, param) ? AI_VALUE2(type, name, param) : emptyval)

#define SET_AI_VALUE(type, name, value) context->GetValue<type>(name)->Set(value)
#define SET_AI_VALUE2(type, name, param, value) context->GetValue<type>(name, param)->Set(value)
#define RESET_AI_VALUE(type, name) context->GetValue<type>(name)->Reset()
#define RESET_AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Reset()

#define PAI_VALUE(type, name) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name)->Get()
#define PAI_VALUE2(type, name, param) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name, param)->Get()
#define SET_PAI_VALUE(type, name, value) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name)->Set(value)
#define SET_PAI_VALUE2(type, name, param, value) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name, param)->Set(value)
#define PHAS_AI_VALUE(name) player->GetPlayerbotAI()->GetAiObjectContext()->HasValue(name)
#define PHAS_AI_VALUE2(name, param) player->GetPlayerbotAI()->GetAiObjectContext()->HasValue(name, param)
#define MAI_VALUE(type, name) master->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name)->Get()
#define MAI_VALUE2(type, name, param) master->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name, param)->Get()

#define GAI_VALUE(type, name) sSharedObjectContext.GetValue<type>(name)->Get()
#define GAI_VALUE2(type, name, param) sSharedObjectContext.GetValue<type>(name, param)->Get()
#define SET_GAI_VALUE(type, name, value) sSharedObjectContext.GetValue<type>(name)->Set(value)
#define SET_GAI_VALUE2(type, name, param, value) sSharedObjectContext.GetValue<type>(name, param)->Set(value)

#define MEM_AI_VALUE(type, name) dynamic_cast<MemoryCalculatedValue<type>*>(context->GetUntypedValue(name))
#define LOG_AI_VALUE(type, name) dynamic_cast<LogCalculatedValue<type>*>(context->GetUntypedValue(name))