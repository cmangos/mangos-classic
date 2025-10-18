#pragma once

#include "playerbot/strategy/actions/GenericActions.h"
#include "playerbot/strategy/actions/MovementActions.h"

namespace ai
{
	class CastBloodPresenceAction : public CastBuffSpellAction {
	public:
		CastBloodPresenceAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blood presence") {}
	};

	class CastFrostPresenceAction : public CastBuffSpellAction {
	public:
		CastFrostPresenceAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "frost presence") {}
	};

	class CastUnholyPresenceAction : public CastBuffSpellAction {
	public:
		CastUnholyPresenceAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "unholy presence") {}
	};

	class CastDeathchillAction : public CastBuffSpellAction {
	public:
		CastDeathchillAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "deathchill") {}
		virtual NextAction** getPrerequisites() {
			return NextAction::merge(NextAction::array(0, new NextAction("frost presence"), NULL), CastSpellAction::getPrerequisites());
		}
	};

	class CastDarkCommandAction : public CastBuffSpellAction {
	public:
		CastDarkCommandAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "dark command") {}
		virtual NextAction** getPrerequisites() {
			return NextAction::merge(NextAction::array(0, new NextAction("blood presence"), NULL), CastSpellAction::getPrerequisites());
		}
	};

	BEGIN_RANGED_SPELL_ACTION(CastDeathGripAction, "death grip")
		END_SPELL_ACTION()

	// Unholy presence
	class CastUnholyMeleeSpellAction : public CastMeleeSpellAction {
	public:
		CastUnholyMeleeSpellAction(PlayerbotAI* ai, std::string spell) : CastMeleeSpellAction(ai, spell) {}
		virtual NextAction** getPrerequisites() {
			return NextAction::merge(NextAction::array(0, new NextAction("unholy presence"), NULL), CastMeleeSpellAction::getPrerequisites());
		}
	};


	// Frost presence
	class CastFrostMeleeSpellAction : public CastMeleeSpellAction {
	public:
		CastFrostMeleeSpellAction(PlayerbotAI* ai, std::string spell) : CastMeleeSpellAction(ai, spell) {}
		virtual NextAction** getPrerequisites() {
			return NextAction::merge(NextAction::array(0, new NextAction("frost presence"), NULL), CastMeleeSpellAction::getPrerequisites());
		}
	};

	// Blood presence
	class CastBloodMeleeSpellAction : public CastMeleeSpellAction {
	public:
		CastBloodMeleeSpellAction(PlayerbotAI* ai, std::string spell) : CastMeleeSpellAction(ai, spell) {}
		virtual NextAction** getPrerequisites() {
			return NextAction::merge(NextAction::array(0, new NextAction("blood presence"), NULL), CastMeleeSpellAction::getPrerequisites());
		}
	};
		// a
	class CastRuneStrikeAction : public CastMeleeSpellAction {
	public:
	CastRuneStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "rune strike") {}
	};
	//debuff
	BEGIN_MELEE_DEBUFF_ACTION(CastPestilenceAction, "pestilence")
	END_SPELL_ACTION()

	//debuff
	BEGIN_RANGED_DEBUFF_ACTION(CastHowlingBlastAction, "howling blast")
	END_SPELL_ACTION()

	//debuff it
	BEGIN_RANGED_DEBUFF_ACTION(CastIcyTouchAction, "icy touch")
	END_SPELL_ACTION()


	class CastIcyTouchOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
	{
	public:
		CastIcyTouchOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "icy touch") {}
	};

	//debuff ps
	BEGIN_MELEE_DEBUFF_ACTION(CastPlagueStrikeAction, "plague strike")
	END_SPELL_ACTION()


	class CastPlagueStrikeOnAttackerAction : public CastMeleeDebuffSpellOnAttackerAction
	{
	public:
		CastPlagueStrikeOnAttackerAction(PlayerbotAI* ai) : CastMeleeDebuffSpellOnAttackerAction(ai, "plague strike") {}
	};

	//debuff
	BEGIN_RANGED_DEBUFF_ACTION(CastMarkOfBloodAction, "mark of blood")
	END_SPELL_ACTION()

	class CastMarkOfBloodOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
	{
	public:
		CastMarkOfBloodOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "mark of blood") {}
	};

	class CastUnholyBlightAction : public CastBuffSpellAction
	{
	public:
		CastUnholyBlightAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "unholy blight") {}
	};

	class CastSummonGargoyleAction : public CastBuffSpellAction
	{
	public:
		CastSummonGargoyleAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon gargoyle") {}
	};

	class CastGhoulFrenzyAction : public CastBuffSpellAction
	{
	public:
		CastGhoulFrenzyAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "ghoul frenzy") {}
	};

	BEGIN_MELEE_SPELL_ACTION(CastCorpseExplosionAction, "corpse explosion")
	END_SPELL_ACTION()

	BEGIN_MELEE_SPELL_ACTION(CastAntiMagicShellAction, "anti magic shell")
	END_SPELL_ACTION()


	BEGIN_MELEE_SPELL_ACTION(CastAntiMagicZoneAction, "anti magic zone")
	END_SPELL_ACTION()


	class CastChainsOfIceAction : public CastSpellAction {
	public:
		CastChainsOfIceAction(PlayerbotAI* ai) : CastSpellAction(ai, "chains of ice") {}
	};

	class CastHungeringColdAction : public CastMeleeSpellAction {
	public:
		CastHungeringColdAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "hungering cold") {}
	};

	class CastHeartStrikeAction : public CastMeleeSpellAction {
	public:
		CastHeartStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "heart strike") {}
	};

	class CastBloodStrikeAction : public CastMeleeSpellAction {
	public:
		CastBloodStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "blood strike") {}
	};
	
	class CastFrostStrikeAction : public CastMeleeSpellAction {
	public:
		CastFrostStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "frost strike") {}
	};

	class CastObliterateAction : public CastMeleeSpellAction {
	public:
		CastObliterateAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "obliterate") {}
	};

	class CastDeathStrikeAction : public CastMeleeSpellAction {
	public:
		CastDeathStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "death strike") {}
	};

	class CastScourgeStrikeAction : public CastMeleeSpellAction {
	public:
		CastScourgeStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "scorgue strike") {}
	};

	class CastDeathCoilAction : public CastSpellAction {
	public:
		CastDeathCoilAction(PlayerbotAI* ai) : CastSpellAction(ai, "death coill") {}
	};

	class CastBloodBoilAction : public CastBuffSpellAction {
	public:
		CastBloodBoilAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blood boil") {}
	};

	class CastDeathAndDecayAction : public CastSpellAction {
	public:
		CastDeathAndDecayAction(PlayerbotAI* ai) : CastSpellAction(ai, "death and decay") {}
	};

	class CastHornOfWinterAction : public CastBuffSpellAction
    {
	public:
		CastHornOfWinterAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "horn of winter") {}
	};

	class CastImprovedIcyTalonsAction : public CastBuffSpellAction
	{
	public:
		CastImprovedIcyTalonsAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "improved icy talons") {}
	};

	class CastBoneShieldAction : public CastBuffSpellAction
	{
	public:
		CastBoneShieldAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "bone shield") {}
	};

	class CastDeathPactAction : public CastBuffSpellAction
	{
	public:
		CastDeathPactAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "death pact") {}
	};

	class CastDeathRuneMasteryAction : public CastBuffSpellAction
	{
	public:
		CastDeathRuneMasteryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "death rune mastery") {}
	};

	class CastDancingWeaponAction : public CastBuffSpellAction
	{
	public:
		CastDancingWeaponAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "dancing weapon") {}
	};

	class CastEmpowerRuneWeaponAction : public CastBuffSpellAction
	{
	public:
		CastEmpowerRuneWeaponAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "empower rune weapon") {}
	};

	class CastArmyOfTheDeadAction : public CastBuffSpellAction
	{
	public:
		CastArmyOfTheDeadAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "army of the dead") {}
	};


	class CastRaiseDeadAction : public CastBuffSpellAction
	{
	public:
		CastRaiseDeadAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "raise dead") {}
		virtual bool isPossible() override;
	};

	class CastKillingMachineAction : public CastBuffSpellAction
	{
	public:
		CastKillingMachineAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "killing machine") {}
	};

	class CastIceboundFortitudeAction : public CastBuffSpellAction
	{
	public:
		CastIceboundFortitudeAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "icebound fortitude") {}
	};

	class CastUnbreakableArmorAction : public CastBuffSpellAction
	{
	public:
		CastUnbreakableArmorAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "unbreakable armor") {}
	};

	class CastVampiricBloodAction : public CastBuffSpellAction
	{
	public:
		CastVampiricBloodAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "vampiric blood") {}
	};
	
	class CastMindFreezeAction : public CastMeleeSpellAction {
	public:
		CastMindFreezeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "mind freeze") {}
	};

	class CastStrangulateAction : public CastMeleeSpellAction {
	public:
		CastStrangulateAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "strangulate") {}
	};

    class CastMindFreezeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
    {
    public:
		CastMindFreezeOnEnemyHealerAction(PlayerbotAI* ai) : CastSpellOnEnemyHealerAction(ai, "mind freeze") {}
    };

	class CastRuneTapAction : public CastMeleeSpellAction {
	public:
		CastRuneTapAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "rune tap") {}

	};
	class CastBloodTapAction : public CastMeleeSpellAction {
	public:
		CastBloodTapAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "blood tap") {}
	};

	const std::vector<uint32> RUNEFORGES = { 190557, 191746, 191747, 191748, 191757, 191758 };

	class RuneforgeAction : public MovementAction
	{
	public:
		RuneforgeAction(PlayerbotAI* ai) : MovementAction(ai, "runeforge") {}

		virtual bool Execute(Event& event);
		virtual bool isUseful();
	};
}
