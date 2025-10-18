#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"

namespace ai
{
    class NaxxramasEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NaxxramasEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable naxxramas strategy", "+naxxramas") {}
    };

    class NaxxramasDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NaxxramasDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable naxxramas strategy", "-naxxramas") {}
    };

    class FourHorsemanEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        FourHorsemanEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable four horseman fight strategy", "+four horseman") {}
    };

    class FourHorsemanDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        FourHorsemanDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable four horseman fight strategy", "-four horseman") {}
    };
}