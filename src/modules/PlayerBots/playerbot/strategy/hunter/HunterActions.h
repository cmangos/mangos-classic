#pragma once

#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
    BEGIN_RANGED_DEBUFF_ACTION(CastHuntersMarkAction, "hunter's mark")
    END_SPELL_ACTION()

    class CastAutoShotAction : public CastSpellAction
    {
    public:
        CastAutoShotAction(PlayerbotAI* ai) : CastSpellAction(ai, "auto shot") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    BEGIN_RANGED_SPELL_ACTION(CastTranquilizingShotAction, "tranquilizing shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastArcaneShotAction, "arcane shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastExplosiveShotAction, "explosive shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastAimedShotAction, "aimed shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastChimeraShotAction, "chimera shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastConcussiveShotAction, "concussive shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastScatterShotAction, "scatter shot")
    END_SPELL_ACTION()

    class CastScatterShotOnClosestAttackerTargetingMeAction : public CastRangedDebuffSpellAction
    {
    public:
        CastScatterShotOnClosestAttackerTargetingMeAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "scatter shot") {}
        std::string GetTargetName() override { return "closest attacker targeting me target"; }
    };

    BEGIN_RANGED_SPELL_ACTION(CastDistractingShotAction, "distracting shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastMultiShotAction, "multi-shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastVolleyAction, "volley")
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastSerpentStingAction, "serpent sting")
        virtual bool isUseful();
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastViperStingAction, "viper sting")
        virtual bool isUseful();
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastScorpidStingAction, "scorpid sting")
    END_SPELL_ACTION()

    class CastAspectOfTheMonkeyAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheMonkeyAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the monkey") {}
    };

    class CastAspectOfTheHawkAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheHawkAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the hawk") {}
    };

    class CastAspectOfTheWildAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheWildAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the wild") {}
    };

    class CastAspectOfTheCheetahAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheCheetahAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the cheetah") {}
        
        bool isUseful() override
        {
            return CastBuffSpellAction::isUseful() && !AI_VALUE(bool, "has attackers");
        }
    };

    class CastAspectOfThePackAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfThePackAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the pack") {}
    
        bool isUseful() override
        {
            return CastBuffSpellAction::isUseful() && !AI_VALUE(bool, "has attackers");
        }
    };

    class CastAspectOfTheViperAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheViperAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the viper") {}
    };

    class CastAspectOfTheBeastAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheBeastAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the beast") {}
    };

    class CastAspectOfTheDragonhawkAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheDragonhawkAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the dragonhawk") {}
    };

    class CastCallPetAction : public CastBuffSpellAction
    {
    public:
        CastCallPetAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "call pet") {}
    };

    class CastMendPetAction : public CastAuraSpellAction
    {
    public:
        CastMendPetAction(PlayerbotAI* ai) : CastAuraSpellAction(ai, "mend pet") {}
        virtual std::string GetTargetName() { return "pet target"; }
    };

    class CastRevivePetAction : public CastBuffSpellAction
    {
    public:
        CastRevivePetAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "revive pet") {}
    };

    class CastTrueshotAuraAction : public CastBuffSpellAction
    {
    public:
        CastTrueshotAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "trueshot aura") {}
    };

    class CastFeignDeathAction : public CastBuffSpellAction
    {
    public:
        CastFeignDeathAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "feign death") {}
    };

    REMOVE_BUFF_ACTION(RemoveFeignDeathAction, "feign death");

    class CastRapidFireAction : public CastBuffSpellAction
    {
    public:
        CastRapidFireAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "rapid fire") {}
    };

    class CastKillCommandAction : public CastBuffSpellAction
    {
    public:
        CastKillCommandAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "kill command") {}
    };

    class CastBlackArrow : public CastRangedDebuffSpellAction
    {
    public:
        CastBlackArrow(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "black arrow") {}
    };

    SNARE_ACTION(CastBlackArrowSnareAction, "black arrow");
    SPELL_ACTION(CastSilencingShotAction, "silencing shot");
    ENEMY_HEALER_ACTION(CastSilencingShotOnHealerAction, "silencing shot");
    BUFF_ACTION(CastReadinessAction, "readiness");
    ;
    class CastWingClipAction : public CastMeleeSpellAction
    {
    public:
        CastWingClipAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "wing clip") {}

        virtual bool isUseful()
        {
            return CastMeleeSpellAction::isUseful() && !ai->HasAura(GetSpellName(), GetTarget());
        }
    };

    class CastRaptorStrikeAction : public CastMeleeSpellAction
    {
    public:
        CastRaptorStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "raptor strike") {}

        virtual bool isUseful()
        {
            Unit* target = GetTarget();
            return CastMeleeSpellAction::isUseful() && (ai->HasStrategy("close", BotState::BOT_STATE_COMBAT) || bot->hasUnitState(UNIT_STAT_MELEE_ATTACKING));
        }
    };

    class CastSerpentStingOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastSerpentStingOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "serpent sting") {}
    };

    class CastViperStingOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastViperStingOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "viper sting") {}
    };

    class FeedPetAction : public Action
    {
    public:
        FeedPetAction(PlayerbotAI* ai) : Action(ai, "feed pet") {}
        virtual bool Execute(Event& event);
    };

    class CastBestialWrathAction : public CastBuffSpellAction
    {
    public:
        CastBestialWrathAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "bestial wrath") {}
    };

    class CastScareBeastAction : public CastSpellAction
    {
    public:
        CastScareBeastAction(PlayerbotAI* ai) : CastSpellAction(ai, "scare beast") {}
    };

    class CastScareBeastCcAction : public CastCrowdControlSpellAction
    {
    public:
        CastScareBeastCcAction(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, "scare beast") {}
    };

    BUFF_ACTION(IntimidationAction, "intimidation");
    BUFF_ACTION(DeterrenceAction, "deterrence");
    MELEE_ACTION(CastCounterattackAction, "counterattack");
    SNARE_ACTION(WyvernStingSnareAction, "wyvern sting");
    MELEE_ACTION(MongooseBiteAction, "mongoose bite");

    class TameBeastAction : public CastSpellAction
    {
    public:
        TameBeastAction(PlayerbotAI* ai) : CastSpellAction(ai, "tame beast") {}
    };

    class CastFlareAction : public CastSpellAction
    {
    public:
        CastFlareAction(PlayerbotAI* ai) : CastSpellAction(ai, "flare") {}
        virtual std::string GetTargetName() override { return "nearest stealthed unit"; }
    };

    class CastSteadyShotAction : public CastSpellAction
    {
    public:
        CastSteadyShotAction(PlayerbotAI* ai) : CastSpellAction(ai, "steady shot") {}
        virtual bool Execute(Event& event);

    private:
        uint32 weaponDelay;
    };

    class TrapOnTargetAction : public CastSpellAction
    {
    public:
#ifdef MANGOSBOT_ZERO
        // For vanilla, bots need to feign death before dropping the trap
        TrapOnTargetAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, "feign death"), trapSpell(spell)
        {
            trapSpellID = AI_VALUE2(uint32, "spell id", trapSpell);
        }
