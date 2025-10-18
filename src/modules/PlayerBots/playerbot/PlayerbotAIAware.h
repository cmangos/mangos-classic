#pragma once
#include <string>

#ifndef GenerateBotHelp
//#define GenerateBotHelp //Enable only for help generation
#endif

class PlayerbotAI;

namespace ai
{
    class PlayerbotAIAware 
    {
    public:
        PlayerbotAIAware(PlayerbotAI* const ai) : ai(ai) { }
        virtual ~PlayerbotAIAware() = default;
        virtual std::string getName() { return std::string(); }
    protected:
        PlayerbotAI* ai;
    };
}