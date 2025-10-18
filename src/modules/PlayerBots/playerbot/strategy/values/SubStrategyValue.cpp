
#include "SubStrategyValue.h"

using namespace ai;

void SubStrategyValue::Set(std::string newValue)
{
	SetValues(value, newValue, allowedValues);
}


void SubStrategyValue::SetValues(std::string& currentValue, const std::string newValue, const std::string allowedValues)
{
	std::vector<std::string> currentValues = StrSplit(currentValue, ",");
	std::vector<std::string> newValues = StrSplit(newValue, ",");
	std::vector<std::string> allowedVals = StrSplit(allowedValues, ",");

	for (std::string& newVal : newValues)
	{
		char operation = newVal[0];

		if (operation == '-' || operation == '+' || operation == '~')
			newVal = newVal.substr(1);
		else
			operation = '+';

		if (allowedValues.size() && std::find(allowedVals.begin(), allowedVals.end(), newVal) == allowedVals.end())
			continue;

		auto found = std::find(currentValues.begin(), currentValues.end(), newVal);

		if (found == currentValues.end())
		{
			if (operation != '-')
				currentValues.push_back(newVal);
		}
		else
		{
			if (operation != '+')
				currentValues.erase(found);
		}
	}

	currentValue = "";

	for (std::string value : currentValues)
		currentValue += value + ",";

	if(!currentValues.empty())
		currentValue.pop_back();
}

bool SubStrategyValue::Load(std::string value)
{
    this->value = "";
    Set(value);
    return true;
}