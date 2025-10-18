
#include "playerbot/playerbot.h"
#include "WarlockActions.h"
#include "WarlockAiObjectContext.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "WarlockTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/actions/UseItemAction.h"
#include "AfflictionWarlockStrategy.h"
#include "DestructionWarlockStrategy.h"
#include "DemonologyWarlockStrategy.h"

namespace ai
{
    namespace warlock
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockAoePveStrategy(ai); };
                creators["aoe destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockAoePvpStrategy(ai); };
                creators["aoe destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockAoeRaidStrategy(ai); };
                creators["aoe demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockAoePveStrategy(ai); };
                creators["aoe demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockAoePvpStrategy(ai); };
                creators["aoe demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockAoeRaidStrategy(ai); };
                creators["aoe affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockAoePveStrategy(ai); };
                creators["aoe affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockAoePvpStrategy(ai); };
                creators["aoe affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockAoeRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockBuffPveStrategy(ai); };
                creators["buff destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockBuffPvpStrategy(ai); };
                creators["buff destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockBuffRaidStrategy(ai); };
                creators["buff demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBuffPveStrategy(ai); };
                creators["buff demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBuffPvpStrategy(ai); };
                creators["buff demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBuffRaidStrategy(ai); };
                creators["buff affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBuffPveStrategy(ai); };
                creators["buff affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBuffPvpStrategy(ai); };
                creators["buff affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockBoostPveStrategy(ai); };
                creators["boost destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockBoostPvpStrategy(ai); };
                creators["boost destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockBoostRaidStrategy(ai); };
                creators["boost demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBoostPveStrategy(ai); };
                creators["boost demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBoostPvpStrategy(ai); };
                creators["boost demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockBoostRaidStrategy(ai); };
                creators["boost affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBoostPveStrategy(ai); };
                creators["boost affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBoostPvpStrategy(ai); };
                creators["boost affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockBoostRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["demonology"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPlaceholderStrategy(ai); };
                creators["affliction"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPlaceholderStrategy(ai); };
                creators["destruction"] = [](PlayerbotAI* ai) { return new DestructionWarlockPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockPvpStrategy(ai); };
                creators["destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockPveStrategy(ai); };
                creators["destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockRaidStrategy(ai); };
                creators["demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPvpStrategy(ai); };
                creators["demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPveStrategy(ai); };
                creators["demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockRaidStrategy(ai); };
                creators["affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPvpStrategy(ai); };
                creators["affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPveStrategy(ai); };
                creators["affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockRaidStrategy(ai); };
            }
        };

        class PetManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PetManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["pet"] = [](PlayerbotAI* ai) { return new WarlockPetPlaceholderStrategy(ai); };
                creators["pet imp"] = [](PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet imp", "no imp", "summon imp"); };
                creators["pet voidwalker"] = [](PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet voidwalker", "no voidwalker", "summon voidwalker"); };
                creators["pet succubus"] = [](PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet succubus", "no succubus", "summon succubus"); };
                creators["pet felhunter"] = [](PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet felhunter", "no felhunter", "summon felhunter"); };
                creators["pet felguard"] = [](PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet felguard", "no felguard", "summon felguard"); };
            }
        };

        class PetSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PetSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["pet destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockPetPveStrategy(ai); };
                creators["pet destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockPetPvpStrategy(ai); };
                creators["pet destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockPetRaidStrategy(ai); };
                creators["pet demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPetPveStrategy(ai); };
                creators["pet demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPetPvpStrategy(ai); };
                creators["pet demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockPetRaidStrategy(ai); };
                creators["pet affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPetPveStrategy(ai); };
                creators["pet affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPetPvpStrategy(ai); };
                creators["pet affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockPetRaidStrategy(ai); };
            }
        };

        class CurseManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CurseManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["curse"] = [](PlayerbotAI* ai) { return new WarlockCursePlaceholderStrategy(ai); };
                creators["curse agony"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse agony", "curse of agony", "curse of agony"); };
                creators["curse doom"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse doom", "curse of doom", "curse of doom"); };
                creators["curse elements"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse elements", "curse of the elements", "curse of the elements"); };
                creators["curse recklessness"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse recklessness", "curse of recklessness", "curse of recklessness"); };
                creators["curse weakness"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse weakness", "curse of weakness", "curse of weakness"); };
                creators["curse tongues"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse tongues", "curse of tongues", "curse of tongues"); };
                creators["curse shadow"] = [](PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse shadow", "curse of shadow", "curse of shadow"); };
            }
        };

        class CurseSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CurseSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["curse destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockCursesPveStrategy(ai); };
                creators["curse destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockCursesPvpStrategy(ai); };
                creators["curse destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockCursesRaidStrategy(ai); };
                creators["curse demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCursesPveStrategy(ai); };
                creators["curse demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCursesPvpStrategy(ai); };
                creators["curse demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCursesRaidStrategy(ai); };
                creators["curse affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCursesPveStrategy(ai); };
                creators["curse affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCursesPvpStrategy(ai); };
                creators["curse affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCursesRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc destruction pve"] = [](PlayerbotAI* ai) { return new DestructionWarlockCcPveStrategy(ai); };
                creators["cc destruction pvp"] = [](PlayerbotAI* ai) { return new DestructionWarlockCcPvpStrategy(ai); };
                creators["cc destruction raid"] = [](PlayerbotAI* ai) { return new DestructionWarlockCcRaidStrategy(ai); };
                creators["cc demonology pve"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCcPveStrategy(ai); };
                creators["cc demonology pvp"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCcPvpStrategy(ai); };
                creators["cc demonology raid"] = [](PlayerbotAI* ai) { return new DemonologyWarlockCcRaidStrategy(ai); };
                creators["cc affliction pve"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCcPveStrategy(ai); };
                creators["cc affliction pvp"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCcPvpStrategy(ai); };
                creators["cc affliction raid"] = [](PlayerbotAI* ai) { return new AfflictionWarlockCcRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["shadow trance"] = [](PlayerbotAI* ai) { return new ShadowTranceTrigger(ai); };
                creators["demon armor"] = [](PlayerbotAI* ai) { return new DemonArmorTrigger(ai); };
                creators["no healthstone"] = [](PlayerbotAI* ai) { return new HasHealthstoneTrigger(ai); };
                creators["no firestone"] = [](PlayerbotAI* ai) { return new HasFirestoneTrigger(ai); };
                creators["no spellstone"] = [](PlayerbotAI* ai) { return new HasSpellstoneTrigger(ai); };
                creators["corruption"] = [](PlayerbotAI* ai) { return new CorruptionTrigger(ai); };
                creators["corruption on attacker"] = [](PlayerbotAI* ai) { return new CorruptionOnAttackerTrigger(ai); };
                creators["curse of agony"] = [](PlayerbotAI* ai) { return new CurseOfAgonyTrigger(ai); };
                creators["curse of agony on attacker"] = [](PlayerbotAI* ai) { return new CurseOfAgonyOnAttackerTrigger(ai); };
                creators["curse of doom"] = [](PlayerbotAI* ai) { return new CurseOfDoomTrigger(ai); };
                creators["curse of doom on attacker"] = [](PlayerbotAI* ai) { return new CurseOfDoomOnAttackerTrigger(ai); };
                creators["curse of the elements"] = [](PlayerbotAI* ai) { return new CurseOfTheElementsTrigger(ai); };
                creators["curse of the elements on attacker"] = [](PlayerbotAI* ai) { return new CurseOfTheElementsOnAttackerTrigger(ai); };
                creators["curse of recklessness"] = [](PlayerbotAI* ai) { return new CurseOfRecklessnessTrigger(ai); };
                creators["curse of recklessness on attacker"] = [](PlayerbotAI* ai) { return new CurseOfRecklessnessOnAttackerTrigger(ai); };
                creators["curse of weakness"] = [](PlayerbotAI* ai) { return new CurseOfWeaknessTrigger(ai); };
                creators["curse of weakness on attacker"] = [](PlayerbotAI* ai) { return new CurseOfWeaknessOnAttackerTrigger(ai); };
                creators["curse of tongues"] = [](PlayerbotAI* ai) { return new CurseOfTonguesTrigger(ai); };
                creators["curse of tongues on attacker"] = [](PlayerbotAI* ai) { return new CurseOfTonguesOnAttackerTrigger(ai); };
                creators["curse of shadow"] = [](PlayerbotAI* ai) { return new CurseOfShadowTrigger(ai); };
                creators["curse of shadow on attacker"] = [](PlayerbotAI* ai) { return new CurseOfShadowOnAttackerTrigger(ai); };
                creators["banish"] = [](PlayerbotAI* ai) { return new BanishTrigger(ai); };
                creators["spellstone"] = [](PlayerbotAI* ai) { return new SpellstoneTrigger(ai); };
                creators["backlash"] = [](PlayerbotAI* ai) { return new BacklashTrigger(ai); };
                creators["fear"] = [](PlayerbotAI* ai) { return new FearTrigger(ai); };
                creators["fear pvp"] = [](PlayerbotAI* ai) { return new FearPvpTrigger(ai); };
                creators["immolate"] = [](PlayerbotAI* ai) { return new ImmolateTrigger(ai); };
                creators["amplify curse"] = [](PlayerbotAI* ai) { return new AmplifyCurseTrigger(ai); };
                creators["siphon life"] = [](PlayerbotAI* ai) { return new SiphonLifeTrigger(ai); };
                creators["siphon life on attacker"] = [](PlayerbotAI* ai) { return new SiphonLifeOnAttackerTrigger(ai); };
                creators["death coil interrupt"] = [](PlayerbotAI* ai) { return new DeathCoilInterruptTrigger(ai); };
                creators["death coil on enemy healer"] = [](PlayerbotAI* ai) { return new DeathCoilInterruptTHealerTrigger(ai); };
                creators["death coil on snare target"] = [](PlayerbotAI* ai) { return new DeathCoilSnareTrigger(ai); };
                creators["inferno"] = [](PlayerbotAI* ai) { return new InfernoTrigger(ai); };
                creators["shadowfury interrupt"] = [](PlayerbotAI* ai) { return new ShadowfuryInterruptTrigger(ai); };
                creators["shadowfury on snare target"] = [](PlayerbotAI* ai) { return new ShadowfurySnareTrigger(ai); };
                creators["unstable affliction"] = [](PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); };
                creators["unstable affliction on attacker"] = [](PlayerbotAI* ai) { return new UnstableAfflictionOnAttackerTrigger(ai); };
                creators["life tap"] = [](PlayerbotAI* ai) { return new LifeTapTrigger(ai); };
                creators["drain soul"] = [](PlayerbotAI* ai) { return new DrainSoulTrigger(ai); };
                creators["no curse"] = [](PlayerbotAI* ai) { return new NoCurseTrigger(ai); };
                creators["no curse on attacker"] = [](PlayerbotAI* ai) { return new NoCurseOnAttackerTrigger(ai); };
                creators["conflagrate"] = [](PlayerbotAI* ai) { return new ConflagrateTrigger(ai); };
                creators["demonic sacrifice"] = [](PlayerbotAI* ai) { return new DemonicSacrificeTrigger(ai); };
                creators["no imp"] = [](PlayerbotAI* ai) { return new NoImpTrigger(ai); };
                creators["no voidwalker"] = [](PlayerbotAI* ai) { return new NoVoidwalkerTrigger(ai); };
                creators["no succubus"] = [](PlayerbotAI* ai) { return new NoSuccubusTrigger(ai); };
                creators["no felhunter"] = [](PlayerbotAI* ai) { return new NoFelhunterTrigger(ai); };
                creators["no felguard"] = [](PlayerbotAI* ai) { return new NoFelguardTrigger(ai); };
                creators["spell lock"] = [](PlayerbotAI* ai) { return new SpellLockTrigger(ai); };
                creators["spell lock enemy healer"] = [](PlayerbotAI* ai) { return new SpellLockEnemyHealerTrigger(ai); };
                creators["seed of corruption on attacker"] = [](PlayerbotAI* ai) { return new SeedOfCorruptionOnAttackerTrigger(ai); };
                creators["soulstone"] = [](PlayerbotAI* ai) { return new SoulstoneTrigger(ai); };
                creators["shadow ward"] = [](PlayerbotAI* ai) { return new ShadowWardTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["fel armor"] = [](PlayerbotAI* ai) { return new CastFelArmorAction(ai); };
                creators["demon armor"] = [](PlayerbotAI* ai) { return new CastDemonArmorAction(ai); };
                creators["demon skin"] = [](PlayerbotAI* ai) { return new CastDemonSkinAction(ai); };
                creators["create healthstone"] = [](PlayerbotAI* ai) { return new CastCreateHealthstoneAction(ai); };
                creators["create firestone"] = [](PlayerbotAI* ai) { return new CastCreateFirestoneAction(ai); };
                creators["create spellstone"] = [](PlayerbotAI* ai) { return new CastCreateSpellstoneAction(ai); };
                creators["spellstone"] = [](PlayerbotAI* ai) { return new UseSpellItemAction(ai, "spellstone"); };
                creators["summon voidwalker"] = [](PlayerbotAI* ai) { return new CastSummonVoidwalkerAction(ai); };
                creators["summon succubus"] = [](PlayerbotAI* ai) { return new CastSummonSuccubusAction(ai); };
                creators["summon felhunter"] = [](PlayerbotAI* ai) { return new CastSummonFelhunterAction(ai); };
                creators["summon imp"] = [](PlayerbotAI* ai) { return new CastSummonImpAction(ai); };
                creators["summon felguard"] = [](PlayerbotAI* ai) { return new CastSummonFelguardAction(ai); };
                creators["summon inferno"] = [](PlayerbotAI* ai) { return new CastSummonInfernoAction(ai); };
                creators["immolate"] = [](PlayerbotAI* ai) { return new CastImmolateAction(ai); };
                creators["corruption"] = [](PlayerbotAI* ai) { return new CastCorruptionAction(ai); };
                creators["corruption on attacker"] = [](PlayerbotAI* ai) { return new CastCorruptionOnAttackerAction(ai); };
                creators["seed of corruption on attacker"] = [](PlayerbotAI* ai) { return new CastSeedOfCorruptionOnAttackerAction(ai); };
                creators["siphon life"] = [](PlayerbotAI* ai) { return new CastSiphonLifeAction(ai); };
                creators["siphon life on attacker"] = [](PlayerbotAI* ai) { return new CastSiphonLifeOnAttackerAction(ai); };
                creators["curse of agony"] = [](PlayerbotAI* ai) { return new CastCurseOfAgonyAction(ai); };
                creators["curse of agony on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfAgonyOnAttackerAction(ai); };
                creators["curse of doom"] = [](PlayerbotAI* ai) { return new CastCurseOfDoomAction(ai); };
                creators["curse of doom on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfDoomOnAttackerAction(ai); };
                creators["curse of the elements"] = [](PlayerbotAI* ai) { return new CastCurseOfTheElementsAction(ai); };
                creators["curse of the elements on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfTheElementsOnAttackerAction(ai); };
                creators["curse of recklessness"] = [](PlayerbotAI* ai) { return new CastCurseOfRecklessnessAction(ai); };
                creators["curse of recklessness on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfRecklessnessOnAttackerAction(ai); };
                creators["curse of weakness"] = [](PlayerbotAI* ai) { return new CastCurseOfWeaknessAction(ai); };
                creators["curse of weakness on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfWeaknessOnAttackerAction(ai); };
                creators["curse of tongues"] = [](PlayerbotAI* ai) { return new CastCurseOfTonguesAction(ai); };
                creators["curse of tongues on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfTonguesOnAttackerAction(ai); };
                creators["curse of shadow"] = [](PlayerbotAI* ai) { return new CastCurseOfShadowAction(ai); };
                creators["curse of shadow on attacker"] = [](PlayerbotAI* ai) { return new CastCurseOfShadowOnAttackerAction(ai); };
                creators["shadow bolt"] = [](PlayerbotAI* ai) { return new CastShadowBoltAction(ai); };
                creators["drain soul"] = [](PlayerbotAI* ai) { return new CastDrainSoulAction(ai); };
                creators["drain mana"] = [](PlayerbotAI* ai) { return new CastDrainManaAction(ai); };
                creators["drain life"] = [](PlayerbotAI* ai) { return new CastDrainLifeAction(ai); };
                creators["banish"] = [](PlayerbotAI* ai) { return new CastBanishAction(ai); };
                creators["banish on cc"] = [](PlayerbotAI* ai) { return new CastBanishOnCcAction(ai); };
                creators["seed of corruption"] = [](PlayerbotAI* ai) { return new CastSeedOfCorruptionAction(ai); };
                creators["rain of fire"] = [](PlayerbotAI* ai) { return new CastRainOfFireAction(ai); };
                creators["shadowfury"] = [](PlayerbotAI* ai) { return new CastShadowfuryAction(ai); };
                creators["shadowfury on snare target"] = [](PlayerbotAI* ai) { return new CastShadowfurySnareAction(ai); };
                creators["life tap"] = [](PlayerbotAI* ai) { return new CastLifeTapAction(ai); };
                creators["fear"] = [](PlayerbotAI* ai) { return new CastFearAction(ai); };
                creators["fear on cc"] = [](PlayerbotAI* ai) { return new CastFearOnCcAction(ai); };
                creators["incinerate"] = [](PlayerbotAI* ai) { return new CastIncinerateAction(ai); };
                creators["conflagrate"] = [](PlayerbotAI* ai) { return new CastConflagrateAction(ai); };
                creators["amplify curse"] = [](PlayerbotAI* ai) { return new CastAmplifyCurseAction(ai); };
                creators["shadowburn"] = [](PlayerbotAI* ai) { return new CastShadowburnAction(ai); };
                creators["death coil"] = [](PlayerbotAI* ai) { return new CastDeathCoilAction(ai); };
                creators["death coil on enemy healer"] = [](PlayerbotAI* ai) { return new CastDeathCoilOnHealerAction(ai); };
                creators["death coil on snare target"] = [](PlayerbotAI* ai) { return new CastDeathCoilSnareAction(ai); };
                creators["dark pact"] = [](PlayerbotAI* ai) { return new CastDarkPactAction(ai); };
                creators["howl of terror"] = [](PlayerbotAI* ai) { return new CastHowlOfTerrorAction(ai); };
                creators["unstable affliction"] = [](PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); };
                creators["unstable affliction on attacker"] = [](PlayerbotAI* ai) { return new CastUnstableAfflictionOnAttackerAction(ai); };
                creators["soulshatter"] = [](PlayerbotAI* ai) { return new CastSoulShatterAction(ai); };
                creators["searing pain"] = [](PlayerbotAI* ai) { return new CastSearingPainAction(ai); };
                creators["soul fire"] = [](PlayerbotAI* ai) { return new CastSoulFireAction(ai); };
                creators["curse of exhaustion"] = [](PlayerbotAI* ai) { return new CastCurseOfExhaustionAction(ai); };
                creators["demonic sacrifice"] = [](PlayerbotAI* ai) { return new CastDemonicSacrificeAction(ai); };
                creators["soul link"] = [](PlayerbotAI* ai) { return new CastSoulLinkAction(ai); };
                creators["sacrifice"] = [](PlayerbotAI* ai) { return new CastSacrificeAction(ai); };
                creators["spell lock"] = [](PlayerbotAI* ai) { return new CastSpellLockAction(ai); };
                creators["spell lock on enemy healer"] = [](PlayerbotAI* ai) { return new CastSpellLockOnEnemyHealerAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateWarlockPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateWarlockPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateWarlockRaidStrategiesAction(ai); };
                creators["soulstone"] = [](PlayerbotAI* ai) { return new CastSoulstoneAction(ai); };
                creators["shadow ward"] = [](PlayerbotAI* ai) { return new CastShadowWardAction(ai); };
            }
        };
    };
};

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warlock::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::PetManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::PetSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CurseSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CurseManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::warlock::AiObjectContextInternal());
    triggerContexts.Add(new ai::warlock::TriggerFactoryInternal());
}
