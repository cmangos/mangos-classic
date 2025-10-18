#pragma once

#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    DEBUFF_TRIGGER(HolyFireTrigger, "holy fire");
    DEBUFF_TRIGGER(PowerWordPainTrigger, "shadow word: pain");
    DEBUFF_ENEMY_TRIGGER(PowerWordPainOnAttackerTrigger, "shadow word: pain");
    DEBUFF_TRIGGER(VampiricTouchTrigger, "vampiric touch");
    DEBUFF_ENEMY_TRIGGER(VampiricTouchOnAttackerTrigger, "vampiric touch");
    DEBUFF_TRIGGER(VampiricEmbraceTrigger, "vampiric embrace");
    CURE_TRIGGER(DispelMagicTrigger, "dispel magic", DISPEL_MAGIC);
    CURE_PARTY_TRIGGER(DispelMagicPartyMemberTrigger, "dispel magic", DISPEL_MAGIC);
    CURE_TRIGGER(CureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
    CURE_PARTY_TRIGGER(PartyMemberCureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
    BUFF_TRIGGER_A(InnerFireTrigger, "inner fire");
    BUFF_TRIGGER_A(ShadowformTrigger, "shadowform");
    BUFF_TRIGGER(InnerFocusTrigger, "inner focus");
    CC_TRIGGER(ShackleUndeadTrigger, "shackle undead");
    INTERRUPT_TRIGGER(SilenceTrigger, "silence");
    INTERRUPT_HEALER_TRIGGER(SilenceEnemyHealerTrigger, "silence");

    // racials
    DEBUFF_TRIGGER(DevouringPlagueTrigger, "devouring plague");
    BUFF_TRIGGER(TouchOfWeaknessTrigger, "touch of weakness");
    DEBUFF_TRIGGER(HexOfWeaknessTrigger, "hex of weakness");
    BUFF_TRIGGER(ShadowguardTrigger, "shadowguard");
    DEFLECT_TRIGGER(FeedbackTrigger, "feedback");
    SNARE_TRIGGER(ChastiseTrigger, "chastise");
    DEBUFF_TRIGGER(StarshardsTrigger, "starshards");

    BOOST_TRIGGER_A(ShadowfiendTrigger, "shadowfiend");
    CAN_CAST_TRIGGER(MindBlastTrigger, "mind blast");
    CAN_CAST_TRIGGER(SmiteTrigger, "smite");

    class PowerWordFortitudeOnPartyTrigger : public BuffOnPartyTrigger 
    {
    public:
        PowerWordFortitudeOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "power word: fortitude", 4) {}
        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of fortitude", GetTarget()); }
    };

    class PowerWordFortitudeTrigger : public BuffTrigger 
    {
    public:
        PowerWordFortitudeTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "power word: fortitude", 4) {}
        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("prayer of fortitude", GetTarget()); }
    };

    class DivineSpiritOnPartyTrigger : public BuffOnPartyTrigger 
    {
    public:
        DivineSpiritOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "divine spirit", 4) {}
        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of spirit", GetTarget()); }
    };

    class DivineSpiritTrigger : public BuffTrigger 
    {
    public:
        DivineSpiritTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "divine spirit", 4) {}
        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("prayer of spirit", GetTarget()); }
    };

    class ShadowProtectionOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        ShadowProtectionOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "shadow protection", 4) {}
        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of shadow protection", GetTarget()); }
    };

    class ShadowProtectionTrigger : public BuffTrigger 
    {
    public:
        ShadowProtectionTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "shadow protection", 4) {}
        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("prayer of shadow protection", GetTarget()); }
    };

    class PrayerOfFortitudeOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfFortitudeOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of fortitude", "power word: fortitude", 4) {}
    };

    class PrayerOfSpiritOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfSpiritOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of spirit", "divine spirit", 4) {}
    };

    class PrayerOfShadowProtectionOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfShadowProtectionOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of shadow protection", "shadow protection", 4) {}
    };

    class BindingHealTrigger : public PartyMemberLowHealthTrigger 
    {
    public:
        BindingHealTrigger(PlayerbotAI* ai) : PartyMemberLowHealthTrigger(ai, "binding heal", sPlayerbotAIConfig.lowHealth, 0) {}

        virtual bool IsActive()
        {
            return PartyMemberLowHealthTrigger::IsActive() && AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.mediumHealth;
        }
    };

    class PowerInfusionTrigger : public SpellTargetTrigger
    {
    public:
        PowerInfusionTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "power infusion", "boost targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };

    class FearWardTrigger : public SpellTargetTrigger
    {
    public:
        FearWardTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "fear ward", "buff targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };
}
