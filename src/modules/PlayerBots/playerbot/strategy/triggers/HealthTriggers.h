#pragma once
#include "playerbot/strategy/Trigger.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class ValueInRangeTrigger : public Trigger
    {
    public:
        ValueInRangeTrigger(PlayerbotAI* ai, std::string name, float maxValue, float minValue) : Trigger(ai, name) 
        {
            this->maxValue = maxValue;
            this->minValue = minValue;
        }

    public:
        virtual float GetValue() = 0;

        virtual bool IsActive() 
        {
            float value = GetValue();
            return value < maxValue && value >= minValue;
        }

    protected:
        float maxValue, minValue;
    };

	class HealthInRangeTrigger : public ValueInRangeTrigger
	{
	public:
		HealthInRangeTrigger(PlayerbotAI* ai, std::string name, float maxValue, float minValue = 0, bool isTankRequired = false) : ValueInRangeTrigger(ai, name, maxValue, minValue) 
        {
            this->isTankRequired = isTankRequired;
        }

        bool IsActive() override;
		float GetValue() override;

    protected:
        bool isTankRequired;
	};

    class LowHealthTrigger : public HealthInRangeTrigger
    {
    public:
        LowHealthTrigger(PlayerbotAI* ai, std::string name = "low health",
            float value = sPlayerbotAIConfig.lowHealth, float minValue = sPlayerbotAIConfig.criticalHealth) :
            HealthInRangeTrigger(ai, name, value, minValue) {}

		virtual std::string GetTargetName() { return "self target"; }
    };

    class CriticalHealthTrigger : public LowHealthTrigger
    {
    public:
        CriticalHealthTrigger(PlayerbotAI* ai) :
            LowHealthTrigger(ai, "critical health", sPlayerbotAIConfig.criticalHealth, 0) {}
    };

    class MediumHealthTrigger : public LowHealthTrigger
    {
    public:
        MediumHealthTrigger(PlayerbotAI* ai) :
            LowHealthTrigger(ai, "medium health", sPlayerbotAIConfig.mediumHealth, sPlayerbotAIConfig.lowHealth) {}
    };

    class AlmostFullHealthTrigger : public LowHealthTrigger
    {
    public:
        AlmostFullHealthTrigger(PlayerbotAI* ai) :
            LowHealthTrigger(ai, "almost full health", sPlayerbotAIConfig.almostFullHealth, sPlayerbotAIConfig.mediumHealth) {}
    };

    class PartyMemberLowHealthTrigger : public HealthInRangeTrigger
    {
    public:
        PartyMemberLowHealthTrigger(PlayerbotAI* ai, std::string name = "party member low health", float value = sPlayerbotAIConfig.lowHealth, float minValue = sPlayerbotAIConfig.criticalHealth, bool isTankRequired = false) :
            HealthInRangeTrigger(ai, name, value, minValue, isTankRequired) {}

        virtual std::string GetTargetName() { return "party member to heal"; }
    };

    class PartyMemberCriticalHealthTrigger : public PartyMemberLowHealthTrigger
    {
    public:
        PartyMemberCriticalHealthTrigger(PlayerbotAI* ai) :
            PartyMemberLowHealthTrigger(ai, "party member critical health", sPlayerbotAIConfig.criticalHealth, 0) {}
    };

    class PartyMemberMediumHealthTrigger : public PartyMemberLowHealthTrigger
    {
    public:
        PartyMemberMediumHealthTrigger(PlayerbotAI* ai) :
            PartyMemberLowHealthTrigger(ai, "party member medium health", sPlayerbotAIConfig.mediumHealth,sPlayerbotAIConfig.lowHealth) {}
    };

    class PartyMemberAlmostFullHealthTrigger : public PartyMemberLowHealthTrigger
    {
    public:
        PartyMemberAlmostFullHealthTrigger(PlayerbotAI* ai) :
            PartyMemberLowHealthTrigger(ai, "party member almost full health", sPlayerbotAIConfig.almostFullHealth,sPlayerbotAIConfig.mediumHealth) {}
    };

    class TargetLowHealthTrigger : public HealthInRangeTrigger 
    {
    public:
        TargetLowHealthTrigger(PlayerbotAI* ai, float value, float minValue = 0) : HealthInRangeTrigger(ai, "target low health", value, minValue) {}
        virtual std::string GetTargetName() { return "current target"; }
    };

    class TargetCriticalHealthTrigger : public TargetLowHealthTrigger
    {
    public:
        TargetCriticalHealthTrigger(PlayerbotAI* ai) : TargetLowHealthTrigger(ai, 20) {}
    };

	class PartyMemberDeadTrigger : public Trigger 
    {
	public:
		PartyMemberDeadTrigger(PlayerbotAI* ai) : Trigger(ai, "resurrect", 3) {}
        virtual std::string GetTargetName() { return "party member to resurrect"; }
		virtual bool IsActive();
	};

    class DeadTrigger : public Trigger 
    {
    public:
        DeadTrigger(PlayerbotAI* ai) : Trigger(ai, "dead") {}
        virtual std::string GetTargetName() { return "self target"; }
        virtual bool IsActive();
    };

    class AoeHealTrigger : public Trigger 
    {
    public:
    	AoeHealTrigger(PlayerbotAI* ai, std::string name, std::string type, int count) :
    		Trigger(ai, name), type(type), count(count) {}
    public:
        virtual bool IsActive();

    protected:
        int count;
        std::string type;
    };

    class HealTargetFullHealthTrigger : public Trigger
    {
    public:
        HealTargetFullHealthTrigger(PlayerbotAI* ai, std::string name = "heal target full health") : Trigger(ai, name) {}
        bool IsActive() override;
    };
}
