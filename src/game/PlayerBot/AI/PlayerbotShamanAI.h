/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PLAYERBOTSHAMANAI_H
#define _PLAYERBOTSHAMANAI_H

#include "../Base/PlayerbotClassAI.h"

enum
{
    SPELL_ENHANCEMENT,
    SPELL_RESTORATION,
    SPELL_ELEMENTAL
};

enum
{
    ANCESTRAL_SPIRIT_1              = 2008,
    ASTRAL_RECALL_1                 = 556,
    BLOODLUST_1                     = 2825,
    CHAIN_HEAL_1                    = 1064,
    CHAIN_LIGHTNING_1               = 421,
    CLEANSING_TOTEM_1               = 8170,
    CURE_DISEASE_SHAMAN_1           = 2870,
    CURE_POISON_SHAMAN_1            = 526,
    EARTH_ELEMENTAL_TOTEM_1         = 2062,
    EARTH_SHIELD_1                  = 974,
    EARTH_SHOCK_1                   = 8042,
    EARTHBIND_TOTEM_1               = 2484,
    ELEMENTAL_MASTERY_1             = 16166,
    FIRE_ELEMENTAL_TOTEM_1          = 2894,
    FIRE_NOVA_1                     = 1535,
    FIRE_RESISTANCE_TOTEM_1         = 8184,
    FLAME_SHOCK_1                   = 8050,
    FLAMETONGUE_TOTEM_1             = 8227,
    FLAMETONGUE_WEAPON_1            = 8024,
    FROST_RESISTANCE_TOTEM_1        = 8181,
    FROST_SHOCK_1                   = 8056,
    FROSTBRAND_WEAPON_1             = 8033,
    GHOST_WOLF_1                    = 2645,
    GROUNDING_TOTEM_1               = 8177,
    HEALING_STREAM_TOTEM_1          = 5394,
    HEALING_WAVE_1                  = 331,
    LESSER_HEALING_WAVE_1           = 8004,
    LIGHTNING_BOLT_1                = 403,
    LIGHTNING_SHIELD_1              = 324,
    MAGMA_TOTEM_1                   = 8190,
    MANA_SPRING_TOTEM_1             = 5675,
    MANA_TIDE_TOTEM_1               = 16190,
    NATURE_RESISTANCE_TOTEM_1       = 10595,
    NATURES_SWIFTNESS_SHAMAN_1      = 16188,
    PURGE_1                         = 370,
    ROCKBITER_WEAPON_1              = 8017,
    SEARING_TOTEM_1                 = 3599,
    SENTRY_TOTEM_1                  = 6495,
    STONECLAW_TOTEM_1               = 5730,
    STONESKIN_TOTEM_1               = 8071,
    STORMSTRIKE_1                   = 17364,
    STRENGTH_OF_EARTH_TOTEM_1       = 8075,
    TREMOR_TOTEM_1                  = 8143,
    WATER_BREATHING_1               = 131,
    WATER_WALKING_1                 = 546,
    WINDFURY_TOTEM_1                = 8512,
    WINDFURY_WEAPON_1               = 8232,
    WRATH_OF_AIR_TOTEM_1            = 3738,

    //Totem Buffs
    STRENGTH_OF_EARTH_EFFECT_1      = 8076,
    FLAMETONGUE_EFFECT_1            = 8026,
    MAGMA_TOTEM_EFFECT_1            = 8188,
    STONECLAW_EFFECT_1              = 5728,
    FIRE_RESISTANCE_EFFECT_1        = 8185,
    FROST_RESISTANCE_EFFECT_1       = 8182,
    GROUDNING_EFFECT_1              = 8178,
    NATURE_RESISTANCE_EFFECT_1      = 10596,
    STONESKIN_EFFECT_1              = 8072,
    WINDFURY_EFFECT_1               = 8515,
    WRATH_OF_AIR_EFFECT_1           = 2895,
    CLEANSING_TOTEM_EFFECT_1        = 8172,
    MANA_SPRING_EFFECT_1            = 5677,
    TREMOR_TOTEM_EFFECT_1           = 8145,
    EARTHBIND_EFFECT_1              = 6474,
    // FIRE_ELEMENTAL_TOTEM uses spell effect index 2
    // SEARING_TOTEM uses spell effect index 0
    // EARTH_ELEMENTAL_TOTEM uses spell effect indexes 1 and 2

