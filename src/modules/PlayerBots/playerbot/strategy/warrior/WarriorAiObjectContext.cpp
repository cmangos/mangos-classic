
#include "playerbot/playerbot.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "WarriorActions.h"
#include "WarriorTriggers.h"
#include "WarriorAiObjectContext.h"
#include "ProtectionWarriorStrategy.h"
#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"

namespace ai
{
    namespace warrior
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
                creators["aoe arms pve"] = [](PlayerbotAI* ai) { return new ArmsWarriorAoePveStrategy(ai); };
                creators["aoe arms pvp"] = [](PlayerbotAI* ai) { return new ArmsWarriorAoePvpStrategy(ai); };
                creators["aoe arms raid"] = [](PlayerbotAI* ai) { return new ArmsWarriorAoeRaidStrategy(ai); };
                creators["aoe fury pve"] = [](PlayerbotAI* ai) { return new FuryWarriorAoePveStrategy(ai); };
                creators["aoe fury pvp"] = [](PlayerbotAI* ai) { return new FuryWarriorAoePvpStrategy(ai); };
                creators["aoe fury raid"] = [](PlayerbotAI* ai) { return new FuryWarriorAoeRaidStrategy(ai); };
                creators["aoe protection pve"] = [](PlayerbotAI* ai) { return new ProtectionWarriorAoePveStrategy(ai); };
                creators["aoe protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionWarriorAoePvpStrategy(ai); };
                creators["aoe protection raid"] = [](PlayerbotAI* ai) { return new ProtectionWarriorAoeRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff arms pve"] = [](PlayerbotAI* ai) { return new ArmsWarriorBuffPveStrategy(ai); };
                creators["buff arms pvp"] = [](PlayerbotAI* ai) { return new ArmsWarriorBuffPvpStrategy(ai); };
                creators["buff arms raid"] = [](PlayerbotAI* ai) { return new ArmsWarriorBuffRaidStrategy(ai); };
                creators["buff fury pve"] = [](PlayerbotAI* ai) { return new FuryWarriorBuffPveStrategy(ai); };
                creators["buff fury pvp"] = [](PlayerbotAI* ai) { return new FuryWarriorBuffPvpStrategy(ai); };
                creators["buff fury raid"] = [](PlayerbotAI* ai) { return new FuryWarriorBuffRaidStrategy(ai); };
                creators["buff protection pve"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBuffPveStrategy(ai); };
                creators["buff protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBuffPvpStrategy(ai); };
                creators["buff protection raid"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost arms pve"] = [](PlayerbotAI* ai) { return new ArmsWarriorBoostPveStrategy(ai); };
                creators["boost arms pvp"] = [](PlayerbotAI* ai) { return new ArmsWarriorBoostPvpStrategy(ai); };
                creators["boost arms raid"] = [](PlayerbotAI* ai) { return new ArmsWarriorBoostRaidStrategy(ai); };
                creators["boost fury pve"] = [](PlayerbotAI* ai) { return new FuryWarriorBoostPveStrategy(ai); };
                creators["boost fury pvp"] = [](PlayerbotAI* ai) { return new FuryWarriorBoostPvpStrategy(ai); };
                creators["boost fury raid"] = [](PlayerbotAI* ai) { return new FuryWarriorBoostRaidStrategy(ai); };
                creators["boost protection pve"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBoostPveStrategy(ai); };
                creators["boost protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBoostPvpStrategy(ai); };
                creators["boost protection raid"] = [](PlayerbotAI* ai) { return new ProtectionWarriorBoostRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc fury pve"] = [](PlayerbotAI* ai) { return new FuryWarriorCcPveStrategy(ai); };
                creators["cc fury pvp"] = [](PlayerbotAI* ai) { return new FuryWarriorCcPvpStrategy(ai); };
                creators["cc fury raid"] = [](PlayerbotAI* ai) { return new FuryWarriorCcRaidStrategy(ai); };
                creators["cc protection pve"] = [](PlayerbotAI* ai) { return new ProtectionWarriorCcPveStrategy(ai); };
                creators["cc protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionWarriorCcPvpStrategy(ai); };
                creators["cc protection raid"] = [](PlayerbotAI* ai) { return new ProtectionWarriorCcRaidStrategy(ai); };
                creators["cc arms pve"] = [](PlayerbotAI* ai) { return new ArmsWarriorCcPveStrategy(ai); };
                creators["cc arms pvp"] = [](PlayerbotAI* ai) { return new ArmsWarriorCcPvpStrategy(ai); };
                creators["cc arms raid"] = [](PlayerbotAI* ai) { return new ArmsWarriorCcRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["fury"] = [](PlayerbotAI* ai) { return new FuryWarriorPlaceholderStrategy(ai); };
                creators["protection"] = [](PlayerbotAI* ai) { return new ProtectionWarriorPlaceholderStrategy(ai); };
                creators["tank"] = [](PlayerbotAI* ai) { return new ProtectionWarriorPlaceholderStrategy(ai); };
                creators["arms"] = [](PlayerbotAI* ai) { return new ArmsWarriorPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["arms pvp"] = [](PlayerbotAI* ai) { return new ArmsWarriorPvpStrategy(ai); };
                creators["arms pve"] = [](PlayerbotAI* ai) { return new ArmsWarriorPveStrategy(ai); };
                creators["arms raid"] = [](PlayerbotAI* ai) { return new ArmsWarriorRaidStrategy(ai); };
                creators["fury pvp"] = [](PlayerbotAI* ai) { return new FuryWarriorPvpStrategy(ai); };
                creators["fury pve"] = [](PlayerbotAI* ai) { return new FuryWarriorPveStrategy(ai); };
                creators["fury raid"] = [](PlayerbotAI* ai) { return new FuryWarriorRaidStrategy(ai); };
                creators["protection pvp"] = [](PlayerbotAI* ai) { return new ProtectionWarriorPvpStrategy(ai); };
                creators["protection pve"] = [](PlayerbotAI* ai) { return new ProtectionWarriorPveStrategy(ai); };
                creators["protection raid"] = [](PlayerbotAI* ai) { return new ProtectionWarriorRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["hamstring"] = [](PlayerbotAI* ai) { return new HamstringTrigger(ai); };
                creators["victory rush"] = [](PlayerbotAI* ai) { return new VictoryRushTrigger(ai); };
                creators["death wish"] = [](PlayerbotAI* ai) { return new DeathWishTrigger(ai); };
                creators["battle shout"] = [](PlayerbotAI* ai) { return new BattleShoutTrigger(ai); };
                creators["rend"] = [](PlayerbotAI* ai) { return new RendDebuffTrigger(ai); };
                creators["rend on attacker"] = [](PlayerbotAI* ai) { return new RendDebuffOnAttackerTrigger(ai); };
                creators["bloodrage"] = [](PlayerbotAI* ai) { return new BloodrageBuffTrigger(ai); };
                creators["shield bash"] = [](PlayerbotAI* ai) { return new ShieldBashInterruptSpellTrigger(ai); };
                creators["disarm"] = [](PlayerbotAI* ai) { return new DisarmDebuffTrigger(ai); };
                creators["concussion blow"] = [](PlayerbotAI* ai) { return new ConcussionBlowTrigger(ai); };
                creators["sword and board"] = [](PlayerbotAI* ai) { return new SwordAndBoardTrigger(ai); };
                creators["shield bash on enemy healer"] = [](PlayerbotAI* ai) { return new ShieldBashInterruptEnemyHealerSpellTrigger(ai); };
                creators["battle stance"] = [](PlayerbotAI* ai) { return new BattleStanceTrigger(ai); };
                creators["defensive stance"] = [](PlayerbotAI* ai) { return new DefensiveStanceTrigger(ai); };
                creators["berserker stance"] = [](PlayerbotAI* ai) { return new BerserkerStanceTrigger(ai); };
                creators["shield block"] = [](PlayerbotAI* ai) { return new ShieldBlockTrigger(ai); };
                creators["sunder armor"] = [](PlayerbotAI* ai) { return new SunderArmorDebuffTrigger(ai); };
                creators["demoralizing shout"] = [](PlayerbotAI* ai) { return new DemoralizingShoutDebuffTrigger(ai); };
                creators["devastate"] = [](PlayerbotAI* ai) { return new DevastateAvailableTrigger(ai); };
                creators["revenge"] = [](PlayerbotAI* ai) { return new RevengeAvailableTrigger(ai); };
                creators["overpower"] = [](PlayerbotAI* ai) { return new OverpowerAvailableTrigger(ai); };
                creators["mocking blow"] = [](PlayerbotAI* ai) { return new MockingBlowTrigger(ai); };
                creators["rampage"] = [](PlayerbotAI* ai) { return new RampageAvailableTrigger(ai); };
                creators["mortal strike"] = [](PlayerbotAI* ai) { return new MortalStrikeDebuffTrigger(ai); };
                creators["thunder clap on snare target"] = [](PlayerbotAI* ai) { return new ThunderClapSnareTrigger(ai); };
                creators["thunder clap"] = [](PlayerbotAI* ai) { return new ThunderClapTrigger(ai); };
                creators["bloodthirst"] = [](PlayerbotAI* ai) { return new BloodthirstTrigger(ai); };
                creators["whirlwind"] = [](PlayerbotAI* ai) { return new WhirlwindTrigger(ai); };
                creators["heroic strike"] = [](PlayerbotAI* ai) { return new HeroicStrikeTrigger(ai); };
                creators["berserker rage"] = [](PlayerbotAI* ai) { return new BerserkerRageBuffTrigger(ai); };
                creators["pummel on enemy healer"] = [](PlayerbotAI* ai) { return new PummelInterruptEnemyHealerSpellTrigger(ai); };
                creators["pummel"] = [](PlayerbotAI* ai) { return new PummelInterruptSpellTrigger(ai); };
                creators["intercept on enemy healer"] = [](PlayerbotAI* ai) { return new InterceptInterruptEnemyHealerSpellTrigger(ai); };
                creators["intercept"] = [](PlayerbotAI* ai) { return new InterceptInterruptSpellTrigger(ai); };
                creators["taunt on snare target"] = [](PlayerbotAI* ai) { return new TauntSnareTrigger(ai); };
                creators["commanding shout"] = [](PlayerbotAI* ai) { return new CommandingShoutTrigger(ai); };
                creators["intercept on snare target"] = [](PlayerbotAI* ai) { return new InterceptSnareTrigger(ai); };
                creators["spell reflection"] = [](PlayerbotAI* ai) { return new SpellReflectionTrigger(ai); };
                creators["sudden death"] = [](PlayerbotAI* ai) { return new SuddenDeathTrigger(ai); };
                creators["instant slam"] = [](PlayerbotAI* ai) { return new SlamInstantTrigger(ai); };
                creators["shockwave"] = [](PlayerbotAI* ai) { return new ShockwaveTrigger(ai); };
                creators["shockwave on snare target"] = [](PlayerbotAI* ai) { return new ShockwaveSnareTrigger(ai); };
                creators["taste for blood"] = [](PlayerbotAI* ai) { return new TasteForBloodTrigger(ai); };
                creators["thunder clap and rage"] = [](PlayerbotAI* ai) { return new TwoTriggers(ai, "thunder clap", "medium rage available"); };
                creators["thunder clap and rage and aoe"] = [](PlayerbotAI* ai) { return new TwoTriggers(ai, "thunder clap and rage", "ranged light aoe"); };
                creators["intercept can cast"] = [](PlayerbotAI* ai) { return new InterceptCanCastTrigger(ai); };
                creators["intercept and far enemy"] = [](PlayerbotAI* ai) { return new TwoTriggers(ai, "enemy is out of melee", "intercept can cast"); };
                creators["intercept and rage"] = [](PlayerbotAI* ai) { return new TwoTriggers(ai, "intercept and far enemy", "light rage available"); };
                creators["recklessness"] = [](PlayerbotAI* ai) { return new RecklessnessTrigger(ai); };
                creators["slam"] = [](PlayerbotAI* ai) { return new SlamTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["devastate"] = [](PlayerbotAI* ai) { return new CastDevastateAction(ai); };
                creators["overpower"] = [](PlayerbotAI* ai) { return new CastOverpowerAction(ai); };
                creators["charge"] = [](PlayerbotAI* ai) { return new CastChargeAction(ai); };
                creators["bloodthirst"] = [](PlayerbotAI* ai) { return new CastBloodthirstAction(ai); };
                creators["rend"] = [](PlayerbotAI* ai) { return new CastRendAction(ai); };
                creators["rend on attacker"] = [](PlayerbotAI* ai) { return new CastRendOnAttackerAction(ai); };
                creators["mocking blow"] = [](PlayerbotAI* ai) { return new CastMockingBlowAction(ai); };
                creators["death wish"] = [](PlayerbotAI* ai) { return new CastDeathWishAction(ai); };
                creators["berserker rage"] = [](PlayerbotAI* ai) { return new CastBerserkerRageAction(ai); };
                creators["victory rush"] = [](PlayerbotAI* ai) { return new CastVictoryRushAction(ai); };
                creators["execute"] = [](PlayerbotAI* ai) { return new CastExecuteAction(ai); };
                creators["defensive stance"] = [](PlayerbotAI* ai) { return new CastDefensiveStanceAction(ai); };
                creators["hamstring"] = [](PlayerbotAI* ai) { return new CastHamstringAction(ai); };
                creators["shield bash"] = [](PlayerbotAI* ai) { return new CastShieldBashAction(ai); };
                creators["shield block"] = [](PlayerbotAI* ai) { return new CastShieldBlockAction(ai); };
                creators["bloodrage"] = [](PlayerbotAI* ai) { return new CastBloodrageAction(ai); };
                creators["battle stance"] = [](PlayerbotAI* ai) { return new CastBattleStanceAction(ai); };
                creators["heroic strike"] = [](PlayerbotAI* ai) { return new CastHeroicStrikeAction(ai); };
                creators["intimidating shout"] = [](PlayerbotAI* ai) { return new CastIntimidatingShoutAction(ai); };
                creators["demoralizing shout"] = [](PlayerbotAI* ai) { return new CastDemoralizingShoutAction(ai); };
                creators["challenging shout"] = [](PlayerbotAI* ai) { return new CastChallengingShoutAction(ai); };
                creators["shield wall"] = [](PlayerbotAI* ai) { return new CastShieldWallAction(ai); };
                creators["battle shout"] = [](PlayerbotAI* ai) { return new CastBattleShoutAction(ai); };
                creators["battle shout taunt"] = [](PlayerbotAI* ai) { return new CastBattleShoutTauntAction(ai); };
                creators["thunder clap"] = [](PlayerbotAI* ai) { return new CastThunderClapAction(ai); };
                creators["taunt"] = [](PlayerbotAI* ai) { return new CastTauntAction(ai); };
                creators["revenge"] = [](PlayerbotAI* ai) { return new CastRevengeAction(ai); };
                creators["slam"] = [](PlayerbotAI* ai) { return new CastSlamAction(ai); };
                creators["shield slam"] = [](PlayerbotAI* ai) { return new CastShieldSlamAction(ai); };
                creators["disarm"] = [](PlayerbotAI* ai) { return new CastDisarmAction(ai); };
                creators["sunder armor"] = [](PlayerbotAI* ai) { return new CastSunderArmorAction(ai); };
                creators["last stand"] = [](PlayerbotAI* ai) { return new CastLastStandAction(ai); };
                creators["shockwave"] = [](PlayerbotAI* ai) { return new CastShockwaveAction(ai); };
                creators["shockwave on snare target"] = [](PlayerbotAI* ai) { return new CastShockwaveSnareAction(ai); };
                creators["cleave"] = [](PlayerbotAI* ai) { return new CastCleaveAction(ai); };
                creators["concussion blow"] = [](PlayerbotAI* ai) { return new CastConcussionBlowAction(ai); };
                creators["shield bash on enemy healer"] = [](PlayerbotAI* ai) { return new CastShieldBashOnEnemyHealerAction(ai); };
                creators["berserker stance"] = [](PlayerbotAI* ai) { return new CastBerserkerStanceAction(ai); };
                creators["commanding shout"] = [](PlayerbotAI* ai) { return new CastCommandingShoutAction(ai); };
                creators["retaliation"] = [](PlayerbotAI* ai) { return new CastRetaliationAction(ai); };
                creators["mortal strike"] = [](PlayerbotAI* ai) { return new CastMortalStrikeAction(ai); };
                creators["sweeping strikes"] = [](PlayerbotAI* ai) { return new CastSweepingStrikesAction(ai); };
                creators["intercept"] = [](PlayerbotAI* ai) { return new CastInterceptAction(ai); };
                creators["whirlwind"] = [](PlayerbotAI* ai) { return new CastWhirlwindAction(ai); };
                creators["pummel"] = [](PlayerbotAI* ai) { return new CastPummelAction(ai); };
                creators["pummel on enemy healer"] = [](PlayerbotAI* ai) { return new CastPummelOnEnemyHealerAction(ai); };
                creators["recklessness"] = [](PlayerbotAI* ai) { return new CastRecklessnessAction(ai); };
                creators["piercing howl"] = [](PlayerbotAI* ai) { return new CastPiercingHowlAction(ai); };
                creators["rampage"] = [](PlayerbotAI* ai) { return new CastRampageAction(ai); };
                creators["intervene"] = [](PlayerbotAI* ai) { return new CastInterveneAction(ai); };
                creators["spell reflection"] = [](PlayerbotAI* ai) { return new CastSpellReflectionAction(ai); };
                creators["thunder clap on snare target"] = [](PlayerbotAI* ai) { return new CastThunderClapSnareAction(ai); };
                creators["taunt on snare target"] = [](PlayerbotAI* ai) { return new CastTauntOnSnareTargetAction(ai); };
                creators["intercept on enemy healer"] = [](PlayerbotAI* ai) { return new CastInterceptOnEnemyHealerAction(ai); };
                creators["intercept on snare target"] = [](PlayerbotAI* ai) { return new CastInterceptOnSnareTargetAction(ai); };
                creators["bladestorm"] = [](PlayerbotAI* ai) { return new CastBladestormAction(ai); };
                creators["heroic throw"] = [](PlayerbotAI* ai) { return new CastHeroicThrowAction(ai); };
                creators["heroic throw on snare target"] = [](PlayerbotAI* ai) { return new CastHeroicThrowSnareAction(ai); };
                creators["shattering throw"] = [](PlayerbotAI* ai) { return new CastShatteringThrowAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateWarriorPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateWarriorPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateWarriorRaidStrategiesAction(ai); };
            }
        };
    };
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warrior::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::warrior::AiObjectContextInternal());
    triggerContexts.Add(new ai::warrior::TriggerFactoryInternal());
}
