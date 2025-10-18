#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
    class NaxxramasEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        NaxxramasEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter naxxramas", "naxxramas", 533) {}
    };

    class NaxxramasLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        NaxxramasLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave naxxramas", "naxxramas", 533) {}
    };

    class FourHorsemanStartFightTrigger : public StartBossFightTrigger
    {
    public:
        FourHorsemanStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start four horseman fight", "four horseman", 16062) {}
    };

    class FourHorsemanEndFightTrigger : public EndBossFightTrigger
    {
    public:
        FourHorsemanEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end four horseman fight", "four horseman", 16062) {}
    };
}