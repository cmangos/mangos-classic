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

#ifndef _PLAYERHUNTERAI_H
#define _PLAYERHUNTERAI_H

#include "../Base/PlayerbotClassAI.h"

enum
{
    SPELL_HUNTER
};

enum HunterSpells
{
    ARCANE_SHOT_1                   = 3044,
    ASPECT_OF_THE_BEAST_1           = 13161,
    ASPECT_OF_THE_CHEETAH_1         = 5118,
    ASPECT_OF_THE_HAWK_1            = 13165,
    ASPECT_OF_THE_MONKEY_1          = 13163,
    ASPECT_OF_THE_PACK_1            = 13159,
    ASPECT_OF_THE_WILD_1            = 20043,
    AUTO_SHOT_1                     = 75,
    BEAST_LORE_1                    = 1462,
    CALL_PET_1                      = 883,
    CONCUSSIVE_SHOT_1               = 5116,
    DETERRENCE_1                    = 19263,
    DISENGAGE_1                     = 781,
    DISMISS_PET_1                   = 2641,
    DISTRACTING_SHOT_1              = 20736,
    EAGLE_EYE_1                     = 6197,
    EXPLOSIVE_TRAP_1                = 13813,
    EYES_OF_THE_BEAST_1             = 1002,
    FEED_PET_1                      = 6991,
    FEIGN_DEATH_1                   = 5384,
    FLARE_1                         = 1543,
    FREEZING_TRAP_1                 = 1499,
    FROST_TRAP_1                    = 13809,
    HUNTERS_MARK_1                  = 1130,
    IMMOLATION_TRAP_1               = 13795,
    MEND_PET_1                      = 136,
    MONGOOSE_BITE_1                 = 1495,
    MULTISHOT_1                     = 2643,
    RAPID_FIRE_1                    = 3045,
    RAPTOR_STRIKE_1                 = 2973,
    REVIVE_PET_1                    = 982,
    SCARE_BEAST_1                   = 1513,
    SCORPID_STING_1                 = 3043,
    SERPENT_STING_1                 = 1978,
    TAME_BEAST_1                    = 1515,
    TRACK_BEASTS_1                  = 1494,
    TRACK_DEMONS_1                  = 19878,
    TRACK_DRAGONKIN_1               = 19879,
    TRACK_ELEMENTALS_1              = 19880,
    TRACK_GIANTS_1                  = 19882,
    TRACK_HIDDEN_1                  = 19885,
    TRACK_HUMANOIDS_1               = 19883,
    TRACK_UNDEAD_1                  = 19884,
    TRANQUILIZING_SHOT_1            = 19801,
    VIPER_STING_1                   = 3034,
    VOLLEY_1                        = 1510,
    WING_CLIP_1                     = 2974,
    AIMED_SHOT_1                    = 19434,
    BESTIAL_WRATH_1                 = 19574,
    BLACK_ARROW_1                   = 3674,
    COUNTERATTACK_1                 = 19306,
    INTIMIDATION_1                  = 19577,
    READINESS_1                     = 23989,
    SCATTER_SHOT_1                  = 19503,
    TRUESHOT_AURA_1                 = 19506,
    WYVERN_STING_1                  = 19386
};

//class Player;

class PlayerbotHunterAI : PlayerbotClassAI
{
    public:
        PlayerbotHunterAI(Player& master, Player& bot, PlayerbotAI& ai);
        virtual ~PlayerbotHunterAI();
        static bool HasPet(Player* bot);

        // all combat actions go here
        CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget) override;

        // all non combat actions go here, ex buffs, heals, rezzes
        void DoNonCombatActions() override;

        // buff a specific player, usually a real PC who is not in group
        //void BuffPlayer(Player *target);

    private:
        CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget) override;

        // Hunter
        bool IsTargetEnraged(Unit* pTarget);
        bool m_petSummonFailed;
        bool m_rangedCombat;
        bool m_has_ammo;

        uint32 PET_SUMMON,
               PET_DISMISS,
               PET_REVIVE,
               PET_MEND,
               PET_FEED,
               BESTIAL_WRATH,
               BAD_ATTITUDE,
               SONIC_BLAST,
               DEMORALIZING_SCREECH,
               INTIMIDATION;

        uint32 AUTO_SHOT,
               HUNTERS_MARK,
               ARCANE_SHOT,
               CONCUSSIVE_SHOT,
               DISTRACTING_SHOT,
               MULTI_SHOT,
               EXPLOSIVE_SHOT,
               SERPENT_STING,
               SCORPID_STING,
               VIPER_STING,
               WYVERN_STING,
               AIMED_SHOT,
               VOLLEY,
               BLACK_ARROW,
               TRANQUILIZING_SHOT;

        uint32 RAPTOR_STRIKE,
               WING_CLIP,
               MONGOOSE_BITE,
               DISENGAGE,
               DETERRENCE,
               FEIGN_DEATH;

        uint32 FREEZING_TRAP,
               IMMOLATION_TRAP,
               FROST_TRAP,
               EXPLOSIVE_TRAP;

        uint32 ASPECT_OF_THE_HAWK,
               ASPECT_OF_THE_MONKEY,
               RAPID_FIRE,
               TRUESHOT_AURA;

        // racial
        uint32 STONEFORM,
               ESCAPE_ARTIST,
               EVERY_MAN_FOR_HIMSELF,
               SHADOWMELD,
               BLOOD_FURY,
               WAR_STOMP,
               BERSERKING,
               WILL_OF_THE_FORSAKEN;
};

#endif
