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

#include "PlayerbotShamanAI.h"
#include "../../Entities/Totem.h"

class PlayerbotAI;
PlayerbotShamanAI::PlayerbotShamanAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    // restoration
    CHAIN_HEAL               = m_ai.initSpell(CHAIN_HEAL_1);
    HEALING_WAVE             = m_ai.initSpell(HEALING_WAVE_1);
    LESSER_HEALING_WAVE      = m_ai.initSpell(LESSER_HEALING_WAVE_1);
    ANCESTRAL_SPIRIT         = m_ai.initSpell(ANCESTRAL_SPIRIT_1);
    EARTH_SHIELD             = m_ai.initSpell(EARTH_SHIELD_1);
    TREMOR_TOTEM             = m_ai.initSpell(TREMOR_TOTEM_1); // totems
    HEALING_STREAM_TOTEM     = m_ai.initSpell(HEALING_STREAM_TOTEM_1);
    MANA_SPRING_TOTEM        = m_ai.initSpell(MANA_SPRING_TOTEM_1);
    MANA_TIDE_TOTEM          = m_ai.initSpell(MANA_TIDE_TOTEM_1);
    CURE_DISEASE_SHAMAN      = m_ai.initSpell(CURE_DISEASE_SHAMAN_1);
    CURE_POISON_SHAMAN       = m_ai.initSpell(CURE_POISON_SHAMAN_1);
    NATURES_SWIFTNESS_SHAMAN = m_ai.initSpell(NATURES_SWIFTNESS_SHAMAN_1);
    // enhancement
    FOCUSED                  = 0; // Focused what?
    STORMSTRIKE              = m_ai.initSpell(STORMSTRIKE_1);
    BLOODLUST                = m_ai.initSpell(BLOODLUST_1);
    LIGHTNING_SHIELD         = m_ai.initSpell(LIGHTNING_SHIELD_1);
    ROCKBITER_WEAPON         = m_ai.initSpell(ROCKBITER_WEAPON_1);
    FLAMETONGUE_WEAPON       = m_ai.initSpell(FLAMETONGUE_WEAPON_1);
    FROSTBRAND_WEAPON        = m_ai.initSpell(FROSTBRAND_WEAPON_1);
    WINDFURY_WEAPON          = m_ai.initSpell(WINDFURY_WEAPON_1);
    STONESKIN_TOTEM          = m_ai.initSpell(STONESKIN_TOTEM_1); // totems
    STRENGTH_OF_EARTH_TOTEM  = m_ai.initSpell(STRENGTH_OF_EARTH_TOTEM_1);
    FROST_RESISTANCE_TOTEM   = m_ai.initSpell(FROST_RESISTANCE_TOTEM_1);
    FLAMETONGUE_TOTEM        = m_ai.initSpell(FLAMETONGUE_TOTEM_1);
    FIRE_RESISTANCE_TOTEM    = m_ai.initSpell(FIRE_RESISTANCE_TOTEM_1);
    GROUNDING_TOTEM          = m_ai.initSpell(GROUNDING_TOTEM_1);
    NATURE_RESISTANCE_TOTEM  = m_ai.initSpell(NATURE_RESISTANCE_TOTEM_1);
    WIND_FURY_TOTEM          = m_ai.initSpell(WINDFURY_TOTEM_1);
    STONESKIN_TOTEM          = m_ai.initSpell(STONESKIN_TOTEM_1);
    WRATH_OF_AIR_TOTEM       = m_ai.initSpell(WRATH_OF_AIR_TOTEM_1);
    EARTH_ELEMENTAL_TOTEM    = m_ai.initSpell(EARTH_ELEMENTAL_TOTEM_1);
    // elemental
    LIGHTNING_BOLT           = m_ai.initSpell(LIGHTNING_BOLT_1);
    EARTH_SHOCK              = m_ai.initSpell(EARTH_SHOCK_1);
    FLAME_SHOCK              = m_ai.initSpell(FLAME_SHOCK_1);
    PURGE                    = m_ai.initSpell(PURGE_1);
    FROST_SHOCK              = m_ai.initSpell(FROST_SHOCK_1);
    CHAIN_LIGHTNING          = m_ai.initSpell(CHAIN_LIGHTNING_1);
    STONECLAW_TOTEM          = m_ai.initSpell(STONECLAW_TOTEM_1); // totems
    SEARING_TOTEM            = m_ai.initSpell(SEARING_TOTEM_1);
    FIRE_NOVA_TOTEM          = 0; // NPC only spell, check FIRE_NOVA_1
    MAGMA_TOTEM              = m_ai.initSpell(MAGMA_TOTEM_1);
    EARTHBIND_TOTEM          = m_ai.initSpell(EARTHBIND_TOTEM_1);
    FIRE_ELEMENTAL_TOTEM     = m_ai.initSpell(FIRE_ELEMENTAL_TOTEM_1);
    ELEMENTAL_MASTERY        = m_ai.initSpell(ELEMENTAL_MASTERY_1);

    RECENTLY_BANDAGED        = 11196; // first aid check

    // racial
    BLOOD_FURY               = m_ai.initSpell(BLOOD_FURY_ALL); // orc
    WAR_STOMP                = m_ai.initSpell(WAR_STOMP_ALL); // tauren
    BERSERKING               = m_ai.initSpell(BERSERKING_ALL); // troll

    // totem buffs
    STRENGTH_OF_EARTH_EFFECT    = m_ai.initSpell(STRENGTH_OF_EARTH_EFFECT_1);
    FLAMETONGUE_EFFECT          = m_ai.initSpell(FLAMETONGUE_EFFECT_1);
    MAGMA_TOTEM_EFFECT          = m_ai.initSpell(MAGMA_TOTEM_EFFECT_1);
    STONECLAW_EFFECT            = m_ai.initSpell(STONECLAW_EFFECT_1);
    FIRE_RESISTANCE_EFFECT      = m_ai.initSpell(FIRE_RESISTANCE_EFFECT_1);
    FROST_RESISTANCE_EFFECT     = m_ai.initSpell(FROST_RESISTANCE_EFFECT_1);
    GROUDNING_EFFECT            = m_ai.initSpell(GROUDNING_EFFECT_1);
    NATURE_RESISTANCE_EFFECT    = m_ai.initSpell(NATURE_RESISTANCE_EFFECT_1);
    STONESKIN_EFFECT            = m_ai.initSpell(STONESKIN_EFFECT_1);
    WINDFURY_EFFECT             = m_ai.initSpell(WINDFURY_EFFECT_1);
    WRATH_OF_AIR_EFFECT         = m_ai.initSpell(WRATH_OF_AIR_EFFECT_1);
    CLEANSING_TOTEM_EFFECT      = m_ai.initSpell(CLEANSING_TOTEM_EFFECT_1);
    MANA_SPRING_EFFECT          = m_ai.initSpell(MANA_SPRING_EFFECT_1);
    TREMOR_TOTEM_EFFECT         = m_ai.initSpell(TREMOR_TOTEM_EFFECT_1);
    STONECLAW_EFFECT            = m_ai.initSpell(STONECLAW_EFFECT_1);
    EARTHBIND_EFFECT            = m_ai.initSpell(EARTHBIND_EFFECT_1);
}

