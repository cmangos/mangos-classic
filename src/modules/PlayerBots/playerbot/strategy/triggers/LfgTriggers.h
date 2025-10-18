#pragma once

#include "playerbot/strategy/Trigger.h"

namespace ai
{
class LfgProposalActiveTrigger : public Trigger
{
public:
    LfgProposalActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "lfg proposal active", 5) {}

    virtual bool IsActive();
};

class UnknownDungeonTrigger : public Trigger
{
public:
    UnknownDungeonTrigger(PlayerbotAI* ai) : Trigger(ai, "unknown dungeon", 20) {}

    virtual bool IsActive();
};
}
