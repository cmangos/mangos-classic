#ifndef _PlayerbotMageAI_H
#define _PlayerbotMageAI_H

#include "../Base/PlayerbotClassAI.h"

enum
{
    SPELL_FROST,
    SPELL_FIRE,
    SPELL_ARCANE
};

enum ManaGemIds
{
    MANA_RUBY_DISPLAYID             = 7045,
    MANA_CITRINE_DISPLAYID          = 6496,
    MANA_AGATE_DISPLAYID            = 6851,
    MANA_JADE_DISPLAYID             = 7393
};

enum MageSpells
{
    AMPLIFY_MAGIC_1                 = 1008,
    ARCANE_BRILLIANCE_1             = 23028,
    ARCANE_EXPLOSION_1              = 1449,
    ARCANE_INTELLECT_1              = 1459,
    ARCANE_MISSILES_1               = 5143,
    ARCANE_POWER_1                  = 12042,
    BLAST_WAVE_1                    = 11113,
    BLINK_1                         = 1953,
    BLIZZARD_1                      = 10,
    CLEARCASTING_1                  = 12536,
    COLD_SNAP_1                     = 12472,
    COMBUSTION_1                    = 11129,
    CONE_OF_COLD_1                  = 120,
    CONJURE_FOOD_1                  = 587,
    CONJURE_MANA_GEM_1              = 759,
    CONJURE_WATER_1                 = 5504,
    COUNTERSPELL_1                  = 2139,
    DAMPEN_MAGIC_1                  = 604,
    EVOCATION_1                     = 12051,
    FIRE_BLAST_1                    = 2136,
    FIRE_WARD_1                     = 543,
    FIREBALL_1                      = 133,
    FLAMESTRIKE_1                   = 2120,
    FROST_ARMOR_1                   = 168,
    FROST_NOVA_1                    = 122,
    FROST_WARD_1                    = 6143,
    FROSTBOLT_1                     = 116,
    ICE_ARMOR_1                     = 7302,
    ICE_BARRIER_1                   = 11426,
    ICE_BLOCK_1                     = 11958,
    MAGE_ARMOR_1                    = 6117,
    MANA_SHIELD_1                   = 1463,
    POLYMORPH_1                     = 118,
    PRESENCE_OF_MIND_1              = 12043,
    PYROBLAST_1                     = 11366,
    REMOVE_CURSE_MAGE_1             = 475,
    SCORCH_1                        = 2948,
    SHOOT_2                         = 5019,
    SLOW_FALL_1                     = 130
};

enum MageTalents
{
    IMPROVED_SCORCH_1               = 11095,
    IMPROVED_SCORCH_2               = 12872,
    IMPROVED_SCORCH_3               = 12873
};

static const uint32 uiImprovedScorch[3] =
{
    IMPROVED_SCORCH_1, IMPROVED_SCORCH_2, IMPROVED_SCORCH_3
};

//class Player;

class MANGOS_DLL_SPEC PlayerbotMageAI : PlayerbotClassAI
{
public:
    PlayerbotMageAI(Player * const master, Player * const bot, PlayerbotAI * const ai);
    virtual ~PlayerbotMageAI();

    // all combat actions go here
    CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget);
    uint32 Neutralize(uint8 creatureType);

    // all non combat actions go here, ex buffs, heals, rezzes
    void DoNonCombatActions();

private:
    CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget);
    CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget);
    CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget);
    Item* FindManaGem() const;

    CombatManeuverReturns CastSpell(uint32 nextAction, Unit *pTarget = nullptr) { return CastSpellWand(nextAction, pTarget, SHOOT); }

    static bool BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit *target);

    uint8 CheckFrostCooldowns();

    // ARCANE
    uint32 ARCANE_MISSILES,
           ARCANE_EXPLOSION,
           COUNTERSPELL,
           EVOCATION,
           POLYMORPH,
           PRESENCE_OF_MIND,
           ARCANE_POWER;

    // RANGED
    uint32 SHOOT;

    // FIRE
    uint32 FIREBALL,
           FIRE_BLAST,
           FLAMESTRIKE,
           SCORCH,
           FIRE_VULNERABILITY,
           IMPROVED_SCORCH,
           PYROBLAST,
           BLAST_WAVE,
           COMBUSTION,
           FIRE_WARD;

    // FROST
    uint32 FROSTBOLT,
           FROST_NOVA,
           BLIZZARD,
           CONE_OF_COLD,
           ICE_BARRIER,
           FROST_WARD,
           ICE_BLOCK,
           COLD_SNAP;

    // buffs
    uint32 FROST_ARMOR,
           ICE_ARMOR,
           MAGE_ARMOR,
           ARCANE_INTELLECT,
           ARCANE_BRILLIANCE,
           MANA_SHIELD,
           DAMPEN_MAGIC,
           AMPLIFY_MAGIC,
           MAGE_REMOVE_CURSE;

    // racial
    uint32 STONEFORM,
           ESCAPE_ARTIST,
           PERCEPTION,
           SHADOWMELD,
           BLOOD_FURY,
           WAR_STOMP,
           BERSERKING,
           WILL_OF_THE_FORSAKEN;

    uint32 CONJURE_WATER,
           CONJURE_FOOD,
           CONJURE_MANA_GEM;
};

#endif
