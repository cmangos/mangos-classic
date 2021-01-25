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

#include "PlayerbotDruidAI.h"

class PlayerbotAI;

PlayerbotDruidAI::PlayerbotDruidAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    MOONFIRE                      = m_ai.initSpell(MOONFIRE_1); // attacks
    STARFIRE                      = m_ai.initSpell(STARFIRE_1);
    WRATH                         = m_ai.initSpell(WRATH_1);
    ROOTS                         = m_ai.initSpell(ENTANGLING_ROOTS_1);
    INSECT_SWARM                  = m_ai.initSpell(INSECT_SWARM_1);
    MARK_OF_THE_WILD              = m_ai.initSpell(MARK_OF_THE_WILD_1); // buffs
    GIFT_OF_THE_WILD              = m_ai.initSpell(GIFT_OF_THE_WILD_1);
    THORNS                        = m_ai.initSpell(THORNS_1);
    BARKSKIN                      = m_ai.initSpell(BARKSKIN_1);
    HIBERNATE                     = m_ai.initSpell(HIBERNATE_1);
    INNERVATE                     = m_ai.initSpell(INNERVATE_1);
    FAERIE_FIRE                   = m_ai.initSpell(FAERIE_FIRE_1); // debuffs
    FAERIE_FIRE_FERAL             = m_ai.initSpell(FAERIE_FIRE_FERAL_1);
    REJUVENATION                  = m_ai.initSpell(REJUVENATION_1); // heals
    REGROWTH                      = m_ai.initSpell(REGROWTH_1);
    OMEN_OF_CLARITY               = m_ai.initSpell(OMEN_OF_CLARITY_1);
    NATURES_SWIFTNESS             = m_ai.initSpell(NATURES_SWIFTNESS_DRUID_1);
    HEALING_TOUCH                 = m_ai.initSpell(HEALING_TOUCH_1);
    SWIFTMEND                     = m_ai.initSpell(SWIFTMEND_1);
    TRANQUILITY                   = m_ai.initSpell(TRANQUILITY_1);
    REBIRTH                       = m_ai.initSpell(REBIRTH_1);
    REMOVE_CURSE                  = m_ai.initSpell(REMOVE_CURSE_DRUID_1);
    CURE_POISON                   = m_ai.initSpell(CURE_POISON_1);
    ABOLISH_POISON                = m_ai.initSpell(ABOLISH_POISON_1);
    // Druid Forms
    MOONKIN_FORM                  = m_ai.initSpell(MOONKIN_FORM_1);
    DIRE_BEAR_FORM                = m_ai.initSpell(DIRE_BEAR_FORM_1);
    BEAR_FORM                     = m_ai.initSpell(BEAR_FORM_1);
    CAT_FORM                      = m_ai.initSpell(CAT_FORM_1);
    TRAVEL_FORM                   = m_ai.initSpell(TRAVEL_FORM_1);
    // Cat Attack type's
    RAKE                          = m_ai.initSpell(RAKE_1);
    CLAW                          = m_ai.initSpell(CLAW_1); // 45
    COWER                         = m_ai.initSpell(COWER_1); // 20
    SHRED                         = m_ai.initSpell(SHRED_1);
    TIGERS_FURY                   = m_ai.initSpell(TIGERS_FURY_1);
    // Cat Finishing Move's
    RIP                           = m_ai.initSpell(RIP_1); // 30
    FEROCIOUS_BITE                = m_ai.initSpell(FEROCIOUS_BITE_1); // 35
    // Bear/Dire Bear Attacks & Buffs
    BASH                          = m_ai.initSpell(BASH_1);
    MAUL                          = m_ai.initSpell(MAUL_1); // 15
    SWIPE                         = m_ai.initSpell(SWIPE_BEAR_1); // 20
    DEMORALIZING_ROAR             = m_ai.initSpell(DEMORALIZING_ROAR_1); // 10
    CHALLENGING_ROAR              = m_ai.initSpell(CHALLENGING_ROAR_1);
    ENRAGE                        = m_ai.initSpell(ENRAGE_1);
    GROWL                         = m_ai.initSpell(GROWL_1);

    RECENTLY_BANDAGED             = 11196; // first aid check

    // racial
    SHADOWMELD                    = m_ai.initSpell(SHADOWMELD_ALL);
    WAR_STOMP                     = m_ai.initSpell(WAR_STOMP_ALL); // tauren
}

