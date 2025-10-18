#pragma once
#include "playerbot/strategy/actions/GenericActions.h"
#include "playerbot/strategy/actions/UseItemAction.h"

namespace ai
{
    BUFF_ACTION(CastColdBloodAction, "cold blood");

    BUFF_ACTION_U(CastPreparationAction, "preparation", !bot->IsSpellReady(14177) || !bot->IsSpellReady(2983) || !bot->IsSpellReady(2094));

    class CastShadowstepAction : public CastSpellAction 
    {
    public:
        CastShadowstepAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowstep") {}

        virtual bool isPossible() { return true; }

        virtual bool isUseful() 
        {
            return bot->HasSpell(36554) && bot->IsSpellReady(36554);
        }

        virtual bool Execute(Event& event) 
        {
            return bot->CastSpell(GetTarget(), 36554, TRIGGERED_OLD_TRIGGERED);
        }
    };

	class CastEvasionAction : public CastBuffSpellAction
	{
	public:
		CastEvasionAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "evasion") {}
	};

	class CastSprintAction : public CastBuffSpellAction
	{
	public:
		CastSprintAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "sprint") {}
        virtual std::string GetTargetName() { return "self target"; }
	};

    class CastStealthAction : public CastBuffSpellAction
    {
    public:
        CastStealthAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "stealth") {}
        
        virtual std::string GetTargetName() { return "self target"; }
        
        virtual bool isUseful()
        {
            if (ai->HasAura("stealth", bot))
            {
                return false;
            }

            // do not use with WSG flag
            return !ai->HasAura(23333, bot) && !ai->HasAura(23335, bot) && !ai->HasAura(34976, bot);
        }

        virtual bool Execute(Event& event)
        {
            if (ai->CastSpell("stealth", bot))
            {
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_NON_COMBAT);
                bot->InterruptSpell(CURRENT_MELEE_SPELL);
            }

            return true;
        }
    };

    class RogueUnstealthAction : public Action 
    {
    public:
        RogueUnstealthAction(PlayerbotAI* ai) : Action(ai, "unstealth") {}

        bool Execute(Event& event) override
        {
            if (ai->HasAura("stealth", bot))
            {
                ai->RemoveAura("stealth");
                ai->ChangeStrategy("-stealthed", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("-stealthed", BotState::BOT_STATE_NON_COMBAT);
                return true;
            }

            return false;
        }
    };

    class CheckStealthAction : public Action 
    {
    public:
        CheckStealthAction(PlayerbotAI* ai) : Action(ai, "check stealth") {}

        bool Execute(Event& event) override
        {
            const bool isStealthed = ai->HasAura("stealth", bot);
            const bool hasStealthStrategy = ai->HasStrategy("stealthed", BotState::BOT_STATE_COMBAT);

            if (isStealthed && !hasStealthStrategy)
            {
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_NON_COMBAT);
                return true;
            }
            else if (!isStealthed && hasStealthStrategy)
            {
                ai->ChangeStrategy("-stealthed", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("-stealthed", BotState::BOT_STATE_NON_COMBAT);
                return true;
            }

            return false;
        }

        bool isUseful() override
        {
            const bool isStealthed = ai->HasAura("stealth", bot);
            const bool hasStealthStrategy = ai->HasStrategy("stealthed", BotState::BOT_STATE_COMBAT);
            return (isStealthed && !hasStealthStrategy) || (!isStealthed && hasStealthStrategy);
        }
    };

	class CastKickAction : public CastMeleeSpellAction
	{
	public:
		CastKickAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "kick") {}
	};

	class CastFeintAction : public CastBuffSpellAction
	{
	public:
		CastFeintAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "feint") {}
	};

	class CastDismantleAction : public CastSpellAction
	{
	public:
		CastDismantleAction(PlayerbotAI* ai) : CastSpellAction(ai, "dismantle") {}
	};

	class CastDistractAction : public CastSpellAction
	{
	public:
		CastDistractAction(PlayerbotAI* ai) : CastSpellAction(ai, "distract") {}
	};

	class CastVanishAction : public CastBuffSpellAction
	{
	public:
		CastVanishAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "vanish") {}

        virtual bool isUseful()
        {
            // do not use with WSG flag or EYE flag
            return !ai->HasAura(23333, bot) && !ai->HasAura(23335, bot) && !ai->HasAura(34976, bot);
        }

        virtual bool Execute(Event& event)
        {
            if (ai->CastSpell("vanish", bot))
            {
                if (ai->HasStrategy("stealth", BotState::BOT_STATE_COMBAT))
                {
                    ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                    ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_NON_COMBAT);
                }

                bot->InterruptSpell(CURRENT_MELEE_SPELL);
                return true;
            }
            return false;
        }
	};

    MELEE_DEBUFF_ACTION_R(CastBlindAction, "blind", 10.0f);

	class CastBladeFlurryAction : public CastBuffSpellAction
	{
	public:
		CastBladeFlurryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blade flurry") {}
	};

	class CastAdrenalineRushAction : public CastBuffSpellAction
	{
	public:
		CastAdrenalineRushAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "adrenaline rush") {}
	};

	class CastKillingSpreeAction : public CastBuffSpellAction
	{
	public:
		CastKillingSpreeAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "killing spree") {}
	};

    class CastKickOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
    {
    public:
        CastKickOnEnemyHealerAction(PlayerbotAI* ai) : CastSpellOnEnemyHealerAction(ai, "kick")
        {
            range = ATTACK_DISTANCE;
        }

    private:
        std::string GetReachActionName() override { return "reach melee"; }
    };

    class CastTricksOfTheTradeOnPartyAction : public BuffOnPartyAction 
    {
    public:
        CastTricksOfTheTradeOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "tricks of the trade") {}
    };

    class CastCloakOfShadowsAction : public CastCureSpellAction
    {
    public:
        CastCloakOfShadowsAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "cloak of shadows") {}
    };

    class CastSapAction : public CastMeleeSpellAction
    {
    public:
        CastSapAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "sap") {}

        virtual Value<Unit*>* GetTargetValue()
        {
            return context->GetValue<Unit*>("cc target", getName());
        }

        virtual bool isUseful() { return true; }
    };

    class CastGarroteAction : public CastMeleeSpellAction
    {
    public:
        CastGarroteAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "garrote") {}
    };

    class CastCheapShotAction : public CastMeleeSpellAction
    {
    public:
        CastCheapShotAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "cheap shot") {}
    };

    class CastAmbushAction : public CastMeleeSpellAction
    {
    public:
        CastAmbushAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "ambush") {}
    };

    class CastEviscerateAction : public CastMeleeSpellAction
    {
    public:
        CastEviscerateAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "eviscerate") {}
    };

    class CastSliceAndDiceAction : public CastMeleeSpellAction
    {
    public:
        CastSliceAndDiceAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "slice and dice") {}
    };

    class CastExposeArmorAction : public CastMeleeSpellAction
    {
    public:
        CastExposeArmorAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "expose armor") {}
    };

    class CastRuptureAction : public CastMeleeDebuffSpellAction
    {
    public:
        CastRuptureAction(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, "rupture") {}
    };

    class CastKidneyShotAction : public CastMeleeSpellAction
    {
    public:
        CastKidneyShotAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "kidney shot") {}
    };

    class CastComboAction : public CastMeleeSpellAction
    {
    public:
        CastComboAction(PlayerbotAI* ai, std::string name) : CastMeleeSpellAction(ai, name) {}

        virtual bool isUseful()
        {
            return CastMeleeSpellAction::isUseful() && AI_VALUE2(uint8, "combo", "current target") < 5;
        }
    };

    class CastSinisterStrikeAction : public CastComboAction
    {
    public:
        CastSinisterStrikeAction(PlayerbotAI* ai) : CastComboAction(ai, "sinister strike") {}
    };

    class CastMutilateAction : public CastComboAction
    {
    public:
        CastMutilateAction(PlayerbotAI* ai) : CastComboAction(ai, "mutilate") {}
    };

    class CastRiposteAction : public CastComboAction
    {
    public:
        CastRiposteAction(PlayerbotAI* ai) : CastComboAction(ai, "riposte") {}
    };

    class CastGougeAction : public CastComboAction
    {
    public:
        CastGougeAction(PlayerbotAI* ai) : CastComboAction(ai, "gouge") {}
    };

    class CastBackstabAction : public CastComboAction
    {
    public:
        CastBackstabAction(PlayerbotAI* ai) : CastComboAction(ai, "backstab") {}
    };

    class CastHemorrhageAction : public CastComboAction
    {
    public:
        CastHemorrhageAction(PlayerbotAI* ai) : CastComboAction(ai, "hemorrhage") {}
    };

    class CastGhostlyStrikeAction : public CastComboAction
    {
    public:
        CastGhostlyStrikeAction(PlayerbotAI* ai) : CastComboAction(ai, "ghostly strike") {}

        virtual bool isUseful()
        {
            return CastComboAction::isUseful() && GetTarget() && (GetTarget()->getClass() == CLASS_WARRIOR ||
                GetTarget()->getClass() == CLASS_ROGUE);
        }
    };

    SPELL_ACTION(CastPremeditationAction, "premeditation");

    class ApplyPoisonAction : public Action
    {
    public:
        ApplyPoisonAction(PlayerbotAI* ai, bool inMainHand, const std::vector<uint32>& inPoisonItemIds, std::string name = "apply poison")
        : Action(ai, name)
        , mainHand(inMainHand)
        , poisonItemIds(inPoisonItemIds) {}

        bool isUseful() override
        {
            // Always allow poison on offhand
            if (!mainHand)
                return true;

#ifndef MANGOSBOT_TWO
            // Deny if grouped with shaman over 32 in same subgroup (non-WOTLK)
            if (bot->GetGroup())
            {
                Group* group = bot->GetGroup();
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->getSource();
                    if (!member || member == bot || !member->IsInWorld() || !group->SameSubGroup(bot, member))
                        continue;

                    if (member->getClass() == CLASS_SHAMAN && member->GetLevel() > 32)
                        return false;
                }
            }
#endif
            return true;
        }

        bool Execute(Event& event) override
        {
            // Pick a poison item
            uint32 poisonLevel = 0;
            uint32 poisonItemId = 0;
            const ItemPrototype* poisonProto = nullptr;
            for (const uint32 itemId : poisonItemIds)
            {
                // Check if bot can use the poison
                const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
                if (proto && (bot->GetLevel() >= proto->RequiredLevel) && (poisonLevel < proto->RequiredLevel))
                {
                    // Check if the bot has the poison in the bag
                    if (ai->HasCheat(BotCheatMask::item) || bot->HasItemCount(itemId, 1))
                    {
                        poisonItemId = itemId;
                        poisonLevel = proto->RequiredLevel;
                        poisonProto = proto;
                    }
                }
            }

            if (poisonItemId > 0)
            {
                const uint8 weaponSlot = mainHand ? EQUIPMENT_SLOT_MAINHAND : EQUIPMENT_SLOT_OFFHAND;
                if (ai->HasCheat(BotCheatMask::item))
                {
                    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, weaponSlot);
                    if (weapon)
                    {
                        SpellCastTargets targets;
                        targets.setItemTarget(weapon);
                        targets.m_targetMask = TARGET_FLAG_ITEM;

                        int count = 0;
                        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
                        {
                            const _Spell& spellData = poisonProto->Spells[i];
                            if (spellData.SpellId)
                            {
                                const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
                                if (spellInfo)
                                {
                                    BotUseItemSpell* spell = new BotUseItemSpell(bot, spellInfo, (count > 0) ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
                                    spell->m_clientCast = true;

                                    if (spell->ForceSpellStart(&targets) == SPELL_CAST_OK)
                                    {
                                        bot->RemoveSpellCooldown(*spellInfo, false);
                                        bot->AddCooldown(*spellInfo, poisonProto, false);
                                        SetDuration(3000);
                                    }
                                    else
                                    {
                                        return false;
                                    }

                                    ++count;
                                }
                            }
                        }

                        return count;
                    }
                }
                else
                {
                    Item* poisonItem = bot->GetItemByEntry(poisonItemId);
                    if (poisonItem)
                    {
                        ai->ImbueItem(poisonItem, weaponSlot);
                        SetDuration(3000);
                        return true;
                    }
                }
            }

            return false;
        }

        bool isPossible() override
        {
            // Check if poison item is in bag
            for (const uint32 itemId : poisonItemIds)
            {
                const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
                if (proto && (bot->GetLevel() >= proto->RequiredLevel))
                {
                    if (ai->HasCheat(BotCheatMask::item) || bot->HasItemCount(itemId, 1))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

    private:
        bool mainHand;
        std::vector<uint32> poisonItemIds;
    };

    class ApplyDeadlyPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyDeadlyPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 2892, 2893, 8984, 8985, 20844, 22053, 22054, 43232, 43233 }, "apply deadly poison") {}
    };

    class ApplyCripplingPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyCripplingPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 3775, 3776 }, "apply crippling poison") {}
    };

    class ApplyMindPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyMindPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 5237, 6951, 9186 }, "apply mind poison") {}
    };

    class ApplyInstantPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyInstantPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 6947, 6949, 6950, 8926, 8927, 8928, 21927, 43230, 43231 }, "apply instant poison") {}
    };

    class ApplyWoundPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyWoundPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 10918, 10920, 10921, 10922, 22055, 43234, 43235 }, "apply wound poison") {}
    };

    class ApplyAnestheticPoisonAction : public ApplyPoisonAction
    {
    public:
        ApplyAnestheticPoisonAction(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonAction(ai, inMainHand, { 21835, 43237 }, "apply anesthetic poison") {}
    };

    class UpdateRoguePveStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateRoguePveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
        {
            std::vector<std::string> strategiesRequired = { "assassination" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff assassination pve", strategiesRequired);

            strategiesRequired = { "assassination", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost assassination pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost assassination pve", strategiesRequired);

            strategiesRequired = { "combat" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "combat pve", strategiesRequired);

            strategiesRequired = { "combat", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe combat pve", strategiesRequired);

            strategiesRequired = { "combat", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc combat pve", strategiesRequired);

            strategiesRequired = { "combat", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth combat pve", strategiesRequired);

            strategiesRequired = { "combat", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons combat pve", strategiesRequired);

            strategiesRequired = { "combat", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff combat pve", strategiesRequired);

            strategiesRequired = { "combat", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost combat pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost combat pve", strategiesRequired);

            strategiesRequired = { "subtlety" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff subtlety pve", strategiesRequired);

            strategiesRequired = { "subtlety", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost subtlety pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost subtlety pve", strategiesRequired);
        }
    };

    class UpdateRoguePvpStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateRoguePvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
        {
            std::vector<std::string> strategiesRequired = { "assassination" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff assassination pvp", strategiesRequired);

            strategiesRequired = { "assassination", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost assassination pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost assassination pvp", strategiesRequired);

            strategiesRequired = { "combat" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff combat pvp", strategiesRequired);

            strategiesRequired = { "combat", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost combat pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost combat pvp", strategiesRequired);

            strategiesRequired = { "subtlety" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff subtlety pvp", strategiesRequired);

            strategiesRequired = { "subtlety", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost subtlety pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost subtlety pvp", strategiesRequired);
        }
    };

    class UpdateRogueRaidStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateRogueRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
        {
            std::vector<std::string> strategiesRequired = { "assassination" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff assassination raid", strategiesRequired);

            strategiesRequired = { "assassination", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost assassination raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost assassination raid", strategiesRequired);

            strategiesRequired = { "combat" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "combat raid", strategiesRequired);

            strategiesRequired = { "combat", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe combat raid", strategiesRequired);

            strategiesRequired = { "combat", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc combat raid", strategiesRequired);

            strategiesRequired = { "combat", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth combat raid", strategiesRequired);

            strategiesRequired = { "combat", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons combat raid", strategiesRequired);

            strategiesRequired = { "combat", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff combat raid", strategiesRequired);

            strategiesRequired = { "combat", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost combat raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost combat raid", strategiesRequired);

            strategiesRequired = { "subtlety" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "stealth" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "stealth subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "stealth subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "poisons" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "poisons subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "poisons subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff subtlety raid", strategiesRequired);

            strategiesRequired = { "subtlety", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost subtlety raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost subtlety raid", strategiesRequired);
        }
    };
}
