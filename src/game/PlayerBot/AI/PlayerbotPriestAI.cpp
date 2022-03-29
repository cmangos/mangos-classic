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

#include "PlayerbotPriestAI.h"

class PlayerbotAI;

PlayerbotPriestAI::PlayerbotPriestAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    RENEW                         = m_ai.initSpell(RENEW_1);
    LESSER_HEAL                   = m_ai.initSpell(LESSER_HEAL_1);
    FLASH_HEAL                    = m_ai.initSpell(FLASH_HEAL_1);
    (FLASH_HEAL > 0) ? FLASH_HEAL : FLASH_HEAL = LESSER_HEAL;
    HEAL                          = m_ai.initSpell(HEAL_1);
    (HEAL > 0) ? HEAL : HEAL = FLASH_HEAL;
    GREATER_HEAL                  = m_ai.initSpell(GREATER_HEAL_1);
    (GREATER_HEAL > 0) ? GREATER_HEAL : GREATER_HEAL = HEAL;
    RESURRECTION                  = m_ai.initSpell(RESURRECTION_1);
    SMITE                         = m_ai.initSpell(SMITE_1);
    MANA_BURN                     = m_ai.initSpell(MANA_BURN_1);
    HOLY_NOVA                     = m_ai.initSpell(HOLY_NOVA_1);
    HOLY_FIRE                     = m_ai.initSpell(HOLY_FIRE_1);
    DESPERATE_PRAYER              = m_ai.initSpell(DESPERATE_PRAYER_1);
    PRAYER_OF_HEALING             = m_ai.initSpell(PRAYER_OF_HEALING_1);
    CURE_DISEASE                  = m_ai.initSpell(CURE_DISEASE_1);
    ABOLISH_DISEASE               = m_ai.initSpell(ABOLISH_DISEASE_1);
    SHACKLE_UNDEAD                = m_ai.initSpell(SHACKLE_UNDEAD_1);

    // SHADOW
    FADE                          = m_ai.initSpell(FADE_1);
    SHADOW_WORD_PAIN              = m_ai.initSpell(SHADOW_WORD_PAIN_1);
    MIND_BLAST                    = m_ai.initSpell(MIND_BLAST_1);
    SCREAM                        = m_ai.initSpell(PSYCHIC_SCREAM_1);
    MIND_FLAY                     = m_ai.initSpell(MIND_FLAY_1);
    DEVOURING_PLAGUE              = m_ai.initSpell(DEVOURING_PLAGUE_1);
    SHADOW_PROTECTION             = m_ai.initSpell(SHADOW_PROTECTION_1);
    PRAYER_OF_SHADOW_PROTECTION   = m_ai.initSpell(PRAYER_OF_SHADOW_PROTECTION_1);
    SHADOWFORM                    = m_ai.initSpell(SHADOWFORM_1);
    VAMPIRIC_EMBRACE              = m_ai.initSpell(VAMPIRIC_EMBRACE_1);

    // RANGED COMBAT
    SHOOT                         = m_ai.initSpell(SHOOT_1);

    // DISCIPLINE
    INNER_FIRE                    = m_ai.initSpell(INNER_FIRE_1);
    POWER_WORD_SHIELD             = m_ai.initSpell(POWER_WORD_SHIELD_1);
    POWER_WORD_FORTITUDE          = m_ai.initSpell(POWER_WORD_FORTITUDE_1);
    PRAYER_OF_FORTITUDE           = m_ai.initSpell(PRAYER_OF_FORTITUDE_1);
    FEAR_WARD                     = m_ai.initSpell(FEAR_WARD_1);
    DIVINE_SPIRIT                 = m_ai.initSpell(DIVINE_SPIRIT_1);
    PRAYER_OF_SPIRIT              = m_ai.initSpell(PRAYER_OF_SPIRIT_1);
    POWER_INFUSION                = m_ai.initSpell(POWER_INFUSION_1);
    INNER_FOCUS                   = m_ai.initSpell(INNER_FOCUS_1);
    PRIEST_DISPEL_MAGIC           = m_ai.initSpell(DISPEL_MAGIC_1);

    RECENTLY_BANDAGED             = 11196; // first aid check

    // racial
    STONEFORM                     = m_ai.initSpell(STONEFORM_ALL); // dwarf
    ELUNES_GRACE                  = m_ai.initSpell(ELUNES_GRACE_1); // night elf
    PERCEPTION                    = m_ai.initSpell(PERCEPTION_ALL); // human
    SHADOWMELD                    = m_ai.initSpell(SHADOWMELD_ALL);
    BERSERKING                    = m_ai.initSpell(BERSERKING_ALL); // troll
    WILL_OF_THE_FORSAKEN          = m_ai.initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead
}

