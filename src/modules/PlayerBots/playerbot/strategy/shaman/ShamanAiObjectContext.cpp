
#include "playerbot/playerbot.h"
#include "ShamanActions.h"
#include "ShamanAiObjectContext.h"
#include "ShamanTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "ElementalShamanStrategy.h"
#include "RestorationShamanStrategy.h"
#include "EnhancementShamanStrategy.h"

namespace ai
{
    namespace shaman
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
                creators["cure"] = [](PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "lightning bolt"); };
                creators["offheal"] = [](PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanAoePveStrategy(ai); };
                creators["aoe elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanAoePvpStrategy(ai); };
                creators["aoe elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanAoeRaidStrategy(ai); };
                creators["aoe restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanAoePveStrategy(ai); };
                creators["aoe restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanAoePvpStrategy(ai); };
                creators["aoe restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanAoeRaidStrategy(ai); };
                creators["aoe enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanAoePveStrategy(ai); };
                creators["aoe enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanAoePvpStrategy(ai); };
                creators["aoe enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanAoeRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanCcPveStrategy(ai); };
                creators["cc elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanCcPvpStrategy(ai); };
                creators["cc elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanCcRaidStrategy(ai); };
                creators["cc restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanCcPveStrategy(ai); };
                creators["cc restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanCcPvpStrategy(ai); };
                creators["cc restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanCcRaidStrategy(ai); };
                creators["cc enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanCcPveStrategy(ai); };
                creators["cc enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanCcPvpStrategy(ai); };
                creators["cc enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanCcRaidStrategy(ai); };
            }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanCurePveStrategy(ai); };
                creators["cure elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanCurePvpStrategy(ai); };
                creators["cure elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanCureRaidStrategy(ai); };
                creators["cure restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanCurePveStrategy(ai); };
                creators["cure restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanCurePvpStrategy(ai); };
                creators["cure restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanCureRaidStrategy(ai); };
                creators["cure enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanCurePveStrategy(ai); };
                creators["cure enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanCurePvpStrategy(ai); };
                creators["cure enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanCureRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanBuffPveStrategy(ai); };
                creators["buff elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanBuffPvpStrategy(ai); };
                creators["buff elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanBuffRaidStrategy(ai); };
                creators["buff restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanBuffPveStrategy(ai); };
                creators["buff restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanBuffPvpStrategy(ai); };
                creators["buff restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanBuffRaidStrategy(ai); };
                creators["buff enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanBuffPveStrategy(ai); };
                creators["buff enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanBuffPvpStrategy(ai); };
                creators["buff enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanBuffRaidStrategy(ai); };
            }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = [](PlayerbotAI* ai) { return new ShamanOffhealPveStrategy(ai); };
                creators["offheal pvp"] = [](PlayerbotAI* ai) { return new ShamanOffhealPvpStrategy(ai); };
                creators["offheal raid"] = [](PlayerbotAI* ai) { return new ShamanOffhealRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanBoostPveStrategy(ai); };
                creators["boost elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanBoostPvpStrategy(ai); };
                creators["boost elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanBoostRaidStrategy(ai); };
                creators["boost restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanBoostPveStrategy(ai); };
                creators["boost restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanBoostPvpStrategy(ai); };
                creators["boost restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanBoostRaidStrategy(ai); };
                creators["boost enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanBoostPveStrategy(ai); };
                creators["boost enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanBoostPvpStrategy(ai); };
                creators["boost enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanBoostRaidStrategy(ai); };
            }
        };

        class EarthTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            EarthTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["totems"] = [](PlayerbotAI* ai) { return new ShamanTotemsPlaceholderStrategy(ai); };
                creators["totembar elements"] = [](PlayerbotAI* ai) { return new ShamanTotemBarElementsStrategy(ai); };
                creators["totembar ancestors"] = [](PlayerbotAI* ai) { return new ShamanTotemBarAncestorsStrategy(ai); };
                creators["totembar spirits"] = [](PlayerbotAI* ai) { return new ShamanTotemBarSpiritsStrategy(ai); };
                creators["totem earth stoneclaw"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem earth stoneclaw", "earth totem", "stoneclaw totem"); };
                creators["totem earth stoneskin"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem earth stoneskin", "earth totem", "stoneskin totem"); };
                creators["totem earth earthbind"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem earth earthbind", "earth totem", "earthbind totem"); };
                creators["totem earth strength"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem earth strength", "earth totem", "strength of earth totem"); };
                creators["totem earth tremor"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem earth tremor", "earth totem", "tremor totem"); };
            }
        };

        class FireTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            FireTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["totems"] = [](PlayerbotAI* ai) { return new ShamanTotemsPlaceholderStrategy(ai); };
                creators["totembar elements"] = [](PlayerbotAI* ai) { return new ShamanTotemBarElementsStrategy(ai); };
                creators["totembar ancestors"] = [](PlayerbotAI* ai) { return new ShamanTotemBarAncestorsStrategy(ai); };
                creators["totembar spirits"] = [](PlayerbotAI* ai) { return new ShamanTotemBarSpiritsStrategy(ai); };
                creators["totem fire nova"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire nova", "fire totem", "fire nova"); };
                creators["totem fire flametongue"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire flametongue", "fire totem", "flametongue totem"); };
                creators["totem fire resistance"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire resistance", "fire totem", "frost resistance totem"); };
                creators["totem fire magma"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire magma", "fire totem", "magma totem"); };
                creators["totem fire searing"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire searing", "fire totem", "searing totem"); };
                creators["totem fire wrath"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem fire wrath", "fire totem", "totem of wrath"); };
            }
        };

        class WaterTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            WaterTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["totems"] = [](PlayerbotAI* ai) { return new ShamanTotemsPlaceholderStrategy(ai); };
                creators["totembar elements"] = [](PlayerbotAI* ai) { return new ShamanTotemBarElementsStrategy(ai); };
                creators["totembar ancestors"] = [](PlayerbotAI* ai) { return new ShamanTotemBarAncestorsStrategy(ai); };
                creators["totembar spirits"] = [](PlayerbotAI* ai) { return new ShamanTotemBarSpiritsStrategy(ai); };
                creators["totem water cleansing"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem water cleansing", "water totem", "disease cleansing totem"); };
                creators["totem water resistance"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem water resistance", "water totem", "fire resistance totem"); };
                creators["totem water healing"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem water healing", "water totem", "healing stream totem"); };
                creators["totem water mana"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem water mana", "water totem", "mana spring totem"); };
                creators["totem water poison"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem water poison", "water totem", "poison cleansing totem"); };
            }
        };

        class AirTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AirTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["totems"] = [](PlayerbotAI* ai) { return new ShamanTotemsPlaceholderStrategy(ai); };
                creators["totembar elements"] = [](PlayerbotAI* ai) { return new ShamanTotemBarElementsStrategy(ai); };
                creators["totembar ancestors"] = [](PlayerbotAI* ai) { return new ShamanTotemBarAncestorsStrategy(ai); };
                creators["totembar spirits"] = [](PlayerbotAI* ai) { return new ShamanTotemBarSpiritsStrategy(ai); };
                creators["totem air grace"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air grace", "air totem", "grace of air totem"); };
                creators["totem air grounding"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air grounding", "air totem", "grounding totem"); };
                creators["totem air resistance"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air resistance", "air totem", "nature resistance totem"); };
                creators["totem air tranquil"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air tranquil", "air totem", "tranquil air totem"); };
                creators["totem air windfury"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air windfury", "air totem", "windfury totem"); };
                creators["totem air windwall"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air windwall", "air totem", "windwall totem"); };
                creators["totem air wrath"] = [](PlayerbotAI* ai) { return new ShamanManualTotemStrategy(ai, "totem air wrath", "air totem", "wrath of air totem"); };
            }
        };

        class TotemsSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            TotemsSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["totems elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanTotemsPveStrategy(ai); };
                creators["totems elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanTotemsPvpStrategy(ai); };
                creators["totems elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanTotemsRaidStrategy(ai); };
                creators["totems restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanTotemsPveStrategy(ai); };
                creators["totems restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanTotemsPvpStrategy(ai); };
                creators["totems restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanTotemsRaidStrategy(ai); };
                creators["totems enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanTotemsPveStrategy(ai); };
                creators["totems enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanTotemsPvpStrategy(ai); };
                creators["totems enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanTotemsRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["restoration"] = [](PlayerbotAI* ai) { return new RestorationShamanPlaceholderStrategy(ai); };
                creators["heal"] = [](PlayerbotAI* ai) { return new RestorationShamanPlaceholderStrategy(ai); };
                creators["enhancement"] = [](PlayerbotAI* ai) { return new EnhancementShamanPlaceholderStrategy(ai); };
                creators["elemental"] = [](PlayerbotAI* ai) { return new ElementalShamanPlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["elemental pvp"] = [](PlayerbotAI* ai) { return new ElementalShamanPvpStrategy(ai); };
                creators["elemental pve"] = [](PlayerbotAI* ai) { return new ElementalShamanPveStrategy(ai); };
                creators["elemental raid"] = [](PlayerbotAI* ai) { return new ElementalShamanRaidStrategy(ai); };
                creators["restoration pvp"] = [](PlayerbotAI* ai) { return new RestorationShamanPvpStrategy(ai); };
                creators["restoration pve"] = [](PlayerbotAI* ai) { return new RestorationShamanPveStrategy(ai); };
                creators["restoration raid"] = [](PlayerbotAI* ai) { return new RestorationShamanRaidStrategy(ai); };
                creators["enhancement pvp"] = [](PlayerbotAI* ai) { return new EnhancementShamanPvpStrategy(ai); };
                creators["enhancement pve"] = [](PlayerbotAI* ai) { return new EnhancementShamanPveStrategy(ai); };
                creators["enhancement raid"] = [](PlayerbotAI* ai) { return new EnhancementShamanRaidStrategy(ai); };
            }
        };
    };
};

namespace ai
{
    namespace shaman
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["wind shear"] = [](PlayerbotAI* ai) { return new WindShearInterruptSpellTrigger(ai); };
                creators["purge"] = [](PlayerbotAI* ai) { return new PurgeTrigger(ai); };
                creators["shaman weapon"] = [](PlayerbotAI* ai) { return new ShamanWeaponTrigger(ai); };
                creators["water shield"] = [](PlayerbotAI* ai) { return new WaterShieldTrigger(ai); };
                creators["lightning shield"] = [](PlayerbotAI* ai) { return new LightningShieldTrigger(ai); };
                creators["water breathing"] = [](PlayerbotAI* ai) { return new WaterBreathingTrigger(ai); };
                creators["water walking"] = [](PlayerbotAI* ai) { return new WaterWalkingTrigger(ai); };
                creators["water breathing on party"] = [](PlayerbotAI* ai) { return new WaterBreathingOnPartyTrigger(ai); };
                creators["water walking on party"] = [](PlayerbotAI* ai) { return new WaterWalkingOnPartyTrigger(ai); };
                creators["cleanse spirit poison"] = [](PlayerbotAI* ai) { return new CleanseSpiritPoisonTrigger(ai); };
                creators["cleanse spirit curse"] = [](PlayerbotAI* ai) { return new CleanseSpiritCurseTrigger(ai); };
                creators["cleanse spirit disease"] = [](PlayerbotAI* ai) { return new CleanseSpiritDiseaseTrigger(ai); };
                creators["party member cleanse spirit poison"] = [](PlayerbotAI* ai) { return new PartyMemberCleanseSpiritPoisonTrigger(ai); };
                creators["party member cleanse spirit curse"] = [](PlayerbotAI* ai) { return new PartyMemberCleanseSpiritCurseTrigger(ai); };
                creators["party member cleanse spirit disease"] = [](PlayerbotAI* ai) { return new PartyMemberCleanseSpiritDiseaseTrigger(ai); };
                creators["shock"] = [](PlayerbotAI* ai) { return new ShockTrigger(ai); };
                creators["frost shock snare"] = [](PlayerbotAI* ai) { return new FrostShockSnareTrigger(ai); };
                creators["heroism"] = [](PlayerbotAI* ai) { return new HeroismTrigger(ai); };
                creators["bloodlust"] = [](PlayerbotAI* ai) { return new BloodlustTrigger(ai); };
                creators["maelstrom weapon"] = [](PlayerbotAI* ai) { return new MaelstromWeaponTrigger(ai); };
                creators["wind shear on enemy healer"] = [](PlayerbotAI* ai) { return new WindShearInterruptEnemyHealerSpellTrigger(ai); };
                creators["cure poison"] = [](PlayerbotAI* ai) { return new CurePoisonTrigger(ai); };
                creators["party member cure poison"] = [](PlayerbotAI* ai) { return new PartyMemberCurePoisonTrigger(ai); };
                creators["cure disease"] = [](PlayerbotAI* ai) { return new CureDiseaseTrigger(ai); };
                creators["party member cure disease"] = [](PlayerbotAI* ai) { return new PartyMemberCureDiseaseTrigger(ai); };
                creators["fire totem"] = [](PlayerbotAI* ai) { return new FireTotemTrigger(ai); };
                creators["fire totem aoe"] = [](PlayerbotAI* ai) { return new FireTotemAoeTrigger(ai); };
                creators["earth totem"] = [](PlayerbotAI* ai) { return new EarthTotemTrigger(ai); };
                creators["water totem"] = [](PlayerbotAI* ai) { return new WaterTotemTrigger(ai); };
                creators["air totem"] = [](PlayerbotAI* ai) { return new AirTotemTrigger(ai); };
                creators["call of the elements"] = [](PlayerbotAI* ai) { return new TotemsAreNotSummonedTrigger(ai); };
                creators["call of the ancestors"] = [](PlayerbotAI* ai) { return new TotemsAreNotSummonedTrigger(ai); };
                creators["call of the spirits"] = [](PlayerbotAI* ai) { return new TotemsAreNotSummonedTrigger(ai); };
                creators["totemic recall"] = [](PlayerbotAI* ai) { return new ReadyToRemoveTotemsTrigger(ai); };
                creators["earth shield on party tank"] = [](PlayerbotAI* ai) { return new PartyTankEarthShieldTrigger(ai); };
                creators["chain lightning"] = [](PlayerbotAI* ai) { return new ChainLightningTrigger(ai); };
                creators["stormstrike"] = [](PlayerbotAI* ai) { return new StormstrikeTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["water shield"] = [](PlayerbotAI* ai) { return new CastWaterShieldAction(ai); };
                creators["lightning shield"] = [](PlayerbotAI* ai) { return new CastLightningShieldAction(ai); };
                creators["strength of earth totem"] = [](PlayerbotAI* ai) { return new CastStrengthOfEarthTotemAction(ai); };
                creators["flametongue totem"] = [](PlayerbotAI* ai) { return new CastFlametongueTotemAction(ai); };
                creators["searing totem"] = [](PlayerbotAI* ai) { return new CastSearingTotemAction(ai); };
                creators["magma totem"] = [](PlayerbotAI* ai) { return new CastMagmaTotemAction(ai); };
                creators["totem of wrath"] = [](PlayerbotAI* ai) { return new CastTotemOfWrathAction(ai); };
                creators["windfury totem"] = [](PlayerbotAI* ai) { return new CastWindfuryTotemAction(ai); };
                creators["stoneskin totem"] = [](PlayerbotAI* ai) { return new CastStoneskinTotemAction(ai); };
                creators["stoneclaw totem"] = [](PlayerbotAI* ai) { return new CastStoneclawTotemAction(ai); };
                creators["grounding totem"] = [](PlayerbotAI* ai) { return new CastGroundingTotemAction(ai); };
                creators["grace of air totem"] = [](PlayerbotAI* ai) { return new CastGraceOfAirTotemAction(ai); };
                creators["windwall totem"] = [](PlayerbotAI* ai) { return new CastWindwallTotemAction(ai); };
                creators["mana spring totem"] = [](PlayerbotAI* ai) { return new CastManaSpringTotemAction(ai); };
                creators["mana tide totem"] = [](PlayerbotAI* ai) { return new CastManaTideTotemAction(ai); };
                creators["earthbind totem"] = [](PlayerbotAI* ai) { return new CastEarthbindTotemAction(ai); };
                creators["tremor totem"] = [](PlayerbotAI* ai) { return new CastTremorTotemAction(ai); };
                creators["tranquil air totem"] = [](PlayerbotAI* ai) { return new CastTranquilAirTotemAction(ai); };
                creators["healing stream totem"] = [](PlayerbotAI* ai) { return new CastHealingStreamTotemAction(ai); };
                creators["wrath of air totem"] = [](PlayerbotAI* ai) { return new CastWrathOfAirTotemAction(ai); };
                creators["frost resistance totem"] = [](PlayerbotAI* ai) { return new CastFrostResistanceTotemAction(ai); };
                creators["fire resistance totem"] = [](PlayerbotAI* ai) { return new CastFireResistanceTotemAction(ai); };
                creators["nature resistance totem"] = [](PlayerbotAI* ai) { return new CastNatureResistanceTotemAction(ai); };
                creators["disease cleansing totem"] = [](PlayerbotAI* ai) { return new CastDiseaseCleansingTotemAction(ai); };
                creators["poison cleansing totem"] = [](PlayerbotAI* ai) { return new CastPoisonCleansingTotemAction(ai); };
                creators["wind shear"] = [](PlayerbotAI* ai) { return new CastWindShearAction(ai); };
                creators["wind shear on enemy healer"] = [](PlayerbotAI* ai) { return new CastWindShearOnEnemyHealerAction(ai); };
                creators["rockbiter weapon"] = [](PlayerbotAI* ai) { return new CastRockbiterWeaponAction(ai); };
                creators["flametongue weapon"] = [](PlayerbotAI* ai) { return new CastFlametongueWeaponAction(ai); };
                creators["frostbrand weapon"] = [](PlayerbotAI* ai) { return new CastFrostbrandWeaponAction(ai); };
                creators["windfury weapon"] = [](PlayerbotAI* ai) { return new CastWindfuryWeaponAction(ai); };
                creators["earthliving weapon"] = [](PlayerbotAI* ai) { return new CastEarthlivingWeaponAction(ai); };
                creators["purge"] = [](PlayerbotAI* ai) { return new CastPurgeAction(ai); };
                creators["healing wave"] = [](PlayerbotAI* ai) { return new CastHealingWaveAction(ai); };
                creators["lesser healing wave"] = [](PlayerbotAI* ai) { return new CastLesserHealingWaveAction(ai); };
                creators["healing wave on party"] = [](PlayerbotAI* ai) { return new CastHealingWaveOnPartyAction(ai); };
                creators["lesser healing wave on party"] = [](PlayerbotAI* ai) { return new CastLesserHealingWaveOnPartyAction(ai); };
                creators["earth shield"] = [](PlayerbotAI* ai) { return new CastEarthShieldAction(ai); };
                creators["earth shield on party"] = [](PlayerbotAI* ai) { return new CastEarthShieldOnPartyAction(ai); };
                creators["chain heal"] = [](PlayerbotAI* ai) { return new CastChainHealAction(ai); };
                creators["riptide"] = [](PlayerbotAI* ai) { return new CastRiptideAction(ai); };
                creators["riptide on party"] = [](PlayerbotAI* ai) { return new CastRiptideOnPartyAction(ai); };
                creators["stormstrike"] = [](PlayerbotAI* ai) { return new CastStormstrikeAction(ai); };
                creators["lava lash"] = [](PlayerbotAI* ai) { return new CastLavaLashAction(ai); };
                creators["fire nova"] = [](PlayerbotAI* ai) { return new CastFireNovaAction(ai); };
                creators["ancestral spirit"] = [](PlayerbotAI* ai) { return new CastAncestralSpiritAction(ai); };
                creators["water walking"] = [](PlayerbotAI* ai) { return new CastWaterWalkingAction(ai); };
                creators["water breathing"] = [](PlayerbotAI* ai) { return new CastWaterBreathingAction(ai); };
                creators["water walking on party"] = [](PlayerbotAI* ai) { return new CastWaterWalkingOnPartyAction(ai); };
                creators["water breathing on party"] = [](PlayerbotAI* ai) { return new CastWaterBreathingOnPartyAction(ai); };
                creators["cleanse spirit"] = [](PlayerbotAI* ai) { return new CastCleanseSpiritAction(ai); };
                creators["cleanse spirit poison on party"] = [](PlayerbotAI* ai) { return new CastCleanseSpiritPoisonOnPartyAction(ai); };
                creators["cleanse spirit disease on party"] = [](PlayerbotAI* ai) { return new CastCleanseSpiritDiseaseOnPartyAction(ai); };
                creators["cleanse spirit curse on party"] = [](PlayerbotAI* ai) { return new CastCleanseSpiritCurseOnPartyAction(ai); };
                creators["flame shock"] = [](PlayerbotAI* ai) { return new CastFlameShockAction(ai); };
                creators["earth shock"] = [](PlayerbotAI* ai) { return new CastEarthShockAction(ai); };
                creators["frost shock"] = [](PlayerbotAI* ai) { return new CastFrostShockAction(ai); };
                creators["chain lightning"] = [](PlayerbotAI* ai) { return new CastChainLightningAction(ai); };
                creators["lightning bolt"] = [](PlayerbotAI* ai) { return new CastLightningBoltAction(ai); };
                creators["thunderstorm"] = [](PlayerbotAI* ai) { return new CastThunderstormAction(ai); };
                creators["heroism"] = [](PlayerbotAI* ai) { return new CastHeroismAction(ai); };
                creators["bloodlust"] = [](PlayerbotAI* ai) { return new CastBloodlustAction(ai); };
                creators["cure disease"] = [](PlayerbotAI* ai) { return new CastCureDiseaseAction(ai); };
                creators["cure disease on party"] = [](PlayerbotAI* ai) { return new CastCureDiseaseOnPartyAction(ai); };
                creators["cure poison"] = [](PlayerbotAI* ai) { return new CastCurePoisonAction(ai); };
                creators["cure poison on party"] = [](PlayerbotAI* ai) { return new CastCurePoisonOnPartyAction(ai); };
                creators["ghost wolf"] = [](PlayerbotAI* ai) { return new CastGhostWolfAction(ai); };
                creators["call of the elements"] = [](PlayerbotAI* ai) { return new CastCallOfTheElements(ai); };
                creators["call of the ancestors"] = [](PlayerbotAI* ai) { return new CastCallOfTheAncestors(ai); };
                creators["call of the spirits"] = [](PlayerbotAI* ai) { return new CastCallOfTheSpirits(ai); };
                creators["totemic recall"] = [](PlayerbotAI* ai) { return new CastTotemicRecall(ai); };
                creators["set totembars on levelup"] = [](PlayerbotAI* ai) { return new SetTotemBars(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateShamanPveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateShamanPvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateShamanRaidStrategiesAction(ai); };
                creators["earth shield on party tank"] = [](PlayerbotAI* ai) { return new CastEarthShieldOnPartyTankAction(ai); };
            }
        };
    };
};

ShamanAiObjectContext::ShamanAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::shaman::StrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::EarthTotemStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::FireTotemStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::WaterTotemStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::AirTotemStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::TotemsSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::BoostSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::shaman::AiObjectContextInternal());
    triggerContexts.Add(new ai::shaman::TriggerFactoryInternal());
}
