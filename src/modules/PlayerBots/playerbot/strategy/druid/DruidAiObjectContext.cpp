
#include "playerbot/playerbot.h"
#include "DruidActions.h"
#include "DruidAiObjectContext.h"
#include "TankFeralDruidStrategy.h"
#include "DpsFeralDruidStrategy.h"
#include "BalanceDruidStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "DruidTriggers.h"
#include "RestorationDruidStrategy.h"
#include "playerbot/strategy/generic/PullStrategy.h"

namespace ai
{
    namespace druid
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "faerie fire", "dire bear form"); };
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new StealthPlaceholderStrategy(ai); };
                creators["stealthed"] = [](PlayerbotAI* ai) { return new DpsFeralDruidStealthedStrategy(ai); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
                creators["cure"] = [](PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); };
                creators["powershift"] = [](PlayerbotAI* ai) { return new DpsFeralDruidPowershiftStrategy(ai); };
                creators["offheal"] = [](PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidAoePveStrategy(ai); };
                creators["aoe tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidAoePvpStrategy(ai); };
                creators["aoe tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidAoeRaidStrategy(ai); };
                creators["aoe dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidAoePveStrategy(ai); };
                creators["aoe dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidAoePvpStrategy(ai); };
                creators["aoe dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidAoeRaidStrategy(ai); };
                creators["aoe restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidAoePveStrategy(ai); };
                creators["aoe restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidAoePvpStrategy(ai); };
                creators["aoe restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidAoeRaidStrategy(ai); };
                creators["aoe balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidAoePveStrategy(ai); };
                creators["aoe balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidAoePvpStrategy(ai); };
                creators["aoe balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidAoeRaidStrategy(ai); };
            }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidCurePveStrategy(ai); };
                creators["cure tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidCurePvpStrategy(ai); };
                creators["cure tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidCureRaidStrategy(ai); };
                creators["cure dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCurePveStrategy(ai); };
                creators["cure dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCurePvpStrategy(ai); };
                creators["cure dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCureRaidStrategy(ai); };
                creators["cure restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidCurePveStrategy(ai); };
                creators["cure restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidCurePvpStrategy(ai); };
                creators["cure restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidCureRaidStrategy(ai); };
                creators["cure balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidCurePveStrategy(ai); };
                creators["cure balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidCurePvpStrategy(ai); };
                creators["cure balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidCureRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidBuffPveStrategy(ai); };
                creators["buff tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidBuffPvpStrategy(ai); };
                creators["buff tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidBuffRaidStrategy(ai); };
                creators["buff dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBuffPveStrategy(ai); };
                creators["buff dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBuffPvpStrategy(ai); };
                creators["buff dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBuffRaidStrategy(ai); };
                creators["buff restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidBuffPveStrategy(ai); };
                creators["buff restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidBuffPvpStrategy(ai); };
                creators["buff restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidBuffRaidStrategy(ai); };
                creators["buff balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidBuffPveStrategy(ai); };
                creators["buff balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidBuffPvpStrategy(ai); };
                creators["buff balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidBuffRaidStrategy(ai); };
            }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = [](PlayerbotAI* ai) { return new DruidOffhealPveStrategy(ai); };
                creators["offheal pvp"] = [](PlayerbotAI* ai) { return new DruidOffhealPvpStrategy(ai); };
                creators["offheal raid"] = [](PlayerbotAI* ai) { return new DruidOffhealRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidBoostPveStrategy(ai); };
                creators["boost tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidBoostPvpStrategy(ai); };
                creators["boost tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidBoostRaidStrategy(ai); };
                creators["boost dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBoostPveStrategy(ai); };
                creators["boost dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBoostPvpStrategy(ai); };
                creators["boost dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidBoostRaidStrategy(ai); };
                creators["boost restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidBoostPveStrategy(ai); };
                creators["boost restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidBoostPvpStrategy(ai); };
                creators["boost restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidBoostRaidStrategy(ai); };
                creators["boost balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidBoostPveStrategy(ai); };
                creators["boost balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidBoostPvpStrategy(ai); };
                creators["boost balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidBoostRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidCcPveStrategy(ai); };
                creators["cc tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidCcPvpStrategy(ai); };
                creators["cc tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidCcRaidStrategy(ai); };
                creators["cc dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCcPveStrategy(ai); };
                creators["cc dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCcPvpStrategy(ai); };
                creators["cc dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidCcRaidStrategy(ai); };
                creators["cc restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidCcPveStrategy(ai); };
                creators["cc restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidCcPvpStrategy(ai); };
                creators["cc restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidCcRaidStrategy(ai); };
                creators["cc balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidCcPveStrategy(ai); };
                creators["cc balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidCcPvpStrategy(ai); };
                creators["cc balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidCcRaidStrategy(ai); };
            }
        };

        class StealthSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StealthSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["stealth dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidStealthPveStrategy(ai); };
                creators["stealth dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidStealthPvpStrategy(ai); };
                creators["stealth dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidStealthRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bear"] = [](PlayerbotAI* ai) { return new TankFeralDruidPlaceholderStrategy(ai); };
                creators["tank"] = [](PlayerbotAI* ai) { return new TankFeralDruidPlaceholderStrategy(ai); };
                creators["tank feral"] = [](PlayerbotAI* ai) { return new TankFeralDruidPlaceholderStrategy(ai); };
                creators["cat"] = [](PlayerbotAI* ai) { return new DpsFeralDruidPlaceholderStrategy(ai); };
                creators["dps feral"] = [](PlayerbotAI* ai) { return new DpsFeralDruidPlaceholderStrategy(ai); };
                creators["balance"] = [](PlayerbotAI* ai) { return new BalanceDruidPlaceholderStrategy(ai); };
                creators["restoration"] = [](PlayerbotAI* ai) { return new RestorationDruidPlaceholderStrategy(ai); };
                creators["heal"] = [](PlayerbotAI* ai) { return new RestorationDruidPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["tank feral pvp"] = [](PlayerbotAI* ai) { return new TankFeralDruidPvpStrategy(ai); };
                creators["tank feral pve"] = [](PlayerbotAI* ai) { return new TankFeralDruidPveStrategy(ai); };
                creators["tank feral raid"] = [](PlayerbotAI* ai) { return new TankFeralDruidRaidStrategy(ai); };
                creators["dps feral pvp"] = [](PlayerbotAI* ai) { return new DpsFeralDruidPvpStrategy(ai); };
                creators["dps feral pve"] = [](PlayerbotAI* ai) { return new DpsFeralDruidPveStrategy(ai); };
                creators["dps feral raid"] = [](PlayerbotAI* ai) { return new DpsFeralDruidRaidStrategy(ai); };
                creators["restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationDruidPvpStrategy(ai); };
                creators["restoration pve"] = [](PlayerbotAI* ai) { return new RestorationDruidPveStrategy(ai); };
                creators["restoration raid"] = [](PlayerbotAI* ai) { return new RestorationDruidRaidStrategy(ai); };
                creators["balance pvp"] = [](PlayerbotAI* ai) { return new BalanceDruidPvpStrategy(ai); };
                creators["balance pve"] = [](PlayerbotAI* ai) { return new BalanceDruidPveStrategy(ai); };
                creators["balance raid"] = [](PlayerbotAI* ai) { return new BalanceDruidRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["omen of clarity"] = [](PlayerbotAI* ai) { return new OmenOfClarityTrigger(ai); };
                creators["thorns"] = [](PlayerbotAI* ai) { return new ThornsTrigger(ai); };
                creators["thorns on party"] = [](PlayerbotAI* ai) { return new ThornsOnPartyTrigger(ai); };
                creators["bash"] = [](PlayerbotAI* ai) { return new BashInterruptSpellTrigger(ai); };
                creators["faerie fire (feral)"] = [](PlayerbotAI* ai) { return new FaerieFireFeralTrigger(ai); };
                creators["faerie fire"] = [](PlayerbotAI* ai) { return new FaerieFireTrigger(ai); };
                creators["insect swarm"] = [](PlayerbotAI* ai) { return new InsectSwarmTrigger(ai); };
                creators["moonfire"] = [](PlayerbotAI* ai) { return new MoonfireTrigger(ai); };
                creators["nature's grasp"] = [](PlayerbotAI* ai) { return new NaturesGraspTrigger(ai); };
                creators["tiger's fury"] = [](PlayerbotAI* ai) { return new TigersFuryTrigger(ai); };
                creators["rake"] = [](PlayerbotAI* ai) { return new RakeTrigger(ai); };
                creators["mark of the wild"] = [](PlayerbotAI* ai) { return new MarkOfTheWildTrigger(ai); };
                creators["mark of the wild on party"] = [](PlayerbotAI* ai) { return new MarkOfTheWildOnPartyTrigger(ai); };
                creators["gift of the wild on party"] = [](PlayerbotAI* ai) { return new GiftOfTheWildOnPartyTrigger(ai); };
                creators["cure poison"] = [](PlayerbotAI* ai) { return new CurePoisonTrigger(ai); };
                creators["party member cure poison"] = [](PlayerbotAI* ai) { return new PartyMemberCurePoisonTrigger(ai); };
                creators["remove curse"] = [](PlayerbotAI* ai) { return new RemoveCurseTrigger(ai); };
                creators["party member remove curse"] = [](PlayerbotAI* ai) { return new RemoveCurseOnPartyTrigger(ai); };
                creators["entangling roots"] = [](PlayerbotAI* ai) { return new EntanglingRootsTrigger(ai); };
                creators["entangling roots kite"] = [](PlayerbotAI* ai) { return new EntanglingRootsKiteTrigger(ai); };
                creators["entangling roots snare"] = [](PlayerbotAI* ai) { return new EntanglingRootsSnareTrigger(ai); };
                creators["hibernate"] = [](PlayerbotAI* ai) { return new HibernateTrigger(ai); };
                creators["bear form"] = [](PlayerbotAI* ai) { return new BearFormTrigger(ai); };
                creators["cat form"] = [](PlayerbotAI* ai) { return new CatFormTrigger(ai); };
                creators["tree form"] = [](PlayerbotAI* ai) { return new TreeFormTrigger(ai); };
                creators["moonkin form"] = [](PlayerbotAI* ai) { return new MoonkinFormTrigger(ai); };
                creators["eclipse (solar)"] = [](PlayerbotAI* ai) { return new EclipseSolarTrigger(ai); };
                creators["eclipse (lunar)"] = [](PlayerbotAI* ai) { return new EclipseLunarTrigger(ai); };
                creators["bash on enemy healer"] = [](PlayerbotAI* ai) { return new BashInterruptEnemyHealerSpellTrigger(ai); };
                creators["nature's swiftness"] = [](PlayerbotAI* ai) { return new NaturesSwiftnessTrigger(ai); };
                creators["ferocious bite"] = [](PlayerbotAI* ai) { return new FerociousBiteTrigger(ai); };
                creators["claw"] = [](PlayerbotAI* ai) { return new SpellCanBeCastedTrigger(ai, "claw"); };
                creators["rip"] = [](PlayerbotAI* ai) { return new RipTrigger(ai, 3); };
                creators["enrage"] = [](PlayerbotAI* ai) { return new EnrageTrigger(ai); };
                creators["lacerate"] = [](PlayerbotAI* ai) { return new LacerateTrigger(ai); };
                creators["unstealth"] = [](PlayerbotAI* ai) { return new DruidUnstealthTrigger(ai); };
                creators["in stealth"] = [](PlayerbotAI* ai) { return new InStealthTrigger(ai); };
                creators["no stealth"] = [](PlayerbotAI* ai) { return new NoStealthTrigger(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new StealthTrigger(ai); };
                creators["powershift"] = [](PlayerbotAI* ai) { return new PowershiftTrigger(ai); };
                creators["rebirth"] = [](PlayerbotAI* ai) { return new RebirthTrigger(ai); };
                creators["innervate"] = [](PlayerbotAI* ai) { return new InnervateTrigger(ai); };
                creators["lifebloom"] = [](PlayerbotAI* ai) { return new LifebloomTankTrigger(ai); };
                creators["clearcasting"] = [](PlayerbotAI* ai) { return new ClearcastingTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["feral charge - bear"] = [](PlayerbotAI* ai) { return new CastFeralChargeBearAction(ai); };
                creators["feral charge - cat"] = [](PlayerbotAI* ai) { return new CastFeralChargeCatAction(ai); };
                creators["swipe (bear)"] = [](PlayerbotAI* ai) { return new CastSwipeBearAction(ai); };
                creators["faerie fire (feral)"] = [](PlayerbotAI* ai) { return new CastFaerieFireFeralAction(ai); };
                creators["faerie fire"] = [](PlayerbotAI* ai) { return new CastFaerieFireAction(ai); };
                creators["bear form"] = [](PlayerbotAI* ai) { return new CastBearFormAction(ai); };
                creators["dire bear form"] = [](PlayerbotAI* ai) { return new CastDireBearFormAction(ai); };
                creators["moonkin form"] = [](PlayerbotAI* ai) { return new CastMoonkinFormAction(ai); };
                creators["cat form"] = [](PlayerbotAI* ai) { return new CastCatFormAction(ai); };
                creators["tree form"] = [](PlayerbotAI* ai) { return new CastTreeFormAction(ai); };
                creators["travel form"] = [](PlayerbotAI* ai) { return new CastTravelFormAction(ai); };
                creators["aquatic form"] = [](PlayerbotAI* ai) { return new CastAquaticFormAction(ai); };
                creators["caster form"] = [](PlayerbotAI* ai) { return new CastCasterFormAction(ai); };
                creators["mangle (bear)"] = [](PlayerbotAI* ai) { return new CastMangleBearAction(ai); };
                creators["maul"] = [](PlayerbotAI* ai) { return new CastMaulAction(ai); };
                creators["bash"] = [](PlayerbotAI* ai) { return new CastBashAction(ai); };
                creators["swipe"] = [](PlayerbotAI* ai) { return new CastSwipeAction(ai); };
                creators["growl"] = [](PlayerbotAI* ai) { return new CastGrowlAction(ai); };
                creators["demoralizing roar"] = [](PlayerbotAI* ai) { return new CastDemoralizingRoarAction(ai); };
                creators["hibernate"] = [](PlayerbotAI* ai) { return new CastHibernateAction(ai); };
                creators["entangling roots"] = [](PlayerbotAI* ai) { return new CastEntanglingRootsAction(ai); };
                creators["entangling roots on cc"] = [](PlayerbotAI* ai) { return new CastEntanglingRootsCcAction(ai); };
                creators["entangling roots on snare"] = [](PlayerbotAI* ai) { return new CastEntanglingRootsSnareAction(ai); };
                creators["hibernate"] = [](PlayerbotAI* ai) { return new CastHibernateAction(ai); };
                creators["hibernate on cc"] = [](PlayerbotAI* ai) { return new CastHibernateCcAction(ai); };
                creators["wrath"] = [](PlayerbotAI* ai) { return new CastWrathAction(ai); };
                creators["starfall"] = [](PlayerbotAI* ai) { return new CastStarfallAction(ai); };
                creators["insect swarm"] = [](PlayerbotAI* ai) { return new CastInsectSwarmAction(ai); };
                creators["moonfire"] = [](PlayerbotAI* ai) { return new CastMoonfireAction(ai); };
                creators["starfire"] = [](PlayerbotAI* ai) { return new CastStarfireAction(ai); };
                creators["nature's grasp"] = [](PlayerbotAI* ai) { return new CastNaturesGraspAction(ai); };
                creators["claw"] = [](PlayerbotAI* ai) { return new CastClawAction(ai); };
                creators["mangle (cat)"] = [](PlayerbotAI* ai) { return new CastMangleCatAction(ai); };
                creators["swipe (cat)"] = [](PlayerbotAI* ai) { return new CastSwipeCatAction(ai); };
                creators["rake"] = [](PlayerbotAI* ai) { return new CastRakeAction(ai); };
                creators["ferocious bite"] = [](PlayerbotAI* ai) { return new CastFerociousBiteAction(ai); };
                creators["rip"] = [](PlayerbotAI* ai) { return new CastRipAction(ai); };
                creators["cower"] = [](PlayerbotAI* ai) { return new CastCowerAction(ai); };
                creators["survival instincts"] = [](PlayerbotAI* ai) { return new CastSurvivalInstinctsAction(ai); };
                creators["thorns"] = [](PlayerbotAI* ai) { return new CastThornsAction(ai); };
                creators["thorns on party"] = [](PlayerbotAI* ai) { return new CastThornsOnPartyAction(ai); };
                creators["cure poison"] = [](PlayerbotAI* ai) { return new CastCurePoisonAction(ai); };
                creators["cure poison on party"] = [](PlayerbotAI* ai) { return new CastCurePoisonOnPartyAction(ai); };
                creators["abolish poison"] = [](PlayerbotAI* ai) { return new CastAbolishPoisonAction(ai); };
                creators["abolish poison on party"] = [](PlayerbotAI* ai) { return new CastAbolishPoisonOnPartyAction(ai); };
                creators["remove curse"] = [](PlayerbotAI* ai) { return new CastRemoveCurseAction(ai); };
                creators["remove curse on party"] = [](PlayerbotAI* ai) { return new CastRemoveCurseOnPartyAction(ai); };
                creators["berserk"] = [](PlayerbotAI* ai) { return new CastBerserkAction(ai); };
                creators["tiger's fury"] = [](PlayerbotAI* ai) { return new CastTigersFuryAction(ai); };
                creators["mark of the wild"] = [](PlayerbotAI* ai) { return new CastMarkOfTheWildAction(ai); };
                creators["mark of the wild on party"] = [](PlayerbotAI* ai) { return new CastMarkOfTheWildOnPartyAction(ai); };
                creators["gift of the wild on party"] = [](PlayerbotAI* ai) { return new CastGiftOfTheWildOnPartyAction(ai); };
                creators["regrowth"] = [](PlayerbotAI* ai) { return new CastRegrowthAction(ai); };
                creators["rejuvenation"] = [](PlayerbotAI* ai) { return new CastRejuvenationAction(ai); };
                creators["healing touch"] = [](PlayerbotAI* ai) { return new CastHealingTouchAction(ai); };
                creators["regrowth on party"] = [](PlayerbotAI* ai) { return new CastRegrowthOnPartyAction(ai); };
                creators["rejuvenation on party"] = [](PlayerbotAI* ai) { return new CastRejuvenationOnPartyAction(ai); };
                creators["healing touch on party"] = [](PlayerbotAI* ai) { return new CastHealingTouchOnPartyAction(ai); };
                creators["rebirth"] = [](PlayerbotAI* ai) { return new CastRebirthAction(ai); };
                creators["revive"] = [](PlayerbotAI* ai) { return new CastReviveAction(ai); };
                creators["barskin"] = [](PlayerbotAI* ai) { return new CastBarskinAction(ai); };
                creators["lacerate"] = [](PlayerbotAI* ai) { return new CastLacerateAction(ai); };
                creators["hurricane"] = [](PlayerbotAI* ai) { return new CastHurricaneAction(ai); };
                creators["innervate"] = [](PlayerbotAI* ai) { return new CastInnervateAction(ai); };
                creators["tranquility"] = [](PlayerbotAI* ai) { return new CastTranquilityAction(ai); };
                creators["bash on enemy healer"] = [](PlayerbotAI* ai) { return new CastBashOnEnemyHealerAction(ai); };
                creators["omen of clarity"] = [](PlayerbotAI* ai) { return new CastOmenOfClarityAction(ai); };
                creators["nature's swiftness"] = [](PlayerbotAI* ai) { return new CastNaturesSwiftnessAction(ai); };
                creators["dash"] = [](PlayerbotAI* ai) { return new CastDashAction(ai); };
                creators["shred"] = [](PlayerbotAI* ai) { return new CastShredAction(ai); };
                creators["ravage"] = [](PlayerbotAI* ai) { return new CastRavageAction(ai); };
                creators["pounce"] = [](PlayerbotAI* ai) { return new CastPounceAction(ai); };
                creators["frenzied regeneration"] = [](PlayerbotAI* ai) { return new CastFrenziedRegenerationAction(ai); };
                creators["challenging roar"] = [](PlayerbotAI* ai) { return new CastChallengingRoarAction(ai); };
                creators["enrage"] = [](PlayerbotAI* ai) { return new CastEnrageAction(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new CastProwlAction(ai); };
                creators["unstealth"] = [](PlayerbotAI* ai) { return new DruidUnstealthAction(ai); };
                creators["check stealth"] = [](PlayerbotAI* ai) { return new CheckStealthAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateDruidPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateDruidPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateDruidRaidStrategiesAction(ai); };
                creators["lifebloom"] = [](PlayerbotAI* ai) { return new CastLifebloomAction(ai); };
            }
        };
    };
};

DruidAiObjectContext::DruidAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::druid::StrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::StealthSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::ClassSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::druid::AiObjectContextInternal());
    triggerContexts.Add(new ai::druid::TriggerFactoryInternal());
}
