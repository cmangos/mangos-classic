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

#include "PlayerbotPaladinAI.h"
#include "../Base/PlayerbotMgr.h"

class PlayerbotAI;

PlayerbotPaladinAI::PlayerbotPaladinAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    RETRIBUTION_AURA              = m_ai.initSpell(RETRIBUTION_AURA_1);
    SEAL_OF_COMMAND               = m_ai.initSpell(SEAL_OF_COMMAND_1);
    SEAL_OF_RIGHTEOUSNESS         = m_ai.initSpell(SEAL_OF_RIGHTEOUSNESS_1);
    SEAL_OF_JUSTICE               = m_ai.initSpell(SEAL_OF_JUSTICE_1);
    SEAL_OF_LIGHT                 = m_ai.initSpell(SEAL_OF_LIGHT_1);
    SEAL_OF_WISDOM                = m_ai.initSpell(SEAL_OF_WISDOM_1);
    SEAL_OF_THE_CRUSADER          = m_ai.initSpell(SEAL_OF_THE_CRUSADER_1);
    JUDGEMENT                     = m_ai.initSpell(JUDGEMENT_1);
    BLESSING_OF_MIGHT             = m_ai.initSpell(BLESSING_OF_MIGHT_1);
    GREATER_BLESSING_OF_MIGHT     = m_ai.initSpell(GREATER_BLESSING_OF_MIGHT_1);
    HAMMER_OF_WRATH               = m_ai.initSpell(HAMMER_OF_WRATH_1);
    FLASH_OF_LIGHT                = m_ai.initSpell(FLASH_OF_LIGHT_1); // Holy
    HOLY_LIGHT                    = m_ai.initSpell(HOLY_LIGHT_1);
    HOLY_SHOCK                    = m_ai.initSpell(HOLY_SHOCK_1);
    HOLY_WRATH                    = m_ai.initSpell(HOLY_WRATH_1);
    DIVINE_FAVOR                  = m_ai.initSpell(DIVINE_FAVOR_1);
    CONCENTRATION_AURA            = m_ai.initSpell(CONCENTRATION_AURA_1);
    BLESSING_OF_WISDOM            = m_ai.initSpell(BLESSING_OF_WISDOM_1);
    GREATER_BLESSING_OF_WISDOM    = m_ai.initSpell(GREATER_BLESSING_OF_WISDOM_1);
    CONSECRATION                  = m_ai.initSpell(CONSECRATION_1);
    LAY_ON_HANDS                  = m_ai.initSpell(LAY_ON_HANDS_1);
    EXORCISM                      = m_ai.initSpell(EXORCISM_1);
    BLESSING_OF_KINGS             = m_ai.initSpell(BLESSING_OF_KINGS_1);
    GREATER_BLESSING_OF_KINGS     = m_ai.initSpell(GREATER_BLESSING_OF_KINGS_1);
    BLESSING_OF_SANCTUARY         = m_ai.initSpell(BLESSING_OF_SANCTUARY_1);
    GREATER_BLESSING_OF_SANCTUARY = m_ai.initSpell(GREATER_BLESSING_OF_SANCTUARY_1);
    HAMMER_OF_JUSTICE             = m_ai.initSpell(HAMMER_OF_JUSTICE_1);
    RIGHTEOUS_FURY                = m_ai.initSpell(RIGHTEOUS_FURY_1);
    SHADOW_RESISTANCE_AURA        = m_ai.initSpell(SHADOW_RESISTANCE_AURA_1);
    DEVOTION_AURA                 = m_ai.initSpell(DEVOTION_AURA_1);
    FIRE_RESISTANCE_AURA          = m_ai.initSpell(FIRE_RESISTANCE_AURA_1);
    FROST_RESISTANCE_AURA         = m_ai.initSpell(FROST_RESISTANCE_AURA_1);
    BLESSING_OF_PROTECTION        = m_ai.initSpell(BLESSING_OF_PROTECTION_1);
    DIVINE_PROTECTION             = m_ai.initSpell(DIVINE_PROTECTION_1);
    DIVINE_INTERVENTION           = m_ai.initSpell(DIVINE_INTERVENTION_1);
    DIVINE_SHIELD                 = m_ai.initSpell(DIVINE_SHIELD_1);
    HOLY_SHIELD                   = m_ai.initSpell(HOLY_SHIELD_1);
    BLESSING_OF_SACRIFICE         = m_ai.initSpell(BLESSING_OF_SACRIFICE_1);
    REDEMPTION                    = m_ai.initSpell(REDEMPTION_1);
    PURIFY                        = m_ai.initSpell(PURIFY_1);
    CLEANSE                       = m_ai.initSpell(CLEANSE_1);

    FORBEARANCE                   = 25771; // cannot be protected

    RECENTLY_BANDAGED             = 11196; // first aid check

    // racial
    STONEFORM                     = m_ai.initSpell(STONEFORM_ALL); // dwarf
    PERCEPTION                    = m_ai.initSpell(PERCEPTION_ALL); // human

    //The check doesn't work for now
    //PRAYER_OF_SHADOW_PROTECTION   = m_ai.initSpell(PriestSpells::PRAYER_OF_SHADOW_PROTECTION_1);
}

