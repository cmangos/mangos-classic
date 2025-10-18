
#include "playerbot/playerbot.h"
#include "HunterActions.h"
#include "HunterTriggers.h"
#include "HunterAiObjectContext.h"
#include "BeastMasteryHunterStrategy.h"
#include "MarksmanshipHunterStrategy.h"
#include "SurvivalHunterStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    namespace hunter
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "serpent sting"); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["pet"] = [](PlayerbotAI* ai) { return new HunterPetStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAoePveStrategy(ai); };
                creators["aoe beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAoePvpStrategy(ai); };
                creators["aoe beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAoeRaidStrategy(ai); };
                creators["aoe marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAoePveStrategy(ai); };
                creators["aoe marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAoePvpStrategy(ai); };
                creators["aoe marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAoeRaidStrategy(ai); };
                creators["aoe survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterAoePveStrategy(ai); };
                creators["aoe survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterAoePvpStrategy(ai); };
                creators["aoe survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterAoeRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBuffPveStrategy(ai); };
                creators["buff beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBuffPvpStrategy(ai); };
                creators["buff beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBuffRaidStrategy(ai); };
                creators["buff marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBuffPveStrategy(ai); };
                creators["buff marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBuffPvpStrategy(ai); };
                creators["buff marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBuffRaidStrategy(ai); };
                creators["buff survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterBuffPveStrategy(ai); };
                creators["buff survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterBuffPvpStrategy(ai); };
                creators["buff survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBoostPveStrategy(ai); };
                creators["boost beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBoostPvpStrategy(ai); };
                creators["boost beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterBoostRaidStrategy(ai); };
                creators["boost marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBoostPveStrategy(ai); };
                creators["boost marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBoostPvpStrategy(ai); };
                creators["boost marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterBoostRaidStrategy(ai); };
                creators["boost survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterBoostPveStrategy(ai); };
                creators["boost survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterBoostPvpStrategy(ai); };
                creators["boost survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterBoostRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterCcPveStrategy(ai); };
                creators["cc marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterCcPvpStrategy(ai); };
                creators["cc marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterCcRaidStrategy(ai); };
                creators["cc survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterCcPveStrategy(ai); };
                creators["cc survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterCcPvpStrategy(ai); };
                creators["cc survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterCcRaidStrategy(ai); };
                creators["cc beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterCcPveStrategy(ai); };
                creators["cc beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterCcPvpStrategy(ai); };
                creators["cc beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterCcRaidStrategy(ai); };
            }
        };

        class StingManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StingManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["sting"] = [](PlayerbotAI* ai) { return new HunterStingPlaceholderStrategy(ai); };
                creators["sting serpent"] = [](PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting serpent", "serpent sting", "serpent sting"); };
                creators["sting scorpid"] = [](PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting scorpid", "scorpid sting", "scorpid sting"); };
                creators["sting viper"] = [](PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting viper", "viper sting", "viper sting"); };
            }
        };

        class StingSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StingSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["sting marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterStingPveStrategy(ai); };
                creators["sting marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterStingPvpStrategy(ai); };
                creators["sting marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterStingRaidStrategy(ai); };
                creators["sting survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterStingPveStrategy(ai); };
                creators["sting survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterStingPvpStrategy(ai); };
                creators["sting survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterStingRaidStrategy(ai); };
                creators["sting beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterStingPveStrategy(ai); };
                creators["sting beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterStingPvpStrategy(ai); };
                creators["sting beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterStingRaidStrategy(ai); };
            }
        };

        class AspectManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AspectManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aspect"] = [](PlayerbotAI* ai) { return new HunterAspectPlaceholderStrategy(ai); };
                creators["aspect hawk"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect hawk", "aspect of the hawk", "aspect of the hawk"); };
                creators["aspect monkey"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect monkey", "aspect of the monkey", "aspect of the monkey"); };
                creators["aspect cheetah"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect cheetah", "aspect of the cheetah", "aspect of the cheetah"); };
                creators["aspect pack"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect pack", "aspect of the pack", "aspect of the pack"); };
                creators["aspect beast"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect beast", "aspect of the beast", "aspect of the beast"); };
                creators["aspect wild"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect wild", "aspect of the wild", "aspect of the wild"); };
                creators["aspect viper"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect viper", "aspect of the viper", "aspect of the viper"); };
                creators["aspect dragonhawk"] = [](PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect dragonhawk", "aspect of the dragonhawk", "aspect of the dragonhawk"); };
            }
        };

        class AspectSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AspectSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aspect marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAspectPveStrategy(ai); };
                creators["aspect marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAspectPvpStrategy(ai); };
                creators["aspect marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterAspectRaidStrategy(ai); };
                creators["aspect survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterAspectPveStrategy(ai); };
                creators["aspect survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterAspectPvpStrategy(ai); };
                creators["aspect survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterAspectRaidStrategy(ai); };
                creators["aspect beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAspectPveStrategy(ai); };
                creators["aspect beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAspectPvpStrategy(ai); };
                creators["aspect beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterAspectRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["marksmanship"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterPlaceholderStrategy(ai); };
                creators["survival"] = [](PlayerbotAI* ai) { return new SurvivalHunterPlaceholderStrategy(ai); };
                creators["beast mastery"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["beast mastery pvp"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterPvpStrategy(ai); };
                creators["beast mastery pve"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterPveStrategy(ai); };
                creators["beast mastery raid"] = [](PlayerbotAI* ai) { return new BeastMasteryHunterRaidStrategy(ai); };
                creators["marksmanship pvp"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterPvpStrategy(ai); };
                creators["marksmanship pve"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterPveStrategy(ai); };
                creators["marksmanship raid"] = [](PlayerbotAI* ai) { return new MarksmanshipHunterRaidStrategy(ai); };
                creators["survival pvp"] = [](PlayerbotAI* ai) { return new SurvivalHunterPvpStrategy(ai); };
                creators["survival pve"] = [](PlayerbotAI* ai) { return new SurvivalHunterPveStrategy(ai); };
                creators["survival raid"] = [](PlayerbotAI* ai) { return new SurvivalHunterRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["black arrow"] = [](PlayerbotAI* ai) { return new BlackArrowTrigger(ai); };
                creators["black arrow on snare target"] = [](PlayerbotAI* ai) { return new BlackArrowSnareTrigger(ai); };
                creators["no stings"] = [](PlayerbotAI* ai) { return new HunterNoStingsActiveTrigger(ai); };
                creators["hunters pet dead"] = [](PlayerbotAI* ai) { return new HuntersPetDeadTrigger(ai); };
                creators["hunters pet low health"] = [](PlayerbotAI* ai) { return new HuntersPetLowHealthTrigger(ai); };
                creators["hunter's mark"] = [](PlayerbotAI* ai) { return new HuntersMarkTrigger(ai); };
                creators["freezing trap"] = [](PlayerbotAI* ai) { return new FreezingTrapTrigger(ai); };
                creators["frost trap"] = [](PlayerbotAI* ai) { return new FrostTrapTrigger(ai); };
                creators["explosive trap"] = [](PlayerbotAI* ai) { return new ExplosiveTrapTrigger(ai); };
                creators["rapid fire"] = [](PlayerbotAI* ai) { return new RapidFireTrigger(ai); };
                creators["kill command"] = [](PlayerbotAI* ai) { return new KillCommandBoostTrigger(ai); };
                creators["aspect of the hawk"] = [](PlayerbotAI* ai) { return new AspectOfTheHawkTrigger(ai); };
                creators["aspect of the wild"] = [](PlayerbotAI* ai) { return new AspectOfTheWildTrigger(ai); };
                creators["aspect of the viper"] = [](PlayerbotAI* ai) { return new AspectOfTheViperTrigger(ai); };
                creators["aspect of the monkey"] = [](PlayerbotAI* ai) { return new AspectOfTheMonkeyTrigger(ai); };
                creators["aspect of the beast"] = [](PlayerbotAI* ai) { return new AspectOfTheBeastTrigger(ai); };
                creators["aspect of the cheetah"] = [](PlayerbotAI* ai) { return new AspectOfTheCheetahTrigger(ai); };
                creators["aspect of the dragonhawk"] = [](PlayerbotAI* ai) { return new AspectOfTheDragonhawkTrigger(ai); };
                creators["aspect of the pack"] = [](PlayerbotAI* ai) { return new AspectOfThePackTrigger(ai); };
                creators["trueshot aura"] = [](PlayerbotAI* ai) { return new TrueshotAuraTrigger(ai); };
                creators["serpent sting on attacker"] = [](PlayerbotAI* ai) { return new SerpentStingOnAttackerTrigger(ai); };
                creators["viper sting on attacker"] = [](PlayerbotAI* ai) { return new ViperStingOnAttackerTrigger(ai); };
                creators["pet not happy"] = [](PlayerbotAI* ai) { return new HunterPetNotHappy(ai); };
                creators["concussive shot on snare target"] = [](PlayerbotAI* ai) { return new ConsussiveShotSnareTrigger(ai); };
                creators["scare beast"] = [](PlayerbotAI* ai) { return new ScareBeastTrigger(ai); };
                creators["low ammo"] = [](PlayerbotAI* ai) { return new HunterLowAmmoTrigger(ai); };
                creators["no ammo"] = [](PlayerbotAI* ai) { return new HunterNoAmmoTrigger(ai); };
                creators["has ammo"] = [](PlayerbotAI* ai) { return new HunterHasAmmoTrigger(ai); };
                creators["switch to melee"] = [](PlayerbotAI* ai) { return new SwitchToMeleeTrigger(ai); };
                creators["switch to ranged"] = [](PlayerbotAI* ai) { return new SwitchToRangedTrigger(ai); };
                creators["feign death"] = [](PlayerbotAI* ai) { return new FeignDeathTrigger(ai); };
                creators["scatter shot on snare target"] = [](PlayerbotAI* ai) { return new ScatterShotSnareTrigger(ai); };
                creators["chimera shot"] = [](PlayerbotAI* ai) { return new ChimeraShotCanCastTrigger(ai); };
                creators["explosive shot"] = [](PlayerbotAI* ai) { return new ExplosiveShotCanCastTrigger(ai); };
                creators["multi-shot"] = [](PlayerbotAI* ai) { return new MultishotCanCastTrigger(ai); };
                creators["steady shot"] = [](PlayerbotAI* ai) { return new SteadyShotCanCastTrigger(ai); };
                creators["intimidation on snare target"] = [](PlayerbotAI* ai) { return new IntimidationSnareTrigger(ai); };
                creators["counterattack"] = [](PlayerbotAI* ai) { return new CounterattackCanCastTrigger(ai); };
                creators["wyvern sting"] = [](PlayerbotAI* ai) { return new WybernStingSnareTrigger(ai); };
                creators["mongoose bite"] = [](PlayerbotAI* ai) { return new MongooseBiteCastTrigger(ai); };
                creators["viper sting"] = [](PlayerbotAI* ai) { return new ViperStingTrigger(ai); };
                creators["aimed shot"] = [](PlayerbotAI* ai) { return new AimedShotTrigger(ai); };
                creators["bestial wrath"] = [](PlayerbotAI* ai) { return new BestialWrathBoostTrigger(ai); };
                creators["silencing shot interrupt"] = [](PlayerbotAI* ai) { return new SilencingShotInterruptTrigger(ai); };
                creators["silencing shot on enemy healer"] = [](PlayerbotAI* ai) { return new SilencingShotInterruptHealerTrigger(ai); };
                creators["no beast"] = [](PlayerbotAI* ai) { return new HunterNoPet(ai); };
                creators["stealthed nearby"] = [](PlayerbotAI* ai) { return new StealthedNearbyTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["auto shot"] = [](PlayerbotAI* ai) { return new CastAutoShotAction(ai); };
                creators["aimed shot"] = [](PlayerbotAI* ai) { return new CastAimedShotAction(ai); };
                creators["chimera shot"] = [](PlayerbotAI* ai) { return new CastChimeraShotAction(ai); };
                creators["explosive shot"] = [](PlayerbotAI* ai) { return new CastExplosiveShotAction(ai); };
                creators["arcane shot"] = [](PlayerbotAI* ai) { return new CastArcaneShotAction(ai); };
                creators["tranquilizing shot"] = [](PlayerbotAI* ai) { return new CastTranquilizingShotAction(ai); };
                creators["concussive shot"] = [](PlayerbotAI* ai) { return new CastConcussiveShotAction(ai); };
                creators["distracting shot"] = [](PlayerbotAI* ai) { return new CastDistractingShotAction(ai); };
                creators["multi-shot"] = [](PlayerbotAI* ai) { return new CastMultiShotAction(ai); };
                creators["volley"] = [](PlayerbotAI* ai) { return new CastVolleyAction(ai); };
                creators["serpent sting"] = [](PlayerbotAI* ai) { return new CastSerpentStingAction(ai); };
                creators["serpent sting on attacker"] = [](PlayerbotAI* ai) { return new CastSerpentStingOnAttackerAction(ai); };
                creators["viper sting on attacker"] = [](PlayerbotAI* ai) { return new CastViperStingOnAttackerAction(ai); };
                creators["wyvern sting"] = [](PlayerbotAI* ai) { return new WyvernStingSnareAction(ai); };
                creators["viper sting"] = [](PlayerbotAI* ai) { return new CastViperStingAction(ai); };
                creators["scorpid sting"] = [](PlayerbotAI* ai) { return new CastScorpidStingAction(ai); };
                creators["hunter's mark"] = [](PlayerbotAI* ai) { return new CastHuntersMarkAction(ai); };
                creators["mend pet"] = [](PlayerbotAI* ai) { return new CastMendPetAction(ai); };
                creators["revive pet"] = [](PlayerbotAI* ai) { return new CastRevivePetAction(ai); };
                creators["call pet"] = [](PlayerbotAI* ai) { return new CastCallPetAction(ai); };
                creators["black arrow"] = [](PlayerbotAI* ai) { return new CastBlackArrow(ai); };
                creators["rapid fire"] = [](PlayerbotAI* ai) { return new CastRapidFireAction(ai); };
                creators["kill command"] = [](PlayerbotAI* ai) { return new CastKillCommandAction(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new CastRapidFireAction(ai); };
                creators["readiness"] = [](PlayerbotAI* ai) { return new CastReadinessAction(ai); };
                creators["aspect of the monkey"] = [](PlayerbotAI* ai) { return new CastAspectOfTheMonkeyAction(ai); };
                creators["aspect of the hawk"] = [](PlayerbotAI* ai) { return new CastAspectOfTheHawkAction(ai); };
                creators["aspect of the wild"] = [](PlayerbotAI* ai) { return new CastAspectOfTheWildAction(ai); };
                creators["aspect of the viper"] = [](PlayerbotAI* ai) { return new CastAspectOfTheViperAction(ai); };
                creators["aspect of the pack"] = [](PlayerbotAI* ai) { return new CastAspectOfThePackAction(ai); };
                creators["aspect of the cheetah"] = [](PlayerbotAI* ai) { return new CastAspectOfTheCheetahAction(ai); };
                creators["aspect of the beast"] = [](PlayerbotAI* ai) { return new CastAspectOfTheBeastAction(ai); };
                creators["aspect of the dragonhawk"] = [](PlayerbotAI* ai) { return new CastAspectOfTheDragonhawkAction(ai); };
                creators["remove aspect of the cheetah"] = [](PlayerbotAI* ai) { return new RemoveBuffAction(ai, "aspect of the cheetah"); };
                creators["trueshot aura"] = [](PlayerbotAI* ai) { return new CastTrueshotAuraAction(ai); };
                creators["feign death"] = [](PlayerbotAI* ai) { return new CastFeignDeathAction(ai); };
                creators["wing clip"] = [](PlayerbotAI* ai) { return new CastWingClipAction(ai); };
                creators["raptor strike"] = [](PlayerbotAI* ai) { return new CastRaptorStrikeAction(ai); };
                creators["feed pet"] = [](PlayerbotAI* ai) { return new FeedPetAction(ai); };
                creators["bestial wrath"] = [](PlayerbotAI* ai) { return new CastBestialWrathAction(ai); };
                creators["scare beast"] = [](PlayerbotAI* ai) { return new CastScareBeastAction(ai); };
                creators["scare beast on cc"] = [](PlayerbotAI* ai) { return new CastScareBeastCcAction(ai); };
                creators["remove feign death"] = [](PlayerbotAI* ai) { return new RemoveFeignDeathAction(ai); };
                creators["scatter shot"] = [](PlayerbotAI* ai) { return new CastScatterShotAction(ai); };
                creators["scatter shot on closest attacker targeting me"] = [](PlayerbotAI* ai) { return new CastScatterShotAction(ai); };
                creators["intimidation"] = [](PlayerbotAI* ai) { return new IntimidationAction(ai); };
                creators["deterrence"] = [](PlayerbotAI* ai) { return new DeterrenceAction(ai); };
                creators["counterattack"] = [](PlayerbotAI* ai) { return new CastCounterattackAction(ai); };
                creators["wyvern sting"] = [](PlayerbotAI* ai) { return new WyvernStingSnareAction(ai); };
                creators["mongoose bite"] = [](PlayerbotAI* ai) { return new MongooseBiteAction(ai); };
                creators["black arrow on snare target"] = [](PlayerbotAI* ai) { return new CastBlackArrowSnareAction(ai); };
                creators["silencing shot"] = [](PlayerbotAI* ai) { return new CastSilencingShotAction(ai); };
                creators["silencing shot on enemy healer"] = [](PlayerbotAI* ai) { return new CastSilencingShotOnHealerAction(ai); };
                creators["readiness"] = [](PlayerbotAI* ai) { return new CastReadinessAction(ai); };
                creators["steady shot"] = [](PlayerbotAI* ai) { return new CastSteadyShotAction(ai); };
                creators["tame beast"] = [](PlayerbotAI* ai) { return new TameBeastAction(ai); };
                creators["flare"] = [](PlayerbotAI* ai) { return new CastFlareAction(ai); };
                creators["immolation trap"] = [](PlayerbotAI* ai) { return new CastImmolationTrapAction(ai); };
                creators["frost trap"] = [](PlayerbotAI* ai) { return new CastFrostTrapAction(ai); };
                creators["explosive trap"] = [](PlayerbotAI* ai) { return new CastExplosiveTrapAction(ai); };
                creators["freezing trap"] = [](PlayerbotAI* ai) { return new CastFreezingTrapAction(ai); };
                creators["immolation trap on target"] = [](PlayerbotAI* ai) { return new CastImmolationTrapOnTargetAction(ai); };
                creators["frost trap on target"] = [](PlayerbotAI* ai) { return new CastFrostTrapOnTargetAction(ai); };
                creators["explosive trap on target"] = [](PlayerbotAI* ai) { return new CastExplosiveTrapOnTargetAction(ai); };
                creators["freezing trap on target"] = [](PlayerbotAI* ai) { return new CastFreezingTrapOnTargetAction(ai); };
                creators["freezing trap on cc"] = [](PlayerbotAI* ai) { return new CastFreezingTrapOnCcAction(ai); };
                creators["immolation trap in place"] = [](PlayerbotAI* ai) { return new CastImmolationTrapInPlaceAction(ai); };
                creators["frost trap in place"] = [](PlayerbotAI* ai) { return new CastFrostTrapInPlaceAction(ai); };
                creators["explosive trap in place"] = [](PlayerbotAI* ai) { return new CastExplosiveTrapInPlaceAction(ai); };
                creators["freezing trap in place"] = [](PlayerbotAI* ai) { return new CastFreezingTrapInPlaceAction(ai); };
                creators["dismiss pet"] = [](PlayerbotAI* ai) { return new CastDismissPetAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateHunterPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateHunterPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateHunterRaidStrategiesAction(ai); };
            }
        };
    };
};

HunterAiObjectContext::HunterAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::hunter::StrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::StingManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::StingSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AspectManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AspectSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::hunter::AiObjectContextInternal());
    triggerContexts.Add(new ai::hunter::TriggerFactoryInternal());
}