PlayerbotPriestAI::~PlayerbotPriestAI() {}

CombatManeuverReturns PlayerbotPriestAI::DoFirstCombatManeuver(Unit* pTarget)
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

CombatManeuverReturns PlayerbotPriestAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    if (m_ai.IsHealer())
    {
        // Cast renew on tank
        if (CastHoTOnTank())
            return RETURN_FINISHED_FIRST_MOVES;
    }
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotPriestAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotPriestAI::DoNextCombatManeuver(Unit* pTarget)
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

CombatManeuverReturns PlayerbotPriestAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    bool meleeReach = m_bot.CanReachWithMeleeAttack(pTarget);
    uint32 spec = m_bot.GetSpec();

    // Define a tank bot will look at
    // or Main Tank if healer has order to do so
    Unit* mainTank;
    if (m_ai.IsMainHealer())
        mainTank = GetHealTarget(JOB_MAIN_TANK);
    // Else look at tank from own group if one
    else
        mainTank = GetHealTarget(JOB_TANK, true);
    // Finally get any tank
    if (!mainTank)
        mainTank = GetHealTarget(JOB_TANK);

    if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_RANGED && !meleeReach)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    // switch to melee if in melee range AND can't shoot OR have no ranged (wand) equipped AND is not healer
    else if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE
             && meleeReach
             && (SHOOT == 0 || !m_bot.GetWeaponForAttack(RANGED_ATTACK, true, true))
             && !m_ai.IsHealer())
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    // Dwarves priests will try to buff with Fear Ward
    if (FEAR_WARD > 0 && m_bot.IsSpellReady(FEAR_WARD))
    {
        // Buff tank first
        if (mainTank)
        {
            if (m_ai.In_Reach(mainTank, FEAR_WARD) && !mainTank->HasAura(FEAR_WARD, EFFECT_INDEX_0) && CastSpell(FEAR_WARD, mainTank))
                return RETURN_CONTINUE;
        }
        // Else try to buff master
        if (m_ai.In_Reach(&m_master, FEAR_WARD) && !m_master.HasAura(FEAR_WARD, EFFECT_INDEX_0) && CastSpell(FEAR_WARD, &m_master))
            return RETURN_CONTINUE;
    }

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);
    if (newTarget && !PlayerbotAI::IsNeutralized(newTarget)) // TODO: && party has a tank
    {
        if (FADE > 0 && !m_bot.HasAura(FADE, EFFECT_INDEX_0) && m_bot.IsSpellReady(FADE))
        {
            if (CastSpell(FADE, &m_bot))
            {
                m_ai.TellMaster("I'm casting fade.");
                return RETURN_CONTINUE;
            }
            else
                m_ai.TellMaster("I have AGGRO.");
        }

        // Heal myself
        // TODO: move to HealTarget code
        if (m_ai.GetHealthPercent() < 35 && POWER_WORD_SHIELD > 0 && !m_bot.HasAura(POWER_WORD_SHIELD, EFFECT_INDEX_0) && !m_bot.HasAura(WEAKNED_SOUL, EFFECT_INDEX_0))
        {
            if (CastSpell(POWER_WORD_SHIELD) & RETURN_CONTINUE)
            {
                m_ai.TellMaster("I'm casting PW:S on myself.");
                return RETURN_CONTINUE;
            }
            else if (m_ai.IsHealer()) // Even if any other RETURN_ANY_OK - aside from RETURN_CONTINUE
                m_ai.TellMaster("Your healer's about TO DIE. HELP ME.");
        }
        if (m_ai.GetHealthPercent() < 35 && DESPERATE_PRAYER > 0 && m_ai.In_Reach(&m_bot, DESPERATE_PRAYER) && CastSpell(DESPERATE_PRAYER, &m_bot) & RETURN_CONTINUE)
        {
            m_ai.TellMaster("I'm casting desperate prayer.");
            return RETURN_CONTINUE;
        }
        // Night Elves priest bot can also cast Elune's Grace to improve his/her dodge rating
        if (ELUNES_GRACE && !m_bot.HasAura(ELUNES_GRACE, EFFECT_INDEX_0) && m_bot.IsSpellReady(ELUNES_GRACE) && CastSpell(ELUNES_GRACE, &m_bot))
            return RETURN_CONTINUE;

        // If enemy comes in melee reach
        if (meleeReach)
        {
            // Already healed self or tank. If healer, do nothing else to anger mob
            if (m_ai.IsHealer())
                return RETURN_NO_ACTION_OK; // In a sense, mission accomplished.

            // Have threat, can't quickly lower it. 3 options remain: Stop attacking, lowlevel damage (wand), keep on keeping on.
            if (newTarget->GetHealthPercent() > 25)
            {
                // If elite, do nothing and pray tank gets aggro off you
                if (m_ai.IsElite(newTarget))
                    return RETURN_NO_ACTION_OK;

                // Not an elite. You could insert PSYCHIC SCREAM here but in any PvE situation that's 90-95% likely
                // to worsen the situation for the group. ... So please don't.
                return CastSpell(SHOOT, pTarget);
            }
        }
    }

    // Dispel magic/disease
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Damage tweaking for healers
    if (m_ai.IsHealer())
    {
        // Heal (try to pick a target by on common rules, than heal using each PlayerbotClassAI HealPlayer() method)
        if (FindTargetAndHeal())
            return RETURN_CONTINUE;

        // No one needs to be healed: do small damage instead
        // If target is elite and not handled by MT: do nothing
        if (m_ai.IsElite(pTarget) && mainTank && mainTank->GetVictim() != pTarget)
            return RETURN_NO_ACTION_OK;

        // Cast Shadow Word:Pain on current target and keep its up (if mana >= 40% or target HP < 15%)
        if (SHADOW_WORD_PAIN > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SHADOW_WORD_PAIN) && !pTarget->HasAura(SHADOW_WORD_PAIN, EFFECT_INDEX_0) &&
                (pTarget->GetHealthPercent() < 15 || m_ai.GetManaPercent() >= 40) && CastSpell(SHADOW_WORD_PAIN, pTarget))
            return RETURN_CONTINUE;
        else // else shoot at it
            return CastSpell(SHOOT, pTarget);
    }

    // Damage Spells
    switch (spec)
    {
        case PRIEST_SPEC_HOLY:
            if (HOLY_FIRE > 0 && m_ai.In_Reach(pTarget, HOLY_FIRE) && !pTarget->HasAura(HOLY_FIRE, EFFECT_INDEX_0) && CastSpell(HOLY_FIRE, pTarget))
                return RETURN_CONTINUE;
            if (SMITE > 0 && m_ai.In_Reach(pTarget, SMITE) && CastSpell(SMITE, pTarget))
                return RETURN_CONTINUE;
            //if (HOLY_NOVA > 0 && m_ai.In_Reach(pTarget,HOLY_NOVA) && meleeReach && m_ai.CastSpell(HOLY_NOVA) == SPELL_CAST_OK)
            //    return RETURN_CONTINUE;
            break;

        case PRIEST_SPEC_SHADOW:
            if (DEVOURING_PLAGUE > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, DEVOURING_PLAGUE) && !pTarget->HasAura(DEVOURING_PLAGUE, EFFECT_INDEX_0) && CastSpell(DEVOURING_PLAGUE, pTarget))
                return RETURN_CONTINUE;
            if (SHADOW_WORD_PAIN > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SHADOW_WORD_PAIN) && !pTarget->HasAura(SHADOW_WORD_PAIN, EFFECT_INDEX_0) && CastSpell(SHADOW_WORD_PAIN, pTarget))
                return RETURN_CONTINUE;
            if (MIND_BLAST > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, MIND_BLAST) && (m_bot.IsSpellReady(MIND_BLAST)) && CastSpell(MIND_BLAST, pTarget))
                return RETURN_CONTINUE;
            if (MIND_FLAY > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, MIND_FLAY) && CastSpell(MIND_FLAY, pTarget))
            {
                m_ai.SetIgnoreUpdateTime(3);
                return RETURN_CONTINUE;
            }
            if (SHADOWFORM == 0 && MIND_FLAY == 0 && SMITE > 0 && m_ai.In_Reach(pTarget, SMITE) && CastSpell(SMITE, pTarget)) // low levels
                return RETURN_CONTINUE;
            break;

        case PRIEST_SPEC_DISCIPLINE:
            if (POWER_INFUSION > 0 && m_ai.In_Reach(&m_master, POWER_INFUSION) && CastSpell(POWER_INFUSION, &m_master)) // TODO: just master?
                return RETURN_CONTINUE;
            if (INNER_FOCUS > 0 && m_ai.In_Reach(&m_bot, INNER_FOCUS) && !m_bot.HasAura(INNER_FOCUS, EFFECT_INDEX_0) && CastSpell(INNER_FOCUS, &m_bot))
                return RETURN_CONTINUE;
            if (SMITE > 0 && m_ai.In_Reach(pTarget, SMITE) && CastSpell(SMITE, pTarget))
                return RETURN_CONTINUE;
            break;
    }

    // No spec due to low level OR no spell found yet
    if (MIND_BLAST > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, MIND_BLAST) && (m_bot.IsSpellReady(MIND_BLAST)) && CastSpell(MIND_BLAST, pTarget))
        return RETURN_CONTINUE;
    if (SHADOW_WORD_PAIN > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SHADOW_WORD_PAIN) && !pTarget->HasAura(SHADOW_WORD_PAIN, EFFECT_INDEX_0) && CastSpell(SHADOW_WORD_PAIN, pTarget))
        return RETURN_CONTINUE;
    if (MIND_FLAY > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, MIND_FLAY) && CastSpell(MIND_FLAY, pTarget))
    {
        m_ai.SetIgnoreUpdateTime(3);
        return RETURN_CONTINUE;
    }
    if (SHADOWFORM == 0 && SMITE > 0 && m_ai.In_Reach(pTarget, SMITE) && CastSpell(SMITE, pTarget))
        return RETURN_CONTINUE;

    // Default: shoot with wand
    return CastSpell(SHOOT, pTarget);
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotPriestAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    switch (m_ai.GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
            // TODO: spec tweaking
            if (m_ai.HasAura(SCREAM, *pTarget) && m_ai.GetHealthPercent() < 60 && HEAL && m_ai.In_Reach(pTarget, HEAL) && CastSpell(HEAL) & RETURN_ANY_OK)
                return RETURN_CONTINUE;

            if (SHADOW_WORD_PAIN && m_ai.In_Reach(pTarget, SHADOW_WORD_PAIN) && CastSpell(SHADOW_WORD_PAIN) & RETURN_ANY_OK) // TODO: Check whether enemy has it active yet
                return RETURN_CONTINUE;

            if (m_ai.GetHealthPercent() < 80 && RENEW && m_ai.In_Reach(pTarget, RENEW) && CastSpell(RENEW) & RETURN_ANY_OK) // TODO: Check whether you have renew active on you
                return RETURN_CONTINUE;

            if (SCREAM && m_ai.In_Reach(pTarget, SCREAM) && CastSpell(SCREAM) & RETURN_ANY_OK) // TODO: Check for cooldown
                return RETURN_CONTINUE;

            if (MIND_BLAST && m_ai.In_Reach(pTarget, MIND_BLAST) && CastSpell(MIND_BLAST) & RETURN_ANY_OK) // TODO: Check for cooldown
                return RETURN_CONTINUE;

            if (m_ai.GetHealthPercent() < 50 && GREATER_HEAL && m_ai.In_Reach(pTarget, GREATER_HEAL) && CastSpell(GREATER_HEAL) & RETURN_ANY_OK)
                return RETURN_CONTINUE;

            if (SMITE && m_ai.In_Reach(pTarget, SMITE) && CastSpell(SMITE) & RETURN_ANY_OK)
                return RETURN_CONTINUE;

            m_ai.TellMaster("Couldn't find a spell to cast while dueling");
        default:
            break;
    }

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