#else
        TrapOnTargetAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell), trapSpell(spell) {}
#endif

    protected:
        // Traps don't really have target for the spell
        std::string GetTargetName() override { return "self target"; }

        // The move to target
        virtual std::string GetTrapTargetName() { return "current target"; }

        // The trap spell that will be used
        std::string GetTrapSpellName() { return trapSpell; }

        std::string GetReachActionName() override { return "reach melee"; }
        std::string GetTargetQualifier() override { return GetTrapSpellName(); }
        ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }

        NextAction** getPrerequisites() override
        {
            const std::string reachAction = GetReachActionName();
            const std::string spellName = GetSpellName();
            const std::string targetName = GetTrapTargetName();

            // Generate the reach action with qualifiers
            std::vector<std::string> qualifiers = { spellName, targetName, trapSpell };
            const std::string qualifiersStr = Qualified::MultiQualify(qualifiers, "::");
            return NextAction::merge(NextAction::array(0, new NextAction(reachAction + "::" + qualifiersStr), NULL), Action::getPrerequisites());
        }

#ifdef MANGOSBOT_ZERO
        bool isPossible() override
        {
            // If the trap spell and feign death are not on cooldown
            return sServerFacade.IsSpellReady(bot, trapSpellID) && sServerFacade.IsSpellReady(bot, 5384);
        }

        NextAction** getContinuers() override
        {
            return NextAction::merge(NextAction::array(0, new NextAction(trapSpell, ACTION_PASSTROUGH), NULL), CastSpellAction::getContinuers());
        }
