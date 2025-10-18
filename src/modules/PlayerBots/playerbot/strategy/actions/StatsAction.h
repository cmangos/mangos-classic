#pragma once
#include "GenericActions.h"

namespace ai
{
    class StatsAction : public ChatCommandAction
    {
    public:
        StatsAction(PlayerbotAI* ai) : ChatCommandAction(ai, "stats") {}
        virtual bool Execute(Event& event) override;

        bool isUsefulWhenStunned() override { return true; }
    private:
        void ListBagSlots(std::ostringstream &out);
        void ListXP(std::ostringstream &out);
        void ListRepairCost(std::ostringstream &out);
        void ListGold(std::ostringstream &out);
        void ListPower(std::ostringstream& out);
        uint32 EstRepair(uint16 pos);
        double RepairPercent(uint16 pos);
    };
}