PlayerbotDruidAI::~PlayerbotDruidAI() {}

CombatManeuverReturns PlayerbotDruidAI::DoFirstCombatManeuver(Unit* pTarget)
{
    bool meleeReach = m_bot.CanReachWithMeleeAttack(pTarget);
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.GroupTankHoldsAggro())
        {
            if (PlayerbotAI::ORDERS_TANK & m_ai.GetCombatOrder())
            {
                if (meleeReach)
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
            else if (PlayerbotAI::ORDERS_HEAL & m_ai.GetCombatOrder())
                return _DoNextPVECombatManeuverHeal();
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

CombatManeuverReturns PlayerbotDruidAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotDruidAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotDruidAI::DoNextCombatManeuver(Unit* pTarget)
{
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

CombatManeuverReturns PlayerbotDruidAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    bool meleeReach = m_bot.CanReachWithMeleeAttack(pTarget);

    uint32 spec = m_bot.GetSpec();
    if (spec == 0) // default to spellcasting or healing for healer
        spec = (PlayerbotAI::ORDERS_HEAL & m_ai.GetCombatOrder() ? DRUID_SPEC_RESTORATION : DRUID_SPEC_BALANCE);

    // Make sure healer stays put, don't even melee (aggro) if in range: only melee if feral spec AND not healer
    if (!m_ai.IsHealer() && spec == DRUID_SPEC_FERAL && m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);
    else    // ranged combat in all other cases
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);

    //Unit* pVictim = pTarget->GetVictim();
    uint32 BEAR = (DIRE_BEAR_FORM > 0 ? DIRE_BEAR_FORM : BEAR_FORM);

    // TODO: do something to allow emergency heals for non-healers?
    switch (CheckForms())
    {
        case RETURN_OK_SHIFTING:
            return RETURN_CONTINUE;

        case RETURN_FAIL:
        case RETURN_OK_CANNOTSHIFT:
            if (spec == DRUID_SPEC_FERAL)
                spec = DRUID_SPEC_BALANCE; // Can't shift, force spellcasting
            break; // rest functions without form

        //case RETURN_OK_NOCHANGE: // great!
        //case RETURN_FAIL_WAITINGONSELFBUFF: // This is war dammit! No time for silly buffs during combat...
        default:
            break;
    }

    // Low mana and bot is a caster/healer: cast Innervate on self
    // TODO add group check to also cast on low mana healers or master
    if (m_ai.GetManaPercent() < 15 && ((m_ai.IsHealer() || spec == DRUID_SPEC_RESTORATION)))
        if (INNERVATE > 0 && !m_bot.HasAura(INNERVATE, EFFECT_INDEX_0) && CastSpell(INNERVATE, &m_bot))
            return RETURN_CONTINUE;

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);
    if (newTarget && !(m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK) && !PlayerbotAI::IsNeutralized(newTarget)) // TODO: && party has a tank
    {
        if (HealPlayer(&m_bot) == RETURN_CONTINUE)
            return RETURN_CONTINUE;

        // Aggroed by an elite that came in melee range
        if (m_ai.IsElite(newTarget) && meleeReach)
        {
            // protect the bot with barkskin: the increased casting time is meaningless
            // because bot will then avoid to cast to not angry mob further
            if (m_ai.IsHealer() || spec == DRUID_SPEC_RESTORATION || spec == DRUID_SPEC_BALANCE)
            {
                if (BARKSKIN > 0 && !m_bot.HasAura(BARKSKIN, EFFECT_INDEX_0) && CastSpell(BARKSKIN, &m_bot))
                    return RETURN_CONTINUE;

                return RETURN_NO_ACTION_OK;
            }
            //no other cases: cats have cower in the damage rotation and bears can tank
        }
    }

    if (m_ai.IsHealer())
        if (_DoNextPVECombatManeuverHeal() & RETURN_CONTINUE)
            return RETURN_CONTINUE;

    switch (spec)
    {
        case DRUID_SPEC_FERAL:
            if (BEAR > 0 && m_bot.HasAura(BEAR))
                return _DoNextPVECombatManeuverBear(pTarget);
            if (CAT_FORM > 0 && m_bot.HasAura(CAT_FORM))
                return _DoNextPVECombatManeuverCat(pTarget);
        // NO break - failover to DRUID_SPEC_BALANCE

        case DRUID_SPEC_RESTORATION: // There is no Resto DAMAGE rotation. If you insist, go Balance...
        case DRUID_SPEC_BALANCE:
            if (m_bot.HasAura(BEAR) || m_bot.HasAura(CAT_FORM))
                return RETURN_NO_ACTION_UNKNOWN; // Didn't shift out of inappropriate form

            return _DoNextPVECombatManeuverSpellDPS(pTarget);

            /*if (BASH > 0 && !pTarget->HasAura(BASH, EFFECT_INDEX_0) && DruidSpellCombat < 5 && CastSpell(BASH, pTarget))
                return RETURN_CONTINUE;
            if (CHALLENGING_ROAR > 0 && pVictim != m_bot && !pTarget->HasAura(CHALLENGING_ROAR, EFFECT_INDEX_0) && !pTarget->HasAura(GROWL, EFFECT_INDEX_0) && CastSpell(CHALLENGING_ROAR, pTarget))
                return RETURN_CONTINUE;
            if (ROOTS > 0 && !pTarget->HasAura(ROOTS, EFFECT_INDEX_0) && CastSpell(ROOTS, pTarget))
                return RETURN_CONTINUE;
            */
    }

    return RETURN_NO_ACTION_UNKNOWN;
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotDruidAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (m_ai.CastSpell(MOONFIRE) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

CombatManeuverReturns PlayerbotDruidAI::_DoNextPVECombatManeuverBear(Unit* pTarget)
{
    if (!m_bot.HasAura((DIRE_BEAR_FORM > 0 ? DIRE_BEAR_FORM : BEAR_FORM))) return RETURN_NO_ACTION_ERROR;

    // Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);

    // Face enemy, make sure you're attacking
    m_ai.FaceTarget(pTarget);

    if (PlayerbotAI::ORDERS_TANK & m_ai.GetCombatOrder() && !newTarget && GROWL > 0 && m_bot.IsSpellReady(GROWL))
        if (CastSpell(GROWL, pTarget))
            return RETURN_CONTINUE;

    if (FAERIE_FIRE_FERAL > 0 && m_ai.In_Reach(pTarget, FAERIE_FIRE_FERAL) && !pTarget->HasAura(FAERIE_FIRE_FERAL, EFFECT_INDEX_0))
        if (CastSpell(FAERIE_FIRE_FERAL, pTarget))
            return RETURN_CONTINUE;

    if (SWIPE > 0 && m_ai.In_Reach(pTarget, SWIPE) && m_ai.GetAttackerCount() >= 2 && CastSpell(SWIPE, pTarget))
        return RETURN_CONTINUE;

    if (ENRAGE > 0 && m_bot.IsSpellReady(ENRAGE) && CastSpell(ENRAGE, &m_bot))
        return RETURN_CONTINUE;

    if (DEMORALIZING_ROAR > 0 && !pTarget->HasAura(DEMORALIZING_ROAR, EFFECT_INDEX_0) && CastSpell(DEMORALIZING_ROAR, pTarget))
        return RETURN_CONTINUE;

    if (MAUL > 0 && CastSpell(MAUL, pTarget))
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
}

CombatManeuverReturns PlayerbotDruidAI::_DoNextPVECombatManeuverCat(Unit* pTarget)
{
    if (!m_bot.HasAura(CAT_FORM)) return RETURN_NO_ACTION_UNKNOWN;

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);

    // Face enemy, make sure you're attacking
    m_ai.FaceTarget(pTarget);

    // Attempt to do a finishing move
    if (m_bot.GetComboPoints() >= 5)
    {
        if (RIP > 0 && !pTarget->HasAura(RIP, EFFECT_INDEX_0))
        {
            if (CastSpell(RIP, pTarget))
                return RETURN_CONTINUE;
        }
        // 35 Energy
        else if (FEROCIOUS_BITE > 0)
        {
            if (CastSpell(FEROCIOUS_BITE, pTarget))
                return RETURN_CONTINUE;
        }
    } // End 5 ComboPoints

    if (newTarget && COWER > 0 && m_bot.IsSpellReady(COWER) && CastSpell(COWER, pTarget))
        return RETURN_CONTINUE;

    if (SHRED > 0 && pTarget->isInBackInMap(&m_bot, 5.0f) && m_ai.CastSpell(SHRED, *pTarget) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    if (FAERIE_FIRE_FERAL > 0 && m_ai.In_Reach(pTarget, FAERIE_FIRE_FERAL) && !pTarget->HasAura(FAERIE_FIRE_FERAL, EFFECT_INDEX_0) && CastSpell(FAERIE_FIRE_FERAL, pTarget))
        return RETURN_CONTINUE;

    if (TIGERS_FURY > 0 && m_bot.IsSpellReady(TIGERS_FURY) && !m_bot.HasAura(TIGERS_FURY, EFFECT_INDEX_0) && CastSpell(TIGERS_FURY))
        return RETURN_CONTINUE;

    if (RAKE > 0 && !pTarget->HasAura(RAKE) && CastSpell(RAKE, pTarget))
        return RETURN_CONTINUE;

    if (CLAW > 0 && CastSpell(CLAW, pTarget))
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
}

CombatManeuverReturns PlayerbotDruidAI::_DoNextPVECombatManeuverSpellDPS(Unit* pTarget)
{
    uint32 NATURE = (STARFIRE > 0 ? STARFIRE : WRATH);

    // Dispel curse/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Combat resurrection (only tanks or master. If other targets are required, let master explicitly ask to)
    if (ResurrectPlayer(GetResurrectionTarget(JOB_TANK_MASTER, false)) & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Face enemy, make sure you're attacking
    m_ai.FaceTarget(pTarget);

    if (FAERIE_FIRE > 0 && m_ai.In_Reach(pTarget, FAERIE_FIRE) && !pTarget->HasAura(FAERIE_FIRE, EFFECT_INDEX_0) && CastSpell(FAERIE_FIRE, pTarget))
        return RETURN_CONTINUE;

    if (INSECT_SWARM > 0 && m_ai.In_Reach(pTarget, INSECT_SWARM) && !pTarget->HasAura(INSECT_SWARM, EFFECT_INDEX_0) && CastSpell(INSECT_SWARM, pTarget))
        return RETURN_CONTINUE;

    // Healer? Don't waste more mana on DPS
    if (m_ai.IsHealer())
        return RETURN_NO_ACTION_OK;

    if (MOONFIRE > 0 && m_ai.In_Reach(pTarget, MOONFIRE) && !pTarget->HasAura(MOONFIRE, EFFECT_INDEX_0) && CastSpell(MOONFIRE, pTarget))
        return RETURN_CONTINUE;

    if (NATURE > 0 && CastSpell(NATURE, pTarget))
        return RETURN_CONTINUE;

    if (m_ai.GetCombatStyle() == PlayerbotAI::COMBAT_MELEE)
        m_bot.Attack(pTarget, true);
    else
        m_bot.AttackStop();

    return RETURN_NO_ACTION_UNKNOWN;
}

CombatManeuverReturns PlayerbotDruidAI::_DoNextPVECombatManeuverHeal()
{
    // Dispel curse/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Combat resurrection (only tanks or master. If other targets are required, let master explicitly requests it)
    if (ResurrectPlayer(GetResurrectionTarget(JOB_TANK_MASTER, false)) & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Heal (try to pick a target by on common rules, than heal using each PlayerbotClassAI HealPlayer() method)
    if (FindTargetAndHeal())
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
}

CombatManeuverReturns PlayerbotDruidAI::HealPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::HealPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    uint8 hp = target->GetHealthPercent();

    // If target is out of range (40 yards) and is a tank: move towards it
    // if bot is not asked to stay
    // Other classes have to adjust their position to the healers
    // TODO: This code should be common to all healers and will probably
    // move to a more suitable place like PlayerbotAI::DoCombatMovement()
    if ((GetTargetJob(target) == JOB_TANK || GetTargetJob(target) == JOB_MAIN_TANK)
            && m_bot.GetPlayerbotAI()->GetMovementOrder() != PlayerbotAI::MOVEMENT_STAY
            && !m_ai.In_Reach(target, HEALING_TOUCH))
    {
        m_bot.GetMotionMaster()->MoveFollow(target, 39.0f, m_bot.GetOrientation());
        return RETURN_CONTINUE;
    }

    // Everyone is healthy enough, return OK. MUST correlate to highest value below (should be last HP check)
    if (hp >= 80)
        return RETURN_NO_ACTION_OK;

    // Start heals. Do lowest HP checks at the top

    // Emergency heal: target needs to be healed NOW!
    if (((GetTargetJob(target) == JOB_TANK || GetTargetJob(target) == JOB_MAIN_TANK) && hp < 10)
            || (GetTargetJob(target) != JOB_TANK && GetTargetJob(target) != JOB_MAIN_TANK && hp < 15))
    {
        // first try Nature's Swiftness + Healing Touch: instant heal
        if (NATURES_SWIFTNESS > 0 && m_bot.IsSpellReady(NATURES_SWIFTNESS))
            CastSpell(NATURES_SWIFTNESS, &m_bot);

        if (HEALING_TOUCH > 0 && m_bot.HasAura(NATURES_SWIFTNESS, EFFECT_INDEX_0) && m_ai.In_Reach(target, HEALING_TOUCH) && CastSpell(HEALING_TOUCH, target))
            return RETURN_CONTINUE;

        // Else try to Swiftmend the target if druid HoT is active on it
        if (SWIFTMEND > 0 && m_bot.IsSpellReady(SWIFTMEND) && m_ai.In_Reach(target, SWIFTMEND) && (target->HasAura(REJUVENATION) || target->HasAura(REGROWTH)) && CastSpell(SWIFTMEND, target))
            return RETURN_CONTINUE;
    }

    // Urgent heal: target won't die next second, but first bot needs to gain some time to cast Healing Touch safely
    if (((GetTargetJob(target) == JOB_TANK || GetTargetJob(target) == JOB_MAIN_TANK) && hp < 15)
            || (GetTargetJob(target) != JOB_TANK && GetTargetJob(target) != JOB_MAIN_TANK && hp < 25))
    {
        if (REGROWTH > 0 && m_ai.In_Reach(target, REGROWTH) && !target->HasAura(REGROWTH) && CastSpell(REGROWTH, target))
            return RETURN_CONTINUE;
        if (REJUVENATION > 0 && m_ai.In_Reach(target, REJUVENATION) && target->HasAura(REGROWTH) && !target->HasAura(REJUVENATION) && CastSpell(REJUVENATION, target))
            return RETURN_CONTINUE;
        if (SWIFTMEND > 0 && m_bot.IsSpellReady(SWIFTMEND) && m_ai.In_Reach(target, SWIFTMEND) && (target->HasAura(REJUVENATION) || target->HasAura(REGROWTH)) && CastSpell(SWIFTMEND, target))
            return RETURN_CONTINUE;
    }

    if (hp < 60 && HEALING_TOUCH > 0 && m_ai.In_Reach(target, HEALING_TOUCH) && CastSpell(HEALING_TOUCH, target))
        return RETURN_CONTINUE;

    if (hp < 80 && REJUVENATION > 0 && m_ai.In_Reach(target, REJUVENATION) && !target->HasAura(REJUVENATION) && CastSpell(REJUVENATION, target))
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
} // end HealTarget

CombatManeuverReturns PlayerbotDruidAI::ResurrectPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::ResurrectPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    if (REBIRTH > 0 && m_ai.In_Reach(target, REBIRTH) && m_bot.IsSpellReady(REBIRTH) && m_ai.CastSpell(REBIRTH, *target) == SPELL_CAST_OK)
    {
        std::string msg = "Resurrecting ";
        msg += target->GetName();
        m_bot.Say(msg, LANG_UNIVERSAL);
        return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_ERROR; // not error per se - possibly just OOM
}

CombatManeuverReturns PlayerbotDruidAI::DispelPlayer(Player* /*target*/)
{
    // Remove curse on group members
    if (Player* cursedTarget = GetDispelTarget(DISPEL_CURSE))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(cursedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (REMOVE_CURSE > 0 && CastSpell(REMOVE_CURSE, cursedTarget))
            return RETURN_CONTINUE;
    }

    // Remove poison on group members
    if (Player* poisonedTarget = GetDispelTarget(DISPEL_POISON))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(poisonedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        uint32 cure = ABOLISH_POISON > 0 ? ABOLISH_POISON : CURE_POISON;
        if (cure > 0 && CastSpell(cure, poisonedTarget))
            return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_OK;
}

/**
* CheckForms()
*
* Returns bool - Value indicates success - shape was shifted, already shifted, no need to shift.
*/
uint8 PlayerbotDruidAI::CheckForms()
{
    uint32 spec = m_bot.GetSpec();
    uint32 BEAR = (DIRE_BEAR_FORM > 0 ? DIRE_BEAR_FORM : BEAR_FORM);

    // if bot has healing orders always shift to humanoid form
    // regardless of spec
    if (m_ai.IsHealer() || spec == DRUID_SPEC_RESTORATION)
    {
        if (m_bot.HasAura(CAT_FORM, EFFECT_INDEX_0))
        {
            m_bot.RemoveAurasDueToSpell(CAT_FORM_1);
            //m_ai.TellMaster("FormClearCat");
            return RETURN_OK_SHIFTING;
        }
        if (m_bot.HasAura(BEAR_FORM, EFFECT_INDEX_0))
        {
            m_bot.RemoveAurasDueToSpell(BEAR_FORM_1);
            //m_ai.TellMaster("FormClearBear");
            return RETURN_OK_SHIFTING;
        }
        if (m_bot.HasAura(DIRE_BEAR_FORM, EFFECT_INDEX_0))
        {
            m_bot.RemoveAurasDueToSpell(DIRE_BEAR_FORM_1);
            //m_ai.TellMaster("FormClearDireBear");
            return RETURN_OK_SHIFTING;
        }
        // spellcasting form, but disables healing spells so it's got to go
        if (m_bot.HasAura(MOONKIN_FORM, EFFECT_INDEX_0))
        {
            m_bot.RemoveAurasDueToSpell(MOONKIN_FORM_1);
            //m_ai.TellMaster("FormClearMoonkin");
            return RETURN_OK_SHIFTING;
        }

        return RETURN_OK_NOCHANGE;
    }

    if (spec == DRUID_SPEC_BALANCE)
    {
        if (m_bot.HasAura(MOONKIN_FORM))
            return RETURN_OK_NOCHANGE;

        if (!MOONKIN_FORM)
            return RETURN_OK_CANNOTSHIFT;

        if (CastSpell(MOONKIN_FORM))
            return RETURN_OK_SHIFTING;
        else
            return RETURN_FAIL;
    }

    if (spec == DRUID_SPEC_FERAL)
    {
        // Use Bear form only if we are told we're a tank and have thorns up
        if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK)
        {
            if (m_bot.HasAura(BEAR))
                return RETURN_OK_NOCHANGE;

            if (!BEAR)
                return RETURN_OK_CANNOTSHIFT;

            if (!m_bot.HasAura(THORNS))
                return RETURN_FAIL_WAITINGONSELFBUFF;

            if (CastSpell(BEAR))
                return RETURN_OK_SHIFTING;
            else
                return RETURN_FAIL;
        }
        else // No tank orders - try to go kitty or at least bear
        {
            if (CAT_FORM > 0)
            {
                if (m_bot.HasAura(CAT_FORM))
                    return RETURN_OK_NOCHANGE;

                if (CastSpell(CAT_FORM))
                    return RETURN_OK_SHIFTING;
                else
                    return RETURN_FAIL;
            }

            if (BEAR > 0)
            {
                if (m_bot.HasAura(BEAR))
                    return RETURN_OK_NOCHANGE;

                if (CastSpell(BEAR))
                    return RETURN_OK_SHIFTING;
                else
                    return RETURN_FAIL;
            }

            return RETURN_OK_CANNOTSHIFT;
        }
    }

    // Unknown Spec
    return RETURN_FAIL;
}

void PlayerbotDruidAI::DoNonCombatActions()
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return;

    // Revive
    // No auto-revive out of combat to preserve cooldown. Let master explicitly ask bot to cast Rebirth if needed

    // Dispel curse/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
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

    // Buff group
    // the check for group targets is performed by NeedGroupBuff (if group is found for bots by the function)
    if (NeedGroupBuff(GIFT_OF_THE_WILD, MARK_OF_THE_WILD) && m_ai.HasSpellReagents(GIFT_OF_THE_WILD))
    {
        if (Buff(&PlayerbotDruidAI::BuffHelper, GIFT_OF_THE_WILD) & RETURN_CONTINUE)
            return;
    }
    else if (Buff(&PlayerbotDruidAI::BuffHelper, MARK_OF_THE_WILD) & RETURN_CONTINUE)
        return;
    if (Buff(&PlayerbotDruidAI::BuffHelper, THORNS, (m_bot.GetGroup() ? JOB_TANK | JOB_MAIN_TANK : JOB_ALL)) & RETURN_CONTINUE)
        return;
    if (OMEN_OF_CLARITY > 0 && !m_bot.HasAura(OMEN_OF_CLARITY) && CastSpell(OMEN_OF_CLARITY, &m_bot))
        return;

    // hp/mana check
    if (EatDrinkBandage())
        return;

    if (INNERVATE > 0 && m_ai.In_Reach(&m_bot, INNERVATE) && !m_bot.HasAura(INNERVATE) && m_ai.GetManaPercent() <= 20 && CastSpell(INNERVATE, &m_bot))
        return;

    // Return to fighting form AFTER reviving, healing, buffing
    CheckForms();

    // Nothing else to do, Night Elves will cast Shadowmeld to reduce their aggro versus patrols or nearby mobs
    if (SHADOWMELD > 0 && !m_bot.isMovingOrTurning()
        && !m_bot.IsMounted()
        && !m_bot.HasAura(SHADOWMELD, EFFECT_INDEX_0))
    {
        m_ai.CastSpell(SHADOWMELD, m_bot);
    }
} // end DoNonCombatActions

bool PlayerbotDruidAI::BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target)
{
    if (!ai)          return false;
    if (spellId == 0) return false;
    if (!target)      return false;

    Pet* pet = target->GetPet();
    if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->Buff(spellId, pet, &(PlayerbotDruidAI::GoBuffForm)) == SPELL_CAST_OK)
        return true;

    return ai->Buff(spellId, target, &(PlayerbotDruidAI::GoBuffForm)) == SPELL_CAST_OK;
}

