
#include "BloodDKStrategy.h"
#include "DKActions.h"
#include "DKAiObjectContext.h"
#include "DKReactionStrategy.h"
#include "DKTriggers.h"
#include "FrostDKStrategy.h"
#include "GenericDKNonCombatStrategy.h"
#include "playerbot/playerbot.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/Strategy.h"
#include "UnholyDKStrategy.h"
#include "playerbot/strategy/triggers/ChatCommandTrigger.h"


using namespace ai;


namespace ai
{
    namespace DK
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = [](PlayerbotAI* ai) { return new GenericDKNonCombatStrategy(ai); };
				creators["react"] = [](PlayerbotAI* ai) { return new DKReactionStrategy(ai); };
                creators["pull"] = [](PlayerbotAI* ai) { return new PullStrategy(ai, "icy touch"); };
				creators["frost aoe"] = [](PlayerbotAI* ai) { return new FrostDKAoeStrategy(ai); };
				creators["unholy aoe"] = [](PlayerbotAI* ai) { return new UnholyDKAoeStrategy(ai); };
            }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
			CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["tank"] = [](PlayerbotAI* ai) { return new BloodDKStrategy(ai); };
				creators["blood"] = [](PlayerbotAI* ai) { return new BloodDKStrategy(ai); };
                creators["frost"] = [](PlayerbotAI* ai) { return new FrostDKStrategy(ai); };
				creators["unholy"] = [](PlayerbotAI* ai) { return new UnholyDKStrategy(ai); };
            }
        };

        class DKBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            DKBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bdps"] = [](PlayerbotAI* ai) { return new DKBuffDpsStrategy(ai); };
            }
        };
    };
};


namespace ai
{
    namespace DK
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()  
            {
                creators["bone shield"] = [](PlayerbotAI* ai) { return new BoneShieldTrigger(ai); };
                creators["pestilence"] = [](PlayerbotAI* ai) { return new PestilenceTrigger(ai); };
                creators["blood strike"] = [](PlayerbotAI* ai) { return new BloodStrikeTrigger(ai); };
				creators["plague strike"] = [](PlayerbotAI* ai) { return new PlagueStrikeDebuffTrigger(ai); };
				creators["plague strike on attacker"] = [](PlayerbotAI* ai) { return new PlagueStrikeDebuffOnAttackerTrigger(ai); };
                creators["icy touch"] = [](PlayerbotAI* ai) { return new IcyTouchDebuffTrigger(ai); };
				creators["death coil"] = [](PlayerbotAI* ai) { return new DeathCoilTrigger(ai); };
				creators["icy touch on attacker"] = [](PlayerbotAI* ai) { return new IcyTouchDebuffOnAttackerTrigger(ai); };
                creators["improved icy talons"] = [](PlayerbotAI* ai) { return new ImprovedIcyTalonsTrigger(ai); };
                creators["horn of winter"] = [](PlayerbotAI* ai) { return new HornOfWinterTrigger(ai); };
                creators["mind freeze"] = [](PlayerbotAI* ai) { return new MindFreezeInterruptSpellTrigger(ai); };
                creators["mind freeze on enemy healer"] = [](PlayerbotAI* ai) { return new MindFreezeOnEnemyHealerTrigger(ai); };
				creators["strangulate"] = [](PlayerbotAI* ai) { return new StrangulateInterruptSpellTrigger(ai); };
				creators["strangulate on enemy healer"] = [](PlayerbotAI* ai) { return new StrangulateOnEnemyHealerTrigger(ai); };
				creators["blood tap"] = [](PlayerbotAI* ai) { return new BloodTapTrigger(ai); };
				creators["raise dead"] = [](PlayerbotAI* ai) { return new RaiseDeadTrigger(ai); };
				creators["chains of ice"] = [](PlayerbotAI* ai) { return new ChainsOfIceSnareTrigger(ai); };
				creators["auto runeforge"] = [](PlayerbotAI* ai) { return new AutoRuneForgeTrigger(ai); };
				creators["runeforge"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "runeforge"); };
            }
        };
    };
};


