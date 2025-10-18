#pragma once
#include "playerbot/strategy//AiObject.h"
#include "playerbot/strategy/Value.h"
#include "playerbot/TravelMgr.h"

namespace ai
{
    class FindTargetStrategy
    {
    public:
        FindTargetStrategy(PlayerbotAI* ai)
        {
            result = NULL;
            this->ai = ai;
        }

    public:
        Unit* GetResult() { return result; }

    public:
        virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) = 0;
        void GetPlayerCount(Unit* creature, int* tankCount, int* dpsCount);

    protected:
        Unit* result;
        PlayerbotAI* ai;

    protected:
        std::map<Unit*, int> tankCountCache;
        std::map<Unit*, int> dpsCountCache;
    };

    class FindNonCcTargetStrategy : public FindTargetStrategy
    {
    public:
        FindNonCcTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai) {}

    protected:
        virtual bool IsCcTarget(Unit* attacker);

    };

    class TargetValue : public UnitCalculatedValue
	{
	public:
        TargetValue(PlayerbotAI* ai, std::string name = "target", int checkInterval = 1) : UnitCalculatedValue(ai, name, checkInterval) {}
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "target"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "target"; }
        virtual std::string GetHelpDescription() { return "This value contains a [h:object|unit] which is the best target based on the current target strategy."; }
        virtual std::vector<std::string> GetUsedValues() { return { "possible attack targets" }; }
#endif 
    protected:
        Unit* FindTarget(FindTargetStrategy* strategy);
    };

    class RpgTargetValue : public ManualSetValue<GuidPosition>
    {
    public:
        RpgTargetValue(PlayerbotAI* ai, std::string name = "rpg target") : ManualSetValue<GuidPosition>(ai, GuidPosition(), name) {}
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg target"; } //Must equal iternal name
        virtual std::string GetHelpTypeName() { return "rpg"; }
        virtual std::string GetHelpDescription() { return "This value contains the [h:object|objectGuid] of a [h:object|unit] or [h:object|gameObject] to move to and rpg with.\nThis value is manually set."; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

        virtual std::string Format()
        {
            return chat->formatGuidPosition(value, bot);
        }
    };

    class TravelTargetValue : public ManualSetValue<TravelTarget*>
    {
    public:
        TravelTargetValue(PlayerbotAI* ai, std::string name = "travel target") : ManualSetValue<TravelTarget*>(ai, new TravelTarget(ai), name) {}
        virtual ~TravelTargetValue() { delete value; }
    };	

    class LeaderTravelTargetValue : public CalculatedValue<TravelTarget*>
    {
    public:
        LeaderTravelTargetValue(PlayerbotAI* ai, std::string name = "leader travel target") : CalculatedValue<TravelTarget*>(ai, name) {};

        virtual TravelTarget* Calculate() override;
    };

    class IgnoreRpgTargetValue : public ManualSetValue<std::set<ObjectGuid>& >
    {
    public:
        IgnoreRpgTargetValue(PlayerbotAI* ai) : ManualSetValue<std::set<ObjectGuid>& >(ai, data, "ignore rpg targets") {}

    private:
        std::set<ObjectGuid> data;
    };

    class TalkTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        TalkTargetValue(PlayerbotAI* ai, std::string name = "talk target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class AttackTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        AttackTargetValue(PlayerbotAI* ai, std::string name = "attack target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class PullTargetValue : public UnitManualSetValue
    {
    public:
        PullTargetValue(PlayerbotAI* ai, std::string name = "pull target") : UnitManualSetValue(ai, nullptr, name) {}
        void Set(Unit* unit) override;
        Unit* Get() override;

    private:
        ObjectGuid guid;
    };

    class FollowTargetValue : public UnitCalculatedValue
    {
    public:
        FollowTargetValue(PlayerbotAI* ai, std::string name = "follow target") : UnitCalculatedValue(ai, name) {}
        Unit* Calculate() override;
    };

    class ManualFollowTargetValue : public GuidPositionManualSetValue
    {
    public:
        ManualFollowTargetValue(PlayerbotAI* ai, std::string name = "manual follow target") : GuidPositionManualSetValue(ai, GuidPosition(), name) {}
    };

    class ClosestAttackerTargetingMeTargetValue : public UnitCalculatedValue
    {
    public:
        ClosestAttackerTargetingMeTargetValue(PlayerbotAI* ai, std::string name = "closest attacker targeting me") : UnitCalculatedValue(ai, name) {}
        Unit* Calculate() override;
    };

    class FriendlyManualTargetsValue : public ManualSetValue<std::list<ObjectGuid>>
    {
    public:
        FriendlyManualTargetsValue(PlayerbotAI* ai) : ManualSetValue<std::list<ObjectGuid>>(ai, {}) {}
        std::list<ObjectGuid> Get() override;
        std::list<ObjectGuid> LazyGet() override;
    };

    class FocusHealTargetValue : public FriendlyManualTargetsValue
    {
    public:
        FocusHealTargetValue(PlayerbotAI* ai) : FriendlyManualTargetsValue(ai) {}
    };

    class BoostTargetsValue : public FriendlyManualTargetsValue
    {
    public:
        BoostTargetsValue(PlayerbotAI* ai) : FriendlyManualTargetsValue(ai) {}
    };

    class ReviveTargetsValue : public FriendlyManualTargetsValue
    {
    public:
        ReviveTargetsValue(PlayerbotAI* ai) : FriendlyManualTargetsValue(ai) {}
    };

    class BuffTargetsValue : public FriendlyManualTargetsValue
    {
    public:
        BuffTargetsValue(PlayerbotAI* ai) : FriendlyManualTargetsValue(ai) {}
    };
}
