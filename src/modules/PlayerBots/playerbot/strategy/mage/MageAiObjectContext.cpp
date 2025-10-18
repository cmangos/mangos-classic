
#include "playerbot/playerbot.h"
#include "playerbot/strategy/Strategy.h"
#include "MageActions.h"
#include "MageAiObjectContext.h"
#include "FrostMageStrategy.h"
#include "ArcaneMageStrategy.h"
#include "FireMageStrategy.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "MageTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    namespace mage
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["cure"] = [](PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); };
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
                creators["aoe frost pve"] = [](PlayerbotAI* ai) { return new FrostMageAoePveStrategy(ai); };
                creators["aoe frost pvp"] = [](PlayerbotAI* ai) { return new FrostMageAoePvpStrategy(ai); };
                creators["aoe frost raid"] = [](PlayerbotAI* ai) { return new FrostMageAoeRaidStrategy(ai); };
                creators["aoe fire pve"] = [](PlayerbotAI* ai) { return new FireMageAoePveStrategy(ai); };
                creators["aoe fire pvp"] = [](PlayerbotAI* ai) { return new FireMageAoePvpStrategy(ai); };
                creators["aoe fire raid"] = [](PlayerbotAI* ai) { return new FireMageAoeRaidStrategy(ai); };
                creators["aoe arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMageAoePveStrategy(ai); };
                creators["aoe arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMageAoePvpStrategy(ai); };
                creators["aoe arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageAoeRaidStrategy(ai); };
            }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure frost pve"] = [](PlayerbotAI* ai) { return new FrostMageCurePveStrategy(ai); };
                creators["cure frost pvp"] = [](PlayerbotAI* ai) { return new FrostMageCurePvpStrategy(ai); };
                creators["cure frost raid"] = [](PlayerbotAI* ai) { return new FrostMageCureRaidStrategy(ai); };
                creators["cure fire pve"] = [](PlayerbotAI* ai) { return new FireMageCurePveStrategy(ai); };
                creators["cure fire pvp"] = [](PlayerbotAI* ai) { return new FireMageCurePvpStrategy(ai); };
                creators["cure fire raid"] = [](PlayerbotAI* ai) { return new FireMageCureRaidStrategy(ai); };
                creators["cure arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMageCurePveStrategy(ai); };
                creators["cure arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMageCurePvpStrategy(ai); };
                creators["cure arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageCureRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff frost pve"] = [](PlayerbotAI* ai) { return new FrostMageBuffPveStrategy(ai); };
                creators["buff frost pvp"] = [](PlayerbotAI* ai) { return new FrostMageBuffPvpStrategy(ai); };
                creators["buff frost raid"] = [](PlayerbotAI* ai) { return new FrostMageBuffRaidStrategy(ai); };
                creators["buff fire pve"] = [](PlayerbotAI* ai) { return new FireMageBuffPveStrategy(ai); };
                creators["buff fire pvp"] = [](PlayerbotAI* ai) { return new FireMageBuffPvpStrategy(ai); };
                creators["buff fire raid"] = [](PlayerbotAI* ai) { return new FireMageBuffRaidStrategy(ai); };
                creators["buff arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMageBuffPveStrategy(ai); };
                creators["buff arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMageBuffPvpStrategy(ai); };
                creators["buff arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost frost pve"] = [](PlayerbotAI* ai) { return new FrostMageBoostPveStrategy(ai); };
                creators["boost frost pvp"] = [](PlayerbotAI* ai) { return new FrostMageBoostPvpStrategy(ai); };
                creators["boost frost raid"] = [](PlayerbotAI* ai) { return new FrostMageBoostRaidStrategy(ai); };
                creators["boost fire pve"] = [](PlayerbotAI* ai) { return new FireMageBoostPveStrategy(ai); };
                creators["boost fire pvp"] = [](PlayerbotAI* ai) { return new FireMageBoostPvpStrategy(ai); };
                creators["boost fire raid"] = [](PlayerbotAI* ai) { return new FireMageBoostRaidStrategy(ai); };
                creators["boost arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMageBoostPveStrategy(ai); };
                creators["boost arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMageBoostPvpStrategy(ai); };
                creators["boost arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageBoostRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc fire pve"] = [](PlayerbotAI* ai) { return new FireMageCcPveStrategy(ai); };
                creators["cc fire pvp"] = [](PlayerbotAI* ai) { return new FireMageCcPvpStrategy(ai); };
                creators["cc fire raid"] = [](PlayerbotAI* ai) { return new FireMageCcRaidStrategy(ai); };
                creators["cc arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMageCcPveStrategy(ai); };
                creators["cc arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMageCcPvpStrategy(ai); };
                creators["cc arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageCcRaidStrategy(ai); };
                creators["cc frost pve"] = [](PlayerbotAI* ai) { return new FrostMageCcPveStrategy(ai); };
                creators["cc frost pvp"] = [](PlayerbotAI* ai) { return new FrostMageCcPvpStrategy(ai); };
                creators["cc frost raid"] = [](PlayerbotAI* ai) { return new FrostMageCcRaidStrategy(ai); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["fire"] = [](PlayerbotAI* ai) { return new FireMagePlaceholderStrategy(ai); };
                creators["arcane"] = [](PlayerbotAI* ai) { return new ArcaneMagePlaceholderStrategy(ai); };
                creators["frost"] = [](PlayerbotAI* ai) { return new FrostMagePlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["frost pvp"] = [](PlayerbotAI* ai) { return new FrostMagePvpStrategy(ai); };
                creators["frost pve"] = [](PlayerbotAI* ai) { return new FrostMagePveStrategy(ai); };
                creators["frost raid"] = [](PlayerbotAI* ai) { return new FrostMageRaidStrategy(ai); };
                creators["fire pvp"] = [](PlayerbotAI* ai) { return new FireMagePvpStrategy(ai); };
                creators["fire pve"] = [](PlayerbotAI* ai) { return new FireMagePveStrategy(ai); };
                creators["fire raid"] = [](PlayerbotAI* ai) { return new FireMageRaidStrategy(ai); };
                creators["arcane pvp"] = [](PlayerbotAI* ai) { return new ArcaneMagePvpStrategy(ai); };
                creators["arcane pve"] = [](PlayerbotAI* ai) { return new ArcaneMagePveStrategy(ai); };
                creators["arcane raid"] = [](PlayerbotAI* ai) { return new ArcaneMageRaidStrategy(ai); };
            }
        };
    };
};

namespace ai
{
    namespace mage
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["fireball"] = [](PlayerbotAI* ai) { return new FireballTrigger(ai); };
                creators["pyroblast"] = [](PlayerbotAI* ai) { return new PyroblastTrigger(ai); };
                creators["combustion"] = [](PlayerbotAI* ai) { return new CombustionTrigger(ai); };
                creators["icy veins"] = [](PlayerbotAI* ai) { return new IcyVeinsTrigger(ai); };
                creators["arcane intellect"] = [](PlayerbotAI* ai) { return new ArcaneIntellectTrigger(ai); };
                creators["arcane intellect on party"] = [](PlayerbotAI* ai) { return new ArcaneIntellectOnPartyTrigger(ai); };
                creators["arcane brilliance on party"] = [](PlayerbotAI* ai) { return new ArcaneBrillianceOnPartyTrigger(ai); };
                creators["any mage armor"] = [](PlayerbotAI* ai) { return new AnyMageArmorTrigger(ai); };
                creators["mage armor"] = [](PlayerbotAI* ai) { return new MageArmorTrigger(ai); };
                creators["ice armor"] = [](PlayerbotAI* ai) { return new IceArmorTrigger(ai); };
                creators["molten armor"] = [](PlayerbotAI* ai) { return new MoltenArmorTrigger(ai); };
                creators["remove curse"] = [](PlayerbotAI* ai) { return new RemoveCurseTrigger(ai); };
                creators["remove curse on party"] = [](PlayerbotAI* ai) { return new PartyMemberRemoveCurseTrigger(ai); };
                creators["counterspell"] = [](PlayerbotAI* ai) { return new CounterspellInterruptSpellTrigger(ai); };
                creators["polymorph"] = [](PlayerbotAI* ai) { return new PolymorphTrigger(ai); };
                creators["spellsteal"] = [](PlayerbotAI* ai) { return new SpellstealTrigger(ai); };
                creators["missile barrage"] = [](PlayerbotAI* ai) { return new MissileBarrageTrigger(ai); };
                creators["arcane blast"] = [](PlayerbotAI* ai) { return new ArcaneBlastTrigger(ai); };
                creators["counterspell on enemy healer"] = [](PlayerbotAI* ai) { return new CounterspellEnemyHealerTrigger(ai); };
                creators["arcane power"] = [](PlayerbotAI* ai) { return new ArcanePowerTrigger(ai); };
                creators["presence of mind"] = [](PlayerbotAI* ai) { return new PresenceOfMindTrigger(ai); };
                creators["presence of mind aura"] = [](PlayerbotAI* ai) { return new PresenceOfMindAuraTrigger(ai); };
                creators["fire ward"] = [](PlayerbotAI* ai) { return new FireWardTrigger(ai); };
                creators["frost ward"] = [](PlayerbotAI* ai) { return new FrostWardTrigger(ai); };
                creators["blink"] = [](PlayerbotAI* ai) { return new BlinkTrigger(ai); };
                creators["mana shield"] = [](PlayerbotAI* ai) { return new ManaShieldTrigger(ai); };
                creators["summon water elemental"] = [](PlayerbotAI* ai) { return new WaterElementalBoostTrigger(ai); };
                creators["ice lance"] = [](PlayerbotAI* ai) { return new IceLanceTrigger(ai); };
                creators["fire spells locked"] = [](PlayerbotAI* ai) { return new FireSpellsLocked(ai); };
                creators["cold snap"] = [](PlayerbotAI* ai) { return new ColdSnapTrigger(ai); };
                creators["ice barrier"] = [](PlayerbotAI* ai) { return new IceBarrierTrigger(ai); };
                creators["hot streak"] = [](PlayerbotAI* ai) { return new HotStreakTrigger(ai); };
                creators["free fireball"] = [](PlayerbotAI* ai) { return new FireballOrFrostfireBoltFreeTrigger(ai); };
                creators["fingers of frost"] = [](PlayerbotAI* ai) { return new FingersOfFrostTrigger(ai); };
                creators["mirror image"] = [](PlayerbotAI* ai) { return new MirrorImageTrigger(ai); };                
                creators["no improved scorch"] = [](PlayerbotAI* ai) { return new NoImprovedScorchDebuffTrigger(ai); };
                creators["no fire vulnerability"] = [](PlayerbotAI* ai) { return new NoFireVulnerabilityTrigger(ai); };
                creators["living bomb"] = [](PlayerbotAI* ai) { return new LivingBombTrigger(ai); };
                creators["no food"] = [](PlayerbotAI* ai) { return new NoFoodTrigger(ai); };
                creators["no drink"] = [](PlayerbotAI* ai) { return new NoDrinkTrigger(ai); };
                creators["no mana gem"] = [](PlayerbotAI* ai) { return new NoManaGemTrigger(ai); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["arcane power"] = [](PlayerbotAI* ai) { return new CastArcanePowerAction(ai); };
                creators["presence of mind"] = [](PlayerbotAI* ai) { return new CastPresenceOfMindAction(ai); };
                creators["frostbolt"] = [](PlayerbotAI* ai) { return new CastFrostboltAction(ai); };
                creators["blizzard"] = [](PlayerbotAI* ai) { return new CastBlizzardAction(ai); };
                creators["frost nova"] = [](PlayerbotAI* ai) { return new CastFrostNovaAction(ai); };
                creators["arcane intellect"] = [](PlayerbotAI* ai) { return new CastArcaneIntellectAction(ai); };
                creators["arcane intellect on party"] = [](PlayerbotAI* ai) { return new CastArcaneIntellectOnPartyAction(ai); };
                creators["arcane brilliance on party"] = [](PlayerbotAI* ai) { return new CastArcaneBrillianceOnPartyAction(ai); };
                creators["conjure water"] = [](PlayerbotAI* ai) { return new CastConjureWaterAction(ai); };
                creators["conjure food"] = [](PlayerbotAI* ai) { return new CastConjureFoodAction(ai); };
                creators["conjure mana gem"] = [](PlayerbotAI* ai) { return new CastConjureManaGemAction(ai); };
                creators["mana gem"] = [](PlayerbotAI* ai) { return new UseManaGemAction(ai); };
                creators["molten armor"] = [](PlayerbotAI* ai) { return new CastMoltenArmorAction(ai); };
                creators["mage armor"] = [](PlayerbotAI* ai) { return new CastMageArmorAction(ai); };
                creators["ice armor"] = [](PlayerbotAI* ai) { return new CastIceArmorAction(ai); };
                creators["frost armor"] = [](PlayerbotAI* ai) { return new CastFrostArmorAction(ai); };
                creators["fireball"] = [](PlayerbotAI* ai) { return new CastFireballAction(ai); };
                creators["pyroblast"] = [](PlayerbotAI* ai) { return new CastPyroblastAction(ai); };
                creators["flamestrike"] = [](PlayerbotAI* ai) { return new CastFlamestrikeAction(ai); };
                creators["fire blast"] = [](PlayerbotAI* ai) { return new CastFireBlastAction(ai); };
                creators["scorch"] = [](PlayerbotAI* ai) { return new CastScorchAction(ai); };
                creators["counterspell"] = [](PlayerbotAI* ai) { return new CastCounterspellAction(ai); };
                creators["remove curse"] = [](PlayerbotAI* ai) { return new CastRemoveCurseAction(ai); };
                creators["remove curse on party"] = [](PlayerbotAI* ai) { return new CastRemoveCurseOnPartyAction(ai); };
                creators["remove lesser curse"] = [](PlayerbotAI* ai) { return new CastRemoveLesserCurseAction(ai); };
                creators["remove lesser curse on party"] = [](PlayerbotAI* ai) { return new CastRemoveLesserCurseOnPartyAction(ai); };
                creators["icy veins"] = [](PlayerbotAI* ai) { return new CastIcyVeinsAction(ai); };
                creators["combustion"] = [](PlayerbotAI* ai) { return new CastCombustionAction(ai); };
                creators["ice block"] = [](PlayerbotAI* ai) { return new CastIceBlockAction(ai); };
                creators["polymorph"] = [](PlayerbotAI* ai) { return new CastPolymorphAction(ai); };
                creators["spellsteal"] = [](PlayerbotAI* ai) { return new CastSpellstealAction(ai); };
                creators["invisibility"] = [](PlayerbotAI* ai) { return new CastInvisibilityAction(ai); };
                creators["lesser invisibility"] = [](PlayerbotAI* ai) { return new CastLesserInvisibilityAction(ai); };
                creators["evocation"] = [](PlayerbotAI* ai) { return new CastEvocationAction(ai); };
                creators["arcane blast"] = [](PlayerbotAI* ai) { return new CastArcaneBlastAction(ai); };
                creators["arcane barrage"] = [](PlayerbotAI* ai) { return new CastArcaneBarrageAction(ai); };
                creators["arcane missiles"] = [](PlayerbotAI* ai) { return new CastArcaneMissilesAction(ai); };
                creators["counterspell on enemy healer"] = [](PlayerbotAI* ai) { return new CastCounterspellOnEnemyHealerAction(ai); };
                creators["fire ward"] = [](PlayerbotAI* ai) { return new CastFireWardAction(ai); };
                creators["frost ward"] = [](PlayerbotAI* ai) { return new CastFrostWardAction(ai); };
                creators["blink"] = [](PlayerbotAI* ai) { return new CastBlinkAction(ai); };
                creators["ice barrier"] = [](PlayerbotAI* ai) { return new CastIceBarrierAction(ai); };
                creators["mana shield"] = [](PlayerbotAI* ai) { return new CastManaShieldAction(ai); };
                creators["arcane explosion"] = [](PlayerbotAI* ai) { return new CastArcaneExplosionAction(ai); };
                creators["cone of cold"] = [](PlayerbotAI* ai) { return new CastConeOfColdAction(ai); };
                creators["summon water elemental"] = [](PlayerbotAI* ai) { return new CastSummonWaterElementalAction(ai); };
                creators["ice lance"] = [](PlayerbotAI* ai) { return new CastIceLanceAction(ai); };
                creators["cold snap"] = [](PlayerbotAI* ai) { return new CastColdSnapAction(ai); };
                creators["dragon's breath"] = [](PlayerbotAI* ai) { return new CastDragonsBreathAction(ai); };
                creators["blast wave"] = [](PlayerbotAI* ai) { return new CastBlastWaveAction(ai); };
                creators["living bomb"] = [](PlayerbotAI* ai) { return new CastLivingBombAction(ai); };
                creators["frostfire bolt"] = [](PlayerbotAI* ai) { return new CastFrostfireBoltAction(ai); };
                creators["deep freeze"] = [](PlayerbotAI* ai) { return new DeepFreezeAction(ai); };
                creators["mirror image"] = [](PlayerbotAI* ai) { return new MirrorImageAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateMagePveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateMagePvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateMageRaidStrategiesAction(ai); };
            }
        };
    };
};

MageAiObjectContext::MageAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::mage::StrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::mage::AiObjectContextInternal());
    triggerContexts.Add(new ai::mage::TriggerFactoryInternal());
}
