#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class SeeSpellLocationValue : public LogCalculatedValue<WorldPosition>
    {
    public:
        SeeSpellLocationValue(PlayerbotAI* ai, std::string name = "see spell location") : LogCalculatedValue(ai, name) {};

        virtual bool EqualToLast(WorldPosition value) { return value == lastValue; };

        WorldPosition Calculate() { return this->value; }
    };

    class RTSCSelectedValue : public ManualSetValue<bool>
	{
	public:
        RTSCSelectedValue(PlayerbotAI* ai, bool defaultvalue = false, std::string name = "RTSC selected") : ManualSetValue(ai, defaultvalue,name) {};
    };

    class RTSCNextSpellActionValue : public ManualSetValue<std::string>
    {
    public:
        RTSCNextSpellActionValue(PlayerbotAI* ai, std::string defaultvalue = "", std::string name = "RTSC next spell action") : ManualSetValue(ai, defaultvalue, name) {};
    };

    class RTSCSavedLocationValue : public ManualSetValue<WorldPosition>, public Qualified
    {
    public:
        RTSCSavedLocationValue(PlayerbotAI* ai, WorldPosition defaultvalue = WorldPosition(), std::string name = "RTSC saved location") : ManualSetValue(ai, defaultvalue, name), Qualified() {};
        virtual std::string Save() { return value.to_string(); }
        virtual bool Load(std::string text) { value = WorldPosition(text); return true; }
    };
}
