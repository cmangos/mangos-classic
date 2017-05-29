
#ifndef _PlayerbotRogueAI_H
#define _PlayerbotRogueAI_H

#include "../Base/PlayerbotClassAI.h"

enum
{
    RogueCombat,
    RogueSpellPreventing,
    RogueThreat,
    RogueStealth
};

enum RoguePoisonDisplayId
{
    DEADLY_POISON_DISPLAYID         = 13707,
    CRIPPLING_POISON_DISPLAYID      = 13708,
    MIND_NUMBLING_POISON_DISPLAYID  = 13709,
    INSTANT_POISON_DISPLAYID        = 13710,
    WOUND_POISON_DISPLAYID          = 13708
};

enum RogueSpells
{
    ADRENALINE_RUSH_1               = 13750,
    AMBUSH_1                        = 8676,
    BACKSTAB_1                      = 53,
    BLADE_FLURRY_1                  = 13877,
    BLIND_1                         = 2094,
    CHEAP_SHOT_1                    = 1833,
    COLD_BLOOD_1                    = 14177,
    DISARM_TRAP_1                   = 1842,
    DISTRACT_1                      = 1725,
    EVASION_1                       = 5277,
    EVISCERATE_1                    = 2098,
    EXPOSE_ARMOR_1                  = 8647,
    FEINT_1                         = 1966,
    GARROTE_1                       = 703,
    GHOSTLY_STRIKE_1                = 14278,
    GOUGE_1                         = 1776,
    HEMORRHAGE_1                    = 16511,
    KICK_1                          = 1766,
    KIDNEY_SHOT_1                   = 408,
    PICK_LOCK_1                     = 1804,
    PICK_POCKET_1                   = 921,
    PREMEDITATION_1                 = 14183,
    PREPARATION_1                   = 14185,
    RIPOSTE_1                       = 14251,
    RUPTURE_1                       = 1943,
    SAP_1                           = 6770,
    SINISTER_STRIKE_1               = 1752,
    SLICE_AND_DICE_1                = 5171,
    SPRINT_1                        = 2983,
    STEALTH_1                       = 1784,
    VANISH_1                        = 1856
};
//class Player;

class MANGOS_DLL_SPEC PlayerbotRogueAI : PlayerbotClassAI
{
public:
    PlayerbotRogueAI(Player * const master, Player * const bot, PlayerbotAI * const ai);
    virtual ~PlayerbotRogueAI();

    // all combat actions go here
    CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget);

    // all non combat actions go here, ex buffs, heals, rezzes
    void DoNonCombatActions();

private:
    CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget);
    Item* FindPoison() const;

    // COMBAT
    uint32 ADRENALINE_RUSH,
           SINISTER_STRIKE,
           BACKSTAB,
           GOUGE,
           EVASION,
           SPRINT,
           KICK,
           FEINT;

    // SUBTLETY
    uint32 STEALTH,
           VANISH,
           HEMORRHAGE,
           BLIND,
           PICK_POCKET,
           CRIPPLING_POISON,
           DEADLY_POISON,
           MIND_NUMBING_POISON,
           GHOSTLY_STRIKE,
           DISTRACT,
           PREPARATION,
           PREMEDITATION;

    // ASSASSINATION
    uint32 COLD_BLOOD,
           EVISCERATE,
           SLICE_DICE,
           GARROTE,
           EXPOSE_ARMOR,
           AMBUSH,
           RUPTURE,
           CHEAP_SHOT,
           KIDNEY_SHOT;

    // racial
    uint32 STONEFORM,
           ESCAPE_ARTIST,
           PERCEPTION,
           SHADOWMELD,
           BLOOD_FURY,
           WAR_STOMP,
           BERSERKING,
           WILL_OF_THE_FORSAKEN;

    uint32 SpellSequence, LastSpellCombat, LastSpellSubtlety, LastSpellAssassination, Aura;
};

#endif
