#pragma once
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    DEFLECT_TRIGGER(ShadowWardTrigger, "shadow ward");

	class DemonArmorTrigger : public BuffTrigger
	{
	public:
		DemonArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "demon armor") {}
		virtual bool IsActive();
	};

    class SpellstoneTrigger : public BuffTrigger
    {
    public:
        SpellstoneTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "spellstone") {}
        virtual bool IsActive();
    };

    class NoCurseTrigger : public Trigger
    {
    public:
        NoCurseTrigger(PlayerbotAI* ai) : Trigger(ai, "no curse") {}
        bool IsActive() override;

    private:
        std::string GetTargetName() override { return "current target"; }
    };

    class NoCurseOnAttackerTrigger : public Trigger
    {
    public:
        NoCurseOnAttackerTrigger(PlayerbotAI* ai) : Trigger(ai, "no curse on attacker") {}
        bool IsActive() override;
    };

    DEBUFF_TRIGGER_A(CorruptionTrigger, "corruption");
    DEBUFF_TRIGGER(SiphonLifeTrigger, "siphon life");
    DEBUFF_TRIGGER(UnstableAfflictionTrigger, "unstable affliction");

    INTERRUPT_TRIGGER(DeathCoilInterruptTrigger, "death coil");
    INTERRUPT_HEALER_TRIGGER(DeathCoilInterruptTHealerTrigger, "death coil");
    SNARE_TRIGGER(DeathCoilSnareTrigger, "death coil");
    SNARE_TRIGGER(ShadowfurySnareTrigger, "shadowfury");
    INTERRUPT_TRIGGER(ShadowfuryInterruptTrigger, "shadowfury");

    class CorruptionOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CorruptionOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "corruption") {}
        bool IsActive() override;
    };

    class SeedOfCorruptionOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        SeedOfCorruptionOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "seed of corruption") {}
        bool IsActive() override;
    };

    DEBUFF_TRIGGER(CurseOfAgonyTrigger, "curse of agony");

    class CurseOfAgonyOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfAgonyOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of agony") {}
    };

    DEBUFF_TRIGGER(CurseOfDoomTrigger, "curse of doom");

    class CurseOfDoomOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfDoomOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of doom") {}
    };

    DEBUFF_TRIGGER(CurseOfTheElementsTrigger, "curse of the elements");

    class CurseOfTheElementsOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfTheElementsOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of the elements") {}
    };

    DEBUFF_TRIGGER(CurseOfRecklessnessTrigger, "curse of recklessness");

    class CurseOfRecklessnessOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfRecklessnessOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of recklessness") {}
    };

    DEBUFF_TRIGGER(CurseOfWeaknessTrigger, "curse of weakness");

    class CurseOfWeaknessOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfWeaknessOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of weakness") {}
    };

    DEBUFF_TRIGGER(CurseOfTonguesTrigger, "curse of tongues");

    class CurseOfTonguesOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfTonguesOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of tongues") {}
    };

    DEBUFF_TRIGGER(CurseOfShadowTrigger, "curse of shadow");

    class CurseOfShadowOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        CurseOfShadowOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "curse of shadow") {}
    };

    class SiphonLifeOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        SiphonLifeOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "siphon life") {}
    };

    class UnstableAfflictionOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        UnstableAfflictionOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "unstable affliction") {}
    };

    DEBUFF_TRIGGER(ImmolateTrigger, "immolate");

    class ShadowTranceTrigger : public HasAuraTrigger
    {
    public:
        ShadowTranceTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "shadow trance") {}
    };

    class BacklashTrigger : public HasAuraTrigger
    {
    public:
        BacklashTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "backlash", 1, SPELL_AURA_ADD_PCT_MODIFIER) {}
    };

    class BanishTrigger : public HasCcTargetTrigger
    {
    public:
        BanishTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "banish") {}
    };

    class WarlockConjuredItemTrigger : public ItemCountTrigger
    {
    public:
        WarlockConjuredItemTrigger(PlayerbotAI* ai, std::string item) : ItemCountTrigger(ai, item, 1) {}

        virtual bool IsActive() { return ItemCountTrigger::IsActive() && (ai->HasCheat(BotCheatMask::item) || AI_VALUE2(uint32, "item count", "soul shard") > 0); }
    };

    class HasSpellstoneTrigger : public WarlockConjuredItemTrigger
    {
    public:
        HasSpellstoneTrigger(PlayerbotAI* ai) : WarlockConjuredItemTrigger(ai, "spellstone") {}
    };

    class HasFirestoneTrigger : public WarlockConjuredItemTrigger
    {
    public:
        HasFirestoneTrigger(PlayerbotAI* ai) : WarlockConjuredItemTrigger(ai, "firestone") {}
    };

    class HasHealthstoneTrigger : public WarlockConjuredItemTrigger
    {
    public:
        HasHealthstoneTrigger(PlayerbotAI* ai) : WarlockConjuredItemTrigger(ai, "healthstone") {}
    };

    class FearTrigger : public HasCcTargetTrigger
    {
    public:
        FearTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "fear") {}
    };

    class AmplifyCurseTrigger : public BuffTrigger
    {
    public:
        AmplifyCurseTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "amplify curse") {}
    };

    class InfernoTrigger : public BuffTrigger
    {
    public:
        InfernoTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "inferno", 10) {}
        virtual bool IsActive();
    };

    class LifeTapTrigger : public Trigger
    {
    public:
        LifeTapTrigger(PlayerbotAI* ai) : Trigger(ai, "life tap", 2) {}
        bool IsActive() override;
    };

    class DrainSoulTrigger : public Trigger
    {
    public:
        DrainSoulTrigger(PlayerbotAI* ai) : Trigger(ai, "drain soul") {}
        bool IsActive() override;
    };

    class FearPvpTrigger : public Trigger
    {
    public:
        FearPvpTrigger(PlayerbotAI* ai) : Trigger(ai, "fear pvp") {}
        bool IsActive() override;
    };

    class ConflagrateTrigger : public Trigger
    {
    public:
        ConflagrateTrigger(PlayerbotAI* ai) : Trigger(ai, "conflagrate") {}
        bool IsActive() override;
    };

    class DemonicSacrificeTrigger : public Trigger
    {
    public:
        DemonicSacrificeTrigger(PlayerbotAI* ai) : Trigger(ai, "demonic sacrifice", 5) {}
        bool IsActive() override;
    };

    class SoulLinkTrigger : public Trigger
    {
    public:
        SoulLinkTrigger(PlayerbotAI* ai) : Trigger(ai, "soul link", 5) {}
        bool IsActive() override;
    };

    class NoSpecificPetTrigger : public Trigger
    {
    public:
        NoSpecificPetTrigger(PlayerbotAI* ai, std::string name, uint32 petEntry) : Trigger(ai, name, 5), entry(petEntry) {}
        bool IsActive() override;

    private:
        uint32 entry;
    };

    class NoImpTrigger : public NoSpecificPetTrigger
    {
    public:
        NoImpTrigger(PlayerbotAI* ai) : NoSpecificPetTrigger(ai, "no imp", 416) {}
    };

    class NoVoidwalkerTrigger : public NoSpecificPetTrigger
    {
    public:
        NoVoidwalkerTrigger(PlayerbotAI* ai) : NoSpecificPetTrigger(ai, "no voidwalker", 1860) {}
    };

    class NoSuccubusTrigger : public NoSpecificPetTrigger
    {
    public:
        NoSuccubusTrigger(PlayerbotAI* ai) : NoSpecificPetTrigger(ai, "no succubus", 1863) {}
    };

    class NoFelhunterTrigger : public NoSpecificPetTrigger
    {
    public:
        NoFelhunterTrigger(PlayerbotAI* ai) : NoSpecificPetTrigger(ai, "no felhunter", 417) {}
    };

    class NoFelguardTrigger : public NoSpecificPetTrigger
    {
    public:
        NoFelguardTrigger(PlayerbotAI* ai) : NoSpecificPetTrigger(ai, "no felguard", 17252) {}
    };

    class SpellLockTrigger : public InterruptSpellTrigger
    {
    public:
        SpellLockTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "spell lock") {}

        bool IsActive() override
        {
            return InterruptSpellTrigger::IsActive();
        }
    };

    class SpellLockEnemyHealerTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        SpellLockEnemyHealerTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "spell lock") {}
    };

    class SoulstoneTrigger : public ItemTargetTrigger
    {
    public:
        SoulstoneTrigger(PlayerbotAI* ai) : ItemTargetTrigger(ai, "revive targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
        uint32 GetItemId() override;
    };
}
