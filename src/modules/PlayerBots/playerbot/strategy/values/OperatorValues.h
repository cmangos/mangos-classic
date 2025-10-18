#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class BoolAndValue : public BoolCalculatedValue, public Qualified
    {
    public:
        BoolAndValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "bool and"), Qualified() {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "bool and"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "operator"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will return true if all of the values included in the qualifier return true.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class NotValue : public BoolCalculatedValue, public Qualified
    {
    public:
        NotValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "not"), Qualified() {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "not"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "operator"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will return false if any of the values included in the qualifier return true.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class BoolOrValue : public BoolCalculatedValue, public Qualified
    {
    public:
        BoolOrValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "bool or"), Qualified() {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "bool or"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "operator"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will return true if any of the values included in the qualifier return true.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class GT32Value : public BoolCalculatedValue, public Qualified
    {
    public:
        GT32Value(PlayerbotAI* ai) : BoolCalculatedValue(ai, "gt32"), Qualified() {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "gt"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "operator"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will return true if the first value is greater than the second value";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };
}
