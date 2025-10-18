#pragma once
#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
    // disc
    BUFF_ACTION(CastPowerWordFortitudeAction, "power word: fortitude");
    BUFF_PARTY_ACTION(CastPowerWordFortitudeOnPartyAction, "power word: fortitude");
    GREATER_BUFF_PARTY_ACTION(CastPrayerOfFortitudeOnPartyAction, "prayer of fortitude");
    BUFF_ACTION(CastPowerWordShieldAction, "power word: shield");
    HEAL_PARTY_ACTION(CastPowerWordShieldOnPartyAction, "power word: shield");
    BUFF_ACTION(CastInnerFireAction, "inner fire");
    CURE_ACTION(CastDispelMagicAction, "dispel magic");
    CURE_PARTY_ACTION(CastDispelMagicOnPartyAction, "dispel magic", DISPEL_MAGIC);
    SPELL_ACTION(CastDispelMagicOnTargetAction, "dispel magic");
    CC_ACTION(CastShackleUndeadAction, "shackle undead");
    SPELL_ACTION_U(CastManaBurnAction, "mana burn", AI_VALUE2(uint8, "mana", "self target") < 50 && AI_VALUE2(uint8, "mana", "current target") >= 20);
    BUFF_ACTION(CastLevitateAction, "levitate");
    BUFF_ACTION(CastDivineSpiritAction, "divine spirit");
    BUFF_PARTY_ACTION(CastDivineSpiritOnPartyAction, "divine spirit");
    GREATER_BUFF_PARTY_ACTION(CastPrayerOfSpiritOnPartyAction, "prayer of spirit");
    //disc 2.4.3
    SPELL_ACTION(CastMassDispelAction, "mass dispel");

    // disc talents
    BUFF_ACTION(CastInnerFocusAction, "inner focus");
    // disc 2.4.3 talents
    BUFF_ACTION(CastPainSuppressionAction, "pain suppression");
    PROTECT_ACTION(CastPainSuppressionProtectAction, "pain suppression");

    // holy
    HEAL_ACTION(CastLesserHealAction, "lesser heal");
    HEAL_PARTY_ACTION(CastLesserHealOnPartyAction, "lesser heal");
    HEAL_ACTION(CastHealAction, "heal");
    HEAL_PARTY_ACTION(CastHealOnPartyAction, "heal");
    HEAL_ACTION(CastGreaterHealAction, "greater heal");
    HEAL_PARTY_ACTION(CastGreaterHealOnPartyAction, "greater heal");
    HEAL_ACTION(CastFlashHealAction, "flash heal");
    HEAL_PARTY_ACTION(CastFlashHealOnPartyAction, "flash heal");
    HEAL_ACTION(CastRenewAction, "renew");
    HEAL_HOT_PARTY_ACTION(CastRenewOnPartyAction, "renew");
    // holy 2.4.3
    HEAL_PARTY_ACTION(CastPrayerOfMendingAction, "prayer of mending");
    HEAL_PARTY_ACTION(CastBindingHealAction, "binding heal");
    
    BUFF_ACTION(CastPrayerOfHealingAction, "prayer of healing");
    AOE_HEAL_ACTION(CastLightwellAction, "lightwell");
    AOE_HEAL_ACTION(CastCircleOfHealingAction, "circle of healing");

    SPELL_ACTION(CastSmiteAction, "smite");
    SPELL_ACTION(CastHolyNovaAction, "holy nova");

    RESS_ACTION(CastResurrectionAction, "resurrection");

    CURE_ACTION(CastCureDiseaseAction, "cure disease");
    CURE_PARTY_ACTION(CastCureDiseaseOnPartyAction, "cure disease", DISPEL_DISEASE);
    CURE_ACTION(CastAbolishDiseaseAction, "abolish disease");
    CURE_PARTY_ACTION(CastAbolishDiseaseOnPartyAction, "abolish disease", DISPEL_DISEASE);

    RANGED_DEBUFF_ACTION(CastHolyFireAction, "holy fire");

    // shadow
    RANGED_DEBUFF_ACTION(CastPowerWordPainAction, "shadow word: pain");
    RANGED_DEBUFF_ENEMY_ACTION(CastPowerWordPainOnAttackerAction, "shadow word: pain");
    SPELL_ACTION(CastMindBlastAction, "mind blast");
    SPELL_ACTION(CastPsychicScreamAction, "psychic scream");
    RANGED_DEBUFF_ACTION(CastMindSootheAction, "mind soothe");
    BUFF_ACTION_U(CastFadeAction, "fade", bot->GetGroup());
    BUFF_ACTION(CastShadowProtectionAction, "shadow protection");
    BUFF_PARTY_ACTION(CastShadowProtectionOnPartyAction, "shadow protection");
    GREATER_BUFF_PARTY_ACTION(CastPrayerOfShadowProtectionAction, "prayer of shadow protection");
    // shadow 2.4.3
    BUFF_ACTION(CastShadowfiendAction, "shadowfiend");
    SPELL_ACTION(CastShadowWordDeathAction, "shadow word: death");

    // shadow talents
    SPELL_ACTION(CastMindFlayAction, "mind flay");
    RANGED_DEBUFF_ACTION(CastVampiricEmbraceAction, "vampiric embrace");
    BUFF_ACTION(CastShadowformAction, "shadowform");
    SPELL_ACTION(CastSilenceAction, "silence");
    ENEMY_HEALER_ACTION(CastSilenceOnEnemyHealerAction, "silence");
    // shadow talents 2.4.3
    RANGED_DEBUFF_ACTION(CastVampiricTouchAction, "vampiric touch");
    RANGED_DEBUFF_ENEMY_ACTION(CastVampiricTouchActionOnAttacker, "vampiric touch on attacker");

    // racials
    RANGED_DEBUFF_ACTION(CastDevouringPlagueAction, "devouring plague");
    BUFF_ACTION(CastTouchOfWeaknessAction, "touch of weakness");
    RANGED_DEBUFF_ACTION(CastHexOfWeaknessAction, "hex of weakness");
    BUFF_ACTION(CastShadowguardAction, "shadowguard");
    HEAL_ACTION(CastDesperatePrayerAction, "desperate prayer");
    SPELL_ACTION_U(CastStarshardsAction, "starshards", (AI_VALUE2(uint8, "mana", "self target") > 50 && AI_VALUE(Unit*, "current target") && AI_VALUE2(float, "distance", "current target") > 15.0f));
    BUFF_ACTION(CastElunesGraceAction, "elune's grace");
    BUFF_ACTION(CastFeedbackAction, "feedback");
    BUFF_ACTION(CastSymbolOfHopeAction, "symbol of hope");
    SPELL_ACTION(CastConsumeMagicAction, "consume magic");
    SNARE_ACTION(CastChastiseAction, "chastise");

    class CastRemoveShadowformAction : public Action 
    {
    public:
        CastRemoveShadowformAction(PlayerbotAI* ai) : Action(ai, "remove shadowform") {}
        virtual bool isUseful() { return ai->HasAura("shadowform", AI_VALUE(Unit*, "self target")); }
        virtual bool isPossible() { return true; }

        virtual bool Execute(Event& event) 
        {
            ai->RemoveAura("shadowform");
            return true;
        }
    };

    class CastPowerInfusionAction : public CastSpellTargetAction
    {
    public:
        CastPowerInfusionAction(PlayerbotAI* ai) : CastSpellTargetAction(ai, "power infusion", "boost targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };

    class CastFearWardAction : public CastSpellTargetAction
    {
    public:
        CastFearWardAction(PlayerbotAI* ai) : CastSpellTargetAction(ai, "fear ward", "buff targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };
}

class UpdatePriestPveStrategiesAction : public UpdateStrategyDependenciesAction
{
public:
    UpdatePriestPveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
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

        strategiesRequired = { "shadow/dps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "shadow pve", strategiesRequired);

        strategiesRequired = { "shadow/dps", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe shadow pve", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure shadow pve", strategiesRequired);

        strategiesRequired = { "shadow/dps", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff shadow pve", strategiesRequired);

        strategiesRequired = { "shadow/dps", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost shadow pve", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc shadow pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc shadow pve", strategiesRequired);

        strategiesRequired = { "discipline" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "discipline pve", strategiesRequired);

        strategiesRequired = { "discipline", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe discipline pve", strategiesRequired);

        strategiesRequired = { "discipline", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure discipline pve", strategiesRequired);

        strategiesRequired = { "discipline", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff discipline pve", strategiesRequired);

        strategiesRequired = { "discipline", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost discipline pve", strategiesRequired);

        strategiesRequired = { "discipline", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc discipline pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc discipline pve", strategiesRequired);

        strategiesRequired = { "offheal" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal pve", strategiesRequired);

        strategiesRequired = { "offdps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offdps pve", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offdps pve", strategiesRequired);
    }
};

class UpdatePriestPvpStrategiesAction : public UpdateStrategyDependenciesAction
{
public:
    UpdatePriestPvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
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

        strategiesRequired = { "shadow/dps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "shadow pvp", strategiesRequired);

        strategiesRequired = { "shadow/dps", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe shadow pvp", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure shadow pvp", strategiesRequired);

        strategiesRequired = { "shadow/dps", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff shadow pvp", strategiesRequired);

        strategiesRequired = { "shadow/dps", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost shadow pvp", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc shadow pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc shadow pvp", strategiesRequired);

        strategiesRequired = { "discipline" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "discipline pvp", strategiesRequired);

        strategiesRequired = { "discipline", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe discipline pvp", strategiesRequired);

        strategiesRequired = { "discipline", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure discipline pvp", strategiesRequired);

        strategiesRequired = { "discipline", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff discipline pvp", strategiesRequired);

        strategiesRequired = { "discipline", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost discipline pvp", strategiesRequired);

        strategiesRequired = { "discipline", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc discipline pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc discipline pvp", strategiesRequired);

        strategiesRequired = { "offheal" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal pvp", strategiesRequired);

        strategiesRequired = { "offdps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offdps pvp", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offdps pvp", strategiesRequired);
    }
};

class UpdatePriestRaidStrategiesAction : public UpdateStrategyDependenciesAction
{
public:
    UpdatePriestRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
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

        strategiesRequired = { "shadow/dps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "shadow raid", strategiesRequired);

        strategiesRequired = { "shadow/dps", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe shadow raid", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure shadow raid", strategiesRequired);

        strategiesRequired = { "shadow/dps", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff shadow raid", strategiesRequired);

        strategiesRequired = { "shadow/dps", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost shadow raid", strategiesRequired);

        strategiesRequired = { "shadow/dps", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc shadow raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc shadow raid", strategiesRequired);

        strategiesRequired = { "discipline" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "discipline raid", strategiesRequired);

        strategiesRequired = { "discipline", "aoe" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe discipline raid", strategiesRequired);

        strategiesRequired = { "discipline", "cure" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cure discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cure discipline raid", strategiesRequired);

        strategiesRequired = { "discipline", "buff" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff discipline raid", strategiesRequired);

        strategiesRequired = { "discipline", "boost" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost discipline raid", strategiesRequired);

        strategiesRequired = { "discipline", "cc" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc discipline raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc discipline raid", strategiesRequired);

        strategiesRequired = { "offheal" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offheal raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offheal raid", strategiesRequired);

        strategiesRequired = { "offdps" };
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "offdps raid", strategiesRequired);
        strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "offdps raid", strategiesRequired);
    }
};