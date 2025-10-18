#pragma once
#include "GenericActions.h"

namespace ai
{
    class TradeAction : public ChatCommandAction
    {
    public:
        TradeAction(PlayerbotAI* ai) : ChatCommandAction(ai, "trade") {}
        virtual bool Execute(Event& event) override;

    private:
        bool TradeItem(const Item& item, int8 slot);

    private:
        static std::map<std::string, uint32> slots;
    };
}
