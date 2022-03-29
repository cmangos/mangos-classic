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

#include "PlayerbotWarriorAI.h"
#include "../Base/PlayerbotMgr.h"

class PlayerbotAI;
PlayerbotWarriorAI::PlayerbotWarriorAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    SHOOT_BOW               = m_ai.initSpell(SHOOT_BOW_1); // GENERAL
    SHOOT_GUN               = m_ai.initSpell(SHOOT_GUN_1); // GENERAL
    SHOOT_XBOW              = m_ai.initSpell(SHOOT_XBOW_1); // GENERAL

    BATTLE_STANCE           = m_ai.initSpell(BATTLE_STANCE_1); //ARMS
    CHARGE                  = m_ai.initSpell(CHARGE_1); //ARMS
    OVERPOWER               = m_ai.initSpell(OVERPOWER_1); // ARMS
    HEROIC_STRIKE           = m_ai.initSpell(HEROIC_STRIKE_1); //ARMS
    REND                    = m_ai.initSpell(REND_1); //ARMS
    THUNDER_CLAP            = m_ai.initSpell(THUNDER_CLAP_1);  //ARMS
    HAMSTRING               = m_ai.initSpell(HAMSTRING_1);  //ARMS
    MOCKING_BLOW            = m_ai.initSpell(MOCKING_BLOW_1);  //ARMS
    RETALIATION             = m_ai.initSpell(RETALIATION_1);  //ARMS
    SWEEPING_STRIKES        = m_ai.initSpell(SWEEPING_STRIKES_1); //ARMS
    MORTAL_STRIKE           = m_ai.initSpell(MORTAL_STRIKE_1);  //ARMS
    BLOODRAGE               = m_ai.initSpell(BLOODRAGE_1); //PROTECTION
    DEFENSIVE_STANCE        = m_ai.initSpell(DEFENSIVE_STANCE_1); //PROTECTION
    SUNDER_ARMOR            = m_ai.initSpell(SUNDER_ARMOR_1); //PROTECTION
    TAUNT                   = m_ai.initSpell(TAUNT_1); //PROTECTION
    SHIELD_BASH             = m_ai.initSpell(SHIELD_BASH_1); //PROTECTION
    REVENGE                 = m_ai.initSpell(REVENGE_1); //PROTECTION
    SHIELD_BLOCK            = m_ai.initSpell(SHIELD_BLOCK_1); //PROTECTION
    DISARM                  = m_ai.initSpell(DISARM_1); //PROTECTION
    SHIELD_WALL             = m_ai.initSpell(SHIELD_WALL_1); //PROTECTION
    SHIELD_SLAM             = m_ai.initSpell(SHIELD_SLAM_1); //PROTECTION
    CONCUSSION_BLOW         = m_ai.initSpell(CONCUSSION_BLOW_1); //PROTECTION
    LAST_STAND              = m_ai.initSpell(LAST_STAND_1); //PROTECTION
    BATTLE_SHOUT            = m_ai.initSpell(BATTLE_SHOUT_1); //FURY
    DEMORALIZING_SHOUT      = m_ai.initSpell(DEMORALIZING_SHOUT_1); //FURY
    CLEAVE                  = m_ai.initSpell(CLEAVE_1); //FURY
    INTIMIDATING_SHOUT      = m_ai.initSpell(INTIMIDATING_SHOUT_1); //FURY
    EXECUTE                 = m_ai.initSpell(EXECUTE_1); //FURY
    CHALLENGING_SHOUT       = m_ai.initSpell(CHALLENGING_SHOUT_1); //FURY
    SLAM                    = m_ai.initSpell(SLAM_1); //FURY
    BERSERKER_STANCE        = m_ai.initSpell(BERSERKER_STANCE_1); //FURY
    INTERCEPT               = m_ai.initSpell(INTERCEPT_1); //FURY
    DEATH_WISH              = m_ai.initSpell(DEATH_WISH_1); //FURY
    BERSERKER_RAGE          = m_ai.initSpell(BERSERKER_RAGE_1); //FURY
    WHIRLWIND               = m_ai.initSpell(WHIRLWIND_1); //FURY
    PUMMEL                  = m_ai.initSpell(PUMMEL_1); //FURY
    BLOODTHIRST             = m_ai.initSpell(BLOODTHIRST_1); //FURY
    RECKLESSNESS            = m_ai.initSpell(RECKLESSNESS_1); //FURY
    PIERCING_HOWL           = m_ai.initSpell(PIERCING_HOWL_1); //FURY

    // Talents
    IMPROVED_DEMORALIZING_SHOUT = 0;
    for (unsigned int i : uiImprovedDemoralizingShout)
    {
        if (m_ai.initSpell(i))
            IMPROVED_DEMORALIZING_SHOUT = m_ai.initSpell(i);
    }
    IMPROVED_SLAM = 0;
    for (unsigned int i : uiImprovedSlam)
    {
        if (m_ai.initSpell(i))
            IMPROVED_SLAM = m_ai.initSpell(i);
    }

    RECENTLY_BANDAGED       = 11196; // first aid check

    // racial
    STONEFORM               = m_ai.initSpell(STONEFORM_ALL); // dwarf
    ESCAPE_ARTIST           = m_ai.initSpell(ESCAPE_ARTIST_ALL); // gnome
    PERCEPTION              = m_ai.initSpell(PERCEPTION_ALL); // human
    SHADOWMELD              = m_ai.initSpell(SHADOWMELD_ALL); // night elf
    BLOOD_FURY              = m_ai.initSpell(BLOOD_FURY_ALL); // orc
    WAR_STOMP               = m_ai.initSpell(WAR_STOMP_ALL); // tauren
    BERSERKING              = m_ai.initSpell(BERSERKING_ALL); // troll
    WILL_OF_THE_FORSAKEN    = m_ai.initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead

    //Procs
}
PlayerbotWarriorAI::~PlayerbotWarriorAI() {}

CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.GroupTankHoldsAggro())
        {
            if (PlayerbotAI::ORDERS_TANK & m_ai.GetCombatOrder())
            {
                if (m_bot.GetDistance(pTarget, true, DIST_CALC_COMBAT_REACH_WITH_MELEE) <= ATTACK_DISTANCE)
                {
                    // Set everyone's UpdateAI() waiting to 2 seconds
                    m_ai.SetGroupIgnoreUpdateTime(2);
                    // Clear their TEMP_WAIT_TANKAGGRO flag
                    m_ai.ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
                    // Start attacking, force target on current target
                    m_ai.Attack(m_ai.GetCurrentTarget());

                    // While everyone else is waiting 2 second, we need to build up aggro, so don't return
                }
                else
                {
                    // TODO: add check if target is ranged
                    return RETURN_NO_ACTION_OK; // wait for target to get nearer
                }
            }
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

CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuverPVE(Unit* pTarget)
{
    float fTargetDist = m_bot.GetDistance(pTarget, true, DIST_CALC_COMBAT_REACH_WITH_MELEE);

    // Get bot spec. If bot has tank orders, force spec to protection
    uint32 spec = ((m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK) ? WARRIOR_SPEC_PROTECTION : m_bot.GetSpec());

    if (BERSERKER_STANCE && spec == WARRIOR_SPEC_FURY && (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_ASSIST))
    {
        if (!m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(BERSERKER_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (BLOODRAGE > 0 && m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai.GetRageAmount() <= 10)
            return m_ai.CastSpell(BLOODRAGE) == SPELL_CAST_OK ? RETURN_FINISHED_FIRST_MOVES : RETURN_NO_ACTION_ERROR;
        if (INTERCEPT > 0 && m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
        {
            if (fTargetDist < 8.0f)
                return RETURN_NO_ACTION_OK;
            else if (fTargetDist > 25.0f)
                return RETURN_CONTINUE; // wait to come into range
            else if (INTERCEPT > 0 && m_ai.CastSpell(INTERCEPT, *pTarget) == SPELL_CAST_OK)
            {
                float x, y, z;
                pTarget->GetContactPoint(&m_bot, x, y, z, 3.666666f);
                m_bot.Relocate(x, y, z);
                return RETURN_FINISHED_FIRST_MOVES;
            }
        }
    }
    else if (BATTLE_STANCE && (spec == WARRIOR_SPEC_ARMS || (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_ASSIST)))
    {
        if (!m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(BATTLE_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (CHARGE > 0 && m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
        {
            if (fTargetDist < 8.0f)
                return RETURN_NO_ACTION_OK;
            if (fTargetDist > 25.0f)
                return RETURN_CONTINUE; // wait to come into range
            else if (CHARGE > 0 && m_ai.CastSpell(CHARGE, *pTarget) == SPELL_CAST_OK)
            {
                float x, y, z;
                pTarget->GetContactPoint(&m_bot, x, y, z, 3.666666f);
                m_bot.Relocate(x, y, z);
                return RETURN_FINISHED_FIRST_MOVES;
            }
        }
    }
    else if (DEFENSIVE_STANCE && spec == WARRIOR_SPEC_PROTECTION)
    {
        if (!m_bot.HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DEFENSIVE_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        else if (TAUNT > 0 && m_bot.HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(TAUNT, *pTarget) == SPELL_CAST_OK)
            return RETURN_FINISHED_FIRST_MOVES;
    }

    return RETURN_NO_ACTION_OK;
}

// TODO: blatant copy of PVE for now, please PVP-port it
CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuverPVP(Unit* pTarget)
{
    float fTargetDist = m_bot.GetDistance(pTarget, true, DIST_CALC_COMBAT_REACH_WITH_MELEE);

    if (DEFENSIVE_STANCE && (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK))
    {
        if (!m_bot.HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DEFENSIVE_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        else if (TAUNT > 0 && m_bot.HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(TAUNT, *pTarget) == SPELL_CAST_OK)
            return RETURN_FINISHED_FIRST_MOVES;
    }

    if (BERSERKER_STANCE)
    {
        if (!m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(BERSERKER_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (BLOODRAGE > 0 && m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai.GetRageAmount() <= 10)
            return m_ai.CastSpell(BLOODRAGE) == SPELL_CAST_OK ? RETURN_FINISHED_FIRST_MOVES : RETURN_NO_ACTION_ERROR;
        if (INTERCEPT > 0 && m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
        {
            if (fTargetDist < 8.0f)
                return RETURN_NO_ACTION_OK;
            else if (fTargetDist > 25.0f)
                return RETURN_CONTINUE; // wait to come into range
            else if (INTERCEPT > 0 && m_ai.CastSpell(INTERCEPT, *pTarget) == SPELL_CAST_OK)
            {
                float x, y, z;
                pTarget->GetContactPoint(&m_bot, x, y, z, 3.666666f);
                m_bot.Relocate(x, y, z);
                return RETURN_FINISHED_FIRST_MOVES;
            }
        }
    }

    if (BATTLE_STANCE)
    {
        if (!m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0) && m_ai.CastSpell(BATTLE_STANCE) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (CHARGE > 0 && m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
        {
            if (fTargetDist < 8.0f)
                return RETURN_NO_ACTION_OK;
            if (fTargetDist > 25.0f)
                return RETURN_CONTINUE; // wait to come into range
            else if (CHARGE > 0 && m_ai.CastSpell(CHARGE, *pTarget) == SPELL_CAST_OK)
            {
                float x, y, z;
                pTarget->GetContactPoint(&m_bot, x, y, z, 3.666666f);
                m_bot.Relocate(x, y, z);
                return RETURN_FINISHED_FIRST_MOVES;
            }
        }
    }

    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuver(Unit* pTarget)
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

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    //float fTargetDist = m_bot.GetCombatDistance(pTarget, true);

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);
    Unit* pVictim = pTarget->GetVictim();

    // do shouts, berserker rage, etc...
    if (BERSERKER_RAGE > 0 && !m_bot.HasAura(BERSERKER_RAGE, EFFECT_INDEX_0))
        m_ai.CastSpell(BERSERKER_RAGE);
    else if (BLOODRAGE > 0 && m_ai.GetRageAmount() <= 10)
        m_ai.CastSpell(BLOODRAGE);

    // Prevent low health humanoid from fleeing with Hamstring
    if ((m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0) || m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0)) && pTarget->GetHealthPercent() < 20 && !m_ai.IsElite(pTarget, true))
    {
        if (HAMSTRING > 0 && !pTarget->HasAura(HAMSTRING, EFFECT_INDEX_0) && m_ai.CastSpell(HAMSTRING, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    CheckShouts();

    // Get bot spec. If bot has tank orders, force spec to protection
    uint32 spec = ((m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK) ? WARRIOR_SPEC_PROTECTION : m_bot.GetSpec());

    if (spec == WARRIOR_SPEC_FURY && (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_ASSIST))
    {
        // Try to interrupt spell if target is casting one
        if (pTarget->IsNonMeleeSpellCasted(true))
        {
            if (PUMMEL > 0 && m_bot.IsSpellReady(PUMMEL) && m_ai.CastSpell(PUMMEL, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
        }

        if (DEATH_WISH > 0 && !m_bot.HasAura(DEATH_WISH, EFFECT_INDEX_0) && m_bot.IsSpellReady(DEATH_WISH) && m_ai.CastSpell(DEATH_WISH, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (EXECUTE > 0 && pTarget->GetHealthPercent() < 20 && m_ai.CastSpell(EXECUTE, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (BLOODTHIRST > 0 && m_bot.IsSpellReady(BLOODTHIRST) && m_ai.CastSpell(BLOODTHIRST, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (WHIRLWIND > 0 && m_bot.IsSpellReady(WHIRLWIND) && m_ai.CastSpell(WHIRLWIND, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (SWEEPING_STRIKES > 0 && m_ai.GetAttackerCount() > 1 && !m_bot.HasAura(SWEEPING_STRIKES, EFFECT_INDEX_0) && m_bot.IsSpellReady(SWEEPING_STRIKES) && m_ai.CastSpell(SWEEPING_STRIKES) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (IMPROVED_DEMORALIZING_SHOUT > 0 && DEMORALIZING_SHOUT > 0 && !pTarget->HasAura(DEMORALIZING_SHOUT, EFFECT_INDEX_0) && m_ai.CastSpell(DEMORALIZING_SHOUT) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (SLAM > 0 && IMPROVED_SLAM > 0 && m_ai.CastSpell(SLAM, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (m_bot.GetPowerPercent() > 60.f)
        {
            if (CLEAVE > 0 && m_ai.GetAttackerCount() > 1 && m_bot.IsSpellReady(CLEAVE) && m_ai.CastSpell(CLEAVE, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (HEROIC_STRIKE > 0 && m_ai.CastSpell(HEROIC_STRIKE, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
        }
    }
    else if (spec == WARRIOR_SPEC_ARMS || (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_ASSIST))
    {
        // Try to interrupt spell if target is casting one
        if (pTarget->IsNonMeleeSpellCasted(true))
        {
            if (SHIELD_BASH > 0 && m_ai.CastSpell(SHIELD_BASH, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
        }

        // If bot's target is also attacking the bot, use retaliation for extra attacks
        if (RETALIATION > 0 && pVictim == &m_bot && m_ai.GetAttackerCount() >= 2 && m_bot.IsSpellReady(RETALIATION) && !m_bot.HasAura(RETALIATION, EFFECT_INDEX_0) && m_ai.CastSpell(RETALIATION, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;

        if (EXECUTE > 0 && pTarget->GetHealthPercent() < 20 && m_ai.CastSpell(EXECUTE, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (REND > 0 && !pTarget->HasAura(REND, EFFECT_INDEX_0) && m_ai.CastSpell(REND, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (MORTAL_STRIKE > 0 && m_bot.IsSpellReady(MORTAL_STRIKE) && m_ai.CastSpell(MORTAL_STRIKE, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (OVERPOWER > 0 && m_bot.IsSpellReady(OVERPOWER))
        {
            uint8 base = pTarget->RollMeleeOutcomeAgainst(&m_bot, BASE_ATTACK, SPELL_SCHOOL_MASK_NORMAL);
            uint8 off = pTarget->RollMeleeOutcomeAgainst(&m_bot, OFF_ATTACK, SPELL_SCHOOL_MASK_NORMAL);
            if (base == MELEE_HIT_DODGE || off == MELEE_HIT_DODGE)
            {
                if (m_bot.IsSpellReady(OVERPOWER) && m_ai.CastSpell(OVERPOWER, *pTarget) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
            }
        }
        if (THUNDER_CLAP > 0 && !pTarget->HasAura(THUNDER_CLAP) && m_ai.CastSpell(THUNDER_CLAP, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (HEROIC_STRIKE > 0 && m_ai.CastSpell(HEROIC_STRIKE, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (SLAM > 0 && m_ai.CastSpell(SLAM, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }
    else if (spec == WARRIOR_SPEC_PROTECTION)
    {
        // First check: is bot's target targeting bot?
        if (!newTarget)
        {
            // Cast taunt on bot current target if the mob is targeting someone else
            if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK && TAUNT > 0 && m_bot.IsSpellReady(TAUNT) && m_ai.CastSpell(TAUNT, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
        }

        // If tank is on the verge of dying but "I DON'T WANT TO DIE !!! :'-(("
        // TODO: should behaviour (or treshold) be different between elite and normal mobs? We don't want bots to burn such precious cooldown needlessly
        if (m_bot.GetHealthPercent() < 10)
        {
            // Cast Last Stand first because it has lower cooldown
            if (LAST_STAND > 0 && !m_bot.HasAura(LAST_STAND, EFFECT_INDEX_0) && m_ai.CastSpell(LAST_STAND, m_bot) == SPELL_CAST_OK)
            {
                m_ai.TellMaster("I'm using LAST STAND");
                return RETURN_CONTINUE;
            }
            // Cast Shield Wall only if Last Stand is on cooldown and not active
            if (SHIELD_WALL > 0 && (!m_bot.IsSpellReady(LAST_STAND) || LAST_STAND == 0) && !m_bot.HasAura(LAST_STAND, EFFECT_INDEX_0) && !m_bot.HasAura(SHIELD_WALL, EFFECT_INDEX_0) && m_ai.CastSpell(SHIELD_WALL, m_bot) == SPELL_CAST_OK)
            {
                m_ai.TellMaster("I'm using SHIELD WALL");
                return RETURN_CONTINUE;
            }
        }

        // Try to interrupt spell if target is casting one and target is not a worldboss (they are almost all immune to interrupt)
        if (pTarget->IsNonMeleeSpellCasted(true) && !m_ai.IsElite(pTarget, true))
        {
            if (SHIELD_BASH > 0 && m_ai.CastSpell(SHIELD_BASH, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
        }

        //Do not waste rage applying Sunder Armor if it is already stacked 5 times
        if (SUNDER_ARMOR > 0)
        {
            if (!pTarget->HasAura(SUNDER_ARMOR) && m_ai.CastSpell(SUNDER_ARMOR, *pTarget) == SPELL_CAST_OK)   // no stacks: cast it
                return RETURN_CONTINUE;
            else
            {
                SpellAuraHolder* holder = pTarget->GetSpellAuraHolder(SUNDER_ARMOR);
                if (holder && (holder->GetStackAmount() < 5) && m_ai.CastSpell(SUNDER_ARMOR, *pTarget) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
            }
        }

        if (REVENGE > 0 && m_bot.IsSpellReady(REVENGE))
        {
            uint8 base = pTarget->RollMeleeOutcomeAgainst(&m_bot, BASE_ATTACK, SPELL_SCHOOL_MASK_NORMAL);
            uint8 off = pTarget->RollMeleeOutcomeAgainst(&m_bot, OFF_ATTACK, SPELL_SCHOOL_MASK_NORMAL);
            if (base == MELEE_HIT_PARRY || base == MELEE_HIT_DODGE || base == MELEE_HIT_BLOCK || off == MELEE_HIT_PARRY || off == MELEE_HIT_DODGE || off == MELEE_HIT_BLOCK)
                if (m_ai.CastSpell(REVENGE, *pTarget) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
        }
        if (REND > 0 && !pTarget->HasAura(REND, EFFECT_INDEX_0) && m_ai.CastSpell(REND, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;

        if (DEMORALIZING_SHOUT > 0 && !pTarget->HasAura(DEMORALIZING_SHOUT, EFFECT_INDEX_0) && m_ai.CastSpell(DEMORALIZING_SHOUT, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        // check that target is dangerous (elite) before casting shield block: preserve bot cooldowns
        if (SHIELD_BLOCK > 0 && m_ai.IsElite(pTarget) && !m_bot.HasAura(SHIELD_BLOCK, EFFECT_INDEX_0) && m_ai.CastSpell(SHIELD_BLOCK, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        // TODO: only cast disarm if target has equipment?
        if (DISARM > 0 && !pTarget->HasAura(DISARM, EFFECT_INDEX_0) && m_ai.CastSpell(DISARM, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (CONCUSSION_BLOW > 0 && m_bot.IsSpellReady(CONCUSSION_BLOW) && m_ai.CastSpell(CONCUSSION_BLOW, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (SHIELD_SLAM > 0 && m_bot.IsSpellReady(SHIELD_SLAM) && m_ai.CastSpell(SHIELD_SLAM, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
        if (HEROIC_STRIKE > 0 && m_ai.CastSpell(HEROIC_STRIKE, *pTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    /*    case WarriorBattle:
            if (SWEEPING_STRIKES > 0 && m_ai.GetAttackerCount() >= 2 && !m_bot.HasAura(SWEEPING_STRIKES, EFFECT_INDEX_0) && m_ai.CastSpell(SWEEPING_STRIKES, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (INTIMIDATING_SHOUT > 0 && m_ai.GetAttackerCount() > 5 && m_ai.CastSpell(INTIMIDATING_SHOUT, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (CHALLENGING_SHOUT > 0 && pVictim != m_bot && m_ai.GetHealthPercent() > 25 && !pTarget->HasAura(MOCKING_BLOW, EFFECT_INDEX_0) && !pTarget->HasAura(CHALLENGING_SHOUT, EFFECT_INDEX_0) && m_ai.CastSpell(CHALLENGING_SHOUT, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (CLEAVE > 0 && m_ai.CastSpell(CLEAVE, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (PIERCING_HOWL > 0 && && m_ai.GetAttackerCount() >= 3 && !pTarget->HasAura(WAR_STOMP, EFFECT_INDEX_0) && !pTarget->HasAura(PIERCING_HOWL, EFFECT_INDEX_0) && !pTarget->HasAura(SHOCKWAVE, EFFECT_INDEX_0) && !pTarget->HasAura(CONCUSSION_BLOW, EFFECT_INDEX_0) && m_ai.CastSpell(PIERCING_HOWL, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (MOCKING_BLOW > 0 && pVictim != m_bot && m_ai.GetHealthPercent() > 25 && !pTarget->HasAura(MOCKING_BLOW, EFFECT_INDEX_0) && !pTarget->HasAura(CHALLENGING_SHOUT, EFFECT_INDEX_0) && m_ai.CastSpell(MOCKING_BLOW, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_TAUREN && !pTarget->HasAura(WAR_STOMP, EFFECT_INDEX_0) && !pTarget->HasAura(PIERCING_HOWL, EFFECT_INDEX_0) && !pTarget->HasAura(CONCUSSION_BLOW, EFFECT_INDEX_0) && m_ai.CastSpell(WAR_STOMP, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_HUMAN && m_bot.hasUnitState(UNIT_STAT_STUNNED) || m_bot.HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot.HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || m_bot.HasAuraType(SPELL_AURA_MOD_CHARM) && m_ai.CastSpell(EVERY_MAN_FOR_HIMSELF, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_UNDEAD && m_bot.HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot.HasAuraType(SPELL_AURA_MOD_CHARM) && m_ai.CastSpell(WILL_OF_THE_FORSAKEN, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_DWARF && m_bot.HasAuraState(AURA_STATE_DEADLY_POISON) && m_ai.CastSpell(STONEFORM, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_GNOME && m_bot.hasUnitState(UNIT_STAT_STUNNED) || m_bot.HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) && m_ai.CastSpell(ESCAPE_ARTIST, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_ORC && !m_bot.HasAura(BLOOD_FURY, EFFECT_INDEX_0) && m_ai.CastSpell(BLOOD_FURY, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (m_bot.getRace() == RACE_TROLL && !m_bot.HasAura(BERSERKING, EFFECT_INDEX_0) && m_ai.CastSpell(BERSERKING, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            break;*/

    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (m_ai.CastSpell(HEROIC_STRIKE) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

//Buff and rebuff shouts
void PlayerbotWarriorAI::CheckShouts()
{
    if (!m_bot.HasAura(BATTLE_SHOUT, EFFECT_INDEX_0) && m_ai.CastSpell(BATTLE_SHOUT) == SPELL_CAST_OK)
        return;
}

void PlayerbotWarriorAI::DoNonCombatActions()
{
    uint32 spec = m_bot.GetSpec();

    //Stance Check
    if (spec == WARRIOR_SPEC_ARMS && !m_bot.HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
        m_ai.CastSpell(BATTLE_STANCE);
    else if (spec == WARRIOR_SPEC_FURY && !m_bot.HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
        m_ai.CastSpell(BERSERKER_STANCE);
    else if (spec == WARRIOR_SPEC_PROTECTION && !m_bot.HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0))
        m_ai.CastSpell(DEFENSIVE_STANCE);

    // hp check
    if (EatDrinkBandage(false))
        return;

    // Search and apply stones to weapons
    // Mainhand ...
    Item* stone, * weapon;
    weapon = m_bot.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        stone = m_ai.FindStoneFor(weapon);
        if (stone)
        {
            m_ai.UseItem(stone, EQUIPMENT_SLOT_MAINHAND);
            m_ai.SetIgnoreUpdateTime(5);
        }
    }
    //... and offhand (we add a check to avoid trying to apply stone if the warrior is wielding a shield)
    weapon = m_bot.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (weapon && (weapon->GetProto()->InventoryType == INVTYPE_WEAPONOFFHAND || weapon->GetProto()->InventoryType == INVTYPE_WEAPONMAINHAND)
            && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        stone = m_ai.FindStoneFor(weapon);
        if (stone)
        {
            m_ai.UseItem(stone, EQUIPMENT_SLOT_OFFHAND);
            m_ai.SetIgnoreUpdateTime(5);
        }
    }

    // Nothing else to do, Night Elves will cast Shadowmeld to reduce their aggro versus patrols or nearby mobs
    if (SHADOWMELD > 0 && !m_bot.isMovingOrTurning()
        && !m_bot.IsMounted()
        && !m_bot.HasAura(SHADOWMELD, EFFECT_INDEX_0))
    {
        m_ai.CastSpell(SHADOWMELD, m_bot);
    }
} // end DoNonCombatActions

// Match up with "Pull()" below
bool PlayerbotWarriorAI::CanPull()
{
    return m_bot.GetUInt32Value(PLAYER_AMMO_ID) != 0;
}

// Match up with "CanPull()" above
bool PlayerbotWarriorAI::Pull()
{
    // In Classic, Warriors had 3 different spells for shooting with ranged weapons
    // So we need to determine which one to use
    // First step: look for the item equiped in ranged slot
    Item* pItem = m_bot.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!pItem)
    {
        m_ai.TellMaster("I don't have a ranged weapon equiped.");
        return false;
    }

    ItemPrototype const* pProto = pItem->GetProto();
    if (pProto && pProto->Class == ITEM_CLASS_WEAPON)
    {
        switch (pProto->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_BOW:
                SHOOT = SHOOT_BOW;
                break;
            case ITEM_SUBCLASS_WEAPON_GUN:
                SHOOT = SHOOT_GUN;
                break;
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                SHOOT = SHOOT_XBOW;
                break;
            default:
                m_ai.TellMaster("Can't pull: equiped range item is neither a gun, a bow nor a crossbow.");
                return false;
        }
    }
    else
    {
        m_ai.TellMaster("Can't pull: equiped range item is unkown.");
        return false;
    }

    if (m_bot.GetDistance(m_ai.GetCurrentTarget(), true, DIST_CALC_COMBAT_REACH_WITH_MELEE) > ATTACK_DISTANCE)
    {
        if (!m_ai.In_Reach(m_ai.GetCurrentTarget(), SHOOT))
        {
            m_ai.TellMaster("Can't pull: I'm out of range.");
            return false;
        }
        if (!m_bot.IsWithinLOSInMap(m_ai.GetCurrentTarget()))
        {
            m_ai.TellMaster("Can't pull: target is out of sight.");
            return false;
        }

        // shoot at the target
        m_ai.FaceTarget(m_ai.GetCurrentTarget());
        m_bot.CastSpell(m_ai.GetCurrentTarget(), SHOOT, TRIGGERED_OLD_TRIGGERED);
        m_ai.TellMaster("I'm PULLING %s.", m_ai.GetCurrentTarget()->GetName());
        return true;
    }
    else // target is in melee range
    {
        m_ai.Attack(m_ai.GetCurrentTarget());
        return true;
    }

    m_ai.TellMaster("I cannot pull my target for an unkown reason.");
        return false;
}
