
#include "playerbot/playerbot.h"
#include "PositionValue.h"

using namespace ai;

PositionValue::PositionValue(PlayerbotAI* ai, std::string name)
    : ManualSetValue<ai::PositionMap&>(ai, positions, name)
{
}

std::string PositionValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (ai::PositionMap::iterator i = value.begin(); i != value.end(); ++i)
    {
        std::string name = i->first;
        ai::PositionEntry pos = i->second;
        if (pos.isSet())
        {
            if (!first) out << "^";
            else first = false;

            out << name << "=" << pos.x << "," << pos.y << "," << pos.z << "," << pos.mapId;
        }
    }
    return out.str();
}

bool PositionValue::Load(std::string text)
{
    value.clear();

    std::vector<std::string> ss = split(text, '^');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        std::vector<std::string> s1 = split(*i, '=');
        if (s1.size() != 2) continue;
        std::string name = s1[0];

        std::vector<std::string> s2 = split(s1[1], ',');
        if (s2.size() != 4) continue;
        double x = atof(s2[0].c_str());
        double y = atof(s2[1].c_str());
        double z = atof(s2[2].c_str());
        uint32 mapId = atoi(s2[3].c_str());

        value[name].Set(x, y, z, mapId);
    }
    return true;
}

PositionEntry SinglePositionValue::Calculate()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    return posMap[getQualifier()];
}
void SinglePositionValue::Set(PositionEntry value)
{
    PositionMap& posMap = AI_VALUE(PositionMap&,"position");
    PositionEntry pos = posMap[getQualifier()];
    pos = value;
    posMap[getQualifier()] = pos;
}
void SinglePositionValue::Reset() 
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionEntry pos = posMap[getQualifier()];
    pos.Reset();
    posMap[getQualifier()] = pos;
}

