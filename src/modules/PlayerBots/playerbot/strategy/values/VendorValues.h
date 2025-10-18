#pragma once

#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/NamedObjectContext.h"

class PlayerbotAI;

namespace ai
{      
    //                              itemId, entry
    typedef std::unordered_multimap<uint32, int32> VendorMap;

    //Returns the vendor map of all entries
    class VendorMapValue : public SingleCalculatedValue<VendorMap*>
    {
    public:
        VendorMapValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "vendor map") {}

        virtual VendorMap* Calculate();
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "vendor map"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "item"; }
        virtual std::string GetHelpDescription()
        {
            return "This value returns all creatures and game objects and the items they sell.";
        }
        virtual std::vector<std::string> GetUsedValues() { return {  }; }
#endif 
    };

    //Returns the entries that sell a specific item
    class ItemVendorListValue : public SingleCalculatedValue<std::list<int32>>, public Qualified
    {
    public:
        ItemVendorListValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "item vendor list"), Qualified() {}

        virtual std::list<int32> Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "item vendor list"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "item"; }
        virtual std::string GetHelpDescription()
        {
            return "This value returns all vendors that sell a specific item.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { "vendor map" }; }
#endif 
    };

    class VendorHasUsefulItemValue : public BoolCalculatedValue, public Qualified
    {
    public:
        VendorHasUsefulItemValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "vendor has useful item",2), Qualified() {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "vendor has useful item"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "item"; }
        virtual std::string GetHelpDescription()
        {
            return "This value returns true if the specified vendor sells a usefull item.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { "item usage" }; }
#endif 
    };
}