    //Spec buffs
};
//class Player;

class PlayerbotShamanAI : PlayerbotClassAI
{
    public:
        PlayerbotShamanAI(Player& master, Player& bot, PlayerbotAI& ai);
        virtual ~PlayerbotShamanAI();

        // all combat actions go here
        CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget) override;

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
        CombatManeuverReturns DispelPlayer(Player* target = nullptr);

        void DropTotems();
        void CheckShields();
        void UseCooldowns();

        // ENHANCEMENT
        uint32 ROCKBITER_WEAPON,
               STONESKIN_TOTEM,
               LIGHTNING_SHIELD,
               FLAMETONGUE_WEAPON,
               STRENGTH_OF_EARTH_TOTEM,
               FOCUSED,
               FROSTBRAND_WEAPON,
               FROST_RESISTANCE_TOTEM,
               FLAMETONGUE_TOTEM,
               FIRE_RESISTANCE_TOTEM,
               WINDFURY_WEAPON,
               GROUNDING_TOTEM,
               NATURE_RESISTANCE_TOTEM,
               WIND_FURY_TOTEM,
               STORMSTRIKE,
               WRATH_OF_AIR_TOTEM,
               EARTH_ELEMENTAL_TOTEM,
               BLOODLUST;

        // RESTORATION
        uint32 HEALING_WAVE,
               LESSER_HEALING_WAVE,
               ANCESTRAL_SPIRIT,
               TREMOR_TOTEM,
               HEALING_STREAM_TOTEM,
               MANA_SPRING_TOTEM,
               CHAIN_HEAL,
               MANA_TIDE_TOTEM,
               EARTH_SHIELD,
               CURE_DISEASE_SHAMAN,
               CURE_POISON_SHAMAN,
               NATURES_SWIFTNESS_SHAMAN;

        // ELEMENTAL
        uint32 LIGHTNING_BOLT,
               EARTH_SHOCK,
               STONECLAW_TOTEM,
               FLAME_SHOCK,
               SEARING_TOTEM,
               PURGE,
               FIRE_NOVA_TOTEM,
               FROST_SHOCK,
               MAGMA_TOTEM,
               CHAIN_LIGHTNING,
               FIRE_ELEMENTAL_TOTEM,
               EARTHBIND_TOTEM,
               ELEMENTAL_MASTERY;

        // racial
        uint32 STONEFORM,
               ESCAPE_ARTIST,
               EVERY_MAN_FOR_HIMSELF,
               SHADOWMELD,
               BLOOD_FURY,
               WAR_STOMP,
               BERSERKING,
               WILL_OF_THE_FORSAKEN;

        // totem buffs
        uint32 STRENGTH_OF_EARTH_EFFECT,
               FLAMETONGUE_EFFECT,
               MAGMA_TOTEM_EFFECT,
               STONECLAW_EFFECT,
               FIRE_RESISTANCE_EFFECT,
               FROST_RESISTANCE_EFFECT,
               GROUDNING_EFFECT,
               NATURE_RESISTANCE_EFFECT,
               STONESKIN_EFFECT,
               WINDFURY_EFFECT,
               WRATH_OF_AIR_EFFECT,
               CLEANSING_TOTEM_EFFECT,
               HEALING_STREAM_EFFECT,
               MANA_SPRING_EFFECT,
               TREMOR_TOTEM_EFFECT,
               EARTHBIND_EFFECT;

        uint32 SpellSequence, LastSpellEnhancement, LastSpellRestoration, LastSpellElemental;
};

#endif
