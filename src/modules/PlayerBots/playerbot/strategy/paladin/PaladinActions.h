#pragma once
#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
	// seals
	BUFF_ACTION(CastSealOfRighteousnessAction, "seal of righteousness");
	BUFF_ACTION(CastSealOfJusticeAction, "seal of justice");
	BUFF_ACTION(CastSealOfLightAction, "seal of light");
	BUFF_ACTION(CastSealOfWisdomAction, "seal of wisdom");
	BUFF_ACTION(CastSealOfCommandAction, "seal of command");
	BUFF_ACTION(CastSealOfVengeanceAction, "seal of vengeance");

	class CastJudgementAction : public CastMeleeDebuffSpellAction
	{
	public:
		CastJudgementAction(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, "judgement") { range = 10.0f; }
		virtual bool isUseful() 
		{
			return ai->HasAnyAuraOf(bot, "seal of justice", "seal of command", "seal of vengeance", "seal of blood", "seal of righteousness", "seal of light", "seal of wisdom", NULL);
		}
	};

	// judgements
	MELEE_DEBUFF_ACTION_R(CastJudgementOfLightAction, "judgement of light", 10.0f);
	MELEE_DEBUFF_ACTION_R(CastJudgementOfWisdomAction, "judgement of wisdom", 10.0f);
	MELEE_DEBUFF_ACTION_R(CastJudgementOfJusticeAction, "judgement of justice", 10.0f);

	SPELL_ACTION(CastHolyShockAction, "holy shock");
	HEAL_PARTY_ACTION(CastHolyShockOnPartyAction, "holy shock");

	// consecration
	SPELL_ACTION(CastConsecrationAction, "consecration");

	// repentance
	SNARE_ACTION(CastRepentanceSnareAction, "repentance");
	RANGED_DEBUFF_ACTION(CastRepentanceAction, "repentance");
	ENEMY_HEALER_ACTION(CastRepentanceOnHealerAction, "repentance");

	//hammer of wrath
	SPELL_ACTION(CastHammerOfWrathAction, "hammer of wrath");

	// buffs
	BUFF_ACTION(CastDivineFavorAction, "divine favor");

	// blessings
	BUFF_ACTION(CastBlessingOfFreedomAction, "blessing of freedom");
	// fury
	BUFF_ACTION(CastRighteousFuryAction, "righteous fury");
	BUFF_ACTION(CastAvengingWrathAction, "avenging wrath");

	BUFF_ACTION(CastDivineIlluminationAction, "divine illumination");

	class CastDivineStormAction : public CastBuffSpellAction
	{
	public:
		CastDivineStormAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "divine storm") {}
	};

	class CastCrusaderStrikeAction : public CastMeleeSpellAction
	{
	public:
		CastCrusaderStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "crusader strike") {}
	};

    class CastSealSpellAction : public CastBuffSpellAction
    {
    public:
        CastSealSpellAction(PlayerbotAI* ai, std::string name) : CastBuffSpellAction(ai, name) {}
        virtual bool isUseful() { return AI_VALUE2(bool, "combat", "self target"); }
    };

    // Pick the aura that is not being used by another paladin
	class CastPaladinAuraAction : public CastBuffSpellAction
	{
	public:
		CastPaladinAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "paladin aura") {}
		virtual bool isPossible() { return true; }
		virtual bool isUseful() { return true; }
		virtual bool Execute(Event& event);
	};

    class CastDevotionAuraAction : public CastBuffSpellAction
    {
    public:
        CastDevotionAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "devotion aura") {}
    };

    class CastRetributionAuraAction : public CastBuffSpellAction
    {
    public:
        CastRetributionAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "retribution aura") {}
    };

    class CastConcentrationAuraAction : public CastBuffSpellAction
    {
    public:
        CastConcentrationAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "concentration aura") {}
    };

    class CastShadowResistanceAuraAction : public CastBuffSpellAction
    {
    public:
        CastShadowResistanceAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shadow resistance aura") {}
    };

    class CastFrostResistanceAuraAction : public CastBuffSpellAction
    {
    public:
        CastFrostResistanceAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "frost resistance aura") {}
    };

    class CastFireResistanceAuraAction : public CastBuffSpellAction
    {
    public:
        CastFireResistanceAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "fire resistance aura") {}
    };

    class CastCrusaderAuraAction : public CastBuffSpellAction
    {
    public:
        CastCrusaderAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "crusader aura") {}
    };

    class CastSanctityAuraAction : public CastBuffSpellAction
    {
    public:
        CastSanctityAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "sanctity aura") {}
    };

    // Pick the best blessing based on spec
    class CastBlessingAction : public CastSpellAction
    {
    public:
        CastBlessingAction(PlayerbotAI* ai, std::string name, bool greater) : CastSpellAction(ai, name), greater(greater) {}

    private:
        Unit* GetTarget() override;
        bool isPossible() override;
        bool isUseful() override { return true; }
        virtual std::string GetBlessingForTarget(Unit* target);

    protected:
        virtual std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const = 0;

    private:
        bool greater;
    };

    class CastPveBlessingAction : public CastBlessingAction
    {
    public:
        CastPveBlessingAction(PlayerbotAI* ai, std::string name = "pve blessing", bool greater = false) : CastBlessingAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastPvpBlessingAction : public CastBlessingAction
    {
    public:
        CastPvpBlessingAction(PlayerbotAI* ai, std::string name = "pvp blessing", bool greater = false) : CastBlessingAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastRaidBlessingAction : public CastBlessingAction
    {
    public:
        CastRaidBlessingAction(PlayerbotAI* ai, std::string name = "raid blessing", bool greater = false) : CastBlessingAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastPveGreaterBlessingAction : public CastPveBlessingAction
    {
    public:
        CastPveGreaterBlessingAction(PlayerbotAI* ai) : CastPveBlessingAction(ai, "pve greater blessing", true) {}
    };

    class CastPvpGreaterBlessingAction : public CastPvpBlessingAction
    {
    public:
        CastPvpGreaterBlessingAction(PlayerbotAI* ai) : CastPvpBlessingAction(ai, "pvp greater blessing", true) {}
    };

    class CastRaidGreaterBlessingAction : public CastRaidBlessingAction
    {
    public:
        CastRaidGreaterBlessingAction(PlayerbotAI* ai) : CastRaidBlessingAction(ai, "raid greater blessing", true) {}
    };

    // Pick the best blessing based on target
    class CastBlessingOnPartyAction : public CastSpellAction
    {
    public:
        CastBlessingOnPartyAction(PlayerbotAI* ai, std::string name, bool greater) : CastSpellAction(ai, name), greater(greater) {}

    private:
        Unit* GetTarget() override;
        bool isPossible() override;
        virtual std::string GetBlessingForTarget(Unit* target);

    protected:
        virtual std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const = 0;

    private:
        bool greater;
    };

    class CastPveBlessingOnPartyAction : public CastBlessingOnPartyAction
    {
    public:
        CastPveBlessingOnPartyAction(PlayerbotAI* ai, std::string name = "pve blessing on party", bool greater = false) : CastBlessingOnPartyAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastPvpBlessingOnPartyAction : public CastBlessingOnPartyAction
    {
    public:
        CastPvpBlessingOnPartyAction(PlayerbotAI* ai, std::string name = "pvp blessing on party", bool greater = false) : CastBlessingOnPartyAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastRaidBlessingOnPartyAction : public CastBlessingOnPartyAction
    {
    public:
        CastRaidBlessingOnPartyAction(PlayerbotAI* ai, std::string name = "raid blessing on party", bool greater = false) : CastBlessingOnPartyAction(ai, name, greater) {}

    private:
        std::vector<std::string> GetPossibleBlessingsForTarget(Unit* target) const override;
    };

    class CastPveGreaterBlessingOnPartyAction : public CastPveBlessingOnPartyAction
    {
    public:
        CastPveGreaterBlessingOnPartyAction(PlayerbotAI* ai) : CastPveBlessingOnPartyAction(ai, "pve greater blessing on party", true) {}
    };

    class CastPvpGreaterBlessingOnPartyAction : public CastPvpBlessingOnPartyAction
    {
    public:
        CastPvpGreaterBlessingOnPartyAction(PlayerbotAI* ai) : CastPvpBlessingOnPartyAction(ai, "pvp greater blessing on party", true) {}
    };

    class CastRaidGreaterBlessingOnPartyAction : public CastRaidBlessingOnPartyAction
    {
    public:
        CastRaidGreaterBlessingOnPartyAction(PlayerbotAI* ai) : CastRaidBlessingOnPartyAction(ai, "raid greater blessing on party", true) {}
    };

	class CastBlessingOfMightAction : public CastBuffSpellAction
	{
	public:
		CastBlessingOfMightAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of might") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
	};

	class CastGreaterBlessingOfMightAction : public CastBuffSpellAction
	{
	public:
		CastGreaterBlessingOfMightAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of might") {}
	};

    class CastBlessingOfMightOnPartyAction : public BuffOnPartyAction
    {
    public:
		CastBlessingOfMightOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of might") {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfMightOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
		CastGreaterBlessingOfMightOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of might") {}
    };

	class CastBlessingOfWisdomAction : public CastBuffSpellAction
	{
	public:
		CastBlessingOfWisdomAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of wisdom") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
    };

	class CastGreaterBlessingOfWisdomAction : public CastBuffSpellAction
	{
	public:
		CastGreaterBlessingOfWisdomAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of wisdom") {}
	};

    class CastBlessingOfWisdomOnPartyAction : public BuffOnPartyAction
    {
    public:
        CastBlessingOfWisdomOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of wisdom") {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfWisdomOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
        CastGreaterBlessingOfWisdomOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of wisdom") {}
    };

	class CastBlessingOfKingsAction : public CastBuffSpellAction
	{
	public:
		CastBlessingOfKingsAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of kings") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
	};

	class CastGreaterBlessingOfKingsAction : public CastBuffSpellAction
	{
	public:
		CastGreaterBlessingOfKingsAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of kings") {}
	};

    class CastBlessingOfKingsOnPartyAction : public BuffOnPartyAction
    {
    public:
        CastBlessingOfKingsOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of kings") {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfKingsOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
        CastGreaterBlessingOfKingsOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of kings") {}
    };

	class CastBlessingOfSanctuaryAction : public CastBuffSpellAction
	{
	public:
		CastBlessingOfSanctuaryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of sanctuary") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
	};

	class CastGreaterBlessingOfSanctuaryAction : public CastBuffSpellAction
	{
	public:
		CastGreaterBlessingOfSanctuaryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of sanctuary") {}
	};

    class CastBlessingOfSanctuaryOnPartyAction : public BuffOnPartyAction
    {
    public:
        CastBlessingOfSanctuaryOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of sanctuary") {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfSanctuaryOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
        CastGreaterBlessingOfSanctuaryOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of sanctuary") {}
    };

    class CastBlessingOfLightAction : public CastBuffSpellAction
    {
    public:
        CastBlessingOfLightAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of light") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
    };

    class CastGreaterBlessingOfLightAction : public CastBuffSpellAction
    {
    public:
        CastGreaterBlessingOfLightAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of light") {}
    };

    class CastBlessingOfLightOnPartyAction : public BuffOnPartyAction
    {
    public:
        CastBlessingOfLightOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of light") {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfLightOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
        CastGreaterBlessingOfLightOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of light") {}
    };

    class CastBlessingOfSalvationAction : public CastBuffSpellAction
    {
    public:
        CastBlessingOfSalvationAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blessing of salvation") {}
        bool isUseful() override { return CastBuffSpellAction::isUseful() && !ai->HasAura("greater " + GetSpellName(), GetTarget()); }
    };

    class CastGreaterBlessingOfSalvationAction : public CastBuffSpellAction
    {
    public:
        CastGreaterBlessingOfSalvationAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "greater blessing of salvation") {}
    };

    class CastBlessingOfSalvationOnPartyAction : public BuffOnPartyAction
    {
    public:
        CastBlessingOfSalvationOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of salvation", true) {}
        std::string GetTargetQualifier() override { return GetSpellName() + ",greater " + GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }
    };

    class CastGreaterBlessingOfSalvationOnPartyAction : public GreaterBuffOnPartyAction
    {
    public:
        CastGreaterBlessingOfSalvationOnPartyAction(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, "greater blessing of salvation", true) {}

        bool isUseful() override
        {
            if (GreaterBuffOnPartyAction::isUseful())
            {
                // Don't cast greater salvation on possible tank classes
                Unit* target = GetTarget();
                if (target->IsPlayer())
                {
                    const uint8 playerClass = ((Player*)target)->getClass();
#ifdef MANGOSBOT_TWO
                    if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID || playerClass == CLASS_DEATH_KNIGHT)
#else
                    if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID)
#endif
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }
    };

    class CastHolyLightAction : public CastHealingSpellAction
    {
    public:
        CastHolyLightAction(PlayerbotAI* ai) : CastHealingSpellAction(ai, "holy light") {}
    };

    class CastHolyLightOnPartyAction : public HealPartyMemberAction
    {
    public:
        CastHolyLightOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "holy light") {}
    };

    class CastFlashOfLightAction : public CastHealingSpellAction
    {
    public:
        CastFlashOfLightAction(PlayerbotAI* ai) : CastHealingSpellAction(ai, "flash of light") {}
    };

    class CastFlashOfLightOnPartyAction : public HealPartyMemberAction
    {
    public:
        CastFlashOfLightOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "flash of light") {}
    };

    class CastLayOnHandsAction : public CastHealingSpellAction
    {
    public:
        CastLayOnHandsAction(PlayerbotAI* ai) : CastHealingSpellAction(ai, "lay on hands") {}
    };

    class CastLayOnHandsOnPartyAction : public HealPartyMemberAction
    {
    public:
        CastLayOnHandsOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "lay on hands") {}
    };

	class CastDivineProtectionAction : public CastBuffSpellAction
	{
	public:
		CastDivineProtectionAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "divine protection") {}
	};

    class CastDivineProtectionOnPartyAction : public HealPartyMemberAction
    {
    public:
        CastDivineProtectionOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "divine protection") {}
        virtual std::string getName() { return "divine protection on party"; }
    };

	class CastDivineShieldAction: public CastBuffSpellAction
	{
	public:
		CastDivineShieldAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "divine shield") {}
	};

    class CastHolyWrathAction : public CastMeleeSpellAction
    {
    public:
        CastHolyWrathAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "holy wrath") {}
    };

    class CastHammerOfJusticeAction : public CastMeleeSpellAction
    {
    public:
        CastHammerOfJusticeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "hammer of justice") {}
    };

	class CastHammerOfTheRighteousAction : public CastMeleeSpellAction
	{
	public:
		CastHammerOfTheRighteousAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "hammer of the righteous") {}
	};

	class CastPurifyPoisonAction : public CastCureSpellAction
	{
	public:
		CastPurifyPoisonAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "purify") {}
	};

	class CastPurifyDiseaseAction : public CastCureSpellAction
	{
	public:
		CastPurifyDiseaseAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "purify") {}
	};

    class CastPurifyPoisonOnPartyAction : public CurePartyMemberAction
    {
    public:
        CastPurifyPoisonOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "purify", DISPEL_POISON) {}
        virtual std::string getName() { return "purify poison on party"; }
    };

	class CastPurifyDiseaseOnPartyAction : public CurePartyMemberAction
	{
	public:
		CastPurifyDiseaseOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "purify", DISPEL_DISEASE) {}
		virtual std::string getName() { return "purify disease on party"; }
	};

	class CastHandOfReckoningAction : public CastSpellAction
	{
	public:
		CastHandOfReckoningAction(PlayerbotAI* ai) : CastSpellAction(ai, "hand of reckoning") {}
	};

	class CastRighteousDefenseAction : public CastSpellAction
	{
	public:
		CastRighteousDefenseAction(PlayerbotAI* ai) : CastSpellAction(ai, "righteous defense") {}
	};

	class CastCleansePoisonAction : public CastCureSpellAction
	{
	public:
		CastCleansePoisonAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "cleanse") {}
	};

	class CastCleanseDiseaseAction : public CastCureSpellAction
	{
	public:
		CastCleanseDiseaseAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "cleanse") {}
	};

	class CastCleanseMagicAction : public CastCureSpellAction
	{
	public:
		CastCleanseMagicAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "cleanse") {}
	};

    class CastCleansePoisonOnPartyAction : public CurePartyMemberAction
    {
    public:
        CastCleansePoisonOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "cleanse", DISPEL_POISON) {}
        virtual std::string getName() { return "cleanse poison on party"; }
    };

	class CastCleanseDiseaseOnPartyAction : public CurePartyMemberAction
	{
	public:
		CastCleanseDiseaseOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "cleanse", DISPEL_DISEASE) {}
		virtual std::string getName() { return "cleanse disease on party"; }
	};

	class CastCleanseMagicOnPartyAction : public CurePartyMemberAction
	{
	public:
		CastCleanseMagicOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "cleanse", DISPEL_MAGIC) {}
		virtual std::string getName() { return "cleanse magic on party"; }
	};

	SPELL_ACTION(CastExorcismAction, "exorcism");

    BEGIN_SPELL_ACTION(CastAvengersShieldAction, "avenger's shield")
    END_SPELL_ACTION()

	class CastHolyShieldAction : public CastBuffSpellAction
	{
	public:
		CastHolyShieldAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "holy shield") {}
	};

	class CastRedemptionAction : public ResurrectPartyMemberAction
	{
	public:
		CastRedemptionAction(PlayerbotAI* ai) : ResurrectPartyMemberAction(ai, "redemption") {}
	};

    class CastHammerOfJusticeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
    {
    public:
        CastHammerOfJusticeOnEnemyHealerAction(PlayerbotAI* ai) : CastSpellOnEnemyHealerAction(ai, "hammer of justice") {}
    };

    class CastHammerOfJusticeSnareAction : public CastSnareSpellAction
    {
    public:
        CastHammerOfJusticeSnareAction(PlayerbotAI* ai) : CastSnareSpellAction(ai, "hammer of justice") {}
    };

    class CastTurnUndeadAction : public CastBuffSpellAction
    {
    public:
        CastTurnUndeadAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "turn undead") {}
        virtual Value<Unit*>* GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }
    };

	class CastHandOfSacrificeAction : public BuffOnPartyAction
	{
	public:
		CastHandOfSacrificeAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "hand of sacrifice") {}
	};

	class CastBlessingOfSacrificeAction : public BuffOnPartyAction
	{
	public:
		CastBlessingOfSacrificeAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "blessing of sacrifice") {}
	};

    class CastBlessingOfFreedomOnPartyAction : public CastSpellAction
    {
    public:
		CastBlessingOfFreedomOnPartyAction(PlayerbotAI* ai) : CastSpellAction(ai, "blessing of freedom") {}
        bool isUseful() override { return CastSpellAction::isUseful() && !ai->HasAura(GetSpellName(), GetTarget()); }
        std::string GetReachActionName() override { return "reach spell"; }
        std::string GetTargetName() override { return "party member to remove roots"; }
    };

    class CastBlessingOfProtectionOnPartyAction : public CastProtectSpellAction
    {
    public:
        CastBlessingOfProtectionOnPartyAction(PlayerbotAI* ai) : CastProtectSpellAction(ai, "blessing of protection") {}
        bool isUseful() override 
        { 
            Unit* target = GetTarget();
            if(target && target->IsPlayer() && !ai->IsTank((Player*)target))
            {
                return CastProtectSpellAction::isUseful();
            }

            return false;
        }
    };

    class UpdatePaladinPveStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdatePaladinPveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
        {
            std::vector<std::string> strategiesRequired = { "holy/heal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura holy pve", strategiesRequired);

            strategiesRequired = { "holy/heal", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing holy pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing holy pve", strategiesRequired);

            strategiesRequired = { "retribution" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura retribution pve", strategiesRequired);

            strategiesRequired = { "retribution", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing retribution pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing retribution pve", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing protection pve", strategiesRequired);
        
            strategiesRequired = { "offheal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal pve", strategiesRequired);
        }
    };

    class UpdatePaladinPvpStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdatePaladinPvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
        {
            std::vector<std::string> strategiesRequired = { "holy/heal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura holy pvp", strategiesRequired);

            strategiesRequired = { "holy/heal", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing holy pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing holy pvp", strategiesRequired);

            strategiesRequired = { "retribution" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura retribution pvp", strategiesRequired);

            strategiesRequired = { "retribution", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing retribution pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing retribution pvp", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing protection pvp", strategiesRequired);
        
            strategiesRequired = { "offheal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal pvp", strategiesRequired);
        }
    };

    class UpdatePaladinRaidStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdatePaladinRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
        {
            std::vector<std::string> strategiesRequired = { "holy/heal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura holy raid", strategiesRequired);

            strategiesRequired = { "holy/heal", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing holy raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing holy raid", strategiesRequired);

            strategiesRequired = { "retribution" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura retribution raid", strategiesRequired);

            strategiesRequired = { "retribution", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing retribution raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing retribution raid", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "cure" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "aura" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aura protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aura protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "blessing" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "blessing protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "blessing protection raid", strategiesRequired);
        
            strategiesRequired = { "offheal" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal raid", strategiesRequired);
        }
    };
}