CombatManeuverReturns PlayerbotPriestAI::HealPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::HealPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    uint8 hp = target->GetHealthPercent();

    if (hp >= 90)
        return RETURN_NO_ACTION_OK;

    // If target is out of range (40 yards) and is a tank: move towards it
    // if bot is not asked to stay
    // Other classes have to adjust their position to the healers
    // TODO: This code should be common to all healers and will probably
    // move to a more suitable place like PlayerbotAI::DoCombatMovement()
    if ((GetTargetJob(target) == JOB_TANK || GetTargetJob(target) == JOB_MAIN_TANK)
            && m_bot.GetPlayerbotAI()->GetMovementOrder() != PlayerbotAI::MOVEMENT_STAY
            && !m_ai.In_Reach(target, FLASH_HEAL))
    {
        m_bot.GetMotionMaster()->MoveFollow(target, 39.0f, m_bot.GetOrientation());
        return RETURN_CONTINUE;
    }

    // Get a free and more efficient heal if needed: low mana for bot or average health for target
    if (m_ai.IsInCombat() && (hp < 50 || m_ai.GetManaPercent() < 40))
        if (INNER_FOCUS > 0 && m_bot.IsSpellReady(INNER_FOCUS) && !m_bot.HasAura(INNER_FOCUS, EFFECT_INDEX_0) && CastSpell(INNER_FOCUS, &m_bot))
            return RETURN_CONTINUE;

    if (hp < 25 && POWER_WORD_SHIELD > 0 && m_ai.IsInCombat() && m_ai.In_Reach(target, POWER_WORD_SHIELD) && !m_bot.HasAura(POWER_WORD_SHIELD, EFFECT_INDEX_0) && !target->HasAura(WEAKNED_SOUL, EFFECT_INDEX_0) && m_ai.CastSpell(POWER_WORD_SHIELD, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (hp < 35 && FLASH_HEAL > 0 && m_ai.In_Reach(target, FLASH_HEAL) && m_ai.CastSpell(FLASH_HEAL, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (hp < 50 && GREATER_HEAL > 0 && m_ai.In_Reach(target, GREATER_HEAL) && m_ai.CastSpell(GREATER_HEAL, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (hp < 70 && HEAL > 0 && m_ai.In_Reach(target, HEAL) && m_ai.CastSpell(HEAL, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (hp < 90 && RENEW > 0 && m_ai.In_Reach(target, RENEW) && !target->HasAura(RENEW) && m_ai.CastSpell(RENEW, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    // Group heal. Not really useful until a group check is available?
    //if (hp < 40 && PRAYER_OF_HEALING > 0 && m_ai.CastSpell(PRAYER_OF_HEALING, *target) & RETURN_CONTINUE)
    //    return RETURN_CONTINUE;

    return RETURN_NO_ACTION_OK;
} // end HealTarget

CombatManeuverReturns PlayerbotPriestAI::ResurrectPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::ResurrectPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    if (m_ai.IsInCombat())     // Just in case as this was supposedly checked before calling this function
        return RETURN_NO_ACTION_ERROR;

    if (RESURRECTION > 0 && m_ai.In_Reach(target, RESURRECTION) && m_ai.CastSpell(RESURRECTION, *target) == SPELL_CAST_OK)
    {
        std::string msg = "Resurrecting ";
        msg += target->GetName();
        m_bot.Say(msg, LANG_UNIVERSAL);
        return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_ERROR; // not error per se - possibly just OOM
}

CombatManeuverReturns PlayerbotPriestAI::DispelPlayer(Player* /*target*/)
{
    // Remove negative magic on group members
    if (Player* cursedTarget = GetDispelTarget(DISPEL_MAGIC))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(cursedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (PRIEST_DISPEL_MAGIC > 0 && CastSpell(PRIEST_DISPEL_MAGIC, cursedTarget))
            return RETURN_CONTINUE;
    }

    // Remove disease on group members
    if (Player* diseasedTarget = GetDispelTarget(DISPEL_DISEASE))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(diseasedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        uint32 cure = ABOLISH_DISEASE > 0 ? ABOLISH_DISEASE : CURE_DISEASE;
        if (cure > 0 && CastSpell(cure, diseasedTarget))
            return RETURN_CONTINUE;
    }

    return RETURN_NO_ACTION_OK;
}

void PlayerbotPriestAI::DoNonCombatActions()
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return;

    uint32 spec = m_bot.GetSpec();

    // selfbuff goes first
    if (m_ai.SelfBuff(INNER_FIRE) == SPELL_CAST_OK)
        return;

    // Dispel magic/disease
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return;

    // Revive
    if (ResurrectPlayer(GetResurrectionTarget()) & RETURN_CONTINUE)
        return;

    // After revive
    if (spec == PRIEST_SPEC_SHADOW && SHADOWFORM > 0)
        m_ai.SelfBuff(SHADOWFORM);
    if (VAMPIRIC_EMBRACE > 0)
        m_ai.SelfBuff(VAMPIRIC_EMBRACE);

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

    // Buffing
    // the check for group targets is performed by NeedGroupBuff (if group is found for bots by the function)
    if (NeedGroupBuff(PRAYER_OF_FORTITUDE, POWER_WORD_FORTITUDE) && m_ai.HasSpellReagents(PRAYER_OF_FORTITUDE))
    {
        if (Buff(&PlayerbotPriestAI::BuffHelper, PRAYER_OF_FORTITUDE) & RETURN_CONTINUE)
            return;
    }
    else if (Buff(&PlayerbotPriestAI::BuffHelper, POWER_WORD_FORTITUDE) & RETURN_CONTINUE)
        return;

    if (NeedGroupBuff(PRAYER_OF_SPIRIT, DIVINE_SPIRIT) && m_ai.HasSpellReagents(PRAYER_OF_FORTITUDE))
    {
        if (Buff(&PlayerbotPriestAI::BuffHelper, PRAYER_OF_SPIRIT) & RETURN_CONTINUE)
            return;
    }
    else if (Buff(&PlayerbotPriestAI::BuffHelper, DIVINE_SPIRIT, (JOB_ALL | JOB_MANAONLY)) & RETURN_CONTINUE)
        return;

    if (NeedGroupBuff(PRAYER_OF_SHADOW_PROTECTION, SHADOW_PROTECTION) && m_ai.HasSpellReagents(PRAYER_OF_FORTITUDE))
    {
        if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_SHADOW && Buff(&PlayerbotPriestAI::BuffHelper, PRAYER_OF_SHADOW_PROTECTION) & RETURN_CONTINUE)
            return;
    }
    else if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_SHADOW && Buff(&PlayerbotPriestAI::BuffHelper, SHADOW_PROTECTION) & RETURN_CONTINUE)
        return;

    if (EatDrinkBandage())
        return;

    // Nothing else to do, Night Elves will cast Shadowmeld to reduce their aggro versus patrols or nearby mobs
    if (SHADOWMELD > 0 && !m_bot.isMovingOrTurning()
        && !m_bot.IsMounted()
        && !m_bot.HasAura(SHADOWMELD, EFFECT_INDEX_0))
    {
        m_ai.CastSpell(SHADOWMELD, m_bot);
    }
} // end DoNonCombatActions

// TODO: this and mage's BuffHelper are identical and thus could probably go in PlayerbotClassAI.cpp somewhere
bool PlayerbotPriestAI::BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target)
{
    if (!ai)          return false;
    if (spellId == 0) return false;
    if (!target)      return false;

    Pet* pet = target->GetPet();
    if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->Buff(spellId, pet) == SPELL_CAST_OK)
        return true;

    return ai->Buff(spellId, target) == SPELL_CAST_OK;
}

bool PlayerbotPriestAI::CastHoTOnTank()
{
    if (!m_ai.IsHealer()) return false;

    // Priest HoTs: Renew, Penance (with talents, channeled)
    if (RENEW > 0 && m_ai.In_Reach(m_ai.GetGroupTank(), RENEW))
        return (RETURN_CONTINUE & CastSpell(RENEW, m_ai.GetGroupTank()));

    return false;
}

// Return to UpdateAI the spellId usable to neutralize a target with creaturetype
uint32 PlayerbotPriestAI::Neutralize(uint8 creatureType)
{
    if (!creatureType)  return 0;

    if (creatureType != CREATURE_TYPE_UNDEAD)
    {
        m_ai.TellMaster("I can't shackle that target.");
        return 0;
    }

    if (SHACKLE_UNDEAD > 0)
        return SHACKLE_UNDEAD;
    else
        return 0;
}
