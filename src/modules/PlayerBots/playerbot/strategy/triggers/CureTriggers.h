#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
	class SpellTrigger;

    class NeedCureTrigger : public SpellTrigger 
    {
    public:
        NeedCureTrigger(PlayerbotAI* ai, std::string spell, uint32 dispelType, int checkInterval = 3) : SpellTrigger(ai, spell, checkInterval), dispelType(dispelType) {}
        virtual std::string GetTargetName() { return "self target"; }
        virtual bool IsActive();

    protected:
        uint32 dispelType;
    };

    class TargetAuraDispelTrigger : public NeedCureTrigger 
    {
    public:
        TargetAuraDispelTrigger(PlayerbotAI* ai, std::string spell, uint32 dispelType, int checkInterval = 3) : NeedCureTrigger(ai, spell, dispelType, checkInterval) {}
		virtual std::string GetTargetName() { return "current target"; }
    };

    class PartyMemberNeedCureTrigger : public NeedCureTrigger 
    {
    public:
        PartyMemberNeedCureTrigger(PlayerbotAI* ai, std::string spell, uint32 dispelType) : NeedCureTrigger(ai, spell, dispelType) {}
		virtual Value<Unit*>* GetTargetValue();
    };

    class NeedWorldBuffTrigger : public Trigger 
    {
    public:
        NeedWorldBuffTrigger(PlayerbotAI* ai) : Trigger(ai, "need world buff", 5) {}
        bool IsActive();
    };
}