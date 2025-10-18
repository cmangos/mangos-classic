#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{    
    class SubStrategyValue : public ManualSetValue<std::string>
	{
	public:
        SubStrategyValue(PlayerbotAI* ai, std::string defaultValue = "", std::string name = "substrategy", std::string allowedValues = "") : ManualSetValue(ai, defaultValue, name), allowedValues(allowedValues) {};
        virtual void Set(std::string newValue) override;

        void SetValues(std::string& currentValue, const std::string newValue, const std::string allowedValue);
        virtual std::vector<std::string> GetAllowedValues() { return StrSplit(allowedValues, ","); }

        virtual std::string Save() override { return value; };
        virtual bool Load(std::string value) override;
    private:
        std::string allowedValues;
    };
};

