#pragma once
#include "GenericActions.h"

namespace ai
{
	class TaxiAction : public ChatCommandAction
    {
	public:
		TaxiAction(PlayerbotAI* ai) : ChatCommandAction(ai, "taxi") {}
        virtual bool Execute(Event& event) override;
    };
}