void PlayerbotDruidAI::GoBuffForm(Player* self)
{
    // RANK_1 spell ids used because this is a static method which does not have access to instance.
    // There is only one rank for these spells anyway.
    if (self->HasAura(CAT_FORM_1))
        self->RemoveAurasDueToSpell(CAT_FORM_1);
    if (self->HasAura(BEAR_FORM_1))
        self->RemoveAurasDueToSpell(BEAR_FORM_1);
    if (self->HasAura(DIRE_BEAR_FORM_1))
        self->RemoveAurasDueToSpell(DIRE_BEAR_FORM_1);
    if (self->HasAura(MOONKIN_FORM_1))
        self->RemoveAurasDueToSpell(MOONKIN_FORM_1);
    if (self->HasAura(TRAVEL_FORM_1))
        self->RemoveAurasDueToSpell(TRAVEL_FORM_1);
}

// Match up with "Pull()" below
bool PlayerbotDruidAI::CanPull()
{
    return BEAR_FORM > 0 && FAERIE_FIRE_FERAL;
}

// Match up with "CanPull()" above
bool PlayerbotDruidAI::Pull()
{
    return BEAR_FORM > 0 && (CastSpell(FAERIE_FIRE_FERAL) & RETURN_CONTINUE);
}

bool PlayerbotDruidAI::CastHoTOnTank()
{
    if ((PlayerbotAI::ORDERS_HEAL & m_ai.GetCombatOrder()) == 0) return false;

    // Druid HoTs: Rejuvenation, Regrowth, Tranquility (channeled, AoE)
    if (REJUVENATION > 0)
        return (RETURN_CONTINUE & CastSpell(REJUVENATION, m_ai.GetGroupTank()));

    return false;
}

// Return to UpdateAI the spellId usable to neutralize a target with creaturetype
uint32 PlayerbotDruidAI::Neutralize(uint8 creatureType)
{
    if (!creatureType)  return 0;

    if (creatureType != CREATURE_TYPE_DRAGONKIN && creatureType != CREATURE_TYPE_BEAST)
    {
        m_ai.TellMaster("I can't make that target hibernate.");
        return 0;
    }

    if (HIBERNATE > 0)
        return HIBERNATE;
    else
        return 0;
}
