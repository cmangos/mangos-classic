#pragma once

#include "PvpValues.h"
#include "QuestValues.h"
#include "TrainerValues.h"
#include "VendorValues.h"
#include "TravelValues.h"
#include "LootValues.h"
#include "MountValues.h"
#include "playerbot/PlayerbotAI.h"

namespace ai
{
    class SharedValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        SharedValueContext() : NamedObjectContext(true)
        {
            creators["bg masters"] = [](PlayerbotAI* ai) { return new BgMastersValue(ai); };

            creators["item drop map"] = [](PlayerbotAI* ai) { return new ItemDropMapValue(ai); };
            creators["drop map"] = [](PlayerbotAI* ai) { return new DropMapValue(ai); };
            creators["item drop list"] = [](PlayerbotAI* ai) { return new ItemDropListValue(ai); };
            creators["entry loot list"] = [](PlayerbotAI* ai) { return new EntryLootListValue(ai); };
            creators["loot chance"] = [](PlayerbotAI* ai) { return new LootChanceValue(ai); };

            creators["vendor map"] = [](PlayerbotAI* ai) { return new VendorMapValue(ai); };
            creators["item vendor list"] = [](PlayerbotAI* ai) { return new ItemVendorListValue(ai); };

            creators["entry quest relation"] = [](PlayerbotAI* ai) { return new EntryQuestRelationMapValue(ai); };

            creators["quest guidp map"] = [](PlayerbotAI* ai) { return new QuestGuidpMapValue(ai); };
            creators["quest givers"] = [](PlayerbotAI* ai) { return new QuestGiversValue(ai); };

            creators["trainable spell map"] = [](PlayerbotAI* ai) { return new TrainableSpellMapValue(ai); };

          

            creators["entry travel purpose"] = [](PlayerbotAI* ai) { return new EntryTravelPurposeMapValue(ai); };
            creators["entry guidps"] = [](PlayerbotAI* ai) { return new EntryGuidpsValue(ai); };

            creators["full mount list"] = [](PlayerbotAI* ai) { return new FullMountListValue(ai); };

            creators["global string"] = [](PlayerbotAI* ai) { return new StringManualSetValue(ai); };
        }
    };


    class SharedObjectContext
    {
    public:
        SharedObjectContext() { valueContexts.Add(new SharedValueContext()); };

    public:
        virtual UntypedValue* GetUntypedValue(const std::string& name)
        {
            PlayerbotAI* ai = new PlayerbotAI();
            UntypedValue* value = valueContexts.GetObject(name, ai);
            delete ai;
            return value;
        }

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
    protected:
        NamedObjectContextList<UntypedValue> valueContexts;
    };
#define sSharedObjectContext MaNGOS::Singleton<SharedObjectContext>::Instance()
}