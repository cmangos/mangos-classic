#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/strategy/Value.h"


namespace ai
{
    class GameObjectsValue : public GuidPositionListCalculatedValue
    {
    public:
        GameObjectsValue(PlayerbotAI* ai, std::string name = "gos") : GuidPositionListCalculatedValue(ai, name, 1) {}

        virtual std::list<GuidPosition> Calculate();
    };


    class EntryFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        EntryFilterValue(PlayerbotAI* ai, std::string name = "entry filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual std::list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "entry filter"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "entry"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids of specific entries.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class GuidFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
       GuidFilterValue(PlayerbotAI* ai, std::string name = "guid filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

       virtual std::list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
       virtual std::string GetHelpName() { return "guid filter"; } //Must equal iternal name
       virtual std::string GetHelpTypeName() { return "guid"; }
       virtual std::string GetHelpDescription()
       {
          return "This value will returns only the ObjectGuids of specific entries.";
       }
       virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 

    private:
       std::vector<std::string> QualifierToEntryList(const std::string& qualifier);
    };

    class RangeFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        RangeFilterValue(PlayerbotAI* ai, std::string name = "range filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual std::list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "range filter"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "entry"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids within a specific range.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class GoUsableFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoUsableFilterValue(PlayerbotAI* ai, std::string name = "go usable filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual std::list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "go usable filter"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "entry"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids within a specific range.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };

    class GoTrappedFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoTrappedFilterValue(PlayerbotAI* ai, std::string name = "go trapped filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual std::list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "go trapped filter"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "entry"; }
        virtual std::string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids that are not trapped.";
        }
        virtual std::vector<std::string> GetUsedValues() { return { }; }
#endif 
    };


    class GosInSightValue : public GuidPositionListCalculatedValue, public Qualified
	{
	public:
        GosInSightValue(PlayerbotAI* ai, std::string name = "gos in sight") : GuidPositionListCalculatedValue(ai, name, 3), Qualified() {}

        virtual std::list<GuidPosition> Calculate();
    };

    class GoSCloseValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoSCloseValue(PlayerbotAI* ai, std::string name = "gos close") : GuidPositionListCalculatedValue(ai, name, 3), Qualified() {}

        virtual std::list<GuidPosition> Calculate() { return AI_VALUE2(std::list<GuidPosition>, "range filter", "gos," + std::to_string(INTERACTION_DISTANCE)); }
    };



    class HasObjectValue : public BoolCalculatedValue, public Qualified
    {
    public:
        HasObjectValue(PlayerbotAI* ai, std::string name = "has object") : BoolCalculatedValue(ai, name, 3), Qualified() {}

        virtual bool Calculate() { return !AI_VALUE(std::list<GuidPosition>, getQualifier()).empty(); }
    };
}