#endif

private:
        std::string trapSpell;
        uint32 trapSpellID;
    };

    class TrapOnCcTargetAction : public TrapOnTargetAction
    {
    public:
        TrapOnCcTargetAction(PlayerbotAI* ai, std::string spell) : TrapOnTargetAction(ai, spell) {}
        std::string GetTrapTargetName() override { return "cc target"; }
    };

    class TrapInPlace : public TrapOnTargetAction
    {
    public:
        TrapInPlace(PlayerbotAI* ai, std::string spell) : TrapOnTargetAction(ai, spell) {}
        std::string GetTrapTargetName() override { return "self target"; }
    };

    class CastTrapAction : public CastSpellAction
    {
    public:
        CastTrapAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}

        // Traps don't really have target for the spell
        std::string GetTargetName() override { return "self target"; }

#ifdef MANGOSBOT_ZERO
        bool Execute(Event& event) override
        {
            // The trap could come just after feign death, so better remove it
            ai->RemoveAura("feign death");
            return CastSpellAction::Execute(event);
        }
#endif
    };

    class CastImmolationTrapAction : public CastTrapAction
    {
    public:
        CastImmolationTrapAction(PlayerbotAI* ai) : CastTrapAction(ai, "immolation trap") {}
    };

    class CastImmolationTrapOnTargetAction : public TrapOnTargetAction
    {
    public:
        CastImmolationTrapOnTargetAction(PlayerbotAI* ai) : TrapOnTargetAction(ai, "immolation trap") {}
    };

    class CastImmolationTrapInPlaceAction : public TrapInPlace
    {
    public:
        CastImmolationTrapInPlaceAction(PlayerbotAI* ai) : TrapInPlace(ai, "immolation trap") {}
    };

    class CastFreezingTrapAction : public CastTrapAction
    {
    public:
        CastFreezingTrapAction(PlayerbotAI* ai) : CastTrapAction(ai, "freezing trap") {}
    };

    class CastFreezingTrapOnTargetAction : public TrapOnTargetAction
    {
    public:
        CastFreezingTrapOnTargetAction(PlayerbotAI* ai) : TrapOnTargetAction(ai, "freezing trap") {}
    };

    class CastFreezingTrapInPlaceAction : public TrapInPlace
    {
    public:
        CastFreezingTrapInPlaceAction(PlayerbotAI* ai) : TrapInPlace(ai, "freezing trap") {}
    };

    class CastFreezingTrapOnCcAction : public TrapOnCcTargetAction
    {
    public:
        CastFreezingTrapOnCcAction(PlayerbotAI* ai) : TrapOnCcTargetAction(ai, "freezing trap") {}
    };

    class CastFrostTrapAction : public CastTrapAction
    {
    public:
        CastFrostTrapAction(PlayerbotAI* ai) : CastTrapAction(ai, "frost trap") {}
    };

    class CastFrostTrapOnTargetAction : public TrapOnTargetAction
    {
    public:
        CastFrostTrapOnTargetAction(PlayerbotAI* ai) : TrapOnTargetAction(ai, "frost trap") {}
    };

    class CastFrostTrapInPlaceAction : public TrapInPlace
    {
    public:
        CastFrostTrapInPlaceAction(PlayerbotAI* ai) : TrapInPlace(ai, "frost trap") {}
    };

    class CastExplosiveTrapAction : public CastTrapAction
    {
    public:
        CastExplosiveTrapAction(PlayerbotAI* ai) : CastTrapAction(ai, "explosive trap") {}
    };

    class CastExplosiveTrapOnTargetAction : public TrapOnTargetAction
    {
    public:
        CastExplosiveTrapOnTargetAction(PlayerbotAI* ai) : TrapOnTargetAction(ai, "explosive trap") {}
    };

    class CastExplosiveTrapInPlaceAction : public TrapInPlace
    {
    public:
        CastExplosiveTrapInPlaceAction(PlayerbotAI* ai) : TrapInPlace(ai, "explosive trap") {}
    };

    class CastDismissPetAction : public CastSpellAction
    {
    public:
        CastDismissPetAction(PlayerbotAI* ai) : CastSpellAction(ai, "dismiss pet") {}

        std::string GetTargetName() override { return "self target"; }

        bool isUseful() override
        {
            return AI_VALUE(Unit*, "pet target");
        }
    };

    class UpdateHunterPveStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateHunterPveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
        {
            std::vector<std::string> strategiesRequired = { "beast mastery" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting beast mastery pve", strategiesRequired);

            strategiesRequired = { "beast mastery", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect beast mastery pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect beast mastery pve", strategiesRequired);

            strategiesRequired = { "marksmanship" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting marksmanship pve", strategiesRequired);

            strategiesRequired = { "marksmanship", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect marksmanship pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect marksmanship pve", strategiesRequired);

            strategiesRequired = { "survival" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "survival pve", strategiesRequired);

            strategiesRequired = { "survival", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe survival pve", strategiesRequired);

            strategiesRequired = { "survival", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff survival pve", strategiesRequired);

            strategiesRequired = { "survival", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost survival pve", strategiesRequired);

            strategiesRequired = { "survival", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc survival pve", strategiesRequired);

            strategiesRequired = { "survival", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting survival pve", strategiesRequired);

            strategiesRequired = { "survival", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect survival pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect survival pve", strategiesRequired);
        }
    };

    class UpdateHunterPvpStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateHunterPvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
        {
            std::vector<std::string> strategiesRequired = { "beast mastery" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting beast mastery pvp", strategiesRequired);

            strategiesRequired = { "beast mastery", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect beast mastery pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect beast mastery pvp", strategiesRequired);

            strategiesRequired = { "marksmanship" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting marksmanship pvp", strategiesRequired);

            strategiesRequired = { "marksmanship", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect marksmanship pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect marksmanship pvp", strategiesRequired);

            strategiesRequired = { "survival" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting survival pvp", strategiesRequired);

            strategiesRequired = { "survival", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect survival pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect survival pvp", strategiesRequired);
        }
    };

    class UpdateHunterRaidStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateHunterRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
        {
            std::vector<std::string> strategiesRequired = { "beast mastery" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting beast mastery raid", strategiesRequired);

            strategiesRequired = { "beast mastery", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect beast mastery raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect beast mastery raid", strategiesRequired);

            strategiesRequired = { "marksmanship" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting marksmanship raid", strategiesRequired);

            strategiesRequired = { "marksmanship", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect marksmanship raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect marksmanship raid", strategiesRequired);

            strategiesRequired = { "survival" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "survival raid", strategiesRequired);

            strategiesRequired = { "survival", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe survival raid", strategiesRequired);

            strategiesRequired = { "survival", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff survival raid", strategiesRequired);

            strategiesRequired = { "survival", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost survival raid", strategiesRequired);

            strategiesRequired = { "survival", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc survival raid", strategiesRequired);

            strategiesRequired = { "survival", "sting" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "sting survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "sting survival raid", strategiesRequired);

            strategiesRequired = { "survival", "aspect" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aspect survival raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aspect survival raid", strategiesRequired);
        }
    };
}
