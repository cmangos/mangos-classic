#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class EnemyPlayerNear : public Trigger
    {
    public:
        EnemyPlayerNear(PlayerbotAI* ai) : Trigger(ai, "enemy player near") {}

    public:
        virtual bool IsActive();
    };

    class PlayerHasNoFlag : public Trigger
    {
    public:
        PlayerHasNoFlag(PlayerbotAI* ai) : Trigger(ai, "player has no flag") {}

    public:
        virtual bool IsActive();
    };

    class PlayerHasFlag : public Trigger
    {
    public:
        PlayerHasFlag(PlayerbotAI* ai) : Trigger(ai, "player has flag") {}

    public:
        virtual bool IsActive();
    };

    class EnemyFlagCarrierNear : public Trigger
    {
    public:
        EnemyFlagCarrierNear(PlayerbotAI* ai) : Trigger(ai, "enemy flagcarrier near") {}

    public:
        virtual bool IsActive();
    };

    class TeamFlagCarrierNear : public Trigger
    {
    public:
        TeamFlagCarrierNear(PlayerbotAI* ai) : Trigger(ai, "team flagcarrier near") {}

    public:
        virtual bool IsActive();
    };

    class TeamHasFlag : public Trigger
    {
    public:
        TeamHasFlag(PlayerbotAI* ai) : Trigger(ai, "team has flag") {}

    public:
        virtual bool IsActive();
    };

    class EnemyTeamHasFlag : public Trigger
    {
    public:
        EnemyTeamHasFlag(PlayerbotAI* ai) : Trigger(ai, "enemy team has flag") {}

    public:
        virtual bool IsActive();
    };

    class PlayerIsInBattleground : public Trigger
    {
    public:
        PlayerIsInBattleground(PlayerbotAI* ai) : Trigger(ai, "in battleground") {}

    public:
        virtual bool IsActive();
    };

    class BgWaitingTrigger : public Trigger
    {
    public:
        BgWaitingTrigger(PlayerbotAI* ai) : Trigger(ai, "bg waiting", 30) {}

    public:
        virtual bool IsActive();
    };

    class BgActiveTrigger : public Trigger
    {
    public:
        BgActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "bg active", 1) {}

    public:
        virtual bool IsActive();
    };

    class BgInviteActiveTrigger : public Trigger
    {
    public:
        BgInviteActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "bg invite active", 10) {}

    public:
        virtual bool IsActive();
    };

    class BgEndedTrigger : public Trigger
    {
    public:
        BgEndedTrigger(PlayerbotAI* ai) : Trigger(ai, "bg ended", 10) {}

    public:
        virtual bool IsActive();
    };

    class PlayerIsInBattlegroundWithoutFlag : public Trigger
    {
    public:
        PlayerIsInBattlegroundWithoutFlag(PlayerbotAI* ai) : Trigger(ai, "in battleground without flag") {}

    public:
        virtual bool IsActive();
    };

    class PlayerWantsInBattlegroundTrigger : public Trigger
    {
    public:
        PlayerWantsInBattlegroundTrigger(PlayerbotAI* ai) : Trigger(ai, "wants in bg") {}

    public:
        virtual bool IsActive();
    };

    class VehicleNearTrigger : public Trigger
    {
    public:
        VehicleNearTrigger(PlayerbotAI* ai) : Trigger(ai, "vehicle near", 10) {}

    public:
        virtual bool IsActive();
    };

    class InVehicleTrigger : public Trigger, public Qualified
    {
    public:
        InVehicleTrigger(PlayerbotAI* ai) : Trigger(ai, "in vehicle"), Qualified() {}

    public:
        virtual bool IsActive();
    };
}
