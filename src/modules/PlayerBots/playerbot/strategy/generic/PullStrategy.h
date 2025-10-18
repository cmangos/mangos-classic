#pragma once
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/Multiplier.h"

namespace ai
{
    class PullStrategy : public Strategy
    {
    public:
        PullStrategy(PlayerbotAI* ai, std::string pullAction, std::string prePullAction = "");

    public:
        std::string getName() override { return "pull"; }

        static PullStrategy* Get(PlayerbotAI* ai);
        static uint8 GetMaxPullTime() { return 15; }
        const time_t& GetPullStartTime() const { return pullStartTime; }
        
        bool CanDoPullAction(Unit* target);

        Unit* GetTarget() const;
        bool HasTarget() const { return GetTarget() != nullptr; }

        std::string GetPullActionName() const;
        std::string GetSpellName() const;
        float GetRange() const;

        std::string GetPreActionName() const;

        void RequestPull(Unit* target, bool resetTime = true);
        bool IsPullPendingToStart() const { return pendingToStart; }
        bool HasPullStarted() const { return pullStartTime > 0; }
        void OnPullStarted();
        void OnPullEnded();
        ReactStates GetPetReactState() const { return petReactState; }
        void SetPetReactState(ReactStates reactState) { petReactState = reactState; }

    private:
        void SetTarget(Unit* target);

        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;

    private:
        std::string pullActionName; //shoot
        std::string preActionName;
        bool pendingToStart;
        time_t pullStartTime;
        ReactStates petReactState;
    };

    class PullMultiplier : public Multiplier
    {
    public:
        PullMultiplier(PlayerbotAI* ai) : Multiplier(ai, "pull") {}

    public:
        float GetValue(Action* action) override;
    };

    class PossibleAdsStrategy : public Strategy
    {
    public:
        PossibleAdsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "ads"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class PullBackStrategy : public Strategy
    {
    public:
        PullBackStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "pull back"; }

        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
