#pragma once
#include "GenericActions.h"

namespace ai
{
    class SaveManaAction : public ChatCommandAction
    {
    public:
        SaveManaAction(PlayerbotAI* ai) : ChatCommandAction(ai, "save mana") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        std::string format(double value);
    };
}
