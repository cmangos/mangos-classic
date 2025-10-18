
#include "playerbot/playerbot.h"
#include "SkipSpellsListValue.h"

using namespace ai;

std::string SkipSpellsListValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (std::set<uint32>::iterator i = value.begin(); i != value.end(); ++i)
    {
        if (!first) out << ",";
        else first = false;
        out << *i;
    }
    return out.str();
}

bool SkipSpellsListValue::Load(std::string text)
{
    value.clear();

    std::vector<std::string> ss = split(text, ',');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        value.insert(atoi(i->c_str()));
    }
    return true;
}
