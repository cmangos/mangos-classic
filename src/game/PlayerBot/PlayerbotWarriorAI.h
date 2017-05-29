#ifndef _PlayerbotWarriorAI_H
#define _PlayerbotWarriorAI_H

#include "PlayerbotClassAI.h"

enum
{
    WarriorSpellPreventing,
    WarriorBattle,
    WarriorDefensive,
    WarriorBerserker
};

enum WarriorSpells
{
    BATTLE_SHOUT_1                  = 6673,
    BATTLE_STANCE_1                 = 2457,
    BERSERKER_RAGE_1                = 18499,
    BERSERKER_STANCE_1              = 2458,
    BLOODRAGE_1                     = 2687,
    BLOODTHIRST_1                   = 23881,
    CHALLENGING_SHOUT_1             = 1161,
    CHARGE_1                        = 100,
    CLEAVE_1                        = 845,
    CONCUSSION_BLOW_1               = 12809,
    DEATH_WISH_1                    = 12292,
    DEFENSIVE_STANCE_1              = 71,
    DEMORALIZING_SHOUT_1            = 1160,
    DISARM_1                        = 676,
    EXECUTE_1                       = 5308,
    HAMSTRING_1                     = 1715,
    HEROIC_STRIKE_1                 = 78,
    INTERCEPT_1                     = 20252,
    INTERVENE_1                     = 3411,
    INTIMIDATING_SHOUT_1            = 5246,
    LAST_STAND_1                    = 12975,
    MOCKING_BLOW_1                  = 694,
    MORTAL_STRIKE_1                 = 12294,
    OVERPOWER_1                     = 7384,
    PIERCING_HOWL_1                 = 12323,
    PUMMEL_1                        = 6552,
    RECKLESSNESS_1                  = 1719,
    REND_1                          = 772,
    RETALIATION_1                   = 20230,
    REVENGE_1                       = 6572,
    SHIELD_BASH_1                   = 72,
    SHIELD_BLOCK_1                  = 2565,
    SHIELD_SLAM_1                   = 23922,
    SHIELD_WALL_1                   = 871,
    SHOOT_BOW_1                     = 2480,
    SHOOT_GUN_1                     = 7918,
    SHOOT_XBOW_1                    = 7919,
    SLAM_1                          = 1464,
    SUNDER_ARMOR_1                  = 7386,
    SWEEPING_STRIKES_1              = 12328,
    TAUNT_1                         = 355,
    THUNDER_CLAP_1                  = 6343,
    WHIRLWIND_1                     = 1680

    //Procs
};

//class Player;

class MANGOS_DLL_SPEC PlayerbotWarriorAI : PlayerbotClassAI
{
public:
    PlayerbotWarriorAI(Player * const master, Player * const bot, PlayerbotAI * const ai);
    virtual ~PlayerbotWarriorAI();

    // all combat actions go here
    CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget);
    bool Pull();

    // all non combat actions go here, ex buffs, heals, rezzes
    void DoNonCombatActions();

    //Buff/rebuff shouts
    void CheckShouts();

    // Utility Functions
    bool CanPull();

private:
    CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget);

    // ARMS
    uint32 BATTLE_STANCE,
        CHARGE,
        HEROIC_STRIKE,
        REND,
        THUNDER_CLAP,
        HAMSTRING,
        MOCKING_BLOW,
        RETALIATION,
        SWEEPING_STRIKES,
        MORTAL_STRIKE;

    // PROTECTION
    uint32 DEFENSIVE_STANCE,
        BLOODRAGE,
        SUNDER_ARMOR,
        TAUNT,
        SHIELD_BASH,
        REVENGE,
        SHIELD_BLOCK,
        DISARM,
        SHIELD_WALL,
        SHIELD_SLAM,
        CONCUSSION_BLOW,
        LAST_STAND;

    // FURY
    uint32 BERSERKER_STANCE,
        BATTLE_SHOUT,
        DEMORALIZING_SHOUT,
        OVERPOWER,
        CLEAVE,
        INTIMIDATING_SHOUT,
        EXECUTE,
        CHALLENGING_SHOUT,
        SLAM,
        INTERCEPT,
        DEATH_WISH,
        BERSERKER_RAGE,
        WHIRLWIND,
        PUMMEL,
        BLOODTHIRST,
        RECKLESSNESS,
        PIERCING_HOWL;

    // racial
    uint32 STONEFORM,
        ESCAPE_ARTIST,
        PERCEPTION,
        SHADOWMELD,
        BLOOD_FURY,
        WAR_STOMP,
        BERSERKING,
        WILL_OF_THE_FORSAKEN;

    // general
    uint32 SHOOT,
        SHOOT_BOW,
        SHOOT_GUN,
        SHOOT_XBOW;

    uint32 SpellSequence;
};

#endif
