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

#ifndef _PLAYERBOTDRUIDAI_H
#define _PLAYERBOTDRUIDAI_H

#include "../Base/PlayerbotClassAI.h"

enum DruidSpells
{
    ABOLISH_POISON_1                = 2893,
    AQUATIC_FORM_1                  = 1066,
    BARKSKIN_1                      = 22812,
    BASH_1                          = 5211,
    BEAR_FORM_1                     = 5487,
    CAT_FORM_1                      = 768,
    CHALLENGING_ROAR_1              = 5209,
    CLAW_1                          = 1082,
    COWER_1                         = 8998,
    CURE_POISON_1                   = 8946,
    DASH_1                          = 1850,
    DEMORALIZING_ROAR_1             = 99,
    DIRE_BEAR_FORM_1                = 9634,
    ENRAGE_1                        = 5229,
    ENTANGLING_ROOTS_1              = 339,
    FAERIE_FIRE_1                   = 770,
    FAERIE_FIRE_FERAL_1             = 16857,
    FERAL_CHARGE_BEAR_1             = 16979,
    FEROCIOUS_BITE_1                = 22568,
    FRENZIED_REGENERATION_1         = 22842,
    GIFT_OF_THE_WILD_1              = 21849,
    GROWL_1                         = 6795,
    HEALING_TOUCH_1                 = 5185,
    HIBERNATE_1                     = 2637,
    HURRICANE_1                     = 16914,
    INNERVATE_1                     = 29166,
    INSECT_SWARM_1                  = 5570,
    MARK_OF_THE_WILD_1              = 1126,
    MAUL_1                          = 6807,
    MOONFIRE_1                      = 8921,
    MOONKIN_FORM_1                  = 24858,
    NATURES_GRASP_1                 = 16689,
    NATURES_SWIFTNESS_DRUID_1       = 17116,
    OMEN_OF_CLARITY_1               = 16864,
    POUNCE_1                        = 9005,
    PROWL_1                         = 5215,
    RAKE_1                          = 1822,
    RAVAGE_1                        = 6785,
    REBIRTH_1                       = 20484,
    REGROWTH_1                      = 8936,
    REJUVENATION_1                  = 774,
    REMOVE_CURSE_DRUID_1            = 2782,
    RIP_1                           = 1079,
    SHRED_1                         = 5221,
    SOOTHE_ANIMAL_1                 = 2908,
    STARFIRE_1                      = 2912,
    SWIFTMEND_1                     = 18562,
    SWIPE_BEAR_1                    = 779,
    THORNS_1                        = 467,
    TIGERS_FURY_1                   = 5217,
    TRANQUILITY_1                   = 740,
    TRAVEL_FORM_1                   = 783,
    WRATH_1                         = 5176
};

//class Player;

class PlayerbotDruidAI : PlayerbotClassAI
{
    public:
        PlayerbotDruidAI(Player& master, Player& bot, PlayerbotAI& ai);
        virtual ~PlayerbotDruidAI();

        // all combat actions go here
        CombatManeuverReturns DoFirstCombatManeuver(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuver(Unit* pTarget) override;
        bool CanPull() override;
        bool Pull() override;
        uint32 Neutralize(uint8 creatureType) override;

        // all non combat actions go here, ex buffs, heals, rezzes
        void DoNonCombatActions() override;

        // Utility Functions
        bool CastHoTOnTank();

    private:
        CombatManeuverReturns DoFirstCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVE(Unit* pTarget) override;
        CombatManeuverReturns DoFirstCombatManeuverPVP(Unit* pTarget) override;
        CombatManeuverReturns DoNextCombatManeuverPVP(Unit* pTarget) override;

        CombatManeuverReturns CastSpell(uint32 nextAction, Unit* pTarget = nullptr) { return CastSpellNoRanged(nextAction, pTarget); }

        // Combat Maneuver helper functions
        CombatManeuverReturns _DoNextPVECombatManeuverBear(Unit* pTarget);
        CombatManeuverReturns _DoNextPVECombatManeuverCat(Unit* pTarget);
        CombatManeuverReturns _DoNextPVECombatManeuverSpellDPS(Unit* pTarget);
        CombatManeuverReturns _DoNextPVECombatManeuverHeal();

        // Heals the target based off its hps
        CombatManeuverReturns HealPlayer(Player* target) override;
        // Resurrects the target
        CombatManeuverReturns ResurrectPlayer(Player* target) override;
        // Dispel disease or negative magic effects from an internally selected target
        CombatManeuverReturns DispelPlayer(Player* target = nullptr);

        static bool BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target);
        // Callback method to reset shapeshift forms blocking buffs and heals
        static void GoBuffForm(Player* self);

        //Assumes form based on spec
        uint8 CheckForms();
        enum CheckForms_ReturnValues
        {
            RETURN_FAIL = 0,
            RETURN_FAIL_WAITINGONSELFBUFF,
            RETURN_OK_NOCHANGE,
            RETURN_OK_SHIFTING,
            RETURN_OK_CANNOTSHIFT
        };

        // druid cat/bear/dire bear/moonkin/tree of life forms
        uint32 CAT_FORM,
               BEAR_FORM,
               DIRE_BEAR_FORM,
               MOONKIN_FORM,
               TRAVEL_FORM;

        // druid cat attacks
        uint32 CLAW,
               COWER,
               TIGERS_FURY,
               RAKE,
               RIP,
               SHRED,
               FEROCIOUS_BITE;

        // druid bear/dire bear attacks & buffs
        uint32 BASH,
               MAUL,
               SWIPE,
               DEMORALIZING_ROAR,
               CHALLENGING_ROAR,
               GROWL,
               ENRAGE,
               FAERIE_FIRE_FERAL;

        // druid caster DPS attacks & debuffs
        uint32 MOONFIRE,
               ROOTS,
               WRATH,
               OMEN_OF_CLARITY,
               STARFIRE,
               INSECT_SWARM,
               FAERIE_FIRE,
               HIBERNATE;

        // druid buffs
        uint32 MARK_OF_THE_WILD,
               GIFT_OF_THE_WILD,
               THORNS,
               INNERVATE,
               NATURES_SWIFTNESS,
               BARKSKIN;

        // druid heals
        uint32 REJUVENATION,
               REGROWTH,
               HEALING_TOUCH,
               SWIFTMEND,
               TRANQUILITY,
               REBIRTH,
               REMOVE_CURSE,
               CURE_POISON,
               ABOLISH_POISON;

        // racial
        uint32 STONEFORM,
               ESCAPE_ARTIST,
               EVERY_MAN_FOR_HIMSELF,
               SHADOWMELD,
               BLOOD_FURY,
               WAR_STOMP,
               BERSERKING,
               WILL_OF_THE_FORSAKEN;

        uint32 SpellSequence, DruidSpellCombat;
};

#endif
