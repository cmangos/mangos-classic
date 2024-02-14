#ifndef _PlayerbotPaladinAI_H
#define _PlayerbotPaladinAI_H

#include "../Base/PlayerbotClassAI.h"

enum
{
    Combat,
    Healing
};

enum PaladinSpells
{
    BLESSING_OF_KINGS_1             = 20217,
    BLESSING_OF_MIGHT_1             = 19740,
    BLESSING_OF_SANCTUARY_1         = 20911,
    BLESSING_OF_WISDOM_1            = 19742,
    CLEANSE_1                       = 4987,
    CONCENTRATION_AURA_1            = 19746,
    CONSECRATION_1                  = 26573,
    DEVOTION_AURA_1                 = 465,
    DIVINE_FAVOR_1                  = 20216,
    DIVINE_INTERVENTION_1           = 19752,
    DIVINE_PROTECTION_1             = 498,
    DIVINE_SHIELD_1                 = 642,
    EXORCISM_1                      = 879,
    FIRE_RESISTANCE_AURA_1          = 19891,
    FLASH_OF_LIGHT_1                = 19750,
    FROST_RESISTANCE_AURA_1         = 19888,
    GREATER_BLESSING_OF_KINGS_1     = 25898,
    GREATER_BLESSING_OF_MIGHT_1     = 25782,
    GREATER_BLESSING_OF_SANCTUARY_1 = 25899,
    GREATER_BLESSING_OF_WISDOM_1    = 25894,
    HAMMER_OF_JUSTICE_1             = 853,
    HAMMER_OF_WRATH_1               = 24275,
    HAND_OF_FREEDOM_1               = 1044,
    BLESSING_OF_PROTECTION_1        = 1022,
    BLESSING_OF_SACRIFICE_1         = 6940,
    HAND_OF_SALVATION_1             = 1038,
    HOLY_LIGHT_1                    = 635,
    HOLY_SHIELD_1                   = 20925,
    HOLY_SHOCK_1                    = 20473,
    HOLY_WRATH_1                    = 2812,
    JUDGEMENT_1                     = 20271,
    LAY_ON_HANDS_1                  = 633,
    PURIFY_1                        = 1152,
    REDEMPTION_1                    = 7328,
    REPENTANCE_1                    = 20066,
    RETRIBUTION_AURA_1              = 7294,
    RIGHTEOUS_FURY_1                = 25780,
    SEAL_OF_COMMAND_1               = 20375,
    SEAL_OF_JUSTICE_1               = 20164,
    SEAL_OF_LIGHT_1                 = 20165,
    SEAL_OF_RIGHTEOUSNESS_1         = 21084,
    SEAL_OF_WISDOM_1                = 20166,
    SEAL_OF_THE_CRUSADER_1          = 21082,
    SENSE_UNDEAD_1                  = 5502,
    SHADOW_RESISTANCE_AURA_1        = 19876,
    TURN_EVIL_1                     = 10326,

    // Judgement auras on target
    JUDGEMENT_OF_WISDOM             = 20355, // rank 2: 20354, rank 1: 20186
    JUDGEMENT_OF_JUSTICE            = 20184,
    JUDGEMENT_OF_THE_CRUSADER       = 20303  // rank 5: 20302, rank 4: 20301, rank 3: 20300, rank 2: 20188, rank 1: 21183
};
//class Player;

class PlayerbotPaladinAI : PlayerbotClassAI
{
    public:
        PlayerbotPaladinAI(Player& master, Player& bot, PlayerbotAI& ai);
        virtual ~PlayerbotPaladinAI();

        // all combat actions go here
        CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget) override;
        bool CanPull() override;
        bool Pull() override;

        // all non combat actions go here, ex buffs, heals, rezzes
        void DoNonCombatActions() override;

        // Utility Functions
        bool CastHoTOnTank();

    private:
        CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget) override;

        // Heals the target based off its hps
        CombatManeuverReturns HealPlayer(Player* target) override;
        // Resurrects the target
        CombatManeuverReturns ResurrectPlayer(Player* target) override;
        // Dispel disease or negative magic effects from an internally selected target
        CombatManeuverReturns DispelPlayer(Player* target = nullptr) override;

        //Changes aura according to spec/orders
        void CheckAuras();
        //Changes Seal according to spec
        bool CheckSealAndJudgement(Unit* target);
        uint32 m_CurrentSeal;
        uint32 m_CurrentJudgement;

        static bool BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target);

        // make this public so the static function can access it. Either that or make an accessor function for each
    public:
        // Retribution
        uint32 RETRIBUTION_AURA,
               SEAL_OF_COMMAND,
               GREATER_BLESSING_OF_WISDOM,
               GREATER_BLESSING_OF_MIGHT,
               BLESSING_OF_WISDOM,
               BLESSING_OF_MIGHT,
               HAMMER_OF_JUSTICE,
               RIGHTEOUS_FURY,
               JUDGEMENT;

        // Holy
        uint32 FLASH_OF_LIGHT,
               HOLY_LIGHT,
               DIVINE_SHIELD,
               HAMMER_OF_WRATH,
               CONSECRATION,
               CONCENTRATION_AURA,
               DIVINE_FAVOR,
               HOLY_SHOCK,
               HOLY_WRATH,
               LAY_ON_HANDS,
               EXORCISM,
               REDEMPTION,
               SEAL_OF_JUSTICE,
               SEAL_OF_LIGHT,
               SEAL_OF_RIGHTEOUSNESS,
               SEAL_OF_WISDOM,
               SEAL_OF_THE_CRUSADER,
               PURIFY,
               CLEANSE;

        // Protection
        uint32 GREATER_BLESSING_OF_KINGS,
               BLESSING_OF_KINGS,
               BLESSING_OF_PROTECTION,
               SHADOW_RESISTANCE_AURA,
               DEVOTION_AURA,
               FIRE_RESISTANCE_AURA,
               FROST_RESISTANCE_AURA,
               DEFENSIVE_STANCE,
               BERSERKER_STANCE,
               BATTLE_STANCE,
               DIVINE_SACRIFICE,
               DIVINE_PROTECTION,
               DIVINE_INTERVENTION,
               HOLY_SHIELD,
               AVENGERS_SHIELD,
               RIGHTEOUS_DEFENSE,
               BLESSING_OF_SANCTUARY,
               GREATER_BLESSING_OF_SANCTUARY,
               BLESSING_OF_SACRIFICE,
               SHIELD_OF_RIGHTEOUSNESS,
               HAND_OF_RECKONING,
               HAMMER_OF_THE_RIGHTEOUS;

        // cannot be protected
        uint32 FORBEARANCE;

        // racial
        uint32 STONEFORM,
               ESCAPE_ARTIST,
               PERCEPTION,
               SHADOWMELD,
               BLOOD_FURY,
               WAR_STOMP,
               BERSERKING,
               WILL_OF_THE_FORSAKEN;

        //Non-Stacking buffs
        uint32 PRAYER_OF_SHADOW_PROTECTION;

    private:
        uint32 SpellSequence, CombatCounter, HealCounter;
};

#endif