namespace ai
{
    namespace DK
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                
				// Unholy
				creators["bone shield"] = [](PlayerbotAI* ai) { return new CastBoneShieldAction(ai); };
				creators["plague strike"] = [](PlayerbotAI* ai) { return new CastPlagueStrikeAction(ai); };
				creators["plague strike on attacker"] = [](PlayerbotAI* ai) { return new CastPlagueStrikeOnAttackerAction(ai); };
				creators["death grip"] = [](PlayerbotAI* ai) { return new CastDeathGripAction(ai); };
				creators["death coil"] = [](PlayerbotAI* ai) { return new CastDeathCoilAction(ai); };
				creators["death strike"] = [](PlayerbotAI* ai) { return new CastDeathStrikeAction(ai); };
				creators["unholy blight"] = [](PlayerbotAI* ai) { return new CastUnholyBlightAction(ai); };
				creators["scourge strike"] = [](PlayerbotAI* ai) { return new CastScourgeStrikeAction(ai); };
				creators["death and decay"] = [](PlayerbotAI* ai) { return new CastDeathAndDecayAction(ai); };
				creators["unholy pressence"] = [](PlayerbotAI* ai) { return new CastUnholyPresenceAction(ai); };
				creators["raise dead"] = [](PlayerbotAI* ai) { return new CastRaiseDeadAction(ai); };
				creators["army of the dead"] = [](PlayerbotAI* ai) { return new CastArmyOfTheDeadAction(ai); };
				creators["summon gargoyle"] = [](PlayerbotAI* ai) { return new CastSummonGargoyleAction(ai); };
				creators["anti magic shell"] = [](PlayerbotAI* ai) { return new CastAntiMagicShellAction(ai); };
				creators["anti magic zone"] = [](PlayerbotAI* ai) { return new CastAntiMagicZoneAction(ai); };
				creators["ghoul frenzy"] = [](PlayerbotAI* ai) { return new CastGhoulFrenzyAction(ai); };
				creators["corpse explosion"] = [](PlayerbotAI* ai) { return new CastCorpseExplosionAction(ai); };
				// Frost
				creators["icy touch"] = [](PlayerbotAI* ai) { return new CastIcyTouchAction(ai); };
				creators["icy touch on attacker"] = [](PlayerbotAI* ai) { return new CastIcyTouchOnAttackerAction(ai); };
				creators["obliterate"] = [](PlayerbotAI* ai) { return new CastObliterateAction(ai); };
				creators["howling blast"] = [](PlayerbotAI* ai) { return new CastHowlingBlastAction(ai); };
				creators["frost strike"] = [](PlayerbotAI* ai) { return new CastFrostStrikeAction(ai); };
				creators["chains of ice"] = [](PlayerbotAI* ai) { return new CastChainsOfIceAction(ai); };
				creators["rune strike"] = [](PlayerbotAI* ai) { return new CastRuneStrikeAction(ai); };
				creators["horn of winter"] = [](PlayerbotAI* ai) { return new CastHornOfWinterAction(ai); };
				creators["killing machine"] = [](PlayerbotAI* ai) { return new CastKillingMachineAction(ai); };
				creators["frost presence"] = [](PlayerbotAI* ai) { return new CastFrostPresenceAction(ai); };
				creators["deathchill"] = [](PlayerbotAI* ai) { return new CastDeathchillAction(ai); };
				creators["icebound fortitude"] = [](PlayerbotAI* ai) { return new CastIceboundFortitudeAction(ai); };
				creators["mind freeze"] = [](PlayerbotAI* ai) { return new CastMindFreezeAction(ai); };
				creators["empower rune weapon"] = [](PlayerbotAI* ai) { return new CastEmpowerRuneWeaponAction(ai); };
				creators["hungering cold"] = [](PlayerbotAI* ai) { return new CastHungeringColdAction(ai); };
				creators["unbreakable armor"] = [](PlayerbotAI* ai) { return new CastUnbreakableArmorAction(ai); };
				creators["improved icy talons"] = [](PlayerbotAI* ai) { return new CastImprovedIcyTalonsAction(ai); };
				// blood
				creators["blood strike"] = [](PlayerbotAI* ai) { return new CastBloodStrikeAction(ai); };
				creators["blood tap"] = [](PlayerbotAI* ai) { return new CastBloodTapAction(ai); };
				creators["pestilence"] = [](PlayerbotAI* ai) { return new CastPestilenceAction(ai); };
				creators["strangulate"] = [](PlayerbotAI* ai) { return new CastStrangulateAction(ai); };
				creators["blood boil"] = [](PlayerbotAI* ai) { return new CastBloodBoilAction(ai); };
				creators["heart strike"] = [](PlayerbotAI* ai) { return new CastHeartStrikeAction(ai); };
				creators["mark of_blood"] = [](PlayerbotAI* ai) { return new CastMarkOfBloodAction(ai); };
				creators["blood presence"] = [](PlayerbotAI* ai) { return new CastBloodPresenceAction(ai); };
				creators["rune tap"] = [](PlayerbotAI* ai) { return new CastRuneTapAction(ai); };
				creators["vampiric blood"] = [](PlayerbotAI* ai) { return new CastVampiricBloodAction(ai); };
				creators["death pact"] = [](PlayerbotAI* ai) { return new CastDeathPactAction(ai); };
				creators["death rune_mastery"] = [](PlayerbotAI* ai) { return new CastDeathRuneMasteryAction(ai); };
				creators["dancing weapon"] = [](PlayerbotAI* ai) { return new CastDancingWeaponAction(ai); };
				creators["dark command"] = [](PlayerbotAI* ai) { return new CastDarkCommandAction(ai); };
				creators["mind freeze on enemy healer"] = [](PlayerbotAI* ai) { return new CastMindFreezeOnEnemyHealerAction(ai); };

				creators["runeforge"] = [](PlayerbotAI* ai) { return new RuneforgeAction(ai); };
            }
        };
    };
};

DKAiObjectContext::DKAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::DK::StrategyFactoryInternal());
    strategyContexts.Add(new ai::DK::CombatStrategyFactoryInternal());
    strategyContexts.Add(new ai::DK::DKBuffStrategyFactoryInternal());
    actionContexts.Add(new ai::DK::AiObjectContextInternal());
    triggerContexts.Add(new ai::DK::TriggerFactoryInternal());
}
