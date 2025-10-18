#include "playerbot/playerbot.h"
#include "PriestActions.h"
#include "PriestTriggers.h"
#include "PriestAiObjectContext.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "DisciplinePriestStrategy.h"
#include "ShadowPriestStrategy.h"
#include "HolyPriestStrategy.h"

namespace ai
{
    namespace priest
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["cure"] = [](PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
                creators["offheal"] = [](PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["offdps"] = [](PlayerbotAI* ai) { return new OffdpsPlaceholderStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestAoePveStrategy(ai); };
                creators["aoe discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestAoePvpStrategy(ai); };
                creators["aoe discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestAoeRaidStrategy(ai); };
                creators["aoe holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestAoePveStrategy(ai); };
                creators["aoe holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestAoePvpStrategy(ai); };
                creators["aoe holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestAoeRaidStrategy(ai); };
                creators["aoe shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestAoePveStrategy(ai); };
                creators["aoe shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestAoePvpStrategy(ai); };
                creators["aoe shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestAoeRaidStrategy(ai); };
            }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestCurePveStrategy(ai); };
                creators["cure discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestCurePvpStrategy(ai); };
                creators["cure discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestCureRaidStrategy(ai); };
                creators["cure holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestCurePveStrategy(ai); };
                creators["cure holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestCurePvpStrategy(ai); };
                creators["cure holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestCureRaidStrategy(ai); };
                creators["cure shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestCurePveStrategy(ai); };
                creators["cure shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestCurePvpStrategy(ai); };
                creators["cure shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestCureRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestBuffPveStrategy(ai); };
                creators["buff discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestBuffPvpStrategy(ai); };
                creators["buff discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestBuffRaidStrategy(ai); };
                creators["buff holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestBuffPveStrategy(ai); };
                creators["buff holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestBuffPvpStrategy(ai); };
                creators["buff holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestBuffRaidStrategy(ai); };
                creators["buff shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestBuffPveStrategy(ai); };
                creators["buff shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestBuffPvpStrategy(ai); };
                creators["buff shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestBoostPveStrategy(ai); };
                creators["boost discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestBoostPvpStrategy(ai); };
                creators["boost discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestBoostRaidStrategy(ai); };
                creators["boost holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestBoostPveStrategy(ai); };
                creators["boost holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestBoostPvpStrategy(ai); };
                creators["boost holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestBoostRaidStrategy(ai); };
                creators["boost shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestBoostPveStrategy(ai); };
                creators["boost shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestBoostPvpStrategy(ai); };
                creators["boost shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestBoostRaidStrategy(ai); };
            }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = [](PlayerbotAI* ai) { return new PriestOffhealPveStrategy(ai); };
                creators["offheal pvp"] = [](PlayerbotAI* ai) { return new PriestOffhealPvpStrategy(ai); };
                creators["offheal raid"] = [](PlayerbotAI* ai) { return new PriestOffhealRaidStrategy(ai); };
            }
        };

        class OffdpsSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffdpsSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offdps pve"] = [](PlayerbotAI* ai) { return new PriestOffdpsPveStrategy(ai); };
                creators["offdps pvp"] = [](PlayerbotAI* ai) { return new PriestOffdpsPvpStrategy(ai); };
                creators["offdps raid"] = [](PlayerbotAI* ai) { return new PriestOffdpsRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestCcPveStrategy(ai); };
                creators["cc holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestCcPvpStrategy(ai); };
                creators["cc holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestCcRaidStrategy(ai); };
                creators["cc shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestCcPveStrategy(ai); };
                creators["cc shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestCcPvpStrategy(ai); };
                creators["cc shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestCcRaidStrategy(ai); };
                creators["cc discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestCcPveStrategy(ai); };
                creators["cc discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestCcPvpStrategy(ai); };
                creators["cc discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestCcRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["holy"] = [](PlayerbotAI* ai) { return new HolyPriestPlaceholderStrategy(ai); };
                creators["shadow"] = [](PlayerbotAI* ai) { return new ShadowPriestPlaceholderStrategy(ai); };
                creators["discipline"] = [](PlayerbotAI* ai) { return new DisciplinePriestPlaceholderStrategy(ai); };
                creators["heal"] = [](PlayerbotAI* ai) { return new HolyPriestPlaceholderStrategy(ai); };
                creators["dps"] = [](PlayerbotAI* ai) { return new ShadowPriestPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["discipline pvp"] = [](PlayerbotAI* ai) { return new DisciplinePriestPvpStrategy(ai); };
                creators["discipline pve"] = [](PlayerbotAI* ai) { return new DisciplinePriestPveStrategy(ai); };
                creators["discipline raid"] = [](PlayerbotAI* ai) { return new DisciplinePriestRaidStrategy(ai); };
                creators["holy pvp"] = [](PlayerbotAI* ai) { return new HolyPriestPvpStrategy(ai); };
                creators["holy pve"] = [](PlayerbotAI* ai) { return new HolyPriestPveStrategy(ai); };
                creators["holy raid"] = [](PlayerbotAI* ai) { return new HolyPriestRaidStrategy(ai); };
                creators["shadow pvp"] = [](PlayerbotAI* ai) { return new ShadowPriestPvpStrategy(ai); };
                creators["shadow pve"] = [](PlayerbotAI* ai) { return new ShadowPriestPveStrategy(ai); };
                creators["shadow raid"] = [](PlayerbotAI* ai) { return new ShadowPriestRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["devouring plague"] = [](PlayerbotAI* ai) { return new DevouringPlagueTrigger(ai); };
                creators["shadow word: pain"] = [](PlayerbotAI* ai) { return new PowerWordPainTrigger(ai); };
                creators["shadow word: pain on attacker"] = [](PlayerbotAI* ai) { return new PowerWordPainOnAttackerTrigger(ai); };
                creators["dispel magic"] = [](PlayerbotAI* ai) { return new DispelMagicTrigger(ai); };
                creators["dispel magic on party"] = [](PlayerbotAI* ai) { return new DispelMagicPartyMemberTrigger(ai); };
                creators["cure disease"] = [](PlayerbotAI* ai) { return new CureDiseaseTrigger(ai); };
                creators["party member cure disease"] = [](PlayerbotAI* ai) { return new PartyMemberCureDiseaseTrigger(ai); };
                creators["power word: fortitude"] = [](PlayerbotAI* ai) { return new PowerWordFortitudeTrigger(ai); };
                creators["power word: fortitude on party"] = [](PlayerbotAI* ai) { return new PowerWordFortitudeOnPartyTrigger(ai); };
                creators["divine spirit"] = [](PlayerbotAI* ai) { return new DivineSpiritTrigger(ai); };
                creators["divine spirit on party"] = [](PlayerbotAI* ai) { return new DivineSpiritOnPartyTrigger(ai); };
                creators["inner fire"] = [](PlayerbotAI* ai) { return new InnerFireTrigger(ai); };
                creators["vampiric touch"] = [](PlayerbotAI* ai) { return new VampiricTouchTrigger(ai); };
                creators["vampiric touch on attacker"] = [](PlayerbotAI* ai) { return new VampiricTouchOnAttackerTrigger(ai); };
                creators["vampiric embrace"] = [](PlayerbotAI* ai) { return new VampiricEmbraceTrigger(ai); };
                creators["shadowform"] = [](PlayerbotAI* ai) { return new ShadowformTrigger(ai); };
                creators["power infusion"] = [](PlayerbotAI* ai) { return new PowerInfusionTrigger(ai); };
                creators["inner focus"] = [](PlayerbotAI* ai) { return new InnerFocusTrigger(ai); };
                creators["shadow protection"] = [](PlayerbotAI* ai) { return new ShadowProtectionTrigger(ai); };
                creators["shadow protection on party"] = [](PlayerbotAI* ai) { return new ShadowProtectionOnPartyTrigger(ai); };
                creators["prayer of shadow protection on party"] = [](PlayerbotAI* ai) { return new PrayerOfShadowProtectionOnPartyTrigger(ai); };
                creators["prayer of fortitude on party"] = [](PlayerbotAI* ai) { return new PrayerOfFortitudeOnPartyTrigger(ai); };
                creators["prayer of spirit on party"] = [](PlayerbotAI* ai) { return new PrayerOfSpiritOnPartyTrigger(ai); };
                creators["shackle undead"] = [](PlayerbotAI* ai) { return new ShackleUndeadTrigger(ai); };
                creators["discipline fire"] = [](PlayerbotAI* ai) { return new HolyFireTrigger(ai); };
                creators["touch of weakness"] = [](PlayerbotAI* ai) { return new TouchOfWeaknessTrigger(ai); };
                creators["hex of weakness"] = [](PlayerbotAI* ai) { return new HexOfWeaknessTrigger(ai); };
                creators["shadowguard"] = [](PlayerbotAI* ai) { return new ShadowguardTrigger(ai); };
                creators["starshards"] = [](PlayerbotAI* ai) { return new StarshardsTrigger(ai); };
                creators["fear ward"] = [](PlayerbotAI* ai) { return new FearWardTrigger(ai); };
                creators["feedback"] = [](PlayerbotAI* ai) { return new FeedbackTrigger(ai); };
                creators["binding heal"] = [](PlayerbotAI* ai) { return new BindingHealTrigger(ai); };
                creators["chastise"] = [](PlayerbotAI* ai) { return new ChastiseTrigger(ai); };
                creators["silence"] = [](PlayerbotAI* ai) { return new SilenceTrigger(ai); };
                creators["silence on enemy healer"] = [](PlayerbotAI* ai) { return new SilenceEnemyHealerTrigger(ai); };
                creators["shadowfiend"] = [](PlayerbotAI* ai) { return new ShadowfiendTrigger(ai); };
                creators["mind blast"] = [](PlayerbotAI* ai) { return new MindBlastTrigger(ai); };
                creators["smite"] = [](PlayerbotAI* ai) { return new SmiteTrigger(ai); };
                creators["holy fire"] = [](PlayerbotAI* ai) { return new HolyFireTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["power infusion"] = [](PlayerbotAI* ai) { return new CastPowerInfusionAction(ai); };
                creators["inner focus"] = [](PlayerbotAI* ai) { return new CastInnerFocusAction(ai); };
                creators["shadow word: pain"] = [](PlayerbotAI* ai) { return new CastPowerWordPainAction(ai); };
                creators["shadow word: pain on attacker"] = [](PlayerbotAI* ai) { return new CastPowerWordPainOnAttackerAction(ai); };
                creators["devouring plague"] = [](PlayerbotAI* ai) { return new CastDevouringPlagueAction(ai); };
                creators["mind flay"] = [](PlayerbotAI* ai) { return new CastMindFlayAction(ai); };
                creators["discipline fire"] = [](PlayerbotAI* ai) { return new CastHolyFireAction(ai); };
                creators["smite"] = [](PlayerbotAI* ai) { return new CastSmiteAction(ai); };
                creators["mind blast"] = [](PlayerbotAI* ai) { return new CastMindBlastAction(ai); };
                creators["shadowform"] = [](PlayerbotAI* ai) { return new CastShadowformAction(ai); };
                creators["remove shadowform"] = [](PlayerbotAI* ai) { return new CastRemoveShadowformAction(ai); };
                creators["discipline nova"] = [](PlayerbotAI* ai) { return new CastHolyNovaAction(ai); };
                creators["power word: fortitude"] = [](PlayerbotAI* ai) { return new CastPowerWordFortitudeAction(ai); };
                creators["power word: fortitude on party"] = [](PlayerbotAI* ai) { return new CastPowerWordFortitudeOnPartyAction(ai); };
                creators["divine spirit"] = [](PlayerbotAI* ai) { return new CastDivineSpiritAction(ai); };
                creators["divine spirit on party"] = [](PlayerbotAI* ai) { return new CastDivineSpiritOnPartyAction(ai); };
                creators["power word: shield"] = [](PlayerbotAI* ai) { return new CastPowerWordShieldAction(ai); };
                creators["power word: shield on party"] = [](PlayerbotAI* ai) { return new CastPowerWordShieldOnPartyAction(ai); };
                creators["renew"] = [](PlayerbotAI* ai) { return new CastRenewAction(ai); };
                creators["renew on party"] = [](PlayerbotAI* ai) { return new CastRenewOnPartyAction(ai); };
                creators["greater heal"] = [](PlayerbotAI* ai) { return new CastGreaterHealAction(ai); };
                creators["greater heal on party"] = [](PlayerbotAI* ai) { return new CastGreaterHealOnPartyAction(ai); };
                creators["heal"] = [](PlayerbotAI* ai) { return new CastHealAction(ai); };
                creators["heal on party"] = [](PlayerbotAI* ai) { return new CastHealOnPartyAction(ai); };
                creators["lesser heal"] = [](PlayerbotAI* ai) { return new CastLesserHealAction(ai); };
                creators["lesser heal on party"] = [](PlayerbotAI* ai) { return new CastLesserHealOnPartyAction(ai); };
                creators["flash heal"] = [](PlayerbotAI* ai) { return new CastFlashHealAction(ai); };
                creators["flash heal on party"] = [](PlayerbotAI* ai) { return new CastFlashHealOnPartyAction(ai); };
                creators["dispel magic"] = [](PlayerbotAI* ai) { return new CastDispelMagicAction(ai); };
                creators["dispel magic on party"] = [](PlayerbotAI* ai) { return new CastDispelMagicOnPartyAction(ai); };
                creators["dispel magic on target"] = [](PlayerbotAI* ai) { return new CastDispelMagicOnTargetAction(ai); };
                creators["cure disease"] = [](PlayerbotAI* ai) { return new CastCureDiseaseAction(ai); };
                creators["cure disease on party"] = [](PlayerbotAI* ai) { return new CastCureDiseaseOnPartyAction(ai); };
                creators["abolish disease"] = [](PlayerbotAI* ai) { return new CastAbolishDiseaseAction(ai); };
                creators["abolish disease on party"] = [](PlayerbotAI* ai) { return new CastAbolishDiseaseOnPartyAction(ai); };
                creators["fade"] = [](PlayerbotAI* ai) { return new CastFadeAction(ai); };
                creators["inner fire"] = [](PlayerbotAI* ai) { return new CastInnerFireAction(ai); };
                creators["resurrection"] = [](PlayerbotAI* ai) { return new CastResurrectionAction(ai); };
                creators["circle of healing"] = [](PlayerbotAI* ai) { return new CastCircleOfHealingAction(ai); };
                creators["psychic scream"] = [](PlayerbotAI* ai) { return new CastPsychicScreamAction(ai); };
                creators["vampiric touch"] = [](PlayerbotAI* ai) { return new CastVampiricTouchAction(ai); };
                creators["vampiric touch on attacker"] = [](PlayerbotAI* ai) { return new CastVampiricTouchActionOnAttacker(ai); };
                creators["vampiric embrace"] = [](PlayerbotAI* ai) { return new CastVampiricEmbraceAction(ai); };
                //creators["dispersion"] = [](PlayerbotAI* ai) { return new CastDispersionAction(ai); };
                creators["shadow protection"] = [](PlayerbotAI* ai) { return new CastShadowProtectionAction(ai); };
                creators["shadow protection on party"] = [](PlayerbotAI* ai) { return new CastShadowProtectionOnPartyAction(ai); };
                creators["shackle undead"] = [](PlayerbotAI* ai) { return new CastShackleUndeadAction(ai); };
                creators["prayer of fortitude on party"] = [](PlayerbotAI* ai) { return new CastPrayerOfFortitudeOnPartyAction(ai); };
                creators["prayer of spirit on party"] = [](PlayerbotAI* ai) { return new CastPrayerOfSpiritOnPartyAction(ai); };
                creators["prayer of shadow protection on party"] = [](PlayerbotAI* ai) { return new CastPrayerOfShadowProtectionAction(ai); };
                creators["silence"] = [](PlayerbotAI* ai) { return new CastSilenceAction(ai); };
                creators["silence on enemy healer"] = [](PlayerbotAI* ai) { return new CastSilenceOnEnemyHealerAction(ai); };
                creators["mana burn"] = [](PlayerbotAI* ai) { return new CastManaBurnAction(ai); };
                creators["levitate"] = [](PlayerbotAI* ai) { return new CastLevitateAction(ai); };
                creators["prayer of healing"] = [](PlayerbotAI* ai) { return new CastPrayerOfHealingAction(ai); };
                creators["lightwell"] = [](PlayerbotAI* ai) { return new CastLightwellAction(ai); };
                creators["mind soothe"] = [](PlayerbotAI* ai) { return new CastMindSootheAction(ai); };
                creators["touch of weakness"] = [](PlayerbotAI* ai) { return new CastTouchOfWeaknessAction(ai); };
                creators["hex of weakness"] = [](PlayerbotAI* ai) { return new CastHexOfWeaknessAction(ai); };
                creators["shadowguard"] = [](PlayerbotAI* ai) { return new CastShadowguardAction(ai); };
                creators["desperate prayer"] = [](PlayerbotAI* ai) { return new CastDesperatePrayerAction(ai); };
                creators["fear ward"] = [](PlayerbotAI* ai) { return new CastFearWardAction(ai); };
                creators["starshards"] = [](PlayerbotAI* ai) { return new CastStarshardsAction(ai); };
                creators["elune's grace"] = [](PlayerbotAI* ai) { return new CastElunesGraceAction(ai); };
                creators["feedback"] = [](PlayerbotAI* ai) { return new CastFeedbackAction(ai); };
                creators["symbol of hope"] = [](PlayerbotAI* ai) { return new CastSymbolOfHopeAction(ai); };
                creators["consume magic"] = [](PlayerbotAI* ai) { return new CastConsumeMagicAction(ai); };
                creators["chastise"] = [](PlayerbotAI* ai) { return new CastChastiseAction(ai); };
                creators["shadow word: death"] = [](PlayerbotAI* ai) { return new CastShadowWordDeathAction(ai); };
                creators["shadowfiend"] = [](PlayerbotAI* ai) { return new CastShadowfiendAction(ai); };
                creators["mass dispel"] = [](PlayerbotAI* ai) { return new CastMassDispelAction(ai); };
                creators["pain suppression"] = [](PlayerbotAI* ai) { return new CastPainSuppressionAction(ai); };
                creators["pain suppression on party"] = [](PlayerbotAI* ai) { return new CastPainSuppressionProtectAction(ai); };
                creators["prayer of mending"] = [](PlayerbotAI* ai) { return new CastPrayerOfMendingAction(ai); };
                creators["binding heal"] = [](PlayerbotAI* ai) { return new CastBindingHealAction(ai); };
                creators["holy fire"] = [](PlayerbotAI* ai) { return new CastHolyFireAction(ai); };
                creators["holy nova"] = [](PlayerbotAI* ai) { return new CastHolyNovaAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdatePriestPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdatePriestPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdatePriestRaidStrategiesAction(ai); };
            }
        };
    };
};

PriestAiObjectContext::PriestAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::priest::StrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::OffdpsSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::priest::AiObjectContextInternal());
    triggerContexts.Add(new ai::priest::TriggerFactoryInternal());
}
