#include "playerbot/playerbot.h"
#include "PaladinActions.h"
#include "PaladinTriggers.h"
#include "PaladinAiObjectContext.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "HolyPaladinStrategy.h"
#include "ProtectionPaladinStrategy.h"
#include "RetributionPaladinStrategy.h"

namespace ai
{
    namespace paladin
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["cure"] = [](PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
                creators["offheal"] = [](PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
#ifdef MANGOSBOT_TWO
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "judgement of light", "seal of righteousness"); };
#else
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "judgement", "seal of righteousness"); };
#endif
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinAoePveStrategy(ai); };
                creators["aoe holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinAoePvpStrategy(ai); };
                creators["aoe holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinAoeRaidStrategy(ai); };
                creators["aoe retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinAoePveStrategy(ai); };
                creators["aoe retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinAoePvpStrategy(ai); };
                creators["aoe retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinAoeRaidStrategy(ai); };
                creators["aoe protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAoePveStrategy(ai); };
                creators["aoe protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAoePvpStrategy(ai); };
                creators["aoe protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAoeRaidStrategy(ai); };
            }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinCurePveStrategy(ai); };
                creators["cure holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinCurePvpStrategy(ai); };
                creators["cure holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinCureRaidStrategy(ai); };
                creators["cure retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinCurePveStrategy(ai); };
                creators["cure retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinCurePvpStrategy(ai); };
                creators["cure retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinCureRaidStrategy(ai); };
                creators["cure protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCurePveStrategy(ai); };
                creators["cure protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCurePvpStrategy(ai); };
                creators["cure protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCureRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinBuffPveStrategy(ai); };
                creators["buff holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinBuffPvpStrategy(ai); };
                creators["buff holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinBuffRaidStrategy(ai); };
                creators["buff retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinBuffPveStrategy(ai); };
                creators["buff retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinBuffPvpStrategy(ai); };
                creators["buff retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinBuffRaidStrategy(ai); };
                creators["buff protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBuffPveStrategy(ai); };
                creators["buff protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBuffPvpStrategy(ai); };
                creators["buff protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinBoostPveStrategy(ai); };
                creators["boost holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinBoostPvpStrategy(ai); };
                creators["boost holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinBoostRaidStrategy(ai); };
                creators["boost retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinBoostPveStrategy(ai); };
                creators["boost retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinBoostPvpStrategy(ai); };
                creators["boost retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinBoostRaidStrategy(ai); };
                creators["boost protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBoostPveStrategy(ai); };
                creators["boost protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBoostPvpStrategy(ai); };
                creators["boost protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBoostRaidStrategy(ai); };
            }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = [](PlayerbotAI* ai) { return new PaladinOffhealPveStrategy(ai); };
                creators["offheal pvp"] = [](PlayerbotAI* ai) { return new PaladinOffhealPvpStrategy(ai); };
                creators["offheal raid"] = [](PlayerbotAI* ai) { return new PaladinOffhealRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinCcPveStrategy(ai); };
                creators["cc retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinCcPvpStrategy(ai); };
                creators["cc retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinCcRaidStrategy(ai); };
                creators["cc protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCcPveStrategy(ai); };
                creators["cc protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCcPvpStrategy(ai); };
                creators["cc protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinCcRaidStrategy(ai); };
                creators["cc holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinCcPveStrategy(ai); };
                creators["cc holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinCcPvpStrategy(ai); };
                creators["cc holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinCcRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["retribution"] = [](PlayerbotAI* ai) { return new RetributionPaladinPlaceholderStrategy(ai); };
                creators["protection"] = [](PlayerbotAI* ai) { return new ProtectionPaladinPlaceholderStrategy(ai); };
                creators["tank"] = [](PlayerbotAI* ai) { return new ProtectionPaladinPlaceholderStrategy(ai); };
                creators["holy"] = [](PlayerbotAI* ai) { return new HolyPaladinPlaceholderStrategy(ai); };
                creators["heal"] = [](PlayerbotAI* ai) { return new HolyPaladinPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinPvpStrategy(ai); };
                creators["holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinPveStrategy(ai); };
                creators["holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinRaidStrategy(ai); };
                creators["retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinPvpStrategy(ai); };
                creators["retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinPveStrategy(ai); };
                creators["retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinRaidStrategy(ai); };
                creators["protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinPvpStrategy(ai); };
                creators["protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinPveStrategy(ai); };
                creators["protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinRaidStrategy(ai); };
            }
        };

        class AuraManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AuraManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aura"] = [](PlayerbotAI* ai) { return new PaladinAuraPlaceholderStrategy(ai); };
                creators["aura devotion"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura devotion", "devotion aura", "devotion aura"); };
                creators["aura retribution"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura retribution", "retribution aura", "retribution aura"); };
                creators["aura concentration"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura concentration", "concentration aura", "concentration aura"); };
                creators["aura shadow"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura shadow", "shadow resistance aura", "shadow resistance aura"); };
                creators["aura frost"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura frost", "frost resistance aura", "frost resistance aura"); };
                creators["aura fire"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura fire", "fire resistance aura", "fire resistance aura"); };
                creators["aura crusader"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura crusader", "crusader aura", "crusader aura"); };
                creators["aura sanctity"] = [](PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura sanctity", "sanctity aura", "sanctity aura"); };
            }
        };

        class AuraSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AuraSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aura retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinAuraPveStrategy(ai); };
                creators["aura retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinAuraPvpStrategy(ai); };
                creators["aura retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinAuraRaidStrategy(ai); };
                creators["aura protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAuraPveStrategy(ai); };
                creators["aura protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAuraPvpStrategy(ai); };
                creators["aura protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinAuraRaidStrategy(ai); };
                creators["aura holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinAuraPveStrategy(ai); };
                creators["aura holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinAuraPvpStrategy(ai); };
                creators["aura holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinAuraRaidStrategy(ai); };
            }
        };

        class BlessingManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BlessingManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["blessing"] = [](PlayerbotAI* ai) { return new PaladinBlessingPlaceholderStrategy(ai); };
                creators["blessing might"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing might", "blessing of might", "blessing of might"); };
                creators["blessing wisdom"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing wisdom", "blessing of wisdom", "blessing of wisdom"); };
                creators["blessing kings"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing kings", "blessing of kings", "blessing of kings"); };
                creators["blessing sanctuary"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing sanctuary", "blessing of sanctuary", "blessing of sanctuary"); };
                creators["blessing light"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing light", "blessing of light", "blessing of light"); };
                creators["blessing salvation"] = [](PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing salvation", "blessing of salvation", "blessing of salvation"); };
            }
        };

        class BlessingSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BlessingSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["blessing retribution pve"] = [](PlayerbotAI* ai) { return new RetributionPaladinBlessingPveStrategy(ai); };
                creators["blessing retribution pvp"] = [](PlayerbotAI* ai) { return new RetributionPaladinBlessingPvpStrategy(ai); };
                creators["blessing retribution raid"] = [](PlayerbotAI* ai) { return new RetributionPaladinBlessingRaidStrategy(ai); };
                creators["blessing protection pve"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBlessingPveStrategy(ai); };
                creators["blessing protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBlessingPvpStrategy(ai); };
                creators["blessing protection raid"] = [](PlayerbotAI* ai) { return new ProtectionPaladinBlessingRaidStrategy(ai); };
                creators["blessing holy pve"] = [](PlayerbotAI* ai) { return new HolyPaladinBlessingPveStrategy(ai); };
                creators["blessing holy pvp"] = [](PlayerbotAI* ai) { return new HolyPaladinBlessingPvpStrategy(ai); };
                creators["blessing holy raid"] = [](PlayerbotAI* ai) { return new HolyPaladinBlessingRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["judgement"] = [](PlayerbotAI* ai) { return new JudgementTrigger(ai); };
                creators["judgement of wisdom"] = [](PlayerbotAI* ai) { return new JudgementOfWisdomTrigger(ai); };
                creators["judgement of light"] = [](PlayerbotAI* ai) { return new JudgementOfLightTrigger(ai); };
                creators["seal"] = [](PlayerbotAI* ai) { return new SealTrigger(ai); };
                creators["art of war"] = [](PlayerbotAI* ai) { return new ArtOfWarTrigger(ai); };
                creators["blessing"] = [](PlayerbotAI* ai) { return new BlessingTrigger(ai); };
                creators["greater blessing"] = [](PlayerbotAI* ai) { return new GreaterBlessingTrigger(ai); };
                creators["blessing of might"] = [](PlayerbotAI* ai) { return new BlessingOfMightTrigger(ai); };
                creators["blessing of wisdom"] = [](PlayerbotAI* ai) { return new BlessingOfWisdomTrigger(ai); };
                creators["blessing of kings"] = [](PlayerbotAI* ai) { return new BlessingOfKingsTrigger(ai); };
                creators["blessing of sanctuary"] = [](PlayerbotAI* ai) { return new BlessingOfSanctuaryTrigger(ai); };
                creators["blessing of light"] = [](PlayerbotAI* ai) { return new BlessingOfLightTrigger(ai); };
                creators["blessing of salvation"] = [](PlayerbotAI* ai) { return new BlessingOfSalvationTrigger(ai); };
                creators["greater blessing of might"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfMightTrigger(ai); };
                creators["greater blessing of wisdom"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfWisdomTrigger(ai); };
                creators["greater blessing of kings"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfKingsTrigger(ai); };
                creators["greater blessing of sanctuary"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfSanctuaryTrigger(ai); };
                creators["greater blessing of light"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfLightTrigger(ai); };
                creators["greater blessing of salvation"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfSalvationTrigger(ai); };
                creators["blessing on party"] = [](PlayerbotAI* ai) { return new BlessingOnPartyTrigger(ai); };
                creators["greater blessing on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOnPartyTrigger(ai); };
                creators["blessing of might on party"] = [](PlayerbotAI* ai) { return new BlessingOfMightOnPartyTrigger(ai); };
                creators["blessing of wisdom on party"] = [](PlayerbotAI* ai) { return new BlessingOfWisdomOnPartyTrigger(ai); };
                creators["blessing of kings on party"] = [](PlayerbotAI* ai) { return new BlessingOfKingsOnPartyTrigger(ai); };
                creators["blessing of sanctuary on party"] = [](PlayerbotAI* ai) { return new BlessingOfSanctuaryOnPartyTrigger(ai); };
                creators["blessing of light on party"] = [](PlayerbotAI* ai) { return new BlessingOfLightOnPartyTrigger(ai); };
                creators["blessing of salvation on party"] = [](PlayerbotAI* ai) { return new BlessingOfSalvationOnPartyTrigger(ai); };
                creators["greater blessing of might on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfMightOnPartyTrigger(ai); };
                creators["greater blessing of wisdom on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfWisdomOnPartyTrigger(ai); };
                creators["greater blessing of kings on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfKingsOnPartyTrigger(ai); };
                creators["greater blessing of sanctuary on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfSanctuaryOnPartyTrigger(ai); };
                creators["greater blessing of light on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfLightOnPartyTrigger(ai); };
                creators["greater blessing of salvation on party"] = [](PlayerbotAI* ai) { return new GreaterBlessingOfSalvationOnPartyTrigger(ai); };
                creators["no paladin aura"] = [](PlayerbotAI* ai) { return new NoPaladinAuraTrigger(ai); };
                creators["crusader aura"] = [](PlayerbotAI* ai) { return new CrusaderAuraTrigger(ai); };
                creators["retribution aura"] = [](PlayerbotAI* ai) { return new RetributionAuraTrigger(ai); };
                creators["devotion aura"] = [](PlayerbotAI* ai) { return new DevotionAuraTrigger(ai); };
                creators["sanctity aura"] = [](PlayerbotAI* ai) { return new SanctityAuraTrigger(ai); };
                creators["concentration aura"] = [](PlayerbotAI* ai) { return new ConcentrationAuraTrigger(ai); };
                creators["shadow resistance aura"] = [](PlayerbotAI* ai) { return new ShadowResistanceAuraTrigger(ai); };
                creators["fire resistance aura"] = [](PlayerbotAI* ai) { return new FireResistanceAuraTrigger(ai); };
                creators["frost resistance aura"] = [](PlayerbotAI* ai) { return new FrostResistanceAuraTrigger(ai); };
                creators["hammer of justice snare"] = [](PlayerbotAI* ai) { return new HammerOfJusticeSnareTrigger(ai); };
                creators["hammer of justice interrupt"] = [](PlayerbotAI* ai) { return new HammerOfJusticeInterruptSpellTrigger(ai); };
                creators["cleanse cure disease"] = [](PlayerbotAI* ai) { return new CleanseCureDiseaseTrigger(ai); };
                creators["cleanse party member cure disease"] = [](PlayerbotAI* ai) { return new CleanseCurePartyMemberDiseaseTrigger(ai); };
                creators["cleanse cure poison"] = [](PlayerbotAI* ai) { return new CleanseCurePoisonTrigger(ai); };
                creators["cleanse party member cure poison"] = [](PlayerbotAI* ai) { return new CleanseCurePartyMemberPoisonTrigger(ai); };
                creators["cleanse cure magic"] = [](PlayerbotAI* ai) { return new CleanseCureMagicTrigger(ai); };
                creators["cleanse party member cure magic"] = [](PlayerbotAI* ai) { return new CleanseCurePartyMemberMagicTrigger(ai); };
                creators["righteous fury"] = [](PlayerbotAI* ai) { return new RighteousFuryTrigger(ai); };
                creators["holy shield"] = [](PlayerbotAI* ai) { return new HolyShieldTrigger(ai); };
                creators["hammer of justice on enemy healer"] = [](PlayerbotAI* ai) { return new HammerOfJusticeEnemyHealerTrigger(ai); };
                creators["hammer of justice on snare target"] = [](PlayerbotAI* ai) { return new HammerOfJusticeSnareTrigger(ai); };
                creators["divine favor"] = [](PlayerbotAI* ai) { return new DivineFavorTrigger(ai); };
                creators["turn undead"] = [](PlayerbotAI* ai) { return new TurnUndeadTrigger(ai); };
                creators["avenger's shield"] = [](PlayerbotAI* ai) { return new AvengerShieldTrigger(ai); };
                creators["consecration"] = [](PlayerbotAI* ai) { return new ConsecrationTrigger(ai); };
                creators["exorcism"] = [](PlayerbotAI* ai) { return new ExorcismTrigger(ai); };
                creators["repentance on enemy healer"] = [](PlayerbotAI* ai) { return new RepentanceOnHealerTrigger(ai); };
                creators["repentance on snare target"] = [](PlayerbotAI* ai) { return new RepentanceSnareTrigger(ai); };
                creators["repentance interrupt"] = [](PlayerbotAI* ai) { return new RepentanceInterruptTrigger(ai); };
                creators["hammer of justice on enemy"] = [](PlayerbotAI* ai) { return new HammerOfJusticeOnEnemyTrigger(ai); };
                creators["hand of sacrifice"] = [](PlayerbotAI* ai) { return new HandOfSacrificeTrigger(ai); };
                creators["blessing of sacrifice"] = [](PlayerbotAI* ai) { return new BlessingOfSacrificeTrigger(ai); };
                creators["crusader strike"] = [](PlayerbotAI* ai) { return new CrusaderStrikeTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["seal of command"] = [](PlayerbotAI* ai) { return new CastSealOfCommandAction(ai); };
                creators["seal of vengeance"] = [](PlayerbotAI* ai) { return new CastSealOfVengeanceAction(ai); };
                creators["pve blessing"] = [](PlayerbotAI* ai) { return new CastPveBlessingAction(ai); };
                creators["pve greater blessing"] = [](PlayerbotAI* ai) { return new CastPveGreaterBlessingAction(ai); };
                creators["pvp blessing"] = [](PlayerbotAI* ai) { return new CastPvpBlessingAction(ai); };
                creators["pvp greater blessing"] = [](PlayerbotAI* ai) { return new CastPvpGreaterBlessingAction(ai); };
                creators["raid blessing"] = [](PlayerbotAI* ai) { return new CastRaidBlessingAction(ai); };
                creators["raid greater blessing"] = [](PlayerbotAI* ai) { return new CastRaidGreaterBlessingAction(ai); };
                creators["pve blessing on party"] = [](PlayerbotAI* ai) { return new CastPveBlessingOnPartyAction(ai); };
                creators["pve greater blessing on party"] = [](PlayerbotAI* ai) { return new CastPveGreaterBlessingOnPartyAction(ai); };
                creators["pvp blessing on party"] = [](PlayerbotAI* ai) { return new CastPvpBlessingOnPartyAction(ai); };
                creators["pvp greater blessing on party"] = [](PlayerbotAI* ai) { return new CastPvpGreaterBlessingOnPartyAction(ai); };
                creators["raid blessing on party"] = [](PlayerbotAI* ai) { return new CastRaidBlessingOnPartyAction(ai); };
                creators["raid greater blessing on party"] = [](PlayerbotAI* ai) { return new CastRaidGreaterBlessingOnPartyAction(ai); };
                creators["blessing of might"] = [](PlayerbotAI* ai) { return new CastBlessingOfMightAction(ai); };
                creators["greater blessing of might"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfMightAction(ai); };
                creators["blessing of might on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfMightOnPartyAction(ai); };
                creators["greater blessing of might on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfMightOnPartyAction(ai); };
                creators["blessing of wisdom"] = [](PlayerbotAI* ai) { return new CastBlessingOfWisdomAction(ai); };
                creators["greater blessing of wisdom"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfWisdomAction(ai); };
                creators["blessing of wisdom on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfWisdomOnPartyAction(ai); };
                creators["greater blessing of wisdom on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfWisdomOnPartyAction(ai); };
                creators["blessing of kings"] = [](PlayerbotAI* ai) { return new CastBlessingOfKingsAction(ai); };
                creators["greater blessing of kings"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfKingsAction(ai); };
                creators["blessing of kings on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfKingsOnPartyAction(ai); };
                creators["greater blessing of kings on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfKingsOnPartyAction(ai); };
                creators["blessing of sanctuary"] = [](PlayerbotAI* ai) { return new CastBlessingOfSanctuaryAction(ai); };
                creators["greater blessing of sanctuary"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfSanctuaryAction(ai); };
                creators["blessing of sanctuary on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfSanctuaryOnPartyAction(ai); };
                creators["greater blessing of sanctuary on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfSanctuaryOnPartyAction(ai); };
                creators["blessing of light"] = [](PlayerbotAI* ai) { return new CastBlessingOfLightAction(ai); };
                creators["greater blessing of light"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfLightAction(ai); };
                creators["blessing of light on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfLightOnPartyAction(ai); };
                creators["greater blessing of light on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfLightOnPartyAction(ai); };
                creators["blessing of salvation"] = [](PlayerbotAI* ai) { return new CastBlessingOfSalvationAction(ai); };
                creators["greater blessing of salvation"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfSalvationAction(ai); };
                creators["blessing of salvation on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfSalvationOnPartyAction(ai); };
                creators["greater blessing of salvation on party"] = [](PlayerbotAI* ai) { return new CastGreaterBlessingOfSalvationOnPartyAction(ai); };
                creators["paladin aura"] = [](PlayerbotAI* ai) { return new CastPaladinAuraAction(ai); };
                creators["devotion aura"] = [](PlayerbotAI* ai) { return new CastDevotionAuraAction(ai); };
                creators["concentration aura"] = [](PlayerbotAI* ai) { return new CastConcentrationAuraAction(ai); };
                creators["crusader aura"] = [](PlayerbotAI* ai) { return new CastCrusaderAuraAction(ai); };
                creators["divine storm"] = [](PlayerbotAI* ai) { return new CastDivineStormAction(ai); };
                creators["redemption"] = [](PlayerbotAI* ai) { return new CastRedemptionAction(ai); };
                creators["crusader strike"] = [](PlayerbotAI* ai) { return new CastCrusaderStrikeAction(ai); };
                creators["retribution aura"] = [](PlayerbotAI* ai) { return new CastRetributionAuraAction(ai); };
                creators["shadow resistance aura"] = [](PlayerbotAI* ai) { return new CastShadowResistanceAuraAction(ai); };
                creators["fire resistance aura"] = [](PlayerbotAI* ai) { return new CastFireResistanceAuraAction(ai); };
                creators["frost resistance aura"] = [](PlayerbotAI* ai) { return new CastFrostResistanceAuraAction(ai); };
                creators["sanctity aura"] = [](PlayerbotAI* ai) { return new CastSanctityAuraAction(ai); };
                creators["seal of light"] = [](PlayerbotAI* ai) { return new CastSealOfLightAction(ai); };
                creators["holy wrath"] = [](PlayerbotAI* ai) { return new CastHolyWrathAction(ai); };
                creators["consecration"] = [](PlayerbotAI* ai) { return new CastConsecrationAction(ai); };
                creators["cleanse disease"] = [](PlayerbotAI* ai) { return new CastCleanseDiseaseAction(ai); };
                creators["cleanse poison"] = [](PlayerbotAI* ai) { return new CastCleansePoisonAction(ai); };
                creators["cleanse magic"] = [](PlayerbotAI* ai) { return new CastCleanseMagicAction(ai); };
                creators["purify disease"] = [](PlayerbotAI* ai) { return new CastPurifyDiseaseAction(ai); };
                creators["purify poison"] = [](PlayerbotAI* ai) { return new CastPurifyPoisonAction(ai); };
                creators["cleanse poison on party"] = [](PlayerbotAI* ai) { return new CastCleansePoisonOnPartyAction(ai); };
                creators["cleanse disease on party"] = [](PlayerbotAI* ai) { return new CastCleanseDiseaseOnPartyAction(ai); };
                creators["cleanse magic on party"] = [](PlayerbotAI* ai) { return new CastCleanseMagicOnPartyAction(ai); };
                creators["purify poison on party"] = [](PlayerbotAI* ai) { return new CastPurifyPoisonOnPartyAction(ai); };
                creators["purify disease on party"] = [](PlayerbotAI* ai) { return new CastPurifyDiseaseOnPartyAction(ai); };
                creators["seal of wisdom"] = [](PlayerbotAI* ai) { return new CastSealOfWisdomAction(ai); };
                creators["seal of justice"] = [](PlayerbotAI* ai) { return new CastSealOfJusticeAction(ai); };
                creators["seal of righteousness"] = [](PlayerbotAI* ai) { return new CastSealOfRighteousnessAction(ai); };
                creators["flash of light"] = [](PlayerbotAI* ai) { return new CastFlashOfLightAction(ai); };
                creators["hand of reckoning"] = [](PlayerbotAI* ai) { return new CastHandOfReckoningAction(ai); };
                creators["avenger's shield"] = [](PlayerbotAI* ai) { return new CastAvengersShieldAction(ai); };
                creators["exorcism"] = [](PlayerbotAI* ai) { return new CastExorcismAction(ai); };
                creators["judgement"] = [](PlayerbotAI* ai) { return new CastJudgementAction(ai); };
                creators["judgement of light"] = [](PlayerbotAI* ai) { return new CastJudgementOfLightAction(ai); };
                creators["judgement of wisdom"] = [](PlayerbotAI* ai) { return new CastJudgementOfWisdomAction(ai); };
                creators["divine shield"] = [](PlayerbotAI* ai) { return new CastDivineShieldAction(ai); };
                creators["divine protection"] = [](PlayerbotAI* ai) { return new CastDivineProtectionAction(ai); };
                creators["divine protection on party"] = [](PlayerbotAI* ai) { return new CastDivineProtectionOnPartyAction(ai); };
                creators["hammer of justice"] = [](PlayerbotAI* ai) { return new CastHammerOfJusticeAction(ai); };
                creators["flash of light on party"] = [](PlayerbotAI* ai) { return new CastFlashOfLightOnPartyAction(ai); };
                creators["holy light"] = [](PlayerbotAI* ai) { return new CastHolyLightAction(ai); };
                creators["holy light on party"] = [](PlayerbotAI* ai) { return new CastHolyLightOnPartyAction(ai); };
                creators["lay on hands"] = [](PlayerbotAI* ai) { return new CastLayOnHandsAction(ai); };
                creators["lay on hands on party"] = [](PlayerbotAI* ai) { return new CastLayOnHandsOnPartyAction(ai); };
                creators["judgement of justice"] = [](PlayerbotAI* ai) { return new CastJudgementOfJusticeAction(ai); };
                creators["hammer of wrath"] = [](PlayerbotAI* ai) { return new CastHammerOfWrathAction(ai); };
                creators["holy shield"] = [](PlayerbotAI* ai) { return new CastHolyShieldAction(ai); };
                creators["hammer of the righteous"] = [](PlayerbotAI* ai) { return new CastHammerOfTheRighteousAction(ai); };
                creators["righteous fury"] = [](PlayerbotAI* ai) { return new CastRighteousFuryAction(ai); };
                creators["hammer of justice on enemy healer"] = [](PlayerbotAI* ai) { return new CastHammerOfJusticeOnEnemyHealerAction(ai); };
                creators["hammer of justice on snare target"] = [](PlayerbotAI* ai) { return new CastHammerOfJusticeSnareAction(ai); };
                creators["divine favor"] = [](PlayerbotAI* ai) { return new CastDivineFavorAction(ai); };
                creators["turn undead"] = [](PlayerbotAI* ai) { return new CastTurnUndeadAction(ai); };
                creators["blessing of protection on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfProtectionOnPartyAction(ai); };
                creators["blessing of freedom on party"] = [](PlayerbotAI* ai) { return new CastBlessingOfFreedomOnPartyAction(ai); };
                creators["righteous defense"] = [](PlayerbotAI* ai) { return new CastRighteousDefenseAction(ai); };
                creators["repentance"] = [](PlayerbotAI* ai) { return new CastRepentanceAction(ai); };
                creators["repentance on snare target"] = [](PlayerbotAI* ai) { return new CastRepentanceSnareAction(ai); };
                creators["repentance on enemy healer"] = [](PlayerbotAI* ai) { return new CastRepentanceOnHealerAction(ai); };
                creators["holy shock"] = [](PlayerbotAI* ai) { return new CastHolyShockAction(ai); };
                creators["holy shock on party"] = [](PlayerbotAI* ai) { return new CastHolyShockOnPartyAction(ai); };
                creators["blessing of freedom"] = [](PlayerbotAI* ai) { return new CastBlessingOfFreedomAction(ai); };
                creators["avenging wrath"] = [](PlayerbotAI* ai) { return new CastAvengingWrathAction(ai); };
                creators["divine illumination"] = [](PlayerbotAI* ai) { return new CastDivineIlluminationAction(ai); };
                creators["hand of sacrifice"] = [](PlayerbotAI* ai) { return new CastHandOfSacrificeAction(ai); };
                creators["blessing of sacrifice"] = [](PlayerbotAI* ai) { return new CastBlessingOfSacrificeAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdatePaladinPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdatePaladinPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdatePaladinRaidStrategiesAction(ai); };
            }
        };
    };
};

PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::paladin::StrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AuraManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AuraSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BlessingManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BlessingSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::paladin::AiObjectContextInternal());
    triggerContexts.Add(new ai::paladin::TriggerFactoryInternal());
}
