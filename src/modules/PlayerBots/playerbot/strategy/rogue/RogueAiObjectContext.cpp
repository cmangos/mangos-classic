
#include "playerbot/playerbot.h"
#include "RogueActions.h"
#include "RogueTriggers.h"
#include "RogueAiObjectContext.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "CombatRogueStrategy.h"
#include "AssassinationRogueStrategy.h"
#include "SubtletyRogueStrategy.h"

namespace ai
{
    namespace rogue
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); };
                creators["aoe"] = [](PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); };
                creators["buff"] = [](PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); };
                creators["boost"] = [](PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new StealthPlaceholderStrategy(ai); };
                creators["stealthed"] = [](PlayerbotAI* ai) { return new RogueStealthedStrategy(ai); };
                creators["cc"] = [](PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); };
            }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe combat pve"] = [](PlayerbotAI* ai) { return new CombatRogueAoePveStrategy(ai); };
                creators["aoe combat pvp"] = [](PlayerbotAI* ai) { return new CombatRogueAoePvpStrategy(ai); };
                creators["aoe combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueAoeRaidStrategy(ai); };
                creators["aoe assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRogueAoePveStrategy(ai); };
                creators["aoe assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRogueAoePvpStrategy(ai); };
                creators["aoe assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueAoeRaidStrategy(ai); };
                creators["aoe subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRogueAoePveStrategy(ai); };
                creators["aoe subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRogueAoePvpStrategy(ai); };
                creators["aoe subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueAoeRaidStrategy(ai); };
            }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff combat pve"] = [](PlayerbotAI* ai) { return new CombatRogueBuffPveStrategy(ai); };
                creators["buff combat pvp"] = [](PlayerbotAI* ai) { return new CombatRogueBuffPvpStrategy(ai); };
                creators["buff combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueBuffRaidStrategy(ai); };
                creators["buff assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRogueBuffPveStrategy(ai); };
                creators["buff assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRogueBuffPvpStrategy(ai); };
                creators["buff assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueBuffRaidStrategy(ai); };
                creators["buff subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRogueBuffPveStrategy(ai); };
                creators["buff subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRogueBuffPvpStrategy(ai); };
                creators["buff subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueBuffRaidStrategy(ai); };
            }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost combat pve"] = [](PlayerbotAI* ai) { return new CombatRogueBoostPveStrategy(ai); };
                creators["boost combat pvp"] = [](PlayerbotAI* ai) { return new CombatRogueBoostPvpStrategy(ai); };
                creators["boost combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueBoostRaidStrategy(ai); };
                creators["boost assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRogueBoostPveStrategy(ai); };
                creators["boost assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRogueBoostPvpStrategy(ai); };
                creators["boost assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueBoostRaidStrategy(ai); };
                creators["boost subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRogueBoostPveStrategy(ai); };
                creators["boost subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRogueBoostPvpStrategy(ai); };
                creators["boost subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueBoostRaidStrategy(ai); };
            }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc combat pve"] = [](PlayerbotAI* ai) { return new CombatRogueCcPveStrategy(ai); };
                creators["cc combat pvp"] = [](PlayerbotAI* ai) { return new CombatRogueCcPvpStrategy(ai); };
                creators["cc combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueCcRaidStrategy(ai); };
                creators["cc assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRogueCcPveStrategy(ai); };
                creators["cc assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRogueCcPvpStrategy(ai); };
                creators["cc assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueCcRaidStrategy(ai); };
                creators["cc subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRogueCcPveStrategy(ai); };
                creators["cc subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRogueCcPvpStrategy(ai); };
                creators["cc subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueCcRaidStrategy(ai); };
            }
        };

        class StealthSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StealthSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["stealth combat pve"] = [](PlayerbotAI* ai) { return new CombatRogueStealthPveStrategy(ai); };
                creators["stealth combat pvp"] = [](PlayerbotAI* ai) { return new CombatRogueStealthPvpStrategy(ai); };
                creators["stealth combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueStealthRaidStrategy(ai); };
                creators["stealth assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRogueStealthPveStrategy(ai); };
                creators["stealth assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRogueStealthPvpStrategy(ai); };
                creators["stealth assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueStealthRaidStrategy(ai); };
                creators["stealth subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRogueStealthPveStrategy(ai); };
                creators["stealth subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRogueStealthPvpStrategy(ai); };
                creators["stealth subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueStealthRaidStrategy(ai); };
            }
        };

        class PoisonsSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PoisonsSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["poisons combat pve"] = [](PlayerbotAI* ai) { return new CombatRoguePoisonsPveStrategy(ai); };
                creators["poisons combat pvp"] = [](PlayerbotAI* ai) { return new CombatRoguePoisonsPvpStrategy(ai); };
                creators["poisons combat raid"] = [](PlayerbotAI* ai) { return new CombatRoguePoisonsRaidStrategy(ai); };
                creators["poisons assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRoguePoisonsPveStrategy(ai); };
                creators["poisons assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRoguePoisonsPvpStrategy(ai); };
                creators["poisons assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRoguePoisonsRaidStrategy(ai); };
                creators["poisons subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRoguePoisonsPveStrategy(ai); };
                creators["poisons subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRoguePoisonsPvpStrategy(ai); };
                creators["poisons subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRoguePoisonsRaidStrategy(ai); };
            }
        };

        class PoisonMainStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PoisonMainStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["poisons"] = [](PlayerbotAI* ai) { return new RoguePoisonsPlaceholderStrategy(ai); };
                creators["poison main deadly"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main deadly", "apply deadly poison main hand", "apply deadly poison main hand"); };
                creators["poison main crippling"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main crippling", "apply crippling poison main hand", "apply crippling poison main hand"); };
                creators["poison main mind"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main mind", "apply mind poison main hand", "apply mind poison main hand"); };
                creators["poison main instant"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main instant", "apply instant poison main hand", "apply instant poison main hand"); };
                creators["poison main wound"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main wound", "apply wound poison main hand", "apply wound poison main hand"); };
                creators["poison main anesthetic"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison main anesthetic", "apply anesthetic poison main hand", "apply anesthetic poison main hand"); };
            }
        };

        class PoisonOffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PoisonOffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["poisons"] = [](PlayerbotAI* ai) { return new RoguePoisonsPlaceholderStrategy(ai); };
                creators["poison off deadly"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off deadly", "apply deadly poison off hand", "apply deadly poison off hand"); };
                creators["poison off crippling"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off crippling", "apply crippling poison off hand", "apply crippling poison off hand"); };
                creators["poison off mind"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off mind", "apply mind poison off hand", "apply mind poison off hand"); };
                creators["poison off instant"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off instant", "apply instant poison off hand", "apply instant poison off hand"); };
                creators["poison off wound"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off wound", "apply wound poison off hand", "apply wound poison off hand"); };
                creators["poison off anesthetic"] = [](PlayerbotAI* ai) { return new RogueManualPoisonStrategy(ai, "poison off anesthetic", "apply anesthetic poison off hand", "apply anesthetic poison off hand"); };
            }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["combat"] = [](PlayerbotAI* ai) { return new CombatRoguePlaceholderStrategy(ai); };
                creators["subtlety"] = [](PlayerbotAI* ai) { return new SubtletyRoguePlaceholderStrategy(ai); };
                creators["assassination"] = [](PlayerbotAI* ai) { return new AssassinationRoguePlaceholderStrategy(ai); };
            }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["combat pvp"] = [](PlayerbotAI* ai) { return new CombatRoguePvpStrategy(ai); };
                creators["combat pve"] = [](PlayerbotAI* ai) { return new CombatRoguePveStrategy(ai); };
                creators["combat raid"] = [](PlayerbotAI* ai) { return new CombatRogueRaidStrategy(ai); };
                creators["assassination pvp"] = [](PlayerbotAI* ai) { return new AssassinationRoguePvpStrategy(ai); };
                creators["assassination pve"] = [](PlayerbotAI* ai) { return new AssassinationRoguePveStrategy(ai); };
                creators["assassination raid"] = [](PlayerbotAI* ai) { return new AssassinationRogueRaidStrategy(ai); };
                creators["subtlety pvp"] = [](PlayerbotAI* ai) { return new SubtletyRoguePvpStrategy(ai); };
                creators["subtlety pve"] = [](PlayerbotAI* ai) { return new SubtletyRoguePveStrategy(ai); };
                creators["subtlety raid"] = [](PlayerbotAI* ai) { return new SubtletyRogueRaidStrategy(ai); };
            }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["kick"] = [](PlayerbotAI* ai) { return new KickInterruptSpellTrigger(ai); };
                creators["rupture"] = [](PlayerbotAI* ai) { return new RuptureTrigger(ai); };
                creators["slice and dice"] = [](PlayerbotAI* ai) { return new SliceAndDiceTrigger(ai); };
                creators["expose armor"] = [](PlayerbotAI* ai) { return new ExposeArmorTrigger(ai); };
                creators["kick on enemy healer"] = [](PlayerbotAI* ai) { return new KickInterruptEnemyHealerSpellTrigger(ai); };
                creators["adrenaline rush"] = [](PlayerbotAI* ai) { return new RogueBoostBuffTrigger(ai, "adrenaline rush"); };
                creators["unstealth"] = [](PlayerbotAI* ai) { return new RogueUnstealthTrigger(ai); };
                creators["sap"] = [](PlayerbotAI* ai) { return new SapTrigger(ai); };
                creators["in stealth"] = [](PlayerbotAI* ai) { return new InStealthTrigger(ai); };
                creators["no stealth"] = [](PlayerbotAI* ai) { return new NoStealthTrigger(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new StealthTrigger(ai); };
                creators["sprint"] = [](PlayerbotAI* ai) { return new SprintTrigger(ai); };
                creators["2 combo"] = [](PlayerbotAI* ai) { return new ComboPointsAvailableTrigger(ai, 2); };
                creators["3 combo"] = [](PlayerbotAI* ai) { return new ComboPointsAvailableTrigger(ai, 3); };
                creators["4 combo"] = [](PlayerbotAI* ai) { return new ComboPointsAvailableTrigger(ai, 4); };
                creators["sinister strike"] = [](PlayerbotAI* ai) { return new SpellCanBeCastedTrigger(ai, "sinister strike"); };
                creators["hemorrhage"] = [](PlayerbotAI* ai) { return new SpellCanBeCastedTrigger(ai, "hemorrhage"); };
                creators["killing spree"] = [](PlayerbotAI* ai) { return new RogueBoostBuffTrigger(ai, "killing spree"); };
                creators["eviscerate"] = [](PlayerbotAI* ai) { return new EviscerateTrigger(ai); };
                creators["blade flurry"] = [](PlayerbotAI* ai) { return new RogueBoostBuffTrigger(ai, "blade flurry"); };
                //creators["tricks of the trade on tank"] = [](PlayerbotAI* ai) { return new TricksOfTheTradeOnTankTrigger(ai); };
                creators["cloak of shadows"] = [](PlayerbotAI* ai) { return new CloakOfShadowsTrigger(ai); };
                creators["fan of knives"] = [](PlayerbotAI* ai) { return new SpellCanBeCastedTrigger(ai, "fan of knives"); };
                creators["riposte"] = [](PlayerbotAI* ai) { return new RiposteCastTrigger(ai); };
                creators["apply deadly poison main hand"] = [](PlayerbotAI* ai) { return new ApplyDeadlyPoisonTrigger(ai, true); };
                creators["apply crippling poison main hand"] = [](PlayerbotAI* ai) { return new ApplyCripplingPoisonTrigger(ai, true); };
                creators["apply mind poison main hand"] = [](PlayerbotAI* ai) { return new ApplyMindPoisonTrigger(ai, true); };
                creators["apply instant poison main hand"] = [](PlayerbotAI* ai) { return new ApplyInstantPoisonTrigger(ai, true); };
                creators["apply wound poison main hand"] = [](PlayerbotAI* ai) { return new ApplyWoundPoisonTrigger(ai, true); };
                creators["apply anesthetic poison main hand"] = [](PlayerbotAI* ai) { return new ApplyAnestheticPoisonTrigger(ai, true); };
                creators["apply deadly poison off hand"] = [](PlayerbotAI* ai) { return new ApplyDeadlyPoisonTrigger(ai, false); };
                creators["apply crippling poison off hand"] = [](PlayerbotAI* ai) { return new ApplyCripplingPoisonTrigger(ai, false); };
                creators["apply mind poison off hand"] = [](PlayerbotAI* ai) { return new ApplyMindPoisonTrigger(ai, false); };
                creators["apply instant poison off hand"] = [](PlayerbotAI* ai) { return new ApplyInstantPoisonTrigger(ai, false); };
                creators["apply wound poison off hand"] = [](PlayerbotAI* ai) { return new ApplyWoundPoisonTrigger(ai, false); };
                creators["apply anesthetic poison off hand"] = [](PlayerbotAI* ai) { return new ApplyAnestheticPoisonTrigger(ai, false); };
            }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["riposte"] = [](PlayerbotAI* ai) { return new CastRiposteAction(ai); };
                creators["mutilate"] = [](PlayerbotAI* ai) { return new CastMutilateAction(ai); };
                creators["sinister strike"] = [](PlayerbotAI* ai) { return new CastSinisterStrikeAction(ai); };
                creators["hemorrhage"] = [](PlayerbotAI* ai) { return new CastHemorrhageAction(ai); };
                creators["ghostly strike"] = [](PlayerbotAI* ai) { return new CastGhostlyStrikeAction(ai); };
                creators["gouge"] = [](PlayerbotAI* ai) { return new CastGougeAction(ai); };
                creators["kidney shot"] = [](PlayerbotAI* ai) { return new CastKidneyShotAction(ai); };
                creators["rupture"] = [](PlayerbotAI* ai) { return new CastRuptureAction(ai); };
                creators["slice and dice"] = [](PlayerbotAI* ai) { return new CastSliceAndDiceAction(ai); };
                creators["eviscerate"] = [](PlayerbotAI* ai) { return new CastEviscerateAction(ai); };
                creators["vanish"] = [](PlayerbotAI* ai) { return new CastVanishAction(ai); };
                creators["evasion"] = [](PlayerbotAI* ai) { return new CastEvasionAction(ai); };
                creators["kick"] = [](PlayerbotAI* ai) { return new CastKickAction(ai); };
                creators["feint"] = [](PlayerbotAI* ai) { return new CastFeintAction(ai); };
                creators["backstab"] = [](PlayerbotAI* ai) { return new CastBackstabAction(ai); };
                creators["expose armor"] = [](PlayerbotAI* ai) { return new CastExposeArmorAction(ai); };
                creators["kick on enemy healer"] = [](PlayerbotAI* ai) { return new CastKickOnEnemyHealerAction(ai); };
                creators["blade flurry"] = [](PlayerbotAI* ai) { return new CastBladeFlurryAction(ai); };
                creators["adrenaline rush"] = [](PlayerbotAI* ai) { return new CastAdrenalineRushAction(ai); };
                creators["ambush"] = [](PlayerbotAI* ai) { return new CastAmbushAction(ai); };
                creators["stealth"] = [](PlayerbotAI* ai) { return new CastStealthAction(ai); };
                creators["sprint"] = [](PlayerbotAI* ai) { return new CastSprintAction(ai); };
                creators["garrote"] = [](PlayerbotAI* ai) { return new CastGarroteAction(ai); };
                creators["cheap shot"] = [](PlayerbotAI* ai) { return new CastCheapShotAction(ai); };
                creators["blind"] = [](PlayerbotAI* ai) { return new CastBlindAction(ai); };
                creators["unstealth"] = [](PlayerbotAI* ai) { return new RogueUnstealthAction(ai); };
                creators["sap"] = [](PlayerbotAI* ai) { return new CastSapAction(ai); };
                creators["check stealth"] = [](PlayerbotAI* ai) { return new CheckStealthAction(ai); };
                creators["killing spree"] = [](PlayerbotAI* ai) { return new CastKillingSpreeAction(ai); };
                creators["tricks of the trade"] = [](PlayerbotAI* ai) { return new CastTricksOfTheTradeOnPartyAction(ai); };
                creators["cloak of shadows"] = [](PlayerbotAI* ai) { return new CastCloakOfShadowsAction(ai); };
                //creators["fan of knives"] = [](PlayerbotAI* ai) { return new CastFanOfKnivesAction(ai); };
                creators["cold blood"] = [](PlayerbotAI* ai) { return new CastColdBloodAction(ai); };
                creators["preparation"] = [](PlayerbotAI* ai) { return new CastPreparationAction(ai); };
                creators["premeditation"] = [](PlayerbotAI* ai) { return new CastPremeditationAction(ai); };
                creators["shadowstep"] = [](PlayerbotAI* ai) { return new CastShadowstepAction(ai); };
                creators["update pve strats"] = [](PlayerbotAI* ai) { return new UpdateRoguePveStrategiesAction(ai); };
                creators["update pvp strats"] = [](PlayerbotAI* ai) { return new UpdateRoguePvpStrategiesAction(ai); };
                creators["update raid strats"] = [](PlayerbotAI* ai) { return new UpdateRogueRaidStrategiesAction(ai); };
                creators["apply deadly poison main hand"] = [](PlayerbotAI* ai) { return new ApplyDeadlyPoisonAction(ai, true); };
                creators["apply crippling poison main hand"] = [](PlayerbotAI* ai) { return new ApplyCripplingPoisonAction(ai, true); };
                creators["apply mind poison main hand"] = [](PlayerbotAI* ai) { return new ApplyMindPoisonAction(ai, true); };
                creators["apply instant poison main hand"] = [](PlayerbotAI* ai) { return new ApplyInstantPoisonAction(ai, true); };
                creators["apply wound poison main hand"] = [](PlayerbotAI* ai) { return new ApplyWoundPoisonAction(ai, true); };
                creators["apply anesthetic poison main hand"] = [](PlayerbotAI* ai) { return new ApplyAnestheticPoisonAction(ai, true); };
                creators["apply deadly poison off hand"] = [](PlayerbotAI* ai) { return new ApplyDeadlyPoisonAction(ai, false); };
                creators["apply crippling poison off hand"] = [](PlayerbotAI* ai) { return new ApplyCripplingPoisonAction(ai, false); };
                creators["apply mind poison off hand"] = [](PlayerbotAI* ai) { return new ApplyMindPoisonAction(ai, false); };
                creators["apply instant poison off hand"] = [](PlayerbotAI* ai) { return new ApplyInstantPoisonAction(ai, false); };
                creators["apply wound poison off hand"] = [](PlayerbotAI* ai) { return new ApplyWoundPoisonAction(ai, false); };
                creators["apply anesthetic poison off hand"] = [](PlayerbotAI* ai) { return new ApplyAnestheticPoisonAction(ai, false); };
            }
        };
    };
};

RogueAiObjectContext::RogueAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::rogue::StrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::StealthSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::PoisonsSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::PoisonMainStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::PoisonOffStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::rogue::ClassSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::rogue::AiObjectContextInternal());
    triggerContexts.Add(new ai::rogue::TriggerFactoryInternal());
}
