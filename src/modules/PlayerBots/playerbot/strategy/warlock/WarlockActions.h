#pragma once

#include "playerbot/strategy/actions/GenericActions.h"
#include "playerbot/strategy/actions/UseItemAction.h"

namespace ai
{
	SNARE_ACTION(CastDeathCoilSnareAction, "death coil");
	ENEMY_HEALER_ACTION(CastDeathCoilOnHealerAction, "death coil");
	SPELL_ACTION(CastDeathCoilAction, "death coil");
    BUFF_ACTION(CastShadowWardAction, "shadow ward");
	
	class CastDemonSkinAction : public CastBuffSpellAction 
	{
	public:
		CastDemonSkinAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demon skin") {}
	};

	class CastDemonArmorAction : public CastBuffSpellAction
	{
	public:
		CastDemonArmorAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demon armor") {}
	};

	class CastFelArmorAction : public CastBuffSpellAction
	{
	public:
		CastFelArmorAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "fel armor") {}
	};

    BEGIN_RANGED_SPELL_ACTION(CastShadowBoltAction, "shadow bolt")
    END_SPELL_ACTION()

	class CastDrainSoulAction : public CastSpellAction
	{
	public:
		CastDrainSoulAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain soul") {}
	};

    class CastShadowburnAction : public CastSpellAction
    {
    public:
		CastShadowburnAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowburn") {}

        bool Execute(Event& event) override
        {
            if (CastSpellAction::Execute(event))
            {
                // Remove potential extra soul shard
                if (ai->HasCheat(BotCheatMask::item))
                {
                    Unit* target = GetTarget();
                    if (target && AI_VALUE2(uint8, "health", GetTargetName()) <= 20)
                    {
                        Item* soulShard = bot->GetItemByEntry(6265);
                        if (soulShard)
                        {
                            bot->DestroyItem(soulShard->GetBagSlot(), soulShard->GetSlot(), true);
                        }
                    }
                }
                
                return true;
            }

            return false;
        }
    };

    class CastSoulstoneAction : public CastItemTargetAction
    {
    public:
        CastSoulstoneAction(PlayerbotAI* ai) : CastItemTargetAction(ai, "revive targets", true, true) {}

    private:
        uint32 GetItemId() override 
        { 
            uint32 itemId = 0;
            const uint32 level = bot->GetLevel();
            if (level >= 18 && level < 30)
            {
                itemId = 5232;
            }
            else if (level >= 30 && level < 40)
            {
                itemId = 16892;
            }
            else if (level >= 40 && level < 50)
            {
                itemId = 16893;
            }
            else if (level >= 50 && level < 60)
            {
                itemId = 16895;
            }
            else if (level >= 60 && level < 70)
            {
                itemId = 16896;
            }
            else if (level >= 70 && level < 76)
            {
                itemId = 22116;
            }
            else if (level >= 76)
            {
                itemId = 36895;
            }

            return itemId;
        }
    };

    class CastSoulShatterAction : public CastSpellAction
    {
    public:
		CastSoulShatterAction(PlayerbotAI* ai) : CastSpellAction(ai, "soulshatter") {}
        std::string GetTargetName() override { return "self target"; }
    };

    class CastSoulFireAction : public CastSpellAction
    {
    public:
		CastSoulFireAction(PlayerbotAI* ai) : CastSpellAction(ai, "soul fire") {}
    };

	BUFF_ACTION(CastDarkPactAction, "dark pact");

	class CastDrainManaAction : public CastSpellAction
	{
	public:
		CastDrainManaAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain mana") {}
	};

	class CastDrainLifeAction : public CastSpellAction
	{
	public:
		CastDrainLifeAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain life") {}
	};

    class CastCurseOfExhaustionAction : public CastRangedDebuffSpellAction
    {
    public:
		CastCurseOfExhaustionAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of exhaustion") {}
    };

	class CastCorruptionAction : public CastRangedDebuffSpellAction
	{
	public:
		CastCorruptionAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "corruption") {}
	};

	class CastCorruptionOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
	{
	public:
	    CastCorruptionOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "corruption") {}
	};

    class CastSeedOfCorruptionOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
		CastSeedOfCorruptionOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "seed of corruption") {}
    };

    class CastUnstableAfflictionOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
		CastUnstableAfflictionOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "unstable affliction") {}
    };

    class CastCurseOfAgonyAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfAgonyAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of agony") {}
    };

    class CastCurseOfAgonyOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfAgonyOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of agony") {}
    };

    class CastCurseOfDoomAction : public CastRangedDebuffSpellAction
    {
    public:
		CastCurseOfDoomAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of doom") {}
    };

    class CastCurseOfDoomOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
		CastCurseOfDoomOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of doom") {}
    };

    class CastCurseOfTheElementsAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfTheElementsAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of the elements") {}
    };

    class CastCurseOfTheElementsOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfTheElementsOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of the elements") {}
    };

    class CastCurseOfRecklessnessAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfRecklessnessAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of recklessness") {}
    };

    class CastCurseOfRecklessnessOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfRecklessnessOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of recklessness") {}
    };

    class CastCurseOfWeaknessAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfWeaknessAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of weakness") {}
    };

    class CastCurseOfWeaknessOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfWeaknessOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of weakness") {}
    };

    class CastCurseOfTonguesAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfTonguesAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of tongues") {}
    };

    class CastCurseOfTonguesOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfTonguesOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of tongues") {}
    };

    class CastCurseOfShadowAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCurseOfShadowAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of shadow") {}
    };

    class CastCurseOfShadowOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfShadowOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of shadow") {}
    };

    class CastDemonicSacrificeAction : public CastBuffSpellAction
    {
    public:
		CastDemonicSacrificeAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demonic sacrifice") {}
    };

    class CastSoulLinkAction : public CastBuffSpellAction
    {
    public:
		CastSoulLinkAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "soul link") {}
    };

    class CastSacrificeAction : public CastPetSpellAction
    {
    public:
        CastSacrificeAction(PlayerbotAI* ai) : CastPetSpellAction(ai, "sacrifice") {}
        std::string GetTargetName() override { return "self target"; }
    };

    class CastSpellLockAction : public CastPetSpellAction
    {
    public:
        CastSpellLockAction(PlayerbotAI* ai) : CastPetSpellAction(ai, "spell lock") {}
    };

    class CastSpellLockOnEnemyHealerAction : public CastPetSpellAction
    {
    public:
        CastSpellLockOnEnemyHealerAction(PlayerbotAI* ai) : CastPetSpellAction(ai, "spell lock") {}
        virtual std::string GetTargetName() override { return "enemy healer target"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual std::string getName() override { return GetSpellName() + " on enemy healer"; }
    };

	class CastSummonImpAction : public CastBuffSpellAction
	{
	public:
		CastSummonImpAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon imp") {}
        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            Unit* pet = AI_VALUE(Unit*, "pet target");
            if (pet)
            {
                return pet->GetEntry() != 416;
            }

            return true;
        }
	};

    class CastSummonSuccubusAction : public CastSpellAction
    {
    public:
        CastSummonSuccubusAction(PlayerbotAI* ai) : CastSpellAction(ai, "summon succubus") {}
        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            Unit* pet = AI_VALUE(Unit*, "pet target");
            if (pet)
            {
                return pet->GetEntry() != 1863;
            }

            return true;
        }
    };

	class CastSummonFelhunterAction : public CastSpellAction
	{
	public:
		CastSummonFelhunterAction(PlayerbotAI* ai) : CastSpellAction(ai, "summon felhunter") {}
        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            Unit* pet = AI_VALUE(Unit*, "pet target");
            if (pet)
            {
                return pet->GetEntry() != 417;
            }

            return true;
        }
	};

    class CastSummonVoidwalkerAction : public CastSpellAction
    {
    public:
        CastSummonVoidwalkerAction(PlayerbotAI* ai) : CastSpellAction(ai, "summon voidwalker") {}
        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            Unit* pet = AI_VALUE(Unit*, "pet target");
            if (pet)
            {
                return pet->GetEntry() != 1860;
            }

            return true;
        }
    };

    class CastSummonFelguardAction : public CastSpellAction
    {
    public:
        CastSummonFelguardAction(PlayerbotAI* ai) : CastSpellAction(ai, "summon felguard") {}
        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            Unit* pet = AI_VALUE(Unit*, "pet target");
            if (pet)
            {
                return pet->GetEntry() != 17252;
            }

            return true;
        }
    };

	class CastSummonInfernoAction : public CastSpellAction
	{
	public:
		CastSummonInfernoAction(PlayerbotAI* ai) : CastSpellAction(ai, "inferno") {}
		virtual bool isPossible() { return true; }
	};

	class CastCreateHealthstoneAction : public CastSpellAction
	{
	public:
		CastCreateHealthstoneAction(PlayerbotAI* ai) : CastSpellAction(ai, "create healthstone") {}
        std::string GetTargetName() override { return "self target"; }
	};

	class CastCreateFirestoneAction : public CastSpellAction
	{
	public:
		CastCreateFirestoneAction(PlayerbotAI* ai) : CastSpellAction(ai, "create firestone") {}
        std::string GetTargetName() override { return "self target"; }
	};

	class CastCreateSpellstoneAction : public CastSpellAction
	{
	public:
		CastCreateSpellstoneAction(PlayerbotAI* ai) : CastSpellAction(ai, "create spellstone") {}
        std::string GetTargetName() override { return "self target"; }
	};

    class CastBanishAction : public CastSpellAction
    {
    public:
        CastBanishAction(PlayerbotAI* ai) : CastSpellAction(ai, "banish") {}
        virtual std::string GetTargetName() override { return "snare target"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual ActionThreatType getThreatType() { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class CastBanishOnCcAction : public CastCrowdControlSpellAction
    {
    public:
        CastBanishOnCcAction(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, "banish") {}
    };

    class CastSeedOfCorruptionAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSeedOfCorruptionAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "seed of corruption") {}
    };

    class CastRainOfFireAction : public CastSpellAction
    {
    public:
        CastRainOfFireAction(PlayerbotAI* ai) : CastSpellAction(ai, "rain of fire") {}
    };

    class CastShadowfuryAction : public CastSpellAction
    {
    public:
        CastShadowfuryAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowfury") {}
    };

	SNARE_ACTION(CastShadowfurySnareAction, "shadowfury");
	RANGED_DEBUFF_ACTION(CastUnstableAfflictionAction, "unstable affliction");

    class CastImmolateAction : public CastRangedDebuffSpellAction
    {
    public:
        CastImmolateAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "immolate") {}
    };

    class CastConflagrateAction : public CastSpellAction
    {
    public:
        CastConflagrateAction(PlayerbotAI* ai) : CastSpellAction(ai, "conflagrate") {}
    };

    class CastIncinerateAction : public CastSpellAction
    {
    public:
        CastIncinerateAction(PlayerbotAI* ai) : CastSpellAction(ai, "incinerate") {}
    };

    class CastFearAction : public CastRangedDebuffSpellAction
    {
    public:
        CastFearAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "fear") {}
    };

    class CastFearOnCcAction : public CastCrowdControlSpellAction
    {
    public:
        CastFearOnCcAction(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, "fear") {}
    };

    class CastLifeTapAction: public CastSpellAction
    {
    public:
        CastLifeTapAction(PlayerbotAI* ai) : CastSpellAction(ai, "life tap") {}
        virtual std::string GetTargetName() { return "self target"; }
        virtual bool isUseful() { return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig.lowHealth; }
    };

    class CastAmplifyCurseAction : public CastBuffSpellAction
    {
    public:
        CastAmplifyCurseAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "amplify curse") {}
    };

    class CastSiphonLifeAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSiphonLifeAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "siphon life") {}
    };

    class CastSiphonLifeOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastSiphonLifeOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "siphon life") {}
    };

    class CastHowlOfTerrorAction : public CastMeleeAoeSpellAction
    {
    public:
		CastHowlOfTerrorAction(PlayerbotAI* ai) : CastMeleeAoeSpellAction(ai, "howl of terror", 10.0f) {}
    };

	SPELL_ACTION(CastSearingPainAction, "searing pain");

    class UpdateWarlockPveStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarlockPveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
        {
            std::vector<std::string> strategiesRequired = { "demonology" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost demonology pve", strategiesRequired);

            strategiesRequired = { "demonology", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse demonology pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse demonology pve", strategiesRequired);

            strategiesRequired = { "destruction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost destruction pve", strategiesRequired);

            strategiesRequired = { "destruction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse destruction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse destruction pve", strategiesRequired);

            strategiesRequired = { "affliction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost affliction pve", strategiesRequired);

            strategiesRequired = { "affliction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse affliction pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse affliction pve", strategiesRequired);
        }
    };

    class UpdateWarlockPvpStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarlockPvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
        {
            std::vector<std::string> strategiesRequired = { "demonology" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost demonology pvp", strategiesRequired);

            strategiesRequired = { "demonology", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse demonology pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse demonology pvp", strategiesRequired);

            strategiesRequired = { "destruction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost destruction pvp", strategiesRequired);

            strategiesRequired = { "destruction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse destruction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse destruction pvp", strategiesRequired);

            strategiesRequired = { "affliction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost affliction pvp", strategiesRequired);

            strategiesRequired = { "affliction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse affliction pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse affliction pvp", strategiesRequired);
        }
    };

    class UpdateWarlockRaidStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarlockRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
        {
            std::vector<std::string> strategiesRequired = { "demonology" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost demonology raid", strategiesRequired);

            strategiesRequired = { "demonology", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse demonology raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse demonology raid", strategiesRequired);

            strategiesRequired = { "destruction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost destruction raid", strategiesRequired);

            strategiesRequired = { "destruction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse destruction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse destruction raid", strategiesRequired);

            strategiesRequired = { "affliction" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "pet" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "pet affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "pet affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost affliction raid", strategiesRequired);

            strategiesRequired = { "affliction", "curse" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "curse affliction raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "curse affliction raid", strategiesRequired);
        }
    };
}
