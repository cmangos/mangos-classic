#pragma once
#include "Strategy.h"

namespace ai
{
    class CustomStrategy : public Strategy, public Qualified
    {
    public:
        CustomStrategy(PlayerbotAI* ai) : Strategy(ai), Qualified() {}
        std::string getName() override { return "custom::" + qualifier; }
        void Reset();

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;

        void LoadActionLines(uint32 owner);

    private:
        std::list<std::string> actionLines;
        
    public:
        static std::map<std::string, std::string> actionLinesCache;
    };
}