PlayerbotPaladinAI::~PlayerbotPaladinAI() {}

CombatManeuverReturns PlayerbotPaladinAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.GroupTankHoldsAggro())
        {
            if (PlayerbotAI::ORDERS_TANK & m_ai.GetCombatOrder())
            {
                if (m_bot.CanReachWithMeleeAttack(pTarget))
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

    // Check if bot needs to cast seal on self
    m_CurrentSeal      = 0;
    m_CurrentJudgement = 0;

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

CombatManeuverReturns PlayerbotPaladinAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotPaladinAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotPaladinAI::DoNextCombatManeuver(Unit* pTarget)
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

CombatManeuverReturns PlayerbotPaladinAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    if (!pTarget) return RETURN_NO_ACTION_INVALIDTARGET;

    // damage spells
    uint32 spec = m_bot.GetSpec();
    std::ostringstream out;

    // Make sure healer stays put, don't even melee (aggro) if in range.
    if (m_ai.IsHealer() && m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_RANGED)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    else if (!m_ai.IsHealer() && m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    // Emergency check: bot is about to die: use Divine Shield (first)
    // Use Divine Protection if Divine Shield is not available and bot is not tanking because of the pacify effect
    // TODO adjust treshold (may be too low)
    if (m_ai.GetHealthPercent() < 8)
    {
        if (DIVINE_SHIELD > 0 && m_bot.IsSpellReady(DIVINE_SHIELD) && !m_bot.HasAura(DIVINE_SHIELD, EFFECT_INDEX_0) && !m_bot.HasAura(DIVINE_PROTECTION, EFFECT_INDEX_0) && !m_bot.HasAura(FORBEARANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DIVINE_SHIELD, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;

        if (DIVINE_PROTECTION > 0 && !(m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK) && m_bot.IsSpellReady(DIVINE_PROTECTION) && !m_bot.HasAura(DIVINE_SHIELD, EFFECT_INDEX_0) && !m_bot.HasAura(DIVINE_PROTECTION, EFFECT_INDEX_0) && !m_bot.HasAura(FORBEARANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DIVINE_PROTECTION, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    // Dispel magic/disease/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    // Check if bot needs to cast a seal on self or judge the target
    if (CheckSealAndJudgement(pTarget))
        return RETURN_CONTINUE;

    // Heal (try to pick a target by on common rules, than heal using each PlayerbotClassAI HealPlayer() method)
    if (FindTargetAndHeal())
        return RETURN_CONTINUE;

    //Used to determine if this bot has highest threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);
    if (newTarget && !(m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TANK) && !m_ai.IsNeutralized(newTarget)) // TODO: && party has a tank
    {
        // Aggroed by an elite
        if (m_ai.IsElite(newTarget))
        {
            // Try to stun the mob
            if (HAMMER_OF_JUSTICE > 0 && m_ai.In_Reach(newTarget, HAMMER_OF_JUSTICE) && m_bot.IsSpellReady(HAMMER_OF_JUSTICE) && !newTarget->HasAura(HAMMER_OF_JUSTICE) && m_ai.CastSpell(HAMMER_OF_JUSTICE, *newTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;

            // Bot has low life: use divine powers to protect him/herself
            if (m_ai.GetHealthPercent() < 15)
            {
                if (DIVINE_SHIELD > 0 && m_bot.IsSpellReady(DIVINE_SHIELD) && !m_bot.HasAura(DIVINE_SHIELD, EFFECT_INDEX_0) && !m_bot.HasAura(DIVINE_PROTECTION, EFFECT_INDEX_0) && !m_bot.HasAura(FORBEARANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DIVINE_SHIELD, m_bot) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;

                if (DIVINE_PROTECTION > 0 && m_bot.IsSpellReady(DIVINE_PROTECTION) && !m_bot.HasAura(DIVINE_SHIELD, EFFECT_INDEX_0) && !m_bot.HasAura(DIVINE_PROTECTION, EFFECT_INDEX_0) && !m_bot.HasAura(FORBEARANCE, EFFECT_INDEX_0) && m_ai.CastSpell(DIVINE_PROTECTION, m_bot) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
            }

            // Else: do nothing and pray for tank to pick aggro from mob
            return RETURN_NO_ACTION_OK;
        }
    }

    // Damage rotation
    switch (spec)
    {
        case PALADIN_SPEC_HOLY:
            if (m_ai.IsHealer())
                return RETURN_NO_ACTION_OK;
        // else: DPS (retribution, NEVER protection)

        case PALADIN_SPEC_RETRIBUTION:
            if (HAMMER_OF_WRATH > 0 && pTarget->GetHealth() < pTarget->GetMaxHealth() * 0.20 && m_ai.CastSpell(HAMMER_OF_WRATH, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            return RETURN_CONTINUE;

        case PALADIN_SPEC_PROTECTION:
            //Taunt if orders specify
            if (CONSECRATION > 0 && m_bot.IsSpellReady(CONSECRATION) && m_ai.CastSpell(CONSECRATION, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (HOLY_SHIELD > 0 && !m_bot.HasAura(HOLY_SHIELD) && m_ai.CastSpell(HOLY_SHIELD, m_bot) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (SHIELD_OF_RIGHTEOUSNESS > 0 && m_bot.IsSpellReady(SHIELD_OF_RIGHTEOUSNESS) && m_ai.CastSpell(SHIELD_OF_RIGHTEOUSNESS, *pTarget) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            return RETURN_NO_ACTION_OK;
    }

    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotPaladinAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (m_ai.CastSpell(HAMMER_OF_JUSTICE) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

CombatManeuverReturns PlayerbotPaladinAI::HealPlayer(Player* target)
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
            && !m_ai.In_Reach(target, FLASH_OF_LIGHT))
    {
        m_bot.GetMotionMaster()->MoveFollow(target, 39.0f, m_bot.GetOrientation());
        return RETURN_CONTINUE;
    }

    uint8 hp = target->GetHealthPercent();

    // Everyone is healthy enough, return OK. MUST correlate to highest value below (should be last HP check)
    if (hp >= 80)
        return RETURN_NO_ACTION_OK;

    if (hp < 10 && LAY_ON_HANDS && m_bot.IsSpellReady(LAY_ON_HANDS) && m_ai.In_Reach(target, LAY_ON_HANDS) && m_ai.CastSpell(LAY_ON_HANDS, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    // Target is a moderately wounded healer or a badly wounded not tank? Blessing of Protection!
    if (BLESSING_OF_PROTECTION > 0
            && ((hp < 25 && (GetTargetJob(target) & JOB_HEAL || GetTargetJob(target) & JOB_MAIN_HEAL))
            || (hp < 15 && !(GetTargetJob(target) & JOB_TANK) && !(GetTargetJob(target) & JOB_MAIN_TANK)))
            && m_bot.IsSpellReady(BLESSING_OF_PROTECTION) && m_ai.In_Reach(target, BLESSING_OF_PROTECTION)
            && !target->HasAura(FORBEARANCE, EFFECT_INDEX_0)
            && !target->HasAura(BLESSING_OF_PROTECTION, EFFECT_INDEX_0) && !target->HasAura(DIVINE_PROTECTION, EFFECT_INDEX_0)
            && !target->HasAura(DIVINE_SHIELD, EFFECT_INDEX_0)
            && m_ai.CastSpell(BLESSING_OF_PROTECTION, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    // Low HP : activate Divine Favor to make next heal a critical heal
    if (hp < 25 && DIVINE_FAVOR > 0 && !m_bot.HasAura(DIVINE_FAVOR, EFFECT_INDEX_0) && m_bot.IsSpellReady(DIVINE_FAVOR))
        m_ai.CastSpell(DIVINE_FAVOR, m_bot);

    if (hp < 40 && HOLY_LIGHT > 0 && m_ai.In_Reach(target, HOLY_LIGHT) && m_ai.CastSpell(HOLY_LIGHT, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    if (hp < 60 && HOLY_SHOCK > 0 && m_ai.In_Reach(target, HOLY_SHOCK) && m_ai.CastSpell(HOLY_SHOCK, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    if (hp < 80 && FLASH_OF_LIGHT > 0 && m_ai.In_Reach(target, FLASH_OF_LIGHT) && m_ai.CastSpell(FLASH_OF_LIGHT, *target) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_UNKNOWN;
} // end HealTarget

CombatManeuverReturns PlayerbotPaladinAI::ResurrectPlayer(Player* target)
{
    CombatManeuverReturns r = PlayerbotClassAI::ResurrectPlayer(target);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    if (m_ai.IsInCombat())     // Just in case as this was supposedly checked before calling this function
        return RETURN_NO_ACTION_ERROR;

    if (REDEMPTION > 0 && m_ai.In_Reach(target, REDEMPTION) && m_ai.CastSpell(REDEMPTION, *target) == SPELL_CAST_OK)
    {
        std::string msg = "Resurrecting ";
        msg += target->GetName();
        m_bot.Say(msg, LANG_UNIVERSAL);
        return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_ERROR; // not error per se - possibly just OOM
}

CombatManeuverReturns PlayerbotPaladinAI::DispelPlayer(Player* /*target*/)
{
    uint32 dispel = CLEANSE > 0 ? CLEANSE : PURIFY;
    // Remove negative magic on group members
    if (Player* cursedTarget = GetDispelTarget(DISPEL_MAGIC))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(cursedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (dispel > 0 && m_ai.CastSpell(dispel, *cursedTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    // Remove poison on group members
    if (Player* poisonedTarget = GetDispelTarget(DISPEL_POISON))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(poisonedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (dispel > 0 && m_ai.CastSpell(dispel, *poisonedTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }

    // Remove disease on group members
    if (Player* diseasedTarget = GetDispelTarget(DISPEL_DISEASE))
    {
        CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(diseasedTarget);
        if (r != RETURN_NO_ACTION_OK)
            return r;

        if (dispel > 0 && m_ai.CastSpell(dispel, *diseasedTarget) == SPELL_CAST_OK)
            return RETURN_CONTINUE;
    }
    return RETURN_NO_ACTION_OK;
}

void PlayerbotPaladinAI::CheckAuras()
{
    uint32 spec = m_bot.GetSpec();

    // If we have resist orders, adjust accordingly
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FROST)
    {
        if (!m_bot.HasAura(FROST_RESISTANCE_AURA) && FROST_RESISTANCE_AURA > 0 && !m_bot.HasAura(FROST_RESISTANCE_AURA))
            m_ai.CastSpell(FROST_RESISTANCE_AURA);
        return;
    }
    else if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FIRE)
    {
        if (!m_bot.HasAura(FIRE_RESISTANCE_AURA) && FIRE_RESISTANCE_AURA > 0 && !m_bot.HasAura(FIRE_RESISTANCE_AURA))
            m_ai.CastSpell(FIRE_RESISTANCE_AURA);
        return;
    }
    else if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_SHADOW)
    {
        // Shadow protection check is broken, they stack!
        if (!m_bot.HasAura(SHADOW_RESISTANCE_AURA) && SHADOW_RESISTANCE_AURA > 0 && !m_bot.HasAura(SHADOW_RESISTANCE_AURA)) // /*&& !m_bot.HasAura(PRAYER_OF_SHADOW_PROTECTION)*/ /*&& !m_bot.HasAura(PRAYER_OF_SHADOW_PROTECTION)*/
            m_ai.CastSpell(SHADOW_RESISTANCE_AURA);
        return;
    }

    // if there is a tank in the group, use concentration aura
    bool tankInGroup = false;
    if (m_bot.GetGroup())
    {
        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (auto itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* groupMember = sObjectMgr.GetPlayer(itr->guid);
            if (!groupMember)
                continue;

            if (GetTargetJob(groupMember) & JOB_TANK || GetTargetJob(groupMember) & JOB_MAIN_TANK)
            {
                tankInGroup = true;
                break;
            }
        }
    }

    // If we have no resist orders, adjust aura based on spec or tank
    if (spec == PALADIN_SPEC_PROTECTION || tankInGroup)
    {
        if (DEVOTION_AURA > 0 && !m_bot.HasAura(DEVOTION_AURA))
            m_ai.CastSpell(DEVOTION_AURA);
        return;
    }
    else if (spec == PALADIN_SPEC_HOLY)
    {
        if (CONCENTRATION_AURA > 0 && !m_bot.HasAura(CONCENTRATION_AURA))
            m_ai.CastSpell(CONCENTRATION_AURA);
        return;
    }
    else if (spec == PALADIN_SPEC_RETRIBUTION)
    {
        if (RETRIBUTION_AURA > 0 && !m_bot.HasAura(RETRIBUTION_AURA))
            m_ai.CastSpell(RETRIBUTION_AURA);
        return;
    }
}

// Check if the paladin bot needs to cast/refresh a seal on him/herself
// also check if the paladin bot needs to judge its target and first buff
// him/herself with the relevant seal
// TODO: handle other paladins in group/raid, for example to cast Seal/Judgement of Light
bool PlayerbotPaladinAI::CheckSealAndJudgement(Unit* target)
{
    if (!target)   return false;

    if (target->GetTypeId() == TYPEID_UNIT)
    {
        auto* creature = (Creature*) target;
        // Prevent low health humanoid from fleeing by judging them with Seal of Justice
        if (creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID && target->GetHealthPercent() < 20 && !creature->IsWorldBoss())
        {
            if (SEAL_OF_JUSTICE > 0 && !m_bot.HasAura(SEAL_OF_JUSTICE, EFFECT_INDEX_0) && m_ai.CastSpell(SEAL_OF_JUSTICE, m_bot) == SPELL_CAST_OK)
            {
                m_CurrentSeal = SEAL_OF_JUSTICE;
                m_CurrentJudgement = 0;
                return true;
            }
        }
    }

    // Bot already defined a seal and a judgement and each is active on bot and target: don't waste time to go further
    if (m_CurrentSeal > 0 && m_bot.HasAura(m_CurrentSeal, EFFECT_INDEX_0) && m_CurrentJudgement > 0 && target->HasAura(m_CurrentJudgement, EFFECT_INDEX_0))
        return false;

    // Refresh judgement if needed by forcing paladin bot to cast seal and judgement anew
    // But first, unleash current seal if bot can do extra damage to the target in the process
    if (m_CurrentJudgement > 0 && !target->HasAura(m_CurrentJudgement, EFFECT_INDEX_0))
    {
        if (m_bot.HasAura(SEAL_OF_COMMAND, EFFECT_INDEX_0) || m_bot.HasAura(SEAL_OF_RIGHTEOUSNESS, EFFECT_INDEX_0))
        {
            if (JUDGEMENT > 0 && m_bot.IsSpellReady(JUDGEMENT) && m_ai.In_Reach(target, JUDGEMENT))
                m_ai.CastSpell(JUDGEMENT, *target);
        }

        m_CurrentJudgement = 0;
        m_CurrentSeal = 0;
        return false;
    }

    // Judgement is still active on target: refresh seal on bot if needed
    if (m_CurrentJudgement > 0 && m_CurrentSeal > 0 && !m_bot.HasAura(m_CurrentSeal, EFFECT_INDEX_0))
    {
        if (m_ai.CastSpell(m_CurrentSeal, m_bot) == SPELL_CAST_OK)
            return true;
    }

    // No judgement on target but bot has seal active: time to judge the target
    if (m_CurrentJudgement == 0 && m_CurrentSeal > 0 && m_bot.HasAura(m_CurrentSeal, EFFECT_INDEX_0))
    {
        if (JUDGEMENT > 0 && m_bot.IsSpellReady(JUDGEMENT) && m_ai.In_Reach(target, JUDGEMENT) && m_ai.CastSpell(JUDGEMENT, *target) == SPELL_CAST_OK)
        {
            if (m_CurrentSeal == SEAL_OF_JUSTICE)
                m_CurrentJudgement = JUDGEMENT_OF_JUSTICE;
            else if (m_CurrentSeal == SEAL_OF_WISDOM)
                m_CurrentJudgement = JUDGEMENT_OF_WISDOM;
            else if (m_CurrentSeal == SEAL_OF_THE_CRUSADER)
                m_CurrentJudgement = JUDGEMENT_OF_THE_CRUSADER;
            else
                return false;

            // Set current seal to 0 to force the bot to seal him/herself for combat now that the target is judged
            m_CurrentSeal = 0;
            return true;
        }

        return false;
    }

    // Now bot casts seal on him/herself
    // No judgement on target: look for best seal to judge target next
    // Target already judged: bot will buff him/herself for combat according to spec/orders
    uint32 spec = m_bot.GetSpec();

    // Bypass spec if combat orders were given
    if (m_ai.IsHealer()) spec = PALADIN_SPEC_HOLY;
    if (m_ai.IsTank()) spec = PALADIN_SPEC_PROTECTION;
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_ASSIST) spec = PALADIN_SPEC_RETRIBUTION;

    if (m_CurrentJudgement == 0)
    {
        if (spec == PALADIN_SPEC_HOLY)
            m_CurrentSeal = SEAL_OF_WISDOM;
        else
            m_CurrentSeal = SEAL_OF_THE_CRUSADER;
    }
    else
    {
        switch (spec)
        {
            case PALADIN_SPEC_HOLY:
                m_CurrentSeal = SEAL_OF_WISDOM;
                break;
            case PALADIN_SPEC_RETRIBUTION:
                if (SEAL_OF_COMMAND > 0)
                {
                    m_CurrentSeal = SEAL_OF_COMMAND;
                    break;
                }
            case PALADIN_SPEC_PROTECTION:
            default:    // No spec: try Seal of Righteouness
                m_CurrentSeal = SEAL_OF_RIGHTEOUSNESS;
        }
    }

    return m_CurrentSeal > 0 && !m_bot.HasAura(m_CurrentSeal, EFFECT_INDEX_0) && m_ai.CastSpell(m_CurrentSeal, m_bot) == SPELL_CAST_OK;
}

void PlayerbotPaladinAI::DoNonCombatActions()
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return;

    CheckAuras();

    //Put up RF if tank
    if (m_ai.IsTank())
        m_ai.SelfBuff(RIGHTEOUS_FURY);
    //Disable RF if not tank
    else if (m_bot.HasAura(RIGHTEOUS_FURY))
        m_bot.RemoveAurasDueToSpell(RIGHTEOUS_FURY);

    // Dispel magic/disease/poison
    if (m_ai.HasDispelOrder() && DispelPlayer() & RETURN_CONTINUE)
        return;

    // Revive
    if (ResurrectPlayer(GetResurrectionTarget()) & RETURN_CONTINUE)
        return;

    // Heal
    if (m_ai.IsHealer())
    {
        if (HealPlayer(GetHealTarget()) & RETURN_CONTINUE)
            return; // RETURN_CONTINUE;
    }
    else
    {
        // Is this desirable? Debatable.
        // TODO: In a group/raid with a healer you'd want this bot to focus on DPS (it's not specced/geared for healing either)
        if (HealPlayer(&m_bot) & RETURN_CONTINUE)
            return; // RETURN_CONTINUE;
    }

    // buff group
    if (Buff(&PlayerbotPaladinAI::BuffHelper, 1) & RETURN_CONTINUE) // Paladin's BuffHelper takes care of choosing the specific Blessing so just pass along a non-zero value
        return;

    // hp/mana check
    if (EatDrinkBandage())
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
}

/**
 * BuffHelper
 * BuffHelper is a static function, takes an AI, spellId (ignored for paladin) and a target and attempts to buff them as well as their pets as
 * best as possible.
 *
 * Return bool - returns true if a buff took place.
 */
bool PlayerbotPaladinAI::BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target)
{
    if (!ai)          return false;
    if (spellId == 0) return false;
    if (!target)      return false;

    auto* c = (PlayerbotPaladinAI*) ai->GetClassAI();
    uint32 bigSpellId = 0;

    Pet* pet = target->GetPet();
    uint32 petSpellId = 0, petBigSpellId = 0;

    // See which buff is appropriate according to class
    // TODO: take into account other paladins in the group
    switch (target->getClass())
    {
        case CLASS_DRUID:
        case CLASS_SHAMAN:
        case CLASS_PALADIN:
            spellId = c->BLESSING_OF_MIGHT;
            if (!spellId)
            {
                spellId = c->BLESSING_OF_KINGS;
                if (!spellId)
                {
                    spellId = c->BLESSING_OF_WISDOM;
                    if (!spellId)
                    {
                        spellId = c->BLESSING_OF_SANCTUARY;
                        if (!spellId)
                            return false;
                    }
                }
            }
            break;
        case CLASS_HUNTER:
            if (pet && ai->CanReceiveSpecificSpell(SPELL_BLESSING, pet) && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
            {
                petSpellId = c->BLESSING_OF_MIGHT;
                if (!petSpellId)
                {
                    petSpellId = c->BLESSING_OF_KINGS;
                    if (!petSpellId)
                        petSpellId = c->BLESSING_OF_SANCTUARY;
                }
            }
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            spellId = c->BLESSING_OF_MIGHT;
            if (!spellId)
            {
                spellId = c->BLESSING_OF_KINGS;
                if (!spellId)
                {
                    spellId = c->BLESSING_OF_SANCTUARY;
                    if (!spellId)
                        return false;
                }
            }
            break;
        case CLASS_WARLOCK:
            if (pet && ai->CanReceiveSpecificSpell(SPELL_BLESSING, pet) && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
            {
                if (pet->GetPowerType() == POWER_MANA)
                    petSpellId = c->BLESSING_OF_WISDOM;
                else
                    petSpellId = c->BLESSING_OF_MIGHT;

                if (!petSpellId)
                {
                    petSpellId = c->BLESSING_OF_KINGS;
                    if (!petSpellId)
                        petSpellId = c->BLESSING_OF_SANCTUARY;
                }
            }
        case CLASS_PRIEST:
        case CLASS_MAGE:
            spellId = c->BLESSING_OF_WISDOM;
            if (!spellId)
            {
                spellId = c->BLESSING_OF_KINGS;
                if (!spellId)
                {
                    spellId = c->BLESSING_OF_SANCTUARY;
                    if (!spellId)
                        return false;
                }
            }
            break;
    }

    if (petSpellId == c->BLESSING_OF_MIGHT)
        petBigSpellId = c->GREATER_BLESSING_OF_MIGHT;
    else if (petSpellId == c->BLESSING_OF_WISDOM)
        petBigSpellId = c->GREATER_BLESSING_OF_WISDOM;
    else if (petSpellId == c->BLESSING_OF_KINGS)
        petBigSpellId = c->GREATER_BLESSING_OF_KINGS;
    else if (petSpellId == c->BLESSING_OF_SANCTUARY)
        petBigSpellId = c->GREATER_BLESSING_OF_SANCTUARY;

    if (spellId == c->BLESSING_OF_MIGHT)
        bigSpellId = c->GREATER_BLESSING_OF_MIGHT;
    else if (spellId == c->BLESSING_OF_WISDOM)
        bigSpellId = c->GREATER_BLESSING_OF_WISDOM;
    else if (spellId == c->BLESSING_OF_KINGS)
        bigSpellId = c->GREATER_BLESSING_OF_KINGS;
    else if (spellId == c->BLESSING_OF_SANCTUARY)
        bigSpellId = c->GREATER_BLESSING_OF_SANCTUARY;

    if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->HasSpellReagents(petBigSpellId) && ai->Buff(petBigSpellId, pet) == SPELL_CAST_OK)
        return true;
    if (ai->HasSpellReagents(bigSpellId) && ai->Buff(bigSpellId, target) == SPELL_CAST_OK)
        return true;
    return (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->Buff(petSpellId, pet) == SPELL_CAST_OK) || ai->Buff(spellId, target) == SPELL_CAST_OK;
}

// Match up with "Pull()" below
bool PlayerbotPaladinAI::CanPull()
{
    if (HAND_OF_RECKONING > 0 && m_bot.IsSpellReady(HAND_OF_RECKONING))
        return true;
    return EXORCISM > 0 && m_bot.IsSpellReady(EXORCISM);
}

// Match up with "CanPull()" above
bool PlayerbotPaladinAI::Pull()
{
    return EXORCISM > 0 && m_ai.CastSpell(EXORCISM) == SPELL_CAST_OK;
}

bool PlayerbotPaladinAI::CastHoTOnTank()
{
    if (!m_ai.IsHealer()) return false;

    // Paladin: Sheath of Light (with talents), Flash of Light (with Infusion of Light talent and only on a target with the Sacred Shield buff),
    //          Holy Shock (with Tier 8 set bonus)
    // None of these are HoTs to cast before pulling (I think)

    return false;
}