PlayerbotShamanAI::~PlayerbotShamanAI() {}

CombatManeuverReturns PlayerbotShamanAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.GroupTankHoldsAggro())
        {
            if (PlayerbotAI::ORDERS_HEAL & m_ai.GetCombatOrder())
                return HealPlayer(GetHealTarget());
            else
                return RETURN_NO_ACTION_OK; // wait it out
        }
        else
        {
            m_ai.ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
        }
    }

    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_OOC)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.IsGroupReady())
            return RETURN_NO_ACTION_OK; // wait it out
        else
            m_ai.ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_OOC);
    }

    switch (m_ai.GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoFirstCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoFirstCombatManeuverPVE(pTarget);
    }
}

CombatManeuverReturns PlayerbotShamanAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotShamanAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotShamanAI::DoNextCombatManeuver(Unit* pTarget)
{
    // Face enemy, make sure bot is attacking
    m_ai.FaceTarget(pTarget);

    switch (m_ai.GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoNextCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoNextCombatManeuverPVE(pTarget);
    }
}

CombatManeuverReturns PlayerbotShamanAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    uint32 spec = m_bot.GetSpec();

    // Make sure healer stays put, don't even melee (aggro) if in range.
    if (m_ai.IsHealer() && m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_RANGED)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    else if (!m_ai.IsHealer() && m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    // Dispel disease/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Heal (try to pick a target by on common rules, than heal using each PlayerbotClassAI HealPlayer() method)
    if (FindTargetAndHeal())
        return RETURN_CONTINUE;

    // Damage Spells
    DropTotems();
    CheckShields();
    UseCooldowns();
    switch (spec)
    {
        case SHAMAN_SPEC_ENHANCEMENT:
            if (STORMSTRIKE > 0 && (m_bot.IsSpellReady(STORMSTRIKE)) && m_ai.CastSpell(STORMSTRIKE, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (FLAME_SHOCK > 0 && (!pTarget->HasAura(FLAME_SHOCK)) && m_ai.CastSpell(FLAME_SHOCK, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (EARTH_SHOCK > 0 && (m_bot.IsSpellReady(EARTH_SHOCK)) && m_ai.CastSpell(EARTH_SHOCK, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;

            /*if (FOCUSED > 0 && m_ai.CastSpell(FOCUSED, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;*/
            break;

        case SHAMAN_SPEC_RESTORATION:
        // fall through to elemental

        case SHAMAN_SPEC_ELEMENTAL:
            if (FLAME_SHOCK > 0 && (!pTarget->HasAura(FLAME_SHOCK)) && m_ai.CastSpell(FLAME_SHOCK, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (LIGHTNING_BOLT > 0 && m_ai.CastSpell(LIGHTNING_BOLT, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            /*if (FROST_SHOCK > 0 && !pTarget->HasAura(FROST_SHOCK, EFFECT_INDEX_0) && m_ai.CastSpell(FROST_SHOCK, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;*/
            /*if (CHAIN_LIGHTNING > 0 && m_ai.CastSpell(CHAIN_LIGHTNING, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;*/
    }

    return RETURN_NO_ACTION_OK;
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotShamanAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    DropTotems();
    CheckShields();
    UseCooldowns();

    Player* healTarget = (m_ai.GetScenarioType() == PlayerbotAI::SCENARIO_PVP_DUEL) ? GetHealTarget() : &m_bot;
    if (HealPlayer(healTarget) & (RETURN_NO_ACTION_OK | RETURN_CONTINUE))
        return RETURN_CONTINUE;
    if (m_ai.CastSpell(LIGHTNING_BOLT) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

CombatManeuverReturns PlayerbotShamanAI::HealPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::HealPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    // If target is out of range (40 yards) and is a tank: move towards it
    // if bot is not asked to stay
    // Other classes have to adjust their position to the healers
    // TODO: This code should be common to all healers and will probably
    // move to a more suitable place like PlayerbotAI::DoCombatMovement()
    if ((GetTargetJob(target) == JOB_TANK || GetTargetJob(target) == JOB_MAIN_TANK)
            && m_bot.GetPlayerbotAI()->GetMovementOrder() != PlayerbotAI::MOVEMENT_STAY
            && !m_ai.In_Reach(target, HEALING_WAVE))
    {
        m_bot.GetMotionMaster()->MoveFollow(target, 39.0f, m_bot.GetOrientation());
        return RETURN_CONTINUE;
    }

    // Everyone is healthy enough, return OK. MUST correlate to highest value below (should be last HP check)
    if (target->GetHealthPercent() >= 80)
        return RETURN_NO_ACTION_OK;

    // Technically the best rotation is CHAIN + LHW + LHW subbing in HW for trouble (bad mana efficiency)
    if (target->GetHealthPercent() < 30 && HEALING_WAVE > 0 && m_ai.CastSpell(HEALING_WAVE, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (target->GetHealthPercent() < 50 && LESSER_HEALING_WAVE > 0 && m_ai.CastSpell(LESSER_HEALING_WAVE, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (target->GetHealthPercent() < 80 && CHAIN_HEAL > 0 && m_ai.CastSpell(CHAIN_HEAL, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
} // end HealTarget

CombatManeuverReturns PlayerbotShamanAI::ResurrectPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::ResurrectPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    if (m_ai.IsInCombat())     // Just in case as this was supposedly checked before calling this function
        return RETURN_NO_ACTION_ERROR;

    if (ANCESTRAL_SPIRIT > 0 && m_ai.In_Reach(target, ANCESTRAL_SPIRIT) && m_ai.CastSpell(ANCESTRAL_SPIRIT, *target) == SPELL_CAST_OK)
    {
        std::string msg = "Resurrecting ";
        msg += target->GetName();
        m_bot.Say(msg, LANG_UNIVERSAL);
        return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_ERROR; // not error per se - possibly just OOM
}

CombatManeuverReturns PlayerbotShamanAI::DispelPlayer(Player* /*target*/)
{
    // Remove poison on group members
    if (Player* poisonedTarget = GetDispelTarget(DISPEL_POISON))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(poisonedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (CURE_POISON_SHAMAN > 0 && m_ai.CastSpell(CURE_POISON_SHAMAN, *poisonedTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    // Remove disease on group members
    if (Player* diseasedTarget = GetDispelTarget(DISPEL_DISEASE))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(diseasedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (CURE_DISEASE_SHAMAN > 0 && m_ai.CastSpell(CURE_DISEASE_SHAMAN, *diseasedTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_OK;
}

void PlayerbotShamanAI::DropTotems()
{
    uint32 spec = m_bot.GetSpec();

    Totem* earth = m_bot.GetTotem(TOTEM_SLOT_EARTH);
    Totem* fire = m_bot.GetTotem(TOTEM_SLOT_FIRE);
    Totem* water = m_bot.GetTotem(TOTEM_SLOT_WATER);
    Totem* air = m_bot.GetTotem(TOTEM_SLOT_AIR);

    // Earth Totems
    if ((earth == nullptr) || (m_bot.GetDistance(earth) > 30))
    {
        if (STRENGTH_OF_EARTH_TOTEM > 0 && m_ai.CastSpell(STRENGTH_OF_EARTH_TOTEM) == SPELL_CAST_OK)
            return;
    }

    // Fire Totems
    if ((fire == nullptr) || (m_bot.GetDistance(fire) > 30))
    {
        if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FROST && FROST_RESISTANCE_TOTEM > 0 && m_ai.CastSpell(FROST_RESISTANCE_TOTEM) == SPELL_CAST_OK)
            return;
        // If the spec didn't take totem of wrath, use flametongue
        else if ((spec != SHAMAN_SPEC_ELEMENTAL) && FLAMETONGUE_TOTEM > 0 && m_ai.CastSpell(FLAMETONGUE_TOTEM) == SPELL_CAST_OK)
            return;
    }

    // Air totems
    if ((air == nullptr) || (m_bot.GetDistance(air) > 30))
    {
        if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_NATURE && NATURE_RESISTANCE_TOTEM > 0 && m_ai.CastSpell(NATURE_RESISTANCE_TOTEM) == SPELL_CAST_OK)
            return;
        else if (spec == SHAMAN_SPEC_ENHANCEMENT)
        {
            if (WIND_FURY_TOTEM > 0 /*&& !m_bot.HasAura(IMPROVED_ICY_TALONS)*/ && m_ai.CastSpell(WIND_FURY_TOTEM) == SPELL_CAST_OK)
                return;
        }
        else
        {
            if (WRATH_OF_AIR_TOTEM > 0 && m_ai.CastSpell(WRATH_OF_AIR_TOTEM) == SPELL_CAST_OK)
                return;
        }
    }

    // Water Totems
    if ((water == nullptr) || (m_bot.GetDistance(water) > 30))
    {
        if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FIRE && FIRE_RESISTANCE_TOTEM > 0 && m_ai.CastSpell(FIRE_RESISTANCE_TOTEM) == SPELL_CAST_OK)
            return;
        else if (MANA_SPRING_TOTEM > 0 && m_ai.CastSpell(MANA_SPRING_TOTEM) == SPELL_CAST_OK)
            return;
    }

    /*if (EARTH_ELEMENTAL_TOTEM > 0 && m_ai.CastSpell(EARTH_ELEMENTAL_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (EARTHBIND_TOTEM > 0 && !pTarget->HasAura(EARTHBIND_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(STRENGTH_OF_EARTH_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(EARTHBIND_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (FIRE_ELEMENTAL_TOTEM > 0 && m_ai.CastSpell(FIRE_ELEMENTAL_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (FIRE_NOVA_TOTEM > 0 && m_ai.CastSpell(FIRE_NOVA_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (GROUNDING_TOTEM > 0 && !m_bot.HasAura(GROUNDING_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(WRATH_OF_AIR_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(WIND_FURY_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(GROUNDING_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (HEALING_STREAM_TOTEM > 0 && m_ai.GetHealthPercent() < 50 && !m_bot.HasAura(HEALING_STREAM_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(MANA_SPRING_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(HEALING_STREAM_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (MAGMA_TOTEM > 0 && (!m_bot.HasAura(TOTEM_OF_WRATH, EFFECT_INDEX_0)) && m_ai.CastSpell(MAGMA_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (SEARING_TOTEM > 0 && !pTarget->HasAura(SEARING_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(TOTEM_OF_WRATH, EFFECT_INDEX_0) && m_ai.CastSpell(SEARING_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (STONECLAW_TOTEM > 0 && m_ai.GetHealthPercent() < 51 && !pTarget->HasAura(STONECLAW_TOTEM, EFFECT_INDEX_0) && !pTarget->HasAura(EARTHBIND_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(STRENGTH_OF_EARTH_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(STONECLAW_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (STONESKIN_TOTEM > 0 && !m_bot.HasAura(STONESKIN_TOTEM, EFFECT_INDEX_0) && !m_bot.HasAura(STRENGTH_OF_EARTH_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(STONESKIN_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
    /*if (TREMOR_TOTEM > 0 && !m_bot.HasAura(STRENGTH_OF_EARTH_TOTEM, EFFECT_INDEX_0) && m_ai.CastSpell(TREMOR_TOTEM) == SPELL_CAST_OK)
        return RETURN_CONTINUE;*/
}

void PlayerbotShamanAI::CheckShields()
{
    uint32 spec = m_bot.GetSpec();

    if (spec == SHAMAN_SPEC_ENHANCEMENT && LIGHTNING_SHIELD > 0 && !m_bot.HasAura(LIGHTNING_SHIELD, EFFECT_INDEX_0))
        m_ai.CastSpell(LIGHTNING_SHIELD, m_bot);
    if (EARTH_SHIELD > 0 && !m_master.HasAura(EARTH_SHIELD, EFFECT_INDEX_0))
        m_ai.CastSpell(EARTH_SHIELD, m_master);
}

void PlayerbotShamanAI::UseCooldowns()
{
    uint32 spec = m_bot.GetSpec();

    if (BLOODLUST > 0 && (!m_master.HasAura(BLOODLUST, EFFECT_INDEX_0)) && m_ai.CastSpell(BLOODLUST) == SPELL_CAST_OK)
        return;

    switch (spec)
    {
        case SHAMAN_SPEC_ENHANCEMENT:
            break;

        case SHAMAN_SPEC_ELEMENTAL:
            if (ELEMENTAL_MASTERY > 0 && m_ai.CastSpell(ELEMENTAL_MASTERY, m_bot) == SPELL_CAST_OK)
                return;
            break;

        case SHAMAN_SPEC_RESTORATION:
            if (MANA_TIDE_TOTEM > 0 && m_ai.GetManaPercent() < 50 && m_ai.CastSpell(MANA_TIDE_TOTEM) == SPELL_CAST_OK)
                return;
            else if (NATURES_SWIFTNESS_SHAMAN > 0 && m_ai.CastSpell(NATURES_SWIFTNESS_SHAMAN) == SPELL_CAST_OK)
                return;

        default:
            break;
    }
}

void PlayerbotShamanAI::DoNonCombatActions()
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return;

    // Dispel disease/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return;

    // Revive
    if (ResurrectPlayer(GetResurrectionTarget()) & RETURN_CONTINUE)
        return;

    // Heal
    if (m_ai.IsHealer())
    {
        if (HealPlayer(GetHealTarget()) & RETURN_CONTINUE)
            return;// RETURN_CONTINUE;
    }
    else
    {
        // Is this desirable? Debatable.
        // TODO: In a group/raid with a healer you'd want this bot to focus on DPS (it's not specced/geared for healing either)
        if (HealPlayer(&m_bot) & RETURN_CONTINUE)
            return;// RETURN_CONTINUE;
    }

    uint32 spec = m_bot.GetSpec();

    CheckShields();
    /*
           // buff myself weapon
           if (ROCKBITER_WEAPON > 0)
                (!m_bot.HasAura(ROCKBITER_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(WINDFURY_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FLAMETONGUE_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FROSTBRAND_WEAPON, EFFECT_INDEX_0) && m_ai.CastSpell(ROCKBITER_WEAPON,m_bot) == SPELL_CAST_OK);
           else if (EARTHLIVING_WEAPON > 0)
                (!m_bot.HasAura(EARTHLIVING_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FLAMETONGUE_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FROSTBRAND_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(ROCKBITER_WEAPON, EFFECT_INDEX_0) && m_ai.CastSpell(WINDFURY_WEAPON,m_bot) == SPELL_CAST_OK);
           else if (WINDFURY_WEAPON > 0)
                (!m_bot.HasAura(WINDFURY_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FLAMETONGUE_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FROSTBRAND_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(ROCKBITER_WEAPON, EFFECT_INDEX_0) && m_ai.CastSpell(WINDFURY_WEAPON,m_bot) == SPELL_CAST_OK);
           else if (FLAMETONGUE_WEAPON > 0)
                (!m_bot.HasAura(FLAMETONGUE_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(WINDFURY_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FROSTBRAND_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(ROCKBITER_WEAPON, EFFECT_INDEX_0) && m_ai.CastSpell(FLAMETONGUE_WEAPON,m_bot) == SPELL_CAST_OK);
           else if (FROSTBRAND_WEAPON > 0)
                (!m_bot.HasAura(FROSTBRAND_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(WINDFURY_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(FLAMETONGUE_WEAPON, EFFECT_INDEX_0) && !m_bot.HasAura(ROCKBITER_WEAPON, EFFECT_INDEX_0) && m_ai.CastSpell(FROSTBRAND_WEAPON,m_bot) == SPELL_CAST_OK);
     */
    // Mainhand
    Item* weapon;
    weapon = m_bot.GetItemByPos(EQUIPMENT_SLOT_MAINHAND);
    if (weapon && (weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0) && spec == SHAMAN_SPEC_ELEMENTAL)
        m_ai.CastSpell(FLAMETONGUE_WEAPON, m_bot);
    else if (weapon && (weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0) && spec == SHAMAN_SPEC_ENHANCEMENT)
        m_ai.CastSpell(WINDFURY_WEAPON, m_bot);

    //Offhand
    weapon = m_bot.GetItemByPos(EQUIPMENT_SLOT_OFFHAND);
    if (weapon && (weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0) && spec == SHAMAN_SPEC_ENHANCEMENT)
        m_ai.CastSpell(FLAMETONGUE_WEAPON, m_bot);

    // hp/mana check
    if (EatDrinkBandage())
        return;
} // end DoNonCombatActions

bool PlayerbotShamanAI::CastHoTOnTank()
{
    if (!m_ai.IsHealer()) return false;

    // Shaman: Healing Stream Totem
    // None of these are cast before Pulling

    return false;
}
