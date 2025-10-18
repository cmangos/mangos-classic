#pragma once
#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
    // stances
    BUFF_ACTION(CastBattleStanceAction, "battle stance");
    BUFF_ACTION(CastDefensiveStanceAction, "defensive stance");
    BUFF_ACTION(CastBerserkerStanceAction, "berserker stance");

    // shouts
    MELEE_ACTION_U(CastBattleShoutTauntAction, "battle shout", CastSpellAction::isUseful()); // useful to rebuff
    MELEE_DEBUFF_ACTION_R(CastDemoralizingShoutAction, "demoralizing shout", 8.0f); // low range debuff
    MELEE_ACTION(CastChallengingShoutAction, "challenging shout");
    MELEE_DEBUFF_ACTION_R(CastIntimidatingShoutAction, "intimidating shout", 8.0f);
    // shouts 2.4.3
    BUFF_ACTION(CastCommandingShoutAction, "commanding shout");

    // arms
    MELEE_ACTION(CastHeroicStrikeAction, "heroic strike");
    REACH_ACTION(CastChargeAction, "charge", 8.0f);
    MELEE_DEBUFF_ACTION(CastRendAction, "rend");
    MELEE_DEBUFF_ENEMY_ACTION(CastRendOnAttackerAction, "rend");
    MELEE_DEBUFF_ACTION_R(CastThunderClapAction, "thunder clap", 8.0f);
    SNARE_ACTION(CastThunderClapSnareAction, "thunder clap");
    SNARE_ACTION(CastHamstringAction, "hamstring");
    MELEE_ACTION(CastOverpowerAction, "overpower");
    MELEE_ACTION(CastMockingBlowAction, "mocking blow");
    BUFF_ACTION(CastRetaliationAction, "retaliation");
    // arms 3.3.5
    SPELL_ACTION(CastHeroicThrowAction, "heroic throw");
    SNARE_ACTION(CastHeroicThrowSnareAction, "heroic throw");
    RANGED_DEBUFF_ACTION(CastShatteringThrowAction, "shattering throw");
    
    // arms talents
    MELEE_ACTION(CastMortalStrikeAction, "mortal strike");
    BUFF_ACTION(CastSweepingStrikesAction, "sweeping strikes");
    // arms talents 3.3.5
    BUFF_ACTION(CastBladestormAction, "bladestorm");

    // fury
    MELEE_ACTION(CastCleaveAction, "cleave");
    MELEE_ACTION(CastExecuteAction, "execute");
    REACH_ACTION(CastInterceptAction, "intercept", 8.0f);
    ENEMY_HEALER_ACTION(CastInterceptOnEnemyHealerAction, "intercept");
    SNARE_ACTION(CastInterceptOnSnareTargetAction, "intercept");
    MELEE_ACTION(CastSlamAction, "slam");
    BUFF_ACTION(CastBerserkerRageAction, "berserker rage");
    MELEE_ACTION(CastWhirlwindAction, "whirlwind");
    MELEE_ACTION(CastPummelAction, "pummel");
    ENEMY_HEALER_ACTION(CastPummelOnEnemyHealerAction, "pummel");
    BUFF_ACTION(CastRecklessnessAction, "recklessness");
    // fury 2.4.3
    MELEE_ACTION(CastVictoryRushAction, "victory rush");
    // fury 3.3.5
    BUFF_ACTION(CastEnragedRegenerationAction, "enraged regeneration");
    BUFF_ACTION(CastHeroicFuryAction, "heroic fury");

    // fury talents
    BUFF_ACTION(CastDeathWishAction, "death wish");
    MELEE_ACTION(CastBloodthirstAction, "bloodthirst");
    MELEE_DEBUFF_ACTION_R(CastPiercingHowlAction, "piercing howl", 8.0f);
    // fury talents 2.4.3
    BUFF_ACTION(CastRampageAction, "rampage");

    // protection
    MELEE_ACTION_U(CastTauntAction, "taunt", GetTarget() && GetTarget()->GetVictim() && GetTarget()->GetVictim() != bot);
    SNARE_ACTION(CastTauntOnSnareTargetAction, "taunt");
    BUFF_ACTION(CastBloodrageAction, "bloodrage");
    MELEE_ACTION(CastShieldBashAction, "shield bash");
    ENEMY_HEALER_ACTION(CastShieldBashOnEnemyHealerAction, "shield bash");
    MELEE_ACTION(CastRevengeAction, "revenge");
    BUFF_ACTION(CastShieldBlockAction, "shield block");
    MELEE_DEBUFF_ACTION_U(CastDisarmAction, "disarm", GetTarget() && GetTarget()->IsPlayer() ? !ai->IsRanged((Player*)GetTarget()) : CastMeleeDebuffSpellAction::isUseful());
    MELEE_DEBUFF_ENEMY_ACTION(CastDisarmOnAttackerAction, "disarm");
    BUFF_ACTION(CastShieldWallAction, "shield wall");
    // protection 2.4.3
    PROTECT_ACTION(CastInterveneAction, "intervene");
    BUFF_ACTION(CastSpellReflectionAction, "spell reflection");

    // protection talents
    BUFF_ACTION(CastLastStandAction, "last stand");
    MELEE_ACTION(CastShieldSlamAction, "shield slam");
    MELEE_ACTION(CastConcussionBlowAction, "concussion blow");
    // protection talents 2.4.3
    MELEE_ACTION(CastDevastateAction, "devastate");
    // protection talents 3.3.5
    MELEE_DEBUFF_ACTION_R(CastShockwaveAction, "shockwave", 8.0f);
    SNARE_ACTION(CastShockwaveSnareAction, "shockwave");

    class CastBattleShoutAction : public CastBuffSpellAction
    {
    public:
        CastBattleShoutAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "battle shout") {}

        bool isUseful() override
        {
            static const std::vector<uint32> battleShoutIds = {6673, 5242, 6192, 11549, 11550, 11551, 25289, 2048, 47436};

            for (uint32 id : battleShoutIds)
            {
                if (bot->HasAura(id))
                    return false;
            }

            return CastSpellAction::isUseful();
        }
    };

    class CastSunderArmorAction : public CastMeleeDebuffSpellAction
    {
    public:
        CastSunderArmorAction(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, "sunder armor")
        {
            range = ATTACK_DISTANCE;
        }

        virtual bool isUseful() { return GetTarget() && !ai->HasAura("sunder armor", GetTarget(), true); }
    };

    class UpdateWarriorPveStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarriorPveStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pve strats")
        {
            std::vector<std::string> strategiesRequired = { "arms" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "arms pve", strategiesRequired);

            strategiesRequired = { "arms", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe arms pve", strategiesRequired);

            strategiesRequired = { "arms", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff arms pve", strategiesRequired);

            strategiesRequired = { "arms", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost arms pve", strategiesRequired);

            strategiesRequired = { "arms", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc arms pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc arms pve", strategiesRequired);

            strategiesRequired = { "fury" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "fury pve", strategiesRequired);

            strategiesRequired = { "fury", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe fury pve", strategiesRequired);

            strategiesRequired = { "fury", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff fury pve", strategiesRequired);

            strategiesRequired = { "fury", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost fury pve", strategiesRequired);

            strategiesRequired = { "fury", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc fury pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc fury pve", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection pve", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection pve", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection pve", strategiesRequired);
        }
    };

    class UpdateWarriorPvpStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarriorPvpStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update pvp strats")
        {
            std::vector<std::string> strategiesRequired = { "arms" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "arms pvp", strategiesRequired);

            strategiesRequired = { "arms", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe arms pvp", strategiesRequired);

            strategiesRequired = { "arms", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff arms pvp", strategiesRequired);

            strategiesRequired = { "arms", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost arms pvp", strategiesRequired);

            strategiesRequired = { "arms", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc arms pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc arms pvp", strategiesRequired);

            strategiesRequired = { "fury" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "fury pvp", strategiesRequired);

            strategiesRequired = { "fury", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe fury pvp", strategiesRequired);

            strategiesRequired = { "fury", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff fury pvp", strategiesRequired);

            strategiesRequired = { "fury", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost fury pvp", strategiesRequired);

            strategiesRequired = { "fury", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc fury pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc fury pvp", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection pvp", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection pvp", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection pvp", strategiesRequired);
        }
    };

    class UpdateWarriorRaidStrategiesAction : public UpdateStrategyDependenciesAction
    {
    public:
        UpdateWarriorRaidStrategiesAction(PlayerbotAI* ai) : UpdateStrategyDependenciesAction(ai, "update raid strats")
        {
            std::vector<std::string> strategiesRequired = { "arms" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "arms raid", strategiesRequired);

            strategiesRequired = { "arms", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe arms raid", strategiesRequired);

            strategiesRequired = { "arms", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff arms raid", strategiesRequired);

            strategiesRequired = { "arms", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost arms raid", strategiesRequired);

            strategiesRequired = { "arms", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc arms raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc arms raid", strategiesRequired);

            strategiesRequired = { "fury" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "fury raid", strategiesRequired);

            strategiesRequired = { "fury", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe fury raid", strategiesRequired);

            strategiesRequired = { "fury", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff fury raid", strategiesRequired);

            strategiesRequired = { "fury", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost fury raid", strategiesRequired);

            strategiesRequired = { "fury", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc fury raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc fury raid", strategiesRequired);

            strategiesRequired = { "protection/tank" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_DEAD, "protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_REACTION, "protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "aoe" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "aoe protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "aoe protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "buff" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "buff protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "buff protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "boost" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "boost protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "boost protection raid", strategiesRequired);

            strategiesRequired = { "protection/tank", "cc" };
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_COMBAT, "cc protection raid", strategiesRequired);
            strategiesToUpdate.emplace_back(BotState::BOT_STATE_NON_COMBAT, "cc protection raid", strategiesRequired);
        }
    };
}